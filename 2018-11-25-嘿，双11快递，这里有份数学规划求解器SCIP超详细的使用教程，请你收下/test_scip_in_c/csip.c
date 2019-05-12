#include <string.h>

#include "csip.h"
#include "nlpi/pub_expr.h"
#include "scip/scip.h"
#include "scip/pub_misc.h"
#include "scip/pub_var.h"
#include "scip/scipdefplugins.h"

#pragma comment (lib,"scip.lib")

#define CSIP_MAJOR_VERSION 0
#define CSIP_MINOR_VERSION 6
#define CSIP_PATCH_VERSION 0

/* objective type */
typedef int CSIP_OBJTYPE;
#define CSIP_OBJTYPE_LINEAR 0
#define CSIP_OBJTYPE_NONLINEAR 1

// map return codes: SCIP -> CSIP
static inline int retCodeSCIPtoCSIP(int scipRetCode)
{
	switch (scipRetCode)
	{
	case SCIP_OKAY:
		return CSIP_RETCODE_OK;
	case SCIP_NOMEMORY:
		return CSIP_RETCODE_NOMEMORY;
	default: // all the same for us
		return CSIP_RETCODE_ERROR;
	}
}

// map return codes: CSIP -> SCIP
static inline int retCodeCSIPtoSCIP(int csipRetCode)
{
	switch (csipRetCode)
	{
	case CSIP_RETCODE_OK:
		return SCIP_OKAY;
	case CSIP_RETCODE_NOMEMORY:
		return SCIP_NOMEMORY;
	default: // CSIP_RETCODE_ERROR
		return SCIP_ERROR;
	}
}


// catch return code within CSIP, like SCIP_CALL defined in SCIP
#define CSIP_CALL(x)                                                       \
    do {                                                                   \
        CSIP_RETCODE _retcode = (x);                                       \
        if(_retcode != CSIP_RETCODE_OK)                                    \
        {                                                                  \
            printf("CSIP: failing with retcode %d at %s:%d\n",             \
                   _retcode, __FILE__, __LINE__);                          \
            exit(1);                                                       \
            return _retcode;                                               \
        }                                                                  \
    } while(0)

// catch SCIP return code from CSIP
#define SCIP_in_CSIP(x) CSIP_CALL( retCodeSCIPtoCSIP(x) )

// catch CSIP return code from SCIP
#define CSIP_in_SCIP(x) SCIP_CALL( retCodeCSIPtoSCIP(x) )

// variable sized arrays
#define INITIALSIZE 64
#define GROWFACTOR   2

struct csip_model
{
	SCIP *scip;

	// variable sized array for variables
	int nvars;
	int varssize;
	SCIP_VAR **vars;

	// variable sized array for constraints
	int nconss;
	int consssize;
	SCIP_CONS **conss;

	// counter for callbacks
	int nlazycb;
	int nheur;

	// user-defined solution, is checked before solving
	SCIP_SOL *initialsol;

	// store objective variable for nonlinear objective: the idea is to add an
	// auxiliary constraint and variable to represent nonlinear objectives. If
	// the objective gets change, we set to 0 the objective coefficient of this
	// variable and relax its bounds so the auxiliary constraint is redundant.
	// This is going to mess the model when printed to a file.
	SCIP_VAR *objvar;
	SCIP_CONS *objcons;
	CSIP_OBJTYPE objtype;

	// store message handler to allow for a prefix
	SCIP_MESSAGEHDLR* msghdlr;
};

/*
* local methods
*/

static
CSIP_RETCODE createLinCons(CSIP_MODEL *model, int numindices, int *indices,
	double *coefs, double lhs, double rhs, SCIP_CONS **cons)
{
	SCIP *scip;
	SCIP_VAR *var;
	int i;

	scip = model->scip;

	SCIP_in_CSIP(SCIPcreateConsBasicLinear(scip, cons, "lincons", 0, NULL, NULL,
		lhs, rhs));

	for (i = 0; i < numindices; ++i)
	{
		var = model->vars[indices[i]];
		SCIP_in_CSIP(SCIPaddCoefLinear(scip, *cons, var, coefs[i]));
	}

	return CSIP_RETCODE_OK;
}

static
CSIP_RETCODE addCons(CSIP_MODEL *model, SCIP_CONS *cons, int *idx)
{
	SCIP *scip;

	scip = model->scip;

	SCIP_in_CSIP(SCIPaddCons(scip, cons));

	// do we need to resize?
	if (model->nconss >= model->consssize)
	{
		model->consssize = GROWFACTOR * model->consssize;
		model->conss = (SCIP_CONS **)realloc(
			model->conss, model->consssize * sizeof(SCIP_CONS *));
		if (model->conss == NULL)
		{
			return CSIP_RETCODE_NOMEMORY;
		}
	}

	if (idx != NULL)
	{
		*idx = model->nconss;
		model->conss[*idx] = cons;
	}
	else
	{
		model->conss[model->nconss] = cons;
	}

	++(model->nconss);

	return CSIP_RETCODE_OK;
}

static
CSIP_RETCODE createExprtree(
	CSIP_MODEL *model, int nops, CSIP_OP *ops, int *children, int *begin,
	double *values, SCIP_EXPRTREE **tree)
{
	SCIP *scip;
	SCIP_EXPR **exprs;
	SCIP_VAR **vars;
	int varpos;
	int i;
	int nvars;

	scip = model->scip;
	exprs = (SCIP_EXPR **)malloc(nops * sizeof(SCIP_EXPR *));
	nvars = 0;
	for (i = 0; i < nops; ++i)
	{
		exprs[i] = NULL;
		nvars += (ops[i] == SCIP_EXPR_VARIDX);
	}
	vars = (SCIP_VAR **)malloc(nvars * sizeof(SCIP_VAR *));

	varpos = 0;
	for (i = 0; i < nops; ++i)
	{
		switch (ops[i])
		{
		case SCIP_EXPR_VARIDX:
		{
			int varidx = children[begin[i]];
			assert(1 == begin[i + 1] - begin[i]);
			assert(varidx < model->nvars);
			SCIP_in_CSIP(SCIPexprCreate(SCIPblkmem(scip), &exprs[i],
				ops[i], varpos));
			vars[varpos] = model->vars[varidx];
			++varpos;
			//printf("Seeing variable %d (nchild %d)\n", varidx, begin[i+1] - begin[i]);
		}
		break;
		case SCIP_EXPR_CONST:
			assert(1 == begin[i + 1] - begin[i]);
			SCIP_in_CSIP(SCIPexprCreate(SCIPblkmem(scip), &exprs[i],
				ops[i], values[children[begin[i]]]));
			//printf("Seeing constant %g (nchild %d)\n", values[children[begin[i]]], begin[i+1] - begin[i]);
			break;
		case SCIP_EXPR_MINUS:
			// if we have two children it is a proper minus; otherwise just -1 * ...
			if (begin[i + 1] - begin[i] == 2)
			{
				SCIP_in_CSIP(SCIPexprCreate(SCIPblkmem(scip), &exprs[i],
					ops[i], exprs[children[begin[i]]], exprs[children[begin[i] + 1]]));
			}
			else
			{
				SCIP_EXPR *zeroexpr;
				assert(1 == begin[i + 1] - begin[i]);
				SCIP_in_CSIP(SCIPexprCreate(SCIPblkmem(scip), &zeroexpr,
					SCIP_EXPR_CONST, 0.0));
				// expression is 0 - child
				SCIP_in_CSIP(SCIPexprCreate(SCIPblkmem(scip), &exprs[i],
					ops[i], zeroexpr, exprs[children[begin[i]]]));
			}
			//printf("Seeing a minus (nchild %d)\n",  begin[i+1] - begin[i]);
			break;
		case SCIP_EXPR_REALPOWER:
			assert(2 == begin[i + 1] - begin[i]);
			{
				double exponent;
				// the second child is the exponent which is a const
				exponent = values[children[begin[children[begin[i] + 1]]]];
				//printf("Seeing a power with exponent %g (nchild %d)\n", exponent, begin[i+1] - begin[i]);
				SCIP_in_CSIP(SCIPexprCreate(SCIPblkmem(scip), &exprs[i],
					ops[i], exprs[children[begin[i]]], exponent));
			}
			break;
		case SCIP_EXPR_DIV:
			assert(2 == begin[i + 1] - begin[i]);
			SCIP_in_CSIP(SCIPexprCreate(SCIPblkmem(scip), &exprs[i],
				ops[i], exprs[children[begin[i]]], exprs[children[begin[i] + 1]]));
			//printf("Seeing a division (nchild %d)\n",  begin[i+1] - begin[i]);
			break;
		case SCIP_EXPR_SQRT:
		case SCIP_EXPR_EXP:
		case SCIP_EXPR_LOG:
			assert(1 == begin[i + 1] - begin[i]);
			SCIP_in_CSIP(SCIPexprCreate(SCIPblkmem(scip), &exprs[i],
				ops[i], exprs[children[begin[i]]]));
			//printf("Seeing a sqrt/exp/log (nchild %d)\n",  begin[i+1] - begin[i]);
			break;
		case SCIP_EXPR_SUM:
		case SCIP_EXPR_PRODUCT:
		{
			SCIP_EXPR **childrenexpr;
			int nchildren = begin[i + 1] - begin[i];
			int c;
			childrenexpr = (SCIP_EXPR **)malloc(nchildren * sizeof(SCIP_EXPR *));
			for (c = 0; c < nchildren; ++c)
			{
				childrenexpr[c] = exprs[children[begin[i] + c]];
			}

			SCIP_in_CSIP(SCIPexprCreate(SCIPblkmem(scip), &exprs[i],
				ops[i], nchildren, childrenexpr));

			free(childrenexpr);
			//printf("Seeing a sum/product (nchild %d)\n",  begin[i+1] - begin[i]);
		}
		break;
		default: // don't support
			printf("I don't know what I am seeing %d\n", ops[i]);
			return CSIP_RETCODE_ERROR;
		}
	}
	assert(varpos == nvars);
	// last expression is root
	assert(exprs[nops - 1] != NULL);
	SCIP_in_CSIP(SCIPexprtreeCreate(SCIPblkmem(scip), tree, exprs[nops - 1], nvars,
		0, NULL));

	// assign variables to tree
	SCIP_in_CSIP(SCIPexprtreeSetVars(*tree, nvars, vars));

	// free memory
	free(vars);
	free(exprs);

	return CSIP_RETCODE_OK;
}

static
char *strDup(const char *s) {
	size_t size = strlen(s) + 1;
	char *p = malloc(size);
	if (p)
	{
		memcpy(p, s, size);
	}
	return p;
}

/** When the objective is nonlinear we use the epigraph representation.
* However, changing the objective sense is not  straightforward in that
* case. The purpose of this function is to change an epigraph objective
* to represent the correct objective sense. *Starting* from a correct
* objective, we only need two modifications to correctly change the sense.
* 0) change sense (this step is not done here)
* 1) multiply objective function by -1
* 2) multiply nonlinear function by -1
* max{ t : f(x) >= t } --> min{ -t : -f(x) >= t }
* min{ t : f(x) <= t } --> max{ -t : -f(x) <= t }
*/
CSIP_RETCODE correctObjectiveFunction(CSIP_MODEL *model)
{
	SCIP *scip = model->scip;
	SCIP_CONS *objcons = model->objcons;
	SCIP_VAR *objvar = model->objvar;

	// we only apply this for nonlinear objectives
	if (objvar == NULL)
	{
		return CSIP_RETCODE_OK;
	}

	assert(objcons != NULL);

	// 1)
	SCIP_in_CSIP(SCIPchgVarObj(scip, objvar, -1.0 * SCIPvarGetObj(objvar)));

	// 2)
	// we need to copy the tree, because SCIPsetExprtreesNonlinear
	// is going to delete it
	SCIP_EXPRTREE *exprtree;
	SCIP_in_CSIP(SCIPexprtreeCopy(SCIPblkmem(scip), &exprtree,
		SCIPgetExprtreesNonlinear(scip, objcons)[0]));
	SCIP_Real exprtreecoef = -SCIPgetExprtreeCoefsNonlinear(scip, objcons)[0];

	SCIP_in_CSIP(SCIPsetExprtreesNonlinear(scip, objcons, 1, &exprtree,
		&exprtreecoef));

	return CSIP_RETCODE_OK;
}

/*
* interface methods
*/
int CSIPmajorVersion()
{
	return CSIP_MAJOR_VERSION;
}

int CSIPminorVersion()
{
	return CSIP_MINOR_VERSION;
}

int CSIPpatchVersion()
{
	return CSIP_PATCH_VERSION;
}

int CSIPgetVersion()
{
	return (100 * CSIPmajorVersion()
		+ 10 * CSIPminorVersion()
		+ 1 * CSIPpatchVersion());
}

CSIP_RETCODE CSIPcreateModel(CSIP_MODEL **modelptr)
{
	CSIP_MODEL *model;

	*modelptr = (CSIP_MODEL *)malloc(sizeof(CSIP_MODEL));
	if (*modelptr == NULL)
	{
		return CSIP_RETCODE_NOMEMORY;
	}

	model = *modelptr;

	SCIP_in_CSIP(SCIPcreate(&model->scip));
	SCIP_in_CSIP(SCIPincludeDefaultPlugins(model->scip));
	SCIP_in_CSIP(SCIPcreateProbBasic(model->scip, "name"));

	model->nvars = 0;
	model->varssize = INITIALSIZE;
	model->vars = (SCIP_VAR **)malloc(INITIALSIZE * sizeof(SCIP_VAR *));
	if (model->vars == NULL)
	{
		return CSIP_RETCODE_NOMEMORY;
	}

	model->nconss = 0;
	model->consssize = INITIALSIZE;
	model->conss = (SCIP_CONS **)malloc(INITIALSIZE * sizeof(SCIP_CONS *));
	if (model->conss == NULL)
	{
		return CSIP_RETCODE_NOMEMORY;
	}

	model->nlazycb = 0;
	model->nheur = 0;
	model->initialsol = NULL;
	model->objvar = NULL;
	model->objcons = NULL;
	model->objtype = CSIP_OBJTYPE_LINEAR;
	model->msghdlr = NULL;

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPfreeModel(CSIP_MODEL *model)
{
	int i;

	if (model->initialsol != NULL) // solve was not called?
	{
		SCIP_in_CSIP(SCIPfreeSol(model->scip, &model->initialsol));
	}

	/* SCIPreleaseVar sets the given pointer to NULL. However, this pointer is
	* needed when SCIPfree is called, because it will call the lock method again
	* which works on the vars stored at model, so we give another pointer
	* TODO: maybe this is still wrong and one should free the transformed problem
	* first and then release the vars... we have to check for BMS memory leaks
	*/
	for (i = 0; i < model->nvars; ++i)
	{
		SCIP_VAR *var;
		var = model->vars[i];
		SCIP_in_CSIP(SCIPreleaseVar(model->scip, &var));
	}
	for (i = 0; i < model->nconss; ++i)
	{
		SCIP_in_CSIP(SCIPreleaseCons(model->scip, &model->conss[i]));
	}
	if (model->objvar != NULL)
	{
		assert(model->objcons != NULL);
		SCIP_in_CSIP(SCIPreleaseVar(model->scip, &model->objvar));
		SCIP_in_CSIP(SCIPreleaseCons(model->scip, &model->objcons));
	}
	SCIP_in_CSIP(SCIPfree(&model->scip));

	free(model->conss);
	free(model->vars);
	free(model);

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPaddVar(CSIP_MODEL *model, double lowerbound, double upperbound,
	int vartype, int *idx)
{
	SCIP *scip;
	SCIP_VAR *var;

	scip = model->scip;
	SCIP_in_CSIP(SCIPfreeTransform(scip));

	SCIP_in_CSIP(SCIPcreateVarBasic(scip, &var, NULL, lowerbound, upperbound, 0.0,
		vartype));
	SCIP_in_CSIP(SCIPaddVar(scip, var));

	// do we need to resize?
	if (model->nvars >= model->varssize)
	{
		model->varssize = GROWFACTOR * model->varssize;
		model->vars = (SCIP_VAR **)realloc(
			model->vars, model->varssize * sizeof(SCIP_VAR *));
		if (model->vars == NULL)
		{
			return CSIP_RETCODE_NOMEMORY;
		}
	}

	if (idx != NULL)
	{
		*idx = model->nvars;
		model->vars[*idx] = var;
	}
	else
	{
		model->vars[model->nvars] = var;
	}
	++(model->nvars);

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPchgVarLB(CSIP_MODEL *model, int numindices, int *indices,
	double *lowerbounds)
{
	int i;
	SCIP *scip;
	SCIP_VAR *var;

	scip = model->scip;
	SCIP_in_CSIP(SCIPfreeTransform(scip));

	for (i = 0; i < numindices; ++i)
	{
		var = model->vars[indices[i]];
		SCIP_in_CSIP(SCIPchgVarLb(scip, var, lowerbounds[i]));
	}

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPchgVarUB(CSIP_MODEL *model, int numindices, int *indices,
	double *upperbounds)
{
	int i;
	SCIP *scip;
	SCIP_VAR *var;

	scip = model->scip;
	SCIP_in_CSIP(SCIPfreeTransform(scip));

	for (i = 0; i < numindices; ++i)
	{
		var = model->vars[indices[i]];
		SCIP_in_CSIP(SCIPchgVarUb(scip, var, upperbounds[i]));
	}

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPchgVarType(
	CSIP_MODEL *model, int varindex, CSIP_VARTYPE vartype)
{
	SCIP *scip = model->scip;
	SCIP_VAR *var = model->vars[varindex];
	SCIP_Bool infeas = FALSE;

	SCIP_in_CSIP(SCIPfreeTransform(scip));

	SCIP_in_CSIP(SCIPchgVarType(scip, var, vartype, &infeas));
	// TODO: don't ignore `infeas`?
	// for SCIP, solving a problem with a binary variable with bounds not in [0,1] produces an error
	// here we change them to the correct value, since JuMP seems to expect that behaviour
	// see JuMP tests: [probmod] Test bound modification on binaries
	if (vartype == CSIP_VARTYPE_BINARY && SCIPvarGetLbLocal(var) < 0.0)
	{
		SCIP_in_CSIP(SCIPchgVarLb(scip, var, 0.0));
	}
	if (vartype == CSIP_VARTYPE_BINARY && SCIPvarGetUbLocal(var) > 1.0)
	{
		SCIP_in_CSIP(SCIPchgVarUb(scip, var, 1.0));
	}

	return CSIP_RETCODE_OK;
}

CSIP_VARTYPE CSIPgetVarType(CSIP_MODEL *model, int varindex)
{
	assert(varindex >= 0 && varindex < model->nvars);

	switch (SCIPvarGetType(model->vars[varindex]))
	{
	case SCIP_VARTYPE_BINARY:
		return CSIP_VARTYPE_BINARY;
	case SCIP_VARTYPE_INTEGER:
		return CSIP_VARTYPE_INTEGER;
	case SCIP_VARTYPE_IMPLINT:
		return CSIP_VARTYPE_IMPLINT;
	case SCIP_VARTYPE_CONTINUOUS:
		return CSIP_VARTYPE_CONTINUOUS;
	}
	return -1;
}

CSIP_RETCODE CSIPaddLinCons(CSIP_MODEL *model, int numindices, int *indices,
	double *coefs, double lhs, double rhs, int *idx)
{
	SCIP_CONS *cons;

	SCIP_in_CSIP(SCIPfreeTransform(model->scip));

	CSIP_CALL(createLinCons(model, numindices, indices, coefs, lhs, rhs, &cons));
	CSIP_CALL(addCons(model, cons, idx));

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPaddQuadCons(CSIP_MODEL *model, int numlinindices,
	int *linindices,
	double *lincoefs, int numquadterms,
	int *quadrowindices, int *quadcolindices,
	double *quadcoefs, double lhs, double rhs, int *idx)
{
	int i;
	SCIP *scip;
	SCIP_VAR *linvar;
	SCIP_VAR *var1;
	SCIP_VAR *var2;
	SCIP_CONS *cons;

	scip = model->scip;
	SCIP_in_CSIP(SCIPfreeTransform(scip));

	SCIP_in_CSIP(SCIPcreateConsBasicQuadratic(scip, &cons, "quadcons", 0, NULL,
		NULL, 0, NULL, NULL, NULL, lhs, rhs));

	for (i = 0; i < numlinindices; ++i)
	{
		linvar = model->vars[linindices[i]];
		SCIP_in_CSIP(SCIPaddLinearVarQuadratic(scip, cons, linvar, lincoefs[i]));
	}

	for (i = 0; i < numquadterms; ++i)
	{
		var1 = model->vars[quadrowindices[i]];
		var2 = model->vars[quadcolindices[i]];
		SCIP_in_CSIP(SCIPaddBilinTermQuadratic(scip, cons, var1, var2, quadcoefs[i]));
	}

	CSIP_CALL(addCons(model, cons, idx));

	return CSIP_RETCODE_OK;
}

// we might be assuming that the indices of the children of op[k]
// are always <= k (when op[k] is not VARIDX nor CONST)
// this implies that the root expression is the last one, which is
// another assumption
CSIP_RETCODE CSIPaddNonLinCons(
	CSIP_MODEL *model, int nops, CSIP_OP *ops, int *children, int *begin,
	double *values, double lhs, double rhs, int *idx)
{
	SCIP *scip;
	SCIP_EXPRTREE *tree;
	SCIP_CONS *cons;

	CSIP_CALL(createExprtree(model, nops, ops, children, begin,
		values, &tree));

	scip = model->scip;
	SCIP_in_CSIP(SCIPfreeTransform(scip));

	// create nonlinear constraint
	SCIP_in_CSIP(SCIPcreateConsBasicNonlinear(scip, &cons, "nonlin", 0, NULL, NULL,
		1, &tree, NULL, lhs, rhs));

	CSIP_CALL(addCons(model, cons, idx));

	// free memory
	SCIP_in_CSIP(SCIPexprtreeFree(&tree));

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPaddSOS1(
	CSIP_MODEL *model, int numindices, int *indices, double *weights, int *idx)
{
	SCIP *scip = model->scip;
	SCIP_CONS *cons;
	SCIP_VAR **vars = (SCIP_VAR **)malloc(numindices * sizeof(SCIP_VAR *));
	double* auxweights = weights;

	SCIP_in_CSIP(SCIPfreeTransform(scip));
	if (vars == NULL)
	{
		return CSIP_RETCODE_NOMEMORY;
	}
	for (int i = 0; i < numindices; ++i)
	{
		vars[i] = model->vars[indices[i]];
	}

	/* give weights to avoid an assert in SCIP */
	if (weights == NULL)
	{
		auxweights = (double *)malloc(numindices * sizeof(double));
		if (auxweights == NULL)
		{
			return CSIP_RETCODE_NOMEMORY;
		}
		for (int i = 0; i < numindices; ++i)
		{
			auxweights[i] = i;
		}
	}

	SCIP_in_CSIP(SCIPcreateConsBasicSOS1(
		scip, &cons, "sos1", numindices, vars, auxweights));
	CSIP_CALL(addCons(model, cons, idx));

	if (weights == NULL)
	{
		free(auxweights);
	}
	free(vars);

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPaddSOS2(
	CSIP_MODEL *model, int numindices, int *indices, double *weights, int *idx)
{
	SCIP *scip = model->scip;
	SCIP_CONS *cons;
	SCIP_VAR **vars = (SCIP_VAR **)malloc(numindices * sizeof(SCIP_VAR *));
	double* auxweights = weights;

	SCIP_in_CSIP(SCIPfreeTransform(scip));

	if (vars == NULL)
	{
		return CSIP_RETCODE_NOMEMORY;
	}
	for (int i = 0; i < numindices; ++i)
	{
		vars[i] = model->vars[indices[i]];
	}

	/* give weights to avoid an assert in SCIP */
	if (weights == NULL)
	{
		auxweights = (double *)malloc(numindices * sizeof(double));
		if (auxweights == NULL)
		{
			return CSIP_RETCODE_NOMEMORY;
		}
		for (int i = 0; i < numindices; ++i)
		{
			auxweights[i] = i;
		}
	}

	SCIP_in_CSIP(SCIPcreateConsBasicSOS2(
		scip, &cons, "sos2", numindices, vars, auxweights));
	CSIP_CALL(addCons(model, cons, idx));

	if (weights == NULL)
	{
		free(auxweights);
	}
	free(vars);

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetObj(CSIP_MODEL *model, int numindices, int *indices,
	double *coefs)
{
	int i;
	SCIP *scip;
	SCIP_VAR *var;

	scip = model->scip;
	SCIP_in_CSIP(SCIPfreeTransform(scip));

	for (i = 0; i < numindices; ++i)
	{
		var = model->vars[indices[i]];
		SCIP_in_CSIP(SCIPchgVarObj(scip, var, coefs[i]));
	}
	model->objtype = CSIP_OBJTYPE_LINEAR;

	// if nonlinear objective was set, remove objvar from objective
	// and relax its bounds. This should render the objective constraint
	// redundant
	if (model->objvar != NULL)
	{
		SCIP_in_CSIP(SCIPchgVarObj(scip, model->objvar, 0.0));
		SCIP_in_CSIP(SCIPchgVarLb(scip, model->objvar, -SCIPinfinity(scip)));
		SCIP_in_CSIP(SCIPchgVarUb(scip, model->objvar, SCIPinfinity(scip)));

		// we do not need to remember this variable anymore nor the objcons
		SCIP_in_CSIP(SCIPreleaseVar(scip, &model->objvar));
		SCIP_in_CSIP(SCIPreleaseCons(scip, &model->objcons));
		assert(model->objvar == NULL);
		assert(model->objcons == NULL);
	}

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetQuadObj(CSIP_MODEL *model, int numlinindices,
	int *linindices, double *lincoefs, int numquadterms,
	int *quadrowindices, int *quadcolindices,
	double *quadcoefs)
{
	int nprods;
	int i;
	int opidx;
	int nops;
	int nchildren;
	CSIP_OP *ops;
	int *children;
	int *begin;
	int *prodindices;
	double *values;

	// build the expression representation of a quadratic
	// there are: numlinindices VARIDX, CONST and PROD for the linear part
	// nquadterms VARIDX, VARIDX, CONST and PROD for the quadratic part
	// So there are 3*numlindices + 4*nquadterms operators plus 1 (the sum)
	// Number of children: each VARIDX and CONST contribute with one:
	// nchildren >= 2*numlinindices + 3*nquadterms
	// Then each PROD in numlinindices contribute with 2 and in nquadterms with 3
	// nchildren >= 4*numlinindices + 6*nquadterms
	// The final sum has as many children as products are there, nlinind + nquadterms
	// nchildren = 5*numlinindices + 7*nquadterms
	// NOTE: we will need to store the indices of the PROD operators
	//       and all coefs in a single array `values`
	nprods = numquadterms + numlinindices;
	nchildren = 5 * numlinindices + 7 * numquadterms;
	nops = 3 * numlinindices + 4 * numquadterms + 1;

	ops = (int *)malloc(nops * sizeof(CSIP_OP));
	children = (int *)malloc(nchildren * sizeof(int));
	begin = (int *)malloc((nops + 1) * sizeof(int));
	values = (double *)malloc(nprods * sizeof(double));
	prodindices = (int *)malloc(nprods * sizeof(int));

	begin[0] = 0;
	opidx = -1;
	// linear part
	for (i = 0; i < numlinindices; ++i)
	{
		// variable
		++opidx;
		ops[opidx] = VARIDX;
		begin[opidx + 1] = begin[opidx] + 1; // where its children end
		children[begin[opidx]] = linindices[i]; //children

												// next operator: coef
		++opidx;
		ops[opidx] = CONST;
		begin[opidx + 1] = begin[opidx] + 1;
		children[begin[opidx]] = i;
		values[i] = lincoefs[i];

		// next operator: PROD between coef and variable
		++opidx;
		ops[opidx] = PROD;
		begin[opidx + 1] = begin[opidx] + 2;
		children[begin[opidx]] = opidx - 2;
		children[begin[opidx] + 1] = opidx - 1;

		prodindices[i] = opidx;
	}
	// quadratic part
	for (i = 0; i < numquadterms; ++i)
	{
		// variable 1
		++opidx;
		ops[opidx] = VARIDX;
		begin[opidx + 1] = begin[opidx] + 1; // where its children end
		children[begin[opidx]] = quadrowindices[i]; //children

													// variable 2
		++opidx;
		ops[opidx] = VARIDX;
		begin[opidx + 1] = begin[opidx] + 1; // where its children end
		children[begin[opidx]] = quadcolindices[i]; //children

													// coef
		++opidx;
		ops[opidx] = CONST;
		begin[opidx + 1] = begin[opidx] + 1;
		children[begin[opidx]] = i + numlinindices;
		values[i + numlinindices] = quadcoefs[i];

		// next operator: PROD between var1, var2 and coef
		++opidx;
		ops[opidx] = PROD;
		begin[opidx + 1] = begin[opidx] + 3;
		children[begin[opidx]] = opidx - 3;
		children[begin[opidx] + 1] = opidx - 2;
		children[begin[opidx] + 2] = opidx - 1;

		prodindices[i + numlinindices] = opidx;
	}

	// sum all PRODs
	++opidx;
	ops[opidx] = SUM;
	begin[opidx + 1] = begin[opidx] + nprods;
	for (i = 0; i < nprods; ++i)
	{
		children[begin[opidx] + i] = prodindices[i];
	}

	CSIP_CALL(CSIPsetNonlinearObj(model, nops, ops, children, begin, values));

	// free everything
	free(ops);
	free(children);
	free(begin);
	free(values);
	free(prodindices);

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetNonlinearObj(
	CSIP_MODEL *model, int nops, CSIP_OP *ops, int *children, int *begin,
	double *values)
{
	SCIP *scip;
	SCIP_EXPRTREE *tree;
	SCIP_CONS *cons;

	// get scip, free transform and remove old objective if any
	scip = model->scip;
	SCIP_in_CSIP(SCIPfreeTransform(scip));

	if (model->objvar != NULL)
	{
		SCIP_in_CSIP(SCIPchgVarObj(scip, model->objvar, 0.0));
		SCIP_in_CSIP(SCIPchgVarLb(scip, model->objvar, -SCIPinfinity(scip)));
		SCIP_in_CSIP(SCIPchgVarUb(scip, model->objvar, SCIPinfinity(scip)));

		// we do not need to remember this variable anymore nor objcons
		SCIP_in_CSIP(SCIPreleaseVar(scip, &model->objvar));
		SCIP_in_CSIP(SCIPreleaseCons(scip, &model->objcons));
	}
	assert(model->objvar == NULL);
	assert(model->objcons == NULL);

	// do nothing more if we received an empty expression tree
	assert(nops >= 1);
	if (nops == 1 && ops[0] != SCIP_EXPR_VARIDX)
	{
		return CSIP_RETCODE_OK;
	}

	CSIP_CALL(createExprtree(model, nops, ops, children, begin,
		values, &tree));

	// create nonlinear objective constraint
	SCIP_in_CSIP(SCIPcreateConsBasicNonlinear(scip, &cons,
		"nonlin_obj", 0, NULL, NULL, 1, &tree, NULL,
		-SCIPinfinity(scip), 0.0));

	// add objvar to nonlinear objective
	SCIP_in_CSIP(SCIPcreateVarBasic(scip, &model->objvar, NULL,
		-SCIPinfinity(scip), SCIPinfinity(scip), 1.0,
		SCIP_VARTYPE_CONTINUOUS));
	SCIP_in_CSIP(SCIPaddVar(scip, model->objvar));
	SCIP_in_CSIP(SCIPaddLinearVarNonlinear(scip, cons, model->objvar, -1.0));

	// add objective constraint and remember it
	SCIP_in_CSIP(SCIPaddCons(scip, cons));
	model->objcons = cons;
	model->objtype = CSIP_OBJTYPE_NONLINEAR;

	// the created constraint is correct if sense is minimize, otherwise we
	// have to correct it
	if (SCIPgetObjsense(model->scip) == SCIP_OBJSENSE_MAXIMIZE)
	{
		CSIP_CALL(correctObjectiveFunction(model));
	}

	// free memory
	SCIP_in_CSIP(SCIPexprtreeFree(&tree));

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetSenseMinimize(CSIP_MODEL *model)
{
	SCIP_in_CSIP(SCIPfreeTransform(model->scip));

	if (SCIPgetObjsense(model->scip) != SCIP_OBJSENSE_MINIMIZE)
	{
		SCIP_in_CSIP(SCIPsetObjsense(model->scip, SCIP_OBJSENSE_MINIMIZE));
		CSIP_CALL(correctObjectiveFunction(model));
	}

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetSenseMaximize(CSIP_MODEL *model)
{
	SCIP_in_CSIP(SCIPfreeTransform(model->scip));

	if (SCIPgetObjsense(model->scip) != SCIP_OBJSENSE_MAXIMIZE)
	{
		SCIP_in_CSIP(SCIPsetObjsense(model->scip, SCIP_OBJSENSE_MAXIMIZE));
		CSIP_CALL(correctObjectiveFunction(model));
	}

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsolve(CSIP_MODEL *model)
{
	// add initial solution
	if (model->initialsol != NULL)
	{
		unsigned int stored;
		SCIP_Bool initialsolpartial =
			(SCIPsolGetOrigin(model->initialsol) == SCIP_SOLORIGIN_PARTIAL);

		/* if objective is nonlinear, we need to extend the initial sol with
		* the value of objvar. For this we need to change the objective
		* constraint temporarily to an equality constraint. However the
		* nonlinear contraint handler doesn't allow to change sides, so
		* we have to create a brand new constraint to compute the violation
		*
		* This is not true if the user has given a partial sol, because then
		* we can safely leave the value for the objval unspecified. In fact,
		* that's preferred, because computing the violation might fail.
		*/
		if (model->objcons != NULL && !initialsolpartial)
		{
			SCIP_CONS* tempcons;
			SCIP_Real objvarval;
			SCIP_EXPRTREE *tree;

			/* we do not care about the sign of the expression tree, since
			* tempcons is equality and this already gives the correct sign for
			* objvarval */
			tree = SCIPgetExprtreesNonlinear(model->scip, model->objcons)[0];
			SCIP_in_CSIP(SCIPcreateConsBasicNonlinear(model->scip, &tempcons,
				"temp_nonlin_obj", 0, NULL, NULL, 1, &tree, NULL,
				0.0, 0.0));
			SCIP_in_CSIP(SCIPgetViolationNonlinear(model->scip, tempcons,
				model->initialsol, &objvarval));

			SCIP_in_CSIP(SCIPreleaseCons(model->scip, &tempcons));

			SCIP_in_CSIP(SCIPsetSolVals(model->scip, model->initialsol, 1,
				&model->objvar, &objvarval));
		}


		SCIP_in_CSIP(SCIPaddSolFree(model->scip, &model->initialsol, &stored));
	}

	SCIP_in_CSIP(SCIPsolve(model->scip));

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPinterrupt(CSIP_MODEL *model)
{
	SCIP_in_CSIP(SCIPinterruptSolve(model->scip));
	return CSIP_RETCODE_OK;
}

CSIP_STATUS CSIPgetStatus(CSIP_MODEL *model)
{
	switch (SCIPgetStatus(model->scip))
	{
	case SCIP_STATUS_UNKNOWN:
		return CSIP_STATUS_UNKNOWN;
	case SCIP_STATUS_USERINTERRUPT:
		return CSIP_STATUS_USERLIMIT;
	case SCIP_STATUS_NODELIMIT:
		return CSIP_STATUS_NODELIMIT;
	case SCIP_STATUS_TOTALNODELIMIT:
		return CSIP_STATUS_NODELIMIT;
	case SCIP_STATUS_STALLNODELIMIT:
		return CSIP_STATUS_USERLIMIT;
	case SCIP_STATUS_TIMELIMIT:
		return CSIP_STATUS_TIMELIMIT;
	case SCIP_STATUS_MEMLIMIT:
		return CSIP_STATUS_MEMLIMIT;
	case SCIP_STATUS_GAPLIMIT:
		return CSIP_STATUS_USERLIMIT;
	case SCIP_STATUS_SOLLIMIT:
		return CSIP_STATUS_USERLIMIT;
	case SCIP_STATUS_BESTSOLLIMIT:
		return CSIP_STATUS_USERLIMIT;
	case SCIP_STATUS_RESTARTLIMIT:
		return CSIP_STATUS_USERLIMIT;
	case SCIP_STATUS_OPTIMAL:
		return CSIP_STATUS_OPTIMAL;
	case SCIP_STATUS_INFEASIBLE:
		return CSIP_STATUS_INFEASIBLE;
	case SCIP_STATUS_UNBOUNDED:
		return CSIP_STATUS_UNBOUNDED;
	case SCIP_STATUS_INFORUNBD:
		return CSIP_STATUS_INFORUNBD;
	default:
		return CSIP_STATUS_UNKNOWN;
	}
}

double CSIPgetObjValue(CSIP_MODEL *model)
{
	SCIP_SOL *sol = SCIPgetBestSol(model->scip);
	if (sol == NULL)
	{
		return CSIP_RETCODE_ERROR;
	}

	return SCIPgetSolOrigObj(model->scip, sol);
}

double CSIPgetObjBound(CSIP_MODEL *model)
{
	return SCIPgetDualbound(model->scip);
}


CSIP_RETCODE CSIPgetVarValues(CSIP_MODEL *model, double *output)
{
	int i;
	SCIP *scip;
	SCIP_VAR *var;

	scip = model->scip;

	if (SCIPgetBestSol(scip) == NULL)
	{
		return CSIP_RETCODE_ERROR;
	}

	for (i = 0; i < model->nvars; ++i)
	{
		var = model->vars[i];
		output[i] = SCIPgetSolVal(scip, SCIPgetBestSol(scip), var);
	}

	return CSIP_RETCODE_OK;
}

// Get the type of a parameter
CSIP_PARAMTYPE CSIPgetParamType(CSIP_MODEL *model, const char *name)
{
	SCIP_PARAM *param;

	param = SCIPgetParam(model->scip, name);
	if (param == NULL)
	{
		return CSIP_PARAMTYPE_NOTAPARAM;
	}
	else
	{
		return SCIPparamGetType(param);
	}
}

CSIP_RETCODE CSIPsetBoolParam(
	CSIP_MODEL *model, const char *name, int value)
{
	SCIP_in_CSIP(SCIPsetBoolParam(model->scip, name, value));
	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetIntParam(
	CSIP_MODEL *model, const char *name, int value)
{
	SCIP_in_CSIP(SCIPsetIntParam(model->scip, name, value));
	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetLongintParam(
	CSIP_MODEL *model, const char *name, long long value)
{
	SCIP_in_CSIP(SCIPsetLongintParam(model->scip, name, value));
	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetRealParam(
	CSIP_MODEL *model, const char *name, double value)
{
	SCIP_in_CSIP(SCIPsetRealParam(model->scip, name, value));
	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetCharParam(
	CSIP_MODEL *model, const char *name, char value)
{
	SCIP_in_CSIP(SCIPsetCharParam(model->scip, name, value));
	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPsetStringParam(
	CSIP_MODEL *model, const char *name, const char *value)
{
	SCIP_in_CSIP(SCIPsetStringParam(model->scip, name, value));
	return CSIP_RETCODE_OK;
}

int CSIPgetNumVars(CSIP_MODEL *model)
{
	return model->nvars;
}

int CSIPgetNumConss(CSIP_MODEL *model)
{
	return model->nconss;
}

CSIP_RETCODE CSIPsetInitialSolution(CSIP_MODEL *model, double *values)
{
	// are there missing values?
	SCIP_Bool initialsolpartial = FALSE;
	for (int i = 0; i < model->nvars; ++i)
	{
		SCIP_Real val = values[i];
		if (val != val) // check for NaN
		{
			initialsolpartial = TRUE;
			break;
		}
	}

	// was solution already given?
	if (model->initialsol != NULL)
	{
		SCIP_in_CSIP(SCIPfreeSol(model->scip, &model->initialsol));
	}
	assert(model->initialsol == NULL);

	// create new solution object
	if (initialsolpartial)
	{
		SCIP_in_CSIP(SCIPcreatePartialSol(model->scip, &model->initialsol, NULL));

		// give only the proper values, skip NaN
		for (int i = 0; i < model->nvars; ++i)
		{
			SCIP_Real val = values[i];
			if (val == val) // check for NaN
			{
				SCIP_in_CSIP(SCIPsetSolVal(model->scip, model->initialsol,
					model->vars[i], val));
			}
		}
	}
	else
	{
		SCIP_in_CSIP(SCIPcreateSol(model->scip, &model->initialsol, NULL));

		// copy the given values
		SCIP_in_CSIP(SCIPsetSolVals(model->scip, model->initialsol, model->nvars,
			model->vars, values));
	}

	// it will be given to SCIP in the CSIPsolve call

	return CSIP_RETCODE_OK;
}

void *CSIPgetInternalSCIP(CSIP_MODEL *model)
{
	return model->scip;
}


/*
* Constraint Handler
*/

/* constraint handler data */
struct SCIP_ConshdlrData
{
	CSIP_MODEL *model;
	CSIP_LAZYCALLBACK callback;
	void *userdata;
	SCIP_Bool checkonly;
	SCIP_Bool feasible;
	SCIP_SOL *sol;
};

SCIP_DECL_CONSFREE(consFreeLazy)
{
	SCIP_CONSHDLRDATA *conshdlrdata;

	conshdlrdata = SCIPconshdlrGetData(conshdlr);
	assert(conshdlrdata != NULL);

	SCIPfreeMemory(scip, &conshdlrdata);

	SCIPconshdlrSetData(conshdlr, NULL);

	return SCIP_OKAY;
}

SCIP_DECL_CONSENFOLP(consEnfolpLazy)
{
	SCIP_CONSHDLRDATA *conshdlrdata;

	*result = SCIP_FEASIBLE;

	conshdlrdata = SCIPconshdlrGetData(conshdlr);
	conshdlrdata->checkonly = FALSE;
	conshdlrdata->feasible = TRUE;

	CSIP_in_SCIP(conshdlrdata->callback(conshdlrdata->model,
		conshdlrdata, conshdlrdata->userdata));

	if (!conshdlrdata->feasible)
	{
		*result = SCIP_CONSADDED;
	}

	return SCIP_OKAY;
}

/* enfo pseudo solution just call enfo lp solution */
SCIP_DECL_CONSENFOPS(consEnfopsLazy)
{
	return consEnfolpLazy(scip, conshdlr, conss, nconss, nusefulconss,
		solinfeasible, result);
}

/* check callback */
SCIP_DECL_CONSCHECK(consCheckLazy)
{
	SCIP_CONSHDLRDATA *conshdlrdata;

	*result = SCIP_FEASIBLE;

	conshdlrdata = SCIPconshdlrGetData(conshdlr);
	conshdlrdata->checkonly = TRUE;
	conshdlrdata->feasible = TRUE;
	conshdlrdata->sol = sol;

	CSIP_in_SCIP(conshdlrdata->callback(conshdlrdata->model,
		conshdlrdata, conshdlrdata->userdata));

	if (!conshdlrdata->feasible)
	{
		*result = SCIP_INFEASIBLE;
	}

	return SCIP_OKAY;
}

/* locks callback */
SCIP_DECL_CONSLOCK(consLockLazy)
{
	int i;
	SCIP_VAR *var;
	SCIP_CONSHDLRDATA *conshdlrdata;

	conshdlrdata = SCIPconshdlrGetData(conshdlr);

	assert(scip == conshdlrdata->model->scip);

	for (i = 0; i < conshdlrdata->model->nvars; ++i)
	{
		var = conshdlrdata->model->vars[i];
		SCIP_CALL(SCIPaddVarLocks(scip, var, nlockspos + nlocksneg,
			nlockspos + nlocksneg));
	}

	return SCIP_OKAY;
}

/*
* callback methods
*/

CSIP_RETCODE CSIPaddLazyCallback(CSIP_MODEL *model, CSIP_LAZYCALLBACK callback,
	void *userdata)
{
	SCIP_CONSHDLRDATA *conshdlrdata;
	SCIP_CONSHDLR *conshdlr;
	SCIP *scip;
	char name[SCIP_MAXSTRLEN];
	int enfopriority;
	int checkpriority;
	int eagerfreq;
	SCIP_Bool needscons = FALSE;

	scip = model->scip;

	/* cons_integral has enfo priority 0 and we want to be checked before */
	enfopriority = 1;
	/* we want to be checked as rarely as possible.
	* -5000000 is the smallest proper value in SCIP's conshdlrs. */
	checkpriority = -5000000 - 1;
	/* no eager evaluations?! */
	eagerfreq = -1;

	SCIP_in_CSIP(SCIPallocMemory(scip, &conshdlrdata));

	conshdlrdata->model = model;
	conshdlrdata->callback = callback;
	conshdlrdata->userdata = userdata;

	SCIPsnprintf(name, SCIP_MAXSTRLEN, "lazycons_%d", model->nlazycb);
	SCIP_in_CSIP(SCIPincludeConshdlrBasic(
		scip, &conshdlr, name, "lazy constraint callback",
		enfopriority, checkpriority, eagerfreq, needscons,
		consEnfolpLazy, consEnfopsLazy, consCheckLazy, consLockLazy,
		conshdlrdata));

	SCIP_in_CSIP(SCIPsetConshdlrFree(scip, conshdlr, consFreeLazy));
	model->nlazycb += 1;

	return CSIP_RETCODE_OK;
}

static
CSIP_RETCODE isSolIntegral(SCIP *scip, SCIP_SOL *sol, SCIP_Bool *integral)
{
	// inspired by consCheckIntegral in cons_integral.c
	SCIP_VAR **vars;
	SCIP_Real solval;
	int nbin, nint, nimpl, nallinteger;

	SCIP_in_CSIP(SCIPgetSolVarsData(scip, sol, &vars, NULL,
		&nbin, &nint, &nimpl, NULL));

	*integral = TRUE;
	nallinteger = nbin + nint + nimpl;
	for (int v = 0; v < nallinteger; ++v)
	{
		solval = SCIPgetSolVal(scip, sol, vars[v]);
		if (!SCIPisFeasIntegral(scip, solval))
		{
			*integral = FALSE;
			break;
		}
	}

	return CSIP_RETCODE_OK;
}


CSIP_LAZY_CONTEXT CSIPlazyGetContext(CSIP_LAZYDATA *lazydata)
{
	SCIP_Bool check = lazydata->checkonly;
	SCIP_SOL *sol = check ? lazydata->sol : NULL;
	SCIP_Bool integral = FALSE;
	CSIP_CALL(isSolIntegral(lazydata->model->scip, sol, &integral));

	if (integral)
	{
		return CSIP_LAZY_INTEGRALSOL;
	}
	else if (!check)
	{
		return CSIP_LAZY_LPRELAX;
	}
	else
	{
		return CSIP_LAZY_OTHER;
	}
}

/* returns LP or given solution depending whether we are called from check or enfo */
CSIP_RETCODE CSIPlazyGetVarValues(CSIP_LAZYDATA *lazydata, double *output)
{
	int i;
	SCIP *scip;
	SCIP_VAR *var;
	SCIP_SOL *sol;

	scip = lazydata->model->scip;
	sol = lazydata->checkonly ? lazydata->sol : NULL;

	for (i = 0; i < lazydata->model->nvars; ++i)
	{
		var = lazydata->model->vars[i];
		output[i] = SCIPgetSolVal(scip, sol, var);
	}

	return CSIP_RETCODE_OK;
}

CSIP_RETCODE CSIPlazyAddLinCons(CSIP_LAZYDATA *lazydata, int numindices,
	int *indices,
	double *coefs, double lhs, double rhs, int islocal)
{
	SCIP *scip;
	SCIP_CONS *cons;
	SCIP_SOL *sol;
	SCIP_RESULT result;

	scip = lazydata->model->scip;

	if (lazydata->checkonly)
	{
		sol = lazydata->sol;
	}
	else
	{
		sol = NULL;
	}

	/* Is it reasonable to assume that if we solved the problem, then the lazy constraint
	* is satisfied in the original problem? We get the error:
	* "method <SCIPcreateCons> cannot be called in the solved stage"
	* and I am guessing this is because SCIP is checking whether the solution found in the
	* presolved problem is feasible for the original problem. It could happen it is not feasible
	* because of numerics mainly, hence the question in the comment
	*/
	if (SCIPgetStage(scip) == SCIP_STAGE_SOLVED)
	{
		assert(lazydata->checkonly);
		lazydata->feasible = TRUE; /* to be very explicit */
		return CSIP_RETCODE_OK;
	}

	if (SCIPgetStage(scip) == SCIP_STAGE_TRANSFORMED)
	{
		// we can't even create a cons in this stage, we trust the user's judgement
		lazydata->feasible = FALSE;
		return CSIP_RETCODE_OK;
	}

	CSIP_CALL(createLinCons(lazydata->model, numindices, indices, coefs, lhs, rhs,
		&cons));
	SCIP_in_CSIP(SCIPsetConsLocal(scip, cons, islocal == 1));
	SCIP_in_CSIP(SCIPcheckCons(scip, cons, sol, FALSE, FALSE, FALSE, &result));

	if (result == SCIP_INFEASIBLE)
	{
		lazydata->feasible = FALSE;
	}

	/* can not add constraints here */
	if (SCIPgetStage(scip) == SCIP_STAGE_INIT
		|| SCIPgetStage(scip) == SCIP_STAGE_TRANSFORMING
		|| SCIPgetStage(scip) == SCIP_STAGE_INITSOLVE)
	{
		assert(lazydata->checkonly);
		SCIP_in_CSIP(SCIPreleaseCons(lazydata->model->scip, &cons));
		return CSIP_RETCODE_OK;
	}

	/* we do not store cons, because the original problem does not contain them;
	* and there is an issue when freeTransform is called
	*/
	SCIP_in_CSIP(SCIPaddCons(scip, cons));
	SCIP_in_CSIP(SCIPreleaseCons(lazydata->model->scip, &cons));

	return CSIP_RETCODE_OK;
}

/* Heuristic Plugin */

struct SCIP_HeurData
{
	CSIP_MODEL *model;
	CSIP_HEURCALLBACK callback;
	void *userdata;
	SCIP_HEUR *heur;
	unsigned int stored_sols;
};

static
SCIP_DECL_HEURFREE(heurFreeUser)
{
	SCIP_HEURDATA *heurdata;

	heurdata = SCIPheurGetData(heur);
	assert(heurdata != NULL);

	SCIPfreeMemory(scip, &heurdata);
	SCIPheurSetData(heur, NULL);

	return SCIP_OKAY;
}

static
SCIP_DECL_HEUREXEC(heurExecUser)
{
	SCIP_HEURDATA *heurdata = SCIPheurGetData(heur);
	assert(heurdata != NULL);

	*result = SCIP_DIDNOTFIND;
	heurdata->stored_sols = 0;

	CSIP_in_SCIP(heurdata->callback(heurdata->model, heurdata,
		heurdata->userdata));

	if (heurdata->stored_sols > 0)
	{
		*result = SCIP_FOUNDSOL;
	}

	return SCIP_OKAY;
}

// Copy values of solution to output array. Call this function from your
// heuristic callback. Solution is LP relaxation of current node.
CSIP_RETCODE CSIPheurGetVarValues(CSIP_HEURDATA *heurdata, double *output)
{
	CSIP_MODEL *model = heurdata->model;
	SCIP_in_CSIP(SCIPgetSolVals(model->scip, NULL, model->nvars, model->vars,
		output));
	return CSIP_RETCODE_OK;
}

// Supply a solution (as a dense array). Only complete solutions are supported.
CSIP_RETCODE CSIPheurAddSolution(CSIP_HEURDATA *heurdata, double *values)
{
	SCIP_SOL *sol;
	CSIP_MODEL *model = heurdata->model;
	SCIP *scip = model->scip;
	unsigned int stored = 0;

	SCIP_in_CSIP(SCIPcreateSol(scip, &sol, heurdata->heur));
	SCIP_in_CSIP(SCIPsetSolVals(scip, sol, model->nvars, model->vars, values));
	SCIP_in_CSIP(SCIPtrySolFree(scip, &sol, FALSE, FALSE, TRUE, TRUE, TRUE, &stored));

	if (stored > 0)
	{
		heurdata->stored_sols += 1;
	}

	return CSIP_RETCODE_OK;
}

// Add a heuristic callback to the model.
// You may use userdata to pass any data.
CSIP_RETCODE CSIPaddHeuristicCallback(
	CSIP_MODEL *model, CSIP_HEURCALLBACK callback, void *userdata)
{
	SCIP_HEURDATA *heurdata;
	SCIP_HEUR *heur;
	SCIP *scip;
	char name[SCIP_MAXSTRLEN];

	scip = model->scip;

	SCIP_in_CSIP(SCIPallocMemory(scip, &heurdata));

	SCIPsnprintf(name, SCIP_MAXSTRLEN, "heur_%d", model->nheur);
	SCIP_in_CSIP(SCIPincludeHeurBasic(
		scip, &heur, name, "heuristic callback", 'x',
		1, 1, 0, -1, SCIP_HEURTIMING_AFTERNODE, FALSE,
		heurExecUser, heurdata));
	heurdata->model = model;
	heurdata->callback = callback;
	heurdata->userdata = userdata;
	heurdata->heur = heur;
	heurdata->stored_sols = 0;

	SCIP_in_CSIP(SCIPsetHeurFree(scip, heur, heurFreeUser));
	model->nheur += 1;

	return CSIP_RETCODE_OK;
}

/*
*  Message handler with a prefix
*/


struct SCIP_MessagehdlrData
{
	char* prefix;
};

static void logMessage(
	SCIP_MESSAGEHDLR* messagehdlr, FILE* file, const char* msg)
{
	SCIP_MESSAGEHDLRDATA* messagehdlrdata;
	messagehdlrdata = SCIPmessagehdlrGetData(messagehdlr);

	fputs(messagehdlrdata->prefix, file);
	fputs(msg, file);
	fflush(file);
	return;
}

static SCIP_DECL_MESSAGEHDLRFREE(messageHdlrFree)
{
	SCIP_MESSAGEHDLRDATA* messagehdlrdata = SCIPmessagehdlrGetData(messagehdlr);
	free(messagehdlrdata->prefix);
	SCIPfreeMemory(NULL, &messagehdlrdata);
	return SCIP_OKAY;
}

CSIP_RETCODE CSIPsetMessagePrefix(CSIP_MODEL *model, const char* prefix)
{
	SCIP_MESSAGEHDLR* messagehdlr = NULL;
	SCIP_MESSAGEHDLRDATA* messagehdlrdata = NULL;

	SCIP_in_CSIP(SCIPallocMemory(NULL, &messagehdlrdata));
	messagehdlrdata->prefix = strDup(prefix);
	SCIP_in_CSIP(SCIPmessagehdlrCreate(&messagehdlr, FALSE, NULL, FALSE,
		logMessage, logMessage, logMessage,
		messageHdlrFree, messagehdlrdata));

	SCIP_in_CSIP(SCIPsetMessagehdlr(model->scip, messagehdlr));
	SCIP_in_CSIP(SCIPmessagehdlrRelease(&messagehdlr));

	return CSIP_RETCODE_OK;
}

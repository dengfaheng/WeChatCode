typedef struct csip_model CSIP_MODEL;

/* return codes */
typedef int CSIP_RETCODE;
#define CSIP_RETCODE_OK 0
#define CSIP_RETCODE_ERROR 1
#define CSIP_RETCODE_NOMEMORY 2

/* solver status */
typedef int CSIP_STATUS;
#define CSIP_STATUS_OPTIMAL 0
#define CSIP_STATUS_INFEASIBLE 1
#define CSIP_STATUS_UNBOUNDED 2
#define CSIP_STATUS_INFORUNBD 3
#define CSIP_STATUS_NODELIMIT 4
#define CSIP_STATUS_TIMELIMIT 5
#define CSIP_STATUS_MEMLIMIT 6
#define CSIP_STATUS_USERLIMIT 7
#define CSIP_STATUS_UNKNOWN 8

/* variable types */
typedef int CSIP_VARTYPE;
#define CSIP_VARTYPE_BINARY 0
#define CSIP_VARTYPE_INTEGER 1
#define CSIP_VARTYPE_IMPLINT 2
#define CSIP_VARTYPE_CONTINUOUS 3

/* solving context for lazy callbacks */
typedef int CSIP_LAZY_CONTEXT;
#define CSIP_LAZY_LPRELAX 0     // we have (fractional) LP relaxtion of B&B node
#define CSIP_LAZY_INTEGRALSOL 1 // current candidate is integer feasible
#define CSIP_LAZY_OTHER 2       // e.g., CHECK is called on fractional candidate

/* nonlinear operators */
typedef int CSIP_OP;
#define VARIDX 1
#define CONST 2
#define MINUS 9
#define DIV 11
#define OPSQRT 13
#define POW 14
#define EXP 17
#define LOG 18
#define SUM 64
#define PROD 65

/* parameter types */
typedef int CSIP_PARAMTYPE;
#define CSIP_PARAMTYPE_NOTAPARAM -1
#define CSIP_PARAMTYPE_BOOL 0
#define CSIP_PARAMTYPE_INT 1
#define CSIP_PARAMTYPE_LONGINT 2
#define CSIP_PARAMTYPE_REAL 3
#define CSIP_PARAMTYPE_CHAR 4
#define CSIP_PARAMTYPE_STRING 5

// versioning scheme: major.minor.patch
int CSIPmajorVersion();
int CSIPminorVersion();
int CSIPpatchVersion();

// combined version
int CSIPgetVersion();

/* model definition */

// Create a new model (and solver).
CSIP_RETCODE CSIPcreateModel(CSIP_MODEL **model);

// Free all memory of model (and solver).
CSIP_RETCODE CSIPfreeModel(CSIP_MODEL *model);

// Add new variable to model.
// To omit a bound, use (-)INFINITY.
// The variable index will be assigned to idx; pass NULL if not needed.
CSIP_RETCODE CSIPaddVar(
    CSIP_MODEL *model, double lowerbound, double upperbound,
    CSIP_VARTYPE vartype, int *idx);

// Set new lower bounds for a set of variables.
CSIP_RETCODE CSIPchgVarLB(
    CSIP_MODEL *model, int numindices, int *indices, double *lowerbounds);

// Set new lower bounds for a set of variables.
CSIP_RETCODE CSIPchgVarUB(
    CSIP_MODEL *model, int numindices, int *indices, double *upperbounds);

// Set new type for a variable.
CSIP_RETCODE CSIPchgVarType(
    CSIP_MODEL *model, int varindex, CSIP_VARTYPE vartype);

// Get type of a variable.
CSIP_VARTYPE CSIPgetVarType(CSIP_MODEL *model, int varindex);

// Add new linear constraint to the model, of the form:
//    lhs <= sum_i coefs[i] * vars[i] <= rhs
// For one-sided inequalities, use (-)INFINITY for lhs or rhs.
// The constraint index will be assigned to idx; pass NULL if not needed.
CSIP_RETCODE CSIPaddLinCons(
    CSIP_MODEL *model, int numindices, int *indices, double *coefs,
    double lhs, double rhs, int *idx);

// Add new quadratic constraint to the model, of the form:
//    lhs <= sum_i lincoefs[i] * vars[lin[i]]
//           + sum_j quadcoefs[j] * vars[row[j]] * vars[col[j]] <= rhs
// For one-sided inequalities, use (-)INFINITY for lhs or rhs.
// The constraint index will be assigned to idx; pass NULL if not needed.
CSIP_RETCODE CSIPaddQuadCons(
    CSIP_MODEL *model, int numlinindices, int *linindices, double *lincoefs,
    int numquadterms, int *quadrowindices, int *quadcolindices,
    double *quadcoefs, double lhs, double rhs, int *idx);

// Add new nonlinear constraint to the model, of the form:
//    lhs <= expression <= rhs
// For one-sided inequalities, use (-)INFINITY for lhs or rhs.
// The expression is represented as follows:
// An array of operations, an array with the children of each operation
// and an array indicating which children are from which operation (begin):
// The children of op[k] are the ops/vars/values indexed from begin[k] until
// begin[k+1]-1.
// The child of VARIDX represents the index of the variable.
// The child of CONST represents the index of the constant in the value array.
// All others refer to indices in the op array.
// As an example: if we have a problem with variables x_0, x_1, x_2
// and we want to represent x_2^2, then we have operators:
// [VARIDX, CONST, POWER], with children
// [2, 0, 0, 1] and values [2.0]. begin is given by
// [0, 1, 2,   4] which means that the children of
// VARIDX are 2 -> the variables with index 2 (x_2)
// CONST are 0 -> the value with index 0 (2.0)
// POWER are 0, 1 -> the variable and the const (x_2 ^ 2.0)
// The constraint index will be assigned to idx; pass NULL if not needed.
CSIP_RETCODE CSIPaddNonLinCons(
    CSIP_MODEL *model, int nops, CSIP_OP *ops, int *children, int *begin,
    double *values, double lhs, double rhs, int *idx);

// Add SOS1 (special ordered set of type 1) constraint on a set of
// variables. That is, at most one variable is allowed to take on a
// nonzero value.
// Use weights to determine variable order, or NULL.
// The constraint index will be assigned to idx; pass NULL if not needed.
CSIP_RETCODE CSIPaddSOS1(
    CSIP_MODEL *model, int numindices, int *indices, double *weights, int *idx);

// Add SOS2 (special ordered set of type 2) constraint on a set of
// variables. That is, at most two consecutive variables are allowed
// to take on nonzero values.
// Use weights to determine variable order, or NULL.
// The constraint index will be assigned to idx; pass NULL if not needed.
CSIP_RETCODE CSIPaddSOS2(
    CSIP_MODEL *model, int numindices, int *indices, double *weights, int *idx);

// Set the linear objective function of the form: sum_i coefs[i] * vars[i]
CSIP_RETCODE CSIPsetObj(
    CSIP_MODEL *model, int numindices, int *indices, double *coefs);

// Set a quadratic objective function
CSIP_RETCODE CSIPsetQuadObj(CSIP_MODEL *model, int numlinindices,
                            int *linindices, double *lincoefs, int numquadterms,
                            int *quadrowindices, int *quadcolindices,
                            double *quadcoefs);

// Set a nonlinear objective function. See CSIPaddNonLinCons for an explanation
// about the format
CSIP_RETCODE CSIPsetNonlinearObj(
    CSIP_MODEL *model, int nops, int *ops, int *children, int *begin,
    double *values);

// Set the optimization sense to minimization. This is the default setting.
CSIP_RETCODE CSIPsetSenseMinimize(CSIP_MODEL *model);

// Set the optimization sense to maximization.
CSIP_RETCODE CSIPsetSenseMaximize(CSIP_MODEL *model);

// Solve the model.
CSIP_RETCODE CSIPsolve(CSIP_MODEL *model);

// Interrupt the solving process.
CSIP_RETCODE CSIPinterrupt(CSIP_MODEL *model);

// Copy the values of all variables in the best known solution into
// the output array. The user is responsible for memory allocation.
CSIP_RETCODE CSIPgetVarValues(CSIP_MODEL *model, double *output);

// Get the objective value of the best-known solution.
double CSIPgetObjValue(CSIP_MODEL *model);

// Get the best known bound on the optimal solution
double CSIPgetObjBound(CSIP_MODEL *model);

// Get the solving status.
CSIP_STATUS CSIPgetStatus(CSIP_MODEL *model);

// Get the type of a parameter
CSIP_PARAMTYPE CSIPgetParamType(CSIP_MODEL *model, const char *name);

// Set the value of an existing boolean parameter
CSIP_RETCODE CSIPsetBoolParam(
    CSIP_MODEL *model, const char *name, int value);

// Set the value of an existing int parameter
CSIP_RETCODE CSIPsetIntParam(
    CSIP_MODEL *model, const char *name, int value);

// Set the value of an existing long int parameter
CSIP_RETCODE CSIPsetLongintParam(
    CSIP_MODEL *model, const char *name, long long value);

// Set the value of an existing real parameter
CSIP_RETCODE CSIPsetRealParam(
    CSIP_MODEL *model, const char *name, double value);

// Set the value of an existing char parameter
CSIP_RETCODE CSIPsetCharParam(
    CSIP_MODEL *model, const char *name, char value);

// Set the value of an existing string parameter
CSIP_RETCODE CSIPsetStringParam(
    CSIP_MODEL *model, const char *name, const char *value);

// Get the number of variables added to the model.
int CSIPgetNumVars(CSIP_MODEL *model);

// Get the number of constraints added to the model.
// Beware: constraints added by a lazy callbacks are not counted here!
int CSIPgetNumConss(CSIP_MODEL *model);

// Supply a solution (as a dense array) to be checked at the beginning of the
// solving process. Partial solutions are also supported: Indicate missing
// values with NaN.
CSIP_RETCODE CSIPsetInitialSolution(CSIP_MODEL *model, double *values);

/* lazy constraint callback functions */

typedef struct SCIP_ConshdlrData CSIP_LAZYDATA;

// Get current context in which callback is called. Relates to the solution that
// is available through CSIPlazyGetVarValues.
CSIP_LAZY_CONTEXT CSIPlazyGetContext(CSIP_LAZYDATA *lazydata);

// Copy values of current (relaxation) solution to output array. Call
// this function from your lazy constraint callback.
CSIP_RETCODE CSIPlazyGetVarValues(CSIP_LAZYDATA *lazydata, double *output);

// Add a linear constraint from a lazy constraint callback.
// With islocal, you specify whether the added constraint is only
// valid locally (in the branch-and-bound subtree).
CSIP_RETCODE CSIPlazyAddLinCons(
    CSIP_LAZYDATA *lazydata, int numindices, int *indices, double *coefs,
    double lhs, double rhs, int islocal);

typedef CSIP_RETCODE(*CSIP_LAZYCALLBACK)(
    CSIP_MODEL *model, CSIP_LAZYDATA *lazydata, void *userdata);

// Add a lazy constraint callback to the model.
// You may use userdata to pass any data.
CSIP_RETCODE CSIPaddLazyCallback(
    CSIP_MODEL *model, CSIP_LAZYCALLBACK lazycb, void *userdata);

/* heuristic callback functions */

typedef struct SCIP_HeurData CSIP_HEURDATA;

// signature for heuristic callbacks.
// must only call `CSIPheur*` methods from within callback, passing `heurdata`.
typedef CSIP_RETCODE(*CSIP_HEURCALLBACK)(
    CSIP_MODEL *model, CSIP_HEURDATA *heurdata, void *userdata);

// Copy values of solution to output array. Call this function from your
// heuristic callback. Solution is LP relaxation of current node.
CSIP_RETCODE CSIPheurGetVarValues(CSIP_HEURDATA *heurdata, double *output);

// Supply a solution (as a dense array). Only complete solutions are supported.
CSIP_RETCODE CSIPheurAddSolution(CSIP_HEURDATA *heurdata, double *values);

// Add a heuristic callback to the model.
// You may use userdata to pass any data.
CSIP_RETCODE CSIPaddHeuristicCallback(
    CSIP_MODEL *model, CSIP_HEURCALLBACK heur, void *userdata);

/* advanced usage */

// Get access to the internal SCIP solver. Use at your own risk!
void *CSIPgetInternalSCIP(CSIP_MODEL *model);

/* additional features (on top of SCIP) */

// Set a prefix for all messages.
CSIP_RETCODE CSIPsetMessagePrefix(CSIP_MODEL *model, const char* prefix);

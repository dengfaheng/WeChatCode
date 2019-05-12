#include <assert.h>
#include <stdio.h>
#include <math.h>


#include "csip.h"
#include "minunit.h"


static void test_lp()
{
	/*
	Small LP:
	min -x
	s.t. 2x + y <= 1.5
	x,y >= 0
	solution is (0.75,0) with objval -0.75
	*/
	int numindices = 2;
	int indices[] = { 0, 1 };
	double objcoef[] = { -1.0, 0.0 };
	double conscoef[] = { 2.0, 1.0 };
	double solution[2];
	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	int x_idx, y_idx;
	CHECK(CSIPaddVar(m, 0.0, INFINITY, CSIP_VARTYPE_CONTINUOUS, &x_idx));
	CHECK(CSIPaddVar(m, 0.0, INFINITY, CSIP_VARTYPE_CONTINUOUS, &y_idx));
	mu_assert_int("Wrong var index!", x_idx, 0);
	mu_assert_int("Wrong var index!", y_idx, 1);

	CHECK(CSIPsetObj(m, numindices, indices, objcoef));
	int cons_idx;
	CHECK(CSIPaddLinCons(m, numindices, indices, conscoef, -INFINITY, 1.5,
		&cons_idx));
	mu_assert_int("Wrong cons index!", cons_idx, 0);

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_OPTIMAL);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, -0.75);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 0.75);
	mu_assert_near("Wrong solution!", solution[1], 0.0);

	CHECK(CSIPfreeModel(m));
}

static void test_mip()
{
	/*
	Small MIP:
	min -5x_1 - 3x_2 - 2x_3 - 7x_4 - 4x_5
	s.t. 2x_1 + 8x_2 + 4x_3 + 2x_4 + 5x_5 <= 10
	x Bin
	solution is (1,0,0,1,1) with objval -16
	*/
	int numindices = 5;
	int indices[] = { 0, 1, 2, 3, 4 };
	double objcoef[] = { -5.0, -3.0, -2.0, -7.0, -4.0 };
	double conscoef[] = { 2.0, 8.0, 4.0, 2.0, 5.0 };
	double solution[5];
	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	int var_idx;
	for (int i = 0; i < 5; i++)
	{
		CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_BINARY, &var_idx));
		mu_assert_int("Wrong var index!", var_idx, i);
	}

	CHECK(CSIPsetObj(m, numindices, indices, objcoef));
	int cons_idx;
	CHECK(CSIPaddLinCons(m, numindices, indices, conscoef, -INFINITY, 10.0,
		&cons_idx));
	mu_assert_int("Wrong cons index!", cons_idx, 0);

	CHECK(CSIPsolve(m));
	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_OPTIMAL);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, -16.0);
	double objbound = CSIPgetObjBound(m);
	mu_assert_near("Wrong objective bound!", objbound, -16.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 1.0);
	mu_assert_near("Wrong solution!", solution[1], 0.0);
	mu_assert_near("Wrong solution!", solution[2], 0.0);
	mu_assert_near("Wrong solution!", solution[3], 1.0);
	mu_assert_near("Wrong solution!", solution[4], 1.0);

	CHECK(CSIPfreeModel(m));
}

static void test_mip2()
{
	/*
	Small unbounded MIP:
	min  x
	x Integer
	*/
	int numindices = 1;
	int indices[] = { 0 };
	double objcoef[] = { 1.0 };
	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	int x_idx;
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_INTEGER, &x_idx));
	mu_assert_int("Wrong var index!", x_idx, 0);

	CHECK(CSIPsetObj(m, numindices, indices, objcoef));

	CHECK(CSIPsolve(m));
	int solvestatus = CSIPgetStatus(m);
	mu_assert("Wrong status!", (solvestatus == CSIP_STATUS_UNBOUNDED
		|| solvestatus == CSIP_STATUS_INFORUNBD));

	CHECK(CSIPfreeModel(m));
}

static void test_mip3()
{
	/*
	Small infeasible MIP:
	min  x
	x >= 2
	x <= 1
	*/
	int numindices = 1;
	int indices[] = { 0 };
	double objcoef[] = { 1.0 };
	double conscoef[] = { 1.0 };
	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_INTEGER, NULL));
	CHECK(CSIPsetObj(m, numindices, indices, objcoef));

	CHECK(CSIPaddLinCons(m, numindices, indices, conscoef, -INFINITY, 1.0, NULL));
	CHECK(CSIPaddLinCons(m, numindices, indices, conscoef, 2.0, INFINITY, NULL));

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_INFEASIBLE);

	CHECK(CSIPfreeModel(m));
}

static void test_socp()
{
	/*
	min t
	s.t. x + y >= 1
	x^2 + y^2 <= t^2
	t >= 0
	*/

	int objindices[] = { 0 };
	double objcoef[] = { 1.0 };
	int linindices[] = { 1, 2 };
	double lincoef[] = { 1.0, 1.0 };
	int quadi[] = { 0, 1, 2 };
	int quadj[] = { 0, 1, 2 };
	double quadcoef[] = { -1.0, 1.0, 1.0 };
	double solution[3];

	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	// t
	CHECK(CSIPaddVar(m, 0.0, INFINITY, CSIP_VARTYPE_CONTINUOUS, NULL));
	// x
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_CONTINUOUS, NULL));
	// y
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_CONTINUOUS, NULL));

	mu_assert_int("Wrong number of vars!", CSIPgetNumVars(m), 3);

	// sparse objective
	CHECK(CSIPsetObj(m, 1, objindices, objcoef));

	// sparse constraint
	CHECK(CSIPaddLinCons(m, 2, linindices, lincoef, 1.0, INFINITY, NULL));

	CHECK(CSIPaddQuadCons(m, 0, NULL, NULL, 3, quadi, quadj, quadcoef, -INFINITY,
		0.0, NULL));

	mu_assert_int("Wrong number of conss!", CSIPgetNumConss(m), 2);

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_OPTIMAL);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, sqrt(0.5));

	CHECK(CSIPgetVarValues(m, solution));

	mu_assert_near("Wrong solution!", solution[0], sqrt(0.5));
	// use weaker check, because of nonlinear constraint's abstol
	mu_assert("Wrong solution!", fabs(solution[1] - 0.5) < 0.01);
	mu_assert("Wrong solution!", fabs(solution[2] - 0.5) < 0.01);

	CHECK(CSIPfreeModel(m));
}

static void test_nlp()
{
	/*
	Small NLP:
	max x + y - z^3
	s.t. z^2 <= 1
	x, y <= 0
	solution is 0, 0, -1
	*/
	int nops = 3;
	CSIP_OP ops[] = { VARIDX, CONST, POW };
	int children[] = { 2, 0, 0, 1 };
	int begin[] = { 0, 1, 2, 4 };
	double values[] = { 2.0 };
	double lhs = -INFINITY;
	double rhs = 1;

	CSIP_OP obj_ops[] = { VARIDX, VARIDX, VARIDX, CONST, POW, MINUS, SUM };
	int obj_children[] = { 0, 1, 2, 0, 2, 3, 4, 0, 1, 5 };
	int obj_begin[] = { 0, 1, 2, 3, 4, 6, 7, 10 };
	double obj_values[] = { 3.0 };

	CSIP_MODEL *m;
	double solution[3];

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	int x_idx, y_idx, z_idx;
	CHECK(CSIPaddVar(m, -INFINITY, 0.0, CSIP_VARTYPE_CONTINUOUS, &x_idx));
	CHECK(CSIPaddVar(m, -INFINITY, 0.0, CSIP_VARTYPE_CONTINUOUS, &y_idx));
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_CONTINUOUS, &z_idx));
	mu_assert_int("Wrong var index!", x_idx, 0);
	mu_assert_int("Wrong var index!", y_idx, 1);
	mu_assert_int("Wrong var index!", z_idx, 2);

	int cons_idx;
	CHECK(CSIPaddNonLinCons(m, nops, ops, children, begin, values, lhs, rhs,
		&cons_idx));
	mu_assert_int("Wrong cons index!", cons_idx, 0);

	CHECK(CSIPsetNonlinearObj(m, 7, obj_ops, obj_children, obj_begin, obj_values));
	CHECK(CSIPsetSenseMaximize(m));
	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_OPTIMAL);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, 1.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 0.0);
	mu_assert_near("Wrong solution!", solution[1], 0.0);
	mu_assert_near("Wrong solution!", solution[2], -1.0);

	CHECK(CSIPfreeModel(m));
}

static void test_nlp_no_obj()
{
	/*
	Small feasibility NLP:
	find z
	s.t. z^2 <= 1
	*/
	int nops = 3;
	CSIP_OP ops[] = { VARIDX, CONST, POW };
	int children[] = { 0, 0, 0, 1 };
	int begin[] = { 0, 1, 2, 4 };
	double values[] = { 2.0 };
	double lhs = -INFINITY;
	double rhs = 1;

	CSIP_MODEL *m;
	double solution[1];

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	int z_idx;
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_CONTINUOUS, &z_idx));
	mu_assert_int("Wrong var index!", z_idx, 0);

	int cons_idx;
	CHECK(CSIPaddNonLinCons(m, nops, ops, children, begin, values, lhs, rhs,
		&cons_idx));
	mu_assert_int("Wrong cons index!", cons_idx, 0);

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_OPTIMAL);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert("Violated constraint!", solution[0] * solution[0] <= 1.001);

	CHECK(CSIPfreeModel(m));
}

static void test_quadobj()
{
	/*
	min x^2 + y^2
	s.t. x + y >= 1
	-> {0.5, 0.5}
	*/

	int linindices[] = { 0, 1 };
	double lincoef[] = { 1.0, 1.0 };
	int quadi[] = { 0, 1 };
	int quadj[] = { 0, 1 };
	double quadcoef[] = { 1.0, 1.0 };
	double solution[3];

	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	// x
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_CONTINUOUS, NULL));
	// y
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_CONTINUOUS, NULL));

	mu_assert_int("Wrong number of vars!", CSIPgetNumVars(m), 2);

	// sparse quadratic objective
	CHECK(CSIPsetQuadObj(m, 0, NULL, NULL, 2, quadi, quadj, quadcoef));

	// sparse constraint
	CHECK(CSIPaddLinCons(m, 2, linindices, lincoef, 1.0, INFINITY, NULL));

	mu_assert_int("Wrong number of conss!", CSIPgetNumConss(m), 1);

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_OPTIMAL);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, 0.5);

	CHECK(CSIPgetVarValues(m, solution));

	// use weaker check, because of nonlinear constraint's abstol
	mu_assert("Wrong solution!", fabs(solution[0] - 0.5) < 0.01);
	mu_assert("Wrong solution!", fabs(solution[1] - 0.5) < 0.01);

	CHECK(CSIPfreeModel(m));
}

struct MyData
{
	int foo;
	double *storage;
};

CSIP_RETCODE lazy_callback(CSIP_MODEL *m, CSIP_LAZYDATA *lazydata,
	void *userdata)
{

	struct MyData *data = (struct MyData *) userdata;
	if (data->foo != 10)
	{
		return CSIP_RETCODE_ERROR;
	}
	int indices[] = { 0, 1 };
	double coef[] = { 1.0, 1.0 };

	CSIPlazyGetVarValues(lazydata, data->storage);

	// enforce x + y <= 3, global cut
	if (data->storage[0] + data->storage[1] > 3)
	{
		CSIPlazyAddLinCons(lazydata, 2, indices, coef, -INFINITY, 3.0, 0);
	}

	return CSIP_RETCODE_OK;
}

/* the problem written originally was:
*    min 0.5x + y
*    s.t. -inf <= x,y <= 2
*          x + y <= 3 (lazy)
* which is unbounded, and for some scip-bug reason, it asserted some stuff
* I am changing the problem to reflect Miles original vision
*/
static void test_lazy()
{

	/*
	max 0.5x + y
	s.t. 0 <= x,y <= 2
	x + y <= 3 (lazy)
	solution is (1,2)
	*/

	int objindices[] = { 0, 1 };
	double objcoef[] = { 0.5, 1.0 };
	double solution[2];

	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	// x
	CHECK(CSIPaddVar(m, 0.0, 2.0, CSIP_VARTYPE_INTEGER, NULL));

	// y
	CHECK(CSIPaddVar(m, 0.0, 2.0, CSIP_VARTYPE_INTEGER, NULL));

	CHECK(CSIPsetObj(m, 2, objindices, objcoef));

	CHECK(CSIPsetSenseMaximize(m));

	struct MyData userdata = { 10, &solution[0] };

	CHECK(CSIPaddLazyCallback(m, lazy_callback, &userdata));

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_OPTIMAL);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective!", objval, 2.5);

	CHECK(CSIPgetVarValues(m, solution));

	mu_assert_near("Wrong solution!", solution[0], 1.0);
	mu_assert_near("Wrong solution!", solution[1], 2.0);

	CHECK(CSIPfreeModel(m));
}

CSIP_RETCODE lazy_callback2(CSIP_MODEL *m, CSIP_LAZYDATA *lazydata,
	void *userdata)
{

	struct MyData *data = (struct MyData *) userdata;
	mu_assert("userdata failing", data->foo != 0);

	int indices[] = { 0 };
	double coef[] = { 1.0 };

	if (CSIPlazyGetContext(lazydata) == CSIP_LAZY_INTEGRALSOL)
	{
		CSIPlazyGetVarValues(lazydata, data->storage);
		// make sure we didn't get a fractional solution
		mu_assert_near("lazy context not working", data->storage[0],
			round(data->storage[0]));
	}

	// always add the cut x <= 10
	CSIPlazyAddLinCons(lazydata, 1, indices, coef, -INFINITY, 10.5, 0);

	return CSIP_RETCODE_OK;
}

static void test_lazy2()
{

	/*
	min -x
	s.t. x <= 100.5, integer
	x <= 10.5 (lazy)
	solution is -10
	*/

	int objindices[] = { 0 };
	double objcoef[] = { -1.0 };
	double solution[1];

	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	// x
	CHECK(CSIPaddVar(m, -INFINITY, 100.5, CSIP_VARTYPE_INTEGER, NULL));

	CHECK(CSIPsetObj(m, 1, objindices, objcoef));

	struct MyData userdata = { 10, &solution[0] };

	CHECK(CSIPaddLazyCallback(m, lazy_callback2, &userdata));

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_OPTIMAL);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, -10.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 10.0);

	CHECK(CSIPfreeModel(m));
}

CSIP_RETCODE lazycb_interrupt(CSIP_MODEL *m, CSIP_LAZYDATA *lazydata,
	void *userdata)
{
	CHECK(CSIPinterrupt(m));
	return CSIP_RETCODE_OK;
}

static void test_lazy_interrupt()
{
	/*
	find x
	s.t. x >= 1.5, integer
	solution is interrupted
	*/

	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPaddVar(m, 1.5, INFINITY, CSIP_VARTYPE_INTEGER, NULL));

	CHECK(CSIPaddLazyCallback(m, lazycb_interrupt, NULL));

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_USERLIMIT);

	CHECK(CSIPfreeModel(m));
}

static void test_objsense()
{
	// min/max  x
	// st.      lb <= x <= ub
	CSIP_MODEL *m;
	int objindices[] = { 0 };
	double objcoef[] = { 1.0 };
	double lb = -2.3;
	double ub = 4.2;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPaddVar(m, lb, ub, CSIP_VARTYPE_CONTINUOUS, NULL));
	CHECK(CSIPsetObj(m, 1, objindices, objcoef));

	// default sense is 'minimize'
	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), lb);

	// change sense to 'maximize'
	CHECK(CSIPsetSenseMaximize(m));
	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), ub);

	// change sense to 'minimize'
	CHECK(CSIPsetSenseMinimize(m));
	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), lb);

	CHECK(CSIPfreeModel(m));
}

static void test_sos1()
{
	// max 2x + 3y + 4z
	//     SOS1(x, y, z)
	//     0 <= x, y, z <= 1
	//
	// sol -> (0, 0, 1)

	CSIP_MODEL *m;
	int objindices[] = { 0, 1, 2 };
	double objcoef[] = { 2.0, 3.0, 4.0 };

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // x
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // y
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // z
	CHECK(CSIPaddSOS1(m, 3, objindices, NULL, NULL));
	CHECK(CSIPsetSenseMaximize(m));
	CHECK(CSIPsetObj(m, 3, objindices, objcoef));
	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 4.0);
	CHECK(CSIPfreeModel(m));
}

static void test_sos2()
{
	// max 2x + 3y + 4z
	//     SOS2(x, y, z)
	//     0 <= x, y, z <= 1
	//
	// sol -> (0, 1, 1)

	CSIP_MODEL *m;
	int objindices[] = { 0, 1, 2 };
	double objcoef[] = { 2.0, 3.0, 4.0 };

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // x
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // y
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // z
	CHECK(CSIPaddSOS2(m, 3, objindices, NULL, NULL));
	CHECK(CSIPsetSenseMaximize(m));
	CHECK(CSIPsetObj(m, 3, objindices, objcoef));
	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 7.0);
	CHECK(CSIPfreeModel(m));
}

static void test_sos1_sos2()
{
	// max 2x + 3y + 4z
	//     SOS1(y, z)
	//     SOS2(x, y, z)
	//     0 <= x, y, z <= 1
	//
	// sol -> (1, 0, 0)

	CSIP_MODEL *m;
	int objindices[] = { 0, 1, 2 };
	double objcoef[] = { 2.0, 3.0, 4.0 };

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // x
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // y
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // z
	CHECK(CSIPaddSOS1(m, 2, objindices + 1, NULL, NULL));
	CHECK(CSIPaddSOS2(m, 3, objindices, NULL, NULL));
	CHECK(CSIPsetSenseMaximize(m));
	CHECK(CSIPsetObj(m, 3, objindices, objcoef));
	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 5.0);
	CHECK(CSIPfreeModel(m));
}

static void test_manythings()
{
	// add many vars and conss to test variable sized array
	CSIP_MODEL *m;
	int indices[] = { 0 };
	double coefs[] = { 1.0 };
	int n = 9999;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	for (int i = 0; i < n; ++i)
	{
		CHECK(CSIPaddVar(m, 0.0, i, CSIP_VARTYPE_CONTINUOUS, NULL));
		indices[0] = i;
		CHECK(CSIPaddLinCons(m, 1, indices, coefs, 0.0, 1.0, NULL));
	}
	CHECK(CSIPfreeModel(m));
}


// store cut data
struct DoubleData
{
	int indices[2];
};

CSIP_RETCODE doubly_lazy_cb(CSIP_MODEL *m, CSIP_LAZYDATA *lazydata,
	void *userdata)
{

	struct DoubleData *data = (struct DoubleData *) userdata;
	double coef[] = { 1.0, 1.0 };

	// always add the cut var1 + var2 <= 1
	CSIPlazyAddLinCons(lazydata, 2, data->indices, coef, -INFINITY, 1.0, 0);

	return CSIP_RETCODE_OK;
}

static void test_doublelazy()
{
	// max x + 3y + z
	//     x + y + z <= 2
	//     x + y <= 1 // lazy1
	//     y + z <= 1 // lazy2
	//     0 <= x, y, z <= 1
	//
	// sol -> (0, 1, 0)

	CSIP_MODEL *m;
	int indices[] = { 0, 1, 2 };
	double lincoef[] = { 1.0, 1.0, 1.0 };
	double objcoef[] = { 1.0, 3.0, 1.0 };
	struct DoubleData data1, data2;
	double solution[3];

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // x
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // y
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_CONTINUOUS, NULL));   // z
	CHECK(CSIPaddLinCons(m, 3, indices, lincoef, -INFINITY, 2.0, NULL));
	CHECK(CSIPsetSenseMaximize(m));
	CHECK(CSIPsetObj(m, 3, indices, objcoef));

	data1.indices[0] = 0;
	data1.indices[1] = 1;
	CHECK(CSIPaddLazyCallback(m, doubly_lazy_cb, &data1));

	data2.indices[0] = 2;
	data2.indices[1] = 1;
	CHECK(CSIPaddLazyCallback(m, doubly_lazy_cb, &data2));

	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 3.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 0.0);
	mu_assert_near("Wrong solution!", solution[1], 1.0);
	mu_assert_near("Wrong solution!", solution[2], 0.0);

	CHECK(CSIPfreeModel(m));
}

static void test_changeprob()
{
	// solve two problems in a row:
	//
	// max x + 2y
	//     x + y <= 1
	//     x, y binary
	//
	// --> (0, 1)
	//
	// max x + 2y + 2z
	//     x + y <= 1
	//     x + y + z <= 2
	//     y + z <= 1
	//     x, y, z binary
	//
	// --> (1, 0, 1)

	CSIP_MODEL *m;
	int indices[] = { 0, 1, 2 };
	double lincoef[] = { 1.0, 1.0, 1.0 };
	double objcoef[] = { 1.0, 2.0, 2.0 };
	double solution[3];

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	// first problem
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_BINARY, NULL)); // x
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_BINARY, NULL)); // y
	CHECK(CSIPaddLinCons(m, 2, indices, lincoef, -INFINITY, 1.0, NULL));
	CHECK(CSIPsetSenseMaximize(m));
	CHECK(CSIPsetObj(m, 2, indices, objcoef));

	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 2.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 0.0);
	mu_assert_near("Wrong solution!", solution[1], 1.0);

	// second problem, modifying the first
	CHECK(CSIPaddVar(m, 0.0, 1.0, CSIP_VARTYPE_BINARY, NULL));   // z
	CHECK(CSIPaddLinCons(m, 3, indices, lincoef, -INFINITY, 2.0, NULL));
	CHECK(CSIPaddLinCons(m, 2, indices + 1, lincoef, -INFINITY, 1.0, NULL));
	CHECK(CSIPsetObj(m, 3, indices, objcoef));

	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 3.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 1.0);
	mu_assert_near("Wrong solution!", solution[1], 0.0);
	mu_assert_near("Wrong solution!", solution[2], 1.0);

	CHECK(CSIPfreeModel(m));
}

static void test_changequadprob()
{
	/* Solve two problems in a row:
	min x^2 + y^2
	s.t. x + y >= 1
	-> {0.5, 0.5}

	max -x^2 + y
	x + y == 1
	-> (-0.5, 1.5)
	*/

	int linindices[] = { 0, 1 };
	double lincoef[] = { 1.0, 1.0 };
	int quadi[] = { 0, 1 };
	int quadj[] = { 0, 1 };
	double quadcoef[] = { 1.0, 1.0 };
	double quadcoef2[] = { -1.0 };
	double solution[3];

	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	// x
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_CONTINUOUS, NULL));
	// y
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_CONTINUOUS, NULL));

	mu_assert_int("Wrong number of vars!", CSIPgetNumVars(m), 2);

	// sparse quadratic objective
	CHECK(CSIPsetQuadObj(m, 0, NULL, NULL, 2, quadi, quadj, quadcoef));

	// sparse constraint
	CHECK(CSIPaddLinCons(m, 2, linindices, lincoef, 1.0, INFINITY, NULL));

	mu_assert_int("Wrong number of conss!", CSIPgetNumConss(m), 1);

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_OPTIMAL);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, 0.5);

	CHECK(CSIPgetVarValues(m, solution));

	// use weaker check, because of nonlinear constraint's abstol
	mu_assert("Wrong solution!", fabs(solution[0] - 0.5) < 0.01);
	mu_assert("Wrong solution!", fabs(solution[1] - 0.5) < 0.01);


	// second problem, modifying the first
	CHECK(CSIPsetSenseMaximize(m));
	CHECK(CSIPsetQuadObj(m, 1, &linindices[1], &lincoef[1], 1, quadi, quadj,
		quadcoef2));

	// sparse constraint
	CHECK(CSIPaddLinCons(m, 2, linindices, lincoef, -INFINITY, 1.0, NULL));

	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 1.25);

	CHECK(CSIPgetVarValues(m, solution));
	// use weaker check, because of nonlinear constraint's abstol
	mu_assert("Wrong solution!", fabs(solution[0] + 0.5) < 0.01);
	mu_assert("Wrong solution!", fabs(solution[1] - 1.5) < 0.01);

	CHECK(CSIPfreeModel(m));
}


static void test_changevartype()
{
	// solve two problems in a row:
	//
	// min 2x + 3y
	//     x + y >= 1.5
	//     0 <= x,y <= 9
	//
	// --> (1.5, 0)
	//
	// and with x integer
	//
	// --> (1, 0.5)


	CSIP_MODEL *m;
	int indices[] = { 0, 1 };
	double lincoef[] = { 1.0, 1.0 };
	double objcoef[] = { 2.0, 3.0 };
	double solution[2];

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	// first problem
	CHECK(CSIPaddVar(m, 0.0, 9.0, CSIP_VARTYPE_CONTINUOUS, NULL)); // x
	CHECK(CSIPaddVar(m, 0.0, 9.0, CSIP_VARTYPE_CONTINUOUS, NULL)); // y
	CHECK(CSIPaddLinCons(m, 2, indices, lincoef, 1.5, INFINITY, NULL));
	CHECK(CSIPsetSenseMinimize(m));
	CHECK(CSIPsetObj(m, 2, indices, objcoef));

	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 3.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 1.5);
	mu_assert_near("Wrong solution!", solution[1], 0.0);

	// second problem, modifying the first
	mu_assert_int("Wrong vartype", CSIPgetVarType(m, 0), CSIP_VARTYPE_CONTINUOUS);
	mu_assert_int("Wrong vartype", CSIPgetVarType(m, 1), CSIP_VARTYPE_CONTINUOUS);
	CHECK(CSIPchgVarType(m, 0, CSIP_VARTYPE_INTEGER));
	mu_assert_int("Wrong vartype", CSIPgetVarType(m, 0), CSIP_VARTYPE_INTEGER);
	mu_assert_int("Wrong vartype", CSIPgetVarType(m, 1), CSIP_VARTYPE_CONTINUOUS);

	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_OPTIMAL);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 3.5);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 1.0);
	mu_assert_near("Wrong solution!", solution[1], 0.5);

	CHECK(CSIPfreeModel(m));
}

static void test_initialsol()
{
	// attempt to solve a problem, but specify limits such that only the
	// user-defined initial solution is found
	//
	// min 2x
	//     x in [10, 100], integer

	CSIP_MODEL *m;
	int indices[] = { 0 };
	double objcoef[] = { 2.0 };
	double solution[1];

	double initialsol[] = { 23.0 };

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPsetIntParam(m, "limits/solutions", 1));
	CHECK(CSIPsetIntParam(m, "heuristics/trivial/freq", -1));

	CHECK(CSIPaddVar(m, 10.0, 100.0, CSIP_VARTYPE_INTEGER, NULL)); // x
	CHECK(CSIPsetObj(m, 1, indices, objcoef));

	CHECK(CSIPsetInitialSolution(m, initialsol));


	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_USERLIMIT);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 46.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 23.0);

	CHECK(CSIPfreeModel(m));
}

static void test_initialsol_nlp()
{
	/*
	attempt to solve a small NLP problem, but specify limits such that only
	the user-defined initial solution is found

	max x + y - z^3
	s.t. z^2 <= 1
	x, y <= 0

	optimal solution is 0,  0, -1
	initial solution is 0, -1, -0.5
	*/
	int nops = 3;
	CSIP_OP ops[] = { VARIDX, CONST, POW };
	int children[] = { 2, 0, 0, 1 };
	int begin[] = { 0, 1, 2, 4 };
	double values[] = { 2.0 };
	double lhs = -INFINITY;
	double rhs = 1;

	CSIP_OP obj_ops[] = { VARIDX, VARIDX, VARIDX, CONST, POW, MINUS, SUM };
	int obj_children[] = { 0, 1, 2, 0, 2, 3, 4, 0, 1, 5 };
	int obj_begin[] = { 0, 1, 2, 3, 4, 6, 7, 10 };
	double obj_values[] = { 3.0 };

	CSIP_MODEL *m;
	double solution[3];
	double initialsol[3] = { 0, -1, -0.5 };

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPsetIntParam(m, "limits/solutions", 1));
	CHECK(CSIPsetIntParam(m, "heuristics/trivial/freq", -1));

	int x_idx, y_idx, z_idx;
	CHECK(CSIPaddVar(m, -INFINITY, 0.0, CSIP_VARTYPE_CONTINUOUS, &x_idx));
	CHECK(CSIPaddVar(m, -INFINITY, 0.0, CSIP_VARTYPE_CONTINUOUS, &y_idx));
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_CONTINUOUS, &z_idx));
	mu_assert_int("Wrong var index!", x_idx, 0);
	mu_assert_int("Wrong var index!", y_idx, 1);
	mu_assert_int("Wrong var index!", z_idx, 2);

	int cons_idx;
	CHECK(CSIPaddNonLinCons(m, nops, ops, children, begin, values, lhs, rhs,
		&cons_idx));
	mu_assert_int("Wrong cons index!", cons_idx, 0);

	CHECK(CSIPsetNonlinearObj(m, 7, obj_ops, obj_children, obj_begin, obj_values));
	CHECK(CSIPsetSenseMaximize(m));

	CHECK(CSIPsetInitialSolution(m, initialsol));

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_USERLIMIT);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, 0 - 1.0 + 0.125);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 0.0);
	mu_assert_near("Wrong solution!", solution[1], -1.0);
	mu_assert_near("Wrong solution!", solution[2], -0.5);

	CHECK(CSIPfreeModel(m));
}

static void test_initialsol_partial()
{
	/*
	attempt to solve a small MIP problem, but specify limits such that only
	the user-defined partial initial solution is found

	max x + 2y
	s.t. x + y == 2
	0 <= x, y <= 2  (integer)

	optimal solution is 0,  2
	initial solution is 1,  ?  (partial)
	*/

	CSIP_MODEL *m;
	int indices[2] = { 0, 1 };
	double solution[2];
	double lincoef[2] = { 1.0, 1.0 };
	double objcoef[2] = { 1.0, 2.0 };
	double mynan = 0.0;
	double initialsol[2] = { 1, mynan };

	mu_assert_int("Not Not a Number!", (mynan != mynan), 1);

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPsetIntParam(m, "limits/solutions", 1));
	CHECK(CSIPsetIntParam(m, "heuristics/trivial/freq", -1));

	CHECK(CSIPaddVar(m, 0.0, 2.0, CSIP_VARTYPE_INTEGER, NULL));
	CHECK(CSIPaddVar(m, 0.0, 2.0, CSIP_VARTYPE_INTEGER, NULL));

	CHECK(CSIPaddLinCons(m, 2, indices, lincoef, 2.0, 2.0, NULL));

	CHECK(CSIPsetObj(m, 2, indices, objcoef));
	CHECK(CSIPsetSenseMaximize(m));

	CHECK(CSIPsetInitialSolution(m, initialsol));

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_USERLIMIT);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, 1.0*1.0 + 2.0*1.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 1.0);
	mu_assert_near("Wrong solution!", solution[1], 1.0);

	CHECK(CSIPfreeModel(m));
}

static void test_initialsol_nlp_partial()
{
	/*
	attempt to solve a small NLP problem, but specify limits such that only
	the user-defined, partial initial solution is found

	max x + y - z^3
	s.t. z^2 <= 1
	x, y <= 0

	optimal solution is 0,  0, -1
	initial solution is ?, -1, -1
	*/
	int nops = 3;
	CSIP_OP ops[] = { VARIDX, CONST, POW };
	int children[] = { 2, 0, 0, 1 };
	int begin[] = { 0, 1, 2, 4 };
	double values[] = { 2.0 };
	double lhs = -INFINITY;
	double rhs = 1;

	CSIP_OP obj_ops[] = { VARIDX, VARIDX, VARIDX, CONST, POW, MINUS, SUM };
	int obj_children[] = { 0, 1, 2, 0, 2, 3, 4, 0, 1, 5 };
	int obj_begin[] = { 0, 1, 2, 3, 4, 6, 7, 10 };
	double obj_values[] = { 3.0 };

	CSIP_MODEL *m;
	double solution[3];
	double mynan = 0.0;
	double initialsol[3] = { mynan, -1, -1 };

	mu_assert_int("Not Not a Number!", (mynan != mynan), 1);

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPsetIntParam(m, "limits/solutions", 1));
	CHECK(CSIPsetIntParam(m, "heuristics/trivial/freq", -1));

	int x_idx, y_idx, z_idx;
	CHECK(CSIPaddVar(m, -INFINITY, 0.0, CSIP_VARTYPE_INTEGER, &x_idx));
	CHECK(CSIPaddVar(m, -INFINITY, 0.0, CSIP_VARTYPE_INTEGER, &y_idx));
	CHECK(CSIPaddVar(m, -INFINITY, INFINITY, CSIP_VARTYPE_INTEGER, &z_idx));
	mu_assert_int("Wrong var index!", x_idx, 0);
	mu_assert_int("Wrong var index!", y_idx, 1);
	mu_assert_int("Wrong var index!", z_idx, 2);

	int cons_idx;
	CHECK(CSIPaddNonLinCons(m, nops, ops, children, begin, values, lhs, rhs,
		&cons_idx));
	mu_assert_int("Wrong cons index!", cons_idx, 0);

	CHECK(CSIPsetNonlinearObj(m, 7, obj_ops, obj_children, obj_begin, obj_values));
	CHECK(CSIPsetSenseMaximize(m));

	CHECK(CSIPsetInitialSolution(m, initialsol));

	CHECK(CSIPsolve(m));

	int solvestatus = CSIPgetStatus(m);
	mu_assert_int("Wrong status!", solvestatus, CSIP_STATUS_USERLIMIT);

	double objval = CSIPgetObjValue(m);
	mu_assert_near("Wrong objective value!", objval, 0 - 1.0 + 1.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 0.0);
	mu_assert_near("Wrong solution!", solution[1], -1.0);
	mu_assert_near("Wrong solution!", solution[2], -1.0);

	CHECK(CSIPfreeModel(m));
}

CSIP_RETCODE heurcb(CSIP_MODEL *model, CSIP_HEURDATA *heurdata, void *userdata)
{
	double sol[] = { 2.0, 2.0 };
	mu_assert("Invalid userdata", userdata == NULL);
	CHECK(CSIPheurAddSolution(heurdata, sol));
	return CSIP_RETCODE_OK;
}

static void test_heurcb()
{
	// attempt to solve a problem, but specify limits such that only the
	// solution given from the heuristic callback is found
	//
	// min x + y
	//     2x + 3y >= 6
	//     3x + 2y >= 6
	//     x,y in [0, 3] integer

	CSIP_MODEL *m;
	int indices[] = { 0, 1 };
	double objcoef[] = { 1.0, 1.0 };
	double coef1[] = { 2.0, 3.0 };
	double coef2[] = { 3.0, 2.0 };
	double solution[2];

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	CHECK(CSIPsetIntParam(m, "limits/solutions", 1));
	CHECK(CSIPsetIntParam(m, "heuristics/feaspump/freq", -1));
	CHECK(CSIPsetIntParam(m, "heuristics/randrounding/freq", -1));
	CHECK(CSIPsetIntParam(m, "heuristics/rounding/freq", -1));
	CHECK(CSIPsetIntParam(m, "heuristics/shiftandpropagate/freq", -1));
	CHECK(CSIPsetIntParam(m, "heuristics/shifting/freq", -1));
	CHECK(CSIPsetIntParam(m, "heuristics/simplerounding/freq", -1));
	CHECK(CSIPsetIntParam(m, "heuristics/trivial/freq", -1));
	CHECK(CSIPsetIntParam(m, "presolving/maxrounds", 0));
	CHECK(CSIPsetIntParam(m, "separating/maxroundsroot", 0));

	CHECK(CSIPaddVar(m, 0.0, 3.0, CSIP_VARTYPE_INTEGER, NULL)); // x
	CHECK(CSIPaddVar(m, 0.0, 3.0, CSIP_VARTYPE_INTEGER, NULL)); // y
	CHECK(CSIPaddLinCons(m, 2, indices, coef1, 6.0, INFINITY, NULL));
	CHECK(CSIPaddLinCons(m, 2, indices, coef2, 6.0, INFINITY, NULL));
	CHECK(CSIPsetObj(m, 2, indices, objcoef));

	CHECK(CSIPaddHeuristicCallback(m, heurcb, NULL));

	CHECK(CSIPsolve(m));
	mu_assert_int("Wrong status!", CSIPgetStatus(m), CSIP_STATUS_USERLIMIT);
	mu_assert_near("Wrong objective value!", CSIPgetObjValue(m), 4.0);

	CHECK(CSIPgetVarValues(m, solution));
	mu_assert_near("Wrong solution!", solution[0], 2.0);
	mu_assert_near("Wrong solution!", solution[1], 2.0);

	CHECK(CSIPfreeModel(m));
}


static void test_params()
{
	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));
	mu_assert_int("Wrong param type!", CSIPgetParamType(m, "display/verblevel"),
		CSIP_PARAMTYPE_INT);
	mu_assert_int("Wrong param type!", CSIPgetParamType(m, "what am I?"),
		CSIP_PARAMTYPE_NOTAPARAM);

	CHECK(CSIPfreeModel(m));
}

static void test_prefix()
{
	CSIP_MODEL *m;

	CHECK(CSIPcreateModel(&m));
	CHECK(CSIPsetIntParam(m, "display/verblevel", 2));

	char prefix[] = "test!prefix - ";
	CHECK(CSIPsetMessagePrefix(m, prefix));

	CHECK(CSIPsolve(m));
	CHECK(CSIPfreeModel(m));

	// just checks that nothing crashes
	// need to increase verblevel to see some output
}

int main(int argc, char **argv)
{
	printf("Running tests...\n");

	mu_run_test(test_lp);
	mu_run_test(test_mip);
	mu_run_test(test_mip2);
	mu_run_test(test_mip3);
	mu_run_test(test_socp);
	mu_run_test(test_nlp);
	mu_run_test(test_nlp_no_obj);
	mu_run_test(test_quadobj);
	mu_run_test(test_lazy);
	mu_run_test(test_lazy2);
	mu_run_test(test_lazy_interrupt);
	mu_run_test(test_objsense);
	mu_run_test(test_sos1);
	mu_run_test(test_sos2);
	mu_run_test(test_sos1_sos2);
	mu_run_test(test_manythings);
	mu_run_test(test_doublelazy);
	mu_run_test(test_changeprob);
	mu_run_test(test_changequadprob);
	mu_run_test(test_changevartype);
	mu_run_test(test_initialsol);
	mu_run_test(test_initialsol_nlp);
	//mu_run_test(test_initialsol_partial);
	//mu_run_test(test_initialsol_nlp_partial);
	mu_run_test(test_heurcb);
	mu_run_test(test_params);
	mu_run_test(test_prefix);

	printf("All tests passed!\n");
	system("pause");

	
	return 0;
}

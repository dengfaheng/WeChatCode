/* ALNS_Framework - a framework to develop ALNS based solvers
 *
 * Copyright (C) 2012 Renaud Masson
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 3 as published by the Free Software Foundation
 * (the "LGPL"). If you do not alter this notice, a recipient may use
 * your version of this file under the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-3; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the ALNS_Framework library.
 *
 *
 * Contributor(s):
 *	Renaud Masson
 */

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "TSPSolution.h"
#include "TSP_Best_Insert.h"
#include "TSP_Random_Insert.h"
#include "TSP_Random_Removal.h"
#include "TSP_Worst_Removal.h"
#include "TSP_History_Removal.h"
#include "TSP_LS.h"
#include "ALNS_inc.h"

using namespace std;

const int CITY_SIZE = 52;

typedef struct
{
	double x;
	double y;
}CITIES;

//berlin52城市坐标，最优解7542好像
CITIES berlin52[CITY_SIZE] = { { 565,575 },{ 25,185 },{ 345,750 },{ 945,685 },{ 845,655 },
{ 880,660 },{ 25,230 },{ 525,1000 },{ 580,1175 },{ 650,1130 },{ 1605,620 },
{ 1220,580 },{ 1465,200 },{ 1530,5 },{ 845,680 },{ 725,370 },{ 145,665 },
{ 415,635 },{ 510,875 },{ 560,365 },{ 300,465 },{ 520,585 },{ 480,415 },
{ 835,625 },{ 975,580 },{ 1215,245 },{ 1320,315 },{ 1250,400 },{ 660,180 },
{ 410,250 },{ 420,555 },{ 575,665 },{ 1150,1160 },{ 700,580 },{ 685,595 },
{ 685,610 },{ 770,610 },{ 795,645 },{ 720,635 },{ 760,650 },{ 475,960 },
{ 95,260 },{ 875,920 },{ 700,500 },{ 555,815 },{ 830,485 },{ 1170,65 },
{ 830,610 },{ 605,625 },{ 595,360 },{ 1340,725 },{ 1740,245 } };

int main(int argc, char* argv[])
{

	double** distances = new double*[CITY_SIZE];
	for(int i = 0; i < CITY_SIZE; i++)
	{
		distances[i] = new double[CITY_SIZE];
		for(int j = 0; j < CITY_SIZE; j++)
		{
			distances[i][j] = sqrt((berlin52[i].x-berlin52[j].x)*(berlin52[i].x-berlin52[j].x)+(berlin52[i].y-berlin52[j].y)*(berlin52[i].y-berlin52[j].y));
		}
	}
	TSPSolution initialSol(distances,CITY_SIZE);
	TSP_Best_Insert bestI("Best Insertion");
	TSP_Random_Insert randomI("Random Insertion");
	TSP_Random_Removal randomR("Random Removal");
	TSP_Worst_Removal worstR("Worst Removal");
	TSP_History_Removal historyR("History Removal",CITY_SIZE);

	randomI.repairSolution(dynamic_cast<ISolution&>(initialSol));


	ALNS_Parameters alnsParam;
	alnsParam.loadXMLParameters("./param.xml");

	CoolingSchedule_Parameters csParam(alnsParam);
	csParam.loadXMLParameters("./param.xml");
	ICoolingSchedule* cs = CoolingScheduleFactory::makeCoolingSchedule(dynamic_cast<ISolution&>(initialSol),csParam);
	SimulatedAnnealing sa(*cs);



	OperatorManager opMan(alnsParam);
	opMan.addDestroyOperator(dynamic_cast<ADestroyOperator&>(randomR));
	opMan.addDestroyOperator(dynamic_cast<ADestroyOperator&>(worstR));
	opMan.addDestroyOperator(dynamic_cast<ADestroyOperator&>(historyR));
	opMan.addRepairOperator(dynamic_cast<ARepairOperator&>(bestI));
	opMan.addRepairOperator(dynamic_cast<ARepairOperator&>(randomI));

	SimpleBestSolutionManager bestSM(alnsParam);
	SimpleLocalSearchManager simpleLsManager(alnsParam);

	TSP_LS ls("My LS");
	TSP_LS lsB("LS FD");

	simpleLsManager.addLocalSearchOperator(dynamic_cast<ILocalSearch&>(ls));
	simpleLsManager.addLocalSearchOperator(dynamic_cast<ILocalSearch&>(lsB));

	ALNS alns("tspExample",dynamic_cast<ISolution&>(initialSol),dynamic_cast<IAcceptanceModule&>(sa),alnsParam,dynamic_cast<AOperatorManager&>(opMan),dynamic_cast<IBestSolutionManager&>(bestSM),dynamic_cast<ILocalSearchManager&>(simpleLsManager));

	alns.addUpdatable(dynamic_cast<IUpdatable&>(historyR));

	alns.solve();

	for(int i = 0; i < CITY_SIZE; i++)
	{
		delete[] distances[i];
	}
	delete[] distances;

	delete cs;

	return 0;
}

////////////////////////
//TSP问题 变邻域搜索求解代码
//基于Berlin52例子求解
//作者：infinitor
//时间：2018-04-12
////////////////////////


#include <iostream>
#include <cmath>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <windows.h>
#include <memory.h>
#include <string.h>
#include <iomanip>
#include <algorithm> 
#define DEBUG

using namespace std;

#define CITY_SIZE 52 //城市数量


//城市坐标
typedef struct candidate
{
	int x;
	int y;
}city, CITIES;

//解决方案
typedef struct Solution
{
	int permutation[CITY_SIZE]; //城市排列
	int cost;						 //该排列对应的总路线长度
}SOLUTION;

//城市排列
int permutation[CITY_SIZE];
//城市坐标数组
CITIES cities[CITY_SIZE];


//berlin52城市坐标，最优解7542好像
CITIES berlin52[CITY_SIZE] =
{ 
{ 565,575 },{ 25,185 },{ 345,750 },{ 945,685 },{ 845,655 },
{ 880,660 },{ 25,230 },{ 525,1000 },{ 580,1175 },{ 650,1130 },{ 1605,620 },
{ 1220,580 },{ 1465,200 },{ 1530,5 },{ 845,680 },{ 725,370 },{ 145,665 },
{ 415,635 },{ 510,875 },{ 560,365 },{ 300,465 },{ 520,585 },{ 480,415 },
{ 835,625 },{ 975,580 },{ 1215,245 },{ 1320,315 },{ 1250,400 },{ 660,180 },
{ 410,250 },{ 420,555 },{ 575,665 },{ 1150,1160 },{ 700,580 },{ 685,595 },
{ 685,610 },{ 770,610 },{ 795,645 },{ 720,635 },{ 760,650 },{ 475,960 },
{ 95,260 },{ 875,920 },{ 700,500 },{ 555,815 },{ 830,485 },{ 1170,65 },
{ 830,610 },{ 605,625 },{ 595,360 },{ 1340,725 },{ 1740,245 } 
};
//优化值
int Delta1[CITY_SIZE][CITY_SIZE] = { 0 };


//计算两个城市间距离
int distance_2city(city c1, city c2)
{
	int distance = 0;
	distance = sqrt((double)((c1.x - c2.x)*(c1.x - c2.x) + (c1.y - c2.y)*(c1.y - c2.y)));

	return distance;
}

//根据产生的城市序列，计算旅游总距离
//所谓城市序列，就是城市先后访问的顺序，比如可以先访问ABC，也可以先访问BAC等等
//访问顺序不同，那么总路线长度也是不同的
//p_perm 城市序列参数
int cost_total(int * cities_permutation, CITIES * cities)
{
	int total_distance = 0;
	int c1, c2;
	//逛一圈，看看最后的总距离是多少
	for (int i = 0; i < CITY_SIZE; i++)
	{
		c1 = cities_permutation[i];
		if (i == CITY_SIZE - 1) //最后一个城市和第一个城市计算距离
		{
			c2 = cities_permutation[0];
		}
		else
		{
			c2 = cities_permutation[i + 1];
		}
		total_distance += distance_2city(cities[c1], cities[c2]);
	}

	return total_distance;
}

//获取随机城市排列
void random_permutation(int * cities_permutation)
{
	int i, r, temp;
	for (i = 0; i < CITY_SIZE; i++)
	{
		cities_permutation[i] = i; //初始化城市排列，初始按顺序排
	}
	
	random_shuffle(cities_permutation, cities_permutation + CITY_SIZE); //随机化排序 

}
//对应two_opt_swap的去重
int calc_delta1(int i, int k, int *tmp, CITIES * cities) {
	int delta = 0;
	/*
	以下计算说明：
	对于每个方案，翻转以后没必要再次重新计算总距离
	只需要在翻转的头尾做个小小处理

	比如：
	有城市序列   1-2-3-4-5 总距离 = d12 + d23 + d34 + d45 + d51 = A
	翻转后的序列 1-4-3-2-5 总距离 = d14 + d43 + d32 + d25 + d51 = B
	由于 dij 与 dji是一样的，所以B也可以表示成 B = A - d12 - d45 + d14 + d25
	下面的优化就是基于这种原理
	*/
	if (i == 0)
	{
		if (k == CITY_SIZE - 1)
		{
			delta = 0;
		}
		else
		{
			delta = 0
				- distance_2city(cities[tmp[k]], cities[tmp[k + 1]])
				+ distance_2city(cities[tmp[i]], cities[tmp[k + 1]])
				- distance_2city(cities[tmp[CITY_SIZE - 1]], cities[tmp[i]])
				+ distance_2city(cities[tmp[CITY_SIZE - 1]], cities[tmp[k]]);
		}

	}
	else
	{
		if (k == CITY_SIZE - 1)
		{
			delta = 0
				- distance_2city(cities[tmp[i - 1]], cities[tmp[i]])
				+ distance_2city(cities[tmp[i - 1]], cities[tmp[k]])
				- distance_2city(cities[tmp[0]], cities[tmp[k]])
				+ distance_2city(cities[tmp[i]], cities[tmp[0]]);
		}
		else
		{
			delta = 0
				- distance_2city(cities[tmp[i - 1]], cities[tmp[i]])
				+ distance_2city(cities[tmp[i - 1]], cities[tmp[k]])
				- distance_2city(cities[tmp[k]], cities[tmp[k + 1]])
				+ distance_2city(cities[tmp[i]], cities[tmp[k + 1]]);
		}
	}

	return delta;
}


/*
去重处理，对于Delta数组来说，对于城市序列1-2-3-4-5-6-7-8-9-10，如果对3-5应用了邻域操作2-opt ， 事实上对于
7-10之间的翻转是不需要重复计算的。 所以用Delta提前预处理一下。

当然由于这里的计算本身是O（1） 的，事实上并没有带来时间复杂度的减少（更新操作反而增加了复杂度）
如果delta计算 是O（n）的，这种去重操作效果是明显的。
*/
//对应two_opt_swap的去重更新
void Update1(int i, int k, int *tmp, CITIES * cities, int Delta[CITY_SIZE][CITY_SIZE]) {
	if (i && k != CITY_SIZE - 1) {
		i--; k++;
		for (int j = i; j <= k; j++) {
			for (int l = j + 1; l < CITY_SIZE; l++) {
				Delta[j][l] = calc_delta1(j, l, tmp, cities);
			}
		}

		for (int j = 0; j < k; j++) {
			for (int l = i; l <= k; l++) {
				if (j >= l) continue;
				Delta[j][l] = calc_delta1(j, l, tmp, cities);
			}
		}
	}// 如果不是边界，更新(i-1, k + 1)之间的 
	else {
		for (i = 0; i < CITY_SIZE - 1; i++)
		{
			for (k = i + 1; k < CITY_SIZE; k++)
			{
				Delta[i][k] = calc_delta1(i, k, tmp, cities);
			}
		}
	}// 边界要特殊更新 

}


// two_opt_swap算子 
void two_opt_swap(int *cities_permutation, int b, int c) 
{
	vector<int> v;
	for (int i = 0; i < b; i++) 
	{
		v.push_back(cities_permutation[i]);
	}
	for (int i = c; i >= b; i--) 
	{
		v.push_back(cities_permutation[i]);
	}
	for (int i = c + 1; i < CITY_SIZE; i++) 
	{
		v.push_back(cities_permutation[i]);
	}

	for (int i = 0; i < CITY_SIZE; i++)
	{
		cities_permutation[i] = v[i];
	}

}

//邻域结构1 使用two_opt_swap算子
void neighborhood_one(SOLUTION & solution, CITIES *cities)
{
	int i, k, count = 0;
	int max_no_improve = 60;

	int inital_cost = solution.cost; //初始花费
	int now_cost = 0;

	//SOLUTION current_solution = solution;

	for (int i = 0; i < CITY_SIZE - 1; i++)
	{
		for (k = i + 1; k < CITY_SIZE; k++)
		{
			Delta1[i][k] = calc_delta1(i, k, solution.permutation, cities);
		}
	}

	do 
	{
		count++;
		for (i = 0; i < CITY_SIZE - 1; i++)
		{
			for (k = i + 1; k < CITY_SIZE; k++)
			{
				if (Delta1[i][k] < 0)
				{
					//current_solution = solution;
					two_opt_swap(solution.permutation, i, k);

					now_cost = inital_cost + Delta1[i][k];
					solution.cost = now_cost;

					inital_cost = solution.cost;

					Update1(i, k, solution.permutation, cities, Delta1);

					count = 0; //count复位
					
				}

			 }
		  }
	}while (count <= max_no_improve);

}

//two_h_opt_swap的去重
int calc_delta2(int i, int k, int *cities_permutation, CITIES * cities)
{
	int delta = 0;
	if (i == 0)
	{
		if ( k == i+1)
		{
			delta = 0;
		}
		else if ( k == CITY_SIZE -1)
		{
			delta = 0
				- distance_2city(cities[cities_permutation[i]], cities[cities_permutation[i + 1]])
				- distance_2city(cities[cities_permutation[k]], cities[cities_permutation[k - 1]])
				+ distance_2city(cities[cities_permutation[k]], cities[cities_permutation[i+1]])
				+ distance_2city(cities[cities_permutation[k - 1]], cities[cities_permutation[i]]);
		}
		else
		{
			delta = 0
				- distance_2city(cities[cities_permutation[i]], cities[cities_permutation[i + 1]])
				- distance_2city(cities[cities_permutation[k]], cities[cities_permutation[k - 1]])
				- distance_2city(cities[cities_permutation[k]], cities[cities_permutation[k + 1]])
				+ distance_2city(cities[cities_permutation[k - 1]], cities[cities_permutation[k + 1]])
				+ distance_2city(cities[cities_permutation[i]], cities[cities_permutation[k]])
				+ distance_2city(cities[cities_permutation[k]], cities[cities_permutation[i + 1]]);
		}
	}
	else
	{
		if (k == i + 1)
		{
			delta = 0;
		}
		else if ( k == CITY_SIZE - 1)
		{
			delta = 0
				- distance_2city(cities[cities_permutation[i]], cities[cities_permutation[i + 1]])
				- distance_2city(cities[cities_permutation[0]], cities[cities_permutation[k]])
				- distance_2city(cities[cities_permutation[k]], cities[cities_permutation[k-1]])
				+ distance_2city(cities[cities_permutation[k]], cities[cities_permutation[i + 1]])
				+ distance_2city(cities[cities_permutation[k-1]], cities[cities_permutation[0]])
				+ distance_2city(cities[cities_permutation[i]], cities[cities_permutation[k]]);
		}
		else
		{
			delta = 0
				- distance_2city(cities[cities_permutation[i]], cities[cities_permutation[i + 1]])
				- distance_2city(cities[cities_permutation[k]], cities[cities_permutation[k + 1]])
				- distance_2city(cities[cities_permutation[k]], cities[cities_permutation[k - 1]])
				+ distance_2city(cities[cities_permutation[i]], cities[cities_permutation[k]])
				+ distance_2city(cities[cities_permutation[k]], cities[cities_permutation[i + 1]])
				+ distance_2city(cities[cities_permutation[k - 1]], cities[cities_permutation[k + 1]]);

		}
	}

	return delta;

}



//two_h_opt_swap算子
void two_h_opt_swap(int *cities_permutation, int a, int d) 
{
	int n = CITY_SIZE;
	vector<int> v;
	v.push_back(cities_permutation[a]);
	v.push_back(cities_permutation[d]);
	// i = 1 to account for a already added
	for (int i = 1; i < n; i++) 
	{
		int idx = (a + i) % n;
		// Ignore d which has been added already
		if (idx != d) 
		{
			v.push_back(cities_permutation[idx]);
		}
	}

	for (int i = 0; i < v.size(); i++)
	{
		cities_permutation[i] = v[i];
	}

}


//邻域结构2 使用two_h_opt_swap算子
void neighborhood_two(SOLUTION & solution, CITIES *cities)
{
	int i, k, count = 0;
	int max_no_improve = 60;
	int inital_cost = solution.cost; //初始花费
	int now_cost = 0;
	int delta = 0;

	do
	{
		count++;
		for (i = 0; i < CITY_SIZE - 1; i++)
		{
			for (k = i + 1; k < CITY_SIZE; k++)
			{
				
				delta = calc_delta2(i, k, solution.permutation, cities);

				if (delta < 0)
				{
					//cout<<"delta = " <<delta<<endl; 

					two_h_opt_swap(solution.permutation, i, k);

					now_cost = inital_cost + delta;
					solution.cost = now_cost;

					inital_cost = solution.cost;

					count = 0; //count复位
				}
			}
		}
	} while (count <= max_no_improve);
}


//VND
//best_solution最优解
//current_solution当前解
void variable_neighborhood_descent(SOLUTION & solution, CITIES * cities)
{

	SOLUTION current_solution = solution;
	int l = 1;
	cout  <<"=====================VariableNeighborhoodDescent=====================" << endl;
	while(true)
	{
		switch (l)
		{
		case 1:
			neighborhood_one(current_solution, cities);
			cout << setw(45) << setiosflags(ios::left)  <<"Now in neighborhood_one , current_solution = " << current_solution.cost << setw(10) << setiosflags(ios::left) << "  solution = " << solution.cost << endl;
			if (current_solution.cost < solution.cost)
			{
				solution = current_solution;
				l = 0;
			}
			break;
		case 2:
			neighborhood_two(current_solution, cities);
			cout << setw(45) << setiosflags(ios::left) << "Now in neighborhood_two , current_solution = " << current_solution.cost << setw(10) << setiosflags(ios::left) << "  solution = " << solution.cost << endl;
			if (current_solution.cost < solution.cost)
			{
				solution = current_solution;
				l = 0;
			}
			break;

		default:
			return;
		}
		l++;

	}

}

//将城市序列分成4块，然后按块重新打乱顺序。
//用于扰动函数
void double_bridge_move(int * cities_permutation)
{
	int pos1 = 1 + rand() % (CITY_SIZE / 4);
	int pos2 = pos1 + 1 + rand() % (CITY_SIZE / 4);
	int pos3 = pos2 + 1 + rand() % (CITY_SIZE / 4);

	int i;
	vector<int> v;
	//第一块
	for (i = 0; i < pos1; i++)
	{
		v.push_back(cities_permutation[i]);
	}

	//第二块
	for (i = pos3; i < CITY_SIZE; i++)
	{
		v.push_back(cities_permutation[i]);
	}
	//第三块
	for (i = pos2; i < pos3; i++)
	{
		v.push_back(cities_permutation[i]);
	}

	//第四块
	for (i = pos1; i < pos2; i++)
	{
		v.push_back(cities_permutation[i]);
	}


	for (i = 0; i < (int)v.size(); i++)
	{
		cities_permutation[i] = v[i];
	}


}

//抖动
void shaking(SOLUTION &solution, CITIES *cities)
{
	double_bridge_move(solution.permutation);
	solution.cost = cost_total(solution.permutation, cities);
}


void variable_neighborhood_search(SOLUTION & best_solution, CITIES * cities)
{

	int max_iterations = 5;

	int count = 0, it = 0;

	SOLUTION current_solution = best_solution;

	//算法开始
	do 
	{
		cout << endl << "\t\tAlgorithm VNS iterated  " << it+1 << "  times" << endl;
		count++;
		it++;
		shaking(current_solution, cities);

		variable_neighborhood_descent(current_solution, cities); 

		if (current_solution.cost < best_solution.cost)
		{
			best_solution = current_solution;
			count = 0;
		}

		cout << "\t\t全局best_solution = " << best_solution.cost << endl;

	} while (count <= max_iterations);


}


int main()
{

	srand((unsigned) time(0));

	SOLUTION best_solution;

	random_permutation(best_solution.permutation);
	best_solution.cost = cost_total(best_solution.permutation, berlin52);

	cout << "初始总路线长度 = " << best_solution.cost << endl;

	variable_neighborhood_search(best_solution, berlin52);

	cout << endl << endl << "搜索完成！ 最优路线总长度 = " << best_solution.cost << endl;
	cout << "最优访问城市序列如下：" << endl;
	for (int i = 0; i < CITY_SIZE; i++)
	{
		cout << setw(4) << setiosflags(ios::left) << best_solution.permutation[i];
	}

	cout << endl << endl;

	return 0;
}

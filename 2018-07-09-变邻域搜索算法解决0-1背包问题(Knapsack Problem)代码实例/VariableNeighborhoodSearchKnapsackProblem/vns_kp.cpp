#include <iostream>
#include <vector>
#include <ctime>
#include <iomanip>
using namespace std;

// 物品的数量 每一个物品有0和1两种选择 0代表选择当前物品 1代表不选择当前物品
const int n = 100;

//算法最大迭代次数
const int Max_Iteration = 1000;

//邻域数量
const int MaxFlip = 3;
int flip = 1;


//背包最大容量
const int maxWeight = 5 * n;

//记录已经检查的背包数量
int solutionsChecked = 0;

//物品对应价值&&重量
int values[n] = { 0 };
int weights[n] = { 0 };

//随机数种子
const int seed = 5113; //2971


/************************************************************************/
/* 
	解决方案类：

*/
/************************************************************************/

typedef struct Knapsack_Problem_Solution
{
	int selection[n] = { 0 };  //当前方案的物品选择情况 selection[i] == 0 or 1 <==> 不选择 or 选择 第i个物品
	int total_values = 0;      //当前方案下物品总价值
	int total_weights = 0;    //当前方案下物品总重量
}KP_Solution;

//对selection[n]进行评价，计算total_values和total_weights
void Evaluate_Solution(KP_Solution & x)
{
	x.total_values = 0;
	x.total_weights = 0;
	for (int i = 0; i < n; i++)
	{
		x.total_values += x.selection[i] * values[i];
		x.total_weights += x.selection[i] * weights[i];
	}

	if (x.total_weights > maxWeight)
	{
		x.total_values = maxWeight - x.total_weights; //超过背包最大容纳重量，价值设置为负数
	}

}


//邻居解集合
vector<KP_Solution> nbrhood;

void MySwap(int &a, int &b)
{
	int temp = a;
	a = b;
	b = temp;
}

//利用邻域动作生成邻居解
void neighborhood(KP_Solution &x, int flip)
{
	//邻域动作1
	if (flip == 1)
	{
		nbrhood.clear();
		for (int i = 0; i < n; i++)
		{
			nbrhood.push_back(x);
			if (nbrhood[i].selection[i] == 1)
			{
				nbrhood[i].selection[i] = 0;
			}
			else
			{
				nbrhood[i].selection[i] = 1;
			}
		}
	}
	//邻域动作2
	else if (flip == 2)
	{
		nbrhood.clear();
		int a = -1;
		for (int i = 0; i < n; i++)
		{
			for (int j = i; j < n; j++)
			{
				if (i != j)
				{
					a += 1;
					nbrhood.push_back(x);

					if (nbrhood[a].selection[i] == 1)
					{
						nbrhood[a].selection[i] = 0;
					}
					else
					{
						nbrhood[a].selection[i] = 1;
					}

					if (nbrhood[a].selection[j] == 1)
					{
						nbrhood[a].selection[j] = 0;
					}
					else
					{
						nbrhood[a].selection[j] = 1;
					}

				}
			}
		}
	}
	//邻域动作3
	else
	{
		nbrhood.clear();
		for (int i = 0; i < n; i++)
		{
			nbrhood.push_back(x);
			if ( i < 3)
			{
				MySwap(nbrhood[i].selection[i], nbrhood[i].selection[n + i - 3]);
			}
			else
			{
				MySwap(nbrhood[i].selection[i], nbrhood[i].selection[i - 3]);
			}
		}
	}


}
//随机生成价值和重量
void Rand_Value_Weight()
{
	for (int i = 0; i < n; i++)
	{
		values[i] = rand() % 90 + 10; // 10 - 100
		weights[i] = rand() % 15 + 5; // 5 - 20
	}
}

//随机生成解决方案
void Random_Solution(KP_Solution &x)
{
	x.total_values = 0;
	x.total_weights = 0;
	for (int i = 0; i < n; i++)
	{
		double rate = (rand() % 100) / 100.0;
		if ( rate < 0.8 )
		{
			x.selection[i] = 0;
		}
		else
		{
			x.selection[i] = 1;
		}
	}
}

void Variable_Neighborhood_Descent(KP_Solution &x)
{
	int flip = 1;
	KP_Solution x_curr;
	while ( flip < MaxFlip + 1)
	{
		neighborhood(x, flip);
		x_curr = nbrhood[0];
		Evaluate_Solution(x_curr);

		for(unsigned int i = 1; i < nbrhood.size(); i++)
		{
			solutionsChecked += 1;

			Evaluate_Solution(nbrhood[i]);

			if (nbrhood[i].total_values > x_curr.total_values)
			{
				x_curr = nbrhood[i];
			}
		}
		//邻域复位
		if (x_curr.total_weights > x.total_weights)
		{
			x = x_curr;
			flip = 1;
		}
		else
		{
			flip += 1;
		}
	}
}




void Shaking_Procdure(KP_Solution &x)
{

	int num = rand() % (n / 10) + 3; // 3 - 8
	for (int i = 0; i < num; i++)
	{
		int pos = rand() % n;
		if (x.selection[i] == 0)
		{
			x.selection[i] = 1;
		}
		else
		{
			x.selection[i] = 0;
		}
	}

	Evaluate_Solution(x);
}

void Variable_Neighborhood_Search(KP_Solution &x, int iteration)
{
	KP_Solution best = x;
	Variable_Neighborhood_Descent(best);
	for (int i = 0; i < iteration; i++)
	{
		Shaking_Procdure(x);

		Variable_Neighborhood_Descent(x);
		if (best.total_values < x.total_values)
		{
			best = x;
		}
	}

	x = best;
}

int main()
{
	srand(seed);

	KP_Solution kpx;

	Rand_Value_Weight();

	Random_Solution(kpx);

	Variable_Neighborhood_Search(kpx, Max_Iteration);

	cout << "石头重量为：" << endl;

	for (int i = 0; i < n; i++)
	{
		cout << setw(2) <<weights[i] << "  ";
		if ((i + 1) % 25 == 0)
		{
			cout << endl;
		}
	}

	cout << "\n石头价值为：" << endl;

	for (int i = 0; i < n; i++)
	{
		cout << values[i] << "  ";
		if ((i + 1) % 25 == 0)
		{
			cout << endl;
		}
	}

	cout << endl << "最终结果: 已检查的总方案数 = " << solutionsChecked << endl;
	cout << "背包最大容量为:" << maxWeight << endl;
	cout << "找到最大价值为: " << kpx.total_values << endl;
	cout << "背包当前重量为: " << kpx.total_weights << endl;

	for (int i = 0; i < n; i++)
	{
		cout << kpx.selection[i] << "  ";
		if ((i+1) % 25 == 0)
		{
			cout << endl;
		}
	}

	return 0;
}
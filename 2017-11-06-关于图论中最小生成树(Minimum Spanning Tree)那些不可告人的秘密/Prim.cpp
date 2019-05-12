#include<iostream>
#define NO 99999999   //99999999代表两点之间不可达
#define N 5
using namespace std;

bool visit[N];
long long money[N] = { 0 };
long long graph[N][N] = {0};

void initgraph()
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			scanf(" %lld", &graph[i][j]);
		}
	}
	
}

void printgraph()
{
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			printf(" %lld", graph[i][j]);
		}
	}

}

long long prim(int city)
{
	initgraph();
	int index = city;
	long long  sum = 0;
	int i = 0;
	int j = 0;
	cout <<"访问节点：" <<index << "\n";
	memset(visit, false, sizeof(visit));
	visit[city] = true;
	for (i = 0; i < N; i++)
	{
		money[i] = graph[city][i];//初始化，每个与城市city间相连的费用存入money，以便后续比较
	}
		
	for (i = 1; i < N; i++)
	{
		long long  minor = NO;
		for (j = 0; j < N; j++)
		{
			if ((visit[j] == false) && money[j] < minor)  //找到未访问的城市中，与当前最小生成树中的城市间费用最小的城市
			{
				minor = money[j];
				index = j;
			}
		}
		visit[index] = true;
		cout << "访问节点：" << index << "\n";
		sum += minor; //求总的最低费用
		/*这里是一个更新，如果未访问城市与当前城市间的费用更低，就更新money,保存更低的费用*/
		for (j = 0; j < N; j++)
		{
			if ((visit[j] == false) && money[j]>graph[index][j])
			{
				money[j] = graph[index][j];
			}
		}
	}
	cout << endl;
	return sum;               //返回总费用最小值
}
int main()
{
	cout << "修路最低总费用为："<< prim(0) << endl;//从城市0开始

	system("pause");
	return 0;
}
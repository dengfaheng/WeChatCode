//prime算法

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

int prim(int city)
{
	initgraph();
	printgraph();
	int index = city;
	int sum = 0;
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
		int minor = NO;
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
	return 0;
}



//kruskal算法

#include<cstdio>
#include<iostream>
#include<cstring>
#include<cstdlib>
#include<algorithm>
#include<cmath>
#include<map>
#include<set>
#include<list>
#include<vector>
using namespace std;
#define N 10005
#define M 50005
#define qm 100005
#define INF 2147483647
struct arr{
	int ff, tt, ww;
}c[M << 1];// 存储边的集合，ff，tt，ww为一条从ff连接到tt的权值为ww的边 
int tot = 0;//边的总数 
int ans = 0;//最小生成树的权值和 
int f[N];//并查集 
bool comp(const arr & a, const arr & b){
	return a.ww < b.ww;
}
int m, n;//边数量，点数量 
int getfa(int x){
	return f[x] == x ? x : f[x] = getfa(f[x]);
}//并查集，带路径压缩 
 
inline void add(int x, int y, int z){
	c[++tot].ff = x;
	c[tot].tt = y;
	c[tot].ww = z;
	return;
}//新增一条边 

void kruscal(){
	for (int i = 1; i <= n; i ++) f[i] = i;
	for (int i = 1; i <= m; i ++){
		int fx = getfa(c[i].ff);//寻找祖先 
		int fy = getfa(c[i].tt);
		if (fx != fy){//不在一个集合，合并加入一条边 
			f[fx] = fy;
			ans += c[i].ww;
		}
	}

	return;
} 
int main(){
	freopen("input10.txt", "r", stdin);
	freopen("output10.txt", "w", stdout);
	scanf("%d%d",&n, &m);
	int x, y, z;
	for (int i = 1; i <= m; i ++){
		scanf("%d %d %d", &x, &y, &z);
		add(x, y, z);
	}
	sort(c + 1, c + 1 + m, comp);//快速排序 
	kruscal();
	printf("%d\n", ans);
	return 0;
}
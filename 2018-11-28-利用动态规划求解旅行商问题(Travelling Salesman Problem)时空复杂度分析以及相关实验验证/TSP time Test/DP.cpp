#include<bits/stdc++.h>
using namespace std;
// const
const int INF = 0x3f3f3f3f;
#define sqr(x) ((x)*(x))
//variables
string file_name;
int type;// type == 1 full martix, type == 2 EUC_2D type == 3 GEO 4 EXL
int s;
long long N;//number of nodes
int init_point;
double **dp;//the dynamic optimize function
double **dis;//distance between two cities
double ans;
// the struct
struct vertex{
	double x, y;// coord of vertex
	int id;// the id of vertex 
	
	int input(FILE *fp){
		return fscanf(fp, "%d %lf %lf", &id, &x, &y);
	}
	
}*node;

double EUC_2D(const vertex &a, const vertex &b){
	return sqrt(sqr(a.x - b.x) + sqr(a.y - b.y));
} 

void io(FILE *fp){//the input
	fscanf(fp, "%lld", &N);
	

	
	node = new vertex[N + 5];
	dis = new double*[N + 5];
	
	for (int i = 0; i < N; i ++){
		node[i].input(fp);
	}
	for (int i = 0; i < N; i ++){
		dis[i] = new double[N];
		for (int j = 0; j < N; j ++)
				dis[i][j] = EUC_2D(node[i], node[j]);// compute distance
	}

	fclose(fp);
	return;
}


void init(){// initilization

    dp = new double*[(1 << N) + 5];
    
    if (dp == NULL){
    	printf("error!");
    	assert(dp != NULL);
    } 
    for(int i = 0; i < (1 << N); i++){
    	dp[i] = new double[N + 5];
    	for(int j = 0; j < N; j++)
        	dp[i][j] = INF;//initilize the dp function
    }
    ans = INF;
    return;
}

double slove(){
	long long M = (1 << N);//The set of all solutions
	dp[1][0] = 0;
	for (int i = 1; i < M; i ++){
		for (int j = 1; j < N; j ++){
			if (i & (1 << j)) continue;// j has been in set i
			if (!(i & 1)) continue;// the start city is in the set i
			for (int k = 0; k < N; k ++){
				if (i & (1 << k)){// node k has been visited, or rather, k is in the set i
					dp[(1 << j) | i][j] = min(dp[(1 << j) | i][j], dp[i][k] + dis[k][j]);// dp
				}// add a node j to set i
			}
		}
	}
	for (int i = 0; i < N; i ++)
		ans = min(dp[M - 1][i] + dis[i][0], ans);// select the best solution
	return ans;
}

int main(int argc, char *argv[]){
	FILE *fp = fopen(argv[1], "r");
	io(fp);
	init();
	FILE *timer = fopen("time.csv", "a");
	double tic = clock();
	printf("%.2lf\n", slove());
	fprintf(timer, "%d, %.10f\n", N, (clock() - tic) / CLOCKS_PER_SEC);
	system("wmic process where name=\"DP.exe\" get WorkingSetSize");
	delete[] dp;
	delete[] node;
	delete[] dis;
	fclose(fp);
	return 0;
} 


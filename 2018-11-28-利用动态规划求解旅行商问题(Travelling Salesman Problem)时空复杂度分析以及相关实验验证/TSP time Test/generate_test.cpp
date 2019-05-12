#include<bits/stdc++.h>
using namespace std;

int main(){
	int n;
	FILE *fp;
	FILE *cmd = fopen("test.bat", "w");
	char str[30];
	for (n = 3; n <= 33; n ++){
		
		sprintf(str, "%d.txt", n);
		fp = fopen(str, "w");
		fprintf(cmd, ".\\DP.exe %d.txt\n", n);
		fprintf(fp, "%d\n", n);
		
		for (int i = 0; i < n; i ++){
			fprintf(fp, "%d %d %d\n", i, rand() % 100, rand() % 100);
		} 
		fclose(fp);
	}
	fprintf(cmd, "pause\n");
} 

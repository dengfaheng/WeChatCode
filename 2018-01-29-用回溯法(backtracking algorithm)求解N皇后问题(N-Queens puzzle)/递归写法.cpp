#include <stdio.h>
#include <stdlib.h>

const int N=20;   //最多放皇后的个数
int q[N];         //i表示皇后所在的行号，
				  //q[i]表示皇后所在的列号
int cont = 0;     //统计解的个数
//输出一个解
void print(int n)
{
    int i,j;
    cont++;
    printf("第%d个解：",cont);
    for(i=1;i<=n;i++)
        printf("(%d,%d) ",i,q[i]);
    printf("\n");
    for(i=1;i<=n;i++)        //行
    {
        for(j=1;j<=n;j++)    //列
        {
            if(q[i]!=j)
                printf("x ");
            else
                printf("Q ");
        }
        printf("\n");
    }
}
//检验第i行的k列上是否可以摆放皇后
int find(int i,int k)
{
    int j=1;
    while(j<i)  //j=1~i-1是已经放置了皇后的行
    {
        //第j行的皇后是否在k列或(j,q[j])与(i,k)是否在斜线上
        if(q[j]==k || abs(j-i)==abs(q[j]-k))
            return 0;
        j++;
    }
    return 1;
}
//放置皇后到棋盘上
void place(int k,int n)
{
    int j;
    if(k>n)
        print(n); //递归出口
    else
    {
        for(j=1;j<=n;j++)   //试探第k行的每一个列
        {
            if(find(k,j))
            {
                q[k] = j;   //保存位置
                place(k+1,n);  //接着下一行
            }
        }
    }
}
int main1111(void)
{
    int n;
    printf("请输入皇后的个数(n<=20),n=:");
    scanf("%d",&n);
    if(n>20)
        printf("n值太大，不能求解!\n");
    else
    {
        printf("%d皇后问题求解如下(每列的皇后所在的行数):\n",n);
        place(1,n);        //问题从最初状态解起
        printf("\n");
    }
    system("pause");
    return 0;
}

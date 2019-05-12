

//首先来看函数主体，实现了分割的部分
void mergesort(int a[],int n,int left,int right)
{
    if(left+1<right)
    {
        int mid=(left+right)/2;
        mergesort(a,n,left,mid);
        mergesort(a,n,mid,right);
        merge(a,n,left,mid,right);
    }
}
```
```
//下面来写自己的merge函数！
//L,R是辅助数组！
void merge(int a[],int n,int left,int mid,int right)
{
    int n1=mid-left,n2=right-mid;
    for(int i=0;i<n1;i++)
        L[i]=a[left+i];//储存左数列
    for(int i=0;i<n2;i++)
        R[i]=a[mid+i];//储存右数列
    L[n1]=R[n2]=INF;
    int i=0,j=0;
    for(int k=left;k<right;k++)//合并
    {
        if(L[i]<=R[j])
            a[k]=L[i++];
        else
            a[k]=R[j++];
    }
}

#include<bits/stdc++>
using namespace std;
const int maxn=500000,INF=0x3f3f3f3f;
int L[maxn],R[maxn];
void merge(int a[],int n,int left,int mid,int right)
{
    int n1=mid-left,n2=right-mid;
    for(int i=0;i<n1;i++)
        L[i]=a[left+i];
    for(int i=0;i<n2;i++)
        R[i]=a[mid+i];
    L[n1]=R[n2]=INF;
    int i=0,j=0;
    for(int k=left;k<right;k++)
    {
        if(L[i]<=R[j])
            a[k]=L[i++];
        else
            a[k]=R[j++];
    }
}
void mergesort(int a[],int n,int left,int right)
{
    if(left+1<right)
    {
        int mid=(left+right)/2;
        mergesort(a,n,left,mid);
        mergesort(a,n,mid,right);
        merge(a,n,left,mid,right);
    }
}
int main()
{
    int a[maxn],n;
    cin>>n;
    for(int i=0;i<n;i++)
        cin>>a[i];
    mergesort(a,n,0,n);
    for(int i=0;i<n;i++)
    {
        if(i)
            cout<<" ";
        cout<<a[i];
    }
    cout<<endl;
    return 0;
}




int maxsum(int l,int r)
{
    if(l==r)return a[l];
    int m=(l+r)/2;
    int Max=max(maxsum(l,m),maxsum(m+1,r));//（分解）情况1：完全在左区间，或者完全在右区间

    //（合并）情况2：横跨左右两个区间
    int suml=a[m],t=0;
    for(int i=m;i>=l;i--)
        suml=max(suml,t+=a[i]);
    int sumr=a[m+1];t=0;
    for(int i=m+1;i<=r;i++)
        sumr=max(sumr,t+=a[i]);

    return max(Max,suml+sumr);//取两种情况中最大的。


}






#include <iostream> 
void move(int,char,char,char); 
int main()
{    
     int m;
     cin>>m; 
     move(m,'A','B','C');
} 
/*XYZ分别代表三根柱子*/

void move(int m,char x,char y,char z)
{    
    if(m==1)    
    {        
        cout<<x<<"->"<<z<<endl"; //如果只有一个金片了，那么我们直接将其从X移至Z即可。        
        return;    
    }    
    move(m-1,x,z,y); //先将m-1个金片从X移至Y。    
    cout<<x<<"->"<<z<<endl);//每次递归时，我们总是将1号金片移至Z。    
    move(m-1,y,x,z); //再将整体代换的m-1个金片从Y移至Z
}

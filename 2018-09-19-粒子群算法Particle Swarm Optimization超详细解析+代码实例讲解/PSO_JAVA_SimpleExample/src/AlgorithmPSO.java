public class AlgorithmPSO {
	int n=2; //粒子个数，这里为了方便演示，我们只取两个，观察其运动方向
    double[] y;
    double[] x;
    double[] v;
    double c1=2;
    double c2=2;
    double pbest[];
    double gbest;
    double vmax=0.1; //速度最大值
    //适应度计算函数，每个粒子都有它的适应度
    public void fitnessFunction(){
        for(int i=0;i<n;i++){
            y[i]=-1*x[i]*(x[i]-2);
        }
    }
    public void init(){ //初始化
        x=new double[n];
        v=new double[n];
        y=new double[n];
        pbest=new double[n];
        /***
         * 本来是应该随机产生的，为了方便演示，我这里手动随机落两个点，分别落在最大值两边
         */
        x[0]=0.0;
        x[1]=2.0;
        v[0]=0.01;
        v[1]=0.02;
        fitnessFunction();
        //初始化当前个体最优位置，并找到群体最优位置
        for(int i=0;i<n;i++){
            pbest[i]=y[i];
            if(y[i]>gbest) gbest=y[i];
        }
        System.out.println("算法开始，起始最优解:"+gbest);
        System.out.print("\n");
    }
    public double getMAX(double a,double b){
        return a>b?a:b;
    }
    //粒子群算法
    public void PSO(int max){
        for(int i=0;i<max;i++){
            double w=0.4;
            for(int j=0;j<n;j++){
                //更新位置和速度，下面就是我们之前重点讲解的两条公式。
                v[j]=w*v[j]+c1*Math.random()*(pbest[j]-x[j])+c2*Math.random()*(gbest-x[j]);
                if(v[j]>vmax) v[j]=vmax;//控制速度不超过最大值
                x[j]+=v[j];
                
                //越界判断，范围限定在[0, 2]
                if(x[j]>2) x[j]=2;
                if(x[j]<0) x[j]=0;
                
            }
            fitnessFunction();
            //更新个体极值和群体极值
            for(int j=0;j<n;j++){
                pbest[j]=getMAX(y[j],pbest[j]);
                if(pbest[j]>gbest) gbest=pbest[j];
                System.out.println("粒子n"+j+": x = "+x[j]+"  "+"v = "+v[j]);
            }
            System.out.println("第"+(i+1)+"次迭代，全局最优解 gbest = "+gbest);
            System.out.print("\n");
        }
        
    }
    //运行我们的算法
    public static void main(String[] args){
    	//AlgorithmPSO ts=new AlgorithmPSO();
       // ts.init();
        //ts.PSO(100);//为了方便演示，我们暂时迭代10次。
        
        datatest.a = 10;
        
        System.out.println("a = "+datatest.a);
        
        datatest.a = 2;
        
        System.out.println("a = "+datatest.a);
        
    }

}

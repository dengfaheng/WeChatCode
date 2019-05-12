package AFAS_PACK;

import java.io.IOException;

public class Fish {
    public int dim;                   //每条鱼的维度
    public int[] x;                //每条鱼的具体多维坐标
    public double fit;                //鱼的适应值，浓度
    public int visaul;             //每条鱼的视野
    public final double[] H = new double[256];
    public final double[] W = new double[256];
    
 
    public Fish(int dim, int visaul) throws IOException {
        super();
        this.dim = dim;
        this.visaul = visaul;
        x = new int[dim];
        for(int i=0;i<dim;i++)
            x[i] = (int) Math.floor(256*Math.random());
        fit = 0;
        //init();
    }
    /*getfit = newfunction(this.x[0],this.x[1]);*/
    
 
    public double distance(Fish f)
    {
        double a = 0;
        for(int i=0;i<dim;i++)
        {
            if(this.x[i]-f.x[i]==0)
                a = 0.00001;
            else 
                a += (this.x[i]-f.x[i])*(this.x[i]-f.x[i]);
        }
        return Math.sqrt(a);
    }
    
    
    public  double newfunction(int[] w) throws IOException {          
    
        return -(w[0]*w[0]-160*w[0]+640+w[1]*w[1]-260*w[1]+16900);
        
    }
}
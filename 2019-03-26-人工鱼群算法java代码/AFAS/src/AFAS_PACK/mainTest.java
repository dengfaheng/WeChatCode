package AFAS_PACK;

import java.io.IOException;

public class mainTest {
 
    /**
     * @param args
     * @throws IOException
     * @author sun 
     */
    public static void main(String[] args) throws IOException {
        //int fishNum, int tryTime, int dim, double step, double delta, double visual   
        System.out.println("begin");
        AFAS run = new AFAS(10,5,2,5,0.2,10);
        run.doAFAS(40 );//括号内为迭代次数
    }
 
}
package lpsolve_test;
import lpsolve.*;
import java.util.Arrays;


public class Optimator {
	private static LpSolve problem;
	/**
	 * 求解整数规划问题
	 * @param goal			目标函数矩阵，由于LpSolve读取数组时从下标1开始读取，数据需从下标1开始填充，0-1的放前面，有上限的放后面
	 * @param stIeMatrix	不等式约束方程矩阵，由于LpSolve读取数组时从下标1开始读取，内层数据需从下标1开始填充
	 * @param stEqMatrix	等式约束方程矩阵，由于LpSolve读取数组时从下标1开始读取，内层数据需从下标1开始填充
	 * @param stIeRest		不等式约束条件矩阵，每次传入的是单个数字，不需要从1开始填充
	 * @param stEqRest		等式约束条件矩阵，每次传入的是单个数字，不需要从1开始填充
	 * @param ups			上限约束矩阵
	 */
	public static void optimate(double[] goal,double[][] stIeMatrix,double[][] stEqMatrix,
			double[] stIeRest,double[] stEqRest,double[] ups) throws LpSolveException{

		//1、创建LpSolve对象
		problem = LpSolve.makeLp(0, goal.length-1);
		//2、添加目标函数，会从下标1开始读取!下标1的参数会被忽略
		problem.setObjFn(goal);
		
		//3、循环添加不等式约束，外层循环一次代表一个不等式
		if(stIeMatrix!=null){
			for(int i=0;i<stIeMatrix.length;i++){
				//同样数组的读取会从下标1开始
				problem.addConstraint(stIeMatrix[i], LpSolve.LE, stIeRest[i]);
			}
		}
		
		//4、循环添加等式约束，外层循环一次代表一个等式
		if(stEqMatrix!=null){
			for(int i=0;i<stEqMatrix.length;i++){
				//同样数组的读取会从下标1开始
				problem.addConstraint(stEqMatrix[i], LpSolve.EQ, stEqRest[i]);
			}
		}
		
		//5、设置参数的整数约束，1代表第一个参数
		for(int i=1;i<goal.length;i++){
			problem.setInt(i, true);
		}
		
		//6、设置指定参数的上限值
		for(int i=1;i<=ups.length;i++){
			problem.setUpbo(i, ups[i-1]);
		}
		
		problem.printLp();
		//求解
		problem.solve();
	}
	/**
	 * 得到最优解
	 * @return
	 * @throws LpSolveException
	 */
	public static double getObjective() throws LpSolveException{
		if(problem!=null){
			return problem.getWorkingObjective();
		}else{
			try {
				throw new Exception("还没有进行求解！");
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} 
			return 0;
		}
	}
	/**
	 * 得到最优解对应的变量
	 * @return
	 * @throws LpSolveException
	 */
	public static double[] getVariables() throws LpSolveException{
		if(problem!=null){
			return problem.getPtrVariables();
		}else{
			try {
				throw new Exception("还没有进行求解！");
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			} 
			return null;
		}
	}
	
//测试
	public static void main(String[] args) {
		try {
			double[][] stIeMatrix = new double[1][];
			stIeMatrix[0] = new double[]{0,1,2,3,4};
			double[] stRest = new double[]{1};
			optimate(new double[]{0,1,2,3,4},stIeMatrix,stIeMatrix,stRest,stRest,new double[]{1,1,5,6});
			System.out.println(getObjective());
			System.out.println(Arrays.toString(getVariables()));
		} catch (LpSolveException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
}

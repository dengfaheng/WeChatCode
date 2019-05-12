package com.dfh.holtwinters;

import java.util.ArrayList;
import java.util.List;


/**
 * 三次指数平滑预测，最多外推一个周期
 * @author Administrator
 *
 */
public class HoltWintersTripleExponentialImpl {
	
	
	static double[] alphas=new double[]{0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9};
	
	public static Double getbetterforecast(List<Integer> y, int period, int m) {
		double[] result=betterforecast(y,period,m);
		return result[result.length-1];
	}
	
	/**
	 * 求出最佳预测
	 * @param y
	 * @param alpha
	 * @param beta
	 * @param gamma
	 * @param period
	 * @param m
	 * @param debug
	 * @return
	 */
	public static double[] betterforecast(List<Integer> y, int period, int m) {
		double[] result = null;
		double avg=0;
		for(int i=0;i<alphas.length;i++){
			int sum=0;
			double[] temp=forecast(y,alphas[i],0.25,0,period,m,false);
			for(int j=0;j<y.size();j++){
				sum+=Math.abs(y.get(j)-temp[j]);
			}
			if(i==0){
				avg=sum/y.size();
				result=temp;
			}
			if(avg<(sum/y.size())){
				result=temp;
			}
		}
		return result;
	}
	
	/**
	 * This method is the entry point. It calculates the initial values and returns the forecast
	 * for the m periods.
	 * @param y 数据集合
	 * @param alpha - Exponential smoothing coefficients for level, trend, seasonal components.
	 * @param beta - Exponential smoothing coefficients for level, trend, seasonal components.
	 * @param gamma - Exponential smoothing coefficients for level, trend, seasonal components.
	 * @param perdiod - 一个完整周期的长度，至少需要两个周期 
	 * @param m -预测的个数
	 * @param debug - 是否打印详细信息
	 */
	public static double[] forecast(List<Integer> y, double alpha, double beta,
			double gamma, int period, int m, boolean debug) {


		if (y == null) {
			return null;
		}
		
		//获取季节数
		int seasons = y.size() / period;
		//平滑值st0
		double a0 = calculateInitialLevel(y, period);
		//bt0
		double b0 = calculateInitialTrend(y, period);
		//初始化季节因子
		double[] initialSeasonalIndices = calculateSeasonalIndices(y, period, seasons);


		if (debug) {
			System.out.println(String.format(
					"Total observations: %d, Seasons %d, Periods %d", y.size(),
					seasons, period));
			System.out.println("Initial level value a0: " + a0);
			System.out.println("Initial trend value b0: " + b0);
			printArray("Seasonal Indices: ", initialSeasonalIndices);
		}


		double[] forecast = calculateHoltWinters(y, a0, b0, alpha, beta, gamma,
				initialSeasonalIndices, period, m, debug);


		if (debug) {
			printArray("Forecast", forecast);
		}


		return forecast;
	}
	
	/**
	 * This method realizes the Holt-Winters equations.
	 * 计算进行预测
	 * @param y
	 * @param a0
	 * @param b0
	 * @param alpha
	 * @param beta
	 * @param gamma
	 * @param initialSeasonalIndices
	 * @param period
	 * @param m
	 * @param debug
	 * @return - Forecast for m periods.  
	 */
	private static double[] calculateHoltWinters(List<Integer> y, double a0, double b0, double alpha,
			double beta, double gamma, double[] initialSeasonalIndices, int period, int m, boolean debug) {
		//整体平滑 st=alpha * y [i] / It [i-period] +（1.0  -  alpha）*（St [i  -  1] + Bt [i  -  1]）
		double[] St = new double[y.size()];
		//趋势平滑 Bt [i] = gamma *（St [i] -St [i-1]）+（1-gamma）* Bt [i-1]
		double[] Bt = new double[y.size()];
		//季节性平滑It [i] = beta * y [i] / St [i] +（1.0  -  beta）* It [i  -  period]
		double[] It = new double[y.size()];
		//预测Ft [i + m] =（St [i] +（m * Bt [i]））* It [i  -  period + m]
		double[] Ft = new double[y.size() + m];
		
		//Initialize base values  初始化
		St[1] = a0;
		Bt[1] = b0;
		
		//季节性平滑
		for (int i = 0; i < period; i++) {
			It[i] = initialSeasonalIndices[i];
		}
		//预测初始化
		Ft[0+m] = (St[0] + (m * Bt[0])) * It[0];//因为Bt [0] = 0，所以这实际上是0
		Ft[m + 1] = (St[1] + (m * Bt[1])) * It[1];//Forecast starts from period + 2
		
		//Start calculations
		for (int i = 2; i < y.size(); i++) {
			//Calculate overall smoothing  计算所有的平滑值
			if((i - period) >= 0) {
				St[i] = alpha * y.get(i) / It[i - period] + (1.0 - alpha) * (St[i - 1] + Bt[i - 1]);
			} else {
				St[i] = alpha * y.get(i) + (1.0 - alpha) * (St[i - 1] + Bt[i - 1]);
			}
			
			//Calculate trend smoothing  计算所有的趋势
			Bt[i] = gamma * (St[i] - St[i - 1]) + (1 - gamma) * Bt[i - 1];
			
			//Calculate seasonal smoothing  计算所有的季节趋势
			if((i - period) >= 0) {
				It[i] = beta * y.get(i) / St[i] + (1.0 - beta) * It[i - period];
			}
			//Calculate forecast  计算预期值
			if( ((i + m) >= period) ){
				Ft[i + m] = (St[i] + (m * Bt[i])) * It[i - period + m];
				if(Ft[i+m]<=0){
					Ft[i+m]=1;
				}
			}
			if(debug){
				System.out.println(String.format(
						"i = %d, y = %d, S = %f, Bt = %f, It = %f, F = %f", i,
						y.get(i), St[i], Bt[i], It[i], Ft[i]));
			}
		}
		return Ft;
	}


	/**
	 * See: http://robjhyndman.com/researchtips/hw-initialization/
	 * 1st period's average can be taken. But y[0] works better.
	 * 
	 * @return - Initial Level value i.e. St[1]
	 */
	private static double calculateInitialLevel(List<Integer> y, int period) {


		/**		
 		double sum = 0;
		for (int i = 0; i < period; i++) {
			sum += y[i];
		}
		
		return sum / period;
		 **/
		return y.get(0);
	}
	
	/**
	 * See: http://www.itl.nist.gov/div898/handbook/pmc/section4/pmc435.htm
	 * 
	 * @return - Initial trend - Bt[1]
	 */
	private static double calculateInitialTrend(List<Integer> y, int period){
		
		double sum = 0;
		
		for (int i = 0; i < period; i++) {			
			sum += (y.get(period+i)- y.get(i));
		}
		
		return sum / (period * period);
	}
	
	/**
	 * See: http://www.itl.nist.gov/div898/handbook/pmc/section4/pmc435.htm
	 * 计算及季节性指数
	 * @return - Seasonal Indices.
	 */
	private static double[] calculateSeasonalIndices(List<Integer> y, int period, int seasons){
						
		double[] seasonalAverage = new double[seasons];
		double[] seasonalIndices = new double[period];
		
		double[] averagedObservations = new double[y.size()];
		
		//获取所有季节各个周期的平均值
		for (int i = 0; i < seasons; i++) {
			for (int j = 0; j < period; j++) {
				seasonalAverage[i] += y.get((i * period) + j);
			}
			seasonalAverage[i] /= period;
		}
		//平均观测值
		for (int i = 0; i < seasons; i++) {
			for (int j = 0; j < period; j++) {
				//真正的值/平均值
				averagedObservations[(i * period) + j] =y.get((i * period) + j) / seasonalAverage[i];				
			}			
		}
		//初始季节性指数
		for (int i = 0; i < period; i++) {
			for (int j = 0; j < seasons; j++) {
				seasonalIndices[i] += averagedObservations[(j * period) + i];
			}			
			seasonalIndices[i] /= seasons;
		}
		return seasonalIndices;
	}
	
	/**
	 * Utility method to pring array values.
	 * 
	 * @param description
	 * @param data
	 */
	private static void printArray(String description, double[] data){
		System.out.println(String.format("******************* %s *********************", description));
		
		for (int i = 0; i < data.length; i++) {
			System.out.println(data[i]);
		}
		
		System.out.println(String.format("*****************************************************************", description));
	}
	
	/**
	 * 求出二次最佳预测
	 * @param y
	 * @param m
	 * @return
	 */
	public static double[] betterSec(List<Integer> y, int m) {
		double[] result = null;
		double avg=0;
		for(int i=0;i<alphas.length;i++){
			int sum=0;
			double[] temp=getSecExpect(y,m,alphas[i]);
			for(int j=0;j<y.size();j++){
				sum+=Math.abs(y.get(j)-temp[j]);
			}
			if(i==0){
				avg=sum/y.size();
				result=temp;
			}
			if(avg<sum/y.size()){
				result=temp;
			}
		}
		return result;
	}
	
	/**
	 * 二次指数平滑法求预测值 预测线性趋势
	 * 
	 * @param list
	 *            基础数据集合
	 * @param year
	 *            未来第几期
	 * @param modulus
	 *            平滑系数
	 * @return 预测值
	 */
	public static double[] getSecExpect(List<Integer> list, int year, Double modulus) {
		if (list.size() < 20 || modulus <= 0 || modulus >= 1) {
			return null;
		}
		double[] ft=new double[list.size()+year];
		Double modulusLeft = 1 - modulus;
		// 一次预测 >20项默认第一个值为初始值
		Double lastIndex = Double.parseDouble(list.get(0).toString());
		// 二次预测 >20项默认第一个值为初始值
		Double lastSecIndex = Double.parseDouble(list.get(0).toString());
		for (int i=0;i<list.size();i++) {
			// st1
			lastIndex = modulus * list.get(i) + modulusLeft * lastIndex;
			// st2
			lastSecIndex = modulus * lastIndex + modulusLeft * lastSecIndex;
			// AT
			Double a = 2 * lastIndex - lastSecIndex;
			// BT
			Double b = (modulus / modulusLeft) * (lastIndex - lastSecIndex);
			ft[i+1]=a+b*year;
			if(ft[i+1]<=0){
				ft[i+1]=1;
			}
		}
		
		return ft;
	}




	


	/**
	 * 三次指数平滑法求预测值 预测线性趋势
	 * 
	 * @param list
	 *            基础数据集合
	 * @param year
	 *            未来第几期
	 * @param modulus
	 *            平滑系数
	 * @return 预测值
	 */
	public static Double getThridExpect(List<Integer> list, int year, Double modulus) {
		if (list.size() < 20 || modulus <= 0 || modulus >= 1) {
			return null;
		}
		Double modulusLeft = 1 - modulus;
		// 一次预测 >20项默认第一个值为初始值
		Double lastIndex =(double) ((list.get(0)+list.get(1)+list.get(2))/3);
		// 二次预测 >20项默认第一个值为初始值
		Double lastSecIndex = lastIndex;
		// 三次预测 >20项默认第一个为初始值
		Double lastThreadIndes = lastIndex;
		for (Integer data : list) {
			// st1
			lastIndex = modulus * data + modulusLeft * lastIndex;
			// st2
			lastSecIndex = modulus * lastIndex + modulusLeft * lastSecIndex;
			// st3
			lastThreadIndes = modulus * lastSecIndex + modulusLeft * lastThreadIndes;
		}
		// AT
		Double a = 3 * lastIndex - 3 * lastSecIndex + lastThreadIndes;
		// BT
		Double b = (modulus / (2 * modulusLeft * modulusLeft)) * ((6 - 5 * modulus) * lastIndex
				- 2 * (5 - 4 * modulus) * lastSecIndex + (4 - 3 * modulus) * lastThreadIndes);
		// CT
		Double c = ((modulus * modulus) / (2 * modulusLeft *modulusLeft))
				* (lastIndex - 2 * lastSecIndex + lastThreadIndes);
		Double except=a+b*year+c*year*year;
		return except<0?0:except;
	}

}

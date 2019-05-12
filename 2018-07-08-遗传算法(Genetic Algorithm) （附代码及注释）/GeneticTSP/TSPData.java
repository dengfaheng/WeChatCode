package GeneticTSP;

/**
 * TSP数据类
 * 包含：
 * 		disMap 各个城市间距离矩阵
 */

public class TSPData {
	
	static int CITY_NUM; //城市数
	static final int SPECIES_NUM=200; //种群数
	static final int DEVELOP_NUM=1000; //进化代数
	static final float pcl=0.6f,pch=0.95f;//交叉概率
	static final float pm=0.4f;//变异概率
	static final float[][] disMap; //地图数据
	static
	{
//		int[][] cityPosition={
//				{0,0},{12,32},{5,25},{8,45},{33,17},
//				{25,7},{15,15},{15,25},{25,15},{41,12}};//10个城市（最优解:147）
//		int[][] cityPosition={
//				{60,200},{180,200},{80,180},{140,180},
//				{20,160},{100,160},{200,160},{140,140},
//				{40,120},{100,120},{180,100},{60,80},
//				{120,80},{180,60},{20,40},{100,40},
//				{200,40},{20,20},{60,20},{160,20}};//20个城市（最优解:870）
//		
		//城市坐标集合
		int[][] cityPosition={
				{1304,        2312},{3639,        1315},         
				{4177,        2244},{3712,        1399},         	
				{3488,        1535},{3326,        1556},         
				{3238,        1229},{4196,        1004},         
				{4312,         790},{4386,         570},
				{3007,        1970},{2562,        1756},
				{2788,        1491},{2381,        1676},
				{1332,         695},{3715,        1678},
				{3918,        2179},{4061,        2370},
				{3780,        2212},{3676,        2578},
				{4029,        2838},{4263,        2931},
				{3429,        1908},{3507,        2367},
				{3394,        2643},{3439,        3201},
				{2935,        3240},{3140,        3550},
				{2545,        2357},{2778,        2826},
				{2370,        2975}};//31个城市（最优解:14700）
		
		//路径集合
		CITY_NUM=cityPosition.length;
		disMap=new float[CITY_NUM][CITY_NUM];
		for(int i=0;i<CITY_NUM;i++)
		{
			for(int j=i;j<CITY_NUM;j++)
			{
				float dis=(float)Math.sqrt(Math.pow((cityPosition[i][0] - cityPosition[j][0]),2) + Math.pow((cityPosition[i][1] - cityPosition[j][1]),2));
				
				disMap[i][j]=dis;
				disMap[j][i]=disMap[i][j];
			}
		}	
	}

}

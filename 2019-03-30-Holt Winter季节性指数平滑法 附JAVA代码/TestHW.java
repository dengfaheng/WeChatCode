package com.dfh.holtwinters;
import java.util.ArrayList;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import jxl.*;
import jxl.read.biff.BiffException;   
import jxl.Workbook;
import jxl.write.Label;
import jxl.write.WritableSheet;
import jxl.write.WritableWorkbook;
import jxl.write.WriteException;
import jxl.write.biff.RowsExceededException;

public class TestHW {

	public static void main(String[] args) throws BiffException, IOException, RowsExceededException, WriteException {
		// TODO Auto-generated method stub
		
		HoltWintersTripleExponentialImpl myHoltWinters = new HoltWintersTripleExponentialImpl();
		ArrayList<Integer> orgHSP = new ArrayList<Integer>();
		
		//1:创建workbook
        Workbook workbook=Workbook.getWorkbook(new File("/Users/dengfaheng/data.xls")); 
        //2:获取第一个工作表sheet
        Sheet sheet=workbook.getSheet(1);
        //3:获取数据
        System.out.println("行："+sheet.getRows());
        System.out.println("列："+sheet.getColumns());
        for(int i=1;i<sheet.getRows()-3;i++){
            Cell cell=sheet.getCell(7,i);
            orgHSP.add(Integer.parseInt((cell.getContents().trim())));
            //System.out.print(cell.getContents()+" ");
        }
        //最后一步：关闭资源
        workbook.close();
        System.out.println("orgHSP = "+orgHSP);
        
		
		double []res = myHoltWinters.betterforecast(orgHSP, 4, 3);

		
	       //1:创建excel文件
        File file=new File("/Users/dengfaheng/test.xls");
        file.createNewFile();
        
        //2:创建工作簿
        WritableWorkbook workbook1=Workbook.createWorkbook(file);
        //3:创建sheet,设置第二三四..个sheet，依次类推即可
        WritableSheet sheet1=workbook1.createSheet("HSP", 0);
        //4：设置titles

        //5:单元格
        Label label=null;
        //6:给第一行设置列名
 
        //x,y,第一行的列名 列 行
        label=new Label(0,0,"预测HSP");
        //7：添加单元格
        sheet1.addCell(label);

        //8：模拟数据库导入数据
        for(int i=1;i<=res.length;i++){
            //添加编号，第二行第一列
            label=new Label(0,i,(res[i-1])+"");
            sheet1.addCell(label);
        }
        
        //写入数据，一定记得写入数据，不然你都开始怀疑世界了，excel里面啥都没有
        workbook1.write();
        //最后一步，关闭工作簿
        workbook1.close();
		

	}

}

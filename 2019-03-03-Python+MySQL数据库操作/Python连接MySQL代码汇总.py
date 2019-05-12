#--------------3.2--------------

import mysql.connector  
#连接数据库  
config = {  
    'user' : 'blank',        #用户名
    'password' :'password',  #自己设定的密码
    'host' : '127.0.0.1',   #ip地址，本地填为127.0.0.1，也可以填localhost
    'port':'3306',           #端口，本地的一般为3306
    'database' : 'test_s'   #数据库名字，这里选用test_s
}  
con = mysql.connector.connect(**config)  




#--------------3.3--------------

#STEP 2

# 创建一个表       
# buffered=True 不设的话，查询结果没有读完会报错
# raise errors.InternalError("Unread result found")
mycursor = con.cursor(buffered=True)
mycursor.execute("CREATE TABLE customers (id INT AUTO_INCREMENT PRIMARY KEY, \  
                                          name VARCHAR(255), address VARCHAR(255), \  
                                          sex VARCHAR(225), age INT(10), sl INT(10))")  

#STEP 5
def tableExists(mycursor, name):  
    stmt = "SHOW TABLES LIKE '"+name+"'"  
    mycursor.execute(stmt)  
    return mycursor.fetchone()  
mycursor = con.cursor()  
if tableExists(mycursor, 'customers'):  
    print("table already exists")  
else:  
    print("table not exists") 


#STEP 6
import mysql.connector  
#连接数据库  
config = {  
    'user' : 'blank',        
    'password' :'fuying123888',  
    'host' : '127.0.0.1',   
    'port':'3306',           
    'database' : 'test_s'  
}  
con = mysql.connector.connect(**config)  
# 检查一个表是否存在  
def tableExists(mycursor, name):  
    stmt = "SHOW TABLES LIKE '"+name+"'"  
    mycursor.execute(stmt)  
    return mycursor.fetchone()     
# 删除一个表（无论它是否已经存在）  
def dropTable(mycursor, name):  
    stmt = "DROP TABLE IF EXISTS "+name  
    mycursor.execute(stmt)  
# buffered=True 不设的话，查询结果没有读完会报错  
# raise errors.InternalError("Unread result found")  
mycursor = con.cursor(buffered=True)  
# 删除临时表  
tableName = 'customers'  
dropTable(mycursor, tableName)  
# 创建一个表       
mycursor.execute("CREATE TABLE customers(id INT AUTO_INCREMENT PRIMARY KEY,\
                               name VARCHAR(255), address VARCHAR(255), \  
                               sex VARCHAR(225), age INT(10), sl INT(10))")

#--------------3.4--------------


#3.4.1

# 往表里插入一些记录  
sql="INSERT INTO customers(name,address,sex,age,sl) VALUES(%s, %s,%s,%s,%s)"
val = ("John", "Highway 21","M",23,5000)  
mycursor.execute(sql, val)  
val = ("Jenny", "Highway 29","F",30,12500)  
mycursor.execute(sql, val)  
val=[("Tom","ABC 35","M",35,14000),  
     ("Tom1","Highway 29","M",28,6700),  
     ("Lily","Road 11","F",30,8000),  
     ("Martin","Road 24","M",35,14000),  
     ("Sally","Fast 56","M",32,15000)]  
mycursor.executemany(sql, val)  
con.commit()  

#3.4.2

# 将Sally改为Tiny  
sql="UPDATE customers SET name='Tiny' WHERE name ='Sally'"  
mycursor.execute(sql)  
con.commit()  

#3.4.3

#删除名字为Tiny的记录  
sql="DELETE FROM  customers WHERE name='Tiny'"  
mycursor.execute(sql)  
con.commit()  

#3.4.4

#查询这里面所有的人：  
sql="SELECT * FROM customers"  
mycursor.execute(sql)  
myresult = mycursor.fetchall()     # fetchall() 获取所有记录  
for x in myresult:  
  print(x)  


#限定条件查找
sql="SELECT * FROM customers WHERE age > 30"  
mycursor.execute(sql)  
myresult = mycursor.fetchall()     # fetchall() 获取所有记录  
for x in myresult:  
  print(x)  


#通配符查找

#%代表零个、一个或者多个数字或字符  
#_代表一个单一的数字或者字符  
sql = "SELECT * FROM customers WHERE name LIKE '%t%'"  
mycursor.execute(sql)  
myresult=mycursor.fetchall()  
for x in myresult:  
    print(x)  

#排序
    
#按照年龄排序  
sql = "SELECT * FROM customers ORDER BY age"  
mycursor.execute(sql)  
myresult=mycursor.fetchall()  
for x in myresult:  
    print(x)  

#LIMIT
    
#找出其中工资最高的3个人  
sql = "SELECT * FROM customers ORDER BY sl DESC LIMIT 3"  
mycursor.execute(sql)  
myresult=mycursor.fetchall()  
for x in myresult:  
    print(x)  


#二次筛选
    
#统计在年龄处于20-30之间的人中，选择薪资大于5000的人  
sql = "SELECT * FROM customers WHERE age>20 and age<=30 HAVING sl>5000 "  
mycursor.execute(sql)  
myresult=mycursor.fetchall()  
for x in myresult:  
    print(x)  
con.commit()  
con.close()  


#--------------3.5--------------

#统计出男女的薪水总数  
sql = "SELECT sex,sum(sl) FROM customers GROUP BY sex"  
mycursor.execute(sql)  
myresult=mycursor.fetchall()  
for x in myresult:  
    print(x)  

# 按照性别进行分组，统计出年龄在20-30岁的消费者的薪资  
sql = "SELECT sex,sum(sl) FROM customers WHERE age>20 and age<=30 GROUP BY sex ORDER BY sl"  
mycursor.execute(sql)  
myresult=mycursor.fetchall()  
for x in myresult:  
    print(x)  




#--------------3.6--------------

# 分批读取并且处理将薪资大于8000的消费者的记录存到另一张表中  
# 创建一个临时表   
tmpName = 'cust_tmp'  
dropTable(mycursor, tmpName)    
mycursor.execute("CREATE TABLE cust_tmp(id INT AUTO_INCREMENT PRIMARY KEY,\
                               name VARCHAR(255), address VARCHAR(255), \  
                               sex VARCHAR(225), age INT(10), sl INT(10))")
ins = con.cursor(buffered=True)  
if tableExists(mycursor, tableName):  
    print("process table: %s", tableName)  
    # 查询表里的记录  
    sql = "SELECT * FROM customers WHERE address is not null"  
    mycursor.execute(sql)  
    # 每次处理 batchsize 条记录，直到所有查询结果处理完  
    batchsize = 3  
    readsize = batchsize  
    while readsize == batchsize:  
        print("before batch")  
        myresult = mycursor.fetchmany(size=batchsize)  
        for x in myresult:  
            if x[5]>8000:  
                ins.execute("INSERT INTO"+tmpName+"(id,name,address,sex,age,sl) VALUES (%s, %s,%s, %s,%s,%s)", x)  
                print(x)  
        readsize = len(myresult)  
else:  
    print("table: does not exists", tableName)  
con.commit()  
con.close()  




























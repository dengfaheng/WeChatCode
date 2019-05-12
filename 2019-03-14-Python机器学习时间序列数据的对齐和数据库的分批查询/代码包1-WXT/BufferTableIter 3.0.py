# -*- coding: utf-8 -*-
"""
Created on Sat Mar  9 15:04:39 2019

@author: WXT（847718009@qq.com

类BufferTableIter 3.0 - 实现数据库连接
"""

class DBTable:
    def __init__(self, con, sql, batch_size):
        ############修改部分#############
        self.con = con #创建连接
        self.sql = sql #需要执行的sql语句
        ################################
        self.bs = batch_size
    
    def __iter__(self):
        return BufferTableIter(self)
    
class BufferTableIter():
    # 初始化
    def __init__(self, dbTable):
        ############修改部分#############
        self.cursor = dbTable.con.cursor(buffered=True) #创建游标
        self.cursor.execute(dbTable.sql) #执行sql语句
        self.readCount = 0
        ################################
        self.bs = dbTable.bs 
        self.buf = [] 
        self.idx = 0 
        
    def readBatch(self):
        if self.idx == len(self.buf):
            ############修改部分#############
            self.buf = self.cursor.fetchmany(size=self.bs) #从数据库中读取批次数据

            #更新指针
            self.readCount += len(self.buf)
            self.idx = 0   
            ################################
    
    def hasNext(self):
        self.readBatch()
        return self.idx < len(self.buf)
    
    def readNext(self):
        self.readBatch()
        if self.idx < len(self.buf):
            line = self.buf[self.idx]
            self.idx += 1
            return line
        return None
        
    def __iter__(self):
        return self
    
    def __next__(self):
        if self.hasNext():
            return self.readNext()
        raise StopIteration
        
if __name__ == '__main__':
    
    import mysql.connector    
    con = mysql.connector.connect(
      host="127.0.0.1",
      user="WXT",
      passwd="12345",
      database="wxt_db",
      use_pure="False")    
    
# =============================================================================
#     mycursor = con.cursor(buffered=True)    
#     # 创建一个表     
#     mycursor.execute("CREATE TABLE patient (time VARCHAR(255), hr INT, hxpl INT)")   
#     # 往表里插入一些记录
#     mycursor.execute("INSERT INTO patient (time, HR, HXPL) VALUES ('01:30',128,19)")
#     mycursor.execute("INSERT INTO patient (time, HR, HXPL) VALUES ('05:00',124,20)")
#     mycursor.execute("INSERT INTO patient (time, HR, HXPL) VALUES ('13:00',131,18)")
#     mycursor.execute("INSERT INTO patient (time, HR, HXPL) VALUES ('20:00',138,24)")
#     mycursor.execute("INSERT INTO patient (time, HR, HXPL) VALUES ('21:30',122,22)")
#     
#     con.commit()
#     con.close()    
# =============================================================================
    
    # 检查一个表是否存在
    def tableExists(mycursor, name):
        stmt = "SHOW TABLES LIKE '"+name+"'"
        mycursor.execute(stmt)
        return mycursor.fetchone()
    
    try:    
        mycursor = con.cursor(buffered=True)
        
        if tableExists(mycursor, 'patient'):
            print('process table:','patient')
            print("--------")
    
            #查询表里的记录
            sql = "SELECT * FROM patient"
    
            dbTbl = DBTable(con, sql, 2)
            btr = BufferTableIter(dbTbl)
    
            for rec in dbTbl:
                print("read record:", rec)
    
    finally:    
        con.close() #关闭数据库
    
    
    
# -*- coding: utf-8 -*-
"""
Created on Sat Mar  9 15:01:36 2019

@author: WXT（847718009@qq.com）

类BufferTableIter 2.1 - 实现重复迭代功能
"""

class DBTable:
    def __init__(self, data, batch_size):
        self.data = data
        self.bs = batch_size
    def __iter__(self):
        print("__iter__ called")
        return BufferTableIter(self)

class BufferTableIter():
    # 初始化
    def __init__(self, dbTable):
        ############修改部分#############
        self.data = dbTable.data 
        self.bs = dbTable.bs 
        ##################################
        self.buf = [] 
        self.idx = 0 
        self.offset = 0 
        
    def readBatch(self):
        if self.idx == len(self.buf):
            slice_size = min(len(self.data)-self.offset, self.bs) 
            self.buf = self.data[self.offset : self.offset+slice_size]

            #更新指针
            self.offset += slice_size
            self.idx = 0    
    
    def hasNext(self):
        self.readBatch()
        return self.idx < len(self.buf)
    
    def readNext(self):
        self.readBatch()
        if self.idx < len(self.buf):
            line = self.buf[self.idx]
            self.idx += 1
            return line
        
    def __iter__(self):
        return self
    
    def __next__(self):
        if self.hasNext():
            return self.readNext()
        else:
            raise StopIteration
            
if __name__ == '__main__':
    #实例化
    ts1 = [('01:30',128,19),
           ('05:00',124,20),
           ('13:00',131,18),
           ('20:00',138,24),
           ('21:30',122,22)]
    
    dbTable = DBTable(ts1, 2)
    btr = BufferTableIter(dbTable)
    
    #DBTable实例化出来的对象可以任意多次重复遍历
    for l in dbTable: print(l)
    for l in dbTable: print(l)  
    
# =============================================================================
#     #BufferTableIter则不能多次重复遍历
#     for l in btr: print(l)
#     print('--------------')
#     for l in btr: print(l)
# =============================================================================

            
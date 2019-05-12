# -*- coding: utf-8 -*-
"""
Created on Sat Mar  9 14:58:57 2019

@author: WXT（847718009@qq.com）

类BufferTableIter 1.0 - 实现一般迭代功能
"""

class BufferTableIter():
    # 初始化
    def __init__(self, data, batch_size ):
        self.data = data # 数据源
        self.bs = batch_size # 每次读取的批次大小 
        self.buf = [] # 用来存储当前批次的数据，初始化为空 
        self.idx = 0 # 当前批次数据（self.buf）指针，初始化为0
        self.offset = 0 # 数据源（data）指针，初始化为0
        
    def readBatch(self):
        if self.idx == len(self.buf):
            slice_size = min(len(self.data)-self.offset, self.bs) #读多少
            self.buf = self.data[self.offset : self.offset+slice_size]

            #更新指针
            self.offset += slice_size
            self.idx = 0    
            
            # 为了便于观察，每读完一个批次做一个标注
            print('-------')
    
    def hasNext(self):
        self.readBatch()
        return self.idx < len(self.buf)
    
    def readNext(self):
        self.readBatch()
        if self.idx < len(self.buf):
            line = self.buf[self.idx]
            self.idx += 1
            return line
        
    ############新增部分#############
    def __iter__(self):
        print('iter called')
        return self
    
    def __next__(self):
        if self.hasNext():
            return self.readNext()
        else:
            raise StopIteration
            
    ##################################

if __name__ == '__main__':           
    #实例化
    ts1 = [('01:30',128,19),
           ('05:00',124,20),
           ('13:00',131,18),
           ('20:00',138,24),
           ('21:30',122,22)]
    
    print('通过next()方法调用：')
    btr = BufferTableIter(ts1, 2)
    while btr.hasNext():
        print(btr.readNext())
        ##或者：
        #print(next(btr))
    
    print('通过for循环调用：')
    btr = BufferTableIter(ts1, 2)    
    for l in btr: print(l)
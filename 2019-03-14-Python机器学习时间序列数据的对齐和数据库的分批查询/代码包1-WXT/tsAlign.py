# -*- coding: utf-8 -*-
"""
Created on Sat Mar  9 14:50:47 2019

@author: WXT（847718009@qq.com）
"""

def tsAlign(x, y):
    '''
    x，y: lsit of tuple，
            每个tuple代表一条记录，tule的第0个元素为这条记录的id（eg，时间），
            list里的tuple按照tuple的id从小到大进行排序
    
    把x和y在id上进行对齐：
        若x，y同时存在某个id，则将这两个tuple进行合并；
        若x存在某个id而y不存在，则合并 x当前id对应的tuple 和 y小于当前id的最大id对应的tuple。
    
    return
        z: 对齐了x和y之后的list of tuple
    '''
    # 设置循环初始值
    i = 0 #index of x
    j = 0 #index of y
    z=[]  #store result of merging x and y   
    pre_x = (None,)*len(x)
    pre_y = (None,)*len(y)
    
    while (i<len(x)) and (j<len(y)):
        #若x当前的id小于y当前的id，则合并x当前的tuple和y的前一个tuple
        if x[i][0] < y[j][0]:
            z.append(x[i] + pre_y[1:])
            pre_x = x[i]
            i += 1
        #若x当前的id大于y当前的id，则合并x的前一个tuple和y当前的tuple
        elif x[i][0] > y[j][0]:
            z.append((y[j][0],) + pre_x[1:] + y[j][1:]) #注意：在定义tuple的时候，若只有一个元素，需要在元素后加个逗号
            pre_y = y[j]
            j += 1
        #若x当前的id等于y当前的id，则合并x当前的tuple和y当前的tuple
        else:
            z.append(x[i] + y[j][1:])
            pre_x = x[i]
            pre_y = y[j]
            i += 1
            j +=1
            
    while i < len(x):
        z.append(x[i] + pre_y[1:])
        i += 1
        
    while j < len(y):
            z.append((y[j][0],) + pre_x[1:] + y[j][1:])
            j += 1    
    
    return z


if __name__ == '__main__':
    
    ts1 = [('01:30',128,19),
           ('05:00',124,20),
           ('13:00',131,18),
           ('20:00',138,24),
           ('21:30',122,22)]

    ts2 = [('01:30',7,129,60),
           ('09:30',6.5,112,63),
           ('12:00',8,135,74),
           ('13:00',8.5,110,60),
           ('20:32',7,78,49),
           ('22:00',7.5,96,55),
           ('23:30',6,124,59)]
    
    print(tsAlign(ts1, ts2))
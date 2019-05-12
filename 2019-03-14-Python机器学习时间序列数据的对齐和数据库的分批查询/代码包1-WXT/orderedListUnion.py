# -*- coding: utf-8 -*-
"""
Created on Sat Mar  9 14:48:17 2019

@author: WXT（847718009@qq.com）
"""

def orderedListUnion(a, b):
    '''
    合并两个按从小到大排好序的序列a，b
    '''
       
    # 设置循环初始值
    idx_a = 0
    idx_b = 0     
    c = []

    # 声明变量len_a,len_b，指向序列a,b的长度，用来控制循环条件
    len_a = len(a) 
    len_b = len(b)        
    while (idx_a < len_a) and (idx_b < len_b):
        
        #若两个元素相等，则将该元素添加到c，且两个idx同时右移：
        if a[idx_a] == b[idx_b]:
            c.append(a[idx_a])
            idx_a += 1
            idx_b += 1
        
        #若不相等，取较小的元素，且较小元素的idx右移
        elif a[idx_a] < b[idx_b]:
            c.append(a[idx_a])
            idx_a += 1
        else:
            c.append(b[idx_b])
            idx_b += 1
            
    # 当一个序列遍历结束后，跳出循环，将未遍历完的序列的剩余元素添加到c
    if idx_a == len_a:
        c = c + b[idx_b:]
    if idx_b == len_b:
        c = c + a[idx_a:]
        
    return c

if __name__ == '__main__':
    a = [1,3,7,9,11]
    b = [3,4,7,8]
    print(orderedListUnion(a,b))

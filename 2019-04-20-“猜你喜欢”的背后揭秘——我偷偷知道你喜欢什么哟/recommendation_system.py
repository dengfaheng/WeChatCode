# -*- coding: utf-8 -*-
"""
Created on Thu Mar 21 19:29:54 2019

@author: o
"""
import pandas as pd
import numpy as np
from math import sqrt
import copy

def load_data(path):
    df = pd.read_excel(path)
    #去掉不需要的列
    df = df[df.columns[5:]]
    #1表示没看过，2~6表示1~5星
    df.replace([1,2,3,4,5,6],[0,1,2,3,4,5],inplace = True)
    columns = df.columns
    df = np.array(df)
    #测试过程中发现有nan存在，原来是因为有人恶作剧，全填了没看过，导致分母为0
    #此处要删除全为0的行
    delete = []
    for i in range(df.shape[0]):
        all_0 = (df[i] == [0]*15)
        flag = False
        for k in range(15):
            if all_0[k] == False:
                flag = True
                break
        if flag == False:
            delete.append(i)
            print(i)
    df = np.delete(df,delete,0)
    return df,columns


#定义几种衡量相似度的标准
#余弦相似度
def cos(score,your_score):
    cos = []
    len1 = 0
    for i in range(15):
        len1 += pow(your_score[i],2)
    len1 = sqrt(len1)
    for i in range(score.shape[0]):
        len2 = 0
        for k in range(15):
            len2 += pow(score[i][k],2)
        len2 = sqrt(len2)
        cos.append(np.dot(your_score,score[i])/(len1*len2))                
    return cos

#欧氏距离
def euclidean(score,your_score):
    euclidean = []
    for i in range(score.shape[0]):
        dist = 0
        for k in range(score.shape[1]):
            dist += pow((score[i][k]-your_score[k]),2)
        dist = sqrt(dist)
        euclidean.append(dist)
    return euclidean


#pearson相似度
#Python有内置函数corrcoef()可以直接计算，不过这里还是手写巩固一下吧~
def pearson(score,your_score):
    pearson = []
    n = score.shape[1]
    sum_y = 0
    count = 0
    #计算目标用户打分的均值
    for i in range(n):
        if your_score[i]!=0:
            count += 1
            sum_y += your_score[i]
    mean_y = sum_y/count
    print('\n')
    print('你的平均打分为：',mean_y)
    print('\n')
    #计算目标用户打分向量的长度
    len1 = 0
    for i in range(n):
        if your_score[i]!=0:
            your_score[i] -= mean_y
        len1 += pow(your_score[i],2)
    len1 = sqrt(len1)
    #print(len1,mean_y,your_score)
    
    for i in range(score.shape[0]):
        #计算其他用户打分的均值
       # print(i,score[i])
        count = 0
        sum_x = 0
        for k in range(n):
            if score[i][k]!=0:
                count += 1
                sum_x += score[i][k]
        mean_x = sum_x/count
        #计算其他用户打分向量的长度
        len2 = 0
        for k in range(n):
            if score[i][k]!=0:
                score[i][k] -= mean_x
            len2 += pow(score[i][k],2)
        len2 = sqrt(len2)
        #print(len2,mean_x,score[i],'\n','\n')
        #分母不可为零，不然会产生nan
        if len2 == 0:
            pearson.append(0)
        else:
            pearson.append(np.dot(your_score,score[i])/len1/len2)
    return pearson,mean_y      


#找到相似度最高的用户
def find_nearest(sim):
    index = [i for i in range(len(sim))]
    #index和sim的元组列表
    sorted_value = list(zip(index,sim))
    #降序排序
    sorted_value = sorted(sorted_value,key = lambda x : x[1],reverse = True)
    return sorted_value


#user-based collaborative_filtering
def collaborative_filtering(score,your_score,movies):
    #目标用户对15部电影有无看过的bool列表
    seen1 = np.array([bool(i) for i in your_score])
    #使用pearson过程中会改变score矩阵的值，需要用deepcopy复制一份
    score1 = copy.deepcopy(score)
    #几种相似度的衡量
    #sim = cos(score,your_score)
    #sim = euclidean(score,your_score)
    sim, mean_target= pearson(score1,your_score)
    #找到最相似的用户
    sorted_value = find_nearest(sim)
    
    #找到相似度>0的用户数量
    count = 0
    for i in range(score.shape[0]):
        if sorted_value[i][1]<=0:
            break
        else:
            count += 1
    #取根值，去掉正相似度中偏低的user
    count = int(sqrt(count))
    
    #加权打分 进行推荐
    print('使用user-based协同过滤进行加权预测打分：')
    for i in range(score.shape[1]):
        #如果目标用户没看过
        if not seen1[i]:
            #初始化分子分母
            numerator = denominator = 0
            for k in range(count):
                index = sorted_value[k][0]
                if score[index][i] != 0:
                    numerator += score[index][i]*sorted_value[k][1]
                    denominator += sorted_value[k][1]
            if not denominator:
                print(movies[i],'无相关度高的人看过，无法预测得分')
            elif numerator/denominator > mean_target:
                print(movies[i],':',numerator/denominator,'推荐观看')
            else:
                print(movies[i],":",numerator/denominator,'不推荐观看')
    return None


#梯度下降+隐因子模型
def latent_factors(score,your_score,movies):
    #目标用户的打分向量整合进打分矩阵
    score1 = np.vstack([your_score,score])
    #打分矩阵的维度
    n,m = score1.shape[0],score1.shape[1]
    #隐因子数量设为K
    K = 15
    #最大迭代次数
    max_iteration = 1000
    #学习速率和正则化因子
    alpha = 0.01
    beta = 0.01
    #LossFunction改变值小于threshold就结束
    threshold = 0.7
    #迭代次数
    count = 0
    #初始化分解后的矩阵P、Q
    p = np.random.random([n,K])
    q = np.random.random([m,K])
    #全体用户对15部电影有无看过的bool矩阵
    bool_matrix = [[bool(k) for k in i] for i in score1]
    
    while True:
        count += 1
        #更新P Q矩阵
        for i in range(n):
            for j in range(m):
                if bool_matrix[i][j]:
                    eij = score1[i][j] - np.dot(p[i],q[j])
                    for k in range(K):
                        #同时更新pik和qjk
                        diff=[0,0]
                        diff[0] = p[i][k] + alpha*(2*eij*q[j][k]-beta*p[i][k])
                        diff[1] = q[j][k] + alpha*(2*eij*p[i][k]-beta*q[j][k])
                        p[i][k] = diff[0]
                        q[j][k] = diff[1]
        #计算误差
        error = 0
        for i in range(n):
            for j in range(m):
                if bool_matrix[i][j]:                    
                    error += pow((score1[i][j]-np.dot(p[i],q[j])),2)
                    for k in range(K):
                        error += beta/2*(pow(p[i][k],2)+pow(q[j][k],2))
        RMSE = sqrt(error/n)
        print(count,'root_mean_square_error:',RMSE)
        if RMSE<threshold or count>max_iteration:
            break    
     
    #打印目标用户没看过的电影
    seen1 = np.array([bool(i) for i in your_score])
    print('你没看过的影片有：')
    for i in range(m):
        if not seen1[i]:
            print(movies[i])
    print('\n')
    
    #输出梯度下降预测分数
    predict = np.dot(p[0],q.T)
    print('使用梯度下降+隐因子模型进行预测打分：')
    for i in range(m):
        if not seen1[i]:
            print(movies[i],':',predict[i]) 
    return None


def recommend():
    #请自行修改路径
    path = r'C:\Users\o\Desktop\请给15部电影打分吧.xls'
    #原始打分矩阵，电影名称列表
    score, movies = load_data(path)
    #必须转换成Float类型，不然会出现分母为0的情况
    score = score.astype(float)
    your_score = []
    #构造目标用户打分向量
    print('请依次输入你对15部电影的打分(0表示没看过，1~5表示1~5星，以空格分隔):')
    print(movies)
    str = input()
    your_score = np.array([int(i) for i in str.split(' ')]).astype(float)
    #进行预测
    latent_factors(score,your_score,movies)
    collaborative_filtering(score,your_score,movies)
    return None


if __name__=='__main__':
    recommend()
    
    
    
    
    
    
    
    
    
    
    
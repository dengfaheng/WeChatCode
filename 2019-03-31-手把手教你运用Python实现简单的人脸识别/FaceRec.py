#-----Stage 1-----
#-----获取人脸样本-----
import cv2

#调用笔记本内置摄像头，参数为0，如果有其他的摄像头可以调整参数为1,2
cap = cv2.VideoCapture(0)
#调用人脸分类器，要根据实际路径调整3
face_detector = cv2.CascadeClassifier(r'X:/Users/73950/Desktop/FaceRec/haarcascade_frontalface_default.xml')  #待更改
#为即将录入的脸标记一个id
face_id = input('\n User data input,Look at the camera and wait ...')
#sampleNum用来计数样本数目
sample_num = 0

while True:    
    #从摄像头读取图片
    success,img = cap.read()    
    #转为灰度图片，减少程序符合，提高识别度
    if success is True: 
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY) 
    else:   
        break
    #检测人脸，将每一帧摄像头记录的数据带入OpenCv中，让Classifier判断人脸
    #其中gray为要检测的灰度图像，1.3为每次图像尺寸减小的比例，5为minNeighbors
    faces = face_detector.detectMultiScale(gray, 1.3, 5)

    #框选人脸，for循环保证一个能检测的实时动态视频流
    for (x, y, w, h) in faces:
        #xy为左上角的坐标,w为宽，h为高，用rectangle为人脸标记画框
        cv2.rectangle(img, (x, y), (x+w, y+w), (255, 0, 0))
        #成功框选则样本数增加
        sample_num += 1  
        #保存图像，把灰度图片看成二维数组来检测人脸区域
        #(这里是建立了data的文件夹，当然也可以设置为其他路径或者调用数据库)
        cv2.imwrite("data/User."+str(face_id)+'.'+str(sample_num)+'.jpg',gray[y:y+h,x:x+w]) 
        #显示图片
        cv2.imshow('image',img)       
        #保持画面的连续。waitkey方法可以绑定按键保证画面的收放，通过q键退出摄像
    k = cv2.waitKey(1)        
    if k == '27':
        break        
        #或者得到800个样本后退出摄像，这里可以根据实际情况修改数据量，实际测试后800张的效果是比较理想的
    elif sample_num >= 800:
        break
        
#关闭摄像头，释放资源
cap.realease()
cv2.destroyAllWindows()
        
#-----Stage 1-----
#-----建立模型、创建数据集-----

import os
import cv2
import numpy as np
from PIL import Image
#导入pillow库，用于处理图像
#设置之前收集好的数据文件路径
path = 'data'

#初始化识别的方法
recog = cv2.face.LBPHFaceRecognizer_create()

#调用熟悉的人脸分类器
detector = cv2.CascadeClassifier('haarcascade_frontalface_default.xml')

#创建一个函数，用于从数据集文件夹中获取训练图片,并获取id
#注意图片的命名格式为User.id.sampleNum
def get_images_and_labels(path):
    image_paths = [os.path.join(path,f) for f in os.listdir(path)]
    #新建连个list用于存放
    face_samples = []
    ids = []
    
    #遍历图片路径，导入图片和id添加到list中
    for image_path in image_paths:
        
        #通过图片路径将其转换为灰度图片
        img = Image.open(image_path).convert('L')
        
        #将图片转化为数组
        img_np = np.array(img,'uint8')
        
        if os.path.split(image_path)[-1].split(".")[-1] != 'jpg':
            continue
        
        #为了获取id，将图片和路径分裂并获取
        id = int(os.path.split(image_path)[-1].split(".")[1])
        faces = detector.detectMultiScale(img_np)
        
        #将获取的图片和id添加到list中
        for(x,y,w,h) in faces:
            face_samples.append(img_np[y:y+h,x:x+w])
            ids.append(id)
    return face_samples,ids

#调用函数并将数据喂给识别器训练
print('Training...')
faces,ids = get_images_and_labels(path)
#训练模型
recog.train(faces,np.array(ids))
#保存模型
recog.save('trainner/trainner.yml')


#-----Stage 1-----
#-----检测、校验并输出结果-----

import cv2
import numpy as np

#准备好识别方法
recognizer = cv2.face.LBPHFaceRecognizer_create()

#使用之前训练好的模型
recognizer.read('trainner/trainner.yml')

#再次调用人脸分类器
cascade_path = "haarcascade_frontalface_default.xml" 
face_cascade = cv2.CascadeClassifier(cascade_path)

#加载一个字体，用于识别后，在图片上标注出对象的名字
font = cv2.FONT_HERSHEY_SIMPLEX

idnum = 0
#设置好与ID号码对应的用户名，如下，如0对应的就是初始

names = ['初始','admin','user1','user2','user3']

#调用摄像头
cam = cv2.VideoCapture(0)
minW = 0.1*cam.get(3)
minH = 0.1*cam.get(4)

while True:
    ret,img = cam.read()
    gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
    #识别人脸
    faces = face_cascade.detectMultiScale(
            gray,
            scaleFactor = 1.2,
            minNeighbors = 5,
            minSize = (int(minW),int(minH))
            )
    #进行校验
    for(x,y,w,h) in faces:
        cv2.rectangle(img,(x,y),(x+w,y+h),(0,255,0),2)
        idnum,confidence = recognizer.predict(gray[y:y+h,x:x+w])
        
        #计算出一个检验结果
        if confidence < 100:
            idum = names[idnum]
            confidence = "{0}%",format(round(100-confidence))
        else:
            idum = "unknown"
            confidence = "{0}%",format(round(100-confidence))
       
        #输出检验结果以及用户名
        cv2.putText(img,str(idum),(x+5,y-5),font,1,(0,0,255),1)
        cv2.putText(img,str(confidence),(x+5,y+h-5),font,1,(0,0,0),1)
    
        #展示结果
        cv2.imshow('camera',img)
        k = cv2.waitKey(20)
        if k == 27:
            break

#释放资源
cam.release()
cv2.destroyAllWindows()
 1#include<SoftwareSerial.h>
 2
 3SoftwareSerialBT(12,11);
 4
 5intservopin = 10;
 6
 7voidservopulse(int angle){
 8
 9/*
10
11       该函数转自CSDN，稍作修改了。
12
13原po：https://blog.csdn.net/wzxxtt62267018/article/details/79785690
14
15此处不适用Servo库，会造成舵机在没有指令时鬼畜抖动
16
17*/
18
19  int pulsewidth = ( angle * 11 ) + 500;
20
21  for(int i = 0; i < 50; i++){
22
23  digitalWrite(servopin,HIGH);
24
25  delayMicroseconds(pulsewidth);
26
27  digitalWrite(servopin,LOW);
28
29  delayMicroseconds(20000 - pulsewidth);
30
31  }
32
33}
34
35voidsetup() {
36
37  Serial.begin(9600);
38
39  pinMode(servopin,OUTPUT);
40
41  pinMode(2,OUTPUT);
42
43  digitalWrite(2,HIGH);
44
45  servopulse(0);
46
47  BT.begin(9600);  //设置波特率
48
49  Serial.println("准备工作完成，已就绪"); //调试信息
50
51  BT.listen();
52
53}
54
55
56
57voidloop() {
58
59  if(BT.isListening()){
60
61    if(BT.available() > 0 && BT.read()== 'o'){  //检测接收到的信息是否为开门的相关指令
62
63        Serial.println("已经接收信号");    //调试信息，下同
64
65        for(int i=0;i<50;i++){
66
67          servopulse(135);
68
69        }      //令舵机旋转一定角度以拉动绳子开锁
70
71        Serial.println("舵机正在转动");
72
73        for(int i=0;i<50;i++){
74
75          servopulse(0);
76
77        }      //复位
78
79        Serial.println("舵机正在转动回去");
80
81    }
82
83  }
84
85}
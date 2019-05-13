from time import sleep
import requests

question = input("输入导火线：")
TULING = '图灵'
MOLI = '茉莉'
print(TULING+':'+question)          #导火索  哈哈
while True:
    #图灵
    tuling_data = {
        "key": "5da047a95db8450ea6e710dd065d4be4",
        "info": question,
        "userid": "272872"
    }
    tuling_api_url = 'http://www.tuling123.com/openapi/api'
    t = requests.post(tuling_api_url, data=tuling_data) #post请求

    print(TULING+':'+eval(t.text)["text"]) #用eval函数处理一下图灵返回的消息
    question = eval(t.text)["text"]     #重置question —>让茉莉回答

    #茉莉
    moli_data = {
        "question": question,
        "api_key": "ac00db995a4a8f2a3f3623c82f3cc9d9",
        "api_secret": "anaoutswrz1y"
    }
    moli_api_url = 'http://i.itpk.cn/api.php'       #api地址
    m = requests.post(moli_api_url, data = moli_data)
    print(MOLI+':'+m.text)
    sleep(1)        #设置循环延迟
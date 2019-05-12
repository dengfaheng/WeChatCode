import requests
from bs4 import BeautifulSoup
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.header import Header
import smtplib
import os
def getHTMLText(url):
    try:
        r = requests.get(url)
        print(r.raise_for_status)
        r.encoding = r.apparent_encoding
        return r.text
    except:
        return ""

def parserHTMLWeather(html):
    try:
        dirt = {}
        soup = BeautifulSoup(html,"html.parser")
        place = soup.find(name = "head").find("title")
        dirt["place"] = str(place.string).split("-")[0]
        AnnoceTime = soup.find(name = 'div', attrs = {"class":"btitle"}).find("span")
        dirt["AnnoceTime"] = str(AnnoceTime.string)
        Everyday = AnnoceTime.find_parent().find_next_sibling().find_all(name = "div",class_ = "detail")
        for eachday in Everyday:
            info = eachday.find(name = "div",class_ = "day")
            thisDay = {}
            date = str(info.find(name = "div",class_ = "date").string)
            week = str(info.find(name = "div",class_ = "week").string)
            wdesc = str(info.find(name = "div",class_ = "wdesc").string)
            temp = str(info.find(name = "div",class_ = "temp").string)
            direct = str(info.find(name = "div",class_ = "direct").string)
            wind = str(info.find(name = "div",class_ = "wind").string)

            thisDay["date"] = date
            thisDay["week"] = week
            thisDay["wdesc"] = wdesc
            thisDay["temp"] = temp
            thisDay["direct"] = direct
            thisDay["wind"] = wind
            dirt[thisDay["date"]] = thisDay

        return dirt
    except:
        return {}

def parserHTMLPicture(imag,imagLink):
    try:
        soup = BeautifulSoup(imag,"html.parser")
        imagAddress = soup.find(name='div',class_ = 'photolst clearfix').find_all(name = 'img')
        for image in imagAddress:
            imagLink.append(image['src'])
        
        return imagLink
    except:
        return []

def downloadPicture(url,name):
    root = 'C:\\Users\\10990\\Pictures\\'#这里填保存的路径
    path = root + str(name) + '.jpg'
    try:
        if not os.path.exists(root):
            os.mkdir(root)
        if not os.path.exists(path):
            r = requests.get(url)
            with open(path,'wb') as f:
                f.write(r.content)
                f.close()
                print("文件保存成功")
        else:
            print("文件已存在")
    except:
        print("爬取失败")

def makeMessage(dirt,image):
    #编辑消息
    print(dirt)
    message = dirt["place"]+' 今天 '
    items = {'wdesc','temp','direct','wind'}
    for item in items:
        message += dirt["\n        今天\n       "][item].strip('\n        ')+" "
    for temp in message.split(" "):
        if temp.find("℃") != -1:
            if eval(temp.split("℃")[0]) > 25:
                message += "今天很热,尽量别出门啦"
            elif eval(temp.split("℃")[0]) < 12:
                message += "今天很冷,注意保暖"
    if message.find("雨") != -1:
        message += " 出门的话记得带伞"
    print(message)
    
    #生成邮件对象
    msgRoot = MIMEMultipart('related')
    msgRoot['From'] = Header("我是发信人","utf-8")
    msgRoot['To'] = Header('我是收信人','utf-8')
    subject = '赴戍登程口占示家人'
    msgRoot['Subject'] = Header(subject,'utf-8')

    msgAlternative = MIMEMultipart('alternative')
    msgRoot.attach(msgAlternative)
    
    mail_msg = '''
    <p> 力微任重久神疲，再竭衰庸定不支。
        苟利国家生死以，岂因祸福避趋之？
        谪居正是君恩厚，养拙刚于戍卒宜。
        戏与山妻谈故事，试吟断送老头皮。
    </p>
    <p>'''+message+'''</p>
    <p><img src = "cid:image1"></p>
'''
    msgAlternative.attach(MIMEText(mail_msg,'html','utf-8'))
    
    catalog = 'C:\\Users\\10990\\Pictures\\' + str(image) + ".jpg"
    #指定图片为当前目录
    with open(catalog,'rb') as fp:
        msgImage = MIMEImage(fp.read())
        fp.close()

    #定义图片在ID,在HTML文本中引用
    msgImage.add_header('Content-ID','<image1>')
    msgRoot.attach(msgImage)
    return msgRoot

def sendMsg(message):
    mail_host = "smtp.qq.com"#要使用的smtp服务器
    mail_user = "*******"#用户名和密码
    mail_pass = "********"
    sender = '********'#发送者
    receivers = ['*******']#收信者，注意这里是一个列表，就是说可以群发，当然劝君莫浪~~
    try:
        smtpObj = smtplib.SMTP()
        smtpObj.connect(mail_host)
        smtpObj.ehlo()
        smtpObj.starttls()
        smtpObj.login(mail_user,mail_pass)
        smtpObj.sendmail(sender,receivers,message.as_string())
        print("邮件发送成功")
        smtpObj.quit()
    except smtplib.SMTPException:
        print("Error:无法发送邮件")
def main():
#    print("河北HE 内蒙古NM 陕西SN 黑龙江HL 河南HA")
#    province = input("input the province,the big alpha for short:")
#    city = input("input the city you wanna search plz:")
    province = "HB"
    city = "wuhan"
    url = "http://www.nmc.cn/publish/forecast/A" + province + "/" + city + ".html"
    html = getHTMLText(url)
    url = "https://www.douban.com/photos/album/157693223/"
image = getHTMLText(url)
imagLink = []
    whetherInfo = parserHTMLWeather(html)
    name = 1
    for image in imagLink:
        print(image)
    for image in imagLink:
        downloadPicture(image,name)
        name += 1
with open('pictureName.txt','r') as f:
        name = eval(f.read())
        f.close()
    with open('pictureName.txt','w') as f:
        newName = str(name + 1)
        f.write(newName)
        f.close()
    msgRoot = makeMessage(whetherInfo,name)
sendMsg(msgRoot)
main()

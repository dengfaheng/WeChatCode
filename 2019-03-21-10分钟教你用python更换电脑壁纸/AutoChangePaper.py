import requests
from bs4 import BeautifulSoup
import os
import time
import win32api
import win32con
import win32gui
def getHTML(url):
    try:
        r = requests.get(url)
        print("get html successfully")
        r.raise_for_status()
        r.encoding = r.apparent_encoding
        #print(r.text)
        return r.text
    except:
        return ""
def parserHTML(html):
    try:
        soup = BeautifulSoup(html,"html.parser")
        print(soup.find('a',attrs={'class':'preview'})['href'])
        return soup.find('a',attrs={'class':'preview'})['href']
    except:
        pass
def getPhoto(html):
    try:
        soup = BeautifulSoup(html,"html.parser")
        print(soup.find('img',attrs={'id':'wallpaper'})['src'])
        return soup.find('img',attrs={'id':'wallpaper'})['src']
    except:
        pass
def savePicture(imag):
    root = 'C:\\Users\\10990\\Pictures\\DesktopPaper\\'
    path = root + imag.split('/')[-1]
    try:
        if not os.path.exists(root):
            os.mkdir(root)
        if not os.path.exists(path):
            print("downloading picture")
            r = requests.get('http:' + imag)
            print("downloaded")
            with open(path,'wb') as f:
                f.write(r.content)
                time.sleep(5)
                f.close()
                print("Picture Saved Successfully")
        else:
            print("Picture Exist")
        return path
    except:
        print("fail to get the picture")
def setWallPaper(Imag):
    # 打开指定注册表路径
    reg_key = win32api.RegOpenKeyEx(win32con.HKEY_CURRENT_USER, "Control Panel\\Desktop", 0, win32con.KEY_SET_VALUE)
    # 最后的参数:2拉伸,0居中,6适应,10填充,0平铺
    win32api.RegSetValueEx(reg_key, "WallpaperStyle", 0, win32con.REG_SZ, "2")
    # 最后的参数:1表示平铺,拉伸居中等都是0
    win32api.RegSetValueEx(reg_key, "TileWallpaper", 0, win32con.REG_SZ, "0")
    # 刷新桌面
    win32gui.SystemParametersInfo(win32con.SPI_SETDESKWALLPAPER,Imag, win32con.SPIF_SENDWININICHANGE)
def main():
    keyword = input()
    url = 'https://alpha.wallhaven.cc/search?q=' + keyword + '&categories=111&purity=110&sorting=random&order=desc'
    print("grasping photo from wallhaven...")
    HTMLpage = ''
    while HTMLpage == '':
        HTMLpage = getHTML(url)
    print("grasp web successfully")
    photoAddress = parserHTML(HTMLpage)
    photo = getHTML(photoAddress)
    if photo != '':
        imag = getPhoto(photo)
        LocalImag = savePicture(imag)
        setWallPaper(LocalImag)
    
if __name__ == '__main__':
    main()

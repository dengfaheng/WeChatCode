import requests
from bs4 import BeautifulSoup
def getHTML(url):
    try:
        r = requests.get(url)
        r.raise_for_status()
        print("get html successfully")
        r.encoding = 'utf-8'
        #print(r.text)
        return r.text
    except:
        return ""
def parseHTML(html):
    try:
        soup = BeautifulSoup(html,"html.parser")
        A = soup.find_all('span',attrs = {'class':'short'})
        B = []
        for i in A:
            B.append(i.get_text())
        return B
    except:
        return []
def main():
    discuss = []
    a = 0
    for i in range(0,100,20):
        url = 'https://movie.douban.com/subject/26100958/comments?start='+ str(i) +'&limit=20&sort=new_score&status=P'
        HTMLpage = getHTML(url)
        #print(HTMLpage)
        for t in parseHTML(HTMLpage):
            discuss.append(t)
    for i in discuss:
        print(str(a) + ':' + i)
#        print(i)
        a = a + 1
        
if __name__ == "__main__":
    main()

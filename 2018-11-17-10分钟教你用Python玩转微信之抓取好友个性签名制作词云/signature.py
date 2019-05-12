import itchat
import re
import jieba
import matplotlib.pyplot as plt
from wordcloud import WordCloud

itchat.login()
friends = itchat.get_friends(update=True)[0:]
tList = []
for i in friends:
    signature = i["Signature"].replace(" ", "").replace("span", "").replace("class", "").replace("emoji", "")
    rep = re.compile("< =.+/>")
    signature = rep.sub("", signature)
    tList.append(signature)

# 拼接字符串
text = "".join(tList)

# jieba分词
wordlist_jieba = jieba.cut(text, cut_all=True)
wl_space_split = " ".join(wordlist_jieba)

# 这里要选择字体存放路径，这里是Mac的，win的字体在windows／Fonts中
# #如果是中文必须要添加字体路径，否则会显示成框框
my_wordcloud = WordCloud(background_color="white", width=1600, height=1200,
                         font_path='C:\Windows\Fonts\苹方\PingFang Bold_0.ttf').generate(wl_space_split)

plt.imshow(my_wordcloud)
plt.axis("off")
plt.show()
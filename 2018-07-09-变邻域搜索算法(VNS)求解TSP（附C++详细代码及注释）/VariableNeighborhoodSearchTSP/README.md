# 00 前言
上次变邻域搜索的推文发出来以后，看过的小伙伴纷纷叫好。小编大受鼓舞，连夜赶工，总算是完成了手头上的一份关于变邻域搜索算法解TSP问题的代码。今天，就在此给大家双手奉上啦，希望大家能ENJOY哦！

# 01 代码说明
本次代码还是基于**求解TSP旅行商**问题的。至于什么是TSP问题，小编这实在是不想科普啦……

![](http://upload-images.jianshu.io/upload_images/10386940-04bddfb09b2e458a?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

**代码是基于迭代搜索那个代码魔改过来的。**其实看了这么多启发式算法解TSP问题的代码。想必各位都有了一个比较清晰的认识，其实呀。**之前介绍的模拟退火、遗传算法、迭代搜索和现在的变邻域等等，是十分相似滴。**最大的不同在于算法框架的不同而已，像什么扰动啦，邻域动作啦。代码基本是不变的。所以大家可以多联想，多思考，**学习就是一个探求事物本质的过程嘛！**

至于算法框架什么的概念，大家看上一篇关于VNS的推文啦。

这里就不做过多介绍了。再次贴一下伪代码。代码是基于伪代码写的。不过本文的代码只做了一个shaking的邻域，vnd的邻域做了两个。这里给大家说明一下。

![](http://upload-images.jianshu.io/upload_images/10386940-a3495db27f4afd6b?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

简要说说算法vnd里面两个邻域使用的算子：

**two_opt_swap**

没啥好说的，区间反转。直接上图：

![](http://upload-images.jianshu.io/upload_images/10386940-9958b8bb8d583de2?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

**two_h_opt_swap**

还是要说一点，随机产生两点，塞进新排列头部。其余的按顺序往后逐个塞进去。嗯，来看图片~

![](http://upload-images.jianshu.io/upload_images/10386940-cf0da50cd1d48dba?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

**下面上代码啦！欲直接下载代码文件，关注我们的公众号哦！回复【VNSTSP】即可，不包括【】哦**


![](http://upload-images.jianshu.io/upload_images/10386940-ba0c519723650398.jpg?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

# 01 前言
经过小编这几天冒着挂科的风险，日日修炼，终于赶在考试周中又给大家更新了一篇干货文章。关于用变邻域搜索解决0-1背包问题的代码。怎样，大家有没有很感动？

# 02 什么是0-1背包问题？
> 0-1 背包问题：给定 n 种物品和一个容量为 C 的背包，物品 i 的重量是** w_i，其价值为 v_i **。
> 问：应该如何选择装入背包的物品，使得装入背包中的物品的总价值最大？

为什么叫0-1背包问题呢？显然，面对每个物品，我们只有选择拿取或者不拿两种选择，不能选择装入某物品的一部分，也不能装入同一物品多次。拿就是1，不拿就是0。因此，就叫0-1背包问题。So simple, so naive. 

![](http://upload-images.jianshu.io/upload_images/10386940-876f51df94ae8915?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

# 03 代码小讲解
下面就几个邻域小动作给大家讲解一下。

![](http://upload-images.jianshu.io/upload_images/10386940-8fcbbfd66735e422?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

#### 解决方案设计

假设我们面前有n种物品，那么我们可以将解决方案设置成一个一维数组selection[n]。数组weights[n]表示重量，数组values[n]表示价值。

*   selection[i] = 1 表示装入第i个物品。

*   selection[i] = 0 表示不装入第i个物品。

*   总价值total_value  = selection[i] * values[i]。 (i=1,2,3,4……n)

*   总重量total_weight = selection[i] * weights[i]。(i=1,2,3,4……n)

#### 邻域动作1

将解决方案selection[n]的第i位**取反**(i=1,2,3,4……n)。比如：

> 有方案0010，那么生成的邻居解则有1010(第一位取反)、0110(第二位取反)、0000(第三位取反)、0011(第四位取反)。

不知道这么通俗易懂的大家understand了没有。

#### 邻域动作2

对于解决方案selection[n]，**在第i  (i=1,2,3,4……n)位取反的情况下**，依次将第j ** (j=i+1,2,3,4……n)位也取反**。还是for 一个 example吧。

对于解决方案0010。产生的邻居解如下：

![image](http://upload-images.jianshu.io/upload_images/10386940-1307afb739e3ac6a?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

#### 邻域动作3

交换第i位和第i-3位的数。如果i<3。就交换最后的，比如：

*   selection[0]和selection[n-1]交换。

*   selection[1]和selection[n-2]交换。

*   selection[2]和selection[n-3]交换。

#### shaking程序

这个比较简单，随机取反一些位就行了。

**下面上代码啦！欲直接下载代码文件，关注我们的公众号哦！回复【VNSKP】即可，不包括【】哦**


![](http://upload-images.jianshu.io/upload_images/10386940-ba0c519723650398.jpg?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)
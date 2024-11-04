# 类设计：

IO：输入输出

GameModel：没有成员变量，只提供几个工具函数检查游戏状态

Properity：通过map和一个Chess映射，存储节点的属性值

MCTS：主要算法，通过接受Chess进行计算搜索

Chess：存储当前棋盘的状态，以及最后一个落点

ConcurrencyCaluate：和并发没有关系，给出了选择算法和价值评估的方法： 这是一个后加上的评估函数，使用专业知识进行评估  可以修改较多

# 待办事项

- √√√√√√√删除judgeAll()相关的代码，提高速度  

- √√√√√√√解决player = 1 还是 0 的问题，先后后手问题  
  - 相关的变量： nowblack  player  
  - USB()  bestchild()  UCTsearch()  treePolicy()  expandNode(player)
  - UCTsearch__ -> treePolicy__ ->  expandNode__
  - ​                                       -> UCB__
  - ​                                        ->conFun__ -> defaultPolicy__
  - ​                    -> bestChild__
  - judge__

- bestchild() 中的goodNext的使用

- ChooseCnt的作用是什么？  选择作用   当搜索次数较少的时候

- point替代int x   int y

- √√√√√√√使用Hash  unordered_map  经过测试 发现Hashmap和map效率几乎相同，但map要更高一点。。。

- 使用cerr的输出位置是标准错误流，不是标准输出流。。。。 可能是因为这个原因导致输出没有被平台识别到

- 使用基于策略的算法进行辅助判断       最后把MCTS的优先级结果进行改变

- 修改simulation()函数

- UCT    <-   selection     需要改写UCT的内容，增加局面评估函数   改变value的值

  ​            <-    bestChild




# 可能的优化方向

- 根据先手后手进行优化
- 缺乏终结比赛的能力![73063579523](C:\Users\asus\Desktop\gobang\gobang-2024\assets\1730635795239.png)



# alpha  beta 优化

- 剪枝       增加搜索深度和次数   
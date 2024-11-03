# 类设计：

IO：输入输出

GameModel：没有成员变量，只提供几个工具函数检查游戏状态

Properity：通过map和一个Chess映射，存储节点的属性值

MCTS：主要算法，通过接受Chess进行计算搜索

Chess：存储当前棋盘的状态，以及最后一个落点

ConcurrencyCaluate：和并发没有关系，给出了选择算法和价值评估的方法。

# 待办事项

- 删除judgeAll()相关的代码，提高速度
- 解决player = 1 还是 0 的问题，先后后手问题
  - 相关的变量： nowblack  player  
  - USB()  bestchild()  UCTsearch()  treePolicy()  expandNode(player)
  - UCTsearch__ -> treePolicy__ ->  expandNode__
  - ​                                       -> UCB__
  - ​                                        ->conFun__ -> defaultPolicy__
  - ​                    -> bestChild__
  - judge__




# 可能的优化方向

- 在一些情况，过于强调封堵，没有运营
- 根据先手后手进行优化
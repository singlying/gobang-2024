#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <limits>
#include <iostream>
#include <random>
#include <map>
#include <sstream>
#include <vector>
using namespace std;

#define point pair<int, int>  // 点

// input
#define START 0
#define PLACE 1
#define TURN 2
#define END 3

// 基本参数
#define BLACK 1  // 黑白棋
#define WHITE 2
#define BOARD_SIZE 12  // 边界
#define MIN 0

constexpr static int boxNum = 11;

// 输入输出
class IO {
public:
    static int getCommand(const std::string& command) {
        static const std::map<std::string, int> commandMap =
        { {"START", START}, {"PLACE", PLACE}, {"TURN", TURN} };

        auto it = commandMap.find(command);
        if (it != commandMap.end()) {
            return it->second;  // 返回对应的宏值
        }
        else {
            return END;  // 未找到指令返回 END
        }
    }

    // 测试 printf 哪个更快？ // \n 和 endl 那个更快？
    static void output_OK() { cerr << "OK\n"; }
    static void output_PLACE(const point p) {
        cerr << (int)p.first << ' ' << (int)p.second << '\n';
    }
    static void output_DEBUG(const string& message) { cerr << "DEBUG " << message << '\n'; }
};

class Chess
{
private:
    int gomoku[boxNum + 1][boxNum + 1];
    std::pair<int, int> lastPoint;

public:
    /// player = 1白棋 / 2黑棋   这里整个是反的！！！！

    void setChess(int x, int y, int player) {
        gomoku[x][y] = player;
        lastPoint = std::make_pair(x, y);
    }

    int getChess(int x, int y) {
        return gomoku[x][y];
    }

    /// 返回最后一次下的棋子坐标
    std::pair<int, int> getLastPoint() {
        return lastPoint;
    }

    std::vector<std::vector<int>> convertGomokuToVec() const {
        std::vector<std::vector<int>> temp;

        temp.clear();
        for (int i = 0; i <= boxNum; i++)
        {
            std::vector<int> lineBoard;
            for (int j = 0; j <= boxNum; j++)
                lineBoard.push_back(0);
            temp.push_back(lineBoard);
        }


        for (int i = 0; i <= boxNum; i++)
            for (int j = 0; j <= boxNum; j++)
            {
                if (gomoku[i][j] == 1)
                    temp[i][j] = 1;
                else if (gomoku[i][j] == 2)
                    temp[i][j] = -1;
                else
                    temp[i][j] = 0;
            }
        return temp;
    }

public:
    Chess() {
        lastPoint = std::make_pair(0, 0);

        for (int i = 0; i < boxNum + 1; ++i)
            for (int j = 0; j < boxNum + 1; ++j)
                gomoku[i][j] = 0;
    }

    Chess(int rgomoku[boxNum + 1][boxNum + 1]) {
        lastPoint = std::make_pair(0, 0);

        for (int i = 0; i < boxNum + 1; ++i)
            for (int j = 0; j < boxNum + 1; ++j)
                gomoku[i][j] = rgomoku[i][j];
    }

public:

    friend bool operator<(Chess x, Chess y)//用于搜索棋盘状态
    {
        for (int i = 0; i <= boxNum; i++)
        {
            for (int j = 0; j <= boxNum; j++)
            {
                if (x.gomoku[i][j] < y.gomoku[i][j])
                    return 1;
                else if (x.gomoku[i][j] > y.gomoku[i][j])
                    return 0;
            }
        }
        return 0;
    }

    friend bool operator==(Chess x, Chess y)//用于判断棋盘状态是否相等
    {
        for (int i = 0; i <= boxNum; i++)
        {
            for (int j = 0; j < boxNum; j++)
            {
                if (x.gomoku[i][j] != y.gomoku[i][j])
                    return 0;
            }
        }
        return 1;
    }
};

class GameModel
{
public:
    GameModel() = default;

    // 判断（x,y）处nowwhile是否赢
    static bool judge(Chess chess, int x, int y, bool nowWhite) {
        int count = 0;
        int flag = nowWhite ? 1 : 2;
        //横向
        for (int i = ((x - 4) < 0 ? 0 : x - 4); i <= ((x + 4) > boxNum ? boxNum : x + 4); ++i)
        {
            if (chess.getChess(i, y) == flag)
                count++;
            else
                count = 0;
            if (count == 5)
                return true;
        }
        //纵向
        count = 0;
        for (int j = ((y - 4) < 0 ? 0 : y - 4); j <= ((y + 4) > boxNum ? boxNum : y + 4); ++j)
        {
            if (chess.getChess(x, j) == flag)
                count++;
            else
                count = 0;
            if (count == 5)
                return true;
        }
        //左下到右上
        count = 0;
        for (int i = ((x - 4) < 0 ? 0 : x - 4); i <= ((x + 4) > boxNum ? boxNum : x + 4); ++i)
        {
            int yy = y + i - x;

            if (yy < 0)
                continue;
            else if (yy > boxNum)
                break;

            if (chess.getChess(i, yy) == flag)
                count++;
            else
                count = 0;
            if (count == 5)
                return true;
        }
        //左上到右下
        count = 0;
        for (int i = ((x - 4) < 0 ? 0 : x - 4); i <= ((x + 4) > boxNum ? boxNum : x + 4); ++i)
        {
            int yy = y + x - i;

            if (yy > boxNum)
                continue;
            else if (yy < 0)
                break;

            if (chess.getChess(i, yy) == flag)
                count++;
            else
                count = 0;
            if (count == 5)
                return true;
        }

        return false;
    }

    // 1代表白棋赢，2代表黑棋赢，0代表没结束
    static int judgeAll(Chess chess) {
        for (int i = 0; i <= boxNum; i++)
            for (int j = 0; j <= boxNum; j++)
            {
                if (judge(chess, i, j, true))
                    return 1;
                else if (judge(chess, i, j, false))
                    return 2;
            }

        return 0;
    }

    // 返回1时代表棋盘满了，是终端节点 0 棋盘没有满
    static int is_terminate(Chess chess) {
        for (int i = 0; i <= boxNum; i++)
            for (int j = 0; j <= boxNum; j++)
                if (!chess.getChess(i, j))
                    return 0;

        return 1;
    }
};

// nowWhite 用于UI判断      1为白色 0为黑色
// nowblack 用于MCTS内部判断 1为黑色 0为白色
// player   用于判断用户     0为空位 1为白色 2为黑色

// 每个节点的属性
class Properity
{
public:
    double mockNum; // 模拟次数
    double value; // 节点价值
    std::vector<Chess> vec;  // 关联的棋子
};

class MCTS
{
public:
    ////////////using Tree = MultiwayTree<_Multiway_tree_traits<Chess>>;
    ////////////Tree tree;

    static const int searchRange = 2;       ///< 搜索范围
    static const int selectNum = 50;        ///< 选择次数
    static const int simulationNum = 8;     ///< 每个状态模拟次数

    std::map<Chess, Properity> mp; // 每个棋局的状态及其对应的属性
    std::map<Chess, Chess> fa;  // 父节点

    int chooseCnt; // 选择次数

    // 初始化棋局
    void initChess(Chess chess) {
        Properity p;
        p.value = 0, p.mockNum = 0;
        mp[chess] = p;
    }
    static void initDoubleVector(std::vector<std::vector<int>>& rhs) {
        rhs.clear();
        for (int i = 0; i <= boxNum; i++)
        {
            std::vector<int> lineBoard;
            for (int j = 0; j <= boxNum; j++)
                lineBoard.push_back(0);
            rhs.push_back(lineBoard);
        }
    }

public:
    MCTS() = default;

    // 实现树的搜索，返回最优的落子位置
    /// 搜索函数传入的是原来那个chess
    Chess UCTsearch(Chess chess, std::pair<int, int> center, int player) {
        if (mp.find(chess) == mp.end())
            initChess(chess);

        fa.clear();

        ConcurrencyCaluate choose;
        goodNext = choose.bestChildPro(chess);
        root = chess;
        mp.clear();

        chooseCnt = 0; // 选择次数
        while (chooseCnt <= selectNum)
        {
            chooseCnt++;
            std::pair<Chess, int> selectPoint = treePolicy(chess, center, player);

            /////////////// 移除线程池相关代码
            for (int i = 1; i <= simulationNum; i++)
                Confun(selectPoint.first, selectPoint.second ^ 1, chess);
        }

        IO::output_DEBUG("root: " + to_string(mp[root].value));

        for (auto& it : mp[root].vec)
        {
            IO::output_DEBUG("child:" + to_string(mp[it].value));
        }

        Chess ans = bestChild(chess, player);
        return ans;
    }

    /// @brief 选择
    std::pair<Chess, int> treePolicy(Chess chess, std::pair<int, int> center, int player) {

        while (!GameModel::is_terminate(chess) && !GameModel::judgeAll(chess)) // 这里可以删除掉
        {
            int x1 = std::max(0, center.first - searchRange);
            int x2 = std::min(boxNum, center.first + searchRange);
            int y1 = std::max(0, center.second - searchRange);
            int y2 = std::min(boxNum, center.second + searchRange);

            ////////////////if (cntNum(chess, x1, x2, y1, y2) + tree.find(chess).getChildren().size() < (x2 - x1 + 1) * (y2 - y1 + 1))
            if (cntNum(chess, x1, x2, y1, y2) + mp[chess].vec.size() < (x2 - x1 + 1) * (y2 - y1 + 1))
            {
                return std::make_pair(expandNode(chess, center, player), player); /////////////// 这里的返回值需要改
            }
            else
            {
                Chess y = chess;
                std::vector<Chess>::iterator it;

                ////////////////if (tree.find(y).getChildren().size() == 0)break;
                if (mp[y].vec.size() == 0)break;
                double maxn = -std::numeric_limits<double>::infinity();

                ////////////////auto IterChildVec = tree.find(y).getChildren();
                ////////////////for (auto it = IterChildVec.begin(); it != IterChildVec.end(); it++)
                for (auto it = mp[y].vec.begin(); it != mp[y].vec.end(); it++)
                {
                    if (UCB(*it, player) >= maxn)///////////////原来是一个*
                    {
                        maxn = UCB(*it, player);///////////////原来是一个*
                        chess = *it;///////////////原来是一个*
                    }
                }
                fa[chess] = y;
            }
            player ^= 1;
        }
    }

    /// @brief 扩展
    /// @attention 目前只有5*5的范围内搜索，搜索100000次，如果运气非常非常非常非常不好就expand不到那1/25的点~
    Chess expandNode(Chess chess, std::pair<int, int> center, int nowblack) {
        Chess y = chess;

        int x1 = std::max(0, center.first - searchRange);
        int x2 = std::min(boxNum, center.first + searchRange);
        int y1 = std::max(0, center.second - searchRange);
        int y2 = std::min(boxNum, center.second + searchRange);

        int putCnt = 0;
        while (putCnt <= 10000)
        {
            int i = x1 + rand() % (x2 - x1 + 1);
            int j = y1 + rand() % (y2 - y1 + 1);
            int o = chess.getChess(i, j);
            y.setChess(i, j, nowblack);
            if (!chess.getChess(i, j) && mp.find(y) == mp.end())
            {
                if (goodNext == y) // 特殊情况
                {
                    initChess(y);
                    mp[y].value += 1000;
                    mp[chess].vec.push_back(goodNext);
                    fa[y] = chess;
                    ///////////////tree.find(chess).addChild(goodNext);
                    return y;
                }


                ///////////////initChess(y);
                ///////////////tree.find(chess).addChild(y);
                initChess(y);
                mp[chess].vec.push_back(y);
                fa[y] = chess;
                return y;
            }
            y.setChess(i, j, o);
            putCnt++;
        }
    }


    /// <summary>
    /// bestChildPro()侧重于评估并选择下一步的落子，而bestChild()则是基于已有的搜索结果来选择最佳子节点。
    /// 前者是一个实时决策过程，后者则是在已有树结构的基础上进行选择。
    /// </summary>
    Chess bestChild(Chess chess, int nowblack) {
        Chess ans = chess;
        std::vector<Chess>::iterator it;
        double maxn = -std::numeric_limits<double>::infinity(); /// 比最小值还小才行
        ////////////////auto IterChildVec = tree.find(chess).getChildren();
        ///////////////for (auto it = IterChildVec.begin(); it != IterChildVec.end(); it++)
        for (auto it = mp[chess].vec.begin(); it != mp[chess].vec.end(); it++)
        {
            if (UCB(*it, nowblack) >= maxn)///////////////原来是一个*
            {
                maxn = UCB(*it, nowblack);///////////////原来是一个*
                ans = *it;///////////////原来是一个*
            }
        }
        if (chooseCnt >= 25)
        {
            std::vector<Chess>::iterator iter = std::find(mp[root].vec.begin(), mp[root].vec.end(), goodNext);
            if (iter == mp[root].vec.end())
            {
                mp[chess].vec.push_back(goodNext);
                ///////////////tree.find(chess).addChild(goodNext);
                ans = goodNext;
            }
        }

        return ans;
    }

    /// @brief 模拟    游戏至结束，黑色赢返回1，白色返回-1，和棋返回0
    void defaultPolicy(Chess chess, int nowblack, int& value) {
        while (1)
        {
            if (GameModel::judgeAll(chess) || GameModel::is_terminate(chess))
                break;
            std::pair<int, int> h = calCenter(chess);

            if (nowblack)
            {
                ConcurrencyCaluate cal;
                chess = cal.bestChildPro(chess);
                nowblack ^= 1;
            }

            int randNum = rand() % 100;
            int i = 0, j = 0;
            if (randNum < 50)
            {
                i = std::min(std::max(0, h.first - searchRange + rand() % (searchRange * 2 + 1)), boxNum);
                j = std::min(std::max(0, h.second - searchRange + rand() % (searchRange * 2 + 1)), boxNum);
            }
            else
            {
                i = rand() % (boxNum + 1);
                j = rand() % (boxNum + 1);
            }
            if (!chess.getChess(i, j))
            {
                chess.setChess(i, j, nowblack + 1);
                nowblack ^= 1;
            }
        }

        if (GameModel::judgeAll(chess) == 1)
            value = -1;
        else if (GameModel::judgeAll(chess) == 2)
            value = 1;
        else
            value = 0;
    }

    /// @brief 回退
    void backUp(Chess x, Chess y, int value) {
        mp[x].value += value;
        mp[x].mockNum++;
        while (!(x == y))
        {
            if (fa.find(x) == fa.end())
                break;
            x = fa[x];

            ////////////////if (x == *tree.getRoot())
            ////////////////    break;
            ////////////////x = *tree.find(x).getParent();

            mp[x].value += value;
            mp[x].mockNum++;
        }
    }

    /// @brief UCB值 计算用于选择下一个节点
    double UCB(Chess chess, int player) {
        if (mp[chess].mockNum == 0)
            return 0;
        double val = mp[chess].value, mocknum = mp[chess].mockNum;
        if (val + mocknum == 0)
            return -std::numeric_limits<double>::infinity();
        if (player == 1)    // black
            return val / mocknum + sqrt(log(mocknum) / mocknum);
        else if (player == 0) // white
            return -val / mocknum + sqrt(log(mocknum) / mocknum);
    }

    void Confun(Chess chess, int nowblack, Chess y)
    {
        int val;
        defaultPolicy(chess, nowblack, val);  // 选择
        backUp(chess, y, val);           // 回溯
    }

    //    std::pair<int, std::pair<int, int>> checkFastwin_1(Chess chess);
    //    std::pair<int, std::pair<int, int>> checkFastwin_2(Chess chess);

    // 返回当前局面平均坐标
    static std::pair<int, int> calCenter(Chess chess) {
        int cnt = 0, p1 = 0, p2 = 0;
        for (int i = 0; i <= boxNum; i++)
            for (int j = 0; j <= boxNum; j++)
            {
                if (chess.getChess(i, j))
                {
                    cnt++;
                    p1 += i;
                    p2 += j;
                }
            }

        p1 = std::max(0, p1 / cnt);
        p2 = std::max(0, p2 / cnt);
        return std::make_pair(p1, p2);
    }

    // 计算某个区域内棋子的数量
    int cntNum(Chess chess, int x1, int x2, int y1, int y2) {
        int sum = 0;
        for (int i = x1; i <= x2; i++)
            for (int j = y1; j <= y2; j++)
                if (chess.getChess(i, j))
                    sum++;

        return sum;
    }

    Chess root; // 当前局面
    Chess goodNext; // 最好的下一步
};

// 处理并发计算
class ConcurrencyCaluate
{
public:

    std::vector<std::vector<int>> gameMapVec; // 存储当前游戏棋盘和棋子的情况,空白为0，白子1，黑子-1
    std::vector<std::vector<int>> scoreMapVec; // 存储各个点位的评分情况，作为AI下棋依据

    // 大概思路：遍历所有的没有遍历所有没有下棋的点，然后计算每个点的评分，选择评分最高的点下棋
    Chess bestChildPro(Chess chess) {
        MCTS::initDoubleVector(gameMapVec);
        MCTS::initDoubleVector(scoreMapVec);

        gameMapVec = chess.convertGomokuToVec();
        calculateScore();

        // 从评分中找出最大分数的位置
        int maxScore = 0;
        std::vector<std::pair<int, int>> maxPoints;

        for (int row = 0; row <= boxNum; row++)
            for (int col = 0; col <= boxNum; col++)
            {
                // 前提是这个坐标是空的
                if (gameMapVec[row][col] == 0)
                {
                    if (scoreMapVec[row][col] > maxScore)          // 找最大的数和坐标
                    {
                        maxPoints.clear();
                        maxScore = scoreMapVec[row][col];
                        maxPoints.push_back(std::make_pair(row, col));
                    }
                    else if (scoreMapVec[row][col] == maxScore)     // 如果有多个最大的数，都存起来
                        maxPoints.push_back(std::make_pair(row, col));
                }
            }

        int index = rand() % maxPoints.size(); // 从多个最大的数中 随机挑选一个

        std::pair<int, int> pointPair = maxPoints.at(index);

        chess.setChess(pointPair.first, pointPair.second, 2); // AI棋子

        return chess;
    }

    /// @brief 价值算法
    void calculateScore() {
        // 统计玩家或者电脑连成的子
        int personNum = 0; // 玩家连成子的个数
        int botNum = 0; // AI连成子的个数
        int emptyNum = 0; // 各方向空白位的个数

        // 清空评分数组
        scoreMapVec.clear();
        for (int i = 0; i <= boxNum; i++)
        {
            std::vector<int> lineScores;
            for (int j = 0; j <= boxNum; j++)
                lineScores.push_back(0);
            scoreMapVec.push_back(lineScores);
        }

        // 计分（此处是完全遍历，其实可以用bfs或者dfs加减枝降低复杂度，通过调整权重值，调整AI智能程度以及攻守风格）
        for (int row = 0; row <= boxNum; row++)
            for (int col = 0; col <= boxNum; col++)
            {
                // 空白点就算
                if (row > 0 && col > 0 &&
                    gameMapVec[row][col] == 0)
                {
                    // 遍历周围八个方向
                    for (int y = -1; y <= 1; y++)
                        for (int x = -1; x <= 1; x++)
                        {
                            // 重置
                            personNum = 0;
                            botNum = 0;
                            emptyNum = 0;

                            // 原坐标不算
                            if (!(y == 0 && x == 0))
                            {
                                // 每个方向延伸4个子

                                // 对玩家白子评分（正反两个方向）
                                for (int i = 1; i <= 4; i++)
                                {
                                    if (row + i * y >= 0 && row + i * y <= boxNum &&
                                        col + i * x >= 0 && col + i * x <= boxNum &&
                                        gameMapVec[row + i * y][col + i * x] == 1) // 玩家的子
                                    {
                                        personNum++;
                                    }
                                    else if (row + i * y >= 0 && row + i * y <= boxNum &&
                                        col + i * x >= 0 && col + i * x <= boxNum &&
                                        gameMapVec[row + i * y][col + i * x] == 0) // 空白位
                                    {
                                        emptyNum++;
                                        break;
                                    }
                                    else            // 出边界
                                        break;
                                }

                                for (int i = 1; i <= 4; i++)
                                {
                                    if (row - i * y >= 0 && row - i * y <= boxNum &&
                                        col - i * x >= 0 && col - i * x <= boxNum &&
                                        gameMapVec[row - i * y][col - i * x] == 1) // 玩家的子
                                    {
                                        personNum++;
                                    }
                                    else if (row - i * y >= 0 && row - i * y <= boxNum &&
                                        col - i * x >= 0 && col - i * x <= boxNum &&
                                        gameMapVec[row - i * y][col - i * x] == 0) // 空白位
                                    {
                                        emptyNum++;
                                        break;
                                    }
                                    else            // 出边界
                                        break;
                                }

                                if (personNum == 1)                      // 杀二
                                    scoreMapVec[row][col] += 10;
                                else if (personNum == 2)                 // 杀三
                                {
                                    if (emptyNum == 1)
                                        scoreMapVec[row][col] += 30;
                                    else if (emptyNum == 2)
                                        scoreMapVec[row][col] += 40;
                                }
                                else if (personNum == 3)                 // 杀四
                                {
                                    // 量变空位不一样，优先级不一样
                                    if (emptyNum == 1)
                                        scoreMapVec[row][col] += 60;
                                    else if (emptyNum == 2)
                                        scoreMapVec[row][col] += 110;
                                }
                                else if (personNum == 4)                 // 杀五
                                    scoreMapVec[row][col] += 10100;

                                // 进行一次清空
                                emptyNum = 0;

                                // 对AI黑子评分
                                for (int i = 1; i <= 4; i++)
                                {
                                    if (row + i * y >= 0 && row + i * y <= boxNum &&
                                        col + i * x >= 0 && col + i * x <= boxNum &&
                                        gameMapVec[row + i * y][col + i * x] == 1) // 玩家的子
                                    {
                                        botNum++;
                                    }
                                    else if (row + i * y >= 0 && row + i * y <= boxNum &&
                                        col + i * x >= 0 && col + i * x <= boxNum &&
                                        gameMapVec[row + i * y][col + i * x] == 0) // 空白位
                                    {
                                        emptyNum++;
                                        break;
                                    }
                                    else            // 出边界
                                        break;
                                }

                                for (int i = 1; i <= 4; i++)
                                {
                                    if (row - i * y >= 0 && row - i * y <= boxNum &&
                                        col - i * x >= 0 && col - i * x <= boxNum &&
                                        gameMapVec[row - i * y][col - i * x] == -1) // AI的子
                                    {
                                        botNum++;
                                    }
                                    else if (row - i * y >= 0 && row - i * y <= boxNum &&
                                        col - i * x >= 0 && col - i * x <= boxNum &&
                                        gameMapVec[row - i * y][col - i * x] == 0) // 空白位
                                    {
                                        emptyNum++;
                                        break;
                                    }
                                    else            // 出边界
                                        break;
                                }

                                if (botNum == 0)                      // 普通下子
                                    scoreMapVec[row][col] += 5;
                                else if (botNum == 1)                 // 活二
                                    scoreMapVec[row][col] += 10;
                                else if (botNum == 2)
                                {
                                    if (emptyNum == 1)                // 死三
                                        scoreMapVec[row][col] += 25;
                                    else if (emptyNum == 2)
                                        scoreMapVec[row][col] += 50;  // 活三
                                }
                                else if (botNum == 3)
                                {
                                    if (emptyNum == 1)                // 死四
                                        scoreMapVec[row][col] += 55;
                                    else if (emptyNum == 2)
                                        scoreMapVec[row][col] += 100; // 活四
                                }
                                else if (botNum >= 4)
                                    scoreMapVec[row][col] += 10000;   // 活五

                            }
                        }

                }
            }
    }
};

int main() {
    MCTS mcts;
    Chess chess;
    int player;
    int opponent;

    // 暂时使用 while 忙等待来实现  之后修改为多线程，在等待中继续进行计算
    string input;
    while (getline(cin, input)) {
        string command;
        istringstream iss(input);
        iss >> command;

        switch (IO::getCommand(command)) {
            case START:
                int temp;
                iss >> temp;
                player = static_cast<int>(temp);
                opponent = (player == 1 ? 2 : 1);
                IO::output_OK();
                break;
            case PLACE:
                int x, y;
                iss >> x >> y;
                chess.setChess(x, y, opponent);
                break;
            case TURN: {
                chess = mcts.UCTsearch(chess, chess.getLastPoint(), player); /////// 这是1还是多少？
                IO::output_DEBUG(to_string(chess.getLastPoint().first) + " " + to_string(chess.getLastPoint().second));
                IO::output_PLACE(make_pair(chess.getLastPoint().first, chess.getLastPoint().second));
                break;
            }
            case END:
                return 0;
        }
    }

    return 0;
}
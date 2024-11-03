#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <limits>
#include <iostream>
#include <random>
#include <map>
#include <unordered_map>
#include <utility>
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
        static const unordered_map<std::string, int> commandMap =
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

    friend struct ChessHash;

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

        gomoku[5][5] = WHITE;
        gomoku[5][6] = BLACK;
        gomoku[6][5] = BLACK;
        gomoku[6][6] = WHITE;

        setChess(6, 6, WHITE); // 当己方为黑棋时，给出一个白棋的上一步位点
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
        for (int i = ((x - 4) < 0 ? 0 : x - 4); i <= ((x + 4) > boxNum ? boxNum : x + 4); ++i) {
            if (chess.getChess(i, y) == flag) count++;
            else count = 0;
            if (count == 5) return true;
        }
        //纵向
        count = 0;
        for (int j = ((y - 4) < 0 ? 0 : y - 4); j <= ((y + 4) > boxNum ? boxNum : y + 4); ++j) {
            if (chess.getChess(x, j) == flag) count++;
            else count = 0;
            if (count == 5) return true;
        }
        //左下到右上
        count = 0;
        for (int i = ((x - 4) < 0 ? 0 : x - 4); i <= ((x + 4) > boxNum ? boxNum : x + 4); ++i) {
            int yy = y + i - x;

            if (yy < 0) continue;
            else if (yy > boxNum) break;

            if (chess.getChess(i, yy) == flag) count++;
            else count = 0;
            if (count == 5) return true;
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

    // 1代表黑棋赢，2代表白棋赢，0代表没结束
    static int judgeAll(Chess chess) {
        int x = chess.getLastPoint().first;
        int y = chess.getLastPoint().second;
        int player = chess.getChess(x, y);

        // 定义四个方向
        const std::vector<std::pair<int, int>> directions = {
            {1, 0}, {0, 1}, {1, 1}, {1, -1}
        };

        for (const auto& dir : directions) {
            int count = 1; // 包含自己

            // 向一个方向检查
            for (int step = 1; step < 5; ++step) {
                int nx = x + dir.first * step;
                int ny = y + dir.second * step;
                if (nx > 0 && nx <= boxNum && ny > 0 && ny <= boxNum && chess.getChess(nx, ny) == player) {
                    count++;
                }
                else {
                    break;
                }
            }

            // 向反方向检查
            for (int step = 1; step < 5; ++step) {
                int nx = x - dir.first * step;
                int ny = y - dir.second * step;
                if (nx > 0 && nx <= boxNum && ny > 0 && ny <= boxNum && chess.getChess(nx, ny) == player) {
                    count++;
                }
                else {
                    break;
                }
            }

            // 检查是否有五个连成一线
            if (count >= 5) {
                return player; // 返回获胜玩家
            }
        }
        return 0; // 游戏未结束
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

class ValueCaluate;

// 定义棋局状态的哈希函数
struct ChessHash {
    std::size_t operator()(const Chess& chess) const {
        // 使用 std::hash 计算棋盘状态的哈希值
        std::size_t hashValue = 0;
        std::hash<int> intHasher;

        // 哈希棋盘
        for (int i = 1; i <= boxNum; ++i) {
            for (int j = 1; j <= boxNum; ++j) {
                hashValue ^= intHasher(chess.gomoku[i][j]) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
            }
        }

        // 哈希上一次落子
        hashValue ^= intHasher(chess.lastPoint.first) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
        hashValue ^= intHasher(chess.lastPoint.second) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);

        return hashValue;
    }
};

class MCTS
{
public:
    ////////////using Tree = MultiwayTree<_Multiway_tree_traits<Chess>>;
    ////////////Tree tree;

    static const int searchRange = 2;       ///< 搜索范围
    static const int selectNum = 100;        ///< 选择次数
    static const int simulationNum = 8;     ///< 每个状态模拟次数

    std::map<Chess, Properity> mp; // 每个棋局中棋子个数 
    std::map<Chess, Chess> fa; // 父节点
    //unordered_map<Chess, Properity, ChessHash> mp; //  Hash map   测试后发现效率比map略低
    //unordered_map<Chess, Chess, ChessHash> fa; // 父节点

    int chooseCnt = 0; // 选择次数

    int current_player = 1; // 当前玩家 1黑 2白
    int current_opponent = 2;

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
    Chess UCTsearch(Chess chess, std::pair<int, int> center, int player);

    /// @brief 选择
    Chess Selection(Chess chess, std::pair<int, int> center, int player) {

        while (!GameModel::is_terminate(chess) && !GameModel::judgeAll(chess)) // 这里可以删除掉
        {
            int x1 = std::max(0, center.first - searchRange);
            int x2 = std::min(boxNum, center.first + searchRange);
            int y1 = std::max(0, center.second - searchRange);
            int y2 = std::min(boxNum, center.second + searchRange);

            // 如果当前范围中还有 没有棋子的节点 且 没有被扩展的节点
            if (cntNum(chess, x1, x2, y1, y2) + mp[chess].vec.size() < (x2 - x1 + 1) * (y2 - y1 + 1)) {
                return expandNode(chess, center, player); /////////////// 这里的返回值需要改
            }
            else { // 如果选中范围中节点全部被扩展过
                Chess y = chess;

                if (mp[y].vec.size() == 0) {
                    IO::output_DEBUG("treePolicy() error：当前范围内所有节点都被扩展过，但当前节点没有子节点");
                    break;
                }// 这一步是不能被执行的 否则没有返回值了  

                double maxn = -std::numeric_limits<double>::infinity();

                // 找最佳的子节点进行下一层的搜索
                for (auto it = mp[y].vec.begin(); it != mp[y].vec.end(); it++) {
                    if (UCB(*it, player) >= maxn) {
                        maxn = UCB(*it, player);
                        chess = *it;
                    }
                }
                fa[chess] = y;
            }
            player = (player == 1 ? 2 : 1);
        }
    }

    /// @brief 扩展
    /// @attention 目前只有5*5的范围内搜索，搜索100000次，如果运气非常非常非常非常不好就expand不到那1/25的点~
    Chess expandNode(Chess chess, std::pair<int, int> center, int player) {
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
            y.setChess(i, j, player);
            if (!o && mp.find(y) == mp.end()) // 如果当前位置为空 y未被扩展过
            {
                if (goodNext == y) // 特殊情况  如果当前要扩展的节点刚好是现在最佳的节点，则提高其优先级
                {
                    initChess(y);
                    mp[y].value += 1000;
                    mp[chess].vec.push_back(goodNext);
                    fa[y] = chess;
                    return y;
                }


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
        double maxn = -std::numeric_limits<double>::infinity();

        for (auto it = mp[chess].vec.begin(); it != mp[chess].vec.end(); it++)
        {
            if (UCB(*it, nowblack) >= maxn) {
                maxn = UCB(*it, nowblack);
                ans = *it;
            }
        }
        if (chooseCnt >= 25) // 这是在干什么 ？  这样会导致几乎选定的一定是goodNext  和搜索就没关系了
        {
            std::vector<Chess>::iterator iter = std::find(mp[root].vec.begin(), mp[root].vec.end(), goodNext);
            if (iter == mp[root].vec.end()) // 如果预测的goodNext没有被随机模拟出来
            {
                mp[chess].vec.push_back(goodNext);
                ///////////////tree.find(chess).addChild(goodNext);
                ans = goodNext;
            }
        }

        return ans;
    }

    /// @brief 模拟    游戏至结束，黑色赢返回1，白色返回-1，和棋返回0
    int Simulation(Chess chess, int nowblack);

    /// @brief 回退
    void backUp(Chess child, Chess origin, int value) {
        mp[child].value += value;
        mp[child].mockNum++;
        while (!(child == origin))
        {
            if (fa.find(child) == fa.end())
                break;
            child = fa[child];

            mp[child].value += value;
            mp[child].mockNum++;
        }
    }

    /// @brief UCB值 计算用于选择下一个节点
    double UCB(Chess chess, int player) {
        if (mp[chess].mockNum == 0)
            return 0;
        double val = mp[chess].value, mocknum = mp[chess].mockNum;
        if (val + mocknum == 0)
            return -std::numeric_limits<double>::infinity();
        if (player == current_player)    // black
            return val / mocknum + sqrt(log(mocknum) / mocknum);
        else // white
            return -val / mocknum + sqrt(log(mocknum) / mocknum);
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

class ValueCaluate
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
        int oppNum = 0; // 玩家连成子的个数
        int myNum = 0; // AI连成子的个数
        int emptyNum = 0; // 各方向空白位的个数


        // 计分（此处是完全遍历，其实可以用bfs或者dfs加减枝降低复杂度，通过调整权重值，调整AI智能程度以及攻守风格）
        for (int row = 0; row <= boxNum; row++)
            for (int col = 0; col <= boxNum; col++)
            {
                // 空白点就算
                if (row > 0 && col > 0 && gameMapVec[row][col] == 0)
                {
                    // 遍历周围八个方向
                    for (int y = -1; y <= 1; y++)
                        for (int x = -1; x <= 1; x++)
                        {
                            // 重置
                            oppNum = 0;
                            myNum = 0;
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
                                        oppNum++;
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
                                        oppNum++;
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

                                if (oppNum == 1)                      // 杀二
                                    scoreMapVec[row][col] += 10;
                                else if (oppNum == 2)                 // 杀三
                                {
                                    if (emptyNum == 1)
                                        scoreMapVec[row][col] += 30;
                                    else if (emptyNum == 2)
                                        scoreMapVec[row][col] += 40;
                                }
                                else if (oppNum == 3)                 // 杀四
                                {
                                    // 量变空位不一样，优先级不一样
                                    if (emptyNum == 1)
                                        scoreMapVec[row][col] += 60;
                                    else if (emptyNum == 2)
                                        scoreMapVec[row][col] += 110;
                                }
                                else if (oppNum == 4)                 // 杀五
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
                                        myNum++;
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
                                        myNum++;
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

                                if (myNum == 0)                      // 普通下子
                                    scoreMapVec[row][col] += 5;
                                else if (myNum == 1)                 // 活二
                                    scoreMapVec[row][col] += 10;
                                else if (myNum == 2)
                                {
                                    if (emptyNum == 1)                // 死三
                                        scoreMapVec[row][col] += 25;
                                    else if (emptyNum == 2)
                                        scoreMapVec[row][col] += 50;  // 活三
                                }
                                else if (myNum == 3)
                                {
                                    if (emptyNum == 1)                // 死四
                                        scoreMapVec[row][col] += 55;
                                    else if (emptyNum == 2)
                                        scoreMapVec[row][col] += 100; // 活四
                                }
                                else if (myNum >= 4)
                                    scoreMapVec[row][col] += 10000;   // 活五

                            }
                        }

                }
            }
    }
};


Chess MCTS::UCTsearch(Chess chess, std::pair<int, int> center, int player) {
    // Get the starting time
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::duration<double>(1.5); // Set end time for 1.5 seconds

    if (mp.find(chess) == mp.end())
        initChess(chess);

    fa.clear();

    ValueCaluate choose;
    goodNext = choose.bestChildPro(chess);
    root = chess;
    mp.clear();


    while (std::chrono::steady_clock::now() < endTime) {
    //while (chooseCnt <= selectNum) {
        chooseCnt++;
        Chess selectPoint = Selection(chess, center, player); // 函数返回新的Chess和当前player

        for (int i = 1; i <= simulationNum; i++) {
            int newPlayer = (player == 1) ? 2 : 1;
            int val = Simulation(selectPoint, newPlayer);  // 仿真
            backUp(selectPoint, chess, val);           // 回溯

        }
    }
    IO::output_DEBUG("模拟次数   " + to_string(chooseCnt));
    return bestChild(chess, player);
}


int MCTS::Simulation(Chess chess, int player) {
    while (true)
    {
        if (GameModel::judgeAll(chess) || GameModel::is_terminate(chess))
            break;
        std::pair<int, int> h = calCenter(chess);

        if (player == this->current_player) {
            ValueCaluate cal;
            chess = cal.bestChildPro(chess);
            player = (player == 1 ? 2 : 1);
        }

        // 这里开始随机下棋
        int randNum = rand() % 100;
        int i = 0, j = 0;
        if (randNum < 50) {
            i = std::min(std::max(0, h.first - searchRange + rand() % (searchRange * 2 + 1)), boxNum);
            j = std::min(std::max(0, h.second - searchRange + rand() % (searchRange * 2 + 1)), boxNum);
        }
        else {
            i = rand() % (boxNum + 1);
            j = rand() % (boxNum + 1);
        }
        if (!chess.getChess(i, j)) {
            chess.setChess(i, j, player);
            player = (player == 1 ? 2 : 1);
        }
    }

    int result = GameModel::judgeAll(chess);
    return (result == current_opponent) ? -1 : (result == current_player) ? 1 : 0;
}

int main() {
    MCTS mcts;
    Chess chess;
    int player = 1;
    int opponent = 2;

    // 暂时使用 while 忙等待来实现  之后修改为多线程，在等待中继续进行计算
    string input;
    while (getline(cin, input)) {

        // 开始计时
        auto startTime = std::chrono::high_resolution_clock::now();

        string command;
        istringstream iss(input);
        iss >> command;

        switch (IO::getCommand(command)) {
            case START:
                int temp;
                iss >> temp;
                player = static_cast<int>(temp);
                opponent = (player == 1 ? 2 : 1);
                mcts.current_player = player;
                mcts.current_opponent = opponent;
                IO::output_OK();
                break;
            case PLACE:
                int x, y;
                iss >> x >> y;
                chess.setChess(x, y, opponent);
                break;
            case TURN: {
                // 执行需要计时的代码块
                chess = mcts.UCTsearch(chess, chess.getLastPoint(), player);
                IO::output_PLACE(make_pair(chess.getLastPoint().first, chess.getLastPoint().second));

                // 结束计时
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                IO::output_DEBUG("代码执行时间（毫秒）: " + to_string(duration.count()));


                break;
            }
            case END:
                return 0;
        }
    }

    return 0;
}
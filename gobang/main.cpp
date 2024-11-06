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

// 棋形  for ValueAlgo
#define FIVE 7					// 成五
#define LIVEFOUR 6				// 活四
#define RUSHFOUR 5				// 冲四
#define LIVETHREE 4				// 活四
#define SLEEPTHREE 3			// 眠三
#define LIVETWO 2				// 活二
#define SLEEPTWO 1				// 眠二

#define Empty 2 // 这个是别人的代码定义  先暂时这么写   等下再改

// 输入输出
class IO {
public:
    static int getCommand(const std::string& command) {
        static const unordered_map<std::string, int> commandMap =
        { {"START", START}, {"PLACE", PLACE}, {"TURN", TURN} };

        auto it = commandMap.find(command);
        if (it != commandMap.end()) return it->second;  // 返回对应的宏值
        else return END;  // 未找到指令返回 END
    }

    static void output_OK() { printf("OK\n"); fflush(stdout); }
    static void output_PLACE(const point p) { printf("%d %d\n", p.first, p.second); fflush(stdout); }
    static void output_DEBUG(const string& message) { printf("DEBUG %s\n", message.c_str()); fflush(stdout); }

    IO() = delete;
};

class Chess
{
private:
    std::vector<std::vector<int>> gomoku;
    point lastPoint;
    vector<vector<vector<vector<int>>>> pattern; // 存储棋盘单个点的棋形  2个player 4各方向
    friend struct ChessHash;

public:
    // 对外接口
    void setChess(int x, int y, int player) { gomoku[x][y] = player; lastPoint = std::make_pair(x, y); }
    int getChess(int x, int y) const { return gomoku[x][y]; }
    int& getpattern(int x, int y, int player, int direction) { return pattern[x][y][player][direction]; }
    point getLastPoint() const { return lastPoint; }
    std::vector<std::vector<int>> getGomoku() const { return this->gomoku; }

    Chess(): gomoku(boxNum, std::vector<int>(boxNum + 1, 0)), 
    pattern(BOARD_SIZE, vector<vector<vector<int>>>(BOARD_SIZE, vector<vector<int>>(2, vector<int>(4)))) {
        lastPoint = std::make_pair(0, 0);

        gomoku[5][5] = WHITE;
        gomoku[5][6] = BLACK;
        gomoku[6][5] = BLACK;
        gomoku[6][6] = WHITE;

        setChess(6, 6, WHITE); // 当己方为黑棋时，给出一个白棋的上一步位点
    }

    Chess(int rgomoku[boxNum + 1][boxNum + 1]) {
        lastPoint = std::make_pair(-1, -1);

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

    // 1代表黑棋赢，2代表白棋赢，0代表没结束
    static int judgeAll(Chess chess) {
        int x = chess.getLastPoint().first;
        int y = chess.getLastPoint().second;
        int player = chess.getChess(x, y);

        // 定义四个方向
        const std::vector<point> directions = {
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

// class ValueAlgo;

// 定义棋局状态的哈希函数  当前并没有启用ChessHash **
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
    Chess UCTsearch(Chess chess, point center, int player);

    /// @brief 选择
    Chess Selection(Chess chess, point center, int player) {

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
                    if (UCT(*it, player) >= maxn) {
                        maxn = UCT(*it, player);
                        chess = *it;
                    }
                }
                fa[chess] = y;
            }
            player = (player == 1 ? 2 : 1);
        }
    }

    // 扩展
    /// @attention 目前只有5*5的范围内搜索，搜索100000次，如果运气非常非常非常非常不好就expand不到那1/25的点~
    Chess expandNode(Chess chess, point center, int player) {
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

    Chess bestChild(Chess chess, int nowblack) {
        Chess ans = chess;
        double maxn = -std::numeric_limits<double>::infinity();

        for (auto it = mp[chess].vec.begin(); it != mp[chess].vec.end(); it++)
        {
            if (UCT(*it, nowblack) >= maxn) {
                maxn = UCT(*it, nowblack);
                ans = *it;
            }
        }
        //if (chooseCnt >= 25) // 这是在干什么 ？  这样会导致几乎选定的一定是goodNext  和搜索就没关系了
        //{
        //    std::vector<Chess>::iterator iter = std::find(mp[root].vec.begin(), mp[root].vec.end(), goodNext);
        //    if (iter == mp[root].vec.end()) // 如果预测的goodNext没有被随机模拟出来
        //    {
        //        mp[chess].vec.push_back(goodNext);
        //        ///////////////tree.find(chess).addChild(goodNext);
        //        ans = goodNext;
        //    }
        //}

        return ans;
    }

    // 模拟    游戏至结束，黑色赢返回1，白色返回-1，和棋返回0
    int Simulation(Chess chess, int nowblack);

    // 回退
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

    // UCT值 计算用于选择下一个节点
    double UCT(Chess chess, int player);

    // 返回当前局面平均坐标
    static point calCenter(Chess chess) {
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

class ValueAlgo
{
public:
    // 方向向量
    const int X[4] = { 1, 0, 1, 1 };
    const int Y[4] = { 0, 1, 1, -1 };

    static int typeAssistanceTable[10][6][6][3];         // 棋型判断辅助表
    static int patternTable[65536][2];                   // 棋型表
    static int moveEvaluateTable[8][8][8][8];            // 走法评价表

    ValueAlgo() = delete; // 禁止实例化

    // 更新棋型
    static void UpdateType(Chess& chess, int row, int col) {
        int a, b;
        int key;

        for (int i = 0; i < 4; ++i)
        {
            a = row + X[i];
            b = col + Y[i];
            for (int j = 0; j < 4 && a >= 0 && b >= 0 && a < BOARD_SIZE && b < BOARD_SIZE; a += X[i], b += Y[i], ++j)
            {
                key = GetKeyValue(chess, a, b, i);
                chess.getpattern(a, b, 0, i) = patternTable[key][0];
                chess.getpattern(a, b, 1, i) = patternTable[key][1];
            }
            a = row - X[i];
            b = col - Y[i];
            for (int k = 0; k < 4 && a >= 0 && b >= 0 && a < BOARD_SIZE && b < BOARD_SIZE; a -= X[i], b -= Y[i], ++k)
            {
                key = GetKeyValue(chess, a, b, i);
                chess.getpattern(a, b, 0, i) = patternTable[key][0];
                chess.getpattern(a, b, 1, i) = patternTable[key][1];
            }
        }
    }

    // 获取该位置的key编码  row,col : 棋盘位置的坐标  i : 四个方向
    static int GetKeyValue(const Chess& chess, int row, int col, int i) {
        const int stepX = X[i];
        const int stepY = Y[i];
        int key = (chess.getChess(row - stepX * 4, col - stepY * 4))
            ^ (chess.getChess(row - stepX * 3, col - stepY * 3) << 2)
            ^ (chess.getChess(row - stepX * 2, col - stepY * 2) << 4)
            ^ (chess.getChess(row - stepX * 1, col - stepY * 1) << 6)
            ^ (chess.getChess(row + stepX * 1, col + stepY * 1) << 8)
            ^ (chess.getChess(row + stepX * 2, col + stepY * 2) << 10)
            ^ (chess.getChess(row + stepX * 3, col + stepY * 3) << 12)
            ^ (chess.getChess(row + stepX * 4, col + stepY * 4) << 14);

        return key;
    }

    // 获取走法的价值 a,b,c,d : 四个方向的棋型   /////////////////// 这个函数需要修改  和UCT算法的权值进行匹配
    static int GetPval(int a, int b, int c, int d) {
        int type[8] = { 0 };
        type[a]++; type[b]++; type[c]++; type[d]++;

        if (type[FIVE] > 0)
            return 5000;
        if (type[LIVEFOUR] > 0 || type[RUSHFOUR] > 1)
            return 1200;
        if (type[RUSHFOUR] > 0 && type[LIVETHREE] > 0)
            return 1000;
        if (type[LIVETHREE] > 1)
            return 200;

        int value[6] = { 0, 2, 5, 5, 12, 12 };
        int score = 0;
        for (int i = 1; i <= RUSHFOUR; i++)
        {
            score += value[i] * type[i];
        }

        return score;
    }

    // 获取棋型 key : 共16位,每两位存储该位置的状态：黑、白、空、棋盘外  color : 表示要判断哪一方的棋型：黑或白
    // 判断key棋型，填充棋型表
    static int LineType(int color, int key) {
        int line_left[9];
        int line_right[9];

        for (int i = 0; i < 9; i++)
        {
            if (i == 4)
            {
                line_left[i] = color;
                line_right[i] = color;
            }
            else
            {
                line_left[i] = key & 3;
                line_right[8 - i] = key & 3;
                key >>= 2;
            }
        }

        // 从左往右判断，然后从右往左判断
        int p1 = ShortLine(line_left);
        int p2 = ShortLine(line_right);

        // 如果两个方向都是眠三，有可能是活三，复查
        if (p1 == SLEEPTHREE && p2 == SLEEPTHREE)
        {
            return CheckThreeType(line_left);
        }
        // 如果两个方向都是眠四，有可能是活四，复查
        else if (p1 == RUSHFOUR && p2 == RUSHFOUR)
        {
            return CheckFourType(line_left);
        }
        // 返回二者中最大那个
        else
        {
            return p1 > p2 ? p1 : p2;
        }
    }

    // 判断棋型
    static int ShortLine(int* line) {
        int empty = 0, block = 0;
        int len = 1, len_back = 1, count = 1;
        int k;

        int player = line[4];
        for (k = 5; k <= 8; k++)
        {
            if (line[k] == player)
            {
                if (empty + count > 4)
                    break;
                ++count;
                ++len;
                len_back = empty + count;
            }
            else if (line[k] == Empty)
            {
                ++len;
                ++empty;
            }
            else
            {
                if (line[k - 1] == player)
                {
                    block++;
                }
                break;
            }
        }
        // 计算中间空格
        empty = len_back - count;
        for (k = 3; k >= 0; k--)
        {
            if (line[k] == player)
            {
                if (empty + count > 4)
                    break;
                ++count;
                ++len;
                len_back = empty + count;
            }
            else if (line[k] == Empty)
            {
                ++len;
                ++empty;
            }
            else
            {
                if (line[k + 1] == player)
                {
                    block++;
                }
                break;
            }
        }
        return typeAssistanceTable[len][len_back][count][block];
    }
    static int CheckThreeType(int* line) {     // 判断是活三还是眠三
        int color = line[4];
        int type;
        for (int i = 0; i < 9; i++)
        {
            if (line[i] == Empty)
            {
                line[i] = color;
                type = CheckFourType(line);
                line[i] = Empty;
                if (type == LIVEFOUR)
                    return LIVETHREE;
            }
        }
        return SLEEPTHREE;
    }
    static int CheckFourType(int* line) {      // 判断是活四还是眠四
        int i, j, count;

        int five = 0;
        int color = line[4];
        for (i = 0; i < 9; i++)
        {
            if (line[i] == Empty)
            {
                count = 0;
                for (j = i - 1; j >= 0 && line[j] == color; j--)
                    count++;
                for (j = i + 1; j <= 8 && line[j] == color; j++)
                    count++;
                if (count >= 4)
                    five++;
            }
        }
        // 如果有两个空位置能成五，就是活四
        return five >= 2 ? LIVEFOUR : RUSHFOUR;
    }

    // 生成棋型辅助表
    static int ChessTypeAssistance(int len, int len_back, int count, int block) {
        if (len >= 5 && count > 1)
        {
            if (count == 5)
            {
                return FIVE;
            }
            if (len > 5 && len_back < 5 && block == 0)
            {
                switch (count)
                {
                    case 2:
                        return LIVETWO;
                    case 3:
                        return LIVETHREE;
                    case 4:
                        return LIVEFOUR;
                }
            }
            else
            {
                switch (count)
                {
                    case 2:
                        return SLEEPTWO;
                    case 3:
                        return SLEEPTHREE;
                    case 4:
                        return RUSHFOUR;
                }
            }
        }
        return 0;
    }

    // 主函数
    static void initValueAlgo() {
        // 初始化棋型辅助表
        for (int i = 5; i < 10; ++i) {
            for (int j = 0; j < 6; ++j) {
                for (int k = 1; k < 6; ++k) {
                    for (int l = 0; l < 3; ++l) {
                        typeAssistanceTable[i][j][k][l] = ChessTypeAssistance(i, j, k, l);
                    }
                }
            }
        }
        //初始化棋型表
        for (int key = 0; key < 65536; key++) {
            patternTable[key][0] = LineType(0, key);
            patternTable[key][1] = LineType(1, key);
        }
        //初始化走法评价表
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                for (int k = 0; k < 8; ++k) {
                    for (int l = 0; l < 8; ++l) {
                        moveEvaluateTable[i][j][k][l] = GetPval(i, j, k, l);
                    }
                }
            }
        }
    }
};

Chess MCTS::UCTsearch(Chess chess, point center, int player) {
    // Get the starting time
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::duration<double>(1.9); // Set end time for 1.9 seconds

    fa.clear();

    root = chess;
    mp.clear();

    while (std::chrono::steady_clock::now() < endTime) {
        chooseCnt++;
        Chess selectPoint = Selection(chess, center, player);   //////// UCT

        for (int i = 1; i <= simulationNum; i++) {
            int newPlayer = (player == 1) ? 2 : 1;
            int val = Simulation(selectPoint, newPlayer);
            backUp(selectPoint, chess, val);
        }
    }

    IO::output_DEBUG("sining's code simulation time" + to_string(chooseCnt));
    return bestChild(chess, player);        ///////////////// UCT
}

int MCTS::Simulation(Chess chess, int player) {
    while (true)
    {
        if (GameModel::judgeAll(chess) || GameModel::is_terminate(chess))
            break;
        point h = calCenter(chess);

        if (player == this->current_player) {
            ValueAlgo cal;
            chess = cal.ValueAssess(chess); ///// ????
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

double MCTS::UCT(Chess chess, int player) {
    if (mp[chess].mockNum == 0)
        return 0;
    double val = mp[chess].value, mocknum = mp[chess].mockNum;
    if (val + mocknum == 0) // 这代表每一次都输
        return -std::numeric_limits<double>::infinity();
    if (player == current_player)    // black win
        return val / mocknum + sqrt(2 * log(mocknum) / mocknum);  // 参数是 2 还是多少？
    else // white win
        return -val / mocknum + sqrt(2 * log(mocknum) / mocknum); /// 为什么是负的 simulation 返回值有-1
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
                ValueAlgo::initValueAlgo();

                chess = mcts.UCTsearch(chess, chess.getLastPoint(), player);
                IO::output_PLACE(make_pair(chess.getLastPoint().first, chess.getLastPoint().second));

                // 结束计时
                auto endTime = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                IO::output_DEBUG("singing's code run time(ms): " + to_string(duration.count()));

                break;
            }
            case END:
                return 0;
        }
    }

    return 0;
}
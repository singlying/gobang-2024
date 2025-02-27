﻿#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
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
#define WHITE 0
#define EMPTY 2
#define OUT 3

#define BOARD_SIZE 12  // 边界
#define MIN 0

#define MAXMOVES 10  // 每次扩展节点最多扩展的数量

constexpr static int boxNum = 11;



// 棋形  for ValueAlgo
#define L_FIVE 7        // 成五
#define L_FOOUR 6    // 活四
#define R_FOUR 5    // 冲四
#define L_THREE 4   // 活四
#define SLEEP_THREE 3  // 眠三
#define L_TWO 2     // 活二
#define SLEEP_TWO 1    // 眠二

// 输入输出
class IO {
   public:
    static int getCommand(const std::string& command) {
        static const unordered_map<std::string, int> commandMap =
            {{"START", START}, {"PLACE", PLACE}, {"TURN", TURN}};

        auto it = commandMap.find(command);
        if (it != commandMap.end())
            return it->second;  // 返回对应的宏值
        else
            return END;  // 未找到指令返回 END
    }

    static void output_OK() {
        printf("OK\n");
        fflush(stdout);
    }
    static void output_PLACE(const point p) {
        printf("%d %d\n", p.first, p.second);
        fflush(stdout);
    }
    static void output_DEBUG(const string& message) {
        printf("DEBUG %s\n", message.c_str());
        fflush(stdout);
    }

    IO() = delete;
};

class Chess {
   private:
    std::vector<std::vector<int>> gomoku; // 棋盘
    point lastPoint;
    vector<vector<vector<vector<int>>>> pattern;  // 存储棋盘单个点的棋形  2个player 4各方向
    friend struct ChessHash;

   public:
    // 对外接口
    void setChess(int x, int y, int player);
    int getChess(int x, int y) const {
        if (x < 0 || x > BOARD_SIZE - 1 || y < 0 || y > BOARD_SIZE - 1)
            return OUT;
        return gomoku[x][y];
    }
    bool isValidMove(int x, int y) const { return gomoku[x][y] == 0; }
    int& getpattern(int x, int y, int player, int direction) { return pattern[x][y][player][direction]; }
    point getLastPoint() const { return lastPoint; }
    std::vector<std::vector<int>> getGomoku() const { return this->gomoku; }

    Chess()
        : gomoku(BOARD_SIZE, std::vector<int>(BOARD_SIZE, EMPTY)),
          pattern(BOARD_SIZE, vector<vector<vector<int>>>(BOARD_SIZE, vector<vector<int>>(2, vector<int>(4)))) {
        lastPoint = std::make_pair(0, 0);

        setChess(5, 5, WHITE);
        setChess(5, 6, BLACK);
        setChess(6, 5, BLACK);
        setChess(6, 6, WHITE);  // 当己方为黑棋时，给出一个白棋的上一步位点
    }

    Chess(int rgomoku[boxNum + 1][boxNum + 1]) {
        lastPoint = std::make_pair(-1, -1);

        for (int i = 0; i < boxNum + 1; ++i)
            for (int j = 0; j < boxNum + 1; ++j)
                gomoku[i][j] = rgomoku[i][j];
    }

   public:
    friend bool operator<(Chess x, Chess y)  // 用于搜索棋盘状态
    {
        for (int i = 0; i <= boxNum; i++) {
            for (int j = 0; j <= boxNum; j++) {
                if (x.gomoku[i][j] < y.gomoku[i][j])
                    return 1;
                else if (x.gomoku[i][j] > y.gomoku[i][j])
                    return 0;
            }
        }
        return 0;
    }

    friend bool operator==(Chess x, Chess y)  // 用于判断棋盘状态是否相等
    {
        for (int i = 0; i <= boxNum; i++) {
            for (int j = 0; j < boxNum; j++) {
                if (x.gomoku[i][j] != y.gomoku[i][j])
                    return 0;
            }
        }
        return 1;
    }
};

static int typeAssistanceTable[10][6][6][3] = { 0 };  // 初始化棋型判断辅助表
static int patternTable[65536][2] = { 0 };            // 初始化棋型表
static int moveEvaluateTable[8][8][8][8] = { 0 };     // 初始化走法评价表

class GameModel {
   public:
    GameModel() = default;

    static int getopponent(int player) { return player == BLACK ? WHITE : BLACK; }

    // 1代表黑棋赢，-1代表白棋赢，0代表没结束
    static int judgeAll(Chess chess) {
        int x = chess.getLastPoint().first;
        int y = chess.getLastPoint().second;
        int player = chess.getChess(x, y);

        // 定义四个方向
        const std::vector<point> directions = {
            {1, 0}, {0, 1}, {1, 1}, {1, -1}};

        for (const auto& dir : directions) {
            int count = 1;  // 包含自己

            // 向一个方向检查
            for (int step = 1; step < 5; ++step) {
                int nx = x + dir.first * step;
                int ny = y + dir.second * step;
                if (nx > 0 && nx <= boxNum && ny > 0 && ny <= boxNum && chess.getChess(nx, ny) == player) {
                    count++;
                } else {
                    break;
                }
            }

            // 向反方向检查
            for (int step = 1; step < 5; ++step) {
                int nx = x - dir.first * step;
                int ny = y - dir.second * step;
                if (nx > 0 && nx <= boxNum && ny > 0 && ny <= boxNum && chess.getChess(nx, ny) == player) {
                    count++;
                } else {
                    break;
                }
            }

            // 检查是否有五个连成一线
            if (count >= 5) {
                return player == BLACK ? 1 : -1;  // 返回获胜玩家
            }
        }
        return 0;  // 游戏未结束
    }

    // 返回1时代表棋盘满了，是终端节点 0 棋盘没有满
    static int is_terminate(Chess chess) {
        for (int i = 0; i <= boxNum; i++)
            for (int j = 0; j <= boxNum; j++)
                if (chess.getChess(i, j) != EMPTY)
                    return 0;

        return 1;
    }
};

// 每个节点的属性
class Properity {
   public:
    double visits;  // 模拟次数
    double wins;    // 节点价值
    int player;
    std::vector<Chess> vec;  // 关联的棋子
};

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

class MCTS {
   public:
    static const int searchRange = 2;    ///< 搜索范围
    static const int selectNum = 100;    ///< 选择次数
    static const int simulationNum = 8;  ///< 每个状态模拟次数

    //std::map<Chess, Properity> mp;  // 每个棋局中棋子个数
    //std::map<Chess, Chess> fa;      // 父节点
     unordered_map<Chess, Properity, ChessHash> mp; //  Hash map   测试后发现效率比map略低
     unordered_map<Chess, Chess, ChessHash> fa; // 父节点

    int is_black_start_first_drop = 0;  // 当前的落子数

    int current_player = 1;  // 当前玩家 1黑 0白
    int current_opponent = 0;

    // 初始化棋局
    void initChess(Chess chess, int player = WHITE) {
        Properity p;
        p.wins = 0, p.visits = 0;
        p.player = player;
        mp[chess] = p;
    }

   public:
    MCTS() = default;

    // 实现树的搜索，返回最优的落子位置
    /// 搜索函数传入的是原来那个chess
    point UCTsearch2(Chess chess, int player);

    Chess Selection2(Chess chess, int player) {
        while (!mp[chess].vec.empty()) {
            Chess currentNode = chess;
            double maxn = -std::numeric_limits<double>::infinity();
            for (auto it = mp[currentNode].vec.begin(); it != mp[currentNode].vec.end(); it++) {
                int uct = UCT2(chess, *it, player);
                if (uct >= maxn) {
                    maxn = uct;
                    chess = *it;
                }
                fa[chess] = currentNode;
            }
        }
        return chess;
    }

    // 扩展
    void expandNode2(Chess chess) {  // 这个实现中没有center中心计算
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (chess.isValidMove(i, j)) {
                    Chess newnode = chess;
                    int player = mp[chess].player;
                    newnode.setChess(i, j, GameModel::getopponent(player));
                    initChess(newnode, GameModel::getopponent(player));

                    // if (mp.find(chess) == mp.end())
                    //     throw "error : mp.find(chess) == mp.end()";

                    mp[chess].vec.push_back(newnode);
                    fa[newnode] = chess;
                }
            }
        }
    }

    void expandNode3(Chess chess, int player) {  // 价值评估版
        // 生成走法
        int value;
        vector<pair<point, int>> tempMemList;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (chess.getChess(i, j) == EMPTY) {
                    value = EvaluateMove(chess, make_pair(i, j));
                    tempMemList.push_back({make_pair(i, j), value});
                }
            }
        }

        // 按照价值排序
        quickSort(tempMemList, 0, tempMemList.size() - 1);
        vector<point> move;
        CutMoveList(chess, move, tempMemList);
        if (move.size() == 0) {  // 如果没有特别紧急情况，则只选取前MAXMOVES进行扩展
            for (int i = 0; i < MAXMOVES && i < tempMemList.size(); i++)
                move.push_back(tempMemList[i].first);
        }

        for (int i = 0; i < move.size(); i++) {
            Chess newnode = chess;
            int player = mp[chess].player;
            newnode.setChess(move[i].first, move[i].second, GameModel::getopponent(player));
            mp[chess].vec.push_back(newnode);
            fa[newnode] = chess;
        }
    }

    int EvaluateMove(Chess& chess, point p) const {
        int player_score = moveEvaluateTable[chess.getpattern(p.first, p.second, current_player, 0)][chess.getpattern(p.first, p.second, current_player, 1)]
                                            [chess.getpattern(p.first, p.second, current_player, 2)][chess.getpattern(p.first, p.second, current_player, 3)];
        int opponent_score = moveEvaluateTable[chess.getpattern(p.first, p.second, current_opponent, 0)][chess.getpattern(p.first, p.second, current_opponent, 1)]
                                              [chess.getpattern(p.first, p.second, current_opponent, 2)][chess.getpattern(p.first, p.second, current_opponent, 3)];

        if (player_score >= 200 || opponent_score >= 200)
            return player_score >= opponent_score ? player_score * 2 : opponent_score;
        else
            return player_score * 2 + opponent_score;
    }

    // 快速排序
    void quickSort(std::vector<std::pair<point, int>>& tempMemList, int left, int right) {
        if (left >= right)
            return;

        int i = left, j = right;
        int pivot = tempMemList[(left + right) / 2].second;  // 选择中间元素作为基准

        while (i <= j) {
            while (tempMemList[i].second > pivot)
                i++;  // 找到第一个小于等于基准的元素
            while (tempMemList[j].second < pivot)
                j--;  // 找到第一个大于等于基准的元素
            if (i <= j) {
                std::swap(tempMemList[i], tempMemList[j]);  // 交换两个元素
                i++;
                j--;
            }
        }

        if (left < j)
            quickSort(tempMemList, left, j);  // 递归排序左半部分
        if (i < right)
            quickSort(tempMemList, i, right);  // 递归排序右半部分
    }

    // 寻找一些紧急情况 快速返回结果  活四，活三等
    void CutMoveList(Chess& chess, vector<point>& move, vector<pair<point, int>>& tempMemList) {
        // 存在活四或更厉害的棋形棋形，直接返回
        if (tempMemList[0].second >= 2400) {
            move.push_back(tempMemList[0].first);
            return;
        }
        // 对方活三
        if (tempMemList[0].second == 1200) {
            // 寻找对方能活四的点
            for (int i = 0; i < tempMemList.size(); i++) {
                if (tempMemList[i].second == 1200) {
                    move.push_back(tempMemList[i].first);
                } else
                    break;
            }

            if (move.size() != 0)
                return;

            // 寻找双方能冲四的点
            for (int i = move.size(); i < tempMemList.size(); i++) {
                point p = tempMemList[i].first;
                if (isType(chess, p, current_player, R_FOUR) || isType(chess, p, current_opponent, R_FOUR)) {
                    move.push_back(p);
                    if (move.size() >= MAXMOVES)
                        break;
                }
            }
        }
    }

    inline bool isType(Chess& chess, point p, int color, int type) {
        return chess.getpattern(p.first, p.second, color, 0) == type || chess.getpattern(p.first, p.second, color, 1) == type || chess.getpattern(p.first, p.second, color, 2) == type || chess.getpattern(p.first, p.second, color, 3) == type;
    }

    // 模拟    游戏至结束，黑色赢返回1，白色返回-1，和棋返回0
    int Simulation(Chess chess, int player) {
        while (true) {
            if (GameModel::judgeAll(chess) || GameModel::is_terminate(chess))
                break;

            point h = calCenter(chess);

            // if (player == this->current_player) {
            //     ValueAlgo cal;
            //     chess = cal.ValueAssess(chess); ///// ????
            //     player = (player == 1 ? 2 : 1);
            // }

            // 这里开始随机下棋
            int randNum = rand() % 100;
            int i = 0, j = 0;
            if (randNum < 50) {
                i = std::min(std::max(0, h.first - searchRange + rand() % (searchRange * 2 + 1)), boxNum);
                j = std::min(std::max(0, h.second - searchRange + rand() % (searchRange * 2 + 1)), boxNum);
            } else {
                i = rand() % (boxNum + 1);
                j = rand() % (boxNum + 1);
            }
            if (!chess.getChess(i, j)) {
                chess.setChess(i, j, player);
                player = (player == 1 ? 2 : 1);
            }
        }

        int result = GameModel::judgeAll(chess);
        return (result == current_opponent) ? -1 : (result == current_player) ? 1
                                                                              : 0;
    }

    int Simulation2(Chess chess) {
        Chess currentNode = chess;
        int currentplayer = mp[chess].player;
        while (true) {
            vector<point> validMoves;
            for (int i = 0; i < BOARD_SIZE; i++) {
                for (int j = 0; j < BOARD_SIZE; j++) {
                    if (currentNode.isValidMove(i, j)) {
                        validMoves.push_back(point(i, j));
                    }
                }
            }

            if (validMoves.empty())
                break;

            int randIndex = rand() % validMoves.size();  // 随机模拟
            point move = validMoves[randIndex];
            currentNode.setChess(move.first, move.second, currentplayer);

            int result = GameModel::judgeAll(currentNode);
            if (result != 0)
                return result;

            currentplayer = (currentplayer == BLACK ? WHITE : BLACK);
        }

        return 0;  // 平局
    }

    // 回退
    void backUp2(Chess child, Chess root, int wins) {
        mp[child].wins += wins;
        mp[child].visits++;
        while (!(child == root)) {
            if (fa.find(child) == fa.end())
                break;
            child = fa[child];

            mp[child].wins += wins;
            mp[child].visits++;
        }
    }

    Chess bestChild(Chess chess, int nowblack) {
        Chess ans = chess;
        double maxn = -std::numeric_limits<double>::infinity();

        for (auto it = mp[chess].vec.begin(); it != mp[chess].vec.end(); it++) {
            if (UCT(*it, nowblack) >= maxn) {
                maxn = UCT(*it, nowblack);
                ans = *it;
            }
        }

        return ans;
    }

    // UCT值 计算用于选择下一个节点
    double UCT(Chess chess, int player) {
        if (mp[chess].visits == 0)
            return 0;
        double wins = mp[chess].wins, visits = mp[chess].visits;
        if (wins + visits == 0)  // 这代表每一次都输
            return -std::numeric_limits<double>::infinity();
        if (player == current_player)                                // black win
            return wins / visits + sqrt(2 * log(visits) / visits);   // 参数是 2 还是多少？
        else                                                         // white win
            return -wins / visits + sqrt(2 * log(visits) / visits);  /// 为什么是负的 simulation 返回值有-1
    }

    double UCT2(Chess fa, Chess child, int player) {
        if (mp[child].visits == 0)
            return 0;
        double child_wins = mp[child].wins, child_visits = mp[child].visits;
        double fa_visits = mp[fa].visits;
        if (player == current_player)
            return child_wins / child_visits + sqrt(2 * log(fa_visits) / child_visits);
        else
            return -child_wins / child_visits + sqrt(2 * log(fa_visits) / child_visits);
    }

    // 返回当前局面平均坐标
    static point calCenter(Chess chess) {
        int cnt = 0, p1 = 0, p2 = 0;
        for (int i = 0; i <= boxNum; i++)
            for (int j = 0; j <= boxNum; j++) {
                if (chess.getChess(i, j)) {
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

    Chess root;  // 当前局面
};

// 方向向量
const int X[4] = {1, 0, 1, 1};
const int Y[4] = {0, 1, 1, -1};

class ValueAlgo {
   public:
    ValueAlgo() = delete;  // 禁止实例化

    // 更新棋型
    static void UpdateType(Chess& chess, int row, int col) {
        int a, b;
        int key;

        for (int i = 0; i < 4; ++i) {
            a = row + X[i];
            b = col + Y[i];
            for (int j = 0; j < 4 && a >= 0 && b >= 0 && a < BOARD_SIZE && b < BOARD_SIZE; a += X[i], b += Y[i], ++j) {
                key = GetKeyValue(chess, a, b, i);
                chess.getpattern(a, b, 0, i) = patternTable[key][0];
                chess.getpattern(a, b, 1, i) = patternTable[key][1];
            }
            a = row - X[i];
            b = col - Y[i];
            for (int k = 0; k < 4 && a >= 0 && b >= 0 && a < BOARD_SIZE && b < BOARD_SIZE; a -= X[i], b -= Y[i], ++k) {
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
        int key = (chess.getChess(row - stepX * 4, col - stepY * 4)) ^ (chess.getChess(row - stepX * 3, col - stepY * 3) << 2) ^ (chess.getChess(row - stepX * 2, col - stepY * 2) << 4) ^ (chess.getChess(row - stepX * 1, col - stepY * 1) << 6) ^ (chess.getChess(row + stepX * 1, col + stepY * 1) << 8) ^ (chess.getChess(row + stepX * 2, col + stepY * 2) << 10) ^ (chess.getChess(row + stepX * 3, col + stepY * 3) << 12) ^ (chess.getChess(row + stepX * 4, col + stepY * 4) << 14);

        return key;
    }

    // 获取走法的价值 a,b,c,d : 四个方向的棋型
    static int GetPval(int a, int b, int c, int d) {
        int type[8] = {0};
        type[a]++;
        type[b]++;
        type[c]++;
        type[d]++;

        if (type[L_FIVE] > 0)
            return 5000;
        if (type[L_FOOUR] > 0 || type[R_FOUR] > 1)
            return 1200;
        if (type[R_FOUR] > 0 && type[L_THREE] > 0)
            return 1000;
        if (type[L_THREE] > 1)
            return 200;

        int wins[6] = {0, 2, 5, 5, 12, 12};
        int score = 0;
        for (int i = 1; i <= R_FOUR; i++) {
            score += wins[i] * type[i];
        }

        return score;
    }

    // 获取棋型 key : 共16位,每两位存储该位置的状态：黑、白、空、棋盘外  color : 表示要判断哪一方的棋型：黑或白
    // 判断key棋型，填充棋型表
    static int LineType(int color, int key) {
        int line_left[9];
        int line_right[9];

        for (int i = 0; i < 9; i++) {
            if (i == 4) {
                line_left[i] = color;
                line_right[i] = color;
            } else {
                line_left[i] = key & 3;
                line_right[8 - i] = key & 3;
                key >>= 2;
            }
        }

        // 从左往右判断，然后从右往左判断
        int p1 = ShortLine(line_left);
        int p2 = ShortLine(line_right);

        // 如果两个方向都是眠三，有可能是活三，复查
        if (p1 == SLEEP_THREE && p2 == SLEEP_THREE) {
            return CheckThreeType(line_left);
        }
        // 如果两个方向都是眠四，有可能是活四，复查
        else if (p1 == R_FOUR && p2 == R_FOUR) {
            return CheckFourType(line_left);
        }
        // 返回二者中最大那个
        else {
            return p1 > p2 ? p1 : p2;
        }
    }

    // 判断棋型
    static int ShortLine(int* line) {
        int empty = 0, block = 0;
        int len = 1, len_back = 1, count = 1;
        int k;

        int player = line[4];
        for (k = 5; k <= 8; k++) {
            if (line[k] == player) {
                if (empty + count > 4)
                    break;
                ++count;
                ++len;
                len_back = empty + count;
            } else if (line[k] == EMPTY) {
                ++len;
                ++empty;
            } else {
                if (line[k - 1] == player) {
                    block++;
                }
                break;
            }
        }
        // 计算中间空格
        empty = len_back - count;
        for (k = 3; k >= 0; k--) {
            if (line[k] == player) {
                if (empty + count > 4)
                    break;
                ++count;
                ++len;
                len_back = empty + count;
            } else if (line[k] == EMPTY) {
                ++len;
                ++empty;
            } else {
                if (line[k + 1] == player) {
                    block++;
                }
                break;
            }
        }
        return typeAssistanceTable[len][len_back][count][block];
    }
    static int CheckThreeType(int* line) {  // 判断是活三还是眠三
        int color = line[4];
        int type;
        for (int i = 0; i < 9; i++) {
            if (line[i] == EMPTY) {
                line[i] = color;
                type = CheckFourType(line);
                line[i] = EMPTY;
                if (type == L_FOOUR)
                    return L_THREE;
            }
        }
        return SLEEP_THREE;
    }
    static int CheckFourType(int* line) {  // 判断是活四还是眠四
        int i, j, count;

        int five = 0;
        int color = line[4];
        for (i = 0; i < 9; i++) {
            if (line[i] == EMPTY) {
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
        return five >= 2 ? L_FOOUR : R_FOUR;
    }

    // 生成棋型辅助表
    static int ChessTypeAssistance(int len, int len_back, int count, int block) {
        if (len >= 5 && count > 1) {
            if (count == 5) {
                return L_FIVE;
            }
            if (len > 5 && len_back < 5 && block == 0) {
                switch (count) {
                    case 2:
                        return L_TWO;
                    case 3:
                        return L_THREE;
                    case 4:
                        return L_FOOUR;
                }
            } else {
                switch (count) {
                    case 2:
                        return SLEEP_TWO;
                    case 3:
                        return SLEEP_THREE;
                    case 4:
                        return R_FOUR;
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
        // 初始化棋型表
        for (int key = 0; key < 65536; key++) {
            patternTable[key][0] = LineType(0, key);
            patternTable[key][1] = LineType(1, key);
        }
        // 初始化走法评价表
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

void Chess::setChess(int x, int y, int player) {
    gomoku[x][y] = player;
    lastPoint = std::make_pair(x, y);
    ValueAlgo::UpdateType(*this, x, y);
}

#define DEBUG_OUTPUT 0

point MCTS::UCTsearch2(Chess chess, int player) {
    // Get the starting time
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::duration<double>(1.5);  // Set end time for 1.9 seconds

    fa.clear();
    root = chess;
    mp.clear();
    initChess(chess, player);

    if (this->is_black_start_first_drop == 0 && current_player == 1) {
        is_black_start_first_drop = 1;
        return make_pair(7, 6);
    }

    int runNum = 0;
    while (std::chrono::steady_clock::now() < endTime) {
        runNum++;
        auto start = std::chrono::steady_clock::now();

        Chess selectNode = this->Selection2(chess, player);  ///////////

        auto selectionEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double> selectionDuration = selectionEnd - start;
#if DEBUG_OUTPUT
        std::cout << "Selection Duration: " << selectionDuration.count() << " seconds" << std::endl;
#endif
        this->expandNode3(selectNode, player);  /////////////////

        auto expandEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double> expandDuration = expandEnd - selectionEnd;
#if DEBUG_OUTPUT
        std::cout << "Expand Duration: " << expandDuration.count() << " seconds" << std::endl;
#endif

        int result = this->Simulation2(selectNode);  ///////////////////

        auto simulationEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double> simulationDuration = simulationEnd - expandEnd;
#if DEBUG_OUTPUT
        std::cout << "Simulation Duration: " << simulationDuration.count() << " seconds" << std::endl;
#endif

        this->backUp2(selectNode, chess, result);  ////////////////////

        auto backUpEnd = std::chrono::steady_clock::now();
        std::chrono::duration<double> backUpDuration = backUpEnd - simulationEnd;
#if DEBUG_OUTPUT
        std::cout << "BackUp Duration: " << backUpDuration.count() << " seconds" << std::endl;
#endif
    }
    endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
#if DEBUG_OUTPUT
    IO::output_DEBUG("singing's code run 次数: " + to_string(runNum));
#endif

    Chess bestChild;
    double bestVisits = -1e9;
    for (Chess child : mp[chess].vec) {
        int visits = mp[child].visits;
        if (visits > bestVisits) {
            bestVisits = visits;
            bestChild = child;
        }
    }
    return bestChild.getLastPoint();
}

int main() {
    MCTS mcts;
    Chess chess;
    ValueAlgo::initValueAlgo();
    int player = 1;
    int opponent = 0;
    srand(time(NULL));


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
                player = temp == 1 ? 1 : 0;
                opponent = (player == 1 ? 0 : 1);
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
                point result = mcts.UCTsearch2(chess, player);
                chess.setChess(result.first, result.second, player);
                IO::output_PLACE(result);

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
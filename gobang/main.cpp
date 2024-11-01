#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

#define point pair<uint8_t, uint8_t>  // 点

// input
#define START 0
#define PLACE 1
#define TURN 2
#define END 3

// 基本参数
#define BLACK 1  // 黑白棋
#define WHITE 2
#define MAX 12  // 边界
#define MIN 0

// 搜索数节点
class Node {
public:
    point move;              // 当前节点的落子
    int wins;                // 胜利次数
    int visits;              // 访问次数
    vector<Node*> children;  // 子节点

    Node(point move)
        : move(move), wins(0), visits(0), children() {}
};

// 输入输出
class IO {
public:
    int getCommand(const std::string& command) const {
        static const std::unordered_map<std::string, int> commandMap =
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
    void output_OK() const { cerr << "OK\n"; }
    void output_PLACE(const point p) const {
        cerr << (int)p.first << ' ' << (int)p.second << '\n';
    }
    void output_DEBUG(const string& message) const { cerr << "DEBUG " << message << '\n'; }
};

// 算法
class CAL {
private:
    uint8_t player;                 // 己方执棋  
    uint8_t opponent;               // 对方执棋
    vector<vector<uint8_t>> broad;  // 棋盘  0 - 无棋子 1 - 黑棋子 2 - 白棋子

    Node* bestChild(Node* node) {
        Node* best = nullptr;
        double bestValue = -std::numeric_limits<double>::infinity();
        for (auto child : node->children) {
            double uctValue = (double)child->wins / child->visits + sqrt(2 * log(node->visits) / child->visits);
            if (uctValue > bestValue) {
                bestValue = uctValue;
                best = child;
            }
        }
        return best;
    }

    void simulate(Node* node) {
        // 这里可以实现一个随机模拟（蒙特卡罗）来返回胜负
        // 返回模拟结果，更新胜利次数
        // 这里的实现可以根据实际情况进行修改
        // 简单的随机模拟示例
        int result = rand() % 2; // 0 表示输，1 表示赢
        if (result == 1) {
            node->wins++;
        }
        node->visits++;
    }

public:
    CAL() {  // 初始化
        player = 1;
        opponent = 2;
        broad.resize(MAX, vector<uint8_t>(MAX, 0));
        broad[MAX / 2 - 1][MAX / 2 - 1] = WHITE;
        broad[MAX / 2 - 1][MAX / 2] = BLACK;
        broad[MAX / 2][MAX / 2 - 1] = BLACK;
        broad[MAX / 2][MAX / 2] = WHITE;
    }

    void getPlace(const point& p) { this->broad[p.first][p.second] = opponent; }  // 获取对方落子
    void setPlace(const point& p) { this->broad[p.first][p.second] = player; }  // 设置己方落子
    void getPlayer(const uint8_t player) {                                        // 获取执棋
        this->player = player;
        this->opponent = (player == BLACK ? WHITE : BLACK);
    }

    point run() {
        // 初始化根节点，表示当前棋盘状态
        Node* root = new Node({ -1, -1 }); // 表示无落子
        root->visits = 1;

        // 进行多次模拟
        for (int i = 0; i < 1000; ++i) { // 进行1000次模拟
            Node* currentNode = root;

            // 选择阶段
            while (!currentNode->children.empty()) {
                currentNode = bestChild(currentNode);
            }

            // 扩展阶段
            // 生成合法的下一步
            for (int x = 0; x < MAX; ++x) {
                for (int y = 0; y < MAX; ++y) {
                    if (broad[x][y] == 0) { // 如果该点为空
                        Node* childNode = new Node({ static_cast<uint8_t>(x), static_cast<uint8_t>(y) });
                        currentNode->children.push_back(childNode);
                    }
                }
            }

            if (!currentNode->children.empty()) {
                currentNode = currentNode->children[0]; // 选择第一个孩子
            }

            // 模拟阶段
            simulate(currentNode);
        }

        // 找到最佳落子
        Node* bestMoveNode = bestChild(root);
        point bestMove = bestMoveNode->move;

        // 清理内存
        // 应该释放所有节点（在实际代码中需要确保内存管理）

        return bestMove;
    }
};

int main() {
    const IO io;
    CAL cal;

    // 暂时使用 while 忙等待来实现  之后修改为多线程，在等待中继续进行计算
    string input;
    while (getline(cin, input)) {
        string command;
        istringstream iss(input);
        iss >> command;

        switch (io.getCommand(command)) {
            case START:
                int temp;
                iss >> temp;
                cal.getPlayer(static_cast<uint8_t>(temp));
                io.output_OK();
                break;
            case PLACE:
                int x, y;
                iss >> x >> y;
                cal.getPlace({ static_cast<uint8_t>(x), static_cast<uint8_t>(y) });
                break;
            case TURN: {
                point p = cal.run();
                cal.setPlace(p);
                io.output_PLACE(p);
                break;
            }
            case END:
                return 0;
        }
    }

    return 0;
}
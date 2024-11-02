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

struct GameState {
    std::vector<std::vector<int>> board; // 棋盘状态，0为无子，1为黑子，2为白子
    int currentPlayer; // 当前玩家，1为黑子，2为白子

    GameState() : board(BOARD_SIZE, std::vector<int>(BOARD_SIZE, 0)), currentPlayer(1) {
        board[BOARD_SIZE / 2 - 1][BOARD_SIZE / 2 - 1] = WHITE;
        board[BOARD_SIZE / 2 - 1][BOARD_SIZE / 2] = BLACK;
        board[BOARD_SIZE / 2][BOARD_SIZE / 2 - 1] = BLACK;
        board[BOARD_SIZE / 2][BOARD_SIZE / 2] = WHITE;
    }

    bool isWin(int player) {
        int boardSize = board.size();

        // 检查水平方向
        for (int row = 0; row < boardSize; ++row) {
            for (int col = 0; col <= boardSize - 5; ++col) {
                if (board[row][col] == player &&
                    board[row][col + 1] == player &&
                    board[row][col + 2] == player &&
                    board[row][col + 3] == player &&
                    board[row][col + 4] == player) {
                    return true;
                }
            }
        }

        // 检查垂直方向
        for (int row = 0; row <= boardSize - 5; ++row) {
            for (int col = 0; col < boardSize; ++col) {
                if (board[row][col] == player &&
                    board[row + 1][col] == player &&
                    board[row + 2][col] == player &&
                    board[row + 3][col] == player &&
                    board[row + 4][col] == player) {
                    return true;
                }
            }
        }

        // 检查正斜方向
        for (int row = 0; row <= boardSize - 5; ++row) {
            for (int col = 0; col <= boardSize - 5; ++col) {
                if (board[row][col] == player &&
                    board[row + 1][col + 1] == player &&
                    board[row + 2][col + 2] == player &&
                    board[row + 3][col + 3] == player &&
                    board[row + 4][col + 4] == player) {
                    return true;
                }
            }
        }

        // 检查反斜方向
        for (int row = 0; row <= boardSize - 5; ++row) {
            for (int col = 4; col < boardSize; ++col) {
                if (board[row][col] == player &&
                    board[row + 1][col - 1] == player &&
                    board[row + 2][col - 2] == player &&
                    board[row + 3][col - 3] == player &&
                    board[row + 4][col - 4] == player) {
                    return true;
                }
            }
        }

        return false;
    }

    bool isFull() {
        for (const auto& row : board) {
            for (int cell : row) {
                if (cell == 0) return false;
            }
        }
        return true;
    }

    void makeMove(int x, int y) {
        board[x][y] = currentPlayer;
        currentPlayer = (currentPlayer == 1) ? 2 : 1;
    }
};

struct Node {
    GameState state; // 当前状态
    int wins;        // 胜利次数
    int visits;     // 访问次数
    std::vector<Node*> children;
    int lastMoveX;  // 上一次落子的行
    int lastMoveY;  // 上一次落子的列

    Node(const GameState& gameState, int x = -1, int y = -1)
        : state(gameState), wins(0), visits(0), lastMoveX(x), lastMoveY(y) {}
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
public:
    int player;                 // 己方执棋
    int opponent;               // 对方执棋
    GameState initialState;
    Node root;

    double uctValue(Node* node) {
        if (node->visits == 0) return std::numeric_limits<double>::max(); // 避免除以0
        return static_cast<double>(node->wins) / node->visits +
            sqrt(2 * log(node->visits) / node->visits);
    }

    Node* select(Node* root) {
        Node* currentNode = root;
        while (!currentNode->children.empty()) {
            double bestValue = -1;
            Node* bestChild = nullptr;
            for (Node* child : currentNode->children) {
                double value = uctValue(child);
                if (value > bestValue) {
                    bestValue = value;
                    bestChild = child;
                }
            }
            currentNode = bestChild;
        }
        return currentNode;
    }

    void expand(Node* node) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            for (int y = 0; y < BOARD_SIZE; ++y) {
                if (node->state.board[x][y] == 0) {
                    GameState newState = node->state;
                    newState.makeMove(x, y);
                    node->children.push_back(new Node(newState, x, y));
                }
            }
        }
    }

    int simulate(Node* node) {
        // 随机模拟
        GameState tempState = node->state;
        while (!tempState.isFull()) {
            int x = rand() % BOARD_SIZE;
            int y = rand() % BOARD_SIZE;
            if (tempState.board[x][y] == 0) {
                tempState.makeMove(x, y);
                if (tempState.isWin(tempState.currentPlayer)) {
                    return (tempState.currentPlayer == 1) ? 1 : 2; // 返回胜利的玩家
                }
            }
        }
        return 0; // 平局
    }

    void backpropagate(Node* node, int result) {
        while (node != nullptr) {
            node->visits++;
            if (result == 1) node->wins++;
            node = nullptr; // 这里可以改为指向父节点
        }
    }

    Node* uctSearch(Node* root, int iterations) {
        for (int i = 0; i < iterations; ++i) {
            Node* selectedNode = select(root);
            if (!selectedNode->state.isWin(selectedNode->state.currentPlayer)) {
                expand(selectedNode);
                for (Node* child : selectedNode->children) {
                    int result = simulate(child);
                    backpropagate(child, result);
                }
            }
        }
        // 返回胜率最高的子节点
        Node* bestChild = nullptr;
        double bestWinRate = -1;
        for (Node* child : root->children) {
            double winRate = static_cast<double>(child->wins) / child->visits;
            if (winRate > bestWinRate) {
                bestWinRate = winRate;
                bestChild = child;
            }
        }
        return bestChild;
    }
    
public:
    CAL(): root(initialState){  // 初始化
        player = 1;
        opponent = 2;
    }

    //void getPlace(const point& p) { this->broad[p.first][p.second] = opponent; }  // 获取对方落子
    //void setPlace(const point& p) { this->broad[p.first][p.second] = player; }  // 设置己方落子
    void getPlayer(const int player) {                                        // 获取执棋
        this->player = player;
        this->opponent = (player == BLACK ? WHITE : BLACK);
    }

    point run() {
        // UCT搜索
        Node* bestMoveNode = uctSearch(&root, 1000); // 进行1000次迭代
        if (bestMoveNode != nullptr) {
            // 更新状态并打印
            initialState = bestMoveNode->state;
            return {bestMoveNode->lastMoveX, bestMoveNode->lastMoveY};
        } else throw "Error: No valid moves found.";
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
                cal.getPlayer(static_cast<int>(temp));
                io.output_OK();
                break;
            case PLACE:
                int x, y;
                iss >> x >> y;
                //cal.getPlace({ static_cast<int>(x), static_cast<int>(y) });
                cal.root.state.makeMove(x, y);
                break;
            case TURN: {
                point p = cal.run();
                //cal.setPlace(p);
                cal.root.state.makeMove(p.first, p.second);
                io.output_PLACE(p);
                break;
            }
            case END:
                return 0;
        }
    }

    return 0;
}
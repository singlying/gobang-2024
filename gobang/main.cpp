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

constexpr static int boxNum = 11;

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

    // 每个节点的属性
    class Properity {
    public:
        double mockNum; // 模拟次数
        double value; // 节点价值
        std::vector<Chess> vec;  // 关联的棋子
    };


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
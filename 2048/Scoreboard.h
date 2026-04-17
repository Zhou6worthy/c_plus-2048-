#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <string>
#include <fstream> // 用于文件读写
#include <vector>   // 用于存储排行榜数据
#include <tuple>    // 用于返回多个值

// 最大保存条目数
static const int MAX_SCORE_ENTRIES = 10;

// 链表节点：存储单局游戏数据
struct ScoreNode {
    int score;       // 分数（核心排序依据）
    int steps;       // 步数（同分按步数少的排前面）
    std::string time;// 游戏时间（格式：YYYY-MM-DD HH:MM:SS）
    ScoreNode* next; // 下一个节点

    // 节点构造函数
    ScoreNode(int s, int st, std::string t) : score(s), steps(st), time(t), next(nullptr) {}
};

// 排行榜类：单链表实现
class ScoreBoard {
private:
    ScoreNode* head; // 链表头节点

    // 私有辅助函数：释放链表内存（防止内存泄漏）
    void freeList();

public:
    // 构造/析构函数
    ScoreBoard() : head(nullptr) {}
    ~ScoreBoard() { freeList(); } // 析构时自动释放内存

    // 核心功能
    // 返回值：若进入前10返回排名（1..10），否则返回 0
    int addScore(int score, int steps, std::string time); // 按分数降序插入并修剪为前10
    void printScores();                                   // 打印排行榜
    bool saveToFile(const std::string& filename);         // 保存到文件（只保存前10）
    bool loadFromFile(const std::string& filename);       // 从文件加载
    // 获取排行榜数据（用于Qt界面）
    std::vector<std::tuple<int, int, std::string>> getScores(); // 返回分数、步数、时间的列表
};

#endif // SCOREBOARD_H
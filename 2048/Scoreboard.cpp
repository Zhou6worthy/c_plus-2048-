#include "ScoreBoard.h"
#include "utils.h" // 复用分割线、清屏等工具函数
#include <iostream>
#include <iomanip> // 用于格式化输出（setw）

// ===================== 私有辅助函数 =====================
// 释放链表所有节点内存（析构时调用）
void ScoreBoard::freeList() {
    ScoreNode* current = head;
    while (current != nullptr) {
        ScoreNode* temp = current; // 暂存当前节点
        current = current->next;   // 移动到下一个节点
        delete temp;               // 释放当前节点内存
    }
    head = nullptr; // 头节点置空，避免野指针
}

// ===================== 公有核心函数 =====================
// 1. 按分数降序插入节点（核心：单链表的插入排序）
// 返回插入后的排名（1..MAX_SCORE_ENTRIES），若未进入前十返回 0
int ScoreBoard::addScore(int score, int steps, std::string time) {
    // 创建新节点
    ScoreNode* newNode = new ScoreNode(score, steps, time);

    // 空链表 -> 插入为头
    if (head == nullptr) {
        head = newNode;
        // 修剪（虽然只有一条）
        // 计算排名并在需要时修剪尾部
        int rank = 1;
        // 修剪到 MAX_SCORE_ENTRIES（此时不需要实际修剪）
        return (rank <= MAX_SCORE_ENTRIES) ? rank : 0;
    }

    // 比较函数：返回 true 当 newNode 应插入到 a 之前
    auto shouldInsertBefore = [&](ScoreNode* a, ScoreNode* b) {
        if (a->score != b->score) return a->score > b->score;
        return a->steps < b->steps; // 分数相等步数少靠前
    };

    // 情况：插入到头部
    if (shouldInsertBefore(newNode, head)) {
        newNode->next = head;
        head = newNode;
    } else {
        // 遍历找到插入位置
        ScoreNode* current = head;
        while (current->next != nullptr) {
            if (shouldInsertBefore(newNode, current->next)) {
                newNode->next = current->next;
                current->next = newNode;
                break;
            }
            current = current->next;
        }
        if (current->next == nullptr) {
            // 到达末尾，插入尾部
            current->next = newNode;
        }
    }

    // 计算插入后的排名，同时修剪超过 MAX_SCORE_ENTRIES 的节点
    int rank = 1;
    ScoreNode* cur = head;
    ScoreNode* prev = nullptr;
    while (cur != nullptr && rank <= MAX_SCORE_ENTRIES) {
        prev = cur;
        cur = cur->next;
        ++rank;
    }

    // 如果 prev 不为空，则 prev 是第 MAX_SCORE_ENTRIES 个节点（或最后一个存在节点）
    // cur 指向第 MAX_SCORE_ENTRIES+1 个节点（可能为 nullptr）
    // 修剪从 cur 开始的所有节点
    if (prev != nullptr) prev->next = nullptr;
    while (cur != nullptr) {
        ScoreNode* tmp = cur;
        cur = cur->next;
        delete tmp;
    }

    // 现在需要返回新节点的排名（1..MAX_SCORE_ENTRIES），若不在前10则返回 0
    int finalRank = 0;
    cur = head;
    int idx = 1;
    while (cur != nullptr && idx <= MAX_SCORE_ENTRIES) {
        if (cur == newNode) { finalRank = idx; break; }
        cur = cur->next;
        ++idx;
    }

    return finalRank; // 若为0表示未进入前10
}

// 2. 格式化打印排行榜
void ScoreBoard::printScores() {
    clearScreen(); // 复用utils的清屏函数
    printSeparator(60); // 复用utils的分割线函数
    std::cout << std::setw(5) << "排名"
        << std::setw(10) << "分数"
        << std::setw(10) << "步数"
        << std::setw(25) << "游戏时间" << std::endl;
    printSeparator(60);

    ScoreNode* current = head;
    int rank = 1; // 排名
    while (current != nullptr && rank <= MAX_SCORE_ENTRIES) {
        std::cout << std::setw(5) << rank
            << std::setw(10) << current->score
            << std::setw(10) << current->steps
            << std::setw(25) << current->time << std::endl;
        current = current->next;
        rank++;
    }

    // 空链表提示
    if (rank == 1) {
        std::cout << std::setw(30) << "暂无排行榜数据！" << std::endl;
    }

    printSeparator(60);
    waitForKey(); // 复用utils的等待函数
}

// 3. 将链表保存到文本文件（持久化）
// 只保存前 MAX_SCORE_ENTRIES 条
bool ScoreBoard::saveToFile(const std::string& filename) {
    // 打开文件（覆盖写入，ios::out | ios::trunc）
    std::ofstream outFile(filename, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) { // 文件打开失败
        std::cout << "❌ 保存排行榜失败：文件无法打开！\n";
        waitForKey();
        return false;
    }

    // 遍历链表，写入数据（格式：score steps time），仅前 MAX_SCORE_ENTRIES 条
    ScoreNode* current = head;
    int cnt = 0;
    while (current != nullptr && cnt < MAX_SCORE_ENTRIES) {
        outFile << current->score << " "
            << current->steps << " "
            << current->time << std::endl;
        current = current->next;
        ++cnt;
    }

    outFile.close(); // 关闭文件
    std::cout << "✅ 排行榜已保存到 " << filename << "！\n";
    waitForKey();
    return true;
}

// 4. 从文本文件加载排行榜（重建链表）
bool ScoreBoard::loadFromFile(const std::string& filename) {
    // 先释放原有链表内存（避免重复加载）
    freeList();

    // 打开文件（只读）
    std::ifstream inFile(filename, std::ios::in);
    if (!inFile.is_open()) { // 文件不存在/打开失败（首次运行正常）
        std::cout << "ℹ️  未找到排行榜文件，将创建新文件！\n";
        waitForKey();
        return false;
    }

    // 读取每行数据，重建链表（文件中应为按排序写入）
    int score;
    int steps;
    std::string time;
    while (inFile >> score >> steps) {
        inFile.ignore(); // 忽略分数/步数后的空格或换行
        std::getline(inFile, time);
        // 直接插入，不关心返回值；addScore 会在内部修剪为前10
        addScore(score, steps, time);
    }

    inFile.close(); // 关闭文件
    std::cout << "✅ 排行榜已从 " << filename << " 加载！\n";
    waitForKey();
    return true;
}

// 5. 获取排行榜数据（用于Qt界面）
std::vector<std::tuple<int, int, std::string>> ScoreBoard::getScores() {
    std::vector<std::tuple<int, int, std::string>> scores;
    ScoreNode* current = head;
    int rank = 1;
    while (current != nullptr && rank <= MAX_SCORE_ENTRIES) {
        scores.push_back(std::make_tuple(current->score, current->steps, current->time));
        current = current->next;
        rank++;
    }
    return scores;
}
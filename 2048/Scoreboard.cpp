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
void ScoreBoard::addScore(int score, int steps, std::string time) {
    // 创建新节点
    ScoreNode* newNode = new ScoreNode(score, steps, time);

    // 情况1：链表为空 → 新节点作为头节点
    if (head == nullptr) {
        head = newNode;
        return;
    }

    // 情况2：新节点分数 > 头节点分数 → 插入到头部
    if (newNode->score > head->score) {
        newNode->next = head;
        head = newNode;
        return;
    }

    // 情况3：遍历链表，找到插入位置（降序，同分按步数少的排前面）
    ScoreNode* current = head;
    while (current->next != nullptr) {
        // 条件1：下一个节点分数 < 新节点分数 → 插入到current和current->next之间
        if (current->next->score < newNode->score) {
            newNode->next = current->next;
            current->next = newNode;
            return;
        }
        // 条件2：下一个节点分数 == 新节点分数 → 步数少的排前面
        else if (current->next->score == newNode->score) {
            if (current->next->steps > newNode->steps) {
                newNode->next = current->next;
                current->next = newNode;
                return;
            }
        }
        // 继续遍历
        current = current->next;
    }

    // 情况4：遍历到链表末尾 → 插入到最后
    current->next = newNode;
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
    while (current != nullptr) {
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
bool ScoreBoard::saveToFile(const std::string& filename) {
    // 打开文件（覆盖写入，ios::out | ios::trunc）
    std::ofstream outFile(filename, std::ios::out | std::ios::trunc);
    if (!outFile.is_open()) { // 文件打开失败
        std::cout << "❌ 保存排行榜失败：文件无法打开！\n";
        waitForKey();
        return false;
    }

    // 遍历链表，写入数据（格式：score steps time）
    ScoreNode* current = head;
    while (current != nullptr) {
        // 每行存储：分数 步数 时间（用空格分隔，便于读取）
        outFile << current->score << " "
            << current->steps << " "
            << current->time << std::endl;
        current = current->next;
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

    // 读取每行数据，重建链表
    int score, steps;
    std::string time;
    while (inFile >> score >> steps) {
        // 读取时间（注意时间包含空格，需用getline）
        inFile.ignore(); // 忽略分数/步数后的换行符
        std::getline(inFile, time);
        // 插入到链表（自动排序）
        addScore(score, steps, time);
    }

    inFile.close(); // 关闭文件
    std::cout << "✅ 排行榜已从 " << filename << " 加载！\n";
    waitForKey();
    return true;
}
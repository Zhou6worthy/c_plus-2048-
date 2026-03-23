/*定义通用常量和工具函数*/
#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <string>

// 游戏常量
const int DEFAULT_SIZE = 4;  // 默认4×4棋盘
const int PROB_2 = 90;       // 生成2的概率90%
const int WIN_SCORE = 2048;  // 胜利分数

// 工具函数
// 随机数初始化
inline void initRandom() {
    srand((unsigned)time(NULL));
}

// 获取1~max的随机整数
inline int getRandom(int max) {
    return rand() % max + 1;
}

// 清屏（跨平台兼容）
inline void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// 输入验证：确保输入是有效数字/字符
bool isValidInput(char input);
//清理输入缓冲区
void clearInputBuffer();
//获取有效整数输入,适用于多个场景的输入
int getValidIntInput(int min, int max, const std::string& prompt);
//等待用户按任意键继续
void waitForKey();
//格式化打印分割线（美化控制台输出）
void printSeparator(int length = 40);

#endif // UTILS_H

/*封装棋盘、分数、撤销栈核心逻辑*/
#pragma once
#ifndef GAME2048_H
#define GAME2048_H

#include "utils.h"
#include <vector>
#include <stack>

// 游戏状态快照（用于撤销栈）
struct GameState {
    std::vector<std::vector<int>> grid;  // 棋盘状态
    int score;                           // 对应分数
    GameState(std::vector<std::vector<int>> g, int s) : grid(g), score(s) {}
};

class Game2048 {
private:
    int size;                          // 棋盘尺寸（支持3×3/4×4/5×5）
    std::vector<std::vector<int>> grid;// 核心数据结构：二维动态数组（棋盘）
    int score;                         // 当前分数
    bool isGameOver;                   // 游戏结束标记
    std::stack<GameState> undoStack;   // 撤销栈（核心数据结构：栈）
    std::stack<GameState> redoStack;   // 重做栈：用于恢复撤销前的棋盘

    // 私有辅助函数（内部逻辑，对外隐藏）
    // 生成随机数字块（2/4）
    bool generateRandomTile();
    // 单行向左压缩（合并的核心）
    std::vector<int> compressRow(std::vector<int> row, int& addScore);
    // 旋转棋盘（实现上下滑动）
    void rotateGrid();
    // 检查游戏是否结束
    void checkGameOver();
    // 保存当前状态到撤销栈
    void saveState();

public:
    // 构造函数：初始化棋盘
    Game2048(int boardSize = DEFAULT_SIZE);
    // 生成随机数字块的外部接口
    bool spawnNewTile() {
        return generateRandomTile(); // 内部调用私有函数
    }

    // 游戏操作核心函数
    bool moveLeft();   // 左移
    bool moveRight();  // 右移
    bool moveUp();     // 上移
    bool moveDown();   // 下移

    // 撤销/重做功能
    bool undo();
    bool redo();

    // 状态查询函数
    void printGrid();  // 打印棋盘（控制台）
    int getScore() const { return score; }
    bool isOver() const { return isGameOver; }
    bool isWin() const; // 检查是否达到2048

    // 重置游戏
    void reset();
};

#endif // GAME2048_H

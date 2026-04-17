/*封装棋盘、分数、撤销栈核心逻辑*/
#pragma once
#ifndef GAME2048_H
#define GAME2048_H

#include "utils.h"
#include <vector>
#include <stack>
#include <utility>

/**
 * 特殊方块类型（不带 value）
 */
enum SpecialType {
    NONE = 0,
    BOMB,
    FREEZE,
    DOUBLE
};

struct GameState {
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<SpecialType>> specialGrid;
    std::vector<int> columnFreezeTurns;
    int score;

    GameState(std::vector<std::vector<int>> g,
        std::vector<std::vector<SpecialType>> s,
        std::vector<int> cft,
        int sc) : grid(g), specialGrid(s), columnFreezeTurns(cft), score(sc) {}
};

// 公共基类：实现与模式无关的逻辑
class Game2048 {
protected:
    int size;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<SpecialType>> specialGrid;
    int score;
    bool isGameOver;

    std::vector<int> columnFreezeTurns;

    std::stack<GameState> undoStack;
    std::stack<GameState> redoStack;

    // 压缩时上下文，用于记录合并触发的炸弹位置
    bool processIsColumn = false;
    int processIndex = 0;
    std::vector<std::pair<int,int>> pendingExplosions; // (row, col)

    // 模式相关的函数由派生类实现（保留为 protected 或者派生类自行决定访问）
    virtual bool generateRandomTile() = 0;

    // 公共实现
    std::vector<int> compressRow(std::vector<int> row, std::vector<SpecialType>& rowSpecial, int& addScore);
    void rotateGrid();
    void checkGameOver();
    void saveState();
    void processSpecialsAfterMove();

    bool canMoveVertically() const;

public:
    Game2048(int boardSize = DEFAULT_SIZE);
    virtual ~Game2048() = default;

    // 将 printGrid 声明为 public，允许外部通过基类指针/引用调用显示方法
    virtual void printGrid() = 0;

    bool spawnNewTile() { return generateRandomTile(); }

    bool moveLeft();
    bool moveRight();
    bool moveUp();
    bool moveDown();

    bool undo();
    bool redo();

    int getScore() const { return score; }
    bool isOver() const { return isGameOver; }
    bool isWin() const;
    const std::vector<std::vector<int>>& getGrid() const { return grid; }

    void reset();
};

// 经典模式派生
class Game2048Classic : public Game2048 {
public:
    Game2048Classic(int boardSize = DEFAULT_SIZE) : Game2048(boardSize) {}
    virtual ~Game2048Classic() = default;

protected:
    virtual bool generateRandomTile() override; // 仅生成常规 2/4 方块
    virtual void printGrid() override;          // 经典显示（隐藏列号/冻结）
};

// 无尽模式派生
class Game2048Endless : public Game2048 {
public:
    Game2048Endless(int boardSize = DEFAULT_SIZE) : Game2048(boardSize) {}
    virtual ~Game2048Endless() = default;

protected:
    virtual bool generateRandomTile() override; // 可能生成 special 方块
    virtual void printGrid() override;          // 显示列号/冻结信息
};

#endif // GAME2048_H

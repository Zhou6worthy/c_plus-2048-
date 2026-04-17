#include "Game2048.h"
#include "utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

static const int SPECIAL_OVERALL_PCT = 10; // 与原逻辑保持一致
static const int P_DOUBLE = 25;
static const int P_BOMB = 35;
static const int P_FREEZE = 40;

// 无尽模式：可能生成 special（并带初始数值）
bool Game2048Endless::generateRandomTile() {
    std::vector<std::pair<int,int>> emptyCells;
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            if (grid[i][j] == 0) emptyCells.emplace_back(i, j);

    if (emptyCells.empty()) return false;

    int p = getRandom(100);
    if (p <= SPECIAL_OVERALL_PCT) {
        auto cell = emptyCells[getRandom(emptyCells.size()) - 1];
        int r = cell.first, c = cell.second;
        int t = getRandom(100);
        if (t <= P_DOUBLE) specialGrid[r][c] = DOUBLE;
        else if (t <= P_DOUBLE + P_BOMB) specialGrid[r][c] = BOMB;
        else specialGrid[r][c] = FREEZE;
        grid[r][c] = (getRandom(100) <= PROB_2) ? 2 : 4;
        return true;
    }

    auto cell = emptyCells[getRandom(emptyCells.size()) - 1];
    grid[cell.first][cell.second] = (getRandom(100) <= PROB_2) ? 2 : 4;
    return true;
}

void Game2048Endless::printGrid() {
    clearScreen();
    const int CELL_W = 12;
    printSeparator(20 + size * CELL_W);
    std::cout << "2048 Game | 当前分数: " << score << " | 模式: 无尽\n";
    printSeparator(20 + size * CELL_W);

    std::cout << std::left << std::setw(8) << "列:";
    for (int j = 0; j < size; ++j) std::cout << std::setw(CELL_W) << j;
    std::cout << "\n";

    std::cout << std::left << std::setw(8) << "冻结:";
    for (int j = 0; j < size; ++j) {
        if (columnFreezeTurns[j] > 0) std::cout << std::setw(CELL_W) << ("F(" + std::to_string(columnFreezeTurns[j]) + ")");
        else std::cout << std::setw(CELL_W) << "-";
    }
    std::cout << "\n";
    printSeparator(20 + size * CELL_W);

    for (int i = 0; i < size; ++i) {
        std::cout << "|";
        for (int j = 0; j < size; ++j) {
            std::string s;
            SpecialType st = specialGrid[i][j];
            if (grid[i][j] == 0) {
                if (st == BOMB) s = "[B]";
                else if (st == FREEZE) s = "[F]";
                else if (st == DOUBLE) s = "[D]";
                else s = " ";
            } else {
                s = std::to_string(grid[i][j]);
                if (st == BOMB) s += "[B]";
                else if (st == FREEZE) s += "[F]";
                else if (st == DOUBLE) s += "[D]";
            }
            std::cout << std::setw(CELL_W) << s << "|";
        }
        std::cout << "\n";
    }

    printSeparator(20 + size * CELL_W);

    bool anyFrozen = false;
    for (int c = 0; c < size; ++c) {
        if (columnFreezeTurns[c] > 0) {
            if (!anyFrozen) { std::cout << "冻结列: "; anyFrozen = true; }
            std::cout << c << "(" << columnFreezeTurns[c] << "回合) ";
        }
    }
    if (anyFrozen) std::cout << "\n";
}
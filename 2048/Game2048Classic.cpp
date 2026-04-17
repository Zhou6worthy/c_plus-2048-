#include "Game2048.h"
#include "utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

// 经典模式：只生成普通数字块，不生成 special
bool Game2048Classic::generateRandomTile() {
    std::vector<std::pair<int,int>> emptyCells;
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            if (grid[i][j] == 0) emptyCells.emplace_back(i, j);

    if (emptyCells.empty()) return false;
    auto cell = emptyCells[getRandom(emptyCells.size()) - 1];
    grid[cell.first][cell.second] = (getRandom(100) <= PROB_2) ? 2 : 4;
    return true;
}

void Game2048Classic::printGrid() {
    clearScreen();
    const int CELL_W = 12;
    printSeparator(20 + size * CELL_W);
    std::cout << "2048 Game | 当前分数: " << score << " | 模式: 经典\n";
    printSeparator(20 + size * CELL_W);

    // 经典模式不显示列号和冻结信息
    printSeparator(20 + size * CELL_W);

    for (int i = 0; i < size; ++i) {
        std::cout << "|";
        for (int j = 0; j < size; ++j) {
            std::string s;
            SpecialType st = specialGrid[i][j];
            if (grid[i][j] == 0) {
                // 经典模式理论上不应有 special，但为健壮性仍显示 marker
                if (st == BOMB) s = "[B]";
                else if (st == FREEZE) s = "[F]";
                else if (st == DOUBLE) s = "[D]";
                else s = " ";
            } else {
                s = std::to_string(grid[i][j]);
            }
            std::cout << std::setw(CELL_W) << s << "|";
        }
        std::cout << "\n";
    }

    printSeparator(20 + size * CELL_W);
}
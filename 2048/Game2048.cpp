#include "Game2048.h"
#include "utils.h"   // 引入工具函数头文件
#include <algorithm> // reverse
#include <iostream>  
#include <iomanip>   // setw
#include <vector>
#include <stack>

// ===================== 私有辅助函数 =====================
// 1. 生成随机数字块（2/4）：返回是否生成成功
bool Game2048::generateRandomTile() {
    std::vector<std::pair<int, int>> emptyCells; //存储所有空单元格位置
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (grid[i][j] == 0) {
                emptyCells.emplace_back(i, j); //单元格为空，则直接在后面加入
            }
        }
    }

    if (emptyCells.empty()) {
        return false;
    }

    // 复用 utils 中初始化的随机数（initRandom）
    // 随机选择一个空单元格
    auto cell = emptyCells[getRandom(emptyCells.size()) - 1]; // 改用 utils 的 getRandom
    int row = cell.first;
    int col = cell.second;
    grid[row][col] = (getRandom(100) <= PROB_2) ? 2 : 4; // 复用 getRandom 替代 rand()
    return true;
}

// 2. 单行压缩+合并：核心逻辑
std::vector<int> Game2048::compressRow(std::vector<int> row, int& addScore) {
    addScore = 0;
    std::vector<int> newRow;

    // 过滤0
    for (int num : row) {
        if (num != 0) {
            newRow.push_back(num);
        }
    }

    // 合并相邻相等数字
    for (int i = 0; i < (int)newRow.size() - 1; ++i) {
        if (newRow[i] == newRow[i + 1] && newRow[i] != 0) {
            newRow[i] *= 2;
            addScore += newRow[i];
            newRow.erase(newRow.begin() + i + 1);
        }
    }

    // 补0
    while (newRow.size() < size) {
        newRow.push_back(0);
    }

    return newRow;
}

// 3. 旋转棋盘（顺时针90度）
void Game2048::rotateGrid() {
    std::vector<std::vector<int>> tempGrid(size, std::vector<int>(size, 0));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            tempGrid[j][size - 1 - i] = grid[i][j];
        }
    }
    grid = tempGrid;
}

// 4. 检查游戏是否结束
void Game2048::checkGameOver() {
    // 检查空格
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (grid[i][j] == 0) {
                isGameOver = false;
                return;
            }
        }
    }

    // 检查行可合并
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size - 1; ++j) {
            if (grid[i][j] == grid[i][j + 1]) {
                isGameOver = false;
                return;
            }
        }
    }

    // 检查列可合并
    for (int j = 0; j < size; ++j) {
        for (int i = 0; i < size - 1; ++i) {
            if (grid[i][j] == grid[i + 1][j]) {
                isGameOver = false;
                return;
            }
        }
    }

    isGameOver = true;
}

// 5. 保存当前状态到撤销栈
void Game2048::saveState() {
    std::vector<std::vector<int>> gridCopy = grid;
    undoStack.emplace(gridCopy, score);
    while (!redoStack.empty()) {
        redoStack.pop();
    }
}

// ===================== 公有核心函数 =====================
// 构造函数
Game2048::Game2048(int boardSize) : size(boardSize), score(0), isGameOver(false) {
    grid.resize(size, std::vector<int>(size, 0));
    generateRandomTile();
    generateRandomTile();
}

// 左移
bool Game2048::moveLeft() {
    bool moved = false;
    int totalAddScore = 0;

    saveState();

    for (int i = 0; i < size; ++i) {
        std::vector<int> originalRow = grid[i];
        int addScore = 0;
        std::vector<int> newRow = compressRow(originalRow, addScore);

        if (newRow != originalRow) {
            moved = true;
            grid[i] = newRow;
            totalAddScore += addScore;
        }
    }

    score += totalAddScore;
    checkGameOver();
    return moved;
}

// 右移（不再调用 moveLeft，直接复用压缩逻辑，避免重复 saveState）
bool Game2048::moveRight() {
    bool moved = false;
    int totalAddScore = 0;

    saveState();

    // 反转每行（将右移转化为左移问题）
    for (int i = 0; i < size; ++i) {
        std::reverse(grid[i].begin(), grid[i].end());
    }

    // 在反转后的棋盘上执行左移逻辑（不再调用 moveLeft）
    for (int i = 0; i < size; ++i) {
        std::vector<int> originalRow = grid[i];
        int addScore = 0;
        std::vector<int> newRow = compressRow(originalRow, addScore);

        if (newRow != originalRow) {
            moved = true;
            grid[i] = newRow;
            totalAddScore += addScore;
        }
    }

    // 再次反转恢复原方向
    for (int i = 0; i < size; ++i) {
        std::reverse(grid[i].begin(), grid[i].end());
    }

    score += totalAddScore;
    checkGameOver();
    return moved;
}

// 上移（先逆时针旋转一次，即顺时针旋转3次，再在行上做左移，再顺时针旋转1次恢复）
bool Game2048::moveUp() {
    bool moved = false;
    int totalAddScore = 0;

    saveState();

    // 逆时针旋转一次 => 顺时针旋转3次
    rotateGrid();
    rotateGrid();
    rotateGrid();

    // 在旋转后的棋盘上执行左移逻辑
    for (int i = 0; i < size; ++i) {
        std::vector<int> originalRow = grid[i];
        int addScore = 0;
        std::vector<int> newRow = compressRow(originalRow, addScore);

        if (newRow != originalRow) {
            moved = true;
            grid[i] = newRow;
            totalAddScore += addScore;
        }
    }

    // 顺时针旋转一次恢复原方向
    rotateGrid();

    score += totalAddScore;
    checkGameOver();
    return moved;
}

// 下移（先顺时针旋转一次，再在行上做左移，再逆时针旋转一次恢复）
bool Game2048::moveDown() {
    bool moved = false;
    int totalAddScore = 0;

    saveState();

    // 顺时针旋转一次
    rotateGrid();

    // 在旋转后的棋盘上执行左移逻辑（对应原棋盘的下移）
    for (int i = 0; i < size; ++i) {
        std::vector<int> originalRow = grid[i];
        int addScore = 0;
        std::vector<int> newRow = compressRow(originalRow, addScore);

        if (newRow != originalRow) {
            moved = true;
            grid[i] = newRow;
            totalAddScore += addScore;
        }
    }

    // 逆时针旋转一次恢复 => 顺时针旋转3次
    rotateGrid();
    rotateGrid();
    rotateGrid();

    score += totalAddScore;
    checkGameOver();
    return moved;
}

// 撤销
bool Game2048::undo() {
    if (undoStack.empty()) {
        std::cout << "❌ 无法撤销：没有历史记录！\n";
        waitForKey(); // 复用 utils 的等待函数
        return false;
    }

    // 保存当前状态到重做栈
    std::vector<std::vector<int>> currentGrid = grid;
    redoStack.emplace(currentGrid, score);

    // 恢复上一步状态
    GameState prevState = undoStack.top();
    undoStack.pop();
    grid = prevState.grid;
    score = prevState.score;

    checkGameOver();
    std::cout << "✅ 撤销成功！\n";
    waitForKey(); // 复用 utils 的等待函数
    return true;
}

// 重做
bool Game2048::redo() {
    if (redoStack.empty()) {
        std::cout << "❌ 无法重做：没有可重做的操作！\n";
        waitForKey(); // 复用 utils 的等待函数
        return false;
    }

    // 保存当前状态到撤销栈
    std::vector<std::vector<int>> currentGrid = grid;
    undoStack.emplace(currentGrid, score);

    // 恢复重做状态
    GameState nextState = redoStack.top();
    redoStack.pop();
    grid = nextState.grid;
    score = nextState.score;

    checkGameOver();
    std::cout << "✅ 重做成功！\n";
    waitForKey(); // 复用 utils 的等待函数
    return true;
}

// 打印棋盘（复用 utils 的分割线函数）
// 去掉 emoji，以避免控制台编码导致的乱码
void Game2048::printGrid() {
    clearScreen(); // 复用 utils 的清屏函数
    printSeparator(20 + size * 6); // 复用分割线函数，动态调整长度
    std::cout << "2048 Game | 当前分数: " << score << "\n";
    printSeparator(20 + size * 6);

    for (int i = 0; i < size; ++i) {
        std::cout << "|";
        for (int j = 0; j < size; ++j) {
            std::cout << std::setw(6) << (grid[i][j] == 0 ? " " : std::to_string(grid[i][j])) << "|";
        }
        std::cout << "\n";
    }

    printSeparator(20 + size * 6);
}

// 检查胜利
bool Game2048::isWin() const {
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            if (grid[i][j] >= WIN_SCORE) {
                return true;
            }
        }
    }
    return false;
}

// 重置游戏
void Game2048::reset() {
    grid.assign(size, std::vector<int>(size, 0));
    score = 0;
    isGameOver = false;

    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();

    generateRandomTile();
    generateRandomTile();

    std::cout << "🔄 游戏已重置！\n";
    waitForKey(); // 复用 utils 的等待函数
}



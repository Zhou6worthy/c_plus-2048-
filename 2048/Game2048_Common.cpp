#include "Game2048.h"
#include "utils.h"
#include <algorithm>
#include <iostream>
#include <iomanip>

// 公共常量（可放在头或此处）
static const int BOMB_RADIUS = 1;
static const int FREEZE_TURNS = 3;
static const int DOUBLE_MULTIPLIER = 2;

// helper: 比较 special 行
static bool specialRowEqual(const std::vector<SpecialType>& a, const std::vector<SpecialType>& b) {
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i) if (a[i] != b[i]) return false;
    return true;
}

// ctor
Game2048::Game2048(int boardSize) : size(boardSize), score(0), isGameOver(false) {
    grid.resize(size, std::vector<int>(size, 0));
    specialGrid.resize(size, std::vector<SpecialType>(size, NONE));
    columnFreezeTurns.assign(size, 0);
    // 派生类构造后会调用 spawnNewTile() 或由外部调用两次生成初始方块
}

// compressRow（原实现略微改写为不依赖 mode）
std::vector<int> Game2048::compressRow(std::vector<int> row, std::vector<SpecialType>& rowSpecial, int& addScore) {
    addScore = 0;
    std::vector<SpecialType> origSpec = rowSpecial;

    struct Entry { int v; SpecialType s; };
    std::vector<Entry> sources;
    for (int i = 0; i < size; ++i) if (row[i] != 0) sources.push_back({ row[i], origSpec[i] });

    std::vector<int> resultRow(size, 0);
    std::vector<SpecialType> resultSpec = rowSpecial;
    std::vector<bool> merged(size, false);

    int writePos = 0;
    for (size_t k = 0; k < sources.size(); ++k) {
        int val = sources[k].v;
        SpecialType srcSpec = sources[k].s;

        while (writePos < size && resultRow[writePos] != 0) ++writePos;
        if (writePos >= size) break;

        if (writePos > 0 && resultRow[writePos - 1] == val && !merged[writePos - 1]) {
            long baseNew = static_cast<long>(resultRow[writePos - 1]) * 2;
            bool leftHasDouble = (resultSpec[writePos - 1] == DOUBLE);
            bool srcHasDouble = (srcSpec == DOUBLE);
            int multiplier = (leftHasDouble || srcHasDouble) ? DOUBLE_MULTIPLIER : 1;
            long finalNew = baseNew * multiplier;

            addScore += static_cast<int>(finalNew);

            // 炸弹触发：compressRow 只标记 pendingExplosions，实际爆炸在 processSpecialsAfterMove
            bool leftIsBomb = (resultSpec[writePos - 1] == BOMB);
            bool srcIsBomb = (srcSpec == BOMB);
            bool leftHasOtherSpecial = (resultSpec[writePos - 1] != NONE);
            bool srcHasOtherSpecial = (srcSpec != NONE);
            if ((leftIsBomb && srcIsBomb) || (leftIsBomb && !srcHasOtherSpecial) || (srcIsBomb && !leftHasOtherSpecial)) {
                if (!processIsColumn) pendingExplosions.emplace_back(processIndex, writePos - 1);
                else pendingExplosions.emplace_back(writePos - 1, processIndex);
            }

            resultRow[writePos - 1] = static_cast<int>(finalNew);
            merged[writePos - 1] = true;

            if (leftHasDouble) resultSpec[writePos - 1] = NONE;
        } else {
            resultRow[writePos] = val;
            if (resultSpec[writePos] == NONE && srcSpec != NONE) resultSpec[writePos] = srcSpec;
            ++writePos;
        }
    }

    for (int i = 0; i < size; ++i) {
        if (resultRow[i] == 0) resultSpec[i] = NONE;
    }

    rowSpecial = resultSpec;
    return resultRow;
}

// rotateGrid
void Game2048::rotateGrid() {
    std::vector<std::vector<int>> tempGrid(size, std::vector<int>(size, 0));
    std::vector<std::vector<SpecialType>> tempSpec(size, std::vector<SpecialType>(size, NONE));
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j) {
            tempGrid[j][size - 1 - i] = grid[i][j];
            tempSpec[j][size - 1 - i] = specialGrid[i][j];
        }
    grid = tempGrid;
    specialGrid = tempSpec;
}

// checkGameOver
void Game2048::checkGameOver() {
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            if (grid[i][j] == 0) { isGameOver = false; return; }

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size - 1; ++j)
            if (grid[i][j] == grid[i][j + 1]) { isGameOver = false; return; }

    for (int j = 0; j < size; ++j)
        for (int i = 0; i < size - 1; ++i)
            if (grid[i][j] == grid[i + 1][j]) { isGameOver = false; return; }

    isGameOver = true;
}

// saveState
void Game2048::saveState() {
    undoStack.emplace(grid, specialGrid, columnFreezeTurns, score);
    while (!redoStack.empty()) redoStack.pop();
}

// processSpecialsAfterMove (与原实现一致)
void Game2048::processSpecialsAfterMove() {
    for (auto &p : pendingExplosions) {
        int i = p.first, j = p.second;
        if (i < 0 || i >= size || j < 0 || j >= size) continue;
        if (specialGrid[i][j] == BOMB && grid[i][j] != 0) {
            int radius = BOMB_RADIUS;
            for (int di = -radius; di <= radius; ++di)
                for (int dj = -radius; dj <= radius; ++dj) {
                    int ni = i + di, nj = j + dj;
                    if (ni >= 0 && ni < size && nj >= 0 && nj < size) {
                        grid[ni][nj] = 0;
                        specialGrid[ni][nj] = NONE;
                    }
                }
        }
    }
    pendingExplosions.clear();

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            if (specialGrid[i][j] == FREEZE && grid[i][j] != 0) {
                columnFreezeTurns[j] = FREEZE_TURNS;
                specialGrid[i][j] = NONE;
            }

    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            if (grid[i][j] == 0 && specialGrid[i][j] != NONE)
                specialGrid[i][j] = NONE;
}

// canMoveVertically
bool Game2048::canMoveVertically() const {
    for (int col = 0; col < size; ++col) {
        if (columnFreezeTurns[col] > 0) continue;
        for (int r = 0; r < size; ++r) {
            if (grid[r][col] == 0) return true;
            if (r + 1 < size && grid[r][col] == grid[r + 1][col]) return true;
        }
    }
    return false;
}

// moveLeft（支持冻结列作为“垂直冻结 + 水平障碍”）
// 冻结列不会在垂直方向被移动；横向移动时：
//  - 冻结列位置本身作为分段边界（障碍），不能被穿越
//  - 右侧的格子可以移动到包含冻结列的分段左端（即可以进入/合并到冻结列）
// 实现思路：将每一行按冻结列分成若干段，对每一段局部压缩（SSE：不跨段），
// 并把分段压缩时产生的 pendingExplosions 的列索引映射回全局列索引
bool Game2048::moveLeft() {
    bool moved = false;
    int totalAddScore = 0;
    saveState();

    pendingExplosions.clear();

    for (int i = 0; i < size; ++i) {
        processIsColumn = false;
        processIndex = i;

        std::vector<int> originalRow = grid[i];
        std::vector<SpecialType> originalSpec = specialGrid[i];

        std::vector<int> newRow(size, 0);
        std::vector<SpecialType> newSpec(size, NONE);

        int rowAddScore = 0;

        // collect frozen column indices (act as walls)
        std::vector<int> walls;
        for (int c = 0; c < size; ++c) if (columnFreezeTurns[c] > 0) walls.push_back(c);

        // helper: compress a segment [segL, segR] (inclusive) in local coordinates,
        // segL..segR maps to global columns segL..segR
        auto compressSegment = [&](int segL, int segR) {
            int segSize = segR - segL + 1;
            if (segSize <= 0) return;
            // build local arrays
            std::vector<int> segRow(segSize, 0);
            std::vector<SpecialType> segSpec(segSize, NONE);
            for (int k = 0; k < segSize; ++k) {
                segRow[k] = originalRow[segL + k];
                segSpec[k] = originalSpec[segL + k];
            }

            // local compress (same logic as compressRow but for segSize)
            int addScore = 0;
            struct Entry { int v; SpecialType s; };
            std::vector<Entry> sources;
            for (int p = 0; p < segSize; ++p) if (segRow[p] != 0) sources.push_back({ segRow[p], segSpec[p] });

            std::vector<int> resultRow(segSize, 0);
            std::vector<SpecialType> resultSpec = segSpec;
            std::vector<bool> merged(segSize, false);

            int writePos = 0;
            for (size_t k = 0; k < sources.size(); ++k) {
                int val = sources[k].v;
                SpecialType srcSpec = sources[k].s;

                while (writePos < segSize && resultRow[writePos] != 0) ++writePos;
                if (writePos >= segSize) break;

                if (writePos > 0 && resultRow[writePos - 1] == val && !merged[writePos - 1]) {
                    long baseNew = static_cast<long>(resultRow[writePos - 1]) * 2;
                    bool leftHasDouble = (resultSpec[writePos - 1] == DOUBLE);
                    bool srcHasDouble = (srcSpec == DOUBLE);
                    int multiplier = (leftHasDouble || srcHasDouble) ? DOUBLE_MULTIPLIER : 1;
                    long finalNew = baseNew * multiplier;

                    addScore += static_cast<int>(finalNew);

                    bool leftIsBomb = (resultSpec[writePos - 1] == BOMB);
                    bool srcIsBomb = (srcSpec == BOMB);
                    bool leftHasOtherSpecial = (resultSpec[writePos - 1] != NONE);
                    bool srcHasOtherSpecial = (srcSpec != NONE);
                    if ((leftIsBomb && srcIsBomb) || (leftIsBomb && !srcHasOtherSpecial) || (srcIsBomb && !leftHasOtherSpecial)) {
                        // map local writePos-1 back to global column index
                        int globalCol = segL + (writePos - 1);
                        if (!processIsColumn) pendingExplosions.emplace_back(processIndex, globalCol);
                        else pendingExplosions.emplace_back(globalCol, processIndex);
                    }

                    resultRow[writePos - 1] = static_cast<int>(finalNew);
                    merged[writePos - 1] = true;

                    if (leftHasDouble) resultSpec[writePos - 1] = NONE;
                }
                else {
                    resultRow[writePos] = val;
                    if (resultSpec[writePos] == NONE && srcSpec != NONE) resultSpec[writePos] = srcSpec;
                    ++writePos;
                }
            }

            // clear specials on empty slots in this segment
            for (int p = 0; p < segSize; ++p) if (resultRow[p] == 0) resultSpec[p] = NONE;

            // write back to newRow/newSpec (global coordinates)
            for (int p = 0; p < segSize; ++p) {
                newRow[segL + p] = resultRow[p];
                newSpec[segL + p] = resultSpec[p];
            }

            rowAddScore += addScore;
            };

        if (walls.empty()) {
            // no frozen columns, compress whole row as before
            int addScore = 0;
            std::vector<SpecialType> specCopy = originalSpec;
            std::vector<int> compressed = compressRow(originalRow, specCopy, addScore);
            newRow = compressed;
            newSpec = specCopy;
            rowAddScore += addScore;
        }
        else {
            // process segments: for each wall, process left-side segment and the wall+rightblock as a single segment
            int prev = 0;
            for (size_t wi = 0; wi < walls.size(); ++wi) {
                int w = walls[wi];
                // left segment before wall: [prev .. w-1]
                if (prev <= w - 1) compressSegment(prev, w - 1);
                // segment starting at wall: [w .. nextWall-1] (allow tiles from right to move into w)
                int next = (wi + 1 < walls.size()) ? walls[wi + 1] : size;
                if (w <= next - 1) compressSegment(w, next - 1);
                prev = next;
            }
            // tail after last wall (if any)
            if (prev <= size - 1) compressSegment(prev, size - 1);
        }

        bool specChanged = !specialRowEqual(originalSpec, newSpec);
        if (newRow != originalRow || specChanged) {
            moved = true;
            grid[i] = newRow;
            specialGrid[i] = newSpec;
            totalAddScore += rowAddScore;
        }
    }

    if (moved) {
        score += totalAddScore;
        processSpecialsAfterMove();
        for (int& t : columnFreezeTurns) if (t > 0) --t;
    }
    else if (!undoStack.empty()) {
        undoStack.pop();
    }

    checkGameOver();
    return moved;
}

bool Game2048::moveRight() {
    bool moved = false;
    int totalAddScore = 0;
    saveState();

    pendingExplosions.clear();

    // 先把行与 special 行反转，同时反转 columnFreezeTurns（对称处理冻结列）
    for (int i = 0; i < size; ++i) {
        std::reverse(grid[i].begin(), grid[i].end());
        std::reverse(specialGrid[i].begin(), specialGrid[i].end());
    }
    std::reverse(columnFreezeTurns.begin(), columnFreezeTurns.end());

    // 和 moveLeft 对称：对每行按冻结列分段进行局部压缩
    for (int i = 0; i < size; ++i) {
        processIsColumn = false;
        processIndex = i;

        std::vector<int> originalRow = grid[i];
        std::vector<SpecialType> originalSpec = specialGrid[i];

        std::vector<int> newRow(size, 0);
        std::vector<SpecialType> newSpec(size, NONE);

        int rowAddScore = 0;

        // collect frozen column indices (act as walls)
        std::vector<int> walls;
        for (int c = 0; c < size; ++c) if (columnFreezeTurns[c] > 0) walls.push_back(c);

        // helper: compress a segment [segL, segR] (inclusive) in local coordinates
        auto compressSegment = [&](int segL, int segR) {
            int segSize = segR - segL + 1;
            if (segSize <= 0) return;
            std::vector<int> segRow(segSize, 0);
            std::vector<SpecialType> segSpec(segSize, NONE);
            for (int k = 0; k < segSize; ++k) {
                segRow[k] = originalRow[segL + k];
                segSpec[k] = originalSpec[segL + k];
            }

            int addScore = 0;
            struct Entry { int v; SpecialType s; };
            std::vector<Entry> sources;
            for (int p = 0; p < segSize; ++p) if (segRow[p] != 0) sources.push_back({ segRow[p], segSpec[p] });

            std::vector<int> resultRow(segSize, 0);
            std::vector<SpecialType> resultSpec = segSpec;
            std::vector<bool> merged(segSize, false);

            int writePos = 0;
            for (size_t k = 0; k < sources.size(); ++k) {
                int val = sources[k].v;
                SpecialType srcSpec = sources[k].s;

                while (writePos < segSize && resultRow[writePos] != 0) ++writePos;
                if (writePos >= segSize) break;

                if (writePos > 0 && resultRow[writePos - 1] == val && !merged[writePos - 1]) {
                    long baseNew = static_cast<long>(resultRow[writePos - 1]) * 2;
                    bool leftHasDouble = (resultSpec[writePos - 1] == DOUBLE);
                    bool srcHasDouble = (srcSpec == DOUBLE);
                    int multiplier = (leftHasDouble || srcHasDouble) ? DOUBLE_MULTIPLIER : 1;
                    long finalNew = baseNew * multiplier;

                    addScore += static_cast<int>(finalNew);

                    // 仅在双方都是 BOMB 或 一方为 BOMB 且另一方为普通块（无 special）时触发
                    bool leftIsBomb = (resultSpec[writePos - 1] == BOMB);
                    bool srcIsBomb = (srcSpec == BOMB);
                    bool leftHasOtherSpecial = (resultSpec[writePos - 1] != NONE);
                    bool srcHasOtherSpecial = (srcSpec != NONE);
                    if ((leftIsBomb && srcIsBomb) || (leftIsBomb && !srcHasOtherSpecial) || (srcIsBomb && !leftHasOtherSpecial)) {
                        int globalCol = segL + (writePos - 1);
                        if (!processIsColumn) pendingExplosions.emplace_back(processIndex, globalCol);
                        else pendingExplosions.emplace_back(globalCol, processIndex);
                    }

                    resultRow[writePos - 1] = static_cast<int>(finalNew);
                    merged[writePos - 1] = true;

                    if (leftHasDouble) resultSpec[writePos - 1] = NONE;
                }
                else {
                    resultRow[writePos] = val;
                    if (resultSpec[writePos] == NONE && srcSpec != NONE) resultSpec[writePos] = srcSpec;
                    ++writePos;
                }
            }

            for (int p = 0; p < segSize; ++p) if (resultRow[p] == 0) resultSpec[p] = NONE;

            for (int p = 0; p < segSize; ++p) {
                newRow[segL + p] = resultRow[p];
                newSpec[segL + p] = resultSpec[p];
            }

            rowAddScore += addScore;
            };

        if (walls.empty()) {
            int addScore = 0;
            std::vector<SpecialType> specCopy = originalSpec;
            std::vector<int> compressed = compressRow(originalRow, specCopy, addScore);
            newRow = compressed;
            newSpec = specCopy;
            rowAddScore += addScore;
        }
        else {
            int prev = 0;
            for (size_t wi = 0; wi < walls.size(); ++wi) {
                int w = walls[wi];
                if (prev <= w - 1) compressSegment(prev, w - 1);
                int next = (wi + 1 < walls.size()) ? walls[wi + 1] : size;
                if (w <= next - 1) compressSegment(w, next - 1);
                prev = next;
            }
            if (prev <= size - 1) compressSegment(prev, size - 1);
        }

        bool specChanged = !specialRowEqual(originalSpec, newSpec);
        if (newRow != originalRow || specChanged) {
            moved = true;
            grid[i] = newRow;
            specialGrid[i] = newSpec;
            totalAddScore += rowAddScore;
        }
    }

    // 反转回原始显示方向：行、special 行和 columnFreezeTurns 都要恢复
    for (int i = 0; i < size; ++i) {
        std::reverse(grid[i].begin(), grid[i].end());
        std::reverse(specialGrid[i].begin(), specialGrid[i].end());
    }
    std::reverse(columnFreezeTurns.begin(), columnFreezeTurns.end());

    if (moved) {
        score += totalAddScore;
        processSpecialsAfterMove();
        for (int& t : columnFreezeTurns) if (t > 0) --t;
    }
    else if (!undoStack.empty()) {
        undoStack.pop();
    }

    checkGameOver();
    return moved;
}

// moveUp
bool Game2048::moveUp() {
    bool moved = false;
    int totalAddScore = 0;
    saveState();

    pendingExplosions.clear();

    for (int col = 0; col < size; ++col) {
        if (columnFreezeTurns[col] > 0) continue;

        processIsColumn = true;
        processIndex = col;

        std::vector<int> colVals(size);
        std::vector<SpecialType> colSpecs(size);
        for (int r = 0; r < size; ++r) {
            colVals[r] = grid[r][col];
            colSpecs[r] = specialGrid[r][col];
        }

        int addScore = 0;
        std::vector<int> newCol = compressRow(colVals, colSpecs, addScore);

        bool colChanged = false;
        for (int r = 0; r < size; ++r) {
            if (grid[r][col] != newCol[r] || specialGrid[r][col] != colSpecs[r]) { colChanged = true; break; }
        }

        if (colChanged) {
            moved = true;
            totalAddScore += addScore;
            for (int r = 0; r < size; ++r) {
                grid[r][col] = newCol[r];
                specialGrid[r][col] = colSpecs[r];
            }
        }
    }

    if (moved) {
        score += totalAddScore;
        processSpecialsAfterMove();
        for (int& t : columnFreezeTurns) if (t > 0) --t;
    }
    else if (!undoStack.empty()) {
        undoStack.pop();
    }

    checkGameOver();
    return moved;
}

// moveDown
bool Game2048::moveDown() {
    bool moved = false;
    int totalAddScore = 0;
    saveState();

    pendingExplosions.clear();

    for (int col = 0; col < size; ++col) {
        if (columnFreezeTurns[col] > 0) continue;

        processIsColumn = true;
        processIndex = col;

        std::vector<int> colVals(size);
        std::vector<SpecialType> colSpecs(size);
        for (int r = 0; r < size; ++r) {
            colVals[r] = grid[size - 1 - r][col];
            colSpecs[r] = specialGrid[size - 1 - r][col];
        }

        int addScore = 0;
        std::vector<int> newColRev = compressRow(colVals, colSpecs, addScore);

        bool colChanged = false;
        for (int r = 0; r < size; ++r) {
            int newVal = newColRev[size - 1 - r];
            SpecialType newSpec = colSpecs[size - 1 - r];
            if (grid[r][col] != newVal || specialGrid[r][col] != newSpec) { colChanged = true; break; }
        }

        if (colChanged) {
            moved = true;
            totalAddScore += addScore;
            for (int r = 0; r < size; ++r) {
                grid[r][col] = newColRev[size - 1 - r];
                specialGrid[r][col] = colSpecs[size - 1 - r];
            }
        }
    }

    if (moved) {
        score += totalAddScore;
        processSpecialsAfterMove();
        for (int& t : columnFreezeTurns) if (t > 0) --t;
    }
    else if (!undoStack.empty()) {
        undoStack.pop();
    }

    checkGameOver();
    return moved;
}

// undo
bool Game2048::undo() {
    if (undoStack.empty()) {
        std::cout << "❌ 无法撤销：没有历史记录！\n";
        waitForKey();
        return false;
    }

    redoStack.emplace(grid, specialGrid, columnFreezeTurns, score);

    GameState prev = undoStack.top();
    undoStack.pop();
    grid = prev.grid;
    specialGrid = prev.specialGrid;
    columnFreezeTurns = prev.columnFreezeTurns;
    score = prev.score;

    pendingExplosions.clear();
    checkGameOver();
    std::cout << "✅ 撤销成功！\n";
    waitForKey();
    return true;
}

// redo
bool Game2048::redo() {
    if (redoStack.empty()) {
        std::cout << "❌ 无法重做：没有可重做的操作！\n";
        waitForKey();
        return false;
    }

    undoStack.emplace(grid, specialGrid, columnFreezeTurns, score);

    GameState next = redoStack.top();
    redoStack.pop();
    grid = next.grid;
    specialGrid = next.specialGrid;
    columnFreezeTurns = next.columnFreezeTurns;
    score = next.score;

    pendingExplosions.clear();
    checkGameOver();
    std::cout << "✅ 重做成功！\n";
    waitForKey();
    return true;
}



bool Game2048::isWin() const {
    for (int i = 0; i < size; ++i)
        for (int j = 0; j < size; ++j)
            if (grid[i][j] >= WIN_SCORE) return true;
    return false;
}


void Game2048::reset() {
    // 保留当前 board size 和 mode，重置状态为新局
    score = 0;
    isGameOver = false;

    grid.assign(size, std::vector<int>(size, 0));
    specialGrid.assign(size, std::vector<SpecialType>(size, NONE));
    columnFreezeTurns.assign(size, 0);

    // 清空历史栈与待处理爆炸
    while (!undoStack.empty()) undoStack.pop();
    while (!redoStack.empty()) redoStack.pop();
    pendingExplosions.clear();

    // 生成初始两个方块（与构造函数行为一致）
    generateRandomTile();
    generateRandomTile();
}
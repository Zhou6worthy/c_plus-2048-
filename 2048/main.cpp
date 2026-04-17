#include "Game2048.h"
#include "Scoreboard.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <ctime>
#include <memory>

// 获取当前时间字符串（复用）
std::string getCurrentTime() {
    time_t now = time(nullptr);
    struct tm ltm {};
    errno_t err = localtime_s(&ltm, &now);
    if (err != 0) return "获取时间失败";
    char buffer[20] = { 0 };
    sprintf_s(buffer, sizeof(buffer),
        "%04d-%02d-%02d %02d:%02d:%02d",
        1900 + ltm.tm_year, 1 + ltm.tm_mon, ltm.tm_mday,
        ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
    return std::string(buffer);
}

// 游戏主菜单（显示当前无尽模式状态）
void showMenu(bool endless) {
    clearScreen();
    printSeparator(40);
    std::cout << "      🎮 2048 游戏 🎮\n";
    printSeparator(40);
    std::cout << "      1. 开始游戏\n";
    std::cout << "      2. 查看排行榜\n";
    std::cout << "      3. 切换无尽模式（当前：" << (endless ? "已开启" : "未开启") << "）\n";
    std::cout << "      4. 退出游戏\n";
    printSeparator(40);
}

int main() {
    initRandom();

    // 三个按棋盘尺寸划分的排行榜（经典与无尽共用）
    ScoreBoard board3;
    ScoreBoard board4;
    ScoreBoard board5;
    board3.loadFromFile("scores_3x3.txt");
    board4.loadFromFile("scores_4x4.txt");
    board5.loadFromFile("scores_5x5.txt");

    bool endlessMode = false;

    while (true) {
        showMenu(endlessMode);
        int choice = getValidIntInput(1, 4, "请选择菜单选项（1-4）：");

        if (choice == 1) {
            int size = getValidIntInput(3, 5, "请选择棋盘尺寸（3/4/5）：");

            // 按模式实例化派生类
            std::unique_ptr<Game2048> game;
            if (endlessMode) game = std::make_unique<Game2048Endless>(size);
            else game = std::make_unique<Game2048Classic>(size);

            // 生成初始两个方块（派生类的 generateRandomTile 被调用）
            game->spawnNewTile();
            game->spawnNewTile();

            int steps = 0;

            while (true) {
                game->printGrid();
                std::cout << "\n操作说明：W(上) S(下) A(左) D(右) | U(撤销) R(重做) | Q(退出游戏)\n";
                std::cout << "请输入操作：";

                char input;
                std::cin >> input;
                clearInputBuffer();

                if (!isValidInput(input)) {
                    std::cout << "❌ 输入无效！请按提示输入\n";
                    waitForKey();
                    continue;
                }

                bool moved = false;
                switch (input) {
                case 'w': case 'W': moved = game->moveUp(); break;
                case 'a': case 'A': moved = game->moveLeft(); break;
                case 's': case 'S': moved = game->moveDown(); break;
                case 'd': case 'D': moved = game->moveRight(); break;
                case 'u': case 'U': moved = game->undo(); steps = std::max(0, steps - 1); break;
                case 'r': case 'R': moved = game->redo(); steps++; break;
                case 'q': case 'Q':
                    if (game->getScore() > 0) {
                        int rank = 0;
                        if (size == 3) {
                            rank = board3.addScore(game->getScore(), steps, getCurrentTime());
                            board3.saveToFile("scores_3x3.txt");
                        } else if (size == 4) {
                            rank = board4.addScore(game->getScore(), steps, getCurrentTime());
                            board4.saveToFile("scores_4x4.txt");
                        } else {
                            rank = board5.addScore(game->getScore(), steps, getCurrentTime());
                            board5.saveToFile("scores_5x5.txt");
                        }
                        if (rank > 0) {
                            std::cout << "本局分数：" << game->getScore() << "，排名第 " << rank << "（前 " << MAX_SCORE_ENTRIES << "）\n";
                        } else {
                            std::cout << "本局分数：" << game->getScore() << "，未进入前 " << MAX_SCORE_ENTRIES << " 名。\n";
                        }
                        waitForKey();
                    }
                    goto GAME_EXIT;
                }

                if (moved && input != 'u' && input != 'U' && input != 'r' && input != 'R') {
                    steps++;
                    game->spawnNewTile();
                }

                if (game->isWin()) {
                    game->printGrid();
                    std::cout << "🏆 恭喜！你达到了 " << WIN_SCORE << "，胜利！\n";
                    if (game->getScore() > 0) {
                        int rank = 0;
                        if (size == 3) {
                            rank = board3.addScore(game->getScore(), steps, getCurrentTime());
                            board3.saveToFile("scores_3x3.txt");
                        } else if (size == 4) {
                            rank = board4.addScore(game->getScore(), steps, getCurrentTime());
                            board4.saveToFile("scores_4x4.txt");
                        } else {
                            rank = board5.addScore(game->getScore(), steps, getCurrentTime());
                            board5.saveToFile("scores_5x5.txt");
                        }
                        if (rank > 0) {
                            std::cout << "本局分数：" << game->getScore() << "，排名第 " << rank << "（前 " << MAX_SCORE_ENTRIES << "）\n";
                        } else {
                            std::cout << "本局分数：" << game->getScore() << "，未进入前 " << MAX_SCORE_ENTRIES << " 名。\n";
                        }
                    }
                    waitForKey();
                    break;
                }

                if (game->isOver()) {
                    game->printGrid();
                    std::cout << "💀 游戏结束！最终分数：" << game->getScore() << "\n";
                    if (game->getScore() > 0) {
                        int rank = 0;
                        if (size == 3) {
                            rank = board3.addScore(game->getScore(), steps, getCurrentTime());
                            board3.saveToFile("scores_3x3.txt");
                        } else if (size == 4) {
                            rank = board4.addScore(game->getScore(), steps, getCurrentTime());
                            board4.saveToFile("scores_4x4.txt");
                        } else {
                            rank = board5.addScore(game->getScore(), steps, getCurrentTime());
                            board5.saveToFile("scores_5x5.txt");
                        }
                        if (rank > 0) {
                            std::cout << "本局分数：" << game->getScore() << "，排名第 " << rank << "（前 " << MAX_SCORE_ENTRIES << "）\n";
                        } else {
                            std::cout << "本局分数：" << game->getScore() << "，未进入前 " << MAX_SCORE_ENTRIES << " 名。\n";
                        }
                    }
                    waitForKey();
                    break;
                }
            }
        }
        else if (choice == 2) {
            clearScreen();
            printSeparator(30);
            std::cout << "选择棋盘尺寸的排行榜：1. 3x3  2. 4x4  3. 5x5\n";
            printSeparator(30);
            int m = getValidIntInput(1, 3, "请选择（1-3）：");
            if (m == 1) board3.printScores();
            else if (m == 2) board4.printScores();
            else board5.printScores();
        }
        else if (choice == 3) {
            endlessMode = !endlessMode;
            std::cout << (endlessMode ? "✅ 已开启无尽模式（游戏中将生成特殊方块）" : "❌ 已关闭无尽模式（恢复普通规则）") << "\n";
            waitForKey();
        }
        else if (choice == 4) {
            std::cout << "👋 感谢游玩！\n";
            return 0;
        }

    GAME_EXIT:
        continue;
    }

    return 0;
}
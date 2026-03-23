#include "Game2048.h"
#include "ScoreBoard.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <ctime>

// 获取当前时间字符串（复用）
std::string getCurrentTime() {
    time_t now = time(nullptr); // 替代 time(0)，更规范
    struct tm ltm {}; // 初始化 tm 结构体，避免随机值


    errno_t err = localtime_s(&ltm, &now);
    if (err != 0) { // 错误处理
        return "获取时间失败";
    }

    // 格式化时间字符串（YYYY-MM-DD HH:MM:SS）
    char buffer[20] = { 0 }; // 初始化缓冲区，避免乱码
    sprintf_s(
        buffer, sizeof(buffer),
        "%04d-%02d-%02d %02d:%02d:%02d",
        1900 + ltm.tm_year,  // 年份：1900为基准
        1 + ltm.tm_mon,      // 月份：0-11 → 1-12
        ltm.tm_mday,         // 日期
        ltm.tm_hour,         // 小时
        ltm.tm_min,          // 分钟
        ltm.tm_sec           // 秒
    );

    return std::string(buffer);
}

// 游戏主菜单
void showMenu() {
    clearScreen();
    printSeparator(30);
    std::cout << "      🎮 2048 游戏 🎮\n";
    printSeparator(30);
    std::cout << "      1. 开始游戏\n";
    std::cout << "      2. 查看排行榜\n";
    std::cout << "      3. 退出游戏\n";
    printSeparator(30);
}

int main() {
    initRandom(); // 初始化随机数
    ScoreBoard board;
    // 启动时加载排行榜
    board.loadFromFile("scores.txt");

    while (true) {
        showMenu();
        // 复用utils的getValidIntInput选择菜单
        int choice = getValidIntInput(1, 3, "请选择菜单选项（1-3）：");

        if (choice == 1) { // 开始游戏
            int size = getValidIntInput(3, 5, "请选择棋盘尺寸（3/4/5）：");
            Game2048 game(size);
            int steps = 0; // 记录步数

            while (true) {
                game.printGrid();
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
                case 'w': case 'W': moved = game.moveUp(); break;
                case 'a': case 'A': moved = game.moveLeft(); break;
                case 's': case 'S': moved = game.moveDown(); break;
                case 'd': case 'D': game.moveRight(); break;
                case 'u': case 'U': moved = game.undo(); steps--; break;
                case 'r': case 'R': moved = game.redo(); steps++; break;
                case 'q': case 'Q':
                    // 退出游戏时保存当前分数到排行榜
                    if (game.getScore() > 0) {
                        board.addScore(game.getScore(), steps, getCurrentTime());
                        board.saveToFile("scores.txt");
                    }
                    goto GAME_EXIT; // 回到主菜单
                }

                if (moved && input != 'u' && input != 'U' && input != 'r' && input != 'R') {
                    steps++;
                    game.spawnNewTile(); // 调用公有接口生成新块
                }

                // 胜利
                if (game.isWin()) {
                    game.printGrid();
                    std::cout << "🏆 恭喜！你达到了2048，胜利！\n";
                    board.addScore(game.getScore(), steps, getCurrentTime());
                    board.saveToFile("scores.txt");
                    waitForKey();
                    break;
                }

                // 游戏结束
                if (game.isOver()) {
                    game.printGrid();
                    std::cout << "💀 游戏结束！最终分数：" << game.getScore() << "\n";
                    board.addScore(game.getScore(), steps, getCurrentTime());
                    board.saveToFile("scores.txt");
                    waitForKey();
                    break;
                }
            }
        }
        else if (choice == 2) { // 查看排行榜
            board.printScores();
        }
        else if (choice == 3) { // 退出游戏
            std::cout << "👋 感谢游玩！\n";
            return 0;
        }

    GAME_EXIT:
        continue;
    }

    return 0;
}
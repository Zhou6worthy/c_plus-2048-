#include "utils.h"
#include <iostream>
#include <limits> // 用于输入清理

// 验证输入是否为有效操作字符（WASD/wasd/UR/ur/Qq）
bool isValidInput(char input) {
    // 有效操作集合：上下左右、撤销、重做、退出
    switch (input) {
    case 'W': case 'w':
    case 'S': case 's':
    case 'A': case 'a':
    case 'D': case 'd':
    case 'U': case 'u':
    case 'R': case 'r':
    case 'Q': case 'q':
        return true;
    default:
        return false;
    }
}

// 扩展工具1：清理输入缓冲区（解决cin输入错误后卡死的问题）
void clearInputBuffer() {
    std::cin.clear(); // 重置输入状态
    // 忽略缓冲区剩余字符，直到换行符
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// 扩展工具2：获取有效整数输入（比如自定义棋盘尺寸时用）
// min: 最小有效值，max: 最大有效值，prompt: 输入提示语
int getValidIntInput(int min, int max, const std::string& prompt) {
    int input;
    while (true) {
        std::cout << prompt;
        // 检查输入是否为有效整数，且在范围内
        if (std::cin >> input && input >= min && input <= max) {
            clearInputBuffer(); // 清理缓冲区
            return input;
        }
        else {
            clearInputBuffer(); // 清理错误输入
            std::cout << "输入无效！请输入" << min << "到" << max << "之间的整数。\n";
        }
    }
}

// 扩展工具3：等待用户按任意键继续（替代system("pause")，跨平台更友好）
void waitForKey() {
    std::cout << "\n按任意键继续...";
    std::cin.get(); // 等待输入
    clearInputBuffer(); // 清理后续换行符
}

// 扩展工具4：格式化打印分割线（美化控制台输出）
void printSeparator(int length) {
    std::cout << std::string(length, '-') << std::endl;
}


#include "pk_util.h"

bool paramkit::strequals(const std::string& a, const std::string& b, bool ignoreCase)
{
    size_t aLen = a.size();
    if (b.size() != aLen) return false;

    for (size_t i = 0; i < aLen; ++i) {
        if (!ignoreCase) {
            if (a[i] != b[i]) return false;
        }
        else {
            if (tolower(a[i]) != tolower(b[i])) return false;
        }
    }
    return true;
}

bool paramkit::getConsoleColor(HANDLE hConsole, int& color) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (!GetConsoleScreenBufferInfo(hConsole, &info))
        return false;
    color = info.wAttributes;
    return true;
}

void paramkit::printInColor(int color, const std::string &text)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    int prev = 7;
    getConsoleColor(hConsole, prev); // get the previous color

    FlushConsoleInputBuffer(hConsole);
    SetConsoleTextAttribute(hConsole, color); // back to default color
    std::cout << text;
    FlushConsoleInputBuffer(hConsole);

    SetConsoleTextAttribute(hConsole, prev); // back to previous color
}




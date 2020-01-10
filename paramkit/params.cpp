#include "params.h"

#define HILIGHTED_COLOR 0x0f

using namespace paramkit;

namespace paramkit {

    void print_in_color(int color, const std::string &text)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        FlushConsoleInputBuffer(hConsole);
        SetConsoleTextAttribute(hConsole, color); // back to default color
        std::cout << text;
        FlushConsoleInputBuffer(hConsole);
        SetConsoleTextAttribute(hConsole, 7); // back to default color
    }

    void print_param_in_color(int color, const std::string &text)
    {
        print_in_color(color, PARAM_SWITCH1 + text);
    }

    bool is_param(const char *str)
    {
        if (!str) return false;

        const size_t len = strlen(str);
        if (len < 2) return false;

        if (str[0] == PARAM_SWITCH1 || str[0] == PARAM_SWITCH2) {
            return true;
        }
        return false;
    }
};


Params::Params()
{
    /*
    this->addParam(new HexParam(PARAM_CHECKSUM));
    this->setInfo(PARAM_CHECKSUM, "Function checksum: hex");
    */
}

void Params::print()
{
    const int param_color = HILIGHTED_COLOR;

    std::map<std::string, Param*>::iterator itr;
    for (itr = myParams.begin(); itr != myParams.end(); itr++) {
        Param *param = itr->second;
        print_param_in_color(param_color, param->argStr);
        std::cout << ": ";
        std::cout << "\n\t" << std::hex << param->valToString() << "\n";
    }
}

void Params::info()
{
    const int param_color = HILIGHTED_COLOR;

    std::map<std::string, Param*>::iterator itr;
    for (itr = myParams.begin(); itr != myParams.end(); itr++) {
        Param *param = itr->second;
        print_param_in_color(param_color, param->argStr);
        std::cout << " <" << param->type() << ">";
        std::cout << ": ";
        std::cout << "\n\t" << std::hex << param->info << "\n";
    }
}

bool Params::parse(int argc, char* argv[])
{
    size_t count = 0;
    for (int i = 1; i < argc; i++) {
        if (!is_param(argv[i])) {
            continue;
        }
        const char *param_str = &argv[i][1];

        std::map<std::string, Param*>::iterator itr;
        for (itr = myParams.begin(); itr != myParams.end(); itr++) {
            Param *param = itr->second;
            if (!strcmp(param_str, "?"))
            {
                return false;
            }

            if (!strcmp(param_str, param->argStr.c_str())) {
                if ((i + 1) < argc && !(is_param(argv[i + 1]))) {
                    param->parse(argv[i + 1]);
                    count++;
#ifdef _DEBUG
                    std::cout << argv[i] << " : " << argv[i + 1] << "\n";
#endif
                }
                else if (!param->requiredParam) {
                    param->parse(nullptr);
                    count++;
                }
            }
        }
    }
    return (count > 0) ? true : false;
}

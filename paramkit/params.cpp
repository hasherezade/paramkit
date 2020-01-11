#include "params.h"

#define WARNING_COLOR 0x0c
#define HILIGHTED_COLOR 0x0f

#define HEADER_COLOR 14
#define SEPARATOR_COLOR 6

#define PARAM_HELP1 "?"
#define PARAM_HELP2 "help"

using namespace paramkit;

namespace paramkit {

    bool GetColor(HANDLE hConsole, int& color) {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (!GetConsoleScreenBufferInfo(hConsole, &info))
            return false;
        color = info.wAttributes;
        return true;
    }

    void print_in_color(int color, const std::string &text)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        int prev = 7;
        GetColor(hConsole, prev); // get the previous color

        FlushConsoleInputBuffer(hConsole);
        SetConsoleTextAttribute(hConsole, color); // back to default color
        std::cout << text;
        FlushConsoleInputBuffer(hConsole);

        SetConsoleTextAttribute(hConsole, prev); // back to previous color
    }

    void print_param_in_color(int color, const std::string &text)
    {
        print_in_color(color, PARAM_SWITCH1 + text);
    }

    void print_unknown_param(const char *param)
    {
        print_in_color(WARNING_COLOR, "Invalid parameter: ");
        std::cout << param << "\n";
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

void Params::info(bool hilightMissing)
{
    const int hdr_color = HEADER_COLOR;
    const int param_color = HILIGHTED_COLOR;

    print_in_color(hdr_color, "Required: \n\n");
    //Print Required
    std::map<std::string, Param*>::iterator itr;
    for (itr = myParams.begin(); itr != myParams.end(); itr++) {
        Param *param = itr->second;
        if (!param->isRequired) continue;
        int color = param_color;
        if (!param->isSet()) {
            color = WARNING_COLOR;
        }
        print_param_in_color(color, param->argStr);
        std::cout << " <" << param->type() << ">";
        std::cout << "\n\t: " << std::hex << param->info << "\n";
    }

    print_in_color(hdr_color, "\nOptional: \n\n");
    //Print Optional
    for (itr = myParams.begin(); itr != myParams.end(); itr++) {
        Param *param = itr->second;
        if (param->isRequired) continue;

        print_param_in_color(param_color, param->argStr);
        if (param->requiredParam) {
            std::cout << " <" << param->type() << ">";
            std::cout << "\n\t" ;
        }
        std::cout << " : " << param->info << "\n";
    }
    print_in_color(hdr_color, "\nInfo: \n\n");
    print_param_in_color(param_color, PARAM_HELP2);
    std::cout << " : " << "Print this help\n";
}

bool Params::parse(int argc, char* argv[])
{
    size_t count = 0;
    for (int i = 1; i < argc; i++) {
        if (!is_param(argv[i])) {
            continue;
        }
        bool found = false;
        const char *param_str = &argv[i][1];

        std::map<std::string, Param*>::iterator itr;
        for (itr = myParams.begin(); itr != myParams.end(); itr++) {
            Param *param = itr->second;
            if (!strcmp(param_str, PARAM_HELP2) || !strcmp(param_str, PARAM_HELP1))
            {
                return false;
            }

            if (!strcmp(param_str, param->argStr.c_str())) {
                if ((i + 1) < argc && !(is_param(argv[i + 1]))) {
                    param->parse(argv[i + 1]);
                    found = true;
#ifdef _DEBUG
                    std::cout << argv[i] << " : " << argv[i + 1] << "\n";
#endif
                    break;
                }
                else if (!param->requiredParam) {
                    param->parse(nullptr);
                    found = true;
                    break;
                }
            }
        }
        if (found) {
            count++;
        }
        else {
            print_unknown_param(argv[i]);
            return false;
        }
    }
    return (count > 0) ? true : false;
}

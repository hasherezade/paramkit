#include <iostream>
#include <paramkit.h>

#define PARAM_MY_DEC "pdec"
#define PARAM_MY_HEX "phex"

#define PARAM_MY_ASTRING "pastr"
#define PARAM_MY_WSTRING "pwstr"

#define PARAM_MY_BOOL "pbool"
#define PARAM_MY_ENUM "penum"

#define MAX_BUF 50

typedef enum {
    FRUIT_APPLE = 0,
    FRUIT_ORANGE = 1,
    FRUIT_STRAWBERY,
    FRUIT_COUNT
} t_fruits;

//---

using namespace paramkit;

typedef struct {
    DWORD myDec;
    DWORD myHex;
    bool myBool;
    char myABuf[MAX_BUF];
    wchar_t myWBuf[MAX_BUF];
    t_fruits myEnum;
} t_params_struct;

void print_params(t_params_struct &p)
{
    std::cout  << "myDec :  [" << std::dec << p.myDec << "] = " << std::hex << "[0x" << p.myDec << "]\n";
    std::cout  << "myHex :  [" << std::hex << "0x" << p.myHex << "] = [" << std::dec << p.myHex << "]\n";
    std::cout  << "myBool:  [" << std::dec << p.myBool << "]\n";
    std::cout  << "myABuf:  [" << p.myABuf << "]\n";
    std::wcout << "myWBuf:  [" << p.myWBuf << "]\n";
    std::cout  << "myEnum:  [" << std::dec << p.myEnum << "]\n";
}

class DemoParams : public Params
{
public:
    DemoParams()
        : Params()
    {
        this->addParam(new IntParam(PARAM_MY_DEC, true, IntParam::INT_BASE_DEC));
        this->setInfo(PARAM_MY_DEC, "Sample decimal Integer param");

        this->addParam(new IntParam(PARAM_MY_HEX, true, IntParam::INT_BASE_HEX));
        this->setInfo(PARAM_MY_HEX, "Sample hexadecimal Integer param");

        this->addParam(new BoolParam(PARAM_MY_BOOL, false));
        this->setInfo(PARAM_MY_BOOL, "Sample boolean param");

        this->addParam(new StringParam(PARAM_MY_ASTRING, false));
        this->setInfo(PARAM_MY_ASTRING, "Sample string param");

        this->addParam(new WStringParam(PARAM_MY_WSTRING, false));
        this->setInfo(PARAM_MY_WSTRING, "Sample wide string param");

        EnumParam *myEnum = new EnumParam(PARAM_MY_ENUM, GETNAME(t_fruits), false);
        this->addParam(myEnum);
        this->setInfo(PARAM_MY_ENUM, "Sample enum param");
        myEnum->addEnumValue(t_fruits::FRUIT_APPLE, "A", "green apples");
        myEnum->addEnumValue(t_fruits::FRUIT_ORANGE, "O", "oranges");
        myEnum->addEnumValue(t_fruits::FRUIT_STRAWBERY, "S", "fresh strawberries");

        //optional: group parameters
        std::string str_group = "string params";
        this->addGroup(new ParamGroup(str_group));
        this->addParamToGroup(PARAM_MY_ASTRING, str_group);
        this->addParamToGroup(PARAM_MY_WSTRING, str_group);

        str_group = "enums";
        this->addGroup(new ParamGroup(str_group));
        this->addParamToGroup(PARAM_MY_ENUM, str_group);
    }

    bool fillStruct(t_params_struct &paramsStruct)
    {
        copyVal<BoolParam>(PARAM_MY_BOOL, paramsStruct.myBool);
        copyVal<IntParam>(PARAM_MY_DEC, paramsStruct.myDec);
        copyVal<IntParam>(PARAM_MY_HEX, paramsStruct.myHex);
        copyVal<EnumParam>(PARAM_MY_ENUM, paramsStruct.myEnum);

        copyCStr<StringParam>(PARAM_MY_ASTRING, paramsStruct.myABuf, _countof(paramsStruct.myABuf));
        copyCStr<WStringParam>(PARAM_MY_WSTRING, paramsStruct.myWBuf, _countof(paramsStruct.myWBuf));
        return true;
    }

    void printBanner()
    {
        paramkit::print_in_color(CYAN, "Welcome to ParamKit Demo!");
        std::cout << std::endl;
    }
};

int main(int argc, char* argv[])
{
    DemoParams params;
    if (argc < 2) {
        params.printBanner();
        params.printInfo(false);
        return 0;
    }
    if (!params.parse(argc, argv)) {
        return 0;
    }
    std::cout << "\nPrinting the filled params:\n";
    params.print();

    t_params_struct p = { 0 };
    params.fillStruct(p);

    std::cout << "\nConverted to the structure:\n";
    print_params(p);
    std::cout << std::endl;

    if (!params.hasRequiredFilled()) {
        std::cout << "[!] Some of the required parameters are not filled!\n";
    }
    else {
        std::cout << "[+] All the required parameters filled!\n";
    }
    return 0;
}

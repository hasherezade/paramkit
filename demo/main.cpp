#include <iostream>
#include <paramkit.h>

#define PARAM_MY_DEC "pdec"
#define PARAM_MY_HEX "phex"

#define PARAM_MY_STRING "pstr"
#define PARAM_MY_BOOL "pbool"

using namespace paramkit;

typedef struct {
    DWORD myDec;
    DWORD myHex;
    bool myBool;
    char myBuf[50];
} t_params_struct;

void print_params(t_params_struct &p)
{
    std::cout << "myDec:  " << std::dec << p.myDec << "\n";
    std::cout << "myHex:  " << std::hex << p.myHex << "\n";
    std::cout << "myBool: " << std::dec << p.myBool << "\n";
    std::cout << "myBuf:  " << p.myBuf << "\n";
}

class DemoParams : public Params
{
public:
    DemoParams()
        : Params()
    {
        this->addParam(new IntParam(PARAM_MY_DEC, true));
        this->setInfo(PARAM_MY_DEC, "Sample decimal Integer param");

        this->addParam(new IntParam(PARAM_MY_HEX, true, true));
        this->setInfo(PARAM_MY_HEX, "Sample hexadecimal Integer param");

        this->addParam(new BoolParam(PARAM_MY_BOOL, false));
        this->setInfo(PARAM_MY_BOOL, "Sample boolean param");

        this->addParam(new StringParam(PARAM_MY_STRING, false));
        this->setInfo(PARAM_MY_STRING, "Sample string param");
    }

    bool fillStruct(t_params_struct &paramsStruct)
    {
        BoolParam *myBool = dynamic_cast<BoolParam*>(this->getParam(PARAM_MY_BOOL));
        if (myBool) paramsStruct.myBool = myBool->value;

        IntParam *myDec = dynamic_cast<IntParam*>(this->getParam(PARAM_MY_DEC));
        if (myDec) paramsStruct.myDec = myDec->value;

        IntParam *myHex = dynamic_cast<IntParam*>(this->getParam(PARAM_MY_HEX));
        if (myHex) paramsStruct.myHex = myHex->value;

        StringParam *myStr = dynamic_cast<StringParam*>(this->getParam(PARAM_MY_STRING));
        if (myStr) {
            myStr->copyToCStr(paramsStruct.myBuf, sizeof(paramsStruct.myBuf));
        }
        return true;
    }
};

int main(int argc, char* argv[])
{
    DemoParams params;
    if (argc < 2) {
        params.info();
        return 0;
    }
    if (!params.parse(argc, argv)) {
        params.info();
        return 0;
    }

    params.print();

    std::cout << "\nConverted: \n";
    t_params_struct p;
    params.fillStruct(p);
    print_params(p);

    if (!params.hasRequiredFilled()) {
        std::cout << "[!] Some of the required parameters are not filled!\n";
    }
    else {
        std::cout << "[+] All the required parameters filled!\n";
    }
    return 0;
}

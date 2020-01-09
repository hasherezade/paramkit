#include "params.h"

using namespace paramkit;

Params::Params()
{
    /*
    this->addParam(new HexParam(PARAM_CHECKSUM));
    this->setInfo(PARAM_CHECKSUM, "Function checksum: hex");
    */
}

void Params::print()
{
    std::map<std::string, Param*>::iterator itr;
    for (itr = myParams.begin(); itr != myParams.end(); itr++) {
        Param *param = itr->second;
        std::cout << param->argStr << " : " << std::hex << param->value << "\n";
    }
}

void Params::info()
{
    std::map<std::string, Param*>::iterator itr;
    for (itr = myParams.begin(); itr != myParams.end(); itr++) {
        Param *param = itr->second;
        std::cout << param->argStr << " : " << std::hex << param->info << "\n";
    }
}

bool Params::parse(int argc, char* argv[])
{
    size_t count = 0;
    for (int i = 1; i < argc; i++) {
        std::map<std::string, Param*>::iterator itr;
        for (itr = myParams.begin(); itr != myParams.end(); itr++) {
            Param *param = itr->second;
            if (!strcmp(argv[i], "/?"))
            {
                return false;
            }
            if (!strcmp(argv[i], param->argStr.c_str()) && (i + 1) < argc) {
                param->parse(argv[i + 1]);
                count++;
#ifdef _DEBUG
                std::cout << argv[i] << " : " << argv[i + 1] << "\n";
#endif
            }
        }
    }
    return (count > 0) ? true : false;
}

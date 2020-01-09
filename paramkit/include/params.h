#pragma once

#include <Windows.h>

#include <iostream>
#include <string>
#include <map>

#define PARAM_UNINITIALIZED (-1)
namespace paramkit {
    //--
    class Param {
    public:
        Param(const std::string& _argStr)
        {
            argStr = _argStr;
            value = PARAM_UNINITIALIZED;
        }

        virtual void parse(char *arg) {
            DWORD val = 0;
            sscanf(arg, "%d", &val);
            this->value = val;
        }

        std::string argStr;
        std::string info;
        DWORD value;
    };

    class HexParam : public Param {
    public:
        HexParam(const std::string& _argStr)
            : Param(_argStr)
        {
        }

        virtual void parse(char *arg) {
            DWORD val = 0;
            if (sscanf(arg, "%X", &val) == 0) {
                sscanf(arg, "%#X", &val);
            }
            this->value = val;
        }
    };

    class Params {
    public:
        Params();
        virtual ~Params() { releaseParams(); }

        void addParam(Param* param)
        {
            if (!param) return;
            const std::string argStr = param->argStr;
            this->myParams[argStr] = param;
        }

        void setDwordValue(const std::string& str, DWORD val)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(str);
            if (itr != this->myParams.end()) {
                Param *param = itr->second;
                param->value = val;
                return;
            }
            Param *param = new Param(str);
            param->value = val;
            this->myParams[str] = param;
        }

        void setInfo(const std::string& str, const std::string& info)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(str);
            if (itr != this->myParams.end()) {
                Param *param = itr->second;
                param->info = info;
                return;
            }
            Param *param = new Param(str);
            param->info = info;
            this->myParams[str] = param;
        }

        DWORD getDwordValue(const std::string& str)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(str);
            if (itr == this->myParams.end()) return PARAM_UNINITIALIZED;

            Param *param = itr->second;
            return param->value;
        }


        void releaseParams()
        {
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                Param *param = itr->second;
                delete param;
            }
            myParams.clear();
        }

        void print();
        void info();
        bool parse(int argc, char* argv[]);

        std::map<std::string, Param*> myParams;
    };
};

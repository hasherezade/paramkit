#pragma once

#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#define PARAM_UNINITIALIZED (-1)

#define PARAM_SWITCH1 '/'
#define PARAM_SWITCH2 '-'

namespace paramkit {
    //--

    class Param {
    public:
        Param(const std::string& _argStr)
        {
            argStr = _argStr;
            getVal = false;
        }

        virtual std::string valToString() = 0;
        virtual std::string type() = 0;

        virtual void parse(char *arg) = 0;
        virtual bool isSet() = 0;

        std::string argStr;
        bool getVal;
        std::string info;
    };

    class IntParam : public Param {
    public:
        IntParam(const std::string& _argStr, bool _isHex = false)
            : Param(_argStr)
        {
            getVal = true;
            value = PARAM_UNINITIALIZED;
            isHex = _isHex;
        }

        virtual std::string valToString()
        {
            std::stringstream stream;
            if (isHex) {
                stream << std::hex;
            }
            else {
                stream << std::dec;
            }
            stream << value;
            return stream.str();
        }

        virtual std::string type() {
            if (isHex) {
                return "QWORD: hex";
            }
            return "QWORD: dec";
        }

        virtual bool isSet()
        {
            return value != PARAM_UNINITIALIZED;
        }

        virtual void parse(char *arg) {
            uint64_t val = 0;
            if (isHex) {
                if (sscanf(arg, "%llX", &val) == 0) {
                    sscanf(arg, "%#llX", &val);
                }
                this->value = val;
                return;
            }
            sscanf(arg, "%d", &val);
            this->value = val;
        }

        bool isHex;
        uint64_t value;
    };

    class BoolParam : public Param {
    public:
        BoolParam(const std::string& _argStr)
            : Param(_argStr)
        {
            getVal = false;
            value = false;
        }

        virtual std::string type() {
            return "bool";
        }

        virtual std::string valToString()
        {
            std::stringstream stream;
            stream << std::dec;
            if (value) {
                stream << "true";
            }
            else {
                stream << "false";
            }
            return stream.str();
        }

        virtual bool isSet()
        {
            return value;
        }

        virtual void parse(char *arg = nullptr) {
            if (arg == nullptr) {
                this->value = true;
                return;
            }
            DWORD val = 0;
            sscanf(arg, "%d", &val);
            if (val != 0) {
                this->value = true;
            }
            else {
                this->value = false;
            }
        }

        bool value;
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

        bool setIntValue(const std::string& str, uint64_t val)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(str);
            if (itr != this->myParams.end()) {
                IntParam *param = dynamic_cast<IntParam*>(itr->second);
                if (!param) {
                    return false;
                }
                param->value = val;
                return true;
            }
            IntParam *param = new IntParam(str);
            param->value = val;
            this->myParams[str] = param;
            return true;
        }

        bool setInfo(const std::string& str, const std::string& info)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(str);
            if (itr != this->myParams.end()) {
                Param *param = itr->second;
                param->info = info;
                return true;
            }
            return false;
        }

        uint64_t getIntValue(const std::string& str)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(str);
            if (itr == this->myParams.end()) return PARAM_UNINITIALIZED;

            IntParam *param = dynamic_cast<IntParam*>(itr->second);
            if (!param) {
                return false;
            }
            return param->value;
        }

        virtual bool isSet(const std::string& str)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(str);
            if (itr == this->myParams.end()) return false;

            Param *param = itr->second;
            if (!param) {
                return false;
            }
            return param->isSet();
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

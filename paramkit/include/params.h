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

    static int loadInt(const std::string &str, bool isHex = false)
    {
        int intVal = 0;
        std::stringstream ss;
        if (isHex) {
            ss << std::hex << str;
        }
        else {
            ss << std::dec << str;
        }
        ss >> intVal;
        return intVal;
    }

    static int loadInt(const std::wstring &wstr, bool isHex = false)
    {
        int intVal = 0;
        std::string str(wstr.begin(), wstr.end());
        return loadInt(str, isHex );
    }

    //--
    void print_in_color(int color, const std::string &text);

    class Param {
    public:
        Param(const std::string& _argStr, bool _isRequired)
        {
            isRequired = _isRequired;
            argStr = _argStr;
            requiredArg = false;
        }

        Param(const std::string& _argStr, const std::string& _typeDescStr, bool _isRequired)
        {
            isRequired = _isRequired;
            argStr = _argStr;
            typeDescStr = _typeDescStr;
            requiredArg = false;
        }

        virtual std::string valToString() = 0;
        virtual std::string type() const = 0;

        virtual bool parse(const char *arg) = 0;

        virtual bool parse(const wchar_t *arg)
        {
            std::wstring value = arg;
            std::string str(value.begin(), value.end());
            return parse(str.c_str());
        }

        virtual bool isSet() = 0;

        std::string typeDescStr;

    protected:
        std::string argStr;

        std::string info;
        bool isRequired;

        bool requiredArg; // do you need to pass argument to this param

    friend class Params;
    };

    class IntParam : public Param {
    public:
        IntParam(const std::string& _argStr, bool _isRequired, bool _isHex = false)
            : Param(_argStr, _isRequired)
        {
            requiredArg = true;
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

        virtual std::string type() const {
            if (isHex) {
                return "integer: hex";
            }
            return "integer: dec";
        }

        virtual bool isSet()
        {
            return value != PARAM_UNINITIALIZED;
        }

        virtual bool parse(const char *arg) {
            if (!arg) return false;
            this->value = loadInt(arg, isHex);
            return true;
        }

        bool isHex;
        uint64_t value;
    };

    class StringParam : public Param {
    public:
        StringParam(const std::string& _argStr, bool _isRequired)
            : Param(_argStr, _isRequired)
        {
            requiredArg = true;
            value = "";
        }

        virtual std::string valToString()
        {
            return "\"" + value + "\"";
        }

        virtual std::string type() const {
            return "string";
        }

        virtual bool isSet()
        {
            return value.length() > 0;
        }

        virtual bool parse(const char *arg) {
            if(!arg) return false;

            this->value = arg;
            return true;
        }

        size_t copyToCStr(char *buf, size_t buf_max)
        {
            size_t len = value.length() + 1;
            if (len > buf_max) len = buf_max;

            memcpy(buf, value.c_str(), len);
            buf[len] = '\0';
            return len;
        }

        std::string value;
    };

    class WStringParam : public Param {
    public:
        WStringParam(const std::string& _argStr, bool _isRequired)
            : Param(_argStr, _isRequired)
        {
            requiredArg = true;
            value = L"";
        }

        virtual std::string valToString()
        {
            std::string str(value.begin(), value.end());
            return "\"" + str + "\"";
        }

        virtual std::string type() const {
            return "wstring";
        }

        virtual bool isSet()
        {
            return value.length() > 0;
        }

        virtual bool parse(const wchar_t *arg) {
            if (!arg) return false;

            this->value = arg;
            return true;
        }

        virtual bool parse(const char *arg) {
            if (!arg) return false;

            std::string value = arg;
            std::wstring str(value.begin(), value.end());

            this->value = str;
            return true;
        }

        size_t copyToCStr(wchar_t *buf, size_t buf_max)
        {
            size_t len = value.length() + 1;
            if (len > buf_max) len = buf_max;

            memcpy(buf, value.c_str(), len);
            buf[len] = '\0';
            return len;
        }

        std::wstring value;
    };

    class BoolParam : public Param {
    public:
        BoolParam(const std::string& _argStr, bool _isRequired)
            : Param(_argStr, _isRequired)
        {
            requiredArg = false;
            value = false;
        }

        virtual std::string type() const {
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

        virtual bool parse(const char *arg = nullptr) {
            if (!arg) {
                this->value = true;
                return true;
            }

            DWORD val = loadInt(arg);
            if (val != 0) {
                this->value = true;
            }
            else {
                this->value = false;
            }
            return true;
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
            Param *p = getParam(str);
            if (!p) return false;

            IntParam *param = dynamic_cast<IntParam*>(p);
            if (!param) {
                return false;
            }
            param->value = val;
            return true;
        }

        bool setInfo(const std::string& str, const std::string& info)
        {
            Param *p = getParam(str);
            if (!p) return false;

            p->info = info;
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

        virtual bool hasRequiredFilled()
        {
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                Param *param = itr->second;
                if (param->isRequired && !param->isSet()) {
                    return false;
                }
            }
            return true;
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
        void info(bool hilightMissing = false);
        bool parse(int argc, char* argv[]);

    protected:
        void printDesc(const Param &param);
        size_t countRequired();
        size_t countOptional();

        Param * getParam(const std::string &str)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(str);
            if (itr != this->myParams.end()) {
                return itr->second;
            }
            return nullptr;
        }

        std::map<std::string, Param*> myParams;
    };
};


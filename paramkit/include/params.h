#pragma once

#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include "pk_util.h"
#include "term_colors.h"

#define WARNING_COLOR YELLOW
#define HILIGHTED_COLOR WHITE

#define HEADER_COLOR YELLOW
#define SEPARATOR_COLOR BROWN
//--

#define PARAM_HELP1 "?"
#define PARAM_HELP2 "help"

namespace paramkit {

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

        std::string m_info;
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


    void print_param_in_color(int color, const std::string &text)
    {
        print_in_color(color, PARAM_SWITCH1 + text);
    }

    //---

    class Params {
    public:
        Params() {}
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

            p->m_info = info;
            return false;
        }

        void info(bool hilightMissing = false)
        {
            const int hdr_color = HEADER_COLOR;
            const int param_color = HILIGHTED_COLOR;

            std::map<std::string, Param*>::iterator itr;

            if (countRequired() > 0) {
                print_in_color(hdr_color, "Required: \n");
                //Print Required
                for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                    Param *param = itr->second;
                    if (!param || !param->isRequired) continue;
                    int color = param_color;
                    if (hilightMissing && !param->isSet()) {
                        color = WARNING_COLOR;
                    }
                    print_param_in_color(color, param->argStr);
                    printDesc(*param);
                }
            }
            if (countOptional() > 0) {
                print_in_color(hdr_color, "\nOptional: \n");
                //Print Optional
                for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                    Param *param = itr->second;
                    if (!param || param->isRequired) continue;

                    print_param_in_color(param_color, param->argStr);
                    printDesc(*param);
                }
            }

            print_in_color(hdr_color, "\nInfo: \n");
            print_param_in_color(param_color, PARAM_HELP2);
            std::cout << " : " << "Print this help\n";
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

        void print_unknown_param(const std::string &param)
        {
            print_in_color(WARNING_COLOR, "Invalid parameter: ");
            std::cout << param << "\n";
        }

        template <typename T_CHAR>
        bool parse(int argc, T_CHAR* argv[])
        {
            size_t count = 0;
            for (int i = 1; i < argc; i++) {
                if (!is_param(argv[i])) {
                    continue;
                }
                bool found = false;

                std::string param_str = to_string(argv[i]);

                std::map<std::string, Param*>::iterator itr;
                for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                    Param *param = itr->second;
                    if (param_str == PARAM_HELP2 || param_str == PARAM_HELP1)
                    {
                        return false;
                    }

                    if (param_str == param->argStr) {
                        if ((i + 1) < argc && !(is_param(argv[i + 1]))) {
                            param->parse(argv[i + 1]);
                            found = true;
#ifdef _DEBUG
                            std::cout << argv[i] << " : " << argv[i + 1] << "\n";
#endif
                            break;
                        }
                        else if (!param->requiredArg) {
                            param->parse((char*)nullptr);
                            found = true;
                            break;
                        }
                    }
                }
                if (found) {
                    count++;
                }
                else {
                    //const std::string param_str = to_string(argv[i]);
                    //print_unknown_param(param_str);
                    return false;
                }
            }
            return (count > 0) ? true : false;
        }

        void print()
        {
            const int param_color = HILIGHTED_COLOR;

            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                if (!isSet(itr->first)) continue;

                Param *param = itr->second;
                print_param_in_color(param_color, param->argStr);
                std::cout << ": ";
                std::cout << "\n\t" << std::hex << param->valToString() << "\n";
            }
        }

    protected:
        void Params::printDesc(const Param &param)
        {
            if (param.requiredArg) {
                if (param.typeDescStr.length()) {
                    std::cout << " <" << param.typeDescStr << ">";
                }
                else {
                    std::cout << " <" << param.type() << ">";
                }

                std::cout << "\n\t";
            }
            std::cout << " : " << param.m_info << "\n";
        }

        size_t countRequired()
        {
            size_t count = 0;
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                Param *param = itr->second;
                if (param->isRequired) count++;
            }
            return count;
        }

        size_t countOptional()
        {
            size_t count = 0;
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                Param *param = itr->second;
                if (!param->isRequired) count++;
            }
            return count;
        }

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


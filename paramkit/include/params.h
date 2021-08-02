/**
* @file
* @brief   Definitions of the basic parameter types, and the parameters container
*/

#pragma once

#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include "pk_util.h"
#include "term_colors.h"

#define WARNING_COLOR RED
#define HILIGHTED_COLOR WHITE

#define HEADER_COLOR YELLOW
#define SEPARATOR_COLOR BROWN
//--

#define PARAM_HELP1 "?"
#define PARAM_HELP2 "help"

#define PARAM_UNINITIALIZED (-1)

#define PARAM_SWITCH1 '/' ///< The switch used to recognize that the given string should be treated as a parameter (variant 1)
#define PARAM_SWITCH2 '-' ///< The switch used to recognize that the given string should be treated as a parameter (variant 2)

namespace paramkit {

    //! The base class of a parameter
    class Param {
    public:

        //! A constructor of a parameter
        /**
        \param _argStr : the name of the parameter
        \param _isRequired : the flag if this is a required parameter (if false, the parameter is optional)
        */
        Param(const std::string& _argStr, bool _isRequired)
        {
            isRequired = _isRequired;
            argStr = _argStr;
            requiredArg = false;
        }

        //! A constructor of a parameter
        /**
        \param _argStr : the name of the parameter
        \param _typeDescStr : a description of the parameter type
        \param _isRequired : the flag if this is a required parameter (if false, the parameter is optional)
        */
        Param(const std::string& _argStr, const std::string& _typeDescStr, bool _isRequired)
        {
            isRequired = _isRequired;
            argStr = _argStr;
            typeDescStr = _typeDescStr;
            requiredArg = false;
        }

        //! Returns the string representation of the parameter's value
        virtual std::string valToString() = 0;

        //! Returns the string representation of the parameter's type
        virtual std::string type() const = 0;

        //! Parses the parameter from the given string
        virtual bool parse(const char *arg) = 0;

        //! Parses the parameter from the given wide string
        virtual bool parse(const wchar_t *arg)
        {
            std::wstring value = arg;
            std::string str(value.begin(), value.end());
            return parse(str.c_str());
        }

        //! Returns true if the parameter is filled, false otherwise.
        virtual bool isSet() = 0;

    protected:
        std::string argStr; ///< a unique name of the parameter

        std::string typeDescStr; ///< a description of the type of the parameter: what type of values are allowed
        std::string m_info; ///< an information about the the parameter's purpose

        bool isRequired; ///< a flag indicating if this parameter is required
        bool requiredArg; ///< a flag indicating if this parameter needs to be followed by a value

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

        size_t copyToCStr(wchar_t *buf, size_t buf_len)
        {
            buf_len = buf_len * sizeof(wchar_t);
            size_t len = (value.length() + 1) * sizeof(wchar_t);
            if (len > buf_len) len = buf_len;

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
            this->value = loadBoolean(arg);
            return true;
        }

        bool value;
    };

    //---

    //! The class responsible for storing and parsing parameters
    class Params {
    public:
        Params() {}
        virtual ~Params() { releaseParams(); }

        //! Adds a parameter into the storage
        /**
        \param param : an object inheriting from the class Param
        */
        void addParam(Param* param)
        {
            if (!param) return;
            const std::string argStr = param->argStr;
            this->myParams[argStr] = param;
        }

        //! Sets the information about the parameter, defined by its name
        /**
        \param paramName : a unique name of the parameter
        \param info : the description of the parameter
        \return true if setting the info was successful
        */
        bool setInfo(const std::string& paramName, const std::string& info)
        {
            Param *p = getParam(paramName);
            if (!p) return false;

            p->m_info = info;
            return false;
        }

        //! Prints info about all the parameters. Optionally hilights the required ones that are missing.
        /**
        \param hilightMissing : if set, the required parameters that were not filled are printed in red.
        */
        void info(bool hilightMissing = false)
        {
            const int hdr_color = HEADER_COLOR;
            const int param_color = HILIGHTED_COLOR;

            std::map<std::string, Param*>::iterator itr;

            if (countRequired() > 0) {
                printInColor(hdr_color, "Required: \n\n");
                //Print Required
                for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                    Param *param = itr->second;
                    if (!param || !param->isRequired) continue;
                    int color = param_color;
                    if (hilightMissing && !param->isSet()) {
                        color = WARNING_COLOR;
                    }
                    Params::printParamInColor(color, param->argStr);
                    printDesc(*param);
                }
            }
            if (countOptional() > 0) {
                printInColor(hdr_color, "\nOptional: \n\n");
                //Print Optional
                for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                    Param *param = itr->second;
                    if (!param || param->isRequired) continue;

                    Params::printParamInColor(param_color, param->argStr);
                    printDesc(*param);
                }
            }

            printInColor(hdr_color, "\nInfo: \n\n");
            Params::printParamInColor(param_color, PARAM_HELP2);
            std::cout << " : " << "Print this help\n";
        }

        //! Fills an IntParam defined by its name with the given value. If such parameter does not exist, or is not of the type IntParam, returns false. Otherwise returns true.
        /**
        \param paramName : a name of the parameter (of the type IntParam) that is to be filled
        \param val : the value to be set into the parameter
        \return true if setting the value was successful
        */
        bool setIntValue(const std::string& paramName, uint64_t val)
        {
            Param *p = getParam(paramName);
            if (!p) return false;

            IntParam *param = dynamic_cast<IntParam*>(p);
            if (!param) {
                return false;
            }
            param->value = val;
            return true;
        }

        //! Gets an integer value of the IntParam defined by its name. If such parameter does not exist, or is not of IntParam type, returns PARAM_UNINITIALIZED.
        /**
        \param paramName : a name of the parameter (of the type IntParam) which's value is to be retrieved
        */
        uint64_t getIntValue(const std::string& paramName)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(paramName);
            if (itr == this->myParams.end()) return PARAM_UNINITIALIZED;

            IntParam *param = dynamic_cast<IntParam*>(itr->second);
            if (!param) {
                return false;
            }
            return param->value;
        }

        //! Checks if the parameter with the given name is set (filled).
        /**
        \param paramName : a name of the parameter
        \return true if the parameter with the given name exists and is set
        */
        virtual bool isSet(const std::string& paramName)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(paramName);
            if (itr == this->myParams.end()) return false;

            Param *param = itr->second;
            if (!param) {
                return false;
            }
            return param->isSet();
        }

        //! Checks if all the required parameters are filled.
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

        //! Deletes all the added parameters.
        void releaseParams()
        {
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                Param *param = itr->second;
                delete param;
            }
            myParams.clear();
        }

        //! Parses the parameters. Prints a warning if an undefined parameter was supplied.
        template <typename T_CHAR>
        bool parse(int argc, T_CHAR* argv[])
        {
            size_t count = 0;
            for (int i = 1; i < argc; i++) {
                std::string param_str = to_string(argv[i]);
                if (!isParam(param_str)) {
                    continue;
                }
                bool found = false;
                param_str = param_str.substr(1); // skip the first char

                std::map<std::string, Param*>::iterator itr;
                for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                    Param *param = itr->second;
                    if (param_str == PARAM_HELP2 || param_str == PARAM_HELP1)
                    {
                        return false;
                    }

                    if (param_str == param->argStr) {
                        if ((i + 1) < argc && !(isParam(to_string(argv[i + 1])))) {
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
                    const std::string param_str = to_string(argv[i]);
                    printUnknownParam(param_str);
                    return false;
                }
            }
            return (count > 0) ? true : false;
        }

        //! Prints the values of all the parameters that are currently set.
        void print()
        {
            const int param_color = HILIGHTED_COLOR;
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                if (!isSet(itr->first)) continue;

                Param *param = itr->second;
                Params::printParamInColor(param_color, param->argStr);
                std::cout << ": ";
                std::cout << "\n\t" << std::hex << param->valToString() << "\n";
            }
        }

    protected:
        void printUnknownParam(const std::string &param)
        {
            printInColor(WARNING_COLOR, "Invalid parameter: ");
            std::cout << param << "\n";
        }

        //! Prints a formatted description of the parameter, including its unique name, type, and the info.
        void printDesc(const Param &param)
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

        //! Returns the number of required parameters.
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

        //! Returns the number of optional parameters.
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

        //! Retrieve the parameter by its unique name. Returns nullptr if such parameter does not exist.
        Param* getParam(const std::string &str)
        {
            std::map<std::string, Param*>::iterator itr = this->myParams.find(str);
            if (itr != this->myParams.end()) {
                return itr->second;
            }
            return nullptr;
        }

        //! Checks if the string starts from the parameter switch.
        static bool isParam(const std::string &str)
        {
            if (str.length() == 0) return false;

            const size_t len = str.length();
            if (len < 2) return false;

            if (str[0] == PARAM_SWITCH1 || str[0] == PARAM_SWITCH2) {
                return true;
            }
            return false;
        }

        //! Prints the parameter using the given color. Appends the parameter switch to the name.
        static void printParamInColor(int color, const std::string &text)
        {
            printInColor(color, PARAM_SWITCH1 + text);
        }

        std::map<std::string, Param*> myParams;
    };
};


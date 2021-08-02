/**
* @file
* @brief   Basic parameter types
*/

#pragma once

#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>

#include "pk_util.h"

#define PARAM_UNINITIALIZED (-1)

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

    //! A parameter storing an integer value
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

    //! A parameter storing a string value
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
            if (!arg) return false;

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

    //! A parameter storing a wide string value
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

    //! A parameter storing a boolean value
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

};


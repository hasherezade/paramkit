/**
* @file
* @brief   Basic parameter types
*/

#pragma once

#include <windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <set>

#include "pk_util.h"
#include "strings_util.h"

#define PARAM_UNINITIALIZED (-1)
#define INFO_SPACER "\t   "

#define PARAM_SWITCH1 '/' ///< The switch used to recognize that the given string should be treated as a parameter (variant 1)
#define PARAM_SWITCH2 '-' ///< The switch used to recognize that the given string should be treated as a parameter (variant 2)

typedef unsigned __int64 uint64_t;

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
            active = true;
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
            active = true;
        }

        //! Returns the string representation of the parameter's value
        virtual std::string valToString() const = 0;

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

        void setActive(bool _active)
        {
            this->active = _active;
        }

        //! Returns true if the parameter is active, false otherwise.
        virtual bool isActive() const
        {
            return this->active;
        }

        //! Returns true if the parameter is filled, false otherwise.
        virtual bool isSet() const = 0;

        virtual std::string info(bool isExtended) const
        {
            std::stringstream ss;
            ss << m_info;
            if (isExtended) {
                const std::string eInfo = extendedInfo();
                if (eInfo.length()) {
                    ss << "\n" << eInfo;
                }
            }
            return ss.str();
        }

        //! Prints the parameter using the given color. Appends the parameter switch to the name.
        void printInColor(int color)
        {
            print_in_color(color, PARAM_SWITCH1 + this->argStr);
        }

    protected:

        //! Prints a formatted description of the parameter, including its unique name, type, and the info.
        void printDesc(bool isExtended = true) const
        {
            if (requiredArg) {
                if (typeDescStr.length()) {
                    std::cout << " <" << typeDescStr << ">";
                }
                else {
                    std::cout << " <" << type() << ">";
                }
            }
            std::cout << "\n\t";
            std::cout << " : " << info(isExtended);
            std::cout << "\n";
        }

        //! Checks if the param name is similar to the given filter
        virtual bool isNameSimilar(const std::string &filter)
        {
            util::stringsim_type sim_type = util::is_string_similar(argStr, filter);
            return (sim_type != util::SIM_NONE) ? true : false;
        }

        //! Checks if the parameter type contains the keyword
        virtual bool isKeywordInType(const std::string &keyword)
        {
            if (util::has_keyword(this->type(), keyword)) {
                return true;
            }
            return false;
        }

        //! Checks if the description contains the keyword
        virtual bool isKeywordInDescription(const std::string &keyword)
        {
            if (util::has_keyword(m_info, keyword)) return true;

            util::stringsim_type sim_type = util::has_keyword(m_extInfo, keyword);
            return (sim_type != util::SIM_NONE) ? true : false;
        }

        //! Extended information
        virtual std::string extendedInfo() const
        {
            return m_extInfo;
        }

        std::string argStr; ///< a unique name of the parameter

        std::string typeDescStr; ///< a description of the type of the parameter: what type of values are allowed
        std::string m_info; ///< a basic information about the the parameter's purpose
        std::string m_extInfo; ///< an extended information about the the parameter's purpose

        bool isRequired; ///< a flag indicating if this parameter is required
        bool requiredArg; ///< a flag indicating if this parameter needs to be followed by a value
        bool active; ///< a flag indicating if this parameter is available

        friend class Params;
        friend class ParamCompare;
        friend class ParamGroup;
    };

    //! A comparator class for Param class
    class ParamCompare
    {
    public:
        bool operator()(Param* param1, Param* param2) const
        {
            if (!param1 || !param2) return false;
            bool val = ((param1->argStr.compare(param2->argStr) < 0));
            return val;
        }
    };

    //! A parameter storing an integer value
    class IntParam : public Param {
    public:
        
        typedef enum
        {
            INT_BASE_ANY = 0,
            INT_BASE_DEC = 1,
            INT_BASE_HEX = 2,
            INT_BASE_COUNT
        } t_int_base;

        IntParam(const std::string& _argStr, bool _isRequired, t_int_base _base = INT_BASE_ANY)
            : Param(_argStr, _isRequired), 
            base(_base)
        {
            requiredArg = true;
            value = PARAM_UNINITIALIZED;
        }

        virtual std::string valToString() const
        {
            std::stringstream stream;
            if (base == INT_BASE_HEX) {
                stream << std::hex;
            }
            else {
                stream << std::dec;
            }
            stream << value;
            return stream.str();
        }

        virtual std::string type() const {
            if (base == INT_BASE_HEX) {
                return "integer: hex";
            }
            if (base == INT_BASE_DEC) {
                return "integer: dec";
            }
            return "integer: decimal, or hexadecimal with '0x' prefix";
        }

        virtual bool isSet() const
        {
            return value != PARAM_UNINITIALIZED;
        }

        virtual bool parse(const char *arg)
        {
            if (!arg) return false;
            const size_t len = strlen(arg);

            if (!isValidNumber(arg, len)) {
                return false;
            }
            bool isHex = (base == INT_BASE_HEX);
            if (base == INT_BASE_ANY) {
                if (paramkit::is_hex_with_prefix(arg)) {
                    isHex = true;
                }
            }
            this->value = loadInt(arg, isHex);
            return true;
        }

        bool isValidNumber(const char *arg, const size_t len)
        {
            if (base == INT_BASE_ANY) {
                if (paramkit::is_hex_with_prefix(arg) || paramkit::is_dec(arg, len)) {
                    return true;
                }
            }
            if (base == INT_BASE_HEX) {
                if (paramkit::is_hex(arg, len) || paramkit::is_hex_with_prefix(arg)) {
                    return true;
                }
            }
            if (base == INT_BASE_DEC) {
                if (paramkit::is_dec(arg, len)) return true;
            }
            return false;
        }

        t_int_base base;
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

        virtual std::string valToString() const
        {
            return "\"" + value + "\"";
        }

        virtual std::string type() const {
            return "string";
        }

        virtual bool isSet() const
        {
            return value.length() > 0;
        }

        virtual bool parse(const char *arg)
        {
            if (!arg) return false;

            this->value = arg;
            return true;
        }

        //! Copy the stored string value into an external buffer of a given length
        size_t copyToCStr(char *buf, size_t buf_max) const
        {
            return copy_to_cstr(value, buf, buf_max);
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

        virtual std::string valToString() const
        {
            std::string str(value.begin(), value.end());
            return "\"" + str + "\"";
        }

        virtual std::string type() const
        {
            return "wstring";
        }

        virtual bool isSet() const
        {
            return value.length() > 0;
        }

        virtual bool parse(const wchar_t *arg)
        {
            if (!arg) return false;

            this->value = arg;
            return true;
        }

        virtual bool parse(const char *arg)
        {
            if (!arg) return false;

            std::string value = arg;
            std::wstring str(value.begin(), value.end());

            this->value = str;
            return true;
        }

        //! Copy the stored string value into an external buffer of a given length
        size_t copyToCStr(wchar_t *buf, size_t buf_len) const
        {
            return copy_to_cstr(value, buf, buf_len);
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
            isParsed = false;
        }

        virtual std::string type() const
        {
            return "bool";
        }

        virtual std::string valToString() const
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

        virtual bool isSet() const
        {
            return isParsed;
        }

        virtual bool parse(const char *arg = nullptr)
        {
            if (!arg) {
                this->value = true;
                this->isParsed = true;
                return true;
            }
            this->isParsed = loadBoolean(arg, this->value);
            return this->isParsed;
        }

        bool value;
        bool isParsed;
    };


    //! A parameter storing an enum value
    class EnumParam : public Param {
    public:
        EnumParam(const std::string& _argStr, const std::string _enumName, bool _isRequired)
            : Param(_argStr, _isRequired), enumName(_enumName), m_isSet(false)
        {
            requiredArg = true;
            value = PARAM_UNINITIALIZED;
        }

        bool addEnumValue(int value, const std::string &info)
        {
            enumToInfo[value] = info;
            return true;
        }

        bool addEnumValue(int value, const std::string &str_val, const std::string &info)
        {
            if (addEnumValue(value, info)) {
                enumToString[value] = str_val;
            }
            return true;
        }

        virtual std::string valToString() const
        {
            if (!isSet()) {
                return "(undefined)";
            }
            std::map<int, std::string>::const_iterator foundString = enumToString.find(value);
            if (foundString != enumToString.end()) {
                return foundString->second;
            }
            std::stringstream stream;
            stream << std::dec << value;
            return stream.str();
        }

        virtual std::string type() const
        {
            return "*" + enumName;
        }

        virtual bool isSet() const
        {
            if (!m_isSet) return false;
            if (!isInEnumScope(value)) {
                return false;
            }
            return true;
        }

        virtual bool isKeywordInType(const std::string &keyword)
        {
            if (util::has_keyword(enumName, keyword) != util::SIM_NONE) {
                return true;
            }
            return false;
        }

        bool isKeywordInDescription(const std::string &keyword)
        {
            if (Param::isKeywordInDescription(keyword)) {
                return true;
            }
            if (isKeywordInType(keyword)) {
                return true;
            }
            // search the keyword also in the descriptions of particulat options:
            std::map<int, std::string>::const_iterator itr;
            for (itr = enumToInfo.begin(); itr != enumToInfo.end(); ++itr) {
                const std::string valDesc = itr->second;
                util::stringsim_type sim_type = util::has_keyword(valDesc, keyword);
                if (sim_type != util::SIM_NONE) return true;
            }
            return false;
        }

        virtual bool parse(const char *arg)
        {
            if (!arg) return false;

            //try to find by the string representation first:
            const std::string strVal = arg;
            std::map<int, std::string>::iterator itr;
            for (itr = enumToString.begin(); itr != enumToString.end(); ++itr) {
                if (strVal == itr->second) {
                    this->value = itr->first;
                    m_isSet = true;
                    return true;
                }
            }
            //try to find by the integer representation:
            if (!is_number(arg)) {
                return false;
            }
            int intVal = loadInt(arg);
            if (!isInEnumScope(intVal)) {
                // out of the enum scope
                return false;
            }
            this->value = intVal;
            m_isSet = true;
            return true;
        }

        int value;

    protected:

        std::string extendedInfo() const
        {
            std::stringstream stream;
            stream << Param::extendedInfo();
            stream << optionsInfo();
            return stream.str();
        }

        std::string optionsInfo() const
        {
            std::stringstream stream;
            std::map<int, std::string>::const_iterator itr;
            stream << type() << ":\n";
            for (itr = enumToInfo.begin(); itr != enumToInfo.end(); ) {
                int val = itr->first;
                std::map<int, std::string>::const_iterator foundString = enumToString.find(itr->first);

                stream << "\t" << std::dec << val;
                if (foundString != enumToString.end()) {
                    stream << " (" << foundString->second << ")";
                }
                stream << " - ";
                stream << itr->second;
                ++itr;
                if (itr != enumToInfo.end()) {
                    stream << "\n";
                }
            }
            return stream.str();
        }

        bool isInEnumScope(int intVal)const 
        {
            if (enumToInfo.find(intVal) != enumToInfo.end()) {
                return true;
            }
            return false;
        }

        std::map<int, std::string> enumToString; ///< optional: string representation of the enum parameter
        std::map<int, std::string> enumToInfo; ///< required: info about the enum parameter

        std::string enumName;
        bool m_isSet;
    };


    class StringListParam : public StringParam {
    public:
        StringListParam(const std::string& _argStr, bool _isRequired, char _delimiter)
            : StringParam(_argStr, _isRequired),
            delimiter(std::string(1, _delimiter))
        {
        }

        StringListParam(const std::string& _argStr, bool _isRequired, std::string _delimiter)
            : StringParam(_argStr, _isRequired),
            delimiter(_delimiter)
        {
        }

        virtual std::string type() const
        {
            return "list: separated by \'" + delimiter + "\'";
        }

        size_t stripToElements(OUT std::set<std::string> &elements_list)
        {
            return strip_to_list(this->value, this->delimiter, elements_list);
        }

        const std::string delimiter;
    };

    class IntListParam : public StringListParam {
    public:
        IntListParam(const std::string& _argStr, bool _isRequired, char _delimiter)
            : StringListParam(_argStr, _isRequired, _delimiter)
        {
        }

        IntListParam(const std::string& _argStr, bool _isRequired, std::string _delimiter)
            : StringListParam(_argStr, _isRequired, _delimiter)
        {
        }

        virtual std::string type() const
        {
            return "list: dec or hex, separated by \'" + delimiter + "\'";
        }

        virtual bool parse(const char *arg)
        {
            if (!arg) return false;

            std::set<std::string> str_list;
            if (!strip_to_list(arg, this->delimiter, str_list)) {
                return false;
            }
            std::set<std::string>::iterator itr;
            for (itr = str_list.begin(); itr != str_list.end(); ++itr) {
                std::string nextEl = *itr;
                if (!paramkit::is_number(nextEl.c_str())) return false;
            }
            this->value = arg;
            return true;
        }

        size_t stripToIntElements(OUT std::set<long> &elements_list)
        {
            std::set<std::string> str_list;
            if (!stripToElements(str_list)) {
                return 0;
            }
            std::set<std::string>::iterator itr;
            for (itr = str_list.begin(); itr != str_list.end(); ++itr) {
                std::string nextEl = *itr;
                if (!paramkit::is_number(nextEl.c_str())) continue;

                long number = paramkit::get_number(nextEl.c_str());
                elements_list.insert(number);
            }
            return elements_list.size();
        }

    };
};


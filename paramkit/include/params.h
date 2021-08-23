/**
* @file
* @brief   The parameters container
*/

#pragma once

#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include "pk_util.h"
#include "term_colors.h"
#include "param.h"

#define WARNING_COLOR RED
#define HILIGHTED_COLOR WHITE

#define HEADER_COLOR YELLOW
#define SEPARATOR_COLOR BROWN
//--

#define PARAM_HELP1 "?"
#define PARAM_HELP2 "help"

#define PARAM_SWITCH1 '/' ///< The switch used to recognize that the given string should be treated as a parameter (variant 1)
#define PARAM_SWITCH2 '-' ///< The switch used to recognize that the given string should be treated as a parameter (variant 2)

namespace paramkit {

    //! Prints the parameter using the given color. Appends the parameter switch to the name.
    static void printParamInColor(int color, const std::string &text)
    {
        print_in_color(color, PARAM_SWITCH1 + text);
    }

    //---
    class ParamGroup {
    public:
        ParamGroup(const std::string& _name)
        {
            this->name = _name;
        }

        size_t printGroup(bool printGroupName, bool printRequired, bool hilightMissing)
        {
            if (countParams(printRequired) == 0) {
                return 0;
            }
            size_t printed = 0;
            const int param_color = HILIGHTED_COLOR;
            const int separator_color = SEPARATOR_COLOR;

            if (printGroupName && name.length()) {
                print_in_color(separator_color, "\n---" + name + "---\n");
            }
            std::set<Param*>::iterator itr;
            for (itr = params.begin(); itr != params.end(); ++itr) {
                Param* param = (*itr);

                if (!param) continue;
                if (printRequired != param->isRequired) continue;

                int color = param_color;
                if (hilightMissing && param->isRequired && !param->isSet()) {
                    color = WARNING_COLOR;
                }
                printParamInColor(color, param->argStr);
                param->printDesc();
                printed++;
            }
            return printed;
        }

    protected:

        size_t countParams(bool printRequired)
        {
            size_t printed = 0;
            std::set<Param*>::iterator itr;
            for (itr = params.begin(); itr != params.end(); ++itr) {
                Param* param = (*itr);

                if (!param) continue;
                if (printRequired != param->isRequired) continue;

                printed++;
            }
            return printed;
        }

        bool hasParam(Param *param)
        {
            std::set<Param*>::iterator itr = params.find(param);
            if (itr != params.end()) {
                return true;
            }
            return false;
        }

        bool addParam(Param *param)
        {
            if (hasParam(param)) return false;
            this->params.insert(param);
            return false;
        }

        bool removeParam(Param *param)
        {
            std::set<Param*>::iterator itr = params.find(param);
            if (itr != params.end()) {
                params.erase(itr);
                return true;
            }
            return false;
        }

        std::string name;
        std::set<Param*, ParamCompare> params;

        friend class Params;
    };


    //! The class responsible for storing and parsing parameters
    class Params {
    public:
        Params() : generalGroup(nullptr)
        {
        }

        virtual ~Params()
        {
            releaseGroups();
            releaseParams();
        }

        bool addGroup(ParamGroup *group)
        {
            if(!group) return false;
            if (this->paramGroups.find(group->name) != this->paramGroups.end()) {
                return false;
            }
            this->paramGroups[group->name] = group;
        }

        ParamGroup* getParamGroup(const std::string &str)
        {
            std::map<std::string, ParamGroup*>::iterator itr = this->paramGroups.find(str);
            if (itr != this->paramGroups.end()) {
                return itr->second;
            }
            return nullptr;
        }

        bool addParamToGroup(const std::string paramName, const std::string groupName)
        {
            Param* param = this->getParam(paramName);
            ParamGroup *group = this->getParamGroup(groupName);
            return addParamToGroup(param, group);
        }

        //! Adds a parameter into the storage
        /**
        \param param : an object inheriting from the class Param
        */
        void addParam(Param* param)
        {
            if (!param) return;
            const std::string argStr = param->argStr;
            this->myParams[argStr] = param;
            if (!generalGroup) {
                generalGroup = new ParamGroup("");
                this->addGroup(generalGroup);
            }
            this->addParamToGroup(param, this->generalGroup);
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
                print_in_color(hdr_color, "Required: \n");
                //Print Required
                bool printGroupName = (countGroups(true)) ? true : false;
                if (paramGroups.size() > 0) {
                    std::map<std::string, ParamGroup*>::iterator groupItr;
                    for (groupItr = this->paramGroups.begin(); groupItr != paramGroups.end(); ++groupItr) {
                        ParamGroup* group = groupItr->second;
                        group->printGroup(printGroupName, true, hilightMissing);
                    }
                }
            }
            if (countOptional() > 0) {
                print_in_color(hdr_color, "\nOptional: \n");
                //Print Optional
                bool printGroupName = (countGroups(false)) ? true : false;
                if (paramGroups.size() > 0) {
                    std::map<std::string, ParamGroup*>::iterator groupItr;
                    for (groupItr = this->paramGroups.begin(); groupItr != paramGroups.end(); ++groupItr) {
                        ParamGroup* group = groupItr->second;
                        group->printGroup(printGroupName, false, hilightMissing);
                    }
                }
            }

            print_in_color(hdr_color, "\nInfo: \n\n");
            printParamInColor(param_color, PARAM_HELP2);
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

        //! Deletes all the parameters groups.
        void releaseGroups()
        {
            paramToGroup.clear();
            this->generalGroup = nullptr;
            std::map<std::string, ParamGroup*>::iterator itr;
            for (itr = paramGroups.begin(); itr != paramGroups.end(); ++itr) {
                ParamGroup *group = itr->second;
                group->params.clear();
                delete group;
            }
            paramGroups.clear();
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
            bool paramHelp = false;
            size_t count = 0;
            for (int i = 1; i < argc; i++) {
                std::string param_str = to_string(argv[i]);
                if (!isParam(param_str)) {
                    continue;
                }
                bool found = false;
                param_str = param_str.substr(1); // skip the first char

                std::map<std::string, Param*>::iterator itr;
                for (itr = myParams.begin(); itr != myParams.end(); ++itr) {
                    Param *param = itr->second;
                    if (param_str == PARAM_HELP2 || param_str == PARAM_HELP1)
                    {
                        this->info(false);
                        return false;
                    }

                    if (param_str == param->argStr) {
                        // has argument:
                        if ((i + 1) < argc && !(isParam(to_string(argv[i + 1])))) {
                            std::string nextVal = to_string(argv[i + 1]);
                            //help requested explicitly
                            if (nextVal == PARAM_HELP1) {
                                paramHelp = true;
                                paramkit::print_in_color(RED, param_str);
                                param->printDesc();
                                found = true;
                                break;
                            }
                            param->parse(argv[i + 1]);
                            found = true;
#ifdef _DEBUG
                            std::cout << argv[i] << " : " << argv[i + 1] << "\n";
#endif
                            break;
                        }
                        // does not require an argument:
                        if (!param->requiredArg) {
                            param->parse((char*)nullptr);
                            found = true;
                            break;
                        }
                        // requires an argument, but it is missing:
                        paramkit::print_in_color(RED, param_str);
                        param->printDesc();
                        found = true;
                        break;
                    }
                }
                if (found) {
                    count++;
                }
                else {
                    const std::string param_str = to_string(argv[i]);
                    printUnknownParam(param_str);
                }
            }
            if (paramHelp) {
                return false;
            }
            if (!this->hasRequiredFilled()) {
                this->info(true);
                return false;
            }
            return true;
        }

        //! Prints the values of all the parameters that are currently set.
        void print()
        {
            const int param_color = HILIGHTED_COLOR;
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                if (!isSet(itr->first)) continue;

                Param *param = itr->second;
                printParamInColor(param_color, param->argStr);
                std::cout << ": ";
                std::cout << "\n\t" << std::hex << param->valToString() << "\n";
            }
        }

    protected:

        bool addParamToGroup(Param *param, ParamGroup *group)
        {
            if (!param || !group) {
                return false;
            }
            std::map<Param*, ParamGroup*>::iterator itr = paramToGroup.find(param);
            if (itr != paramToGroup.end()) {
                ParamGroup* currentGroup = itr->second;
                if (currentGroup != group) {
                    currentGroup->removeParam(param);
                    paramToGroup.erase(param);
                }
            }
            group->params.insert(param);
            paramToGroup[param] = group;
            return true;
        }

        size_t countGroups(bool required) const
        {
            size_t groups_count = 0;
            std::map<std::string, ParamGroup*>::const_iterator itr;
            for (itr = paramGroups.begin(); itr != paramGroups.end(); ++itr) {
                ParamGroup *group = itr->second;
                if (group == this->generalGroup) continue; //skip the general
                if (group->countParams(required) > 0) {
                    groups_count++;
                }
            }
            return groups_count;
        }

        void printUnknownParam(const std::string &param)
        {
            print_in_color(WARNING_COLOR, "Invalid parameter: ");
            std::cout << param << "\n";
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

        std::map<std::string, Param*> myParams;
        ParamGroup *generalGroup;
        std::map<Param*, ParamGroup*> paramToGroup;
        std::map<std::string, ParamGroup*> paramGroups;
    };
};


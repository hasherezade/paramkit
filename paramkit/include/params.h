/**
* @file
* @brief   The main parameters container and parser
*/

#pragma once

#include <windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include "pk_util.h"
#include "color_scheme.h"
#include "param.h"
#include "param_group.h"
//--

#define PARAM_HELP1 "?"
#define PARAM_HELP2 "help"
#define PARAM_VERSION "version"
#define PARAM_VERSION2 "ver"

namespace paramkit {

    //! The class responsible for storing and parsing parameters (objects of the type Param), possibly divided into groups (ParamGroup)
    class Params {
    public:
        Params(const std::string &version = "")
            : generalGroup(nullptr), versionStr(version),
            paramHelp(PARAM_HELP2, false), paramHelpP(PARAM_HELP2, false), paramInfoP("<param> ?", false),
            paramVersion(PARAM_VERSION, false),
            hdrColor(HEADER_COLOR), paramColor(HILIGHTED_COLOR)
        {
            paramHelp.m_info = "Print complete help.";
            paramHelpP.m_info = "Print help about a given keyword.";
            paramInfoP.m_info = "Print details of a given parameter.";
            paramVersion.m_info = "Print version info.";
        }

        virtual ~Params()
        {
            releaseGroups();
            releaseParams();
        }

        virtual void printBanner()
        {
            return;
        }

        virtual void printVersionInfo()
        {
            if (versionStr.length()) {
                std::cout << versionStr << std::endl;
            }
        }

        bool addGroup(ParamGroup *group)
        {
            if(!group) return false;
            if (this->paramGroups.find(group->name) != this->paramGroups.end()) {
                return false;
            }
            this->paramGroups[group->name] = group;
            return true;
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
        \param basic_info : basic description of the parameter
        \param extended_info : additional description of the parameter
        \return true if setting the info was successful
        */
        bool setInfo(const std::string& paramName, const std::string& basic_info, const std::string& extended_info = "")
        {
            Param *p = getParam(paramName);
            if (!p) return false;

            p->m_info = basic_info;
            p->m_extInfo = extended_info;
            return false;
        }

        //! Prints info about all the parameters. Optionally hilights the required ones that are missing.
        /**
        \param hilightMissing : if set, the required parameters that were not filled are printed in red.
        \param filter : display only parameters similar to the given string
        \param isExtended : display extended info about each parameter
        */
        void printInfo(bool hilightMissing=false, const std::string &filter = "", bool isExtended = true)
        {
            std::cout << "---" << std::endl;
            _info(true, hilightMissing, filter, isExtended);
            _info(false, hilightMissing, filter, isExtended);
            const bool extendedInfoS = (filter.empty() && !hilightMissing) ? isExtended : false;
            printInfoSection(extendedInfoS);
            std::cout << "---" << std::endl;
        }

        //! Prints brief info about all the parameters. Wrapper for printInfo.
        void printBriefInfo()
        {
            printInfo(false, "", false);
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
                return 0;
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
                if (param->isRequired && param->isActive() && !param->isSet()) {
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
            bool helpRequested = false;
            size_t count = 0;
            for (int i = 1; i < argc; i++) {
                std::string param_str = to_string(argv[i]);
                if (!isParam(param_str)) {
                    printUnknownArgument(param_str);
                    continue;
                }
                bool found = false;
                param_str = skipParamPrefix(param_str);

                std::map<std::string, Param*>::iterator itr;
                for (itr = myParams.begin(); itr != myParams.end(); ++itr) {
                    bool paramHelp = false;
                    Param *param = itr->second;
                    if (param_str == PARAM_HELP2 || param_str == PARAM_HELP1) {
                        if (param_str == PARAM_HELP2) {
                            const bool hasArg = (i + 1) < argc && !(isParam(to_string(argv[i + 1])));
                            if (hasArg) {
                                const std::string nextVal = to_string(argv[i + 1]);
                                printHelp(nextVal, true);
                                return false;
                            }
                        }
                        const bool shouldExpand = (param_str == PARAM_HELP1) ? false : true;
                        printHelp("", shouldExpand);
                        return false;
                    }
                    if (this->versionStr.length()) {
                        if (param_str == PARAM_VERSION || param_str == PARAM_VERSION2) {
                            this->printVersionInfo();
                            return false;
                        }
                    }
                    if (param_str == param->argStr) {
                        if (!param->isActive()) {
                            paramkit::print_in_color(RED, "WARNING: chosen inactive parameter: " + param_str + "\n");
                        }
                        // has an argument:
                        const bool hasArg = (i + 1) < argc && 
                            ( param->requiredArg || !(isParam(to_string(argv[i + 1]))) );
                        if (hasArg) {
                            const std::string nextVal = to_string(argv[i + 1]);
                            i++; // increment index: move to the next argument
                            found = true;
                            bool isParsed = false;
                            
                            if (nextVal == PARAM_HELP1) {
                                paramHelp = true;
                                helpRequested = true;
                                isParsed = true;
                            }
                            else {
                                isParsed = param->parse(nextVal.c_str());
                                if (!isParsed) {
                                    paramHelp = true;
                                    helpRequested = true;
                                }
                            }

                            //help requested explicitly or parsing failed
                            if (paramHelp) {
                                if (!isParsed) {
                                    paramkit::print_in_color(RED, "Parsing the parameter failed. Correct options:\n");
                                }
                                paramkit::print_in_color(RED, param_str);
                                param->printDesc();
                                break;
                            }
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
                        paramHelp = true;
                        helpRequested = true;
                        param->printDesc();
                        found = true;
                        break;
                    }
                }
                if (found) {
                    count++;
                }
                else {
                    printUnknownParam(param_str);
                    print_in_color(HILIGHTED_COLOR, "Similar parameters:\n");
                    this->printInfo(false, param_str, true);
                    return false;
                }
            }
            if (helpRequested) {
                return false;
            }
            if (!this->hasRequiredFilled()) {
                print_in_color(WARNING_COLOR, "Missing required parameters:\n");
                this->printInfo(true, "", true);
                return false;
            }
            if (this->countCategory(true) == 0 && countFilled(false) == 0) {
                std::stringstream ss1;
                ss1 << "Run with parameter " << PARAM_SWITCH1 << PARAM_HELP1 << " or " << PARAM_SWITCH1 << PARAM_HELP2 << " to see the options...\n";
                print_in_color(YELLOW, ss1.str());
            }
            return true;
        }

        //! Prints the values of all the parameters that are currently set.
        void print()
        {
            const int paramColor = HILIGHTED_COLOR;
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                if (!isSet(itr->first)) continue;

                Param *param = itr->second;
                if (!param) continue; //should never happen

                param->printInColor(paramColor);
                std::cout << ": ";
                std::cout << std::hex << param->valToString() << "\n";
            }
        }

        template <class PARAM_T, typename FIELD_T>
        bool copyVal(const std::string &paramId, FIELD_T &toFill)
        {
            PARAM_T *myParam = dynamic_cast<PARAM_T*>(this->getParam(paramId));
            if (!myParam) {
                return false;
            }
            if (!myParam->isSet()) {
                return false;
            }
            toFill = static_cast<FIELD_T>(myParam->value);
            return true;
        }

        template <class PARAM_T, typename FIELD_T>
        bool copyCStr(const std::string &paramId, FIELD_T &toFill, size_t toFillLen)
        {
            PARAM_T *myStr = dynamic_cast<PARAM_T*>(this->getParam(paramId));
            if (!myStr || !myStr->isSet()) {
                return false;
            }
            myStr->copyToCStr(toFill, toFillLen);
            return true;
        }

    protected:

        virtual size_t countFilled(bool isRequired)
        {
            size_t count = 0;
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                Param *param = itr->second;
                if (param->isRequired != isRequired) continue;
                if (param->isSet()) {
                    count++;
                }
            }
            return count;
        }

        size_t _info(bool isRequired, bool hilightMissing, const std::string &filter, bool isExtended)
        {
            const bool has_filter = filter.length() > 0 ? true : false;
            std::map<std::string, Param*>::iterator itr;
            size_t printed = 0;
            if (countCategory(isRequired) > 0) {
                const std::string desc = isRequired ? "Required:" : "Optional:";
                print_in_color(hdrColor, "\n"+ desc + "\n");

                size_t total_count = 0;
                bool printGroupName = (countGroups(isRequired, hilightMissing, filter)) ? true : false;
                if (paramGroups.size() > 0) {
                    std::map<std::string, ParamGroup*>::iterator groupItr;
                    for (groupItr = this->paramGroups.begin(); groupItr != paramGroups.end(); ++groupItr) {
                        ParamGroup* group = groupItr->second;
                        if (!group) continue; //should never happen
                        printed += group->printGroup(printGroupName, isRequired, hilightMissing, filter, isExtended);
                        total_count += group->countParams(isRequired, false, "");
                    }
                    if (printed < total_count) {
                        print_in_color(INACTIVE_COLOR, "\n[...]\n");
                    }
                }
            }
            return printed;
        }

        bool printHelp(const std::string helpArg, bool shouldExpand)
        {
            if (helpArg.empty()) {
                this->printBanner();
                this->printInfo(false, "", shouldExpand);
                return false;
            }
            if (helpArg == PARAM_HELP1 || helpArg == PARAM_HELP2) {
                printBanner();
                printInfoSection(true);
                return true;
            }
            if (helpArg == PARAM_VERSION || helpArg == PARAM_VERSION2) {
                if (this->versionStr.length()) {
                    paramVersion.printInColor(paramColor);
                    paramVersion.printDesc(true);
                }
                else {
                    std::cout << "Application version is not set\n";
                }
                return true;
            }
            this->printInfo(false, helpArg, shouldExpand);
            return true;
        }

        void printInfoSection(bool isExtended)
        {
            if (isExtended) {
                // make an example:
                if (myParams.size()) {
                    std::map<std::string, Param*>::iterator itr = myParams.begin();
                    std::stringstream ss1;
                    ss1 << INFO_SPACER << "Example: " << PARAM_SWITCH1 << itr->first << " ?";
                    paramInfoP.m_extInfo = ss1.str();
                }
            }
            print_in_color(hdrColor, "\nInfo:\n");
            paramHelp.printInColor(paramColor);
            paramHelp.printDesc(isExtended);
            paramHelpP.printInColor(paramColor);
            paramHelpP.printDesc(isExtended);
            paramInfoP.printInColor(paramColor);
            paramInfoP.printDesc(isExtended);
            if (this->versionStr.length()) {
                paramVersion.printInColor(paramColor);
                paramVersion.printDesc(isExtended);
            }
        }

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

        size_t countGroups(bool required, bool hilightMissing, const std::string &filter) const
        {
            size_t groups_count = 0;
            std::map<std::string, ParamGroup*>::const_iterator itr;
            for (itr = paramGroups.begin(); itr != paramGroups.end(); ++itr) {
                ParamGroup *group = itr->second;
                if (group == this->generalGroup) continue; //skip the general
                if (group->countParams(required, hilightMissing, filter) > 0) {
                    groups_count++;
                }
            }
            return groups_count;
        }

        //! Returns the number of parameters of particular category: required or optional.
        size_t countCategory(bool isRequired)
        {
            size_t count = 0;
            std::map<std::string, Param*>::iterator itr;
            for (itr = myParams.begin(); itr != myParams.end(); itr++) {
                Param *param = itr->second;
                if (param->isRequired == isRequired) count++;
            }
            return count;
        }

        void printUnknownParam(const std::string &param)
        {
            print_in_color(WARNING_COLOR, "Invalid parameter: ");
            std::cout << param << "\n";
        }

        void printUnknownArgument(const std::string &str)
        {
            print_in_color(WARNING_COLOR, "Redundant argument: ");
            std::cout << str << "\n";
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
            const size_t prefixLen = 1;
            const size_t len = str.length();
            if (len <= prefixLen) return false;

            if (str[0] == PARAM_SWITCH1 || str[0] == PARAM_SWITCH2) {
                return true;
            }
            return false;
        }

        //! Skip the parameter prefix. Example: "/param", '-param', or "--param" is converted to "param".
        std::string skipParamPrefix(std::string &str)
        {
            size_t prefixLen = 1;
            const size_t len = str.length();
            if (len < prefixLen) return str;

            if (str[0] != PARAM_SWITCH1 && str[0] != PARAM_SWITCH2) {
                return str;
            }
            if (len > 2 && str[0] == PARAM_SWITCH2) {
                if (str[1] == PARAM_SWITCH2) { // double prefix: "--", i.e. "--param"
                    prefixLen = 2;
                }
            }
            return str.substr(prefixLen); // skip the first char
        }

        std::string versionStr;
        std::map<std::string, Param*> myParams;

        BoolParam paramHelp;
        StringParam paramHelpP;
        BoolParam paramInfoP;

        BoolParam paramVersion;
        ParamGroup *generalGroup;
        std::map<Param*, ParamGroup*> paramToGroup;
        std::map<std::string, ParamGroup*> paramGroups;

        const int hdrColor;
        const int paramColor;
    };
};


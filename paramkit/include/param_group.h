/**
* @file
* @brief   The group of parameters
*/

#pragma once

#include <windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <set>

#include "color_scheme.h"
#include "param.h"
#include "strings_util.h"

namespace paramkit {

    //---
    //! The class responsible for grouping parameters (objects of the type Param)
    class ParamGroup {
    public:
        //! A constructor of a ParamGroup
        /**
        \param _name : a name of the group that will be used to identify it
        */
        ParamGroup(const std::string& _name)
            : hdrColor(HEADER_COLOR), paramColor(HILIGHTED_COLOR), separatorColor(SEPARATOR_COLOR)
        {
            this->name = _name;
        }

        //! Prints the whole group of parameters (their names and descriptions), optionally with the group name
        /**
        \param printGroupName : a flag indicating if the group name will be printed
        \param printRequired : a flag indicating if the required parameters should be printed. If true, only required are printed. If false, only optional are printed.
        \param hilightMissing : a flag indicating if the required parameters that are not filled should be hiligted.
        \param filter : a string that will be searched among the parameters. If filled, only the parameters that are similar to the given string are printed.
        \param isExtended : print extended info about each parameter
        \return number of printed parameters
        */
        size_t printGroup(bool printGroupName, bool printRequired, bool hilightMissing, const std::string &filter = "", bool isExtended = false)
        {
            if (countParams(printRequired, hilightMissing, filter) == 0) {
                return 0;
            }
            const bool has_filter = filter.length() > 0 ? true : false;
            size_t printed = 0;

            if (printGroupName && name.length()) {
                print_in_color(separatorColor, "\n---" + name + "---\n");
            }
            std::set<Param*, ParamCompare>::iterator itr;
            for (itr = params.begin(); itr != params.end(); ++itr) {
                Param* param = (*itr);

                if (!param) continue;
                if (printRequired != param->isRequired) continue;

                bool should_print = hilightMissing ? false : true;
                int color = paramColor;
                if (hilightMissing && param->isRequired && !param->isSet()) {
                    color = WARNING_COLOR;
                    should_print = true;
                }
                if (has_filter) {
                    bool has_any = param->isNameSimilar(filter);
                    color = has_any ? PARAM_SIMILAR_NAME : paramColor;
                    if (!has_any) {
                        has_any = param->isKeywordInDescription(filter);
                        color = has_any ? PARAM_SIMILAR_DESC : paramColor;
                    }
                    if (!has_any) continue;
                }
                if (should_print) {
                    if (!param->isActive()) {
                        color = INACTIVE_COLOR;
                    }
                    param->printInColor(color);
                    param->printDesc(isExtended);
                    printed++;
                }
            }
            return printed;
        }

    protected:

        size_t countParams(bool printRequired, bool hilightMissing, const std::string &filter)
        {
            const bool has_filter = filter.length() > 0 ? true : false;
            size_t printed = 0;
            std::set<Param*, ParamCompare>::iterator itr;
            for (itr = params.begin(); itr != params.end(); ++itr) {
                Param* param = (*itr);

                if (!param) continue;
                if (printRequired != param->isRequired) continue;
                bool should_print = hilightMissing ? false : true;
                if (hilightMissing && param->isRequired && !param->isSet()) {
                    should_print = true;
                }
                if (has_filter) {
                    should_print = false;
                    if (param->isNameSimilar(filter) || param->isKeywordInDescription(filter)) {
                        should_print = true;
                    }
                }
                if (should_print) {
                    printed++;
                }
            }
            return printed;
        }

        bool hasParam(Param *param)
        {
            std::set<Param*, ParamCompare>::iterator itr = params.find(param);
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
            std::set<Param*, ParamCompare>::iterator itr = params.find(param);
            if (itr != params.end()) {
                params.erase(itr);
                return true;
            }
            return false;
        }

        std::string name;
        std::set<Param*, ParamCompare> params;

        const int hdrColor;
        const int paramColor;
        const int separatorColor;

        friend class Params;
    };

};

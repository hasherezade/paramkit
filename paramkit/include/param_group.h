/**
* @file
* @brief   The group of parameters
*/

#pragma once

#include <Windows.h>

#include <iostream>
#include <string>
#include <sstream>
#include <map>

#include "color_scheme.h"
#include "param.h"
#include "strings_util.h"

namespace paramkit {

    //---
    //! The class responsible for grouping parameters
    class ParamGroup {
    public:
        ParamGroup(const std::string& _name)
        {
            this->name = _name;
        }

        size_t printGroup(bool printGroupName, bool printRequired, bool hilightMissing, const std::string &filter = "")
        {
            if (countParams(printRequired, filter) == 0) {
                return 0;
            }
            const bool has_filter = filter.length() > 0 ? true : false;
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
                if (has_filter) {
                    util::stringsim_type sim_type = util::is_string_similar(param->argStr, filter);
                    color = (sim_type != util::SIM_NONE) ? MAKE_COLOR(RED, BLACK) : param_color;
                    if (sim_type == util::SIM_NONE) {
                        //try to find the keyword in the string description
                        sim_type = util::has_keyword(param->m_info, filter.substr(1));
                        color = (sim_type != util::SIM_NONE) ? MAKE_COLOR(SILVER, DARK_RED) : param_color;
                    }
                    if (sim_type == util::SIM_NONE) continue;
                }

                param->printInColor(color);
                param->printDesc();
                printed++;
            }
            return printed;
        }

    protected:

        size_t countParams(bool printRequired, const std::string &filter)
        {
            const bool has_filter = filter.length() > 0 ? true : false;
            size_t printed = 0;
            std::set<Param*>::iterator itr;
            for (itr = params.begin(); itr != params.end(); ++itr) {
                Param* param = (*itr);

                if (!param) continue;
                if (printRequired != param->isRequired) continue;

                if (has_filter) {
                    util::stringsim_type sim_type = util::is_string_similar(param->argStr, filter);
                    if (sim_type == util::SIM_NONE) {
                        //try to find the keyword in the string description
                        sim_type = util::has_keyword(param->m_info, filter.substr(1));
                    }
                    if (sim_type != util::SIM_NONE) printed++;
                    continue;
                }

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

};

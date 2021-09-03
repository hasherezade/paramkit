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
    //! The class responsible for grouping parameters (objects of the type Param)
    class ParamGroup {
    public:
        //! A constructor of a ParamGroup
        /**
        \param _name : a name of the group that will be used to identify it
        */
        ParamGroup(const std::string& _name)
        {
            this->name = _name;
        }

        //! Prints the whole group of parameters (their names and descriptions), optionally with the group name
        /**
        \param printGroupName : a flag indicating if the group name will be printed
        \param printRequired : a flag indicating if the required parameters should be printed. If true, only required are printed. If false, only optional are printed.
        \param hilightMissing : a flag indicating if the required parameters that are not filled should be hiligted.
        \param filter : a string that will be searched among the parameters. If filled, only the parameters that are similar to the given string are printed.
        \return number of printed parameters
        */
        size_t printGroup(bool printGroupName, bool printRequired, bool hilightMissing, const std::string &filter = "")
        {
            if (countParams(printRequired, hilightMissing, filter) == 0) {
                return 0;
            }
            const bool has_filter = filter.length() > 0 ? true : false;
            size_t printed = 0;
            const int param_color = HILIGHTED_COLOR;

            if (printGroupName && name.length()) {
                print_in_color(SEPARATOR_COLOR, "\n---" + name + "---\n");
            }
            std::set<Param*>::iterator itr;
            for (itr = params.begin(); itr != params.end(); ++itr) {
                Param* param = (*itr);

                if (!param) continue;
                if (printRequired != param->isRequired) continue;
                bool should_print = hilightMissing ? false : true;
                int color = param_color;
                if (hilightMissing && param->isRequired && !param->isSet()) {
                    color = WARNING_COLOR;
                    should_print = true;
                }
                if (has_filter) {
                    util::stringsim_type sim_type = util::is_string_similar(param->argStr, filter);
                    color = (sim_type != util::SIM_NONE) ? PARAM_SIMILAR_NAME : param_color;
                    if (sim_type == util::SIM_NONE) {
                        //try to find the keyword in the string description
                        sim_type = util::has_keyword(param->m_info, filter.substr(1));
                        color = (sim_type != util::SIM_NONE) ? PARAM_SIMILAR_DESC : param_color;
                    }
                    if (sim_type == util::SIM_NONE) continue;
                }
                if (should_print) {
                    param->printInColor(color);
                    param->printDesc();
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
            std::set<Param*>::iterator itr;
            for (itr = params.begin(); itr != params.end(); ++itr) {
                Param* param = (*itr);

                if (!param) continue;
                if (printRequired != param->isRequired) continue;
                bool should_print = hilightMissing ? false : true;
                if (hilightMissing && param->isRequired && !param->isSet()) {
                    should_print = true;
                }
                if (has_filter) {
                    util::stringsim_type sim_type = util::is_string_similar(param->argStr, filter);
                    if (sim_type == util::SIM_NONE) {
                        //try to find the keyword in the string description
                        sim_type = util::has_keyword(param->m_info, filter.substr(1));
                    }
                    if (sim_type != util::SIM_NONE) printed++;
                    continue;
                }
                if (should_print) {
                    printed++;
                }
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

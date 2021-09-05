/**
* @file
* @brief   The set of utility functions related with string processing, and finding similarity between strings
*/

#pragma once
#include <string>

namespace paramkit {

    namespace util {

        enum stringsim_type {
            SIM_NONE = 0,
            SIM_SUBSTR = 1,
            SIM_LAV_DIST,
            SIM_HIST
        };

        std::string to_lowercase(std::string);

        bool is_cstr_equal(char const *a, char const *b, const size_t max_len, bool ignoreCase = true);
        bool strequals(const std::string& a, const std::string& b, bool ignoreCase = true);

        // Calculate Levenshtein distance of two strings
        size_t levenshtein_distance(const char s1[], const char s2[]);

        // Check a similarity in strings histograms
        bool has_similar_histogram(const char s1[], const char s2[]);

        stringsim_type has_keyword(const std::string param, const std::string filter);

        stringsim_type is_string_similar(const std::string &param, const std::string &filter);
    }; //namespace util

}; // namespace paramkit

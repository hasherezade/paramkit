#include "strings_util.h"

#include <algorithm>
#include <cstring>

#define MIN(x,y) ((x) < (y) ? (x) : (y))

std::string paramkit::util::to_lowercase(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), tolower);
    return str;
}

bool paramkit::util::is_cstr_equal(char const *a, char const *b, const size_t max_len, bool ignoreCase)
{
    if (a == b) return true;
    if (!a || !b) return false;
    for (size_t i = 0; i < max_len; ++i) {
        if (ignoreCase) {
            if (tolower(a[i]) != tolower(b[i])) {
                return false;
            }
        }
        else {
            if (a[i] != b[i]) {
                return false;
            }
        }
        if (a[i] == '\0') break;
    }
    return true;
}

bool paramkit::util::strequals(const std::string& a, const std::string& b, bool ignoreCase)
{
    size_t aLen = a.size();
    if (b.size() != aLen) return false;

    for (size_t i = 0; i < aLen; ++i) {
        if (!ignoreCase) {
            if (a[i] != b[i]) return false;
        }
        else {
            if (tolower(a[i]) != tolower(b[i])) return false;
        }
    }
    return true;
}

size_t paramkit::util::levenshtein_distance(const char s1[], const char s2[])
{
    const size_t MAX_LEN = 100;
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);

    if (len1 >= MAX_LEN || len2 >= MAX_LEN) return(-1);

    //init the distance matrix
    int dist[MAX_LEN][MAX_LEN] = { 0 };
    for (int i = 0;i <= len1;i++) {
        dist[0][i] = i;
    }
    for (int j = 0;j <= len2; j++) {
        dist[j][0] = j;
    }
    // calculate
    for (int j = 1;j <= len1; j++) {
        for (int i = 1;i <= len2; i++) {
            int track = 1;
            if (s1[i - 1] == s2[j - 1]) {
                track = 0;
            }
            int t = MIN((dist[i - 1][j] + 1), (dist[i][j - 1] + 1));
            dist[i][j] = MIN(t, (dist[i - 1][j - 1] + track));
        }
    }
    return dist[len2][len1];
}

inline void calc_histogram(const char s1[], size_t hist1[255])
{
    memset(hist1, 0, 255);
    const size_t len1 = strlen(s1);
    for (size_t i = 0; i < len1; i++) {
        const char c = tolower(s1[i]);
        hist1[c]++;
    }
}

inline size_t calc_unique_chars(size_t hist1[255])
{
    size_t count = 0;
    for (size_t i = 0; i < 255; i++) {
        if (hist1[i] != 0) count++;
    }
    return count;
}

bool paramkit::util::has_similar_histogram(const char s1[], const char s2[])
{
    const size_t MAX_LEN = 255;
    size_t hist1[MAX_LEN] = { 0 };
    size_t hist2[MAX_LEN] = { 0 };

    calc_histogram(s1, hist1);
    calc_histogram(s2, hist2);

    size_t sim = 0;
    for (size_t i = 0; i < MAX_LEN; i++) {
        if (hist1[i] != 0 && hist2[i] != 0 ) sim++;
    }
    const size_t uniq1 = calc_unique_chars(hist1);
    const size_t uniq2 = calc_unique_chars(hist2);
    if (sim == uniq1 && sim == uniq2) {
        return true;
    }
    //
    return false;
}

paramkit::util::stringsim_type paramkit::util::has_keyword( std::string param, std::string filter)
{
    if (param.empty() || filter.empty()) {
        return SIM_NONE;
    }
    param = to_lowercase(param);
    filter = to_lowercase(filter);
    const bool sim_found = (param.find(filter) != std::string::npos) || (filter.find(param) != std::string::npos);
    if (sim_found) return SIM_SUBSTR;
    return SIM_NONE;
}

paramkit::util::stringsim_type paramkit::util::is_string_similar(const std::string &param, const std::string &filter)
{
    if (param.empty() || filter.empty()) {
        return SIM_NONE;
    }
    bool sim_found = false;
    if (has_keyword(param, filter) != SIM_NONE) {
        return SIM_SUBSTR;
    }
    size_t dist = util::levenshtein_distance(filter.c_str(), param.c_str());
    if (dist == 1 || dist <= (param.length() / 2)) {
        sim_found = true;
    }
    if (dist >= param.length() || dist >= filter.length()) {
        sim_found = false;
    }
    if (sim_found) return SIM_LAV_DIST;

    sim_found = util::has_similar_histogram(filter.c_str(), param.c_str());
    if (sim_found) return SIM_HIST;

    return SIM_NONE;
}

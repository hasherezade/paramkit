#include "strings_util.h"

#include <algorithm>
#include <cstring>

#define HIST_SIZE 256

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
            const unsigned char c1 = static_cast<unsigned char>(a[i]);
            const unsigned char c2 = static_cast<unsigned char>(b[i]);
            if (tolower(c1) != tolower(c2)) {
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
            const unsigned char c1 = static_cast<unsigned char>(a[i]);
            const unsigned char c2 = static_cast<unsigned char>(b[i]);
            if (tolower(c1) != tolower(c2)) return false;
        }
    }
    return true;
}

int paramkit::util::levenshtein_distance(const char s1[], const char s2[])
{
    // MAX_LEN caps stack usage at MAX_LEN*MAX_LEN*sizeof(int) = ~40 KB.
    // Strings at or beyond this length return -1 (caller must handle).
    const int MAX_LEN = 100;
    const int len1 = static_cast<int>(strlen(s1));
    const int len2 = static_cast<int>(strlen(s2));

    if (len1 >= MAX_LEN || len2 >= MAX_LEN) return -1;

    // dist[i][j] = edit distance between s1[0..i-1] and s2[0..j-1]
    int dist[MAX_LEN][MAX_LEN] = { 0 };
    for (int i = 0; i <= len1; i++) dist[i][0] = i; // deletions from s1
    for (int j = 0; j <= len2; j++) dist[0][j] = j; // insertions into s1

    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            const int track = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            const int t = MIN(dist[i - 1][j] + 1, dist[i][j - 1] + 1);
            dist[i][j] = MIN(t, dist[i - 1][j - 1] + track);
        }
    }
    return dist[len1][len2];
}

inline void calc_histogram(const char s1[], size_t hist1[HIST_SIZE])
{
    const size_t hist1_size = sizeof(hist1[0]) * HIST_SIZE;
    ::memset(hist1, 0, hist1_size);
    const size_t len1 = strlen(s1);
    for (size_t i = 0; i < len1; i++) {
        const unsigned char c = static_cast<unsigned char>(s1[i]);
        hist1[static_cast<unsigned char>(tolower(c))]++;
    }
}

inline size_t calc_unique_chars(const size_t hist1[HIST_SIZE])
{
    size_t count = 0;
    for (size_t i = 0; i < HIST_SIZE; i++) {
        if (hist1[i] != 0) count++;
    }
    return count;
}

bool paramkit::util::has_similar_histogram(const char s1[], const char s2[])
{
    size_t hist1[HIST_SIZE] = { 0 };
    size_t hist2[HIST_SIZE] = { 0 };

    calc_histogram(s1, hist1);
    calc_histogram(s2, hist2);

    size_t sim = 0;
    for (size_t i = 0; i < HIST_SIZE; i++) {
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
    if (has_keyword(param, filter) != SIM_NONE) {
        return SIM_SUBSTR;
    }
    const int dist = util::levenshtein_distance(filter.c_str(), param.c_str());
    if (dist >= 0) {
        // dist == -1 means strings exceeded MAX_LEN; skip Levenshtein similarity in that case
        bool sim_found = (dist == 1 || dist <= static_cast<int>(param.length() / 2));
        if (dist >= static_cast<int>(param.length()) || dist >= static_cast<int>(filter.length())) {
            sim_found = false;
        }
        if (sim_found) return SIM_LAV_DIST;
    }

    if (util::has_similar_histogram(filter.c_str(), param.c_str())) {
        return SIM_HIST;
    }
    return SIM_NONE;
}

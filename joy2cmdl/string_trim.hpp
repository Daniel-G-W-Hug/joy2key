#ifndef STRING_TRIM_HPP
#define STRING_TRIM_HPP

#include <string>

namespace hd
{

// trim from left
inline std::string &ltrim(std::string &s, const char *t = " \t\n\r\f\v")
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from right
inline std::string &rtrim(std::string &s, const char *t = " \t\n\r\f\v")
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from left & right
inline std::string &trim(std::string &s, const char *t = " \t\n\r\f\v")
{
    return ltrim(rtrim(s, t), t);
}

// copying versions

inline std::string ltrim_cp(std::string s, const char *t = " \t\n\r\f\v")
{
    return ltrim(s, t);
}

inline std::string rtrim_cp(std::string s, const char *t = " \t\n\r\f\v")
{
    return rtrim(s, t);
}

inline std::string trim_cp(std::string s, const char *t = " \t\n\r\f\v")
{
    return trim(s, t);
}

} // namespace hd

#endif // STRING_TRIM_HPP
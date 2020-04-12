#ifndef UTIL_STRINGUTIL_HPP
#define UTIL_STRINGUTIL_HPP

#include <string>

namespace sword
{

namespace util
{

inline std::string makeHeader(const std::string text, const char e = '=')
{
   std::string line;
   int count = 10;
   for (int i = 0; i < count; i++) 
       line += e;
   line += " "; 
   line += text;
   line += " ";
   for (int i = 0; i < count; i++) 
       line += e;
   return line;
}

}; // namespace util

}; // namespace sword

#endif /* end of include guard: UTIL_STRINGUTIL_HPP */

#ifndef UTIL_DEBUG_HPP
#define UTIL_DEBUG_HPP

#include <iostream>

#define SWD_DEBUG_MSG(msg) std::cerr << __FILE__ << ": " << __PRETTY_FUNCTION__ << ": " << msg << '\n';

#endif /* end of include guard: UTIL_DEBUG_HPP */

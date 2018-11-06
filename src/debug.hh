#pragma once

#include <string_view>

#define DEBUG_THIS

namespace krc {

// quick & dirty debugging facility
#ifdef DEBUG_THIS
void debug(std::string_view msg);
#else
inline void debug(std::string_view) {}
#endif

}

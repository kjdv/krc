#pragma once

#include <stdexcept>

namespace krc {

class channel_closed : public std::logic_error
{
public:
    using std::logic_error::logic_error;
};

} // namespace krc

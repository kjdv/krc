#pragma once

namespace krc {

struct no_copy
{
    no_copy()
    {}

    no_copy(const no_copy&) = delete;
    no_copy& operator=(const no_copy&) = delete;
};

} // namespace krc

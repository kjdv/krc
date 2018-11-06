#pragma once

namespace krc {
namespace internal {

struct no_copy
{
    no_copy() = default;

    no_copy(const no_copy&) = delete;
    no_copy& operator=(const no_copy&) = delete;
};

} // namespace internal
} // namespace krc

#pragma once

#include <functional>

namespace krc {
namespace internal {

struct defer
{
    std::function<void()> func;
    ~defer()
    {
        func();
    }
};

}
}

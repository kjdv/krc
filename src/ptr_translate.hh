#pragma once

#include <array>
#include <cstring>

namespace krc {

// helper for the deeply unhelpful signature of makecontext, enabling only int arguments to the target function
template <typename T = int> // as template to be able to test with different ptr/int sizes without switching architectures
struct ptr_as_ints
{
    typedef T int_type;
    typedef void* ptr_type;

    static_assert(sizeof(ptr_type) % sizeof(int_type) == 0);

    enum { N = sizeof(ptr_type) / sizeof(int_type) };
    std::array<int_type, N> ints;
};

template <typename T = int>
ptr_as_ints<T> from_ptr(typename ptr_as_ints<T>::ptr_type p)
{
    ptr_as_ints<T> result;
    mempcpy(result.ints.data(), p, ptr_as_ints<T>::N);
    return result;
}

template <typename T = int>
typename ptr_as_ints<T>::ptr_type to_ptr(const ptr_as_ints<T> &ints)
{
    typename ptr_as_ints<T>::ptr_type result;
    mempcpy(result, ints.ints.data(), ptr_as_ints<T>::N);
    return result;
}

}

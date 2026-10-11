#pragma once

#include <concepts>

#include "RozeTestsLibraryErrors.hpp"

class RozeTestLibrary {
    public:
    template <typename T>
        requires std::equality_comparable<T> && Printable<T>
    [[nodiscard]] static bool assertEq(const T& value, const T& expectedValue)
    {
        if (value != expectedValue) [[unlikely]] {
            errorAssertEq(value, expectedValue);
            return false;
        }
        return true;
    }
};

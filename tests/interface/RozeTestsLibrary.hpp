#pragma once

#include <concepts>
#include <iostream>

#include "RozeTestsLibraryErrors.hpp"

class RozeTestLibrary {
    public:
    template <std::equality_comparable T>
    [[nodiscard]] static bool assertEq(const T& value, const T& expectedValue)
    {
        if (value != expectedValue) [[unlikely]] {
            std::cerr << "assertEq failed\n";
            return false;
        }
        return true;
    }
};

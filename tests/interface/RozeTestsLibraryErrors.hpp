#pragma once

#include <iostream>

template <typename T>
concept Printable = requires(std::ostream& os, const T& v) { os << v; };

template <Printable T>
void errorAssertEq(const T& value, const T& expectedValue)
{
    std::cerr << "assertEq failed: expected " << expectedValue << ", got " << value << '\n';
}

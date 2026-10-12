module;

#include <iostream>

export module roze.tests.errors;

export template <typename T>
concept Printable = requires(std::ostream& os, const T& v) { os << v; };

export template <Printable T>
void errorAssertEq(const T& value, const T& expectedValue)
{
    std::cerr << "assertEq failed: expected " << expectedValue << ", got " << value << '\n';
}

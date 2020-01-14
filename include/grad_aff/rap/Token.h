#pragma once

#include <string>

template <typename T>
struct Token {
    std::string key;
    T value;
};
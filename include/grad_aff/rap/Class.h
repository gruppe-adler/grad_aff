#pragma once

#include <optional>
#include <string>
#include <map>

#include "Token.h"

struct Class {
    std::string className = "";
    std::string inheritedClass = "";
    std::vector<Token<int>> tokens;
};
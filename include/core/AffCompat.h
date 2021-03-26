#pragma once

#include <string.h>

#ifdef _MSC_VER
#define aff_strdup _strdup
#else
#define aff_strdup strdup
#endif // _MSC_VER



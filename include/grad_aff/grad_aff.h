// libaff.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <iostream>

#define GRAD_AFF_VERSION 0.1

#if defined(_WIN32) || defined(__CYGWIN__)
#    ifdef GRAD_AFF_STATIC_DEFINE
#        define GRAD_AFF_IMPORT
#        define GRAD_AFF_EXPORT
#    else
#        define GRAD_AFF_IMPORT __declspec(dllimport)
#        define GRAD_AFF_EXPORT __declspec(dllexport)
#    endif
#    define GRAD_AFF_LOCAL
#else
#    define GRAD_AFF_IMPORT __attribute__((visibility("default")))
#    define GRAD_AFF_EXPORT __attribute__((visibility("default")))
#    define GRAD_AFF_LOCAL __attribute__((visibility("hidden")))
#endif

#if defined(GRAD_AFF_EXPORTS)
#    define GRAD_AFF_API GRAD_AFF_EXPORT
#    define GRAD_AFF_EXTIMP
#else
#    define GRAD_AFF_API GRAD_AFF_IMPORT
#    define GRAD_AFF_EXTIMP extern
#endif

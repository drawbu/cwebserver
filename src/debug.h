/*
** EPITECH PROJECT, 2024
** my_teams
** File description:
** debug
*/

#pragma once

#include <stdio.h>

#define NOTHING /* Nothing */

#ifdef DEBUG_MODE
    #define FORMAT(s) "%s:%d: " s "\n"
    #define HEAD __FILE_NAME__, __LINE__
    #define DEBUG_DO(code) (code)
#else
    #define DEBUG_DO(code) NOTHING
#endif

#define DEBUG(fmt, ...)   DEBUG_DO(printf(FORMAT(fmt), HEAD, __VA_ARGS__))
#define DEBUGV(fmt, va)   DEBUG_DO(vprintf(FORMAT(fmt), HEAD, (va)))
#define DEBUG_MSG(string) DEBUG_DO(printf(FORMAT(string), HEAD))

// Avoid unused variable/functions warning
#ifdef DEBUG_MODE
    #define DEBUG_USED
#else
    #define DEBUG_USED UNUSED
#endif

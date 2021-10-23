#ifndef ARGUMENTS_H
#define ARGUMENTS_H


#include <cstddef>


struct Arguments
{
    char* progname; // path to program name
    long long  n;   // Argument "n"
    long long  k;   // Argument "k"
    bool s, m, h;   // flags: -s, -m, -h

    Arguments() noexcept;
};


bool parse_arguments(size_t argc, char** argv, Arguments& result,
    char* errBuffer, size_t bufferSize);


#endif

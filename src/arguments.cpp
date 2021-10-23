#include "arguments.h"

#include <string>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cerrno>


Arguments::Arguments() noexcept :
    progname(nullptr), n(-1), k(-1), s(false), m(false), h(false)
{
}


static bool _parse_single_dash_args(size_t argc, char** argv, Arguments& args,
    size_t& index, char* errBuffer, size_t bufferSize
);
static bool _parse_double_dash_args(size_t argc, char** argv, Arguments& args,
    size_t& index, char *errBuffer, size_t bufferSize
);


bool parse_arguments(size_t argc, char** argv, Arguments& result,
    char* errBuffer, size_t bufferSize)
{
    // Extract program name
    result.progname = argv[0];

    // skip first argument - program name
    for (size_t i = 1; i < argc; i++)
    {
        char* arg = argv[i];
        
        // if arguments start with dash and it's not a number
        // and arguments "k" and "n" while not specified
        if (arg[0] == '-' && !isdigit(arg[1]) &&
            result.n == -1 && result.k == -1)
        {
            if (arg[1] == '-')
            {
                if (!_parse_double_dash_args(argc, argv, result, i,
                    errBuffer, bufferSize))
                {
                    return false;
                }
            }
            else
            {
                if (!_parse_single_dash_args(argc, argv, result, i,
                    errBuffer, bufferSize))
                {
                    return false;
                }
            }
        }
        // if one of the numbers is not set
        else if (result.n == -1 || result.k == -1)
        {
            long long number = 0;
            char* endptr = arg;

            errno = 0;
            number = std::strtoll(arg, &endptr, 0);

            if (errno == ERANGE)
            {
                snprintf(errBuffer, bufferSize, 
                    "Error: the input number \"%s\" is out of the allowed "
                    "range.\n", arg);
                return false;
            }
            else if ((errno == EINVAL && number == 0) || (*endptr != '\0'))
            {
                snprintf(errBuffer, bufferSize,
                    "Error: failed to convertion argument \"%s\" "
                    "to integer number.\n", arg);
                return false;
            }

            if (number < 0)
            {
                snprintf(errBuffer, bufferSize,
                    "Error: the input number \"%s\" must not be "
                    "negative.\n", arg);
                return false;
            }
            
            // if "n" is not set
            if (result.n == -1)
                result.n = number;
            // if "n" is set, but not "k"
            else
                result.k = number;
        }
        else
        {
            snprintf(errBuffer, bufferSize,
                "Error: unknown argument \"%s\".\n", arg);
            return false;
        }
    }

    // If both arguments are not specified, print manual
    if (result.n == -1 && result.k == -1)
        result.h = true;
    // If argument "n" is specified, but not argument "k"
    else if (result.n != -1 && result.k == -1)
    {
        snprintf(errBuffer, bufferSize, 
            "Error: argument \"k\" is not specified.\n");
        return false;
    }
    // If all arguments "n" and "k" are specified
    else
    {
        // If flags "-s" and "-m" are not set, by default use only "-m" flag
        if (!result.s && !result.m)
            result.m = true;
    }

    return true;
}

bool _parse_single_dash_args(size_t argc, char** argv, Arguments& args, 
    size_t& index, char* errBuffer, size_t bufferSize)
{
    std::string arg = argv[index] + 1;  // skip starting dash

    for (auto& chr : arg)
    {
        char flag = chr;

        switch (flag)
        {
        case 's':
        {
            args.s = true;
            break;
        }
        case 'm':
        {
            args.m = true;
            break;
        }
        case 'h':
        {
            args.h = true;
            break;
        }
        default:
        {
            snprintf(errBuffer, bufferSize,
                "Error: unknown parameter \"-%c\".\n", flag);
            return false;
        }
        }
    }

    return true;
}

bool _parse_double_dash_args(size_t argc, char** argv, Arguments& args,
    size_t& index, char* errBuffer, size_t bufferSize)
{
    std::string arg = argv[index] + 2;  // skip starting dashes

    // if argument is "help"
    if (arg == "help")
        args.h = true;
    else
    {
        snprintf(errBuffer, bufferSize,
            "Error: unknown parameter \"%s\".\n", arg.c_str());
        return false;
    }

    return true;
}

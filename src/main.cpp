#include <iostream>
#include <cstdio>
#include <fstream>
#include <string>

#include <mpi.h>

#include "arguments.h"
#include "calculation.h"


constexpr auto MANUAL_FILENAME = "manual.txt";


void print_manual(const char* progname);


int main(int argc, char** argv) 
{
    constexpr size_t bufferSize = 1024;

    Arguments args;
    char errBuffer[bufferSize];
    int comm_size = 0;
    int comm_rank = 0;

    MPI_Init(&argc, &argv);
    
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank); 

    if (!parse_arguments(static_cast<size_t>(argc), argv, args,
        errBuffer, bufferSize))
    {
        if (comm_rank == 0)
            std::cout << errBuffer;

        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
        return -1;
    }

    if (args.h)
    {
        if (comm_rank == 0)
            print_manual(args.progname);
    }
    else
    {
        double timework = 0;
        unsigned long long result   = 0;

        // Perform sequential function and measure their working time
        if (comm_rank == 0 && args.s)
        {
            timework = MPI_Wtime();
            result = partial(args.n, args.k);
            timework = MPI_Wtime() - timework;

            printf("The result of a sequential function: %llu.\n", result);
            printf("Time work: %.3f seconds.\n", timework);
        }

        if (args.m)
        {
            if (comm_size < 2)
            {
                if (comm_rank == 0)
                {
                    printf("Error: to run a parallel version of the program,"
                        "at least 2 processes are required.\n");
                }

                MPI_Barrier(MPI_COMM_WORLD);
                MPI_Abort(MPI_COMM_WORLD, MPI_ERR_OTHER);
                return -1;
            }

            if (comm_rank == 0)
            {
                printf("Up to %d processes are used for parallelization.\n",
                    comm_size);
            }

            // Perform parallelized function and measure their working time
            MPI_Barrier(MPI_COMM_WORLD);
            timework = MPI_Wtime();
            result = mpi_partial(args.n, args.k);
            timework = MPI_Wtime() - timework;

            if (comm_rank == 0)
            {
                printf("The result of the parallelized function: %llu.\n",
                    result);
                printf("Time work: %.3f seconds.\n", timework);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Finalize();
    return 0;
}

void print_manual(const char* _progname)
{
#if defined(_WIN32)
    char path_delim = '\\';
#elif defined(__unix__)
    char path_delim = '/';
#endif

    // characters number of path without manual filename
    std::string progname(_progname);
    size_t n = progname.find_last_of(path_delim);
    std::string man_filename = 
        std::string(progname.begin(), progname.begin() + n + 1) +
        MANUAL_FILENAME;

    // open manual file
    std::ifstream manual_file(man_filename);
    
    // reads every line in the file and prints it to standard output
    while (manual_file.good())
    {
        std::string buffer;

        std::getline(manual_file, buffer);
        buffer += '\n';
        std::cout << buffer;
    }
    
    if (manual_file.bad())
    {
        printf("Error: failed to read manual from file \"%s\".\n",
            man_filename.c_str());
    }
    else if (!manual_file.is_open())
        printf("Failed to open manual file \"%s\".\n", man_filename.c_str());
}

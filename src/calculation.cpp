#include "calculation.h"

#include <cmath>
#include <stack>
#include <utility>

#include <mpi.h>


using pair = std::pair<unsigned long long, unsigned long long>;


unsigned long long partial(long long n, long long k)
{
    std::stack<pair> stack;
    unsigned long long result = 0;

    stack.push(pair(n, k));
    while (!stack.empty())
    {
        pair temp = stack.top();
        stack.pop();
        n = temp.first;
        k = temp.second;

        if (n == k && k > 0)
            result += 1;
        else if (n > 0 && k == 1)
            result += 1;
        else if (n > k && k > 0)
        {
            stack.push(pair(n - 1, k - 1));
            stack.push(pair(n - k, k));
        }
    }

    return result;
}

unsigned long long mpi_partial(long long n, long long k)
{
    using signal_t = char;

    // Signal values
    constexpr signal_t CONTINUE = 0;
    constexpr signal_t DONE     = 1;

    // Data type tags
    constexpr int tagMpiPair    = 0;
    constexpr int tagMpiSignal  = 1;
    constexpr int tagMpiUll     = 2;

    // Local data
    std::stack<pair> stack;
    unsigned long long result = 0;
    signal_t signal = CONTINUE;
    int comm_size = 0;
    int comm_rank = 0;

    // Get size of communicator - count of processes,
    // and rank - number of current process
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);

    // Recipient process index. Using only in process 0
    size_t next_process = (comm_rank + 1) % comm_size;

    // Create pair data type for mpi
    MPI_Datatype mpi_pair;
    const int blockSize[2] = {1, 1};
    const MPI_Aint offsets[2] = 
        {offsetof(pair, first), offsetof(pair, second)};
    const MPI_Datatype types[2] = 
        {MPI_UNSIGNED_LONG_LONG, MPI_UNSIGNED_LONG_LONG};
    MPI_Type_create_struct(2, blockSize, offsets, types, &mpi_pair);
    MPI_Type_commit(&mpi_pair);

    // Process 0 continues to work
    if (comm_rank == 0)
        stack.push(pair(n, k));
    // Other process are waiting for messages from process 0
    else
    {
        MPI_Status status;

        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        // If the message type is a pair
        if (status.MPI_TAG == tagMpiPair)
        {
            pair temp;

            MPI_Recv(&temp, 1, mpi_pair, 0, tagMpiPair, MPI_COMM_WORLD,
                &status);
            stack.push(temp);
        }
        // Otherwise, the process received a message from process 0
        // about completion. And current process skip main loop.
        else if (status.MPI_TAG == tagMpiSignal)
        {
            MPI_Recv(&signal, 1, MPI_CHAR, 0, tagMpiSignal, MPI_COMM_WORLD,
                &status);
        }

    }

    // Main loop
    while (!stack.empty() && signal != DONE)
    {
        pair temp;

        // If the stack size is greater than 1 in process 0, then all elements 
        // except one are sent to other processes.
        while (comm_rank == 0 && stack.size() > 1)
        {
            temp = stack.top();
            stack.pop();

            MPI_Send(&temp, 1, mpi_pair, next_process, tagMpiPair,
                MPI_COMM_WORLD);
            
            next_process = (next_process + 1) % comm_size;
            if (next_process == 0)
                next_process = (next_process + 1) % comm_size;
        }
        temp = stack.top();
        stack.pop();
        n = temp.first;
        k = temp.second;

        if (n == k && k > 0)
            result += 1;
        else if (n > 0 && k == 1)
            result += 1;
        else if (n > k && k > 0)
        {
            stack.push(pair(n - 1, k - 1));
            stack.push(pair(n - k, k));
        }

        // If the process 1 or 2 ... or comm_size-1 has completed its part of 
        // the work
        if (comm_rank != 0 && stack.empty())
        {
            MPI_Status status;
            
            // Waiting for messages from process 0
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                
            // If the message type is a pair
            if (status.MPI_TAG == tagMpiPair)
            {
                MPI_Recv(&temp, 1, mpi_pair, status.MPI_SOURCE,
                    tagMpiPair, MPI_COMM_WORLD, &status);
                stack.push(temp);
            }
            // Otherwise, the process received a message from process 0
            // about completion
            else if (status.MPI_TAG == tagMpiSignal)
            {
                MPI_Recv(&signal, 1, MPI_CHAR, 0, tagMpiSignal,
                    MPI_COMM_WORLD, &status);
            }
        }
    }

    // Waking up blocked processes
    if (comm_rank == 0)
    {
        signal = DONE;
        for (int i = 1; i < comm_size; i++)
            MPI_Send(&signal, 1, MPI_CHAR, i, tagMpiSignal, MPI_COMM_WORLD);
    }

    // Sum up all the local results to the result of the process 0
    if (comm_rank == 0)
    {
        for (int i = 1; i < comm_size; i++)
        {
            MPI_Status status;
            unsigned long long interm_result;
            
            MPI_Recv(&interm_result, 1, MPI_UNSIGNED_LONG_LONG, i, tagMpiUll,
                MPI_COMM_WORLD, &status);
            
            result += interm_result;
        }
    }
    else
    {
        MPI_Send(&result, 1, MPI_UNSIGNED_LONG_LONG, 0, tagMpiUll,
            MPI_COMM_WORLD);
    }

    MPI_Type_free(&mpi_pair);
    return result;
}

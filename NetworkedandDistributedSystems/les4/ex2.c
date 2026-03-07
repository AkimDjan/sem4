#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    REQUEST_TAG = 1,
    RESPONSE_TAG = 2
};

static int random_in_range(int min_value, int max_value) {
    return min_value + rand() % (max_value - min_value + 1);
}

static void check_mpi(int error_code, const char *operation) {
    if (error_code != MPI_SUCCESS) {
        MPI_Abort(MPI_COMM_WORLD, error_code);
    }
}

int main(int argc, char **argv) {
    int rank;
    int size;
    int utc_time;

    check_mpi(MPI_Init(&argc, &argv), "MPI_Init");
    check_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank), "MPI_Comm_rank");
    check_mpi(MPI_Comm_size(MPI_COMM_WORLD, &size), "MPI_Comm_size");

    if (rank != 0) {
        fprintf(stderr, "Serv must be started only for rank 0.\n");
        check_mpi(MPI_Finalize(), "MPI_Finalize");
        return 1;
    }

    if (size < 2) {
        fprintf(stderr, "At least one client process is required.\n");
        check_mpi(MPI_Finalize(), "MPI_Finalize");
        return 1;
    }

    srand(rank);
    utc_time = random_in_range(0, 20);
    printf("Server: initial UTC = %d\n", utc_time);

    for (int handled_clients = 0; handled_clients < size - 1; ++handled_clients) {
        int request = 0;
        int response[2];
        int processing_delay;
        int receive_time;
        int send_time;
        MPI_Status status;

        check_mpi(
            MPI_Recv(&request, 1, MPI_INT, MPI_ANY_SOURCE, REQUEST_TAG, MPI_COMM_WORLD, &status),
            "MPI_Recv"
        );

        utc_time += random_in_range(0, 20);
        receive_time = utc_time;


        processing_delay = random_in_range(0, 3);
        utc_time += processing_delay;
        send_time = utc_time;

        response[0] = send_time;
        response[1] = processing_delay;

        printf("Server: received request from client %d at UTC = %d\n",
               status.MPI_SOURCE, receive_time);
        printf("Server: sent response to client %d at UTC = %d, d = %d\n",
               status.MPI_SOURCE, send_time, processing_delay);

        check_mpi(
            MPI_Send(response, 2, MPI_INT, status.MPI_SOURCE, RESPONSE_TAG, MPI_COMM_WORLD),
            "MPI_Send"
        );
    }

    check_mpi(MPI_Finalize(), "MPI_Finalize");
    return 0;
}

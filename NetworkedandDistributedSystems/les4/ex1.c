#include <math.h>
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
    int local_time;
    int t0;
    int t1;
    int request = 1;
    int response[2];
    int tutc;
    int d;
    int correction;
    int corrected_time;

    check_mpi(MPI_Init(&argc, &argv), "MPI_Init");
    check_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank), "MPI_Comm_rank");
    check_mpi(MPI_Comm_size(MPI_COMM_WORLD, &size), "MPI_Comm_size");

    if (rank == 0) {
        fprintf(stderr, "CLI must be started only for client ranks.\n");
        check_mpi(MPI_Finalize(), "MPI_Finalize");
        return 1;
    }

    if (size < 2) {
        fprintf(stderr, "At least one server and one client are required.\n");
        check_mpi(MPI_Finalize(), "MPI_Finalize");
        return 1;
    }

    srand(rank);
    local_time = random_in_range(0, 20);
    t0 = local_time;

    check_mpi(
        MPI_Send(&request, 1, MPI_INT, 0, REQUEST_TAG, MPI_COMM_WORLD),
        "MPI_Send"
    );
    check_mpi(
        MPI_Recv(response, 2, MPI_INT, 0, RESPONSE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE),
        "MPI_Recv"
    );

    tutc = response[0];
    d = response[1];

    local_time += random_in_range(5, 10);
    t1 = local_time;

    correction = (int) ceil((double) (t1 - t0 - d) / 2.0);
    corrected_time = tutc + correction;

    printf("Client %d: T0 = %d, T1 = %d, received {TUTC = %d, d = %d}, corrected local time = %d\n",
           rank, t0, t1, tutc, d, corrected_time);

    check_mpi(MPI_Finalize(), "MPI_Finalize");
    return 0;
}

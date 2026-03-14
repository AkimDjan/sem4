#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    SYNC_REQUEST_TAG = 1,
    TIME_DIFF_TAG = 2,
    CORRECTION_TAG = 3
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
    int server_time;
    int time_diff;
    int correction;

    check_mpi(MPI_Init(&argc, &argv), "MPI_Init");
    check_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank), "MPI_Comm_rank");
    check_mpi(MPI_Comm_size(MPI_COMM_WORLD, &size), "MPI_Comm_size");

    if (rank == 0) {
        fprintf(stderr, "cli must be started only for client ranks.\n");
        check_mpi(MPI_Finalize(), "MPI_Finalize");
        return 1;
    }

    if (size < 2) {
        fprintf(stderr, "At least one server and one client are required.\n");
        check_mpi(MPI_Finalize(), "MPI_Finalize");
        return 1;
    }

    srand((unsigned int) rank);
    local_time = random_in_range(0, 50);
    printf("Process %d: initial local time = %d\n", rank, local_time);

    check_mpi(
        MPI_Recv(&server_time, 1, MPI_INT, 0, SYNC_REQUEST_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE),
        "MPI_Recv(SYNC_REQUEST)"
    );
    printf("Process %d at time %d received SYNC_REQUEST(server_time=%d) from process 0\n",
           rank, local_time, server_time);

    time_diff = local_time - server_time;
    printf("Process %d at time %d sent TIME_DIFF=%d to process 0\n",
           rank, local_time, time_diff);
    check_mpi(
        MPI_Send(&time_diff, 1, MPI_INT, 0, TIME_DIFF_TAG, MPI_COMM_WORLD),
        "MPI_Send(TIME_DIFF)"
    );

    check_mpi(
        MPI_Recv(&correction, 1, MPI_INT, 0, CORRECTION_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE),
        "MPI_Recv(CORRECTION)"
    );
    printf("Process %d at time %d received CORRECTION=%d from process 0\n",
           rank, local_time, correction);

    local_time += correction;
    printf("Process %d: new agreed time = %d\n", rank, local_time);

    check_mpi(MPI_Finalize(), "MPI_Finalize");
    return 0;
}

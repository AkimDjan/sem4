#include <math.h>
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
    int request_payload;
    int sum_offsets;
    int average_offset;
    int *client_offsets;
    int client_rank;
    int responses_received;

    check_mpi(MPI_Init(&argc, &argv), "MPI_Init");
    check_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank), "MPI_Comm_rank");
    check_mpi(MPI_Comm_size(MPI_COMM_WORLD, &size), "MPI_Comm_size");

    if (rank != 0) {
        fprintf(stderr, "serv must be started only for rank 0.\n");
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
    printf("Process 0: initial local time = %d\n", local_time);

    client_offsets = (int *) calloc((size_t) size, sizeof(int));
    if (client_offsets == NULL) {
        fprintf(stderr, "Process 0: memory allocation failed.\n");
        check_mpi(MPI_Abort(MPI_COMM_WORLD, 1), "MPI_Abort");
    }
    client_offsets[0] = 0;

    request_payload = local_time;
    for (client_rank = 1; client_rank < size; ++client_rank) {
        printf("Process 0 at time %d sent SYNC_REQUEST to process %d\n",
               local_time, client_rank);
        check_mpi(
            MPI_Send(&request_payload, 1, MPI_INT, client_rank, SYNC_REQUEST_TAG, MPI_COMM_WORLD),
            "MPI_Send(SYNC_REQUEST)"
        );
    }

    sum_offsets = 0;
    for (responses_received = 0; responses_received < size - 1; ++responses_received) {
        int client_offset;
        MPI_Status status;

        check_mpi(
            MPI_Recv(&client_offset, 1, MPI_INT, MPI_ANY_SOURCE, TIME_DIFF_TAG, MPI_COMM_WORLD, &status),
            "MPI_Recv(TIME_DIFF)"
        );

        printf("Process 0 at time %d received TIME_DIFF=%d from process %d\n",
               local_time, client_offset, status.MPI_SOURCE);
        client_offsets[status.MPI_SOURCE] = client_offset;
        sum_offsets += client_offset;
    }

    average_offset = (int) ceil((double) sum_offsets / (double) size);

    for (client_rank = 1; client_rank < size; ++client_rank) {
        int correction = average_offset - client_offsets[client_rank];
        printf("Process 0 at time %d sent CORRECTION=%d to process %d\n",
               local_time, correction, client_rank);
        check_mpi(
            MPI_Send(&correction, 1, MPI_INT, client_rank, CORRECTION_TAG, MPI_COMM_WORLD),
            "MPI_Send(CORRECTION)"
        );
    }

    local_time += average_offset;
    printf("Process 0: new agreed time = %d\n", local_time);
    free(client_offsets);

    check_mpi(MPI_Finalize(), "MPI_Finalize");
    return 0;
}

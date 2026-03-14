#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

void Check_For_SUCCESS(int MPI_FUNC) {
    if (MPI_FUNC != MPI_SUCCESS) {MPI_Abort(MPI_COMM_WORLD, -1);}
}

int random_num(int a, int b) {
    return a + rand() % (b - a + 1);
}

int main(int argc, char** argv) {
    Check_For_SUCCESS(MPI_Init(&argc, &argv));
    int size, rank, i, local_time, req, sum, average_offset, *clients, cli_rk, resp_recv;
    Check_For_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &size));
    Check_For_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (rank!=0) {
        printf("Server's rank must be equal 0\n");
        Check_For_SUCCESS(MPI_Finalize());
        return 1;
    }
    srandom(rank);
    local_time = random_num(0,50); //часы демона времени
    printf("Server set initial local time at %d\n", local_time);
    Check_For_SUCCESS(MPI_Send(&local_time, 1, MPI_INT, 0, 1, MPI_COMM_WORLD));
    clients = (int *) calloc((size_t) size, sizeof(int));
    req = local_time;
    for (cli_rk = 1; cli_rk < size; ++cli_rk) {
        printf("Server at time %d sent local time to process %d\n", local_time, cli_rk);
        Check_For_SUCCESS(MPI_Send(&req, 1, MPI_INT, cli_rk, 1, MPI_COMM_WORLD));
    }
    sum = 0;
    for (resp_recv = 0; resp_recv < size - 1; ++resp_recv) {
        int client_offset;
        MPI_Status status;
        Check_For_SUCCESS(MPI_Recv(&client_offset, 1, MPI_INT, MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &status));
        printf("Server at time %d received time difference %d from process %d\n",local_time, client_offset, status.MPI_SOURCE);
        clients[status.MPI_SOURCE] = client_offset;
        sum += client_offset;
    }
    average_offset = (int) ceil((double) sum / (double) size);
    for (cli_rk = 1; cli_rk < size; ++cli_rk) {
        int correction = average_offset - clients[cli_rk];
        printf("Server at time %d sent correction %d to process %d\n",local_time, correction, cli_rk);
        Check_For_SUCCESS(MPI_Send(&correction, 1, MPI_INT, cli_rk, 3, MPI_COMM_WORLD));
    }
    local_time += average_offset;
    printf("Server set new local time %d\n", local_time);
    free(clients);
    Check_For_SUCCESS(MPI_Finalize());
    return 0;
}
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
    int size, rank, T_UTC, i;
    Check_For_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &size));
    Check_For_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (rank!=0) {
        printf("Server's rank must be equal 0");
        Check_For_SUCCESS(MPI_Finalize());
        return 1;
    }
    srandom(rank);
    T_UTC = random_num(0,20);
    for (i=0; i<size-1; ++i) {
        int req=0, resp[2], d, rec_time, send_time;
        MPI_Status status;
        Check_For_SUCCESS(MPI_Recv(&req, 1, MPI_INT, MPI_ANY_SOURCE, 1, MPI_COMM_WORLD, &status));
        rec_time = T_UTC + random_num(0,20);
        d = random_num(0,3);
        send_time = rec_time + d;
        resp[0] = send_time;
        resp[1] = d;
        Check_For_SUCCESS(MPI_Send(resp, 2, MPI_INT, status.MPI_SOURCE, 2, MPI_COMM_WORLD));
    }  
    Check_For_SUCCESS(MPI_Finalize());
    return 0;
}
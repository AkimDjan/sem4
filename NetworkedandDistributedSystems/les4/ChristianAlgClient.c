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
    int size, rank, T_0, req = 1, resp[2], T_UTC, d, T_1, corr_T_1, correction;
    Check_For_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &size));
    Check_For_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (rank==0) {
        printf("Client's rank must be not equal to zero");
        Check_For_SUCCESS(MPI_Finalize());
        return 1;
    }
    srandom(rank);
    T_0 = random_num(0,20); //локальные часы
    Check_For_SUCCESS(MPI_Send(&req, 1, MPI_INT, 0, 1, MPI_COMM_WORLD)); // пусть 1 - код отправления
    Check_For_SUCCESS(MPI_Recv(resp, 2, MPI_INT, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE)); // 2 код принятия
    T_UTC = resp[0];
    d = resp[1];
    T_1 = T_0 + random_num(5,10);
    correction = (int) ceil((double) (T_1 - T_0 - d) / 2.0);
    corr_T_1 = T_UTC + correction;
    printf("Client number%d: T_0 = %d, T_1 = %d, received T_UTC=%d and d=%d, corrected local time = %d\n",rank, T_0, T_1, T_UTC, d, corr_T_1);
    Check_For_SUCCESS(MPI_Finalize());
    return 0;
}
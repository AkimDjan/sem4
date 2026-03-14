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
    int size, rank, local_time, serv_time, time_dif, correction;
    Check_For_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &size));
    Check_For_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (rank==0) {
        printf("Client's rank must be not equal to zero\n");
        Check_For_SUCCESS(MPI_Finalize());
        return 1;
    }
    srandom((unsigned int) rank);
    local_time = random_num(0,50); //локальные часы
    printf("Process number%d set local time at %d\n", rank, local_time);
    Check_For_SUCCESS(MPI_Recv(&serv_time, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE));
    printf("Process number%d at local time %d received server time %d from server\n",rank, local_time, serv_time);
    time_dif = local_time-serv_time;
    printf("Process number%d at local time %d sent time difference %d to server\n",rank, local_time, time_dif);
    Check_For_SUCCESS(MPI_Send(&time_dif, 1, MPI_INT, 0, 2, MPI_COMM_WORLD));
    Check_For_SUCCESS(MPI_Recv(&correction, 1, MPI_INT, 0, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE));
    printf("Process number%d at local time %d received correction time %d from server\n", rank, local_time, correction);
    local_time += correction;
    printf("Process number%d set new corrected time %d\n", rank, local_time);
    Check_For_SUCCESS(MPI_Finalize());
    return 0;
}
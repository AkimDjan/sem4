#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void Check_For_SUCCESS(int MPI_FUNC) {
    if (MPI_FUNC != MPI_SUCCESS) {MPI_Abort(MPI_COMM_WORLD, -1);}
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

int main(int argc, char** argv) {
    int rank, size;
    Check_For_SUCCESS(MPI_Init(&argc, &argv));
    Check_For_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &size));
    Check_For_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    int process_num = rank + 1;
    char filename[20];
    sprintf(filename, "%02d.dat", process_num);
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Process number %d: no events\n", process_num);
        MPI_Finalize();
        return 0;
    }
    int lamport_time = 0, k, d = size-rank;
    while (fscanf(file, "%d", &k) != EOF) {
        lamport_time += d;
        if (k == 0) {   
            printf("Process number %d: internal event, Lamport time = %d\n", process_num, lamport_time);
        }
        else if (k > 0) {
            int target_rank = k - 1;
            printf("Process number %d: sending message to the process %d, Lamport time %d\n", process_num, k, lamport_time);
            Check_For_SUCCESS(MPI_Send(&lamport_time, 1, MPI_INT, target_rank, 0, MPI_COMM_WORLD));
        }
        else if (k < 0) {
            int source_process = -k;
            int source_rank = source_process - 1;
            int recv_clock; 
            Check_For_SUCCESS(MPI_Recv(&recv_clock, 1, MPI_INT, source_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE));
            lamport_time = max(lamport_time, recv_clock + d);
            printf("Process number %d: received message %d, corrected Lamport time = %d\n", process_num, recv_clock, lamport_time);
        }
    }
    fclose(file);
    Check_For_SUCCESS(MPI_Finalize());
    return 0;
}
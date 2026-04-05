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
    sprintf(filename, "v%02d.dat", process_num);
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Process number %d: no events\n", process_num);
        MPI_Finalize();
        return 0;
    }
    int* vec_time = calloc(size, sizeof(int));
    int k, d = rank + 1, i;
    while (fscanf(file, "%d", &k) != EOF) {
        vec_time[rank] += d;
        if (k == 0) {   
            printf("Process number %d: internal event, vector time = [ ", process_num);
            for (i = 0; i < size; i++) {printf("%d ", vec_time[i]);}
            printf("]\n");
        }
        else if (k > 0) {
            int target_rank = k - 1;
            printf("Process number %d: sending message to the process %d, vector time =[ ", process_num, k);
            for (i = 0; i < size; i++) {printf("%d ", vec_time[i]);}
            printf("]\n");
            Check_For_SUCCESS(MPI_Send(vec_time, size, MPI_INT, target_rank, 0, MPI_COMM_WORLD));
        }
        else if (k < 0) {
            int source_process = -k;
            int source_rank = source_process - 1;
            int* recv_vec_time = malloc(size * sizeof(int));
            Check_For_SUCCESS(MPI_Recv(recv_vec_time, size, MPI_INT, source_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE));
            for (i = 0; i < size; i++) {if (i != rank) : vec_time[i] = max(vec_time[i], recv_vec_time[i]);}
            printf("Process number %d: received message, corrected vector time = [ ", process_num);
            for (i = 0; i < size; i++) {printf("%d ", vec_time[i]);}
            printf("]\n");
            free(recv_vec_time);
        }
    }

    free(vec_time);
    fclose(file);
    Check_For_SUCCESS(MPI_Finalize());
    return 0;
}
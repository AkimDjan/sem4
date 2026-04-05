#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define M(i,j) mat_time[(i)*size + (j)]
#define recv_M(i,j) recv_mat_time[(i)*size + (j)]

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
    int* mat_time = calloc(size * size, sizeof(int));
    int k, d = rank + 1, i, j;
    while (fscanf(file, "%d", &k) != EOF) {
        M(rank, rank) += d;
        if (k == 0) {   
            printf("Process number %d: internal event, matrix time = \n", process_num);
            for (i = 0; i < size; i++) {
                printf("[ ");
                for (j = 0; j < size; j++) {
                    printf("%d ", M(i,j));
                }
                printf("]\n");
            }
            printf("]\n");
        }
        else if (k > 0) {
            int target_rank = k - 1;
            printf("Process number %d: sending message to the process %d, matrix time = \n[", process_num, k);
            for (i = 0; i < size; i++) {
                printf("[ ");
                for (j = 0; j < size; j++) {
                    printf("%d ", M(i,j));
                }
                printf("]\n");
            }
            printf("]\n");
            Check_For_SUCCESS(MPI_Send(mat_time, size*size, MPI_INT, target_rank, 0, MPI_COMM_WORLD));
        }
        else if (k < 0) {
            int source_process = -k;
            int source_rank = source_process - 1;
            int* recv_mat_time = calloc(size*size, sizeof(int));
            Check_For_SUCCESS(MPI_Recv(recv_mat_time, size*size, MPI_INT, source_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE));
            for (i = 0; i < size; i++) {
                if (i == rank) continue;
                M(rank, i) = max(M(rank, i), recv_M(source_rank, i));
            }
            for (i = 0; i < size; i++) {
                if (i == rank) continue;
                for (j = 0; j < size; j++) {
                    M(i,j) = max(M(i,j), recv_M(i,j));
                }
            }
            printf("Process number %d: received message, corrected matrix time = \n[", process_num);
            for (i = 0; i < size; i++) {
                printf("[ ");
                for (j = 0; j < size; j++) {
                    printf("%d ", M(i,j));
                }
                printf("]\n");
            }
            printf("]\n");
            free(recv_mat_time);
        }
    }

    free(mat_time);
    fclose(file);
    Check_For_SUCCESS(MPI_Finalize());
    return 0;
}
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

enum {
    REQUEST_TAG = 1,
    GRANT_TAG = 2,
    RELEASE_TAG = 3
};

static void check_mpi(int code, const char *operation) {
    (void) operation;
    if (code != MPI_SUCCESS) {
        MPI_Abort(MPI_COMM_WORLD, code);
    }
}

static int random_section_time(void) {
    return (int) (random() % 6) + 3;
}

static void send_message(int rank, int dst_rank, int tag, int iteration) {
    check_mpi(
        MPI_Send(&iteration, 1, MPI_INT, dst_rank, tag, MPI_COMM_WORLD),
        "MPI_Send"
    );

    if (tag == REQUEST_TAG) {
        printf("Процесс %d: послал сообщение request процессу %d для итерации %d\n", rank, dst_rank, iteration);
    } else if (tag == GRANT_TAG) {
        printf("Процесс %d: послал сообщение grant процессу %d для итерации %d\n", rank, dst_rank, iteration);
    } else if (tag == RELEASE_TAG) {
        printf("Процесс %d: послал сообщение release процессу %d для итерации %d\n", rank, dst_rank, iteration);
    }
    fflush(stdout);
}

static int receive_message(int rank, int src_rank, int tag) {
    int iteration;
    MPI_Status status;

    check_mpi(
        MPI_Recv(&iteration, 1, MPI_INT, src_rank, tag, MPI_COMM_WORLD, &status),
        "MPI_Recv"
    );

    if (tag == REQUEST_TAG) {
        printf("Процесс %d: получил сообщение request от процесса %d для итерации %d\n", rank, status.MPI_SOURCE, iteration);
    } else if (tag == GRANT_TAG) {
        printf("Процесс %d: получил сообщение grant от процесса %d для итерации %d\n", rank, status.MPI_SOURCE, iteration);
    } else if (tag == RELEASE_TAG) {
        printf("Процесс %d: получил сообщение release от процесса %d для итерации %d\n", rank, status.MPI_SOURCE, iteration);
    }
    fflush(stdout);

    return iteration;
}

static void run_coordinator(int rank, int size, int repeat) {
    int release_count;
    int critical_busy;
    int queue_head;
    int queue_tail;
    int queue_capacity;
    int *queue_ranks;
    int *queue_iterations;

    release_count = 0;
    critical_busy = 0;
    queue_head = 0;
    queue_tail = 0;
    queue_capacity = (size - 1) * repeat;
    queue_ranks = (int *) malloc((size_t) queue_capacity * sizeof(int));
    queue_iterations = (int *) malloc((size_t) queue_capacity * sizeof(int));

    if (queue_ranks == NULL || queue_iterations == NULL) {
        fprintf(stderr, "Процесс %d: не удалось выделить память для очереди\n", rank);
        fflush(stderr);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    printf("Процесс %d: координатор запущен\n", rank);
    fflush(stdout);

    while (release_count < (size - 1) * repeat) {
        MPI_Status status;

        check_mpi(
            MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status),
            "MPI_Probe"
        );

        if (status.MPI_TAG == REQUEST_TAG) {
            int iteration;

            iteration = receive_message(rank, status.MPI_SOURCE, REQUEST_TAG);
            if (!critical_busy) {
                critical_busy = 1;
                send_message(rank, status.MPI_SOURCE, GRANT_TAG, iteration);
            } else {
                queue_ranks[queue_tail] = status.MPI_SOURCE;
                queue_iterations[queue_tail] = iteration;
                queue_tail = (queue_tail + 1) % queue_capacity;
                printf("Процесс %d: поставил процесс %d в очередь ожидания для итерации %d\n", rank, status.MPI_SOURCE, iteration);
                fflush(stdout);
            }
        } else if (status.MPI_TAG == RELEASE_TAG) {
            (void) receive_message(rank, status.MPI_SOURCE, RELEASE_TAG);
            ++release_count;

            if (queue_head != queue_tail) {
                int next_rank;
                int next_iteration;

                next_rank = queue_ranks[queue_head];
                next_iteration = queue_iterations[queue_head];
                queue_head = (queue_head + 1) % queue_capacity;
                send_message(rank, next_rank, GRANT_TAG, next_iteration);
            } else {
                critical_busy = 0;
            }
        }
    }

    printf("Процесс %d: координатор завершает работу после %d сообщений release\n", rank, release_count);
    fflush(stdout);

    free(queue_ranks);
    free(queue_iterations);
}

static void run_worker(int rank, int coordinator_rank, int repeat) {
    int i;

    for (i = 0; i < repeat; ++i) {
        int iteration;
        int remainder_time;
        int critical_time;

        iteration = i + 1;
        remainder_time = random_section_time();
        printf("Процесс %d: вошел в remainder section на %d сек., итерация %d\n", rank, remainder_time, iteration);
        fflush(stdout);
        sleep((unsigned int) remainder_time);
        printf("Процесс %d: вышел из remainder section, итерация %d\n", rank, iteration);
        fflush(stdout);

        send_message(rank, coordinator_rank, REQUEST_TAG, iteration);
        (void) receive_message(rank, coordinator_rank, GRANT_TAG);

        critical_time = random_section_time();
        printf("Процесс %d: вошел в critical section на %d сек., итерация %d\n", rank, critical_time, iteration);
        fflush(stdout);
        sleep((unsigned int) critical_time);
        printf("Процесс %d: вышел из critical section, итерация %d\n", rank, iteration);
        fflush(stdout);

        send_message(rank, coordinator_rank, RELEASE_TAG, iteration);
    }

    printf("Процесс %d: завершил все %d итерации\n", rank, repeat);
    fflush(stdout);
}

int main(int argc, char **argv) {
    const int repeat = 3;
    int rank;
    int size;
    int coordinator_rank;

    check_mpi(MPI_Init(&argc, &argv), "MPI_Init");
    check_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank), "MPI_Comm_rank");
    check_mpi(MPI_Comm_size(MPI_COMM_WORLD, &size), "MPI_Comm_size");
    srandom((unsigned int) rank);

    if (size <= 3) {
        if (rank == 0) {
            printf("Ошибка: требуется N > 3\n");
            fflush(stdout);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    coordinator_rank = size - 1;

    if (rank == coordinator_rank) {
        run_coordinator(rank, size, repeat);
    } else {
        run_worker(rank, coordinator_rank, repeat);
    }

    check_mpi(MPI_Finalize(), "MPI_Finalize");
    return 0;
}

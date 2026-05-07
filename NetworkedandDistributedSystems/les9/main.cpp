#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

enum {
    ELECTION_TAG = 1,
    OK_TAG = 2,
    COORDINATOR_TAG = 3
};

static volatile sig_atomic_t timeout_expired = 0;

static void check_mpi(int code, const char *operation) {
    if (code != MPI_SUCCESS) {
        MPI_Abort(MPI_COMM_WORLD, code);
    }
}

static void handler(int nsig) {
    (void) nsig;
    timeout_expired = 1;
}

static int next_rank_in_ring(int rank, int size) {
    return (rank + 1) % size;
}

static int find_coordinator(const int *participants, int size) {
    int coordinator;
    int i;

    coordinator = -1;
    for (i = 0; i < size; ++i) {
        if (participants[i]) {
            coordinator = i;
        }
    }

    return coordinator;
}

static void send_election(int rank, int size, int dst_rank, const int *participants) {
    check_mpi(
        MPI_Send((void *) participants, size, MPI_INT, dst_rank, ELECTION_TAG, MPI_COMM_WORLD),
        "MPI_Send(ELECTION)"
    );
    printf("Процесс %d: послал \"голосование\" процессу %d\n", rank, dst_rank);
    fflush(stdout);
}

static void send_ok(int rank, int dst_rank) {
    int value;

    value = rank;
    check_mpi(
        MPI_Send(&value, 1, MPI_INT, dst_rank, OK_TAG, MPI_COMM_WORLD),
        "MPI_Send(OK)"
    );
    printf("Процесс %d: послал \"ок\" процессу %d\n", rank, dst_rank);
    fflush(stdout);
}

static void send_coordinator(int rank, int dst_rank, int coordinator_rank) {
    check_mpi(
        MPI_Send(&coordinator_rank, 1, MPI_INT, dst_rank, COORDINATOR_TAG, MPI_COMM_WORLD),
        "MPI_Send(COORDINATOR)"
    );
    printf("Процесс %d: послал \"координатор\" процессу %d, координатор = %d\n",
           rank, dst_rank, coordinator_rank);
    fflush(stdout);
}

int main(int argc, char **argv) {
    const int initiator_rank = 1;
    const unsigned int timeout = 2;
    int rank;
    int size;
    int alive;
    int finished;
    int participant;
    int waiting_for_ok;
    int election_sent_to;
    int next_alive_rank;
    int coordinator_started_by_me;
    int *participants;

    check_mpi(MPI_Init(&argc, &argv), "MPI_Init");
    check_mpi(MPI_Comm_rank(MPI_COMM_WORLD, &rank), "MPI_Comm_rank");
    check_mpi(MPI_Comm_size(MPI_COMM_WORLD, &size), "MPI_Comm_size");
    signal(SIGALRM, handler);

    if (size <= 5) {
        if (rank == 0) {
            printf("Ошибка: требуется N > 5\n");
            fflush(stdout);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    participants = (int *) calloc((size_t) size, sizeof(int));
    if (participants == NULL) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    alive = 1;
    if (rank == initiator_rank) {
        printf("Процесс %d: не участвует в жеребьевке и инициирует голосование\n", rank);
        fflush(stdout);
    } else {
        long draw;

        srandom(rank);
        draw = random();
        if (draw % 2 != 0) {
            printf("Процесс %d: число %ld нечетное, процесс продолжает жить\n", rank, draw);
            fflush(stdout);
        } else {
            alive = 0;
            printf("Процесс %d: число %ld четное, процесс считается мертвым\n", rank, draw);
            printf("Процесс %d: аккуратно завершает работу после жеребьевки\n", rank);
            fflush(stdout);
        }
    }

    if (!alive) {
        free(participants);
        check_mpi(MPI_Finalize(), "MPI_Finalize");
        return 0;
    }

    finished = 0;
    participant = 0;
    waiting_for_ok = 0;
    election_sent_to = -1;
    next_alive_rank = -1;
    coordinator_started_by_me = 0;

    if (rank == initiator_rank) {
        participant = 1;
        participants[rank] = 1;
        election_sent_to = next_rank_in_ring(rank, size);

        if (election_sent_to == rank) {
            printf("Процесс %d: обнаружил, что остался один, и завершает работу\n", rank);
            fflush(stdout);
            finished = 1;
        } else {
            send_election(rank, size, election_sent_to, participants);
            waiting_for_ok = 1;
            timeout_expired = 0;
            alarm(timeout);
        }
    } else {
        printf("Процесс %d: жив и ждет сообщения\n", rank);
        fflush(stdout);
    }

    while (!finished) {
        MPI_Status status;
        int has_message;

        check_mpi(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status), "MPI_Iprobe");

        if (has_message) {
            if (status.MPI_TAG == ELECTION_TAG) {
                check_mpi(
                    MPI_Recv(participants, size, MPI_INT, status.MPI_SOURCE, ELECTION_TAG, MPI_COMM_WORLD, &status),
                    "MPI_Recv(ELECTION)"
                );
                printf("Процесс %d: получил \"голосование\" от процесса %d\n", rank, status.MPI_SOURCE);
                fflush(stdout);
                send_ok(rank, status.MPI_SOURCE);

                if (rank == initiator_rank && participant) {
                    int coordinator_rank;

                    coordinator_rank = find_coordinator(participants, size);
                    printf("Процесс %d: получил свое сообщение \"голосование\" обратно\n", rank);
                    printf("Процесс %d: определил координатора %d\n", rank, coordinator_rank);
                    fflush(stdout);

                    if (next_alive_rank == rank) {
                        printf("Процесс %d: обнаружил, что остался один, и завершает работу\n", rank);
                        fflush(stdout);
                        finished = 1;
                    } else {
                        send_coordinator(rank, next_alive_rank, coordinator_rank);
                        coordinator_started_by_me = 1;
                        waiting_for_ok = 0;
                        timeout_expired = 0;
                        alarm(0);
                    }
                } else {
                    participant = 1;
                    participants[rank] = 1;
                    election_sent_to = next_rank_in_ring(rank, size);
                    send_election(rank, size, election_sent_to, participants);
                    waiting_for_ok = 1;
                    timeout_expired = 0;
                    alarm(timeout);
                }
            } else if (status.MPI_TAG == OK_TAG) {
                int ok_value;

                check_mpi(
                    MPI_Recv(&ok_value, 1, MPI_INT, status.MPI_SOURCE, OK_TAG, MPI_COMM_WORLD, &status),
                    "MPI_Recv(OK)"
                );
                printf("Процесс %d: получил \"ок\" от процесса %d\n", rank, status.MPI_SOURCE);
                fflush(stdout);
                next_alive_rank = status.MPI_SOURCE;
                waiting_for_ok = 0;
                timeout_expired = 0;
                alarm(0);
            } else if (status.MPI_TAG == COORDINATOR_TAG) {
                int coordinator_rank;

                check_mpi(
                    MPI_Recv(&coordinator_rank, 1, MPI_INT, status.MPI_SOURCE, COORDINATOR_TAG, MPI_COMM_WORLD, &status),
                    "MPI_Recv(COORDINATOR)"
                );
                printf("Процесс %d: получил \"координатор\" от процесса %d, координатор = %d\n",
                       rank, status.MPI_SOURCE, coordinator_rank);
                fflush(stdout);

                if (coordinator_started_by_me) {
                    printf("Процесс %d: получил свое сообщение \"координатор\" обратно и завершает работу\n", rank);
                    fflush(stdout);
                    finished = 1;
                } else {
                    send_coordinator(rank, next_alive_rank, coordinator_rank);
                    printf("Процесс %d: завершает работу после передачи сообщения \"координатор\"\n", rank);
                    fflush(stdout);
                    finished = 1;
                }
            }

            continue;
        }

        if (waiting_for_ok && timeout_expired) {
            if (election_sent_to == initiator_rank) {
                int coordinator_rank;

                waiting_for_ok = 0;
                timeout_expired = 0;
                alarm(0);
                coordinator_rank = find_coordinator(participants, size);

                printf("Процесс %d: TIMEOUT истек после отправки \"голосование\" процессу %d\n", rank, election_sent_to);
                printf("Процесс %d: процесс %d считается мертвым\n", rank, election_sent_to);
                fflush(stdout);

                if (next_alive_rank == rank) {
                    printf("Процесс %d: обнаружил, что остался один, и завершает работу\n", rank);
                    fflush(stdout);
                    finished = 1;
                } else {
                    printf("Процесс %d: считает координатором процесс %d\n", rank, coordinator_rank);
                    fflush(stdout);
                    send_coordinator(rank, next_alive_rank, coordinator_rank);
                    coordinator_started_by_me = 1;
                }
            } else {
                waiting_for_ok = 0;
                timeout_expired = 0;
                alarm(0);
                printf("Процесс %d: TIMEOUT истек после отправки \"голосование\" процессу %d\n", rank, election_sent_to);
                printf("Процесс %d: процесс %d считается мертвым, продолжается обход по кольцу\n", rank, election_sent_to);
                fflush(stdout);
                election_sent_to = next_rank_in_ring(election_sent_to, size);
                send_election(rank, size, election_sent_to, participants);
                waiting_for_ok = 1;
                timeout_expired = 0;
                alarm(timeout);
            }
        }
    }

    free(participants);
    check_mpi(MPI_Finalize(), "MPI_Finalize");
    return 0;
}

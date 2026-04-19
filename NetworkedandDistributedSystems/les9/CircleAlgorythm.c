#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <mpi.h>

enum {
    voting_tag = 1,
    okey_tag = 2,
    coordinator_tag = 3
};

volatile sig_atomic_t timeout_exp = 0;

void Check_For_SUCCESS(int MPI_FUNC) {
    if (MPI_FUNC != MPI_SUCCESS) {MPI_Abort(MPI_COMM_WORLD, -1);}
}

void handler(int nsig) {
    (void) nsig;
    timeout_exp = 1;
}

static int Next_Process_in_Circle(int rank, int size) {
    return (1+rank)%size;
}

static int Find_Coordinator(const int *processes, int size) {
    int coord = -1, i;
    for (i=0; i<size; ++i) {
        if (processes[i]) {coord = i;}
    }
    return coord;
}

static void Send_Voting(int rank, int size, int dst_rank, const int *processes) {
    Check_For_SUCCESS(MPI_Send((void *) processes, size, MPI_INT, dst_rank, voting_tag, MPI_COMM_WORLD));
    printf("Process %d: send 'voting' to process %d\n", rank, dst_rank);
    fflush(stdout);
}

static void Send_Okey(int rank, int dst_rank) {
    int value = rank;
    Check_For_SUCCESS(MPI_Send(&value, 1, MPI_INT, dst_rank, okey_tag, MPI_COMM_WORLD));
    printf("Process %d: send 'okey' to process %d\n", rank, dst_rank);
    fflush(stdout);
}

static void Send_Coordinator(int rank, int dst_rank, int coord_rank) {
    Check_For_SUCCESS(MPI_Send(&coord_rank, 1, MPI_INT, dst_rank, coordinator_tag, MPI_COMM_WORLD));
    printf("Process %d: send 'coordinator' to process %d, coordinator is %d\n",rank, dst_rank, coord_rank);
    fflush(stdout);
}


int main(int argc, char **argv) {
    const int initiator_rank = 1, timeout = 2;
    int size, rank, alive, finished, participant, wait_ok, election_sent_to, next_alive_rank, coordinator_starts_by_me, *processes;
    Check_For_SUCCESS(MPI_Init(&argc, &argv));
    Check_For_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &size));
    Check_For_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    if (size <= 5) {
        if (rank == 0) {
            printf("Error: need N > 5\n");
            fflush(stdout);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    signal(SIGALRM, handler);
    alive = 1;
    if (rank == initiator_rank) {
        printf("Process %d: does not participate in the lottery and is the first initiator\n", rank);
        fflush(stdout);
    } else {
        long coin; // подбрасываем "монетку"
        srandom(rank);
        coin = random();
        if (coin % 2 == 0) {
            alive = 0;
            printf("Process %d: number %ld is even, process is considered dead\n", rank, coin);
        } else {
            printf("Process %d: number %ld is odd, process continues to live\n", rank, coin);
        }
        fflush(stdout);
    }
    if (!alive) {
        printf("Process %d: gracefully terminates after the lottery\n", rank);
        fflush(stdout);
        Check_For_SUCCESS(MPI_Finalize());
        return 0;
    }
    processes = (int *) calloc((size_t) size, sizeof(int));
    if (processes == NULL) {MPI_Abort(MPI_COMM_WORLD, -1);}
    finished = 0; participant = 0;
    wait_ok = 0; election_sent_to = -1;
    next_alive_rank = -1; coordinator_starts_by_me = 0;
    if (rank == initiator_rank) {
        participant = 1;
        processes[rank] = 1;
        election_sent_to = Next_Process_in_Circle(rank, size);
        if (election_sent_to == rank) {
            printf("Process %d: understand that it is alone and ends work\n", rank);
            fflush(stdout);
            finished = 1;
        } else {
            Send_Voting(rank, size, election_sent_to, processes);
            wait_ok = 1;
            timeout_exp = 0;
            alarm(timeout);
        }
    } else {
        printf("Process %d: alive and is waiting for message\n", rank);
        fflush(stdout);
    }

    while (!finished) {
        MPI_Status status;
        int have_msg;
        Check_For_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &have_msg, &status));
        if (have_msg) {
            if (status.MPI_TAG == voting_tag) {
                Check_For_SUCCESS(MPI_Recv(processes, size, MPI_INT, status.MPI_SOURCE, voting_tag, MPI_COMM_WORLD, &status));
                printf("Process %d: recieved 'voting' from process%d\n", rank, status.MPI_SOURCE);
                fflush(stdout);
                Send_Okey(rank, status.MPI_SOURCE);
                if (rank == initiator_rank && participant) {
                    int coordinator_rank;
                    coordinator_rank = Find_Coordinator(processes, size);
                    printf("Process %d: recieved his message 'voting' back\n", rank);
                    printf("Process %d: itendified coordinator %d\n", rank, coordinator_rank);
                    fflush(stdout);
                    if (next_alive_rank == rank) {
                        printf("Process %d: alive and is waiting for message\n", rank);
                        fflush(stdout);
                        finished = 1;
                    } else {
                        Send_Coordinator(rank, next_alive_rank, coordinator_rank);
                        coordinator_starts_by_me = 1;
                        wait_ok = 0;
                        timeout_exp = 0;
                        alarm(0);
                    }
                } else {
                    participant = 1;
                    processes[rank] = 1;
                    election_sent_to = Next_Process_in_Circle(rank, size);
                    Send_Voting(rank, size, election_sent_to, processes);
                    wait_ok = 1;
                    timeout_exp = 0;
                    alarm(timeout);
                }
            } else if (status.MPI_TAG == okey_tag) {
                int ok_value;
                Check_For_SUCCESS(MPI_Recv(&ok_value, 1, MPI_INT, status.MPI_SOURCE, okey_tag, MPI_COMM_WORLD, &status));
                printf("Process %d: get 'okey' from process %d\n", rank, status.MPI_SOURCE);
                fflush(stdout);
                next_alive_rank = status.MPI_SOURCE;
                wait_ok = 0;
                timeout_exp = 0;
                alarm(0);
            } else if (status.MPI_TAG == coordinator_tag) {
                int coordinator_rank;
                Check_For_SUCCESS(MPI_Recv(&coordinator_rank, 1, MPI_INT, status.MPI_SOURCE, coordinator_tag, MPI_COMM_WORLD, &status));
                printf("Process %d: recieved 'coordinator' from process %d, coordinator is %d\n", rank, status.MPI_SOURCE, coordinator_rank);
                fflush(stdout);
                if (coordinator_starts_by_me) {
                    printf("Process %d: recieved his message 'coordinator' back and ends work\n", rank);
                    fflush(stdout);
                    finished = 1;
                } else {
                    Send_Coordinator(rank, next_alive_rank, coordinator_rank);
                    printf("Process %d: ends his work after sending message 'coordinator'\n", rank);
                    fflush(stdout);
                    finished = 1;
                }
            }
            continue;
        }
        if (wait_ok && timeout_exp) {
            if (election_sent_to == initiator_rank) {
                int coordinator_rank;
                wait_ok = 0;
                timeout_exp = 0;
                alarm(0);
                coordinator_rank = Find_Coordinator(processes, size);
                printf("Process %d: timeout ends after sending 'voting' to process %d\n", rank, election_sent_to);
                printf("Process %d: process %d consideed dead\n", rank, election_sent_to);
                fflush(stdout);
                if (next_alive_rank == rank) {
                    printf("Process %d: understands that he is alone and ends work\n", rank);
                    fflush(stdout);
                    finished = 1;
                } else {
                    printf("Process %d: consider coordinator process %d\n", rank, coordinator_rank);
                    fflush(stdout);
                    Send_Coordinator(rank, next_alive_rank, coordinator_rank);
                    coordinator_starts_by_me = 1;
                }
            } else {
                wait_ok = 0;
                timeout_exp = 0;
                alarm(0);
                printf("Process %d: timeout ends after sending 'voting' to process%d\n", rank, election_sent_to);
                printf("Процесс %d: process %d considered dead, continuing working with next process in the circle\n", rank, election_sent_to);
                fflush(stdout);
                election_sent_to = Next_Process_in_Circle(election_sent_to, size);
                Send_Voting(rank, size, election_sent_to, processes);
                wait_ok = 1;
                timeout_exp = 0;
                alarm(timeout);
            }
        }

    }
    Check_For_SUCCESS(MPI_Finalize());
    return 0;
}


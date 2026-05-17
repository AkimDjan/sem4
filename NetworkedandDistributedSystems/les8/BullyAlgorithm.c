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

void Send_to_Higher_Ranks(int rank, int size, int *sent_count) {
    int value = rank, dst_rank;
    *sent_count = 0;
    for (dst_rank = rank + 1; dst_rank < size; ++dst_rank) {
        Check_For_SUCCESS(MPI_Send(&value, 1, MPI_INT, dst_rank, voting_tag, MPI_COMM_WORLD));
        printf("Process %d: send 'voting' to process %d\n", rank, dst_rank);
        fflush(stdout);
        ++(*sent_count);
    }
}

void Send_to_Lower_Ranks(int rank) {
    int value = rank, dst_rank;
    for (dst_rank = 0; dst_rank < rank; ++dst_rank) {
        Check_For_SUCCESS(MPI_Send(&value, 1, MPI_INT, dst_rank, coordinator_tag, MPI_COMM_WORLD));
        printf("Process %d: send 'coordinator' to process %d\n", rank, dst_rank);
        fflush(stdout);
    }
}


int main(int argc, char **argv) {
    const int initiator_rank = 1, timeout = 5;
    int size, rank, alive, finished, participant, wait_ok;
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
        if (coin % 2 != 0) {
            alive = 0;
            printf("Process %d: number %ld is odd, process is considered dead\n", rank, coin);
        } else {
            printf("Process %d: number %ld is even, process continues to live\n", rank, coin);
        }
        fflush(stdout);
    }
    if (!alive) {
        printf("Process %d: gracefully terminates after the lottery\n", rank);
        fflush(stdout);
        Check_For_SUCCESS(MPI_Finalize());
        return 0;
    }
    finished = 0;
    participant = 0;
    wait_ok = 0;
    if (rank == initiator_rank) {
        int sent_count;
        participant = 1;
        printf("Process %d: starts voting, timeout %u seconds\n", rank, timeout);
        fflush(stdout);
        Send_to_Higher_Ranks(rank, size, &sent_count);
        if (sent_count == 0) {
            printf("Process %d: no processes with higher rank\n", rank);
            printf("Process %d: becomes the coordinator\n", rank);
            fflush(stdout);
            Send_to_Lower_Ranks(rank);
            finished = 1;
        } else {
            wait_ok = 1;
            timeout_exp = 0;
            alarm(timeout);
        }
    }   else {
        printf("Process %d: waits for voting\n", rank);
        fflush(stdout);
    }
    while (!finished) {
        MPI_Status status;
        int have_msg;
        Check_For_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &have_msg, &status));
        if (have_msg) {
            int value;
            Check_For_SUCCESS(MPI_Recv(&value, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status));
            if (status.MPI_TAG == voting_tag) {
                printf("Process %d: received 'voting' from process %d\n", rank, status.MPI_SOURCE);
                fflush(stdout);
                if (alive) {
                    int answer, sent_count;
                    answer = rank;
                    Check_For_SUCCESS(MPI_Send(&answer, 1, MPI_INT, status.MPI_SOURCE, okey_tag, MPI_COMM_WORLD));
                    printf("Process %d: sent 'ok' to process %d\n", rank, status.MPI_SOURCE);
                    fflush(stdout);
                    if (!participant) {
                        participant = 1;
                        printf("Process %d: starts voting itself, timeout %u seconds\n", rank, timeout);
                        fflush(stdout);
                        Send_to_Higher_Ranks(rank, size, &sent_count);
                        if (sent_count == 0) {
                            printf("Process %d: no processes with higher rank\n", rank);
                            printf("Process %d: becomes the coordinator\n", rank);
                            fflush(stdout);
                            Send_to_Lower_Ranks(rank);
                            finished = 1;
                        } else {
                            wait_ok = 1;
                            timeout_exp = 0;
                            alarm(timeout);
                        }
                    }
                } else {
                    printf("Process %d: is considered dead and does not respond to voting\n", rank);
                    fflush(stdout);
                }
            } else if (status.MPI_TAG == okey_tag) {
                if (alive) {
                    printf("Process %d: received 'ok' from process %d\n", rank, status.MPI_SOURCE);
                    printf("Process %d: resets the waiting timer\n", rank);
                    fflush(stdout);
                    alarm(0);
                    timeout_exp = 0;
                    wait_ok = 0;
                }
            } else if (status.MPI_TAG == coordinator_tag) {
                alarm(0);
                timeout_exp = 0;
                printf("Process %d: received 'coordinator' from process %d, new coordinator = %d\n",rank, status.MPI_SOURCE, value);
                if (alive) {
                    printf("Process %d: finishes work\n", rank);
                    fflush(stdout);
                    finished = 1;
                }
            }
        }
        if (wait_ok && timeout_exp) {
            printf("Process %d: timeout expired while waiting for 'ok'\n", rank);
            printf("Process %d: becomes the coordinator\n", rank);
            fflush(stdout);
            Send_to_Lower_Ranks(rank);
            finished = 1;
        }  
    } 
Check_For_SUCCESS(MPI_Finalize());
return 0;
}


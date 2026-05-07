#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <mpi.h>

enum {
    token_tag = 1,
    fin_tag = 2
};

volatile sig_atomic_t timeout_exp = 0;

void Check_For_SUCCESS(int MPI_FUNC) {
    if (MPI_FUNC != MPI_SUCCESS) {MPI_Abort(MPI_COMM_WORLD, -1);}
}

static void handler(int nsig) {
    (void) nsig;
    timeout_exp = 1;
}

static int Next_Process_in_Circle(int rank, int size) {
    return (rank + 1) % size;
}

static void Send_Token(int dst_rank) {
    int value = 1;
    Check_For_SUCCESS(MPI_Send(&value, 1, MPI_INT, dst_rank, token_tag, MPI_COMM_WORLD));
}

static void Send_FIN(int rank, int dst_rank, int round) {
    Check_For_SUCCESS(MPI_Send(&round, 1, MPI_INT, dst_rank, fin_tag, MPI_COMM_WORLD));
    printf("Process %d: send FIN circle %d to process %d\n", rank, round, dst_rank);
    fflush(stdout);
}

static void receive_token(MPI_Status *status) {
    int value;
    Check_For_SUCCESS(MPI_Recv(&value, 1, MPI_INT, status->MPI_SOURCE, token_tag, MPI_COMM_WORLD, status));
}

static int receive_fin(int rank, MPI_Status *status) {
    int round;
    Check_For_SUCCESS(MPI_Recv(&round, 1, MPI_INT, status->MPI_SOURCE, fin_tag, MPI_COMM_WORLD, status));
    printf("Process %d: get FIN circle %d from process %d\n", rank, round, status->MPI_SOURCE);
    fflush(stdout);
    return round;
}

int main(int argc, char **argv) {
    const int repeat = 3;
    int rank, size, next_rank, i = 0, has_token = 0, pending_fin_round = 0, finished = 0;
    Check_For_SUCCESS(MPI_Init(&argc, &argv));
    Check_For_SUCCESS(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    Check_For_SUCCESS(MPI_Comm_size(MPI_COMM_WORLD, &size));
    signal(SIGALRM, handler);
    srandom((unsigned int) rank);
    if (size <= 3) {
        if (rank == 0) {
            printf("Error: need N > 3\n");
            fflush(stdout);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    next_rank = Next_Process_in_Circle(rank, size);
    if (rank == 0) {Send_Token(next_rank);}
    while (i < repeat) {
        int crit_time, rem_time;
        printf("Process %d: before enter the critical section, iter num  %d\n", rank, i + 1);
        fflush(stdout);
        while (!has_token) {
            MPI_Status status;
            int has_message;
            Check_For_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status));
            if (!has_message) {continue;}
            if (status.MPI_TAG == token_tag) {
                receive_token(&status);
                has_token = 1;
            } else if (status.MPI_TAG == fin_tag) {
                pending_fin_round = receive_fin(rank, &status);
            }
        }
        crit_time = (int) (random() % 2) + 1;
        printf("Process %d: enter the critical section on %d seconds\n", rank, crit_time);
        fflush(stdout);
        sleep((unsigned int) crit_time);
        printf("Process %d: get out from critical section\n", rank);
        fflush(stdout);
        Send_Token(next_rank);
        has_token = 0;
        rem_time = (int) (random() % 5) + 1;
        timeout_exp = 0;
        alarm((unsigned int) rem_time);
        printf("Process %d: enter the remainder section on %d seconds\n", rank, rem_time);
        fflush(stdout);
        while (!timeout_exp) {
            MPI_Status status;
            int has_message;
            Check_For_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status));
            if (!has_message) {continue;}
            if (status.MPI_TAG == token_tag) {
                receive_token(&status);
                Send_Token(next_rank);
            } else if (status.MPI_TAG == fin_tag) {
                pending_fin_round = receive_fin(rank, &status);
            }
        }
        alarm(0);
        printf("Process %d: get out from remainder section\n", rank);
        fflush(stdout);
        ++i;
    }
    if (rank == 0) {
        Send_FIN(rank, next_rank, 1);
    } else if (pending_fin_round == 1) {
        Send_FIN(rank, next_rank, 1);
        pending_fin_round = 0;
    }
    while (!finished) {
        MPI_Status status;
        int has_message;
        Check_For_SUCCESS(MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &has_message, &status));
        if (!has_message) {continue;}
        if (status.MPI_TAG == token_tag) {
            receive_token(&status);
            Send_Token(next_rank);
        } else if (status.MPI_TAG == fin_tag) {
            int round = receive_fin(rank, &status);
            if (round == 1) {
                if (rank == 0) {Send_FIN(rank, next_rank, 2);} 
                else {Send_FIN(rank, next_rank, 1);}
            } else {
                if (rank != 0) {Send_FIN(rank, next_rank, 2);}
                finished = 1;
            }
        }
    }
    Check_For_SUCCESS(MPI_Finalize());
    return 0;
}

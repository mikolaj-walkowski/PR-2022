#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "defs.h"

long start, wait_T;

int Laccept[MAXSIZE];
int Taccept[MAXSIZE];
int lClk = 0;

void *posterunek() {
    while(start + wait_T >= time(0)) {
        int flag;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

        if (flag == 1)
        {
            Message msg;
            int type;
            MPI_Recv(&msg, msg_size(), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            msg.clock = lClk;
            switch (status.MPI_TAG)
            {
            case LZ_REQ:
            {
                type = LZ_RES;
                MPI_Send(&msg, msg_size(), MPI_INT, status.MPI_SOURCE, type, MPI_COMM_WORLD);
                break;
            }
            case TP_REQ:
            {
                type = TP_RES;
                MPI_Send(&msg, msg_size(), MPI_INT, status.MPI_SOURCE, type, MPI_COMM_WORLD);
                break;
            }
            default:
                break;
            }
        }
    }
    lClk++;

    pthread_exit(NULL);
}

int rank, size;
Message tmp;
int ResNUM = 0;
Vec T;
Vec L;
int reqID = 0;

void* lazaretReq() {
    vec_init(&L);

    for (int i = 0; i < MAXSIZE; ++i)
    {
        Laccept[i] = 0;
    }

    tmp.clock = lClk;
    tmp.id = ++reqID;
    sendAll(rank, size, tmp, LZ_REQ);
    ResNUM = 0;

    while (!(size - 1 - ResNUM < L_SPACE)) {
        Message msg;
        MPI_Status status;

        MPI_Recv(&msg, msg_size(), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG)
        {
            case LZ_REQ:
            {
                int otherId = status.MPI_SOURCE, otherCL = msg.clock;
                if (lClk < otherCL || (lClk == otherCL && otherId > rank))
                {
                    if (Laccept[otherId] == 0)
                    {
                        Laccept[otherId] = 1;
                        vec_push(&L, msg.id);
                        vec_push(&L, otherId);
                        ResNUM++;
                    }
                }
                break;
            }
            case TP_REQ:
            {
                msg.clock = lClk;
                MPI_Send(&msg, msg_size(), MPI_INT, status.MPI_SOURCE, TP_RES, MPI_COMM_WORLD);
                break;
            }
            case LZ_RES:
            {
                if (reqID == msg.id)
                {
                    if (Laccept[status.MPI_SOURCE] == 0)
                    {
                        ResNUM++;
                        Laccept[status.MPI_SOURCE] = 1;
                    }
                }
                break;
            }
        }
    }


    pthread_exit(NULL);
}

void *teleport() {
    for (int i = 0; i < MAXSIZE; ++i)
    {
        Taccept[i] = 0;
    }

    Message req;
    req.clock = lClk;
    req.id = reqID;

    sendAll(rank, size, req, TP_REQ);

    int ResNUM = 0;
    Vec T;
    vec_init(&T);
    while (!(size - 1 - ResNUM < T_SPACE))
    {
        MPI_Status status;
        Message msg;
        MPI_Recv(&msg, msg_size(), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        switch (status.MPI_TAG)
        {
            case TP_REQ:
            {
                int otherId = status.MPI_SOURCE, otherCL = msg.clock;
                if (lClk < otherCL || (lClk == otherCL && otherId > rank))
                {
                    if (Taccept[otherId] == 0)
                    {
                        vec_push(&T, msg.id);
                        vec_push(&T, otherId);
                        ResNUM++;
                        Taccept[otherId] = 1;
                    }
                }
                break;
            }
            case TP_RES:
            {
                if (reqID == msg.id)
                {
                    if (Taccept[status.MPI_SOURCE] == 0)
                    {
                        ResNUM++;
                        Taccept[status.MPI_SOURCE] = 1;
                    }
                }
                break;
            }
        }
    }
    pthread_exit(NULL);
}

void *lazaret() {
    start = time(0);
    wait_T = (rand() % WAIT) + 1;

    while (start + wait_T >= time(0))
    {
        int flag;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag == 1)
        {
            Message msg;
            int type;
            MPI_Recv(&msg, msg_size(), MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            msg.clock = lClk;
            switch (status.MPI_TAG)
            {
            case TP_REQ:
            {
                type = TP_RES;
                MPI_Send(&msg, msg_size(), MPI_INT, status.MPI_SOURCE, type, MPI_COMM_WORLD);
                break;
            }
            case LZ_REQ:
            { 
                vec_push(&L, msg.id);
                vec_push(&L, status.MPI_SOURCE);
                break;
            }
            }
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int reqID = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    pthread_t tid;
    srand(rank * 100);
    while(1) {
        start = time(0);
        wait_T = (rand() % WAIT) + 1;

        printf("%d| Posterunek\n", rank);
        
        pthread_create(&tid, NULL, posterunek, NULL);
        pthread_join(tid, NULL);
        
        printf("%d| Chce być w lazarecie\n", rank);
        
        pthread_create(&tid, NULL, lazaretReq, NULL);
        pthread_join(tid, NULL);
        
        printf("%d| Zgoda na lazaret | Chce teleport\n", rank);
        
        lClk++;
        ++reqID;

        pthread_create(&tid, NULL, teleport, NULL);
        pthread_join(tid, NULL);

        printf("%d| Zgoda na teleport do lazaretu | Teleportuję...\n", rank);
        sleep(1);
        printf("%d| Jestem w Lazarecie\n", rank);

        while (T.size > 0)
        {
            int rec = vec_pop(&T);
            Message msg;
            msg.clock = lClk;
            msg.id = vec_pop(&T);

            MPI_Send(&msg, msg_size(), MPI_INT, rec, LZ_RES, MPI_COMM_WORLD);
        }
        vec_destroy(&T);

        pthread_create(&tid, NULL, lazaret, NULL);
        pthread_join(tid, NULL);

        printf("%d| Chcę teleport na posterunek\n", rank);

        pthread_create(&tid, NULL, teleport, NULL);
        pthread_join(tid, NULL);

        sleep(1);
        printf("%d| Jestem na posterunku\n", rank);

        while (T.size > 0)
        {
            int rec = vec_pop(&T);
            Message msg;
            msg.clock = lClk;
            msg.id = vec_pop(&T);

            MPI_Send(&msg, msg_size(), MPI_INT, rec, TP_RES, MPI_COMM_WORLD);
        }
        vec_destroy(&T);
        while (L.size > 0)
        {
            int rec = vec_pop(&L);
            printf("%d| wysylam pending zgode\n", rank);
            Message msg;
            msg.clock = lClk;
            msg.id = vec_pop(&L);

            MPI_Send(&msg, msg_size(), MPI_INT, rec, LZ_RES, MPI_COMM_WORLD);
        }
        vec_destroy(&L);



    }
    return 0;
}
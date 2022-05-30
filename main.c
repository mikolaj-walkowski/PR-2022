#include "defs.h"

void DBGprint(Message req, Message res,int rank, MPI_Status s1, int tag2, char* comment, char* color){
    printf("%sComment: %s %s\n\tProcess: %d, dostał wiadomość od: %d , Wiadomość odebrana [tag: %d]: %d , %d. Wiadomość wysłana[tag: %d]: %d, %d.\n",color,comment, RESET,rank, s1.MPI_SOURCE, s1.MPI_TAG, req.id,req.clock, tag2, res.id,res.clock);
}

void teleport(int iClock, int size, int rank, int reqId)
{
    for (int i = 0; i < MAXSIZE; ++i)
    {
        Taccept[i] = 0;
    }

    Message req;
    req.clock = iClock;
    req.id = reqId;

    sendAll(rank, size, req, TP_REQ);

    int ResNUM = 0;
    Vec T;
    vec_init(&T);

    while (!(size - 1 - ResNUM < T_SPACE))
    {
        MPI_Status status;
        Message msg;
        MPI_Recv(&msg, sizeof(Message), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        int type = 0;
        switch (status.MPI_TAG)
        {
        case TP_REQ:
        {
            type = TP_REQ;
            int otherId = status.MPI_SOURCE, otherCL = msg.clock;
            if (iClock < otherCL || (iClock == otherCL && otherId > rank))
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
            type = TP_RES;
            if (reqId == msg.id)
            {
                if (Taccept[status.MPI_SOURCE] == 0)
                {
                    ResNUM++;
                    Taccept[status.MPI_SOURCE] = 1;
                }
            }
            break;
        }
        DBGprint(msg,msg,rank,status,type,"TP req Region",KBLU);
        }

    }

    long start = time(0);
    long wait_T = (rand() % WAIT) + 1;

    printf("%d Zgoda na TP\n", rank);
    while (start + wait_T >= time(0))
        ;

    while (T.size > 0)
    {
        int rec = vec_pop(&T);
        Message msg;
        msg.clock = iClock;
        msg.id = vec_pop(&T);

        MPI_Send(&msg, sizeof(Message), MPI_BYTE, rec, LZ_RES, MPI_COMM_WORLD);
    }
    vec_destroy(&T);
}

int main(int argc, char **argv)
{
    int rank, size;
    int lClk = 0;
    int reqID = 0;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank * 100);
    Vec L;
    vec_init(&L);

    while (1)
    {
        long start = time(0);
        long wait_T = (rand() % WAIT) + 1;
        printf("%d Posterunek\n", rank);

        while (start + wait_T >= time(0))
        {
            int flag;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (flag == 1)
            {
                Message msg;
                int type;
                MPI_Recv(&msg, sizeof(Message), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                msg.clock = lClk;
                switch (status.MPI_TAG)
                {
                case LZ_REQ:
                {
                    type = LZ_RES;
                    MPI_Send(&msg, sizeof(Message), MPI_BYTE, status.MPI_SOURCE, type, MPI_COMM_WORLD);
                    break;
                }
                case TP_REQ:
                {
                    type = TP_RES;
                    MPI_Send(&msg,sizeof(Message), MPI_BYTE, status.MPI_SOURCE, type, MPI_COMM_WORLD);
                    break;
                }
                default:
                    break;
                }
                DBGprint(msg,msg,rank,status,type,"Post Region",KMAG);
            }
        }

        lClk++;

        printf("%d Chce być w lazarecie\n", rank);

        for (int i = 0; i < MAXSIZE; ++i)
        {
            Laccept[i] = 0;
            //TODO debug
        }

        Message tmp;
        tmp.clock = lClk;
        tmp.id = ++reqID;

        sendAll(rank, size, tmp, LZ_REQ);

        int ResNUM = 0;
        while (!(size - 1 - ResNUM < L_SPACE))
        {
            MPI_Status status;
            Message msg;
            MPI_Recv(&msg, sizeof(Message), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int type =0;
            switch (status.MPI_TAG)
            {
                case LZ_REQ:
                {
                    type = LZ_REQ;
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
                    type = TP_REQ;
                    msg.clock = lClk;
                    MPI_Send(&msg, sizeof(Message), MPI_BYTE, status.MPI_SOURCE, TP_RES, MPI_COMM_WORLD);
                    break;
                }
                case LZ_RES:
                {
                    type = LZ_RES;
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
            DBGprint(msg,msg,rank,status,type,"Lazaret Request Region",KGRN);
        }
        // Zgoda na Lazaret użycie TP
        printf("%d Zgoda na Lazaret | Chcę użyć TP\n", rank);
        lClk++;
        ++reqID;
        teleport(lClk, size, rank, reqID);

        // Wyjście z TP
        printf("%d Wyjście z TP\n", rank);

        // Jestem w Lazarecie
        printf("%d Jestem w Lazarecie\n", rank);

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
                MPI_Recv(&msg, sizeof(Message), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                msg.clock = lClk;
                switch (status.MPI_TAG)
                {
                case TP_REQ:
                {
                    type = TP_RES;
                    MPI_Send(&msg, sizeof(Message), MPI_BYTE, status.MPI_SOURCE, type, MPI_COMM_WORLD);
                    break;
                }
                }
                DBGprint(msg,msg,rank,status,type,"Healing Region", KRED);
            }
        }

        // Wyjście z lazaretu TP
        printf("%d Wyjście z lazaretu użycie TP\n", rank);

        lClk++;
        ++reqID;
        teleport(lClk, size, rank, reqID);

        // Wyjście z TP
        printf("%d Wyjście z TP\n", rank);

        while (L.size > 0)
        {
            int rec = vec_pop(&L);

            Message msg;
            msg.clock = lClk;
            msg.id = vec_pop(&L);

            MPI_Send(&msg, sizeof(Message), MPI_BYTE, rec, LZ_RES, MPI_COMM_WORLD);
        }
        printf("%d Zwolnienie LZ\n", rank);
        // Chce być na posterunku
    }

    vec_destroy(&L);
    MPI_Finalize();
}
#include "defs.h"

void DBGprint(Message req, Message res,int rank, MPI_Status s1, int tag2, char* comment, char* color){
    printf("[%d]: %s%s%s from: %d, msg [%s]: %d , %d Response msg [%s]: %d, %d.\n",rank,color,comment,RESET,s1.MPI_SOURCE,tagNames[s1.MPI_TAG],req.id,req.clock, tagNames[tag2], res.id,res.clock);
}

void DBGprintState(int* a,int rank){
   char buff[1000];
    sprintf(buff,"[%d]: ", rank);
   int offset = strlen(buff);
    for (int i = 0; i < size; i++)
    {
        char * color = KGRN;
        if (a[i]==0)
        {
            color = KRED;
        }
        sprintf(buff+ offset,"%s[%d]%s,", color, i,RESET);
        offset = strlen(buff);
    }
    buff[offset] = '\n';
    buff[offset+1] = '\0';
    printf(buff);
}

void teleport(int iClock, int rank, int reqId)
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
        default:{
            break;
        } 
        }
        DBGprint(msg,msg,rank,status,type,"TP req Region",KBLU);
        DBGprintState(Taccept, rank);
    }
    printf("[%d]: %sEXITED TP REQ with ResNUM: %d====%s\n",rank,KRED,ResNUM,RESET);

    long start = time(0);
    long wait_T = (rand() % WAIT) + 1;

    printf("[%d]: %sTP Region ====%s\n",rank,KYEL,RESET);
    while (start + wait_T >= time(0));

    while (T.size > 0)
    {
        int rec = vec_pop(&T);
        Message msg;
        msg.clock = iClock;
        msg.id = vec_pop(&T);

        MPI_Send(&msg, sizeof(Message), MPI_BYTE, rec, TP_RES, MPI_COMM_WORLD);
    }
    vec_destroy(&T);
}

int main(int argc, char **argv)
{
    int rank;
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
        printf("[%d]:%s POSTERUNEK ====== %s\n", rank,KYEL,RESET);

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

         printf("[%d]: %sLAZARET REQ ====%s\n",rank,KYEL,RESET);

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
            DBGprintState(Laccept, rank);
        }
        printf("[%d]: %sEXITED LAZARET REQ with ResNUM: %d====%s\n",rank,KRED,ResNUM,RESET);
        
        // Zgoda na Lazaret użycie TP
        printf("[%d]: %s1ST TP REQ ====%s\n",rank,KYEL,RESET);
        lClk++;
        ++reqID;
        teleport(lClk, rank, reqID);

        // Jestem w Lazarecie
        printf("[%d]: %sLAZARET ====%s\n",rank,KYEL,RESET);

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
        printf("[%d]: %s2ST TP REQ ====%s\n",rank,KYEL,RESET);

        lClk++;
        ++reqID;
        teleport(lClk, rank, reqID);

        while (L.size > 0)
        {
            int rec = vec_pop(&L);

            Message msg;
            msg.clock = lClk;
            msg.id = vec_pop(&L);

            MPI_Send(&msg, sizeof(Message), MPI_BYTE, rec, LZ_RES, MPI_COMM_WORLD);
        }
    }

    vec_destroy(&L);
    MPI_Finalize();
}
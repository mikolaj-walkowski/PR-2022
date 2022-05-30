#include "defs.h"

char* DBGprintState(int* a){
   char *buff = malloc(sizeof(char)*1000);
   int offset = 0;
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
    return buff;
}

void DBGprint(Message *req, Message *res,int rank, MPI_Status s1, int tag2, char* comment, char* color, int* a){
    char *table ="NO TABLE";
    
    if (a != NULL)
    {
        table = DBGprintState(a);
    }
    
    // printf("[%d]: %s%s%s from: %d, msg [%s]: %d, %d Response msg [%s]: %d, %d Table state: %s\n",
    // rank, color, comment, RESET,
    // s1.MPI_SOURCE, tagNames[s1.MPI_TAG], req->id, req->clock,
    // tagNames[tag2], res->id, res->clock,
    // table);
    
    if (a != NULL)
    {
        free(table);
    }
}

void DBGprintRes(Message* msg,int rank, int target, int tag, char* comment, char* color){
    // printf("[%d]: %s%s%s Sending to %d, msg [%s]: %d, %d\n",
    // rank,color,comment,RESET,target,tagNames[tag],msg->id,msg->clock);
}

void teleport(int rank)
{
    for (int i = 0; i < MAXSIZE; ++i)
    {
        Taccept[i] = 0;
    }

    Message req;

    req.clock = lClk;
    req.id = lClk;

    sendAll(rank, size, req, TP_REQ);

    int ResNUM = 0;
    Vec T; vec_init(&T);

    while (!(size - 1 - ResNUM < T_SPACE))
    {
        MPI_Status status;
        Message msg;
        MPI_Recv(&msg, sizeof(Message), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        lClk = max(lClk, msg.clock)+1;
        int type = 0;
        switch (status.MPI_TAG)
        {
        case TP_REQ:
        {
            type = TP_REQ;
            int otherId = status.MPI_SOURCE, otherCL = msg.clock;
            if (reqID < otherCL || ( reqID == otherCL && otherId > rank))
            {
                if (Taccept[otherId] == 0)
                {
                    ResNUM++;
                    Taccept[otherId] = 1;
                }
                vec_push(&T, msg.id);
                vec_push(&T, otherId);
            }
            break;
        }
        case TP_RES:
        {
            type = TP_RES;
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
        default:{
            break;
        } 
        }
        DBGprint(&msg,&msg,rank,status,type,"TP req Region",KBLU, Taccept);
    }
    printf("[%d]: %sEXITED TP REQ with ResNUM: %d====%s\n",rank,KRED,ResNUM,RESET);

    long start = time(0);
    long wait_T = (rand() % WAIT) + WAIT_MIN;

    printf("[%d]: %sTP Region ====%s\n",rank,KYEL,RESET);
    while (start + wait_T >= time(0));

    while (T.size > 0)
    {
        int rec = vec_pop(&T);
        Message msg;
        msg.clock = lClk;
        msg.id = vec_pop(&T);
        MPI_Send(&msg, sizeof(Message), MPI_BYTE, rec, TP_RES, MPI_COMM_WORLD);
        DBGprintRes(&msg,rank,rec,TP_RES,"TP Response",KCYN);
    }
    vec_destroy(&T);
}

int main(int argc, char **argv)
{
    int rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    srand(rank * 100);
    Vec L;
    vec_init(&L);

    while (1)
    {
        long start = time(0);
        long wait_T = (rand() % WAIT) + WAIT_MIN;
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
                lClk = max(msg.clock,lClk) + 1;
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
                DBGprint(&msg,&msg,rank,status,type,"Post Region",KMAG,NULL);
            }
        }

        lClk++;

         printf("[%d]: %sLAZARET REQ ====%s\n",rank,KYEL,RESET);

        for (int i = 0; i < MAXSIZE; ++i)
        {
            Laccept[i] = 0;
        }

        Message tmp;

        tmp.clock = lClk;
        tmp.id = lClk;
        reqID = lClk;

        sendAll(rank, size, tmp, LZ_REQ);

        int ResNUM = 0;
        while (!(size - 1 - ResNUM < L_SPACE))
        {
            MPI_Status status;
            Message msg;
            MPI_Recv(&msg, sizeof(Message), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            lClk = max(lClk, msg.clock)+1;
            int type =0;
            switch (status.MPI_TAG)
            {
                case LZ_REQ:
                {
                    type = LZ_REQ;
                    int otherId = status.MPI_SOURCE, otherCL = msg.clock;
                    if (reqID < otherCL || (reqID == otherCL && otherId > rank))
                    {
                        if (Laccept[otherId] == 0)
                        {
                            Laccept[otherId] = 1;
                            ResNUM++;
                        }
                        vec_push(&L, msg.id);
                        vec_push(&L, otherId);
                    }
                    break;
                }
                case TP_REQ:
                {
                    type = TP_RES;
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
            DBGprint(&msg,&msg,rank,status,type,"Lazaret Request Region",KGRN,Laccept);
        }
        printf("[%d]: %sEXITED LAZARET REQ with ResNUM: %d====%s\n",rank,KRED,ResNUM,RESET);
        
        // Zgoda na Lazaret użycie TP
        printf("[%d]: %s1ST TP REQ ====%s\n",rank,KYEL,RESET);
        lClk++;
        reqID = lClk;
        teleport(rank);
        printf("[%d]: %s1ST TP EXIT ====%s\n",rank,KYEL,RESET);

        // Jestem w Lazarecie
        printf("[%d]: %sLAZARET ====%s\n",rank,KYEL,RESET);

        start = time(0);
        wait_T = (rand() % WAIT) + WAIT_MIN;

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
                lClk= max(lClk, msg.clock)+1;
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
                DBGprint(&msg,&msg,rank,status,type,"Healing Region", KRED,NULL);
            }
        }

        // Wyjście z lazaretu TP
        printf("[%d]: %s2ST TP REQ ====%s\n",rank,KYEL,RESET);

        lClk++;
        reqID = lClk;
        teleport(rank);
        printf("[%d]: %s2ST TP EXIT ====%s\n",rank,KYEL,RESET);
        while (L.size > 0)
        {
            int rec = vec_pop(&L);

            Message msg;
            msg.clock = lClk;
            msg.id = vec_pop(&L);

            MPI_Send(&msg, sizeof(Message), MPI_BYTE, rec, LZ_RES, MPI_COMM_WORLD);
            DBGprintRes(&msg,rank,rec,TP_RES,"LZ Response",KYEL);
        }
    }

    vec_destroy(&L);
    MPI_Finalize();
}
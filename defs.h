#include <mpi.h>

#define L_SPACE 1
#define T_SPACE 1

#define WAIT 5

#define MSG_CLK 0
#define MSG_ID 1
#define MAXSIZE 10000
enum reqTypes
{
    LZ_REQ = 0,
    LZ_RES = 1,
    TP_REQ = 2,
    TP_RES = 3
};

int Laccept[MAXSIZE];
int Taccept[MAXSIZE];

typedef struct message
{
    int clock;
    int id;
} Message;

int msg_size()
{
    return sizeof(Message) / sizeof(int);
}

typedef struct vec
{
    int *data;
    int MAX;
    int size;
} Vec;

void vec_init(Vec *vec)
{
    vec->MAX = 100;
    vec->size = 0;
    vec->data = malloc(sizeof(int) * vec->MAX);
}

void vec_push(Vec *id, int el)
{
    if (id->size == id->MAX)
    {
        id->MAX *= 2;
        id->data = realloc(id->data, id->MAX * sizeof(int));
    }
    id->data[id->size++] = el;
}

int vec_pop(Vec *id)
{
    if (id->size == 0)
        return -1;
    return id->data[--id->size];
}

void vec_destroy(Vec *id)
{
    free(id->data);
}

void sendAll(int rank, int size, Message msg, int type)
{
    for (int i = 0; i < size; i++)
    {
        if (i != rank)
        {
            MPI_Send(&msg, msg_size(), MPI_INT, i, type, MPI_COMM_WORLD);
        }
    }
}

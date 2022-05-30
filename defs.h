#pragma once

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET   "\033[0m"

#define L_SPACE 2 
#define T_SPACE 1

#define WAIT 1
#define WAIT_MIN 0

#define MAXSIZE 10000

enum reqTypes
{
    LZ_REQ = 1,
    LZ_RES = 2,
    TP_REQ = 3,
    TP_RES = 4
};

const char* tagNames[]= {
    "NULL",
    "LZ_REQ",
    "LZ_RES",
    "TP_REQ",
    "TP_RES"
};
int size; 
int lClk = 0;
int reqID = 0;

int Laccept[MAXSIZE];
int Taccept[MAXSIZE];

typedef struct message
{
    int clock;
    int id;
} Message;

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
            MPI_Send(&msg, sizeof(Message), MPI_BYTE, i, type, MPI_COMM_WORLD);
        }
    }
}

int max(int a, int b){
    return a>b? a:b;
}

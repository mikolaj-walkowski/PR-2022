#include <mpi.h>
#include <stdio.h>

#define LSPACE 10;
#define TSPACE 5

enum target{
    LAZARET,
    POSTERUNEK
}

struct list
{
    int *data;
    int MAX= 100;
    int size = 100;
};

typedef struct list List;

void initList(List* list){
    list->data = malloc(sizeof(int)*MAX);
}

void insert(List* id, int el){
    if(++(id->size) > id->MAX){
        id->MAX*=2;
        id->data = realloc(id->data, id->MAX*sizeof(int));
    }
    id[id->size - 1] = el;
}

int pop(List* id){
    if(id->size == 0) return -1;
    return id->data[id->size--];
}

void destroy(List* id){
    free(id->data);
} 

void teleport(int iClock, int size,int rank){
    //TODO MPI req TP 
    int ResNUM = 0 ,List LT;
    initList(&LT);
    while (!(size -1 - ResNUM < LSPACE)) //TODO Switch MBY
    {
        //TODO MPI get msg
        if(true){ // TODO MPI msg typ Treq
            int otherId =0, otherCL = 0; //TODO MPI   
            if(iClock < otherCL ||(iClock == otherCL && otherId > rank)){
                insert(&LT, otherId);
                ResNUM++;
            }
            continue;
        }
        if(true){ // TODO MPI msg typ Tagr -- !!! sprawdź clock
            ResNUM++;
            continue;
        }
    }

    long start = time(), wait_T = rand()%2000;
    while (start + wait_T>= time() )
    {
        //TODO wait
    }

    for (int i = pop(&LR); i != -1; i = pop(&LR)){
        //TODO MPI send resp 
    }
    destroy(&LT);
}

int main( int argc, char **argv )
{
	int rank, size;
    int iClock = 0; 
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	int namelen;
	MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	
    start:
	//Jestem na posterunku
    long start = time(), wait_T = rand()%2000;
    while (start + wait_T>= time() )
    {
        //TODO MPI respond msg
    }
    iClock++;
    // Chce być w lazarecie 
    
    //TODO MPI send laz req
    int ResNUM = 0 ,List LR;

    initList(&LR);
    while (!(size -1 - ResNUM < LSPACE)) //TODO Switch MBY
    {
        //TODO MPI get msg
        if(true){ // TODO MPI msg typ Lreq
            int otherId =0, otherCL = 0; //TODO MPI   
            if(iClock < otherCL ||(iClock == otherCL && otherId > rank)){
                insert(&LR, otherId);
                ResNUM++;
            }
            continue;
        }
        if (true) //TODO MPI msg type TP req
        {
            //TODO MPI respond gut
            continue;
        }
        if(true){ // TODO MPI msg typ Lagr -- !!! sprawdź clock
            ResNUM++;
            continue;
        }
    }
    // Zgoda na Lazaret użycie TP 
    iClock++;
    teleport(iClock,size,rank);
    //Wyjście z TP

    // Jestem w Lazarecie
    start = time(), wait_T = rand()%2000;
    while (start + wait_T>= time() )
    {
        //TODO MPI respond msg
    }
    // Wyjście z lazaretu TP
    iClock++;
    teleport(iClock,size,rank);
    //Wyjście z TP 
    for (int i = pop(&LR); i != -1; i = pop(&LR)){
        //TODO MPI send resp 
    }
    
    //Chce być na posterunku 
    goto start;
	MPI_Finalize();
}
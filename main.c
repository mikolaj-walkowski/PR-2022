#include <mpi.h>
#include <stdio.h>

#define LSPACE 10
#define TSPACE 5
#define MSG_SIZE 5

enum reqTypes{
    LZ_REQ=0,
    LZ_RES=1,
    TP_REQ=2,
    TP_RES =3
    };

struct vec
{
    int *data;
    int MAX;
    int size;
};

typedef struct vec Vec;

void vec_init(Vec* vec){
    vec->MAX = 0;
    vec->size = 0;
    vec->data = malloc(sizeof(int)*vec->MAX);
}

void vec_insert(Vec* id, int el){
    if(++(id->size) > id->MAX){
        id->MAX*=2;
        id->data = realloc(id->data, id->MAX*sizeof(int));
    }
    id->data[id->size - 1] = el;
}

int vec_pop(Vec* id){
    if(id->size == 0) return -1;
    return id->data[id->size--];
}

void vec_destroy(Vec* id){
    free(id->data);
} 

void teleport(int iClock, int size,int rank){
    //TODO MPI req TP 
    int ResNUM = 0 ;
    Vec T;
    vec_init(&T);
    while (!(size -1 - ResNUM < LSPACE)) //TODO Switch MBY
    {
        //TODO MPI get msg
        if(1){ // TODO MPI msg typ Treq
            int otherId =0, otherCL = 0; //TODO MPI   
            if(iClock < otherCL ||(iClock == otherCL && otherId > rank)){
                vec_insert(&T, otherId);
                ResNUM++;
            }
            continue;
        }
        if(1){ // TODO MPI msg typ Tagr -- !!! sprawdź clock
            ResNUM++;
            continue;
        }
    }

    long start = time(), wait_T = rand()%2000;
    while (start + wait_T>= time() )
    {
        //TODO wait
    }

    for (int i = vec_pop(&T); i != -1; i = vec_pop(&T)){
        //TODO MPI send resp 
    }
    vec_destroy(&T);
}

int main( int argc, char **argv )
{
	int rank, size;
    int iClock = 0; 
	MPI_Init( &argc, &argv );
    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	
    start:
	//Jestem na posterunku
    long start = time(), wait_T = rand()%2000;
    while (start + wait_T>= time() )
    {
        MPI_Status status;
        int msg[MSG_SIZE];
        MPI_Recv( msg , MSG_SIZE , MPI_INT ,MPI_ANY_SOURCE, TP_REQ, MPI_COMM_WORLD , &status);
    }
    iClock++;
    // Chce być w lazarecie 
    
    //TODO MPI send laz req
    int ResNUM = 0;
    Vec R;

    vec_init(&R);
    while (!(size -1 - ResNUM < LSPACE)) //TODO Switch MBY
    {
        //TODO MPI get msg
        if(1){ // TODO MPI msg typ Lreq
            int otherId =0, otherCL = 0; //TODO MPI   
            if(iClock < otherCL ||(iClock == otherCL && otherId > rank)){
                vec_insert(&R, otherId);
                ResNUM++;
            }
            continue;
        }
        if (1) //TODO MPI msg type TP req
        {
            //TODO MPI respond gut
            continue;
        }
        if(1){ // TODO MPI msg typ Lagr -- !!! sprawdź clock
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
    for (int i = vec_pop(&R); i != -1; i = vec_pop(&R)){
        //TODO MPI send resp 
    }
    
    //Chce być na posterunku 
    goto start;

	MPI_Finalize();
}
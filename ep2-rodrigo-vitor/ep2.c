#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define VELOCIDADE_SIMULACAO 1

int **pista;
int n_ciclistas;

typedef struct{
    int n;
    int pos_i;
    int pos_j;
} args;

pthread_mutex_t mutex;

void * ciclistas(void * arg){ 
    int volta_atual = 0, vel_met=120, n = *((int *)arg);
    printf("%d",n);
    // for(;;){
    //     usleep(vel_met*1000);
    //     pthread_mutex_lock(&mutex);
    //     pista[]
    //     pthread_mutex_unlock(&mutex);
    // }
    return arg;
}

void primeiraVolta(){

    int index, cont=0;
    int *ordem = malloc(n_ciclistas*sizeof(int));
    int *ordem_ver= malloc(n_ciclistas*sizeof(int));
    for(int i = 0; i < n_ciclistas; i++) ordem_ver[i]=0;
    
    srand(time(NULL));

    for(int i = 0; i < n_ciclistas; i++){
        index = rand() % n_ciclistas;
        while(ordem_ver[index]==1) index = rand() % n_ciclistas;
        ordem[i] = index;
        ordem_ver[index] = 1;

    }
        
    if (n_ciclistas%5 == 0)
        for (int i = n_ciclistas-1; i >= 0; i--)
            pista[(int)((floor((double)n_ciclistas/5))-((floor((double)i/5))+1))][i%5] = ordem[i];
    else
        for (int i = n_ciclistas-1; i >= 0; i--)
            pista[(int)((floor((double)n_ciclistas/5))-(floor((double)i/5)))][i%5] = ordem[i];
    
    free(ordem);
    free(ordem_ver);
}

void printPista(int d){
    fprintf(stderr,"------------------------------- FIM DA PISTA -----------------------------\n");
    for(int i = d-1; i >= 0; i--){
        for (int j = 0; j < 10; j++)
            fprintf(stderr,"%d\t",pista[i][j]);
        fprintf(stderr,"\n");
    }
    fprintf(stderr,"----------------------------- COMECO DA PISTA ----------------------------\n");

}

int main(int argc, char *argv[]){
        int *arg, i, j, d = atoi(argv[1]);
        n_ciclistas = atoi(argv[2]);
        pthread_t tid[n_ciclistas];
        pthread_mutex_init(&mutex, NULL);

        pista = malloc(d*sizeof(int*));
        for (i = 0; i < d; i++){
            pista[i] = malloc(10*sizeof(int));
            for (j = 0; j < 10; j++) pista[i][j] = -1;
        }

        primeiraVolta();

        pthread_mutex_lock(&mutex);
        arg = malloc(n_ciclistas*sizeof(int));
        for (i = 0; i < d; i++){
            if (pista[i][0] == -1) break;
            for (j = 0; j < 10; j++){
                if (pista[i][j] != -1){
                    arg[pista[i][j]] = pista[i][j];
                    if (pthread_create(&tid[pista[i][j]], NULL, ciclistas, &arg[pista[i][j]])) {
                        printf("\n ERROR creating thread n:%d",i);
                        exit(1);
                    }
                    if (pthread_join(tid[pista[i][j]], NULL))  {
                        printf("\n ERROR joining thread");
                        exit(1);
                    }
                }
            }
        }

        printPista(d);
            


        return 0;
}
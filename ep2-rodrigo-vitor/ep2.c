#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

#define VELOCIDADE_SIMULACAO 1

int **pista;
pthread_mutex_t mutex;

void * ciclistas(void * arg){ 
    int volta_atual = 0, vel_met=30, n = *((int *)arg);
    // printf("%d",n);
    pthread_mutex_lock(&mutex);
    pthread_mutex_unlock(&mutex);
    return arg;
}

void primeiraVolta(int n){

    int index, cont=0;
    int *ordem = malloc(n*sizeof(int));
    int *ordem_ver= malloc(n*sizeof(int));
    for(int i = 0; i < n; i++) ordem_ver[i]=0;
    
    srand(time(NULL));

    for(int i = 0; i < n; i++){
        index = rand() % n;
        while(ordem_ver[index]==1) index = rand() % n;
        ordem[i] = index;
        ordem_ver[index] = 1;

    }
        
    if (n%5 == 0)
        for (int i = n-1; i >= 0; i--)
            pista[(int)((floor((double)n/5))-((floor((double)i/5))+1))][i%5] = ordem[i];
    else
        for (int i = n-1; i >= 0; i--)
            pista[(int)((floor((double)n/5))-(floor((double)i/5)))][i%5] = ordem[i];
    
    free(ordem);
    free(ordem_ver);
}

void printPista(int n, int d){
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
        int n = atof(argv[2]);
        pthread_t tid[n];
        pthread_mutex_init(&mutex, NULL);

        pista = malloc(d*sizeof(int*));
        for (i = 0; i < d; i++){
            pista[i] = malloc(10*sizeof(int));
            for (j = 0; j < 10; j++) pista[i][j] = -1;
        }

        pthread_mutex_lock(&mutex);
        arg = malloc(n*sizeof(int));
        for (i = 0; i < n; i++){
            arg[i] = i;
            if (pthread_create(&tid[i], NULL, ciclistas, &arg[i])) {
                printf("\n ERROR creating thread n:%d",i);
                exit(1);
            }
        }

        primeiraVolta(n);

        printPista(n,d);
            


        return 0;
}
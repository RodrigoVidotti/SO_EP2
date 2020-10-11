#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

// 1 - Tempo real, 2 - Metade do tempo real, etc.
#define VELOCIDADE_SIMULACAO 2

// Flag para debug
#define DEBUG 1

// Variáveis globais
int **pista;
int *completaram_volta;
int n_ciclistas;
int tamanho_pista;
int ciclista_90 = 0;
char info_extra[20000];

typedef struct{
    int n;
    int pos_i;
    int pos_j;
} args_struct;

pthread_mutex_t mutex;

void printPista(){
    if (DEBUG){
        fprintf(stderr,"---------------------------- INFORMACOES EXTRAS --------------------------\n");
        fprintf(stderr,"%s",info_extra);
    }
    fprintf(stderr,"------------------------------- FIM DA PISTA -----------------------------\n");
    for(int i = tamanho_pista-1; i >= 0; i--){
        if (i<10) fprintf(stderr,"%d   |\t",i);
        else if(i<100) fprintf(stderr,"%d  |\t",i);
        else if(i<1000) fprintf(stderr,"%d |\t",i);
        else fprintf(stderr,"%d|\t",i);
        for (int j = 0; j < 10; j++){
            if (pista[i][j] == -1) continue;
            fprintf(stderr,"%d\t",pista[i][j]);
        }
        fprintf(stderr,"\n");
    }
    fprintf(stderr,"----------------------------- COMECO DA PISTA ----------------------------\n\n");
}

void * ciclistas(void * ptr){ 
    args_struct *args = ptr;
    int i, ran, volta_atual = 0, vel_met=120, pos_i = args->pos_i, pos_j = args->pos_j, n = args->n;
    char new_info[100];
    for (;;){
        usleep(vel_met*1000/VELOCIDADE_SIMULACAO);
        // Verifica se ele é o vencedor
        pthread_mutex_lock(&mutex);
        if (n_ciclistas == 1){
            n_ciclistas--;
            fprintf (stderr,"Acabou! O vencedor é o ciclista %d!\n",n);
            exit(0);
        }
        pthread_mutex_unlock(&mutex);
        // Se ele vai completar uma volta
        if (pos_i+1 == tamanho_pista){
            pthread_mutex_lock(&mutex);
            completaram_volta[volta_atual]++;
            if (completaram_volta[volta_atual] == n_ciclistas){
                if (!DEBUG) {
                    fprintf(stderr,"--------------- Todos os ciclistas completaram a volta %d ! --------------\n",volta_atual);
                    printPista();
                }
                if (volta_atual % 2 != 0){
                    // Esse é o último da fila e deve ser removido, tambem vamos imprimir as infos de final de volta
                    n_ciclistas--;
                    pista[pos_i][pos_j] = -1;
                    pthread_mutex_unlock(&mutex);
                    return ptr;
                }
            }
            pthread_mutex_unlock(&mutex);

            volta_atual++;

            pthread_mutex_lock(&mutex);
            for (i = 0; i < 10; i++){
                if (pista[0][i] == -1){
                    pista[pos_i][pos_j] = -1;
                    pista[0][i] = n;
                    pos_i = 0;
                    pos_j = i;
                    break;
                }
            }

            if (pos_j >= 1 && pista[pos_i][pos_j-1] == -1){
                pista[pos_i][pos_j-1] = n;
                pista[pos_i][pos_j] = -1;
                pos_j--;
            }
            pthread_mutex_unlock(&mutex);
            // Esse não é o último da fila, sua velocidade deve ser ajustada e deve-se verificar se ele quebrará
            if (volta_atual % 6 == 0){
                ran = rand() % 100;
                if (ran < 5  && n_ciclistas >= 5){
                    sprintf(new_info,"O ciclista %d quebrou! Ele estava na volta %d\n", n, volta_atual);
                    if (!DEBUG) fprintf(stderr,"%s",new_info);
                    else strcat(info_extra,new_info);
                    pthread_mutex_lock(&mutex);
                    n_ciclistas--;
                    pista[pos_i][pos_j] = -1;
                    pthread_mutex_unlock(&mutex);
                    return ptr;
                }
            }
            if (vel_met == 120){
                ran = rand() % 10;
                if (ran < 8) vel_met = 60;
            }else if (vel_met == 60){
                ran = rand() % 10;
                if (ran < 4) vel_met = 120;
            }
            pthread_mutex_lock(&mutex);
            if (n_ciclistas == 2){
                ran = rand() % 100;
                if (ran < 5 && ciclista_90 == 0){
                    vel_met = 40;
                    ciclista_90 = 1;
                }
            }
            pthread_mutex_unlock(&mutex);
        }else{
            // fprintf(stderr,"%d vai avancar um passo\n",n);
            pthread_mutex_lock(&mutex);
            for (i = pos_j; i < 10; i++){
                if (pista[pos_i+1][i] == -1){
                    pista[pos_i][pos_j] = -1;
                    pista[pos_i+1][i] = n;
                    pos_i = pos_i+1;
                    pos_j = i;
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);
        }
    }
    return ptr;
}

void primeiraVolta(){

    int index;
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

int main(int argc, char *argv[]){
        int i, j;
        tamanho_pista = atoi(argv[1]);
        n_ciclistas = atoi(argv[2]);
        completaram_volta = malloc((n_ciclistas+10)*sizeof(int));
        for(i = 0; i < n_ciclistas+10; i++) completaram_volta[i] = 0;
        pthread_t tid[n_ciclistas];
        pthread_mutex_init(&mutex, NULL);

        pista = malloc(tamanho_pista*sizeof(int*));
        for (i = 0; i < tamanho_pista; i++){
            pista[i] = malloc(10*sizeof(int));
            for (j = 0; j < 10; j++) pista[i][j] = -1;
        }

        primeiraVolta();

        pthread_mutex_lock(&mutex);
        
        
        args_struct *args = malloc(n_ciclistas*sizeof(args_struct));
        for (i = 0; i < tamanho_pista; i++){
            if (pista[i][0] == -1) break;
            for (j = 0; j < 10; j++){
                if (pista[i][j] != -1){
                    args[pista[i][j]].n = pista[i][j];
                    args[pista[i][j]].pos_i = i;
                    args[pista[i][j]].pos_j = j;
                    if (pthread_create(&tid[pista[i][j]], NULL, ciclistas, (void *)&args[pista[i][j]])) {
                        fprintf(stderr,"\n ERROR creating thread n:%d",i);
                        exit(1);
                    }
                }
            }
        }

        pthread_mutex_unlock(&mutex);

        fprintf(stderr,"\033c");
        for (;;){
            if (DEBUG) printPista();
            pthread_mutex_lock(&mutex);
            if (n_ciclistas == 0) break;
            pthread_mutex_unlock(&mutex);
            if (ciclista_90) usleep(20*1000/VELOCIDADE_SIMULACAO);
            else usleep(60*1000/VELOCIDADE_SIMULACAO);
            if (DEBUG) fprintf(stderr,"\033c");
        }

        pthread_mutex_destroy(&mutex);

        return 0;
}
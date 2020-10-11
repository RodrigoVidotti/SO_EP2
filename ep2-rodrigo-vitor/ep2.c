#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

// 1 - Tempo real, 2 - Metade do tempo real, etc.
#define VELOCIDADE_SIMULACAO 1

// Flag para debug
#define DEBUG 1

// Variáveis globais
int **pista;
int *completaram_volta;
int n_ciclistas;
int tamanho_pista;
int ciclista_90 = 0;
int *quebraram;
int *tempo_ou_posicao;
int *placar;

typedef struct{
    char *ptr;
    size_t size;
} inf_ext;

inf_ext *info_extra;

typedef struct{
    int n;
    int pos_i;
    int pos_j;
} args_struct;

pthread_mutex_t mutex;

void printPista(){
    if (DEBUG){
        fprintf(stderr,"--------------------------- QUADRO DE INFORMACES -------------------------\n");
        fprintf(stderr,"%s",info_extra->ptr);
    }
    fprintf(stderr,    "------------------------------- FIM DA PISTA -----------------------------\n");
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

void addToExraInfo(char *str){
    char *temp;
    printf("Entrada : %s\n",info_extra->ptr);
    if ((strlen(info_extra->ptr) + strlen(str)) >= info_extra->size){
        printf("Vai alocar %lu memória\n",info_extra->size*2);
        info_extra->size = info_extra->size*2;
        temp = malloc(info_extra->size*2*sizeof(char));
        strcpy(temp,info_extra->ptr);
        strcat(temp,str);
        free(info_extra->ptr);
        info_extra->ptr = temp;
    }else{
        strcat(info_extra->ptr,str);
    }
    printf("Saida : %s\n\n",info_extra->ptr);
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
            placar[n_ciclistas] = n;
            tempo_ou_posicao[n] = volta_atual;
            sprintf(new_info,"Acabou! O vencedor é o ciclista %d!!\n", n);
            if (!DEBUG) fprintf(stderr,"%s",new_info);
            else addToExraInfo(new_info);
            pthread_mutex_unlock(&mutex);
            return ptr;
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
                    placar[n_ciclistas] = n;
                    tempo_ou_posicao[n] = volta_atual;
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
                    sprintf(new_info,"O ciclista %d quebrou! Ele estava na volta %d.\n", n, volta_atual);
                    if (!DEBUG) fprintf(stderr,"%s",new_info);
                    else addToExraInfo(new_info);
                    pthread_mutex_lock(&mutex);
                    quebraram[n] = 1;
                    // Trocar 
                    tempo_ou_posicao[n] = volta_atual;
                    n_ciclistas--;
                    placar[n_ciclistas] = n;
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

void posicionamentoInicial(){

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
        // Declaracão de variáveis e alocacões das globais
        int i, j;
        tamanho_pista = atoi(argv[1]);
        int aux = atoi(argv[2]);
        n_ciclistas = atoi(argv[2]);  
        pthread_t tid[n_ciclistas];
        pthread_mutex_init(&mutex, NULL);
        info_extra = malloc(sizeof(inf_ext));
        // Alocacão de memória

        info_extra->ptr = malloc(30*sizeof(char));
        info_extra->size = 30;


        pista = malloc(tamanho_pista*sizeof(int*));
        completaram_volta = malloc(n_ciclistas*2*sizeof(int));
        quebraram = malloc(n_ciclistas*sizeof(int));
        tempo_ou_posicao = malloc(n_ciclistas*sizeof(int));
        placar = malloc(n_ciclistas*sizeof(int));

        // Inicializacão dos valores das variáveis
        for(i = 0; i < n_ciclistas*2; i++){
            completaram_volta[i] = 0;
            quebraram[i] = 0;
            tempo_ou_posicao[i] = 0;
        }
        for (i = 0; i < tamanho_pista; i++){
            pista[i] = malloc(10*sizeof(int));
            for (j = 0; j < 10; j++) pista[i][j] = -1;
        }

        // Posiciona todos os ciclistas na pista de forma aleatória
        // e cada posicão vazia recebe -1
        posicionamentoInicial();
        pthread_mutex_lock(&mutex);

        // Vetor de structs de argumentos. É passado para cada ciclista:
        //      n: Seu identificador
        //      pos_i: Qual metro da pista ele se encontra inicialmente
        //      pos_j: Qual das 10 posicões daquele metro ele se econcontra inicialmente
        args_struct *args = malloc(n_ciclistas*sizeof(args_struct));

        // Cria threads para todos os ciclistas (inicialmente travadas)
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


        // char new_info[100];
        // sprintf(new_info,"Acabou! O vencedor é o ciclista 3!!");
        // // printf("%s",new_info);
        // addToExraInfo(new_info);
        // addToExraInfo(new_info);
        // sprintf(new_info,"entrada 3");
        // addToExraInfo(new_info);
        // printf("%s",new_info);
        // addToExraInfo(new_info);



        // Comeca a corrida!
        pthread_mutex_unlock(&mutex);

        // Saída das informacões da simulacão
        if (DEBUG) fprintf(stderr,"\033c");
        for (;;){
            if (DEBUG) printPista();
            pthread_mutex_lock(&mutex);
            if (n_ciclistas == 0) break;
            pthread_mutex_unlock(&mutex);
            if (ciclista_90) usleep(20*1000/VELOCIDADE_SIMULACAO);
            else usleep(60*1000/VELOCIDADE_SIMULACAO);
            if (DEBUG) fprintf(stderr,"\033c");
        }

        fprintf(stderr,"--------------------------------- PLACAR ---------------------------------\n");
        int cont=0;
        for (i = 0; i < aux; i++){
            if (!quebraram[i]){
                if (i+1-cont < 10){
                    if (placar[i] < 10) fprintf(stderr,"%d°  Lugar:   %d   | Ele(a) completou sua última volta aos %d segundos. \n",i+1-cont,placar[i],tempo_ou_posicao[i-cont]);
                    else if (placar[i] < 100 ) fprintf(stderr,"%d°  Lugar:   %d  | Ele(a) completou sua última volta aos %d segundos. \n",i+1-cont,placar[i],tempo_ou_posicao[i-cont]);
                    else fprintf(stderr,"%d°  Lugar:   %d | Ele(a) completou sua última volta aos %d segundos. \n",i+1-cont,placar[i],tempo_ou_posicao[i-cont]);
                }
                else if (i+1-cont < 100){
                    if (placar[i] < 10) fprintf(stderr,"%d° Lugar:   %d   | Ele(a) completou sua última volta aos %d segundos. \n",i+1-cont,placar[i],tempo_ou_posicao[i-cont]);
                    else if (placar[i] < 100) fprintf(stderr,"%d° Lugar:   %d  | Ele(a) completou sua última volta aos %d segundos. \n",i+1-cont,placar[i],tempo_ou_posicao[i-cont]);
                    else fprintf(stderr,"%d° Lugar:   %d | Ele(a) completou sua última volta aos %d segundos. \n",i+1-cont,placar[i],tempo_ou_posicao[i-cont]);
                }else{
                    if (placar[i] < 10) fprintf(stderr,"%d°Lugar:   %d   | Ele(a) completou sua última volta aos %d segundos. \n",i+1-cont,placar[i],tempo_ou_posicao[i-cont]);
                    else if (placar[i] < 100) fprintf(stderr,"%d°Lugar:   %d  | Ele(a) completou sua última volta aos %d segundos. \n",i+1-cont,placar[i],tempo_ou_posicao[i-cont]);
                    else fprintf(stderr,"%d°Lugar:   %d | Ele(a) completou sua última volta aos %d segundos. \n",i+1-cont,placar[i],tempo_ou_posicao[i-cont]);
                }
                
            }
            else cont++;
        }
        fprintf(stderr,"\n");
        for (i = 0; i < aux; i++){
            if (quebraram[i])
                fprintf(stderr,"O ciclista %d quebrou enquanto completava sua volta número %d.\n",i,tempo_ou_posicao[i]);         
        }
        fprintf(stderr,"---------------------------- FIM DA SIMULACAO ----------------------------\n");

        // Finalizacão do simulador
        pthread_mutex_destroy(&mutex);
        for (i = 0; i < tamanho_pista; i++) free(pista[i]);
        free(pista);
        free(completaram_volta);
        free(quebraram);

        return 0;
}
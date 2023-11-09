#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "matriz.h"

#define H_MATRIZ 10
#define W_MATRIZ 10

#define SEED_RAND 22
#define MAX_RAND 100

#define NUM_THREADS 2

#define H_MACROBLOCO 5;
#define W_MACROBLOCO 5;

// Variaveis globais
int totalPrimos = 0;
int **mat;
pthread_mutex_t regiaoCritica1;
pthread_mutex_t regiaoCritica2;
int linha = 0;
int coluna = 0;

void* buscaParalela(void*);

// cria e aloca matriz dinamicamente utilizando parametros de altura e largura fixos
int** criaMatriz(){
    srand(SEED_RAND);

    int** mat = (int**) malloc(H_MATRIZ * W_MATRIZ * sizeof(int **));

    for(int i = 0; i < H_MATRIZ; i++) {
        mat[i] = (int *) malloc(W_MATRIZ * sizeof(int *));
        for(int j = 0; j < W_MATRIZ; j++) {
            int num = rand() % MAX_RAND;
            mat[i][j] = num;
        }
    }
    return mat;
}

// libera a matriz da memoria
void liberaMat(int** mat) {
    for(int i = 0; i < H_MATRIZ; i++){
        free(mat[i]);
    }
    free(mat);
}

// verifica se o numero é primo
int ehPrimo(int n) {
    if( n <= 1) return 0;
    double raiz = sqrt(n);

    for(int i = 2; i <= raiz; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

// imprime os valores da matriz
void imprimeMatriz(int **mat) {
    for(int i = 0; i < H_MATRIZ; i++) {
        printf("[");
        for(int j = 0; j < W_MATRIZ; j++){
            printf(" %d ", mat[i][j]);
        }
        printf("]\n");
    }
}

// contagem dos primos realizada um a um
int buscaSerial(int** mat){
    int soma = 0;
    for(int i = 0; i < H_MATRIZ; i++) {
        for(int j = 0; j < W_MATRIZ; j++){
            if(ehPrimo(mat[i][j])) soma += 1;
        }
    }
    return soma;
}

// calcula o tempo de execucao de um trecho de codigo dado o tempo inicial e final
double tempoExecucao(clock_t tempInicial, clock_t tempFinal) {
    return (tempFinal - tempInicial) / (double)CLOCKS_PER_SEC;
}

int contaPrimos(int linhaInicial, int colunaInicial) {
    int soma = 0;
    int linhaMax = linhaInicial + H_MACROBLOCO;
    int colunaMax = colunaInicial + W_MACROBLOCO;

    for(int i = linhaInicial; i < linhaMax ; i++) {

        for(int j = colunaInicial; j < colunaMax; j++) {
            if(ehPrimo(mat[i][j])) soma += 1;
        }
    }

    return soma;
}


void* buscaParalela(void* nenhum) {
    int soma = 0;

    // a logica do while ta errada
    while(linha < H_MATRIZ && coluna < W_MATRIZ) {

        soma += contaPrimos(linha, coluna);

        //pthread_mutex_lock(&regiaoCritica1);
        linha += H_MACROBLOCO;
        coluna += W_MACROBLOCO;
        //thread_mutex_unlock(&regiaoCritica1);

    }

    pthread_mutex_lock(&regiaoCritica2);
    totalPrimos += soma;
    pthread_mutex_unlock(&regiaoCritica2);
}

int main(int argc, char *argv[]) {
    clock_t tempInicial, tempFinal;
    double tempTotal;
    mat = criaMatriz();

    //imprimeMatriz(mat);

    // Busca Serial
    tempInicial = clock();
    totalPrimos += buscaSerial(mat);
    tempFinal = clock();
    //imprimeMatriz(mat);
    printf("\nBUSCA SERIAL \nQtd de primos: %d \nTEMPO TOTAL: %f s", totalPrimos, tempoExecucao(tempInicial, tempFinal));

    // zera a contagem
    totalPrimos = 0;

    tempInicial = clock();

    // cria as threads
    pthread_t workers[NUM_THREADS];

    pthread_mutex_init(&regiaoCritica1, NULL);
    pthread_mutex_init(&regiaoCritica2, NULL);

    for(int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&workers[i], NULL, buscaParalela, argv[1]);
    }

    for(int j = 0; j < NUM_THREADS; j++) {
        pthread_join(workers[j], NULL);
    }

    pthread_mutex_destroy(&regiaoCritica1);
    pthread_mutex_destroy(&regiaoCritica2);

    tempFinal = clock();
    printf("\n\nMULTITHREAD \nQtd de primos: %d\nTEMPO TOTAL: %f s", totalPrimos, tempoExecucao(tempInicial, tempFinal));

    liberaMat(mat);
    return 0;
}

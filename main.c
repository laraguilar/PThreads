#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#define	_WINSOCK_DEPRACATED_NO_WARNINGS 1

#pragma comment(lib,"pthreadVC2.lib")
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define H_MATRIZ 12000
#define W_MATRIZ 12000

#define SEED_RAND 22
#define MAX_RAND 31999

#define NUM_THREADS 12

#define H_MACROBLOCO 1000
#define W_MACROBLOCO 1000

// Variaveis globais
int totalPrimos = 0;
int** mat;
pthread_mutex_t regiaoCritica1;
pthread_mutex_t regiaoCritica2;

int linha = 0;
int coluna = 0;

pthread_t workers[NUM_THREADS];


void* buscaParalela(void*);

// cria e aloca matriz dinamicamente utilizando parametros de altura e largura fixos
void criaMatriz() {
    srand(SEED_RAND);

    mat = (int**)malloc(H_MATRIZ * W_MATRIZ * sizeof(int**));

        for (int i = 0; i < H_MATRIZ; i++) {
            mat[i] = (int*)malloc(W_MATRIZ * sizeof(int*));
            for (int j = 0; j < W_MATRIZ; j++) {
                int num = rand() % MAX_RAND;
                mat[i][j] = num;
            }
        }
   
}

// libera a matriz da memoria
void liberaMat(int** mat) {
    for (int i = 0; i < H_MATRIZ; i++) {
        free(mat[i]);
    }
    free(mat);
}

// verifica se o numero   primo
int ehPrimo(int n) {
    if (n <= 1) return 0;
    double raiz = sqrt(n);

    for (int i = 2; i <= raiz; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

// imprime os valores da matriz
void imprimeMatriz() {
    for (int i = 0; i < H_MATRIZ; i++) {
        printf("[");
        for (int j = 0; j < W_MATRIZ; j++) {
            printf(" %d ", mat[i][j]);
        }
        printf("]\n");
    }
}


// contagem dos primos realizada um a um
int buscaSerial(int** mat) {
    int soma = 0;
    for (int i = 0; i < H_MATRIZ; i++) {
        for (int j = 0; j < W_MATRIZ; j++) {
            if (ehPrimo(mat[i][j])) soma += 1;
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

    for (int i = linhaInicial; i < linhaMax; i++) {
        for (int j = colunaInicial; j < colunaMax; j++) {
            if (ehPrimo(mat[i][j]))
            {
                soma += 1;
            }
        }
    }
    return soma;
}



void* buscaParalela(void* nenhum) {
    int soma = 0;
    int i;
    int j;

    while (linha < H_MATRIZ && coluna < W_MATRIZ) {
        pthread_mutex_lock(&regiaoCritica1);
        i = linha;
        j = coluna;

        // define o valor da coluna para a proxima thread
        coluna += W_MACROBLOCO;

        // verifica se j  chegou a ultima coluna da matriz e pula pra proxima linha de macroblocos
        int cond1 = W_MATRIZ - W_MACROBLOCO;
        if (coluna > cond1) {
            // define o valor de linha e coluna para a proxima thread
            linha += H_MACROBLOCO;
            coluna = 0;
        }
        pthread_mutex_unlock(&regiaoCritica1);

        soma += contaPrimos(i, j);
    }

    pthread_mutex_lock(&regiaoCritica2);
    totalPrimos += soma;
    pthread_mutex_unlock(&regiaoCritica2);
}

int main(int argc, char* argv[]) {
    clock_t tempInicial, tempFinal;
    criaMatriz();

    if (mat == NULL) return 0;

    // Busca Serial
    tempInicial = clock();
    totalPrimos += buscaSerial(mat);
    tempFinal = clock();

    double tempo1 = tempoExecucao(tempInicial, tempFinal);

    printf("\nBUSCA SERIAL \nQtd de primos: %d \nTEMPO TOTAL: %.2f s", totalPrimos, tempo1);

    // zera a contagem
    totalPrimos = 0;

    tempInicial = clock();

    // cria as threads

    pthread_mutex_init(&regiaoCritica1, NULL);
    pthread_mutex_init(&regiaoCritica2, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&workers[i], NULL, buscaParalela, argv[1]);
    }

    for (int j = 0; j < NUM_THREADS; j++) {
        pthread_join(workers[j], NULL);
    }

    pthread_mutex_destroy(&regiaoCritica1);
    pthread_mutex_destroy(&regiaoCritica2);

    tempFinal = clock();

    double tempo2 = tempoExecucao(tempInicial, tempFinal);

    printf("\n\nMULTITHREAD: %d nucleos\nQtd de primos: %d\nTEMPO TOTAL: %.2f s", NUM_THREADS, totalPrimos, tempo2);


    printf("\n\nSPEEDUP: %2f", tempo1 / tempo2);
    liberaMat(mat);
    return 0;
}

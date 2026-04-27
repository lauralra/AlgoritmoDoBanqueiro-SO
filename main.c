#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

// Estruturas de Dados do Banqueiro
int available[NUMBER_OF_RESOURCES];
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

// Mutex para evitar condições de corrida
pthread_mutex_t mutex;

// Protótipos
bool is_safe();
int request_resources(int customer_num, int request[]);
int release_resources(int customer_num, int release[]);
void* customer_behavior(void* customer_num);

int main(int argc, char *argv[]) {
    if (argc != NUMBER_OF_RESOURCES + 1) {
        printf("Erro: Forneça %d instâncias iniciais.\n", NUMBER_OF_RESOURCES);
        return -1;
    }

    // Inicialização do sistema
    pthread_mutex_init(&mutex, NULL);
    srand(time(NULL));

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] = atoi(argv[i + 1]);
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            maximum[i][j] = rand() % (available[j] + 1); // Demanda aleatória
            allocation[i][j] = 0;
            need[i][j] = maximum[i][j];
        }
    }

    // Criação dos Threads Clientes
    pthread_t threads[NUMBER_OF_CUSTOMERS];
    int customer_ids[NUMBER_OF_CUSTOMERS];

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        customer_ids[i] = i;
        pthread_create(&threads[i], NULL, customer_behavior, &customer_ids[i]);
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}

// Implementação da Verificação de Estado Seguro
bool is_safe() {
    int work[NUMBER_OF_RESOURCES];
    bool finish[NUMBER_OF_CUSTOMERS] = {false};

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) work[i] = available[i];

    for (int count = 0; count < NUMBER_OF_CUSTOMERS; count++) {
        bool found = false;
        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
            if (!finish[i]) {
                int j;
                for (j = 0; j < NUMBER_OF_RESOURCES; j++)
                    if (need[i][j] > work[j]) break;

                if (j == NUMBER_OF_RESOURCES) { // Se o cliente i pode finalizar
                    for (int k = 0; k < NUMBER_OF_RESOURCES; k++)
                        work[k] += allocation[i][k];
                    finish[i] = true;
                    found = true;
                }
            }
        }
        if (!found) break;
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
        if (!finish[i]) return false;
    return true;
}

int request_resources(int customer_num, int request[]) {
    pthread_mutex_lock(&mutex);

    // 1. Verificar se pedido > necessidade ou disponível imediato
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > need[customer_num][i] || request[i] > available[i]) {
            pthread_mutex_unlock(&mutex);
            return -1;
        }
    }

    // 2. Simulação de alocação
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] -= request[i];
        allocation[customer_num][i] += request[i];
        need[customer_num][i] -= request[i];
    }

    // 3. Checar segurança
    if (is_safe()) {
        printf("Cliente %d: Recursos concedidos.\n", customer_num);
        pthread_mutex_unlock(&mutex);
        return 0;
    } else {
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            available[i] += request[i];
            allocation[customer_num][i] -= request[i];
            need[customer_num][i] += request[i];
        }
        printf("Cliente %d: Requisição negada (Estado Inseguro).\n", customer_num);
        pthread_mutex_unlock(&mutex);
        return -1;
    }
}

int release_resources(int customer_num, int release[]) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] += release[i];
        allocation[customer_num][i] -= release[i];
        need[customer_num][i] += release[i];
    }
    printf("Cliente %d: Recursos liberados.\n", customer_num);
    pthread_mutex_unlock(&mutex);
    return 0;
}

void* customer_behavior(void* arg) {
    int id = *(int*)arg;
    int request[NUMBER_OF_RESOURCES];
    
    for (int i = 0; i < 5; i++) { // Executa 5 ciclos de pedido/liberação
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
            request[j] = rand() % (need[id][j] + 1);

        request_resources(id, request);
        sleep(1); 
        release_resources(id, request);
        sleep(1);
    }
    return NULL;
}
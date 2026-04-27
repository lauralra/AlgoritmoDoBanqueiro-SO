#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

/* estes podem ser quaisquer valores >= 0 */
    /* o montante disponível de cada recurso */
    int available[NUMBER_OF_RESOURCES];
    /* a demanda máxima de cada cliente */
    int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
    /* o montante correntemente alocado a cada cliente */
    int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
    /* a necessidade remanescente de cada cliente */
    int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

    pthread_mutex_t lock;

int main(int argc, char *argv[]){

    pthread_t threads[NUMBER_OF_CUSTOMERS];
    int customer_ids[NUMBER_OF_CUSTOMERS];

    if (argc < NUMBER_OF_RESOURCES + 1) {
        printf("Erro: Forneça a quantidade de cada recurso.\n");
        return -1;
    }

    /* 1. Inicializa o array available com os argumentos da linha de comando [cite: 38, 41] */
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] = atoi(argv[i + 1]);
    }

    pthread_mutex_init(&lock, NULL); // Inicializa o mutex antes de utilizá-lo

    pthread_mutex_destroy(&lock);


    return 0;
}

 bool is_safe()
    {
        int work[NUMBER_OF_RESOURCES];
        bool finish[NUMBER_OF_CUSTOMERS] = {false};

        // Inicializa Work = Available
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
            work[i] = available[i];

        for (int count = 0; count < NUMBER_OF_CUSTOMERS; count++)
        {
            bool found = false;
            for (int p = 0; p < NUMBER_OF_CUSTOMERS; p++)
            {
                if (!finish[p])
                {
                    bool can_allocate = true;
                    for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
                    {
                        if (need[p][j] > work[j])
                        {
                            can_allocate = false;
                            break;
                        }
                    }

                    if (can_allocate)
                    {
                        for (int j = 0; j < NUMBER_OF_RESOURCES; j++)
                            work[j] += allocation[p][j];
                        finish[p] = true;
                        found = true;
                    }
                }
            }
            if (!found)
                break; // Se não encontrou nenhum cliente que pode terminar, o estado é inseguro
        }

        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++)
            if (!finish[i])
                return false;

        return true;
    }


int request_resources(int customer_num, int request[])
    {
        pthread_mutex_lock(&lock); // Entra na seção crítica

        // 1. Verifica se o pedido é maior que a necessidade ou o que está disponível
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
        {
            if (request[i] > need[customer_num][i] || request[i] > available[i])
            {
                pthread_mutex_unlock(&lock);
                return -1; // Pedido inválido ou recursos insuficientes
            }
        }

        // 2. Simulação: Tenta alocar os recursos
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
        {
            available[i] -= request[i];
            allocation[customer_num][i] += request[i];
            need[customer_num][i] -= request[i];
        }

        // 3. Verifica se o estado resultante é seguro
        if (is_safe())
        {
            pthread_mutex_unlock(&lock);
            return 0; // Sucesso
        }
        else
        {
            // Rollback: desfazer a alocação se for inseguro
            for (int i = 0; i < NUMBER_OF_RESOURCES; i++)
            {
                available[i] += request[i];
                allocation[customer_num][i] -= request[i];
                need[customer_num][i] += request[i];
            }
            pthread_mutex_unlock(&lock);
            return -1; // Estado inseguro, pedido negado
        }
    }

    int release_resources(int customer_num, int release[]) {
    pthread_mutex_lock(&lock);
    // ... (lógica para devolver recursos) ...
    pthread_mutex_unlock(&lock);
    return 0;
}

void* cliente_thread(void* arg) {
    int customer_num = *(int*)arg;

    while (true) {
        int request[NUMBER_OF_RESOURCES];

        // 1. Gera uma solicitação aleatória baseada no que o cliente ainda precisa
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            if (need[customer_num][i] > 0) {
                request[i] = rand() % (need[customer_num][i] + 1);
            } else {
                request[i] = 0;
            }
        }

        printf("Cliente %d solicitando recursos...\n", customer_num);
        if (request_resources(customer_num, request) == 0) {
            printf("Cliente %d: Pedido concedido!\n", customer_num);
            
            // Simula o uso dos recursos
            sleep(rand() % 3 + 1); 

            // 2. Libera os recursos após o uso
            release_resources(customer_num, request);
            printf("Cliente %d: Recursos liberados.\n", customer_num);
        } else {
            printf("Cliente %d: Pedido negado (estado inseguro ou falta de recursos).\n", customer_num);
        }

        sleep(rand() % 3 + 1); // Espera um pouco antes da próxima solicitação
    }
    return NULL;
}

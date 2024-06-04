#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/select.h>

#define BUFSZ 1024

typedef struct {
    int socket;  // Socket do cliente
    struct sockaddr_storage client_addr;  // Endereço do cliente
    socklen_t client_addr_len;  // Tamanho do endereço do cliente
    int escolha;  // Filme escolhido pelo cliente
    int frase;  // Índice da frase atual do filme
} client_info;

char *frases_senhor_aneis[] = {
    "Um anel para todos governar",
    "Na terra de Mordor onde as sombras se deitam",
    "Não é o que temos, mas o que fazemos com o que temos",
    "Não há mal que sempre dure",
    "O mundo está mudando, senhor Frodo"
};

char *frases_poderoso_chefao[] = {
    "Vou fazer uma oferta que ele não pode recusar",
    "Mantenha seus amigos por perto e seus inimigos mais perto ainda",
    "É melhor ser temido que amado",
    "A vingança é um prato que se come frio",
    "Nunca deixe que ninguém saiba o que você está pensando"
};

char *frases_clube_luta[] = {
    "Primeira regra do Clube da Luta: você não fala sobre o Clube da Luta",
    "Segunda regra do Clube da Luta: você não fala sobre o Clube da Luta",
    "O que você possui acabará possuindo você",
    "É apenas depois de perder tudo que somos livres para fazer qualquer coisa",
    "Escolha suas lutas com sabedoria"
};

client_info *clients[FD_SETSIZE];  // Lista de clientes conectados
int client_count = 0;  // Contador de clientes conectados
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex para proteger a lista de clientes

void logexit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void *client_handler(void *data) {
    client_info *client = (client_info *)data;

    char buf[BUFSZ];
    int n;
    struct sockaddr_storage client_addr = client->client_addr;
    socklen_t client_addr_len = client->client_addr_len;

    char **frases;
    switch (client->escolha) {
        case 1:
            frases = frases_senhor_aneis;
            break;
        case 2:
            frases = frases_poderoso_chefao;
            break;
        case 3:
            frases = frases_clube_luta;
            break;
        default:
            frases = NULL;
            break;
    }

    while (client->frase < 5) {
        snprintf(buf, BUFSZ, "%s", frases[client->frase]);
        // Envia a frase selecionada para o cliente
        n = sendto(client->socket, buf, strlen(buf) + 1, 0, (struct sockaddr *)&client_addr, client_addr_len);
        if (n == -1) {
            logexit("sendto");
        }
        client->frase++;
        sleep(3);
    }

    close(client->socket);
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] == client) {
            for (int j = i; j < client_count - 1; ++j) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);
    free(client);

    return NULL;
}

void *display_client_count() {
    while (1) {
        sleep(4);
        pthread_mutex_lock(&client_mutex);
        // Exibe a quantidade de clientes conectados
        printf("Clientes conectados: %d\n", client_count);
        pthread_mutex_unlock(&client_mutex);
    }
    return NULL;
}

void usage() {
    printf("usage: server <ipv4|ipv6> <port>\n");
    printf("example: server ipv4 50501\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage();
    }

    int s;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;
    struct sockaddr_in addr4;
    struct sockaddr_in6 addr6;
    int addr_len;

    // Criação do socket UDP para IPv4 ou IPv6
    if (strcmp(argv[1], "ipv4") == 0) {
        s = socket(AF_INET, SOCK_DGRAM, 0);  // Cria o socket UDP para IPv4
        if (s == -1) {
            logexit("socket");
        }
        memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family = AF_INET;
        addr4.sin_port = htons(atoi(argv[2]));  // Define a porta do servidor
        addr4.sin_addr.s_addr = INADDR_ANY;  // Permite que o servidor receba conexões de qualquer endereço IP
        addr_len = sizeof(addr4);
        if (bind(s, (struct sockaddr *)&addr4, addr_len) == -1) {
            logexit("bind");
        }
    } else if (strcmp(argv[1], "ipv6") == 0) {
        s = socket(AF_INET6, SOCK_DGRAM, 0);  // Cria o socket UDP para IPv6
        if (s == -1) {
            logexit("socket");
        }
        memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_port = htons(atoi(argv[2]));  // Define a porta do servidor
        addr6.sin6_addr = in6addr_any;  // Permite que o servidor receba conexões de qualquer endereço IP
        addr_len = sizeof(addr6);
        if (bind(s, (struct sockaddr *)&addr6, addr_len) == -1) {
            logexit("bind");
        }
    } else {
        usage();
    }

    pthread_t client_count_thread;
    pthread_create(&client_count_thread, NULL, display_client_count, NULL);

    while (1) {
        char buf[BUFSZ];
        client_addr_len = sizeof(client_addr);
        // Recebe a mensagem do cliente
        int n = recvfrom(s, buf, BUFSZ, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (n == -1) {
            logexit("recvfrom");
        }

        client_info *client = malloc(sizeof(client_info));
        client->socket = s;
        client->client_addr = client_addr;
        client->client_addr_len = client_addr_len;
        client->escolha = atoi(buf);  // Converte a mensagem recebida para a escolha do cliente
        client->frase = 0;

        pthread_mutex_lock(&client_mutex);
        clients[client_count++] = client;  // Adiciona o cliente à lista de clientes
        pthread_mutex_unlock(&client_mutex);

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client);  // Cria uma nova thread para gerenciar a conexão do cliente
    }

    close(s);  // Fecha o socket do servidor
    return 0;
}

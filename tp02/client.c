#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFSZ 1024

void logexit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void usage() {
    printf("usage: client <ipv4|ipv6> <server IP> <server port>\n");
    printf("example: client ipv4 127.0.0.1 50501\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 4) {
        usage();
    }

    int s;
    struct sockaddr_storage server_addr;
    socklen_t server_addr_len;
    struct sockaddr_in addr4;
    struct sockaddr_in6 addr6;

    // Criação do socket UDP para IPv4 ou IPv6
    if (strcmp(argv[1], "ipv4") == 0) {
        s = socket(AF_INET, SOCK_DGRAM, 0);  // Cria o socket UDP para IPv4
        if (s == -1) {
            logexit("socket");
        }
        memset(&addr4, 0, sizeof(addr4));
        addr4.sin_family = AF_INET;
        addr4.sin_port = htons(atoi(argv[3]));  // Define a porta do servidor
        if (inet_pton(AF_INET, argv[2], &addr4.sin_addr) <= 0) {
            logexit("inet_pton");  // Converte o endereço IP do servidor
        }
        memcpy(&server_addr, &addr4, sizeof(addr4));
        server_addr_len = sizeof(addr4);
    } else if (strcmp(argv[1], "ipv6") == 0) {
        s = socket(AF_INET6, SOCK_DGRAM, 0);  // Cria o socket UDP para IPv6
        if (s == -1) {
            logexit("socket");
        }
        memset(&addr6, 0, sizeof(addr6));
        addr6.sin6_family = AF_INET6;
        addr6.sin6_port = htons(atoi(argv[3]));  // Define a porta do servidor
        if (inet_pton(AF_INET6, argv[2], &addr6.sin6_addr) <= 0) {
            logexit("inet_pton");  // Converte o endereço IP do servidor
        }
        memcpy(&server_addr, &addr6, sizeof(addr6));
        server_addr_len = sizeof(addr6);
    } else {
        usage();
    }

    while (1) {
        printf("Menu de filmes:\n");
        printf("1) Senhor dos Anéis\n");
        printf("2) O Poderoso Chefão\n");
        printf("3) Clube da Luta\n");
        printf("Escolha um filme (1-3): ");
        int escolha;
        scanf("%d", &escolha);

        if (escolha < 1 || escolha > 3) {
            printf("Escolha inválida\n");
            continue;
        }

        char buf[BUFSZ];
        snprintf(buf, BUFSZ, "%d", escolha);
        // Envia a escolha do filme para o servidor
        int n = sendto(s, buf, strlen(buf) + 1, 0, (struct sockaddr *)&server_addr, server_addr_len);
        if (n == -1) {
            logexit("sendto");
        }

        while (1) {
            // Recebe as frases do filme do servidor
            n = recvfrom(s, buf, BUFSZ, 0, NULL, NULL);
            if (n == -1) {
                logexit("recvfrom");
            }
            printf("Frase: %s\n", buf);
        }
    }

    close(s);  // Fecha o socket do cliente
    return 0;
}

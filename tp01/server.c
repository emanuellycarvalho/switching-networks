#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <math.h>

#define BUFSZ 1024

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void usage() {
  printf("usage: %s <ipv4|ipv6> <port>\n", "server");
  printf("example: %s ipv4 50501\n", "server");
  exit(EXIT_FAILURE);
}

void logexit(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

double haversine(double lat1, double lon1, double lat2, double lon2) {
  double dLat = (lat2 - lat1) * M_PI / 180.0;
  double dLon = (lon2 - lon1) * M_PI / 180.0;

  // convertendo para radianos
  lat1 = (lat1) * M_PI / 180.0;
  lat2 = (lat2) * M_PI / 180.0;

  double a = pow(sin(dLat / 2), 2) +
             pow(sin(dLon / 2), 2) *
             cos(lat1) * cos(lat2);
  double rad = 6371; // raio da Terra em quilômetros
  double c = 2 * asin(sqrt(a));
  return rad * c * 1000; // convertendo para metros
}

int main(int argc, char **argv) {
  if (argc < 3) {
    usage();
  }

  int s;
  s = socket(argv[1][3] == '4' ? AF_INET : AF_INET6, SOCK_STREAM, 0);
  if (s == -1) {
    logexit("socket");
  }

  struct sockaddr_in addr;
  struct sockaddr_in6 addr6;
  void *addrptr;
  socklen_t addrlen;
  if (argv[1][3] == '4') {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = INADDR_ANY;
    addrptr = (struct sockaddr *)&addr;
    addrlen = sizeof(addr);
  } else {
    memset(&addr6, 0, sizeof(addr6));
    addr6.sin6_family = AF_INET6;
    addr6.sin6_port = htons(atoi(argv[2]));
    addr6.sin6_addr = in6addr_any;
    addrptr = (struct sockaddr *)&addr6;
    addrlen = sizeof(addr6);
  }

  if (bind(s, addrptr, addrlen) == -1) {
    logexit("bind");
  }

  if (listen(s, 10) == -1) {
    logexit("listen");
  }

  system("clear");

  while (true) {
    printf("Aguardando solicitação (port %s)\n", argv[2]);
    int client_sock;
    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    client_sock = accept(s, (struct sockaddr *)&client_addr, &client_addrlen);
    if (client_sock == -1) {
      logexit("accept");
    }

    // Receber coordenadas do cliente
    double latitude, longitude;
    recv(client_sock, &latitude, sizeof(latitude), 0);
    recv(client_sock, &longitude, sizeof(longitude), 0);

    // Calcular distância entre o cliente e o motorista (que está na UFMG)
    double distance = haversine(latitude, longitude, -19.871904, -43.966248);

    int option;
    system("clear");
    printf("Nova corrida disponível (%.2f metros)\n", distance);
    printf("0) Rejeitar\n");
    printf("1) Aceitar\n");
    scanf("%d", &option);

    while (option != 1 && option != 0) {
      system("clear");
      printf("Opção inválida!\n");
      printf("Nova corrida (%.2f metros)\n", distance);
      printf("0) Rejeitar\n");
      printf("1) Aceitar\n");
      scanf("%d", &option);
    }

    if (option == 0) {
      // Nega a corrida
      char buf[BUFSZ];
      sprintf(buf, "%.2f", -1.0);
      send(client_sock, buf, strlen(buf) + 1, 0);
    }

    if (option == 1) {
      // Enviar distância inicial para o cliente
      char buf[BUFSZ];
      sprintf(buf, "%.2f", distance);
      send(client_sock, buf, strlen(buf) + 1, 0);

      // Atualizar distância a cada 2 segundos até o motorista chegar
      while (distance > 0) {
        sleep(2);
        distance -= 400;  // Decrementa 400m a cada iteração
        if (distance < 0) {
          distance = 0;
        }

        sprintf(buf, "%.2f", distance);
        send(client_sock, buf, strlen(buf) + 1, 0);
        if (distance == 0) {
          // Motorista chegou
          printf("O motorista chegou!\n");
          close(client_sock);
          break; // Sai do while interno
        }
      }
    }
  }

  close(s);

  return 0;
}

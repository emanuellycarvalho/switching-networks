#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFSZ 1024

void usage() {
  printf("usage: %s <ipv4|ipv6> <server IP> <server port>\n", "client");
  printf("example: %s ipv4 127.0.0.1 50501\n", "client");
  exit(EXIT_FAILURE);
}

void logexit(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
  if (argc < 4) {
    usage();
  }

   system("./server ipv4 50502 &");

  int s;
  s = socket(argv[1][3] == '4' ? AF_INET : AF_INET6, SOCK_STREAM, 0);
  if (s == -1) {
    logexit("Socket");
  }

  struct sockaddr_in addr;
  struct sockaddr_in6 addr6;
  void *addrptr;
  socklen_t addrlen;
  if (argv[1][3] == '4') {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[3]));
    inet_pton(AF_INET, argv[2], &addr.sin_addr);
    addrptr = (struct sockaddr *)&addr;
    addrlen = sizeof(addr);
  } else {
    memset(&addr6, 0, sizeof(addr6));
    addr6.sin6_family = AF_INET6;
    addr6.sin6_port = htons(atoi(argv[3]));
    inet_pton(AF_INET6, argv[2], &addr6.sin6_addr);
    addrptr = (struct sockaddr *)&addr6;
    addrlen = sizeof(addr6);
  }

  if (connect(s, addrptr, addrlen) == -1) {
    system("pkill -f './server ipv4 50502'");
    logexit("Connect");
  }

  printf("Connected to server\n");

  // Menu inicial
  int option;
  do {
    printf("Pressione 1 para sair ou 2 para buscar motorista: ");
    scanf("%d", &option);
    if (option == 2) {
      // Envie suas coordenadas para o servidor
      double latitude, longitude;
      printf("Informe sua latitude e longitude (exemplo: -19.9227 -43.9451): ");
      scanf("%lf %lf", &latitude, &longitude);
      send(s, &latitude, sizeof(latitude), 0);
      send(s, &longitude, sizeof(longitude), 0);

      // Receber a distância inicial do servidor
      char buf[BUFSZ];
      memset(buf, 0, BUFSZ);
      int count = recv(s, buf, BUFSZ, 0);
      if (count <= 0) {
        logexit("recv");
      }
      printf("Distância inicial do motorista: %s metros\n", buf);

      // Receber e exibir mensagens do servidor até o motorista chegar
      while (1) {
        memset(buf, 0, BUFSZ);
        count = recv(s, buf, BUFSZ, 0);
        if (count <= 0) {
          break;
        }
        printf("O motorista está a %s metros\n", buf);
        if (strcmp(buf, "0") == 0) {
          printf("O motorista chegou!\n");
          break;
        }
      }
    }
  } while (option != 1);

  close(s);
  system("pkill -f './server ipv4 50502'");

  return 0;
}

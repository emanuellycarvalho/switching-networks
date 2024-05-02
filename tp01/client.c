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

  // Menu inicial
  int option;
  do {
    printf("0) Sair\n");
    printf("1) Buscar motorista\n");
    scanf("%d", &option);
    if (option == 1) {
      // Conecta com o servidor
      sleep(1);

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
          logexit("Connect");
      }

      // Envie suas coordenadas para o servidor
      double latitude, longitude;
      printf("(Exemplo sugerido: -19.867688 -43.985187)");
      printf("Informe sua latitude e longitude: ");
      scanf("%lf %lf", &latitude, &longitude);
      send(s, &latitude, sizeof(latitude), 0);
      send(s, &longitude, sizeof(longitude), 0);

      system("clear");
      printf("Buscando motorista...\n");
      sleep(3);

      // Receber a distância inicial do servidor
      char buf[BUFSZ];
      memset(buf, 0, BUFSZ);
      int count = recv(s, buf, BUFSZ, 0);
      if (count <= 0) {
        logexit("recv");
      }

      char *endptr;
      double distance = strtod(buf, &endptr);

      if(*endptr != '\0' || distance >= 0){
        printf("Distância inicial do motorista: %s metros\n", buf);

        // Receber e exibir mensagens do servidor até o motorista chegar
        while (strcmp(buf, "0") != 0) {
          memset(buf, 0, BUFSZ);
          count = recv(s, buf, BUFSZ, 0);
          if (count <= 0) {
            break;
          }
          printf("O motorista está a %s metros\n", buf);
        }

        printf("O motorista chegou!\n");
        close(s);
        return 0;
      } else {
        printf("Não foi encontrado um motorista.\n");
      }
    }
  } while (option != 0);

  return 0;
}

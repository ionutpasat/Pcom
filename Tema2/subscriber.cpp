#include "helpers.h"
#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1800

using namespace std;

void usage(char *file) {
  fprintf(stderr, "Usage: %s server_address server_port\n", file);
  exit(0);
}

int main(int argc, char *argv[]) {
  int sockfd, n, ret, err;
  static const int nodelayflag = 1;
  char sub_id[11];
  struct sockaddr_in serv_addr;
  char buffer[BUF_SIZE];
  setvbuf(stdout, NULL, _IONBF, BUFSIZ);

  if (argc < 4) {
    usage(argv[0]);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  DIE(sockfd < 0, "socket");

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[3]));
  ret = inet_aton(argv[2], &serv_addr.sin_addr);
  strcpy(sub_id, argv[1]);
  DIE(ret == 0, "inet_aton");

  ret = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  DIE(ret < 0, "connect");
  send(sockfd, sub_id, 10, 0);

  /* dezactivez algoritmul lui nagle */
  err = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &nodelayflag,
                   sizeof(nodelayflag));
  DIE(err < 0, "TCP_NODELAY");

  fd_set read_set, tmp_set;
  FD_ZERO(&read_set);
  FD_ZERO(&tmp_set);
  FD_SET(STDIN_FILENO, &read_set);
  FD_SET(sockfd, &read_set);
  int fdmax = sockfd;

  while (1) {
		tmp_set = read_set; 
		
		ret = select(fdmax + 1, &tmp_set, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		if (FD_ISSET(STDIN_FILENO, &tmp_set)) {
			// se citeste de la tastatura
			memset(buffer, 0, BUF_SIZE);
			fgets(buffer, BUF_SIZE - 1, stdin);

			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}

			// se trimite mesaj la server
			n = send(sockfd, buffer, strlen(buffer), 0);
			DIE(n < 0, "send");
		}

		if (FD_ISSET(sockfd, &tmp_set)) {
			memset(buffer, 0, BUF_SIZE);
			n = recv(sockfd, buffer, BUF_SIZE, 0);
			DIE(n < 0, "recv");
      /* serverul a inchis conexiunea */
      if(n == 0)
        break;

			printf("%s", buffer);
		}
	}


  close(sockfd);

  return 0;
}

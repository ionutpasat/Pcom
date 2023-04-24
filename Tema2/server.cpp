#include "helpers.h"
using namespace std;

void usage(char *file) {
  fprintf(stderr, "Usage: %s server_port\n", file);
  exit(0);
}

int main(int argc, char *argv[]) {
  int sockTCP, sockUDP, newsockTCP, portno, err, ok;
  vector<struct client *> all_clients;
  char buffer[BUFLEN], sub_id[11];
  static const int nodelay_flag = 1;
  struct sockaddr_in serv_addr, cli_addr;
  socklen_t clilen;
  int n, i, ret;

  setvbuf(stdout, NULL, _IONBF, BUFSIZ);
  fd_set read_fds; // multimea de citire folosita in select()
  fd_set tmp_fds;  // multime folosita temporar
  int fdmax;       // valoare maxima fd din multimea read_fds

  if (argc < 2) {
    usage(argv[0]);
  }

  // se goleste multimea de descriptori de citire (read_fds) si multimea
  // temporara (tmp_fds)
  FD_ZERO(&read_fds);
  FD_ZERO(&tmp_fds);

  sockTCP = socket(AF_INET, SOCK_STREAM, 0);
  DIE(sockTCP < 0, "socketTCP");

  sockUDP = socket(PF_INET, SOCK_DGRAM, 0);
  DIE(sockUDP < 0, "socketUdp");

  portno = atoi(argv[1]);
  DIE(portno == 0, "atoi");

  memset((char *)&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  ret = bind(sockTCP, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
  DIE(ret < 0, "bind");

  ret = bind(sockUDP, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
  DIE(ret < 0, "bind");

  ret = listen(sockTCP, MAX_CLIENTS);
  DIE(ret < 0, "listen");

  // multimea read_fds
  FD_SET(sockTCP, &read_fds);
  FD_SET(sockUDP, &read_fds);
  FD_SET(STDIN_FILENO, &read_fds);
  fdmax = max(sockTCP, sockUDP);

  while (1) {
    tmp_fds = read_fds;
    // {1,}
    ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
    DIE(ret < 0, "select");

    for (i = 0; i <= fdmax; i++) {
      if (FD_ISSET(i, &tmp_fds)) {
        if (i == sockTCP) {
          // a venit o cerere de conexiune pe socketul inactiv (cel cu
          // listen), pe care serverul o accepta
          clilen = sizeof(cli_addr);
          newsockTCP = accept(sockTCP, (struct sockaddr *)&cli_addr, &clilen);
          DIE(newsockTCP < 0, "accept");

          /* dezactivez algoritmul lui Nagle */
          err = setsockopt(newsockTCP, IPPROTO_TCP, TCP_NODELAY, &nodelay_flag,
                           sizeof(nodelay_flag));
          DIE(err < 0, "TCP_NODELAY");

          FD_SET(newsockTCP, &read_fds);
          if (newsockTCP > fdmax) {
            fdmax = newsockTCP;
          }

          ret = recv(newsockTCP, sub_id, 10, 0);
          DIE(ret < 0, "recv");

          /*
           * daca se incearca conectarea cu acelasi id al unui client
           * activ, anulez incercarea si scot socketul din set
           */
          ok = 0;
          for (int k = 0; k < (int)all_clients.size(); k++) {
            if (strcmp(all_clients[k]->id, sub_id) == 0) {
              ok = 1;
              if (all_clients[k]->active) {
                cout << "Client " << all_clients[k]->id
                     << " already connected.\n";
                close(newsockTCP);
                FD_CLR(newsockTCP, &read_fds);
                break;
              }

              /*
               * in schimb daca clientul doar era deconectat, ii schimb starea
               * si ii trimit mesajele din istoric, apoi il golesc
               */
              all_clients[k]->active = 1;
              printf("New client %s connected from port %s:%s.\n",
                     all_clients[k]->id, all_clients[k]->IP,
                     all_clients[k]->port);
              for (int m = 0; m < (int)all_clients[k]->history.size(); m++) {
                n = send(all_clients[k]->socket,
                         all_clients[k]->history[m].c_str(),
                         all_clients[k]->history[m].length(), 0);
                DIE(n < 0, "send");
              }
              all_clients[k]->history.clear();
              break;
            }
          }

          /*
           * pun clientul in lista de clienti si il asignez ca activ
           */
          if (ok == 0) {
            struct client *new_client = new client();
            strcpy(new_client->IP, inet_ntoa(cli_addr.sin_addr));
            strncpy(new_client->id, sub_id, 10);
            new_client->active = 1;
            sprintf(new_client->port, "%d", (int)ntohs(cli_addr.sin_port));
            new_client->socket = newsockTCP;
            all_clients.push_back(new_client);

            printf("New client %s connected from port %s:%d.\n", sub_id,
                   inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
            // se adauga noul socket intors de accept() la multimea
            // descriptorilor de citire
          }
          /*
           * daca se citeste de la tastatura comanda exit, inchid serverul
           *  si toate socketurile clientilor conectati la el
           */
        } else if (i == STDIN_FILENO) {
          memset(buffer, 0, BUFLEN);
          n = read(0, buffer, BUFLEN);
          DIE(n < 0, "read");

          if (strncmp(buffer, "exit", 4) == 0) {
            for (int k = 0; k < (int)all_clients.size(); k++)
              if (all_clients[k]->active == 1)
                close(all_clients[k]->socket);
            close(sockTCP);
            close(sockUDP);
            return 0;
          }
          DIE(true, "comanda gresita");
          /* daca se primeste de la UDP */
        } else if (i == sockUDP) {
          clilen = sizeof(cli_addr);
          char UDPbuf[1551];
          n = recvfrom(sockUDP, UDPbuf, sizeof(UDPbuf), 0,
                       (struct sockaddr *)&cli_addr, &clilen);
          DIE(n < 0, "recvfrom");

          /* segmentez mesajul de la UDP folosind o structura auxiliara */
          UDPstruct *aux_udp_struct = (UDPstruct *)UDPbuf;
          char port[6];
          char ip_port[30];
          char ip_port_topic[80];
          char final[1800];
          /* formez stringul "<IP_CLIENT_UDP>:<PORT_CLIENT_UDP> - <TOPIC> -" */
          sprintf(port, ":%d - ", (int)ntohs(cli_addr.sin_port));
          strcpy(ip_port, strcat(inet_ntoa(cli_addr.sin_addr), port));
          strcpy(ip_port_topic, ip_port);
          strcat(ip_port_topic, aux_udp_struct->topic);
          strcpy(final, ip_port_topic);
          strcat(final, " - ");

          /* switch dupa tipul de date */
          switch ((int)aux_udp_struct->tip_date) {
          case 0: {
            /*
             * folosind tot o structura colectez datele din payloadul primit
             * si le prelucrez corespunzator
             */
            INT *case_int = (INT *)aux_udp_struct->payload;
            memcpy(&case_int->payload, aux_udp_struct->payload + 1, 4);
            int number = (int)ntohl(case_int->payload);
            /*
             * daca bitul de semn e 1 => -number else number
             */
            int tosend = (int)case_int->sign == 1 ? (-1) * number : number;
            strcat(final, "INT");
            char fin_nr[20];
            sprintf(fin_nr, " - %d\n", tosend);
            strcat(final, fin_nr);

            /*
             * caut in vectorul de clienti cine este abonat la topic
             * si le trimit mesajul ulterior creat
             */
            send_message_tcp(all_clients, aux_udp_struct->topic, final);
            break;
          }
          case 1: {
            uint16_t payload;
            memcpy(&payload, aux_udp_struct->payload, sizeof(payload));
            /* prelucrez payloadul */
            float tosend = (float)ntohs(payload) / 100;
            strcat(final, "SHORT_REAL");
            char fin_nr[20];
            /* concatenez nr doar cu 2 zecimale */
            sprintf(fin_nr, " - %.2f\n", tosend);
            strcat(final, fin_nr);

            /*
             * caut in vectorul de clienti cine este abonat la topic
             * si le trimit mesajul ulterior creat
             */
            send_message_tcp(all_clients, aux_udp_struct->topic, final);

            break;
          }
          case 2: {
            /*
             * folosesc din nou o structura sa segmentez payloadul
             */
            FLOAT *case_float = new FLOAT();
            memcpy(&case_float->sign, aux_udp_struct->payload, sizeof(uint8_t));
            memcpy(&case_float->payload,
                   aux_udp_struct->payload + sizeof(uint8_t), sizeof(uint32_t));
            memcpy(&case_float->modul,
                   aux_udp_struct->payload + sizeof(uint8_t) + sizeof(uint32_t),
                   sizeof(uint8_t));
            int number = (int)ntohl(case_float->payload);

            /* construiesc numarul in functie de bitul de semn */
            float tosend =
                case_float->sign == 1
                    ? (-1) * (float)number /
                          (float)pow(10, (float)case_float->modul)
                    : (float)number / (float)pow(10, (float)case_float->modul);

            strcat(final, "FLOAT");
            char fin_nr[30];
            sprintf(fin_nr, " - %f\n", tosend);
            strcat(final, fin_nr);

            /*
             * caut in vectorul de clienti cine este abonat la topic
             * si le trimit mesajul ulterior creat
             */
            send_message_tcp(all_clients, aux_udp_struct->topic, final);
            break;
          }
          case 3: {
            /* aici pot concatena direct payloadul, fiind deja string */
            strcat(final, "STRING - ");
            strcat(final, aux_udp_struct->payload);
            strcat(final, "\n");
            send_message_tcp(all_clients, aux_udp_struct->topic, final);
            break;
          }
          default: {
            DIE(true, "wrong data_type");
            break;
          }
          }
        } else {
          // s-au primit date pe unul din socketii de client,
          // asa ca serverul trebuie sa le receptioneze
          memset(buffer, 0, BUFLEN);
          n = recv(i, buffer, BUFLEN, 0);
          DIE(n < 0, "recv");

          /* daca primesc 0 octeti => clientul s-a deconectat */
          if (n == 0) {
            for (int k = 0; k < (int)all_clients.size(); k++)
              if (all_clients[k]->socket == i) {
                all_clients[k]->active = 0;
                printf("Client %s disconnected.\n", all_clients[k]->id);
                close(i);
                break;
              }

            // se scoate din multimea de citire socketul inchis
            FD_CLR(i, &read_fds);
          } else {
            /* se primesc comenzi de la client */
            char topic[50], command[12];
            int sf;
            /*
             * comanda e subscribe => introduc topicul in vectorul de
             * subscriptii al clientului cu sf-ul coresp.
             */
            if (strncmp(buffer, "subscribe", 9) == 0) {
              /* segmentez comanda cu sscanf */
              sscanf(buffer, "%s %s %d", command, topic, &sf);
              for (int k = 0; k < (int)all_clients.size(); k++) {
                if (all_clients[k]->socket == i) {
                  pair<string, int> new_subscription;
                  new_subscription.first = topic;
                  new_subscription.second = sf;
                  all_clients[k]->subscribed_to.push_back(new_subscription);
                  break;
                }
              }
              /* trimit clientului confirmarea ca s-a abonat la topic */
              memset(buffer, 0, BUFLEN);
              strcpy(buffer, "Subscribed to topic.\n");
              n = send(i, buffer, 22, 0);
              DIE(n < 0, "send");
              continue;
            }
            /*
             * comanda e unsubscribe => sterg topicul din lista de subscriptii
             * a clientului
             */
            if (strncmp(buffer, "unsubscribe", 11) == 0) {
              sscanf(buffer, "%s %s", command, topic);
              for (int k = 0; k < (int)all_clients.size(); k++)
                if (all_clients[k]->socket == i) {
                  for (int m = 0; m < (int)all_clients[k]->subscribed_to.size();
                       m++)
                    if (all_clients[k]->subscribed_to[m].first == topic)
                      all_clients[k]->subscribed_to.erase(
                          remove(all_clients[k]->subscribed_to.begin(),
                                 all_clients[k]->subscribed_to.end(),
                                 all_clients[k]->subscribed_to[m]),
                          all_clients[k]->subscribed_to.end());
                  break;
                }
              /* trimit clientului confirmarea ca s-a dezabonat de la topic */
              memset(buffer, 0, BUFLEN);
              strcpy(buffer, "Unsubscribed from topic.\n");
              n = send(i, buffer, 26, 0);
              DIE(n < 0, "send");
              continue;
            }
            DIE(true, "comanda gresita");
          }
        }
      }
    }
  }

  /* inchid socketii */
  close(sockTCP);
  close(sockUDP);

  return 0;
}

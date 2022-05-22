#include "requests.h"
#include "helpers.h"
#include <arpa/inet.h>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count, char *jwt) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));

  // Step 1: write the method name, URL, request params (if any) and protocol
  // type
  if (query_params != NULL) {
    sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
  } else {
    sprintf(line, "GET %s HTTP/1.1", url);
  }

  compute_message(message, line);

  // Step 2: add the host
  memset(line, 0, LINELEN);
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  if (jwt) {
    memset(line, 0, LINELEN);
    sprintf(line, "%s: %s %s", "Authorization", "Bearer", jwt);
    compute_message(message, line);
  }

  // Step 3 (optional): add headers and/or cookies, according to the protocol
  // format
  if (cookies != NULL) {
    memset(line, 0, LINELEN);
    strcat(line, "Cookie: ");

    for (int i = 0; i < cookies_count - 1; i++) {
      strcat(line, cookies[i]);
      strcat(line, ";");
    }

    strcat(line, cookies[cookies_count - 1]);
    compute_message(message, line);
  }

  // Step 4: add final new line
  compute_message(message, "");
  return message;
}

char *compute_post_request(char *host, char *url, char *content_type,
                           char *body_data, int body_data_fields_count,
                           char **cookies, int cookies_count, char *jwt) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));
  // char *body_data_buffer = (char*)calloc(LINELEN, sizeof(char));

  // Step 1: write the method name, URL and protocol type
  sprintf(line, "POST %s HTTP/1.1", url);
  compute_message(message, line);

  // Step 2: add the host
  memset(line, 0, LINELEN);
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  if (jwt) {
    memset(line, 0, LINELEN);
    sprintf(line, "%s: %s %s", "Authorization", "Bearer", jwt);
    compute_message(message, line);
  }

  sprintf(line, "Content-Type: %s\r\nContent-Length: %ld", content_type,
          strlen(body_data));
  compute_message(message, line);

  // Step 4 (optional): add cookies
  if (cookies != NULL) {
    memset(line, 0, LINELEN);
    strcat(line, "Cookie: ");

    for (int i = 0; i < cookies_count - 1; i++) {
      strcat(line, cookies[i]);
      strcat(line, ";");
    }

    strcat(line, cookies[cookies_count - 1]);
    compute_message(message, line);
  }

  // Step 5: add new line at end of header
  compute_message(message, "");

  // Step 6: add data
  memset(line, 0, LINELEN);
  compute_message(message, body_data);

  free(line);
  return message;
}

char *compute_delete_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count, char *jwt) {
  char *message = (char *)calloc(BUFLEN, sizeof(char));
  char *line = (char *)calloc(LINELEN, sizeof(char));

  // Step 1: write the method name, URL, request params (if any) and protocol
  // type
  if (query_params != NULL) {
    sprintf(line, "DELETE %s?%s HTTP/1.1", url, query_params);
  } else {
    sprintf(line, "DELETE %s HTTP/1.1", url);
  }

  compute_message(message, line);

  // Step 2: add the host
  memset(line, 0, LINELEN);
  sprintf(line, "Host: %s", host);
  compute_message(message, line);

  if (jwt) {
    memset(line, 0, LINELEN);
    sprintf(line, "%s: %s %s", "Authorization", "Bearer", jwt);
    compute_message(message, line);
  }

  // Step 3 (optional): add headers and/or cookies, according to the protocol
  // format
  if (cookies != NULL) {
    memset(line, 0, LINELEN);
    strcat(line, "Cookie: ");

    for (int i = 0; i < cookies_count - 1; i++) {
      strcat(line, cookies[i]);
      strcat(line, ";");
    }

    strcat(line, cookies[cookies_count - 1]);
    compute_message(message, line);
  }

  // Step 4: add final new line
  compute_message(message, "");
  return message;
}

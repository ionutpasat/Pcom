#include "helpers.h"
#include "json.hpp"
#include "requests.h"
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <iostream>
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <string.h>     /* memcpy, memset */
#include <string>
#include <sys/socket.h> /* socket, connect */
#include <unistd.h>     /* read, write, close */

using namespace std;

#define HOST "34.241.4.235"
#define PORT 8080
#define REGISTER "/api/v1/tema/auth/register"
#define LOGIN "/api/v1/tema/auth/login"
#define PAYLOAD "application/json"
#define LIBRARY_ACCES "/api/v1/tema/library/access"
#define GET_BOOKS "/api/v1/tema/library/books"
#define GET_BOOK "/api/v1/tema/library/books/"
#define ADD_BOOK "/api/v1/tema/library/books"
#define DELETE_BOOK "/api/v1/tema/library/books/"
#define LOGOUT "/api/v1/tema/auth/logout"

int main(int argc, char *argv[]) {
  string command, cookie, token;
  int sockfd;
  while (1) {
    cout << "$ ";
    cin >> command;

    /*------------------------------EXIT-------------------------------------*/
    if (strncmp(command.c_str(), "exit", 4) == 0) {
      break;

      /*------------------------------REGISTER----------------------------------*/
    } else if (strncmp(command.c_str(), "register", 8) == 0) {
      string message, response;
      string user, pass, form_data;
      /*----------Initializez procesul de inregistrare----------*/
      cout << "Please enter the registration data required!\n";
      cout << "username: ";
      cin >> user;
      cout << "password: ";
      cin >> pass;
      /*-----Pregatesc datele de trimis la server si le expediez-----*/
      nlohmann::json data = {{"username", user}, {"password", pass}};
      form_data = data.dump();
      sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message =
          compute_post_request((char *)HOST, (char *)REGISTER, (char *)PAYLOAD,
                               (char *)form_data.c_str(), 2, NULL, 0, NULL);
      send_to_server(sockfd, (char *)message.c_str());
      response = receive_from_server(sockfd);
      /*-----------------Retin raspunsul serverului------------------*/
      int status;
      char trash[20];
      sscanf(response.c_str(), "%s %d %s\n", trash, &status, trash);
      /*-----In functie de raspuns se afiseaza mesaj corespunzator-----*/
      switch (status) {
      case 200: {
        cout << "200-OK-Logged in successfully! Welcome!\n";
        break;
      }
      case 400: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "400-Bad request! " << json_err["error"] << "\n";
        break;
      }
      case 500: {
        cout << "500-Internal server error!\n";
        break;
      }
      case 429: {
        cout << "429-Too many requests!\n";
        break;
      }
      default: {
        cout << "Something went wrong! Try again in a few seconds!\n";
        break;
      }
      }
      /*--------------Inchid socketul----------------*/
      close_connection(sockfd);

      /*------------------------------LOGIN-----------------------------------*/
    } else if (strncmp(command.c_str(), "login", 5) == 0) {
      string message, response;
      string user, pass, form_data;
      /*----------Initializez procesul de inregistrare----------*/
      cout << "Please enter the login data!\n";
      cout << "username: ";
      cin >> user;
      cout << "password: ";
      cin >> pass;
      /*-----Pregatesc datele de trimis la server si le expediez-----*/
      nlohmann::json data = {{"username", user}, {"password", pass}};
      form_data = data.dump();
      sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message =
          compute_post_request((char *)HOST, (char *)LOGIN, (char *)PAYLOAD,
                               (char *)form_data.c_str(), 2, NULL, 0, NULL);
      send_to_server(sockfd, (char *)message.c_str());
      response = receive_from_server(sockfd);
      /*-----------------Retin raspunsul serverului------------------*/
      int status;
      char trash[20];
      sscanf(response.c_str(), "%s %d %s\n", trash, &status, trash);
      /*-----In functie de raspuns se afiseaza mesaj corespunzator-----*/
      switch (status) {
      case 200: {
        size_t found_begin = response.find("Set-Cookie:");
        size_t found_end = response.find(" Path");
        cookie =
            response.substr(found_begin + 12, found_end - found_begin - 13);
        cout << "200-OK-Logged in successfully! Welcome!\n";
        break;
      }
      case 400: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "400-Bad request! " << json_err["error"].get<string>() << "\n";
        break;
      }
      case 500: {
        cout << "500-Internal server error!\n";
        break;
      }
      case 429: {
        cout << "429-Too many requests!\n";
        break;
      }
      default: {
        cout << "Something went wrong! Try again in a few seconds!\n";
        break;
      }
      }
      /*--------------Inchid socketul----------------*/
      close_connection(sockfd);
    }

    /*----------------------------ENTER LIBRARY------------------------------*/
    else if (strncmp(command.c_str(), "enter_library", 13) == 0) {
      string message, response;
      /*-------------prelucrez cookie-urile pt. get request------------*/
      char **cookies = (char **)malloc(sizeof(char *));
      cookies[0] = (char *)cookie.c_str();
      sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message = compute_get_request((char *)HOST, (char *)LIBRARY_ACCES, NULL,
                                    (cookie == "" ? NULL : cookies), 1, NULL);
      send_to_server(sockfd, (char *)message.c_str());
      response = receive_from_server(sockfd);
      /*-----------------Retin raspunsul serverului------------------*/
      int status;
      char trash[20];
      sscanf(response.c_str(), "%s %d %s\n", trash, &status, trash);
      /*-----In functie de raspuns se afiseaza mesaj corespunzator-----*/
      switch (status) {
      case 200: {
        char *get_token = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_get_token = nlohmann::json::parse(get_token);
        token = json_get_token["token"].get<string>();
        cout << "200-OK-Acces granted! Welcome to the library!\n";
        break;
      }
      case 401: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "401-Unauthorized! " << json_err["error"].get<string>() << "\n";
        break;
      }
      case 500: {
        cout << "500-Internal server error!\n";
        break;
      }
      case 429: {
        cout << "429-Too many requests!\n";
        break;
      }
      default: {
        cout << "Something went wrong! Try again in a few seconds!\n";
        break;
      }
      }
      /*--------------Inchid socketul----------------*/
      close_connection(sockfd);
    }

    /*----------------------------ADD BOOK------------------------------*/
    else if (strncmp(command.c_str(), "add_book", 8) == 0) {
      string message, response, title, author, genre, publisher;
      int page_count;
      /*--------------------initializare carte----------------------------*/
      cout << "title= ";
      getline(cin >> ws, title);
      cout << "author= ";
      getline(cin >> ws, author);
      cout << "genre= ";
      getline(cin >> ws, genre);
      cout << "publisher= ";
      getline(cin >> ws, publisher);
      cout << "page_count= ";
      while (!(cin >> page_count) || page_count < 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Try again:\npage_count= ";
      }
      nlohmann::json new_book = {{"title", title},
                                 {"author", author},
                                 {"genre", genre},
                                 {"publisher", publisher},
                                 {"page_count", page_count}};
      sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message =
          compute_post_request((char *)HOST, (char *)ADD_BOOK, (char *)PAYLOAD,
                               (char *)new_book.dump().c_str(), 5, NULL, 0,
                               (token == "" ? NULL : (char *)token.c_str()));
      send_to_server(sockfd, (char *)message.c_str());
      response = receive_from_server(sockfd);
      /*-----------------Retin raspunsul serverului------------------*/
      int status;
      char trash[20];
      sscanf(response.c_str(), "%s %d %s\n", trash, &status, trash);
      /*-----In functie de raspuns se afiseaza mesaj corespunzator-----*/
      switch (status) {
      case 200: {
        cout << "200-OK-Book added successfully!\n";
        break;
      }
      case 403: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "403-Forbidden! " << json_err["error"].get<string>() << "\n";
        break;
      }
      case 500: {
        cout << "500-Internal server error!\n";
        break;
      }
      case 429: {
        cout << "429-Too many requests!\n";
        break;
      }
      default: {
        cout << "Something went wrong! Try again in a few seconds!\n";
        break;
      }
      }
      /*--------------Inchid socketul----------------*/
      close_connection(sockfd);
    }

    /*----------------------------GET BOOKS------------------------------*/
    else if (strncmp(command.c_str(), "get_books", 9) == 0) {
      string message, response;
      sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message =
          compute_get_request((char *)HOST, (char *)GET_BOOKS, NULL, NULL, 0,
                              (token == "" ? NULL : (char *)token.c_str()));
      send_to_server(sockfd, (char *)message.c_str());
      response = receive_from_server(sockfd);
      /*-----------------Retin raspunsul serverului------------------*/
      int status;
      char trash[20];
      sscanf(response.c_str(), "%s %d %s\n", trash, &status, trash);
      /*-----In functie de raspuns se afiseaza mesaj corespunzator-----*/
      switch (status) {
      case 200: {
        int i = 1;
        char *feedback = basic_extract_json_array((char *)response.c_str());
        nlohmann::json books = nlohmann::json::parse(feedback);
        for (auto &book : books.items()) {
          nlohmann::json details = book.value();
          cout << "Book " << i++ << " : id = " << details["id"]
               << ", title = " << details["title"] << endl;
        }
        if (i == 1)
          cout << "There are no books in the library! Use 'add_book' to add "
                  "one!\n";
        else
          cout << "200-OK-Books returned successfully!\n";
        break;
      }
      case 403: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "403-Forbidden! " << json_err["error"].get<string>() << "\n";
        break;
      }
      case 500: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "500-Internal server error! " << json_err["error"].get<string>()
             << "\n";
        break;
      }
      case 429: {
        cout << "429-Too many requests!\n";
        break;
      }
      default: {
        cout << "Something went wrong! Try again in a few seconds!\n";
        break;
      }
      }
      /*--------------Inchid socketul----------------*/
      close_connection(sockfd);
    }

    /*----------------------GET BOOK DETAILS------------------------*/
    else if (strncmp(command.c_str(), "get_book", 8) == 0) {
      string message, response, url, book_id;
      /*--------------------Se citeste id-ul cartii----------------------*/
      cout << "id = ";
      cin >> book_id;
      url = GET_BOOK + book_id;
      sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message =
          compute_get_request((char *)HOST, (char *)url.c_str(), NULL, NULL, 0,
                              (token == "" ? NULL : (char *)token.c_str()));
      send_to_server(sockfd, (char *)message.c_str());
      response = receive_from_server(sockfd);
      /*-----------------Retin raspunsul serverului------------------*/
      int status;
      char trash[20];
      sscanf(response.c_str(), "%s %d %s\n", trash, &status, trash);
      /*-----In functie de raspuns se afiseaza mesaj corespunzator-----*/
      switch (status) {
      case 200: {
        /*-----------In caz de succes afisez cartile din library---------*/
        char *feedback = basic_extract_json_array((char *)response.c_str());
        nlohmann::json book = nlohmann::json::parse(feedback);
        for (auto &book_details : book.items()) {
          nlohmann::json details = book_details.value();
          for (nlohmann::json::iterator it = details.begin();
               it != details.end(); ++it) {
            std::cout << it.key() << " = " << it.value() << "\n";
          }
        }
        if (feedback)
          cout << "200-OK-Book details returned successfully!\n";
        else
          cout << "There are no books in the library! Use 'add_book' to add "
                  "one\n";
        break;
      }
      case 403: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "403-Forbidden! " << json_err["error"].get<string>() << "\n";
        break;
      }
      case 404: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "404-Not found! " << json_err["error"].get<string>() << "\n";
        break;
      }
      case 500: {
        cout << "500-Internal server error!\n";
        break;
      }
      case 429: {
        cout << "429-Too many requests!\n";
        break;
      }
      default: {
        cout << "Something went wrong! Try again in a few seconds!\n";
        break;
      }
      }
      /*--------------Inchid socketul----------------*/
      close_connection(sockfd);
    }

    /*----------------------LOGOUT------------------------*/
    else if (strncmp(command.c_str(), "logout", 6) == 0) {
      string message, response;
      /*-------------prelucrez cookie-urile pt. get request------------*/
      char **cookies = (char **)malloc(sizeof(char *));
      cookies[0] = (char *)cookie.c_str();
      sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message = compute_get_request((char *)HOST, (char *)LOGOUT, NULL,
                                    (cookie == "" ? NULL : cookies), 1, NULL);
      send_to_server(sockfd, (char *)message.c_str());
      response = receive_from_server(sockfd);
      /*-----------------Retin raspunsul serverului------------------*/
      int status;
      char trash[20];
      sscanf(response.c_str(), "%s %d %s\n", trash, &status, trash);
      /*-----In functie de raspuns se afiseaza mesaj corespunzator-----*/
      switch (status) {
      case 200: {
        cout << "200-OK-Logged out successfully!\n";
        break;
      }
      case 400: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "400-Bad request! " << json_err["error"].get<string>() << "\n";
        break;
      }
      case 500: {
        cout << "500-Internal server error!\n";
        break;
      }
      case 429: {
        cout << "429-Too many requests!\n";
        break;
      }
      default: {
        cout << "Something went wrong! Try again in a few seconds!\n";
        break;
      }
      }
      /*-----------Inchid socketul si sterg cookie-urile si jwt-----------*/
      close_connection(sockfd);
      cookie.clear();
      token.clear();
    }

    /*----------------------DELETE BOOK------------------------*/
    else if (strncmp(command.c_str(), "delete_book", 11) == 0) {
      /*--------------------Se citeste id-ul cartii----------------------*/
      string message, response, url, book_id;
      cout << "id = ";
      cin >> book_id;
      url = DELETE_BOOK + book_id;
      sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
      message = compute_delete_request(
          (char *)HOST, (char *)url.c_str(), NULL, NULL, 0,
          (token == "" ? NULL : (char *)token.c_str()));
      send_to_server(sockfd, (char *)message.c_str());
      response = receive_from_server(sockfd);
      /*-----------------Retin raspunsul serverului------------------*/
      int status;
      char trash[20];
      sscanf(response.c_str(), "%s %d %s\n", trash, &status, trash);
      /*-----In functie de raspuns se afiseaza mesaj corespunzator-----*/
      switch (status) {
      case 200: {
        cout << "200-OK-Book deleted successfully!\n";
        break;
      }
      case 403: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "403-Forbidden! " << json_err["error"].get<string>() << "\n";
        break;
      }
      case 404: {
        char *error = basic_extract_json_response((char *)response.c_str());
        nlohmann::json json_err = nlohmann::json::parse(error);
        cout << "404-Not found! " << json_err["error"].get<string>() << "\n";
        break;
      }
      case 500: {
        cout << "500-Internal server error!\n";
        break;
      }
      case 429: {
        cout << "429-Too many requests!\n";
        break;
      }
      default: {
        cout << "Something went wrong! Try again in a few seconds!\n";
        break;
      }
      }
      /*--------------Inchid socketul----------------*/
      close_connection(sockfd);
    }
  }

  return 0;
}

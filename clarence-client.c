#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define PORT 8080

void promptDataManipulation(int sock);
void promptTable(int sock, char *str);
void promptDatabase(int sock, char *str);
void logging(const char *user, const char *commands);

int main(int argc, char const *argv[]) {
  struct sockaddr_in address;
  int sock = 0, valread;
  struct sockaddr_in serv_addr;
  char *hello = "Hello from client";
  char buffer[1024] = {0};
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
    return -1;
  }

  memset(&serv_addr, '0', sizeof(serv_addr));

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf("\nInvalid address/ Address not supported \n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nConnection Failed \n");
    return -1;
  }

  while (1) {
    promptDataManipulation(sock);
  }

  return 0;
}

void promptDataManipulation(int sock) {
  char command[100];
  char commander[301];
  printf("⏰ waiting promptDataManipulation\n");
  scanf("%s ", command);
  if (!strcmp(command, "CREATE")) {
    char type[100];
    scanf("%s ", type);

    if (!strcmp(type, "TABLE")) {
      char tableName[100];
      scanf("%s", tableName);
      send(sock, "table", strlen("table"), 0);
      sleep(0.2);
      promptTable(sock, tableName);
      sprintf(commander,"%s %s %s",command,type,tableName);
    } else if (!strcmp(type, "DATABASE")) {
      char dbName[100];
      scanf("%s", dbName);
      send(sock, "database", strlen("database"), 0);
      sleep(0.2);
      promptDatabase(sock, dbName);
      sprintf(commander,"%s %s %s",command,type,dbName);
    }
    // promptDatabase(sock);
  }
  
  logging("USER",commander);
};

void promptTable(int sock, char *str) {
  // ? Remove `;`
  str[strlen(str) - 1] = '\0';

  send(sock, str, strlen(str), 0);
  return;
}

void promptDatabase(int sock, char *str) {
  // ? Remove `;`
  str[strlen(str) - 1] = '\0';

  send(sock, str, strlen(str), 0);
  return;
}

void logging(const char *user, const char *commands) {
  FILE *fp;
  fp = fopen("/home/yusuf/FP.log", "a");
  char timestamp[1000];
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  //2021-05-19 02:05:15:jack:SELECT FROM table1
  sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d",tm.tm_year + 1900, tm.tm_mon + 1,tm.tm_mday,
           tm.tm_hour, tm.tm_min, tm.tm_sec);
  fputs(timestamp, fp);
  fputs(":", fp);
  fputs(user, fp);
  fputs(":", fp);
  fputs(commands, fp);
  fputs("\n", fp);
  fclose(fp);
}
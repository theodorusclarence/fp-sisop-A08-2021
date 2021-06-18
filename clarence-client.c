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
void promptTable(int sock, char *str, char *structure);
void promptDatabase(int sock, char *str);
void promptDropDb(int sock, char *str);
void promptDropTable(int sock, char *str);
void promptDropColumn(int sock, char *str, char *structure);
void promptUse(int sock, char *str);
void promptInsert(int sock, char *str, char *structure);
void promptUpdate(int sock, char *str, char *structure);
void promptSelect(int sock, char *str);
void promptDelete(int sock, char *str);
void promptShowDb(int sock, char *str);
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

  printf("========== IMPORTANT COMMANDS ===========\n");
  printf("Stopping -> STOP DB;\n");

  while (1) {
    char authMessage[1024] = {0};
    int valread;
    printf("🚀🕓 reading for authmessage \n");
    valread = read(sock, authMessage, 1024);
    printf("🚀 authMessage: %s\n", authMessage);

    if (strcmp(authMessage, "wait") != 0) {
      promptDataManipulation(sock);
    } else {
      printf("Waiting...\n");
    }
  }

  return 0;
}

void promptDataManipulation(int sock) {
  char command[100];
  char commander[301];
  printf("⏰ waiting promptDataManipulation\n");
  scanf("%s ", command);

  if (strcmp(command, "STOP") == 0) {
    send(sock, "stop", strlen("stop"), 0);
    exit(0);
  }

  if (!strcmp(command, "CREATE")) {
    char type[100];
    scanf("%s ", type);
    // *================== CREATE TABLE ==================
    if (!strcmp(type, "TABLE")) {
      char tableName[100];
      scanf("%s ", tableName);
      char tableStructure[100];
      getchar();
      scanf("%[^\n]s)", tableStructure);

      send(sock, "table", strlen("table"), 0);
      sleep(0.2);

      promptTable(sock, tableName, tableStructure);
      sprintf(commander,"%s %s %s",command,type,tableName);
    // *================== CREATE DATABASE ==================
    } else if (!strcmp(type, "DATABASE")) {
      char dbName[100];
      scanf("%s", dbName);
      send(sock, "database", strlen("database"), 0);
      sleep(0.2);
      promptDatabase(sock, dbName);
      sprintf(commander,"%s %s %s",command,type,dbName);
    }
    // promptDatabase(sock);
    // *================== INSERT ==================
  } else if (!strcmp(command, "INSERT")) {
    char tableName[100];
    char tableStructure[100];
    scanf(" INTO %s ", tableName);
    getchar();
    scanf("%[^\n]s)", tableStructure);
    send(sock, "insert", strlen("insert"), 0);
    sleep(0.2);
    promptInsert(sock, tableName, tableStructure);
    // *================== USE ==================
  } else if (!strcmp(command, "USE")) {
    char dbName[100];
    scanf("%s", dbName);
    send(sock, "use", strlen("use"), 0);
    sleep(0.2);
    promptUse(sock, dbName);
  } else if (!strcmp(command, "DROP")) {
    char type[100];
    scanf("%s ", type);
    // *================== DROP TABLE ==================
    if (!strcmp(type, "TABLE")) {
      char tableName[100];
      scanf("%s", tableName);
      send(sock, "drop-table", strlen("drop-table"), 0);
      sleep(0.2);
      promptDropTable(sock, tableName);
      sprintf(commander,"%s %s %s",command,type,tableName);
    // *================== DROP DATABASE ==================
    } else if (!strcmp(type, "DATABASE")) {
      char dbName[100];
      scanf("%s", dbName);
      send(sock, "drop-db", strlen("drop-db"), 0);
      sleep(0.2);
      promptDropDb(sock, dbName);
      sprintf(commander,"%s %s %s",command,type,dbName);
    // *================== DROP COLUMN ==================
    } else if (!strcmp(type, "COLUMN")) {
      char columnName[100], dbName[100];
      scanf("%s FROM %s", columnName, dbName);
      send(sock, "drop-column", strlen("drop-column"), 0);
      sleep(0.2);
      promptDropColumn(sock, dbName, columnName);
      sprintf(commander,"%s %s %s",command,type,dbName);
    }
    // *================== SELECT ==================
  } else if (!strcmp(command, "SELECT")) {
    char commands[200];
    scanf("%[^\n]s", commands);
    send(sock, "select", strlen("select"), 0);

    sleep(0.2);
    promptSelect(sock, commands);
    sprintf(commander,"%s %s",command, commands);
    // *================== DELETE ==================
  } else if (!strcmp(command, "DELETE")) {
    char tableName[100];
    scanf("FROM %s", tableName);
    send(sock, "delete", strlen("delete"), 0);

    sleep(0.2);
    promptDelete(sock, tableName);
    sprintf(commander,"%s FROM %s",command, tableName);
  } else if (!strcmp(command, "UPDATE")) {
    char tableName[100];
    scanf("%s", tableName);
    send(sock, "update", strlen("update"), 0);
    char updateQuery[100];
    scanf(" SET %s", updateQuery);


    sleep(0.2);
    promptUpdate(sock, tableName, updateQuery);
    sprintf(commander,"%s %s SET %s",command, tableName, updateQuery);
  } else if (!strcmp(command, "SHOW")) {
    char temp[1000];
    scanf("%s", temp);
    send(sock, "show-db", strlen("show-db"), 0);

    sleep(0.2);
    // promptShowDb(sock);
    sprintf(commander,"%s DB",command);
  }

  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  printf("From Server: %s\n", buffer);
  logging("USER",commander);

  promptDataManipulation(sock);
};

void promptTable(int sock, char *str, char *structure) {
  // Send tableName
  send(sock, str, strlen(str), 0);
  sleep(0.1);
  
  // send table properties
  send(sock, structure, strlen(structure), 0);
  return;
}

void promptDatabase(int sock, char *str) {
  // ? Remove `;`
  str[strlen(str) - 1] = '\0';

  send(sock, str, strlen(str), 0);
  return;
}

void promptUse(int sock, char *str) {
  // ? Remove `;`
  str[strlen(str) - 1] = '\0';

  send(sock, str, strlen(str), 0);
  return;
}

void promptInsert(int sock, char *str, char *structure) {
  // Send tableName
  send(sock, str, strlen(str), 0);
  sleep(0.1);
  
  // send table properties
  send(sock, structure, strlen(structure), 0);
  return;
}

void promptDropDb(int sock, char *str) {
  // ? Remove `;`
  str[strlen(str) - 1] = '\0';

  send(sock, str, strlen(str), 0);
  return;
}

void promptDropTable(int sock, char *str) {
  // ? Remove `;`
  str[strlen(str) - 1] = '\0';

  send(sock, str, strlen(str), 0);
  return;
}

void promptSelect(int sock, char *str) {
  // ? Remove `;`
  // str[strlen(str) - 1] = '\0';

  send(sock, str, strlen(str), 0);
  return;
}

void promptDelete(int sock, char *str) {
  // ? Remove `;`
  str[strlen(str) - 1] = '\0';

  send(sock, str, strlen(str), 0);
  return;
}

void promptUpdate(int sock, char* str, char* structure) {
  // Send tableName
  send(sock, str, strlen(str), 0);
  sleep(0.1);

  // send set properties
  send(sock, structure, strlen(structure), 0);
  return;
}

void promptDropColumn(int sock, char* str, char* structure) {
  // Send tableName
  send(sock, str, strlen(str), 0);
  sleep(0.1);

  // send set properties
  send(sock, structure, strlen(structure), 0);
  return;
}


void logging(const char *user, const char *commands) {
  FILE *fp;
  fp = fopen("/home/clarence/FP.log", "a");
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

#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define PORT 8080
#define SIZE 1024

char databaseUsed[1050];

void handleDatabase(int sock);
void handleTable(int sock);
void handleUse(int sock);
void handleInsert(int sock);
void handleDropDb(int sock);
void handleDropTable(int sock);

void sendSuccess(int sock) {
  send(sock, "🐉 SUCCESS", strlen("🐉 SUCCESS"), 0);
}
void sendError(int sock, char text[]) {
  char msg[1000];
  sprintf(msg, "🔥 ERROR %s", text);
  send(sock, msg, strlen(msg), 0);
}

int main(int argc, char const *argv[]) {
  // CREATE FILES FOLDER
  // mkdir("FILES", 0777);

  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  printf("running on port %d\n", PORT);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // ?  INI CODE ASLI, di mac SO_REUSEPORT harus diapus
  // if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
  //                sizeof(opt))) {

  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }

  pthread_t tid[50];

  if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                           (socklen_t *)&addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }

  while (1) {
    int valread;
    char buffer[1024] = {0};
    valread = read(new_socket, buffer, 1024);
    printf("🚀 [main()] first command: %s\n", buffer);

    if (!strcmp(buffer, "database")) {
      handleDatabase(new_socket);
    } else if (!strcmp(buffer, "table")) {
      handleTable(new_socket);
    } else if (!strcmp(buffer, "use")) {
      handleUse(new_socket);
    } else if (!strcmp(buffer, "insert")) {
      handleInsert(new_socket);
    } else if (!strcmp(buffer, "drop-db")) {
      handleDropDb(new_socket);
    } 
  }

  return 0;
}

void handleDatabase(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  printf("🚀 [handleDatabase()] first command: %s\n", buffer);

  // CREATE DIRECTORY DATABASE
  char folderCommand[1500];
  sprintf(folderCommand, "mkdir -p databases/%s", buffer);
  system(folderCommand);

  // WRITE AVAILABLE DATABASE IN TXT FILES
  FILE* fp = fopen("dblist.txt", "a+");
  fprintf(fp, "%s\n", buffer);
  fclose(fp);

  sendSuccess(sock);
  return;
}

void handleTable(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  // remove ) and ;

  char structure[1024] = {0};
  valread = read(sock, structure, 1024);
  structure[strlen(structure) - 2] = '\0';

  // ? If string is empty, then skip
  if (!strlen(databaseUsed)) { 
    sendError(sock, "Database not found");
    return;
  }

  char tableDir[3000];
  sprintf(tableDir, "databases/%s/%s", databaseUsed, buffer);

  FILE* fp = fopen(tableDir, "w");
  
  char* token = strtok (structure, ",");
  char columnNameLine[1000] = "", columnTypeLine[1000] = "";
  while (token) {
    // get token
    char tableName[100], vartype[100];
    sscanf(token, "%s %s", tableName, vartype); 

    // Write to top of the table
    char temp1[101], temp2[101];
    sprintf(temp1, "%s;", tableName);
    sprintf(temp2, "%s;", vartype);
    strcat(columnNameLine, temp1);
    strcat(columnTypeLine, temp2);

    token = strtok (NULL, ",");
    while (token && *token == '\040')
        token++;
  }

  fprintf(fp, "%s\n%s\n", columnNameLine, columnTypeLine);
  fclose(fp);

  sendSuccess(sock);
}

void handleInsert(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  // remove ) and ;

  char structure[1024] = {0};
  valread = read(sock, structure, 1024);
  structure[strlen(structure) - 2] = '\0';

  // ? If string is empty, then skip
  if (!strlen(databaseUsed)) { 
    sendError(sock, "Database not found");
    return;
  }

  char tableDir[3000];
  sprintf(tableDir, "databases/%s/%s", databaseUsed, buffer);

  FILE* fp = fopen(tableDir, "r");

  int count = 0;
  char line[256];
  while (fgets(line, sizeof line, fp) != NULL) {
    if (count == 1) {
      break;
    } else count++;
  }
  fclose(fp);
  fp = fopen(tableDir, "a+");

  // TODO MOVE TYPES TO ARRAY `typeIsStr`
  char* types = strtok (line, ";");
  char typeIsStr[1000];
  count = 0;
  while (types) {
    if (!strcmp(types, " ")) break;
    typeIsStr[count] = !strcmp(types, "string");
    count++;

    types = strtok (NULL, ";");
  }

  char* token = strtok(structure, ",");
  char insertBuffer[1000] = "";
  count = 0;
  int error = 0;
  while (token) {
    // Check if type is correct
    int isStr = token[0] == '\'';
    if (typeIsStr[count] != isStr) {
      error = 1;
      break;
    }   

    char temp1[201];
    sprintf(temp1, "%s;", token);
    strcat(insertBuffer, temp1);

    count++;
    token = strtok (NULL, ",");
    while (token && *token == '\040')
        token++;
  }
  // TODO IF GAADA ERROR THEN GAS INSERT
  if (error) {
    printf("ERRORRRRRRR\n");
    sendError(sock, "String and Int Type Error");
    return;
  } else {
    insertBuffer[strlen(insertBuffer) - 1] = '\0'; 
    fprintf(fp, "%s\n", insertBuffer);
  }

  fclose(fp);
  sendSuccess(sock);
  return;
}

void handleUse(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  printf("🚀 [handleUse()] first command: %s\n", buffer);

  FILE* fp = fopen("dblist.txt", "r");

  int count = 0, found = 0;
  char line[256];
  while (fgets(line, sizeof line, fp) != NULL) {
    // remove newline at the end
    line[strcspn(line, "\n")] = 0;

    if (!strcmp(line, buffer)){
      found = 1;
      break;
    }

    count++;
  }
  fclose(fp);

  if (found) {
    sprintf(databaseUsed, "%s", buffer);

    printf("🚀 databaseUsed: %s\n", databaseUsed);
    sendSuccess(sock);
  } else {
    sendError(sock, "No Database Found");
  }
  return;
}

void handleDropDb(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  printf("🚀 [handleDropDb()] first command: %s\n", buffer);

  // TODO CHECK IF DATABASE EXISTS
  FILE* fp = fopen("dblist.txt", "rw");

  int count = 0, found = 0;
  char line[256];
  while (fgets(line, sizeof line, fp) != NULL) {
    // remove newline at the end    
    line[strcspn(line, "\n")] = 0;

    if (!strcmp(line, buffer)){
      found = 1;
      break;
    }

    count++;
  }
  fclose(fp);

  if (!found) {
    sendError(sock, "No Database Found");
    return;
  }

  // TODO REMOVE FROM DBLIST
  FILE  *fp2;
  fp = fopen("dblist.txt", "r");
  fp2 = fopen("temp.txt", "w");

  char data[256];
  while (fgets(data, sizeof data, fp) != NULL) {
    printf("data: %s", data);
    data[strcspn(data, "\n")] = 0;

    // If not the db list that want to be deleted
    if (strcmp(data, buffer)){
      fprintf(fp2, "%s\n", data);
    }

    // bzero(data, 1024);
  }
  fclose(fp);
  fclose(fp2);
  remove("dblist.txt");
  rename("temp.txt", "dblist.txt");

  char folderCommand[1500];
  sprintf(folderCommand, "rm -rf databases/%s", buffer);
  system(folderCommand);
  

  sendSuccess(sock);
  return;
}
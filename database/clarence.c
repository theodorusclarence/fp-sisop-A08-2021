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
int connection = 0;
int currentConnection = 0;
int id_socket[1000];
int queue = 0;

void *handleStart(void *args);
void handleStopConnection(int sock);
void handleDatabase(int sock);
void handleTable(int sock);
void handleUse(int sock);
void handleInsert(int sock);
void handleDropDb(int sock);
void handleDropTable(int sock);
void handleDropColumn(int sock);
void handleSelect(int sock);
void handleDelete(int sock);
void handleUpdate(int sock);

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


  while (1) {
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                            (socklen_t *)&addrlen)) < 0) {
      perror("accept");
      exit(EXIT_FAILURE);
    }

    id_socket[currentConnection] = new_socket;

    if (connection > 0) {
      printf("📩 sending wait signal to currentConnection: %d\n",
             currentConnection);
      send(id_socket[currentConnection], "wait", strlen("wait"), 1024);

      pthread_create(&tid[currentConnection], NULL, handleStart, &new_socket);
    } else {
      printf("📩 sending go signal to currentConnection: %d\n",
             currentConnection);
      send(id_socket[currentConnection], "go", strlen("go"), 1024);

      pthread_create(&tid[currentConnection], NULL, handleStart, &new_socket);
    }
    connection++;
    currentConnection++;
  }

  return 0;
}

void *handleStart(void *args) {
    int new_socket = *(int *)args;
    int valread;
    char buffer[1024] = {0};
    valread = read(new_socket, buffer, 1024);
    printf("🚀 [main()] first command: %s\n", buffer);

    // TODO REMOVE TEMPORARY STOP
    if (strcmp(buffer, "stop") == 0) {
      printf(
          "🚀 [handleLogReg()] just got stop signal, proceeding "
          "handleStopConnection()\n");
      handleStopConnection(new_socket);
    }


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
    } else if (!strcmp(buffer, "drop-table")) {
      handleDropTable(new_socket);
    } else if (!strcmp(buffer, "drop-column")) {
      handleDropColumn(new_socket);
    } else if (!strcmp(buffer, "select")) {
      handleSelect(new_socket);
    } else if (!strcmp(buffer, "delete")) {
      handleDelete(new_socket);
    } else if (!strcmp(buffer, "update")) {
      handleUpdate(new_socket);
    }

    handleStart(&new_socket);
    pthread_cancel(pthread_self());
    
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
    sendError(sock, "No database used, run `USE dbName;`");
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
  
  // REMOVE USED DATABASE
  strcpy(databaseUsed, "");

  sendSuccess(sock);
  return;
}

void handleDropTable(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  printf("🚀 [handleDropTable()] first command: %s\n", buffer);

  if (!strlen(databaseUsed)) {
    sendError(sock, "No Database Used, try to run USE dbName;");
    return;
  }

  // TODO CHECK IF TABLE EXISTS
  char tablePath[2100];
  sprintf(tablePath, "databases/%s/%s", databaseUsed, buffer);
  FILE* fp = fopen(tablePath, "r");

  if (!fp) {
    sendError(sock, "No Database Found");
    return;
  }

  remove(tablePath);
  

  sendSuccess(sock);
  return;
}

void handleSelect(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  printf("🚀 [handleSelect()] first command: %s\n", buffer);

  if (!strlen(databaseUsed)) {
    sendError(sock, "No Database Used, try to run USE dbName;");
    return;
  }

  // TODO GET COLUMNS AND TABLENAME
  char columns[200], tableName[200];

  char* token = strtok(buffer, "FROM");
  int count = 0;
  while (token) {
    printf("token: %s\n", token);

    if (count == 0) {
      strcpy(columns, token);
      columns[strlen(columns) - 1] = '\0';
    } else {
      strcpy(tableName, token);
      tableName[strlen(tableName) - 1] = '\0';
    }

    count++;
    token = strtok (NULL, "FROM");
    while (token && *token == '\040')
        token++;
  }

  // TODO CHECK IF TABLE EXISTS
  char tablePath[2100];
  sprintf(tablePath, "databases/%s/%s", databaseUsed, tableName);
  FILE* fp = fopen(tablePath, "r");

  if (!fp) {
    sendError(sock, "No Database Found");
    return;
  }

  char resultBuffer[10000];
  strcat(resultBuffer, "🐉 SUCCESS\n=============\nTable Result:\n");

  // TODO IF * SELECT
  if (!strcmp(columns, "*")) {
    char line[256];
    while (fgets(line, sizeof line, fp) != NULL) {
      strcat(resultBuffer, line);
    }
    fclose(fp);
  } else {
    // TODO PARSE COLUMNS TO ARRAY
    char* column = strtok(columns, ",");
    char columnList[200][200];
    int columnCount = 0;
    while (column) {
      strcpy(columnList[columnCount], column);

      columnCount++;
      column = strtok (NULL, ",");
      while (column && *column == '\040')
          column++;
    }

    // CREATE VAR COMMAND $(f["foo"]), $(f["bar"])
    char varCmd[500];
    for(int i = 0; i < columnCount; i++){
      char eachvar[321];
      sprintf(eachvar, "$(f[\"%s\"])", columnList[i]);
      if (i != 0) {
        strcat(varCmd, ", ");
      }
      strcat(varCmd, eachvar);
    }

    // TODO AWK CMD
    char awkCmd[3000];
    sprintf(awkCmd, "awk -F\";\" \'NR==1 {for (i=1; i<=NF; i++) {f[$i] = i}}{ print %s }\' %s",varCmd, tablePath);

    char path[1035];
    fp = popen(awkCmd, "r");
    while (fgets(path, sizeof(path), fp) != NULL) {
      strcat(resultBuffer, path);
    }
    /* close */
    pclose(fp);
  }

  
  send(sock, resultBuffer, strlen(resultBuffer), 0);
  return;
}

void handleDelete(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  printf("🚀 [handleDelete()] first command: %s\n", buffer);

  if (!strlen(databaseUsed)) {
    sendError(sock, "No Database Used, try to run USE dbName;");
    return;
  }

  // TODO CHECK IF TABLE EXISTS
  char tablePath[2100];
  sprintf(tablePath, "databases/%s/%s", databaseUsed, buffer);
  FILE* fp = fopen(tablePath, "r");

  if (!fp) {
    sendError(sock, "No Database Found");
    return;
  }

  
  FILE* fp2 = fopen("temp", "w");

  char data[256];
  int count = 0;
  while (fgets(data, sizeof data, fp) != NULL) {
    printf("data: %s", data);

    // If the first 2 line
    if (count <= 1){
      fprintf(fp2, "%s", data);
    }
    count++;
    // bzero(data, 1024);
  }
  fclose(fp);
  fclose(fp2);
  remove(tablePath);
  rename("temp", tablePath);

  sendSuccess(sock);
  return;
}

void handleUpdate(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  printf("🚀 [handleUpdate()] first command: %s\n", buffer);


  char structure[1024] = {0};
  valread = read(sock, structure, 1024);
  structure[strlen(structure) - 1] = '\0';

  if (!strlen(databaseUsed)) {
    sendError(sock, "No Database Used, try to run USE dbName;");
    return;
  }

  // TODO CHECK IF TABLE EXISTS
  char tablePath[2100];
  sprintf(tablePath, "databases/%s/%s", databaseUsed, buffer);
  FILE* fp = fopen(tablePath, "r");

  if (!fp) {
    sendError(sock, "No Database Found");
    return;
  }

  // TODO COMPILE UPDATE QUERY
  char* queryTok = strtok(structure, "=");
  char queryTable[100], queryValue[100];
  strcpy(queryTable, queryTok);
  queryTok = strtok(NULL, "=");
  strcpy(queryValue, queryTok);

  printf("queryTable: %s\n",  queryTable);
  printf("queryValue: %s\n",  queryValue);
  
  FILE* fp2 = fopen("temp", "w");

  char data[256];
  int count = 0, matchIndex;
  while (fgets(data, sizeof data, fp) != NULL) {
    printf("data: %s", data);
    // remove '\n'
    data[strcspn(data, "\n")] = 0;

    // If the first 2 line
    if (count == 0){
      fprintf(fp2, "%s\n", data);
      char* tabTok = strtok(data, ";");
      int tabTokCount = 0;
      while (tabTok) {
        if (!strcmp(tabTok, queryTable)) {
          matchIndex = tabTokCount;
        }
        tabTokCount++;
        tabTok = strtok(NULL, ";");
      }
    } else if (count == 1) {
      fprintf(fp2, "%s\n", data);
    } else  {
      char updatedRecord[200] = "";
      char* tabTok = strtok(data, ";");
      int tabTokCount = 0;
      while (tabTok) {
        // printf("tabtok: --%s--\n", tabTok);
        if (tabTokCount == matchIndex) {
          char temp[120];
          sprintf(temp, "%s;", queryValue);
          strcat(updatedRecord, temp);
        } else {
          char temp[120];
          sprintf(temp, "%s;", tabTok);
          strcat(updatedRecord, temp);
        }
        tabTokCount++;
        tabTok = strtok(NULL, ";");
      }
      fprintf(fp2, "%s\n", updatedRecord);
    }
    count++;
    // bzero(data, 1024);
  }
  fclose(fp);
  fclose(fp2);
  remove(tablePath);
  rename("temp", tablePath);

  sendSuccess(sock);
  return;
}

void handleDropColumn(int sock) {
  // WAITING FOR MENU
  int valread;
  char buffer[1024] = {0};
  valread = read(sock, buffer, 1024);
  printf("🚀 [handleUpdate()] first command: %s\n", buffer);
  buffer[strlen(buffer) - 1] = '\0';


  char queryTable[1024] = {0};
  valread = read(sock, queryTable, 1024);
  printf("🚀 [handleUpdate()] queryTable: %s\n", queryTable);

  if (!strlen(databaseUsed)) {
    sendError(sock, "No Database Used, try to run USE dbName;");
    return;
  }

  // TODO CHECK IF TABLE EXISTS
  char tablePath[2100];
  sprintf(tablePath, "databases/%s/%s", databaseUsed, buffer);
  FILE* fp = fopen(tablePath, "r");

  if (!fp) {
    sendError(sock, "No Database Found");
    return;
  }

  // // TODO COMPILE UPDATE QUERY
  // char* queryTok = strtok(structure, "=");
  // char queryTable[100], queryValue[100];
  // strcpy(queryTable, queryTok);
  // queryTok = strtok(NULL, "=");
  // strcpy(queryValue, queryTok);

  // printf("queryTable: %s\n",  queryTable);
  // printf("queryValue: %s\n",  queryValue);
  
  FILE* fp2 = fopen("temp", "w");

  char data[256];
  int count = 0, matchIndex;
  while (fgets(data, sizeof data, fp) != NULL) {
    printf("data: %s", data);
    // remove '\n'
    data[strcspn(data, "\n")] = 0;

    // If the first 2 line
    if (count == 0){
      char updatedRecord[200] = "";
      char* tabTok = strtok(data, ";");
      int tabTokCount = 0;
      while (tabTok) {
        if (!strcmp(tabTok, queryTable)) {
          matchIndex = tabTokCount;
        } else {
          char temp[120];
          sprintf(temp, "%s;", tabTok);
          strcat(updatedRecord, temp);
        }
        tabTokCount++;
        tabTok = strtok(NULL, ";");
      }
      fprintf(fp2, "%s\n", updatedRecord);
    } else  {
      char updatedRecord[200] = "";
      char* tabTok = strtok(data, ";");
      int tabTokCount = 0;
      while (tabTok) {
        // printf("tabtok: --%s--\n", tabTok);
        if (tabTokCount == matchIndex) {
          // do nothing
        } else {
          char temp[120];
          sprintf(temp, "%s;", tabTok);
          strcat(updatedRecord, temp);
        }
        tabTokCount++;
        tabTok = strtok(NULL, ";");
      }
      fprintf(fp2, "%s\n", updatedRecord);
    }
    count++;
    // bzero(data, 1024);
  }
  fclose(fp);
  fclose(fp2);
  remove(tablePath);
  rename("temp", tablePath);

  sendSuccess(sock);
  return;
}

void handleStopConnection(int sock) {
  connection--;

  // ambil connection sebelumnya
  // currentConnection--;
  // go to next connection in queue
  queue++;
  printf("🐮 connection:  %d\n", connection);
  printf("🐮 currentConnection:  %d\n", currentConnection);
  printf("🐮 queue:  %d\n", queue);

  printf("🚥 sending signal to queue: %d is closed to next connectio\n", queue);
  send(id_socket[queue], "go", strlen("go"), 1024);

  // Reset count connection and use the available tid;
  if (queue == currentConnection) {
    queue = 0;
    currentConnection = 0;
    printf("🐮🕓 currentConnection:  %d\n", currentConnection);
    printf("🐮🕓 queue:  %d\n", queue);
  }

  pthread_cancel(pthread_self());
}
# FP

## Implemented

Bagaimana Program Diakses

- [x] Berjalan sebagai daemon
- [x] Untuk bisa akses console database, perlu buka program client (kalau di linux seperti command mysql di bash)
- [x] Program client dan utama berinteraksi lewat socket
- [x] Program client bisa mengakses server dari mana aja

Autentikasi

- [ ] LOGIN
- [ ] LOGIN SUDO
- [x] CREATE USER
- [x] USE (belum ngecek)
- [x] GRANT PERMISSION

Data Definition Language

- [x] CREATE DATABASE
- [x] CREATE TABLE
- [x] DROP DATABASE
- [x] DROP TABLE
- [x] DROP COLUMN

Data Manipulation Language

- [x] INSERT
- [x] UPDATE
- [x] DELETE
- [x] SELECT \*
- [x] SELECT KOLOM
- [ ] WHERE

Logging

- [x] Logging sesuai format (min user)

Reliability

- [ ] program dump
- [x] cron & zip bash

Tambahan Requirement Minimum

- [x] SHOW DB (melihat list database)
- [x] Multiconnection

## Data Definition Language

### CREATE DATABASE

Pada create database, yang dilakukan pada client adalah dengan mengirim nama database dan key `database` ke server.

```c
else if (!strcmp(type, "DATABASE")) {
  char dbName[100];
  scanf("%s", dbName);
  send(sock, "database", strlen("database"), 0);
  sleep(0.2);
  promptDatabase(sock, dbName);
  sprintf(commander,"%s %s %s",command,type,dbName);
}
```

Sedangkan pada server, yang dilakukan adalah dengan membuat database baru yaitu berupa folder menggunakan mkdir -p, yang berarti membuat parent directory databases terlebih dahulu apabila belum ada. Dan membuat sebuah file yaitu dblist.txt untuk keeptrack.

```c
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
```

### CREATE TABLE

Pada create table, pengiriman pada client ada 2 tahap yaitu mengirim nama tabel yang ingin dibuat, dan mengirim rincian value dan tipe datanya.

```c
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
}
```

Pada server, yang dilakukan adalah dengan membersihkan value input yang didapat, kemudian melakukan checking untuk database yang telah digunakan kemudianmembtuat tabelnya dengan membuat sebuah file di dalam database yang telah digunakan kemudian menspecify tipe data" sesuai input dan menyimpan ke dalam file tabel.

```c
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
```

Format penyimpanan dalam tabel adalah sebagai berikut

```
kolom1;kolom2;kolom3
string;int;string
'a';100;'b'
'a';100;'b'
'a';100;'b'
```

### DROP DATABASE

Pada drop database, pada client adalah dengan mengirimkan nama database yang ingin didrop.

```c
else if (!strcmp(type, "DATABASE")) {
  char dbName[100];
  scanf("%s", dbName);
  send(sock, "drop-db", strlen("drop-db"), 0);
  sleep(0.2);
  promptDropDb(sock, dbName);
  sprintf(commander,"%s %s %s",command,type,dbName);
}
```

Kemudian pada server, yang pertama adalah dilakukan jika database yang ingin didelete ada yaitu dengan mengecek dblist yang ada, kemudian kita ingin mendelete nama database yang ingin dihapus dari file dblist.txt, kemudian setelah berhasil, maka kita bisa langsung menghapus dengan meremove menggunakan `rm -rf` sehingga terhapus secara rekursif.

```c
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
```

### DROP COLUMN

Pada pengedropan column, karena ada 2 parameter yang harus dikirim ke server database yaitu nama kolom yang ingin di drop dan nama tabelnya, maka pada client akan melakukan 2 kali input

```c
else if (!strcmp(type, "COLUMN")) {
  char columnName[100], dbName[100];
  scanf("%s FROM %s", columnName, dbName);
  send(sock, "drop-column", strlen("drop-column"), 0);
  sleep(0.2);
  promptDropColumn(sock, dbName, columnName);
  sprintf(commander,"%s %s %s",command,type,dbName);
}

// Kurang lebih isi prompt akan selalu seperti ini.
void promptDropColumn(int sock, char* str, char* structure) {
  // Send tableName
  send(sock, str, strlen(str), 0);
  sleep(0.1);

  // send set properties
  send(sock, structure, strlen(structure), 0);
  return;
}
```

Kemudian setelah di send ke server yang dilakukan adalah dengan mengecek terlebih dahulu akses database, kemudian mengecek apabila table yang ditentukan ada atau tiadk, kemudian mengambil input yaitu kolom yang ingin di drop, kemudian kita bisa mengetahui apa yang ingin kita drop. Kemudian setelah kita tahu kolom yang ingin kita drop, maka kita bisa membaca table terkait, kemudian mengcopy tanpa kolom yang ingin kita drop. Hal ini berlaku untuk semua baris. Pada saat mendelete, kami meng keep-track index ke berapa kolom tersebut, dan mengexclude pada saat menulis ulang file.

```c
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
      updatedRecord[strlen(updatedRecord) - 1] = '\0';
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
      updatedRecord[strlen(updatedRecord) - 1] = '\0';
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
```

## Data Manipulation Language

### INSERT

Pada insert, yang dilakukan adalah dengan mengambil nama table, dan value" yang ingin kita insert.

```c
else if (!strcmp(command, "INSERT")) {
  char tableName[100];
  char tableStructure[100];
  scanf(" INTO %s ", tableName);
  getchar();
  scanf("%[^\n]s)", tableStructure);
  send(sock, "insert", strlen("insert"), 0);
  sleep(0.2);
  promptInsert(sock, tableName, tableStructure);
}
```

Pada server, yang kita lakukan pertama seperti biasa adalah dengan mengecek database, kemudian mengecek apabila tabel ada. Kemudian membaca atribut value yang dikirim, kemudian mengecek apabila database yang ada di dalam tipenya sama dengan tipe yang ingin diinsert, jika tidak sama maka akan disend error. Pengecekan tipe dilakukan dengan membaca line ke dua dari tabel, dan mencocokan dengan tipe data yang di insertkan. Kemudian setelah aman, maka yang perlu dilakukan adalah tinggal mengappend nilai value yang baru saja di tambahkan sesuai dengan format database.

```c
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
```

### UPDATE

Pada update, approach yang diambil kurang lebih mirip pada saat insert, yaitu dengan mengambil nama tabel, dan kolom yang ingin diubah.

```c
else if (!strcmp(command, "UPDATE")) {
  char tableName[100];
  scanf("%s", tableName);
  send(sock, "update", strlen("update"), 0);
  char updateQuery[100];
  scanf(" SET %s", updateQuery);


  sleep(0.2);
  promptUpdate(sock, tableName, updateQuery);
  sprintf(commander,"%s %s SET %s",command, tableName, updateQuery);
}
```

Pada server, hal pertama yang dilakukan adalah dengan mengecek error seperti biasa, kemudian membersihkan input yang masuk dengan memisahkan value `kolom=value` ke variable masing" dengan menggunakan strtok. Kemudian dari data yang sudah didapat, kita bisa mengecek kolom tersebut termasuk ke dalam index berapa dalam tabel, kemudian kita tinggal melakukan penulisan ulang tabel yang lama, ke tabel temporary, tetapi dengan mengganti pada index ke sekian ke value yang diberikan.

```c
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
      updatedRecord[strlen(updatedRecord) - 1] = '\0';
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
```

### DELETE

Pada saat delete, maka yang perlu dikirim adalah hanya nama tabelnya saja.

```c
else if (!strcmp(command, "DELETE")) {
  char tableName[100];
  scanf("FROM %s", tableName);
  send(sock, "delete", strlen("delete"), 0);

  sleep(0.2);
  promptDelete(sock, tableName);
  sprintf(commander,"%s FROM %s",command, tableName);
}
```

Pada server, untuk pendeletan cukup simpel yaitu dengan menulis ulang tabel yang sudah ada, tetapi kita hanya menulis 2 baris pertama saja, karena kita ingin data yang sudah dibuat hilang.

```c
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
```

### SELECT \* & KOLOM

Pada select akan ada 2 kategori yaitu menggunakan \* dan menspecify" kolom" yang ada.

```c
else if (!strcmp(command, "SELECT")) {
  char commands[200];
  scanf("%[^\n]s", commands);
  send(sock, "select", strlen("select"), 0);

  sleep(0.2);
  promptSelect(sock, commands);
  sprintf(commander,"%s %s",command, commands);
}
```

Kemudian pada server, pengecekan \* atau kolom dilakukan. Jika dilakukan select \* maka kita hanya perlu ngeprint semua tabel yang ada dari tabel. Tetapi jika ada kolom yang ditentukan, maka kita bisa mengambil kolom" itu dan menjalankan script awk untuk mempermudah pengambilan data.
Script awk yang dilakukan adalah dengan menyimpan semua kolom ke dalam sebuah array, kemudian menampilkan array tersebut sesuai dengan input yang diberikan. Dengan menggunakan pemanggilan system, untuk mengambil output yang diberikan, kita dapat menggunakan `popen`.

```c
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
```

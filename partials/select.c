#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

int main() {
  char command[100];
  printf("⏰ waiting promptDataManipulation\n");
  scanf("%s ", command);

  char commands[200];
  scanf("%[^\n]s", commands);

  // TODO GET COLUMNS AND TABLENAME
  char columns[200], tableName[200];

  char* token = strtok(commands, "FROM");
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

  printf("columns: --%s--\n", columns);
  printf("tableName: --%s--\n", tableName);

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

  char varCmd[500];
  for(int i = 0; i < columnCount; i++){
    char eachvar[321];
    sprintf(eachvar, "$(f[\"%s\"])", columnList[i]);
    if (i != 0) {
      strcat(varCmd, ", ");
    }
    strcat(varCmd, eachvar);
  }

  printf("varCmd: %s\n", varCmd);
  char tablePath[100] = "../database/databases/haha/testing";
  char awkCmd[1000];
  sprintf(awkCmd, "awk -F\";\" \'NR==1 {for (i=1; i<=NF; i++) {f[$i] = i}}{ print %s }\' %s",varCmd, tablePath);

  FILE *fp;
  char path[1035];
  /* Open the command for reading. */
  fp = popen(awkCmd, "r");
  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path), fp) != NULL) {
    printf("%s", path);
  }
  /* close */
  pclose(fp);


}
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main() {
  char tableStructure[200], tableName[200];
  scanf("%s ", tableName);
  getchar();
  scanf("%[^\n]s)", tableStructure);

  // remove ) and ;
  tableStructure[strlen(tableStructure) - 2] = '\0';

  // TODO GET TYPES
  FILE* fp = fopen("../database/databases/hello/clarence", "ra+");
  int count = 0;
  char line[256];
  while (fgets(line, sizeof line, fp) != NULL) {
    if (count == 1) {
      break;
    } else count++;
  }

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

  char* token = strtok (tableStructure, ",");
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
    return 0;
  } else {
    insertBuffer[strlen(insertBuffer) - 1] = '\0'; 
    fprintf(fp, "%s\n", insertBuffer);
  }

  fclose(fp);
}
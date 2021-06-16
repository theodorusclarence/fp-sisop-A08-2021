#include <stdio.h>
#include <string.h>
#include <ctype.h>

int main() {
  char tableStructure[200], nama[200];
  scanf("%s ", nama);
  getchar();
  scanf("%[^\n]s)", tableStructure);

  // remove ) and ;
  tableStructure[strlen(tableStructure) - 2] = '\0';

  FILE* fp = fopen("test", "w");
  
  char* token = strtok (tableStructure, ",");
  char columnNameLine[1000] = "", columnTypeLine[1000] = "";
  while (token) {
    printf ("token: %s\n", token);         
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
}
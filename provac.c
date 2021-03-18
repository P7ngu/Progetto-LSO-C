#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(void){
    printf("1");
    int flag=0;
    printf("1");

    char *buff;
    printf("1");
    char* user;
    char *pass;
    char good[] = "1 1 3 4";

     char data[] = "1233333333#4566666666#7899999999";


char part1[11];
char part2[11];
char part3[11];

memmove(part1, &data[0], 10);
part1[10] = '\0';
memmove(part2, &data[10], 10);
part2[10] = '\0';
memmove(part3, &data[20], 10);
part3[10] = '\0';

printf("%s - %s - %s", part1, part2, part3);

}


  

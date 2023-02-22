#include <stdio.h>
#include <stdlib.h>

int main()
{
  char str[30] = "20.30300 This is test";
   char *ptr;
   double ret;

   ret = strtod(str, &ptr);
   printf("number part is -> %lf\n", ret);
   printf("str part is --> %s\n", ptr);

   return(0);
}
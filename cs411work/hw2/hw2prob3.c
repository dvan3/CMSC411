/**
File: hw2prob3.c
Author: Dave Van
Date: 2/28/12
E-mail: dvan3@umbc.edu
**/

#include <stdio.h>
#include <stdlib.h>

int main()
{
   long divisor;
   long dividend;
   long long rem;

   printf("Input divisor: ");
   scanf("%ld", &divisor);
   printf("\n");

   printf("Input dividend: ");
   scanf("%ld", &dividend);
   printf("\n");

   rem = dividend;

   rem = rem << 1;

   for(int i = 0; i < 32; i++)
   {
      rem = rem - ((long long)divisor << 32);

      if(rem < 0)
      {
	 rem = rem + ((long long)divisor << 32);
	 rem = rem << 1;
      }
      else
      {
         rem = rem << 1;
         rem = rem | 0x1;
      }
      printf("Remainder = 0x%016llx\n", rem);
      printf("Divisor = 0x%016lx\n", divisor);
      printf("\n");
   }

   printf("Quotient = 0x%016llx\n", (rem & 0x00000000FFFFFFFF));
   
   printf("Remainder = 0x%016llx\n", rem >> 33);

   return 0;
}

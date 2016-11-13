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
   unsigned int divisor;
   unsigned int quotient;
   unsigned int rem;
   int divisor2;
   int rem2;
   int quotient2;

   quotient = 0;

   printf("Input divisor: ");
   scanf("%d", &divisor);
   printf("\n");

   printf("Input Second number: ");
   scanf("%d", &rem);
   printf("\n");

   divisor2 = divisor;
   rem2 = rem;
   quotient2 = divisor2 / rem2;

   printf("Initial Values\n");
   printf("Divisor = %d\n", divisor);
   printf("Remainder = %d\n", rem);
   printf("Quotient = %d\n", quotient);
   printf("\n");

   for(int i = 0; i < 32; i++)
   {
      rem = rem - divisor;
      if(rem < 0)
      {
	 rem = rem + divisor;
	 quotient = quotient << 1;
      }
      else if(rem >= 0)
      {
	 quotient = quotient >> 1;
      }
      divisor = divisor >> 1;
   }

   printf("Real Quotient = %d\n", quotient2);
   printf("FINAL QUOTIENT = %d\n", quotient);

   return 0;
}

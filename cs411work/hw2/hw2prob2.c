/**
File: hw2prob2.c
Author: Dave Van
Date: 2/28/12
E-mail: dvan3@umbc.edu
**/

#include <stdio.h>
#include <stdlib.h>

int main()
{
   unsigned long long multiplicand;
   unsigned long long product;
   long long product2;
   product = 0;

   printf("Input multiplier: ");
   scanf("%lld", &product);
   printf("\n");

   printf("Input multiplicand: ");
   scanf("%lld", &multiplicand);
   printf("\n");

   product2 = product * multiplicand;

   printf("Initial Values");
   printf("Multiplicand = %lld\n", multiplicand);
   printf("Product = %lld\n", product);
   printf("\n");

   for(int i = 0; i <= 31; i++)
   {
      if((product & 0x1) == 1 )
      {
	 product = product + (long long)(multiplicand << 32);
      }
      printf("Multiplicand = 0x%016llx\n", multiplicand);
      printf("Product = 0x%016llx\n", product);
      printf("\n");

      product = product >> 1;
   }
   
   if(product2 == product)
   {
      printf("FINAL PRODUCT = %lld\n", product);
   }
   else
   {
      printf("Dave, you fail at life. You should quit or die. =[\n");
   }

   return 0;
}

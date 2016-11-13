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
   unsigned int multiplier;
   unsigned int multiplicand;
   unsigned int product;
   int multiplier2;
   int multiplicand2;
   int product2;

   product = 0;

   printf("Input multiplier: ");
   scanf("%d", &multiplier);
   printf("\n");

   printf("Input multiplicand: ");
   scanf("%d", &multiplicand);
   printf("\n");

   multiplier2 = multiplier;
   multiplicand2 = multiplicand;
   product2 = multiplier2 * multiplicand2;

   printf("Initial Values");
   printf("Multiplier = %d\n", multiplier);
   printf("Multiplicand = %d\n", multiplicand);
   printf("Product = %d\n", product);
   printf("\n");

   for(int i = 0; i < 31; i++)
   {
      if((multiplier & 0x1) == 1 )
      {
	 product = product + multiplicand;
	 multiplicand = multiplicand;
	 multiplicand = multiplicand << 1;
	 multiplier = multiplier >> 1;

	 printf("Multiplier = %d\n", multiplier);
	 printf("Multiplicand = %d\n", multiplicand);
	 printf("Product = %d\n", product);
	 printf("\n");
      }
      else
      {
	 multiplicand = multiplicand;
	 multiplicand = multiplicand << 1;
	 multiplier = multiplier >> 1;

	 printf("Multiplier = %d\n", multiplier);
	 printf("Multiplicand = %d\n", multiplicand);
	 printf("Product = %d\n", product);
	 printf("\n");
      }
   }
   
   if(product2 == product)
   {
      printf("FINAL PRODUCT = %d\n", product);
   }
   else
   {
      printf("Dave, you fail at life. You should quit. =[\n");
   }
   return 0;
}

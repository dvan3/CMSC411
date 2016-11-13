#include <stdio.h>

void deconstructFloat(unsigned int x) {
    float f = *(float *)&x;	    /* as actual float */
    unsigned int s = x>>31;	    /* sign bit */
    unsigned int e = x>>23 & 0xff;  /* exponent bits */
    unsigned int denorm = e==0;	    /* 1 if denormalized */
    unsigned int m = x & 0x7fffff;  /* mantissa bits */
    printf("f=%-10.8g=0x%06x*2^%d; s=%d e=0x%02x=%d(2^%d) m=0x%06x x=0x%08x\n",
	   f,				  /* orginal float */
	   !denorm<<23|m,denorm+e-127-23, /* with integer mantissa */
	   s, e,e, denorm+e-127, m,	  /* decompose to components */
	   x);				  /* final encoding */
}

int main(int argc, const char *argv[]) {
    if (argc != 2) {
	printf("usage: float <float>\n");
	printf("  prints number next smaller than given float, the float, and next larger\n");
	printf("  for each, print:\n");
	printf("    value as decimal/scientific notation\n");
	printf("    mantissa (with leading 1) and exponent. Scaled so mantissa is an integer\n");
	printf("    sign\n");
	printf("    exponent (in hex, decimal, and effective exponent after bias\n");
	printf("    mantissa\n");
	printf("    full binary encoding for float\n");
	return 1;
    }
	       
    float f; sscanf(argv[1], "%f", &f); /* parse float from command line */
    unsigned int i = *(unsigned int*)&f; /* cast to integer */

    deconstructFloat(i-1);		/* print one smaller */
    deconstructFloat(i);		/* print number */
    deconstructFloat(i+1);		/* print one bigger */

    return 0;
}

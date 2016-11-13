This is a simple program that decomposes a floating point number into
its constituent parts and prints them out. Also prints the next larger
and next smaller float.

Note that the next larger/smaller floats are computed by incrementing/
decrementing the integer representation, so give the next further from
zero or the next closer to zero for negative floats, and report NaN
for the next float smaller than 0 (since it underflows into a number
with all 1's in the exponent).

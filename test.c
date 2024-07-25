#include <stdbool.h>
#include <stdio.h>
#include "round.c"

#define CONST_PI 3.141592653589793

int main()
{
    // Initialize array with arbitrary numbers
    double X[] = {(double)5/3, CONST_PI, INFINITY };

    // Define parameters for target format
    // By default it performs Round to Nearest
    // To perfom Stochastic Rounding, set stochasticRounding bool to true
    int precision = 11;
    int emax = 15;
    int bitWidth = 8;
    bool stochasticRounding = false;

    // Do not modify the code below this line
    int numX = sizeof(X) / sizeof(X[0]);
    doRound(X, numX, precision, emax, bitWidth, stochasticRounding);
}

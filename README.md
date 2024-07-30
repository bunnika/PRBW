# PRBW: Pseudo-Random Bit-Width

PRBW is a C library for simulating stochastic rounding of double-precision floating-point numbers to target precision formats with variable pseudo-random bit-width values. The target format (low-precision format) follows an extension of the formats defined in the IEEE 754 standard [1]. The library features the following parameters:

- `precision`: The number of bits for the significand (mantissa) in the target precision format. It determines the accuracy of the representation.
- `emax`: The maximum exponent value for the target precision format. It controls the range of representable values.
- `bitWidth`: The bit-width used in the stochastic rounding process. It influences the randomness and the precision of the rounding.
- `stochasticRounding`: A boolean to turn stochastic rounding on or off. It performs stochastic rounding when set to `true`; otherwise, round to nearest.

Below is a code snippet that demonstrates how to perform stochastic rounding:

```c
#include <stdbool.h>
#include <stdio.h>
#include "round.c"

#define CONST_PI 3.141592653589793

int main()
{
    // Initialize array with arbitrary numbers
    double X[] = {(double)5/3, CONST_PI, INFINITY};

    // Define parameters for target format
    // By default, it performs Round to Nearest
    // To perform Stochastic Rounding, set stochasticRounding bool to true
    int precision = 11;
    int emax = 15;
    int bitWidth = 8;
    bool stochasticRounding = true;

    // Do not modify the code below this line
    int numX = sizeof(X) / sizeof(X[0]);
    doRound(X, numX, precision, emax, bitWidth, stochasticRounding);

    return 0;
}
```
## References

[1] 754-2019 IEEE Standard for Floating-Point Arithmetic, pp. 1–84, Institute of Electrical and Electronics Engineers, July 2019. Revision of IEEE Std 754-2008.

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define INITRAND(seed) srand(seed)

int doubleExponentBits = 11;
int doubleExponentBias = 1023;
int doubleSignificandBits = 52;
int doubleSignificandShiftDenormalised = 53;

uint64_t doubleSignMask = 0x8000000000000000u;
uint64_t doubleExponentMask = 0x7FF0000000000000u;
uint64_t doubleSignificandMask = 0x000FFFFFFFFFFFFFu;

uint64_t outMask(int start, int length)
{
    return ((1ULL << length) - 1) << start;
}

uint64_t randomDouble()
{
    // Combine two 32-bit random X to create a 64-bit random number
    uint64_t upper = ((uint64_t)rand() << 32);
    uint64_t lower = (uint64_t)rand() & 0xFFFFFFFF;
    return upper | lower;
}

int toCustomRN(void *source, void *target, int precision, int emax)
{
    // Pointer to custom precision
    uint64_t *cp = (uint64_t *) target;
    // Pointer to double precision
    uint64_t *dp = (uint64_t *) source;

    uint64_t doubleSign, doubleExponent, doubleSignificand;
    uint64_t customSign, customExponent, customSignificand;

    int customExponentValue;
    int customExponentBits = (int)log2(emax + 1) + 1; // w
    int customSignificandBits = precision - 1; // t
    int customExponentBias = emax; // emax
    int customSignShift = customExponentBits + precision; // k = w + precision
    int customSignificandNormalised = doubleSignificandBits - customSignificandBits;

    uint64_t customInfinity = outMask(customSignificandBits, customExponentBits);
    uint64_t customExponentMask = outMask(customSignificandBits, customExponentBits);
    uint64_t customSignificandNaN = 1ULL << (customSignificandBits - 1);
    uint64_t customNaN = customExponentMask | customSignificandNaN;

    uint64_t d = *dp++;
    doubleSign = d & doubleSignMask;
    doubleExponent = d & doubleExponentMask;
    doubleSignificand = d & doubleSignificandMask;

    // Print extracted parts of double-precision value
    printf("binary64    : 0x%016llx\n", d);
    printf("Sign        : 0x%016llx\n", doubleSign);
    printf("Exponent    : 0x%016llx\n", doubleExponent);
    printf("Significand : 0x%016llx\n", doubleSignificand);

    if ((d & ~doubleSignMask) == 0)
    {
        // Zero
        *cp++ = (doubleSign >> customSignShift);
    }
    else if (doubleExponent == doubleExponentMask)
    {
        // Infinity or NaN
        if (doubleSignificand == 0)
        {
            // Infinity
            *cp++ = (doubleSign >> customSignShift) | customInfinity;
        }
        else
        {
            // NaN
            *cp++ = customNaN;
        }
    }
    else
    {
        // Normalised number
        // Extract sign bit and shift it to custom-precision position
        customSign = doubleSign >> customSignShift;
        // Adjust exponent from double-precision to custom-precision
        customExponentValue = (int)(doubleExponent >> doubleSignificandBits) - doubleExponentBias + customExponentBias;

        if (customExponentValue >= (1 << customExponentBits) - 1)
        {
            // Overflow
            *cp++ = (doubleSign >> customSignShift) | customInfinity;
        }
        else if (customExponentValue <= 0)
        {
            // Underflow or denormalised
            if ((customSignificandBits + 1 - customExponentValue) > doubleSignificandShiftDenormalised)
            {
                // Too small, set significand to zero
                customSignificand = 0;
            }
            else
            {
                // Denormalised number
                // Add hidden leading bit
                doubleSignificand |= 1ULL << doubleSignificandBits;
                customSignificand = doubleSignificand >> (doubleSignificandShiftDenormalised - customExponentValue + customSignificandBits);
                if ((doubleSignificand >> (doubleSignificandShiftDenormalised - customExponentValue + customSignificandBits - 1)) & 1)
                {
                    // Round significand
                    customSignificand++;
                }
            }
            *cp++ = customSign | customSignificand;
        }
        else
        {
            // Normalised custom-precision number
            // Shift adjusted exponent to custom-precision position
            customExponent = (uint64_t)(customExponentValue << customSignificandBits);
            // Shift significand to fit in custom-precision
            customSignificand = doubleSignificand >> customSignificandNormalised;
            
            if (doubleSignificand & (1ULL << (customSignificandNormalised - 1)))
            {
                // Round significand
                customSignificand++;
            }
            *cp++ = customSign | customExponent | customSignificand;
        }
    }

    return 0;
}

int toCustomSR(void *source, void *target, int precision, int emax, int bitWidth)
{
    // Pointer to custom precision
    uint64_t *cp = (uint64_t *) target;
    // Pointer to double precision
    uint64_t *dp = (uint64_t *) source;

    uint64_t doubleSign, doubleExponent, doubleSignificand;
    uint64_t customSign, customExponent, customSignificand;

    uint64_t customLeastSignificand;
    uint64_t customRandomLeastSignificand;

    int customExponentValue;
    int customExponentBits = (int)log2(emax + 1) + 1; // w
    int customSignificandBits = precision - 1; // t
    int customExponentBias = emax; // emax
    int customSignShift = customExponentBits + precision; // k = w + precision
    int customSignificandNormalised = doubleSignificandBits - customSignificandBits;

    uint64_t customInfinity = outMask(customSignificandBits, customExponentBits);
    uint64_t customExponentMask = outMask(customSignificandBits, customExponentBits);
    uint64_t customSignificandNaN = 1ULL << (customSignificandBits - 1);
    uint64_t customNaN = customExponentMask | customSignificandNaN;

    uint64_t d = *dp++;
    doubleSign = d & doubleSignMask;
    doubleExponent = d & doubleExponentMask;
    doubleSignificand = d & doubleSignificandMask;

    // Print extracted parts of double-precision value
    printf("binary64    : 0x%016llx\n", d);
    printf("Sign        : 0x%016llx\n", doubleSign);
    printf("Exponent    : 0x%016llx\n", doubleExponent);
    printf("Significand : 0x%016llx\n", doubleSignificand);

    if ((d & ~doubleSignMask) == 0)
    {
        // Zero
        *cp++ = (doubleSign >> customSignShift);
    }
    else if (doubleExponent == doubleExponentMask)
    {
        // Infinity or NaN
        if (doubleSignificand == 0)
        {
            // Infinity
            *cp++ = (doubleSign >> customSignShift) | customInfinity;
        }
        else
        {
            // NaN
            *cp++ = customNaN;
        }
    }
    else
    {
        // Normalised number
        // Extract sign bit and shift it to custom-precision position
        customSign = doubleSign >> customSignShift;
        // Adjust exponent from double-precision to custom-precision
        customExponentValue = (int)(doubleExponent >> doubleSignificandBits) - doubleExponentBias + customExponentBias;

        if (customExponentValue >= (1 << customExponentBits) - 1)
        {
            // Overflow
            *cp++ = (doubleSign >> customSignShift) | customInfinity;
        }
        else if (customExponentValue <= 0)
        {
            // Underflow or denormalised
            if ((customSignificandBits + 1 - customExponentValue) > doubleSignificandShiftDenormalised)
            {
                // Too small, set significand to zero
                customSignificand = 0;
            }
            else
            {
                // Denormalised number
                // Add hidden leading bit
                doubleSignificand |= 1ULL << doubleSignificandBits;
                customSignificand = doubleSignificand >> (doubleSignificandShiftDenormalised - customExponentValue + customSignificandBits);
                if ((doubleSignificand >> (doubleSignificandShiftDenormalised - customExponentValue + customSignificandBits - 1)) & 1)
                {
                    // Round significand
                    customSignificand++;
                }
            }
            *cp++ = customSign | customSignificand;
        }
        else
        {
            // Normalised custom-precision number
            // Shift adjusted exponent to custom-precision position
            customExponent = (uint64_t)(customExponentValue << customSignificandBits);
            // Shift significand to fit in custom-precision
            customSignificand = doubleSignificand >> customSignificandNormalised;

            uint64_t customLeastSignificandMask = outMask(0, customSignificandNormalised);
            customLeastSignificand = d & customLeastSignificandMask;

            //INITRAND(time(NULL));
            INITRAND(clock());
            uint64_t customRandomDouble = randomDouble();
            customRandomLeastSignificand = customRandomDouble & customLeastSignificandMask;

            uint64_t customBitWidthMask = outMask(customSignificandNormalised - bitWidth, customSignificandNormalised);
            uint64_t customBitWidth = customRandomLeastSignificand & customBitWidthMask;

            if (customRandomLeastSignificand < customLeastSignificand)
            {
                // Round significand
                customSignificand++;
            }
            *cp++ = customSign | customExponent | customSignificand;
        }
    }

    return 0;
}

int toDouble(void *source, void *target, int precision, int emax)
{
    // Pointer to custom-precision
    uint64_t *cp = (uint64_t *) source;
    // Pointer to double-precision (64-bit) output
    uint64_t *dp = (uint64_t *) target;

    uint64_t customSign, customExponentValue, customSignificand;
    uint64_t doubleSign, doubleExponent, doubleSignificand;
    
    int doubleExponentValue;

    int customExponentBits = (int)log2(emax + 1) + 1; // w;
    int customSignificandBits = precision - 1; // t;
    int doubleSignShift = customExponentBits + precision; // k = w + precision
    int customExponentBias = emax;

    uint64_t customSignMask = outMask(64 - doubleSignShift - 1, 1);
    uint64_t customExponentMask = outMask(customSignificandBits, customExponentBits);
    uint64_t customSignificandMask = outMask(0, customSignificandBits);
    uint64_t customHiddenBitMask = outMask(customSignificandBits, 1);
    
    const uint64_t doubleInfinity = 0x7FF0000000000000;
    const uint64_t doubleNaN = 0xFFF8000000000000;

    uint64_t c = *cp++;
    customSign = c & customSignMask;
    customExponentValue = c & customExponentMask;
    customSignificand = c & customSignificandMask;

    // Print extracted parts of single-precision value
    printf("binaryCP    : 0x%08llx\n", c);
    printf("Sign        : 0x%08llx\n", customSign);
    printf("Exponent    : 0x%08llx\n", customExponentValue);
    printf("Significand : 0x%08llx\n", customSignificand);

    if (customExponentValue == 0)
    {
        // Zero or denormalised
        if (customSignificand == 0)
        {
            // Zero
            uint64_t zero = customSign << doubleSignShift;
            *dp++ = zero;
        }
        else
        {
            // Denormalised
            int e = -1;
            do
            {
                e++;
                customSignificand <<= 1;
            } while ((customSignificand & customHiddenBitMask) == 0);

            // Sign bit
            doubleSign = customSign << doubleSignShift;
            // Adjust exponent
            doubleExponentValue = -customExponentBias - e + doubleExponentBias;
            // Shift exponent to double-precision position
            doubleExponent = (uint64_t)doubleExponentValue << doubleSignificandBits;
            // Shift significand to double-precision position
            doubleSignificand = (uint64_t)(customSignificand & customSignificandMask) << (doubleSignificandBits - customSignificandBits);
            // Combine sign, exponent and significand
            uint64_t denor = doubleSign | doubleExponent | doubleSignificand;
            *dp++ = denor;
        }
    }
    else if (customExponentValue == customExponentMask)
    {
        // Infinity or NaN
        if (customSignificand == 0)
        {
            // Infinity
            uint64_t inf = (customSign << doubleSignShift) | doubleInfinity;
            *dp++ = inf;
        }
        else
        {
            // NaN
            *dp++ = doubleNaN;
        }
    }
    else
    {
        // Normalised
        // Sign bit
        doubleSign = customSign << doubleSignShift;
        // Adjust exponent
        doubleExponentValue = (int)(customExponentValue >> customSignificandBits) - customExponentBias + doubleExponentBias;
        // Shift exponent to double-precision position
        doubleExponent = (uint64_t)doubleExponentValue << doubleSignificandBits;
        // Add implicit leading bit and shift significand to double-precision position
        doubleSignificand = ((uint64_t)customSignificand << (doubleSignificandBits - customSignificandBits));

        // Combine sign, exponent and significand
        uint64_t nor = doubleSign | doubleExponent | doubleSignificand;
        *dp++ = nor;
    }
    
    return 0;
}

void doRound(double X[], int numX, int precision, int emax, int bitWidth, bool stochasticRounding)
{
    uint64_t *custom = (uint64_t *)malloc(sizeof(uint64_t));
    double *backDouble = (double *)malloc(sizeof(double));

    for (int i = 0; i < numX; ++i)
    {
        if (stochasticRounding)
        {
            toCustomSR(&X[i], custom, precision, emax, bitWidth);
        }
        else
        {
            toCustomRN(&X[i], custom, precision, emax);
        }

        toDouble(custom, backDouble, precision, emax);

        printf("Original Value (binary64) : %.15e\n", X[i]);
        printf("Rounded Value (binaryCP)  : %.15e\n", *backDouble);
        printf("\n");
    }

    free(custom);
    free(backDouble);
}

void doIteration(double X[], int numX, int precision, int emax, int bitWidth, int iterations)
{
    uint64_t *custom = (uint64_t *)malloc(sizeof(uint64_t));
    double *backDouble = (double *)malloc(sizeof(double));

    char fileName[32];
    snprintf(fileName, sizeof(fileName), "output/iter%dp%demax%dbw%d", iterations, precision, emax, bitWidth);

    FILE *file = fopen(fileName, "w");

    if (file == NULL)
    {
        perror("Failed to open file");
        free(custom);
        return;
    }

    for (int i = 0; i < numX; ++i)
    {
        for (int j = 0; j < iterations; j++)
        {
            toCustomSR(&X[i], custom, precision, emax, bitWidth);
            toDouble(custom, backDouble, precision, emax);

            printf("Original Value (binary64) : %.15e\n", X[i]);
            printf("Rounded Value (binaryCP)  : %.15e\n", *backDouble);
            fprintf(file, "%.15e\n", *backDouble);

            printf("\n");
        }
    }

    free(custom);
    free(backDouble);
    fclose(file);
}
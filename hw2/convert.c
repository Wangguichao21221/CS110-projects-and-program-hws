#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#define ca25_bias 0x7fffffff
#define fp32_bias 0x7f
#define all_ones 0xffffffff
void convert_to_fp32(unsigned sign, long long int unbiased, unsigned mantissa, char *ca_25_type)
{
    char type[10] = "normal\0";
    unsigned E = unbiased + fp32_bias;
    unsigned M = mantissa >> 9;
    unsigned leading = 1;
    if ((mantissa & 0x1ff) != 0 && strcmp(ca_25_type, "nan\0") != 0)
    {
        M += 1;
        if (M == 0x800000)
        {
            M = 0;
            E += 1;
        }
    }
    if (unbiased < -126 && strcmp(ca_25_type, "nan\0") != 0)
    {
        strcpy(type, "subnormal\0");
        E = 0;
        int dif = -126 - unbiased;
        unsigned M_copy = M;
        M += (1 << 23);
        M >>= dif;
        if (M_copy & ((1 << dif) - 1))
        {
            M += 1;
        }
        if (strcmp(ca_25_type, "inf\0") != 0)
        {
            leading = 0;
        }
    }
    M <<= 1;
    if (strcmp(ca_25_type, "zero\0") == 0)
    {
        E = 0;
        M = 0;
        leading = 0;
        strcpy(type, "zero\0");
    }
    else if (strcmp(ca_25_type, "nan\0") == 0)
    {
        E = 0xff;
        strcpy(type, "nan\0");
    }
    else if ((strcmp(ca_25_type, "inf\0") == 0 || E >= 0xff))
    {
        E = 0xff;
        M = 0;
        strcpy(type, "inf\0");
    }
    printf("fp32 S=%d E=%02x M=%d.%06x %s\n", sign, E, leading, M, type);
    int32_t result = sign << 31 | E << 23 | M >> 1;
    printf("%08x\n", result);
}

void convert_to_ca25(uint64_t ca25)
{
    unsigned mantissa, sign, leading;
    unsigned long int exponent;
    mantissa = ca25 & ca25_bias;
    mantissa <<= 1;
    ca25 >>= 31;
    exponent = ca25 & all_ones;
    ca25 >>= 32;
    sign = ca25 & 0x1;
    leading = 1;
    char type[10] = "normal\0";
    if (exponent == 0xffffffff)
    {
        if (mantissa == 0)
        {
            strcpy(type, "inf\0");
        }
        else
        {
            strcpy(type, "nan\0");
        }
    }
    else if (exponent == 0)
    {
        leading = 0;
        if (mantissa == 0)
        {
            strcpy(type, "zero\0");
        }
        else
        {
            strcpy(type, "subnormal\0");
        }
    }
    long long int unbiased = exponent - ca25_bias;
    printf("ca25 S=%d E=%08x M=%d.%08x %s", sign, (unsigned)exponent, leading, mantissa, type);
    putchar('\n');
    convert_to_fp32(sign, unbiased, mantissa, type);
}
int main()
{
    uint64_t buffer;
    while (scanf("%lx", &buffer) == 1)
    {
        convert_to_ca25(buffer);
    }
    return 0;
}
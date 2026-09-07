#ifndef DEEP_BIGNUM_H
#define DEEP_BIGNUM_H

/* Scientific-notation big number: value = mantissa * 10^exponent
 * mantissa is kept in [1,10) (or 0 for the zero value). */
typedef struct {
    double mantissa;
    long long exponent;
} BigNum;

BigNum bn_zero(void);
BigNum bn_from_double(double d);
double bn_to_double(BigNum a);

BigNum bn_add(BigNum a, BigNum b);
BigNum bn_sub(BigNum a, BigNum b); /* clamps to zero if b > a */
BigNum bn_mul_scalar(BigNum a, double scalar);
BigNum bn_mul(BigNum a, BigNum b);

int bn_compare(BigNum a, BigNum b); /* -1, 0, 1 */
int bn_gte(BigNum a, BigNum b);

void bn_to_string(BigNum a, char *buf, int buflen);

#endif

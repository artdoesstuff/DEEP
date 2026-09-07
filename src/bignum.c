#include "bignum.h"
#include <math.h>
#include <stdio.h>

static BigNum bn_normalize(double m, long long e) {
    BigNum r;
    if (m == 0.0) { r.mantissa = 0.0; r.exponent = 0; return r; }
    int neg = m < 0;
    if (neg) m = -m;
    while (m >= 10.0) { m /= 10.0; e++; }
    while (m < 1.0) { m *= 10.0; e--; }
    if (neg) m = -m;
    r.mantissa = m;
    r.exponent = e;
    return r;
}

BigNum bn_zero(void) {
    BigNum r; r.mantissa = 0.0; r.exponent = 0; return r;
}

BigNum bn_from_double(double d) {
    if (d == 0.0) return bn_zero();
    return bn_normalize(d, 0);
}

double bn_to_double(BigNum a) {
    return a.mantissa * pow(10.0, (double)a.exponent);
}

BigNum bn_add(BigNum a, BigNum b) {
    if (a.mantissa == 0) return b;
    if (b.mantissa == 0) return a;
    BigNum big = a, small = b;
    if (b.exponent > a.exponent) { big = b; small = a; }
    long long diff = big.exponent - small.exponent;
    if (diff > 17) return big; /* smaller value is negligible at this scale */
    double sm = small.mantissa / pow(10.0, (double)diff);
    return bn_normalize(big.mantissa + sm, big.exponent);
}

BigNum bn_sub(BigNum a, BigNum b) {
    if (b.mantissa == 0) return a;
    if (a.mantissa == 0) return bn_zero();
    long long diff = a.exponent - b.exponent;
    if (diff > 17) return a;
    if (diff < -17) return bn_zero();
    if (diff >= 0) {
        double bshift = b.mantissa / pow(10.0, (double)diff);
        double m = a.mantissa - bshift;
        if (m < 0) return bn_zero();
        return bn_normalize(m, a.exponent);
    } else {
        double ashift = a.mantissa / pow(10.0, (double)(-diff));
        double m = ashift - b.mantissa;
        if (m < 0) return bn_zero();
        return bn_normalize(m, b.exponent);
    }
}

BigNum bn_mul_scalar(BigNum a, double scalar) {
    if (scalar == 0.0 || a.mantissa == 0.0) return bn_zero();
    return bn_normalize(a.mantissa * scalar, a.exponent);
}

BigNum bn_mul(BigNum a, BigNum b) {
    if (a.mantissa == 0 || b.mantissa == 0) return bn_zero();
    return bn_normalize(a.mantissa * b.mantissa, a.exponent + b.exponent);
}

int bn_compare(BigNum a, BigNum b) {
    if (a.mantissa == 0 && b.mantissa == 0) return 0;
    if (a.mantissa == 0) return -1;
    if (b.mantissa == 0) return 1;
    if (a.exponent != b.exponent) return a.exponent < b.exponent ? -1 : 1;
    if (fabs(a.mantissa - b.mantissa) < 1e-9) return 0;
    return a.mantissa < b.mantissa ? -1 : 1;
}

int bn_gte(BigNum a, BigNum b) {
    return bn_compare(a, b) >= 0;
}

static const char *suffix_for(long long e3) {
    static const char *suffixes[] = {
        "", "K", "M", "B", "T", "Qa", "Qi", "Sx", "Sp", "Oc", "No",
        "Dc", "Ud", "Dd", "Td", "Qad", "Qid", "Sxd", "Spd", "Ocd", "Nod", "Vg"
    };
    long long n = (long long)(sizeof(suffixes) / sizeof(suffixes[0]));
    if (e3 < 0 || e3 >= n) return NULL;
    return suffixes[e3];
}

void bn_to_string(BigNum a, char *buf, int buflen) {
    if (a.mantissa == 0.0) { snprintf(buf, buflen, "0"); return; }
    if (a.exponent < 3) {
        snprintf(buf, buflen, "%.2f", bn_to_double(a));
        return;
    }
    long long e3 = a.exponent / 3;
    int rem = (int)(a.exponent % 3);
    double m = a.mantissa * pow(10.0, (double)rem);
    const char *suf = suffix_for(e3);
    if (suf) {
        snprintf(buf, buflen, "%.2f%s", m, suf);
    } else {
        snprintf(buf, buflen, "%.3fe%lld", a.mantissa, a.exponent);
    }
}

#include "decimal.h"

static const unsigned int kSignMask = 0x80000000u;
static const unsigned int kScaleMask = 0x00FF0000u;
static const long double kMaxDecimal = 79228162514264337593543950335.0L;

static long double pow10_ld(int exponent) {
  long double result = 1.0L;
  while (exponent > 0) {
    result *= 10.0L;
    exponent--;
  }
  return result;
}

int get_sign(const decimal *value) {
  int result = 0;
  if (value != NULL) {
    unsigned int meta = (unsigned int)value->bits[3];
    result = (meta & kSignMask) != 0u;
  }
  return result;
}

void set_sign(decimal *value, int sign) {
  if (value != NULL) {
    unsigned int meta = (unsigned int)value->bits[3];
    if (sign) {
      meta |= kSignMask;
    } else {
      meta &= ~kSignMask;
    }
    value->bits[3] = (int)meta;
  }
}

int get_scale(const decimal *value) {
  int result = 0;
  if (value != NULL) {
    unsigned int meta = (unsigned int)value->bits[3];
    result = (int)((meta & kScaleMask) >> 16);
  }
  return result;
}

void set_scale(decimal *value, int scale) {
  if (value != NULL) {
    if (scale < 0)
      scale = 0;
    else if (scale > 28)
      scale = 28;
    unsigned int meta = (unsigned int)value->bits[3];
    meta &= ~kScaleMask;
    meta |= ((unsigned int)scale << 16);
    value->bits[3] = (int)meta;
  }
}

void clear_service_bits(decimal *value) {
  if (value != NULL) {
    int scale = get_scale(value);
    int sign = get_sign(value);
    value->bits[3] = 0;
    set_scale(value, scale);
    set_sign(value, sign);
  }
}

int is_zero(decimal value) {
  return (value.bits[0] == 0 && value.bits[1] == 0 && value.bits[2] == 0);
}

void decimal_zero(decimal *value) {
  if (value != NULL) {
    value->bits[0] = 0;
    value->bits[1] = 0;
    value->bits[2] = 0;
    value->bits[3] = 0;
  }
}

int get_bit(const decimal *value, int bit_position) {
  int result = 0;
  if (value != NULL && bit_position >= 0 && bit_position <= 95) {
    int word_index = bit_position / 32;
    int bit_index = bit_position % 32;
    unsigned int word = (unsigned int)value->bits[word_index];
    result = ((word >> bit_index) & 1u) != 0u;
  }
  return result;
}

void set_bit(decimal *value, int bit_position, int bit_value) {
  if (value != NULL && bit_position >= 0 && bit_position <= 95) {
    int word_index = bit_position / 32;
    int bit_index = bit_position % 32;
    unsigned int mask = 1u << bit_index;
    unsigned int word = (unsigned int)value->bits[word_index];
    if (bit_value) {
      word |= mask;
    } else {
      word &= ~mask;
    }
    value->bits[word_index] = (int)word;
  }
}

int shift_left(decimal *value) {
  int overflow = 0;
  if (value != NULL) {
    overflow = (value->bits[2] & 0x80000000) != 0;
    int carry2 = (value->bits[1] & 0x80000000) != 0;
    int carry1 = (value->bits[0] & 0x80000000) != 0;
    value->bits[0] <<= 1;
    value->bits[1] <<= 1;
    value->bits[2] <<= 1;
    if (carry1) value->bits[1] |= 1;
    if (carry2) value->bits[2] |= 1;
  }
  return overflow;
}

int shift_right(decimal *value) {
  int lost_bit = 0;
  if (value != NULL) {
    lost_bit = value->bits[0] & 1;
    int carry0 = (value->bits[1] & 1);
    int carry1 = (value->bits[2] & 1);
    value->bits[2] >>= 1;
    value->bits[1] >>= 1;
    value->bits[0] >>= 1;
    if (carry1) value->bits[1] |= 0x80000000;
    if (carry0) value->bits[0] |= 0x80000000;
  }
  return lost_bit;
}

int mul_by_ten(decimal *value) {
  int result = 1;
  if (value != NULL) {
    unsigned long long carry = 0;
    for (int i = 0; i < 3; ++i) {
      unsigned long long intermediate =
          (unsigned long long)(unsigned int)value->bits[i];
      intermediate = intermediate * 10ull + carry;
      value->bits[i] = (int)(intermediate & 0xFFFFFFFFull);
      carry = intermediate >> 32;
    }
    result = (carry == 0) ? 0 : 1;
  }
  return result;
}

void normalize(decimal *value_1, decimal *value_2) {
  if (!value_1 || !value_2) return;

  int s1 = get_scale(value_1);
  int s2 = get_scale(value_2);
  if (s1 == s2) return;

  if (s1 < s2) {
    int diff = s2 - s1;
    int i = 0;
    while (i < diff) {
      decimal tmp = *value_1;
      if (mul_by_ten(&tmp) != 0) {
        diff = i;
      } else {
        *value_1 = tmp;
        s1++;
        i++;
      }
    }
    set_scale(value_1, s1);
  } else {
    int diff = s1 - s2;
    int i = 0;
    while (i < diff) {
      decimal tmp = *value_2;
      if (mul_by_ten(&tmp) != 0) {
        diff = i;
      } else {
        *value_2 = tmp;
        s2++;
        i++;
      }
    }
    set_scale(value_2, s2);
  }
}

int compare_abs(decimal value_1, decimal value_2) {
  int result = 0;
  decimal a = value_1;
  decimal b = value_2;
  set_sign(&a, 0);
  set_sign(&b, 0);
  normalize(&a, &b);
  if ((unsigned int)a.bits[2] > (unsigned int)b.bits[2])
    result = 1;
  else if ((unsigned int)a.bits[2] < (unsigned int)b.bits[2])
    result = -1;
  else if ((unsigned int)a.bits[1] > (unsigned int)b.bits[1])
    result = 1;
  else if ((unsigned int)a.bits[1] < (unsigned int)b.bits[1])
    result = -1;
  else if ((unsigned int)a.bits[0] > (unsigned int)b.bits[0])
    result = 1;
  else if ((unsigned int)a.bits[0] < (unsigned int)b.bits[0])
    result = -1;
  return result;
}

static int extract_mantissa_and_sign(const decimal *value,
                                     long double *mantissa, int *sign,
                                     int *scale) {
  int result = 1;
  if (value != NULL && mantissa != NULL && sign != NULL && scale != NULL) {
    *mantissa = (unsigned int)value->bits[0];
    *mantissa += (unsigned int)value->bits[1] * 4294967296.0L;
    *mantissa += (unsigned int)value->bits[2] * 18446744073709551616.0L;
    *scale = get_scale(value);
    *sign = get_sign(value);
    result = 0;
  }
  return result;
}

static long double perform_bank_rounding(long double abs_val) {
  long double result = 0.0L;
  long double int_part_ld = floorl(abs_val);
  long double frac = abs_val - int_part_ld;
  if (frac > 0.5L) {
    result = int_part_ld + 1.0L;
  } else if (frac < 0.5L) {
    result = int_part_ld;
  } else {
    result =
        (fmodl(int_part_ld, 2.0L) == 0.0L) ? int_part_ld : int_part_ld + 1.0L;
  }
  return result;
}

static int convert_to_decimal(long double rounded, decimal *result) {
  int status = 1;
  if (result != NULL) {
    long double abs_rounded = fabsl(rounded);
    unsigned long long tmp0 =
        (unsigned long long)fmodl(abs_rounded, 4294967296.0L);
    abs_rounded = floorl(abs_rounded / 4294967296.0L);
    unsigned long long tmp1 =
        (unsigned long long)fmodl(abs_rounded, 4294967296.0L);
    abs_rounded = floorl(abs_rounded / 4294967296.0L);
    unsigned long long tmp2 =
        (unsigned long long)fmodl(abs_rounded, 4294967296.0L);
    abs_rounded = floorl(abs_rounded / 4294967296.0L);
    if (abs_rounded <= 0.0L) {
      result->bits[0] = (int)tmp0;
      result->bits[1] = (int)tmp1;
      result->bits[2] = (int)tmp2;
      result->bits[3] = 0;
      set_scale(result, 0);
      set_sign(result, rounded < 0.0L);
      status = 0;
    }
  }
  return status;
}

int bank_round(decimal value, decimal *result) {
  int status = 1;
  if (result != NULL) {
    long double mantissa = 0.0L;
    int sign = 0;
    int scale = 0;
    if (extract_mantissa_and_sign(&value, &mantissa, &sign, &scale) == 0) {
      long double scaled = mantissa / pow10_ld(scale);
      if (sign) scaled = -scaled;
      long double abs_val = fabsl(scaled);
      long double rounded = perform_bank_rounding(abs_val);
      if (scaled < 0.0L) rounded = -rounded;
      status = convert_to_decimal(rounded, result);
    }
  }
  return status;
}

int from_int_to_decimal(int src, decimal *dst) {
  int result = 1;
  if (dst != NULL) {
    decimal_zero(dst);
    unsigned int magnitude;
    if (src < 0) {
      magnitude = (src == INT_MIN) ? (unsigned int)((long long)INT_MAX + 1)
                                   : (unsigned int)(-src);
      set_sign(dst, 1);
    } else {
      magnitude = (unsigned int)src;
      set_sign(dst, 0);
    }
    dst->bits[0] = (int)magnitude;
    set_scale(dst, 0);
    result = 0;
  }
  return result;
}

int from_decimal_to_int(decimal src, int *dst) {
  int result = 1;
  if (dst != NULL) {
    long double mantissa = 0.0L;
    mantissa += (unsigned int)src.bits[0];
    mantissa += (unsigned int)src.bits[1] * 4294967296.0L;
    mantissa += (unsigned int)src.bits[2] * 18446744073709551616.0L;
    long double scaled = mantissa / pow10_ld(get_scale(&src));
    if (get_sign(&src)) scaled = -scaled;
    long double truncated = (scaled < 0.0L) ? ceill(scaled) : floorl(scaled);
    if (truncated >= (long double)INT32_MIN &&
        truncated <= (long double)INT32_MAX) {
      *dst = (int)truncated;
      result = 0;
    }
  }
  return result;
}

int from_decimal_to_float(decimal src, float *dst) {
  int result = 1;
  if (dst != NULL) {
    long double mantissa = 0.0L;
    mantissa += (unsigned int)src.bits[0];
    mantissa += (unsigned int)src.bits[1] * 4294967296.0L;
    mantissa += (unsigned int)src.bits[2] * 18446744073709551616.0L;
    long double scaled = mantissa / pow10_ld(get_scale(&src));
    if (get_sign(&src)) scaled = -scaled;
    if (fabsl(scaled) <= (long double)FLT_MAX) {
      *dst = (float)scaled;
      result = 0;
    }
  }
  return result;
}

int from_float_to_decimal(float src, decimal *dst) {
  int status = 1;
  if (dst) {
    decimal_zero(dst);
    long double x = src;
    int sign = 0;
    if (isnan(x) || isinf(x) || fabsl(x) > kMaxDecimal) {
      // overflow / invalid
    } else if (fabsl(x) > 0.0L && fabsl(x) < 1e-28L) {
      // underflow to zero
    } else {
      if (x < 0.0L) {
        sign = 1;
        x = -x;
      }
      int scale = 0;
      long double frac = x - floorl(x);
      while (frac > 0.0L && scale < 7) {
        x *= 10.0L;
        scale++;
        frac = x - floorl(x);
      }
      x = roundl(x);
      unsigned long long lo = (unsigned long long)fmodl(x, 4294967296.0L);
      unsigned long long mid =
          (unsigned long long)fmodl(x / 4294967296.0L, 4294967296.0L);
      unsigned long long hi =
          (unsigned long long)floorl(x / (4294967296.0L * 4294967296.0L));
      dst->bits[0] = (unsigned int)lo;
      dst->bits[1] = (unsigned int)mid;
      dst->bits[2] = (unsigned int)hi;
      dst->bits[3] = 0;
      set_scale(dst, scale);
      set_sign(dst, sign);
      status = 0;
    }
  }
  return status;
}

static unsigned int divide_by_10_u96(unsigned int *w2, unsigned int *w1,
                                     unsigned int *w0) {
  unsigned long long remainder = 0ULL;
  unsigned long long cur = (remainder << 32) | (unsigned long long)(*w2);
  unsigned long long q2 = cur / 10ULL;
  remainder = cur % 10ULL;
  cur = (remainder << 32) | (unsigned long long)(*w1);
  unsigned long long q1 = cur / 10ULL;
  remainder = cur % 10ULL;
  cur = (remainder << 32) | (unsigned long long)(*w0);
  unsigned long long q0 = cur / 10ULL;
  remainder = cur % 10ULL;
  *w2 = (unsigned int)q2;
  *w1 = (unsigned int)q1;
  *w0 = (unsigned int)q0;
  return remainder;
}

static int add_one_u96(unsigned int *w2, unsigned int *w1, unsigned int *w0) {
  unsigned long long sum = (unsigned long long)(*w0) + 1ULL;
  *w0 = (unsigned int)sum;
  unsigned long long carry = sum >> 32;
  if (carry) {
    sum = (unsigned long long)(*w1) + carry;
    *w1 = (unsigned int)sum;
    carry = sum >> 32;
    if (carry) {
      sum = (unsigned long long)(*w2) + carry;
      *w2 = (unsigned int)sum;
      carry = sum >> 32;
    }
  }
  return carry;
}

static void add_one(decimal *result) {
  unsigned int w0 = (unsigned int)result->bits[0];
  unsigned int w1 = (unsigned int)result->bits[1];
  unsigned int w2 = (unsigned int)result->bits[2];
  (void)add_one_u96(&w2, &w1, &w0);
  result->bits[0] = (int)w0;
  result->bits[1] = (int)w1;
  result->bits[2] = (int)w2;
}

static int drop_last_digits(decimal *value, int digits_to_remove) {
  int status = 1;
  if (value != NULL && digits_to_remove >= 0) {
    int scale = get_scale(value);
    if (digits_to_remove <= scale) {
      unsigned int w0 = value->bits[0];
      unsigned int w1 = value->bits[1];
      unsigned int w2 = value->bits[2];
      for (int i = 0; i < digits_to_remove; i++) {
        (void)divide_by_10_u96(&w2, &w1, &w0);
      }
      value->bits[0] = (int)w0;
      value->bits[1] = (int)w1;
      value->bits[2] = (int)w2;
      set_scale(value, scale - digits_to_remove);
      status = 0;
    }
  }
  return status;
}

static void floor_truncate(const decimal *value, decimal *result,
                           int scale) {
  unsigned int w0 = (unsigned int)value->bits[0];
  unsigned int w1 = (unsigned int)value->bits[1];
  unsigned int w2 = (unsigned int)value->bits[2];
  int had_fraction = 0;
  for (int i = 0; i < scale; i++) {
    unsigned int rem = divide_by_10_u96(&w2, &w1, &w0);
    if (rem != 0) had_fraction = 1;
  }
  int sign = get_sign(value);
  if (sign && had_fraction) {
    // floor to -int
    (void)add_one_u96(&w2, &w1, &w0);
  }
  result->bits[0] = (int)w0;
  result->bits[1] = (int)w1;
  result->bits[2] = (int)w2;
  set_scale(result, 0);
  set_sign(result, sign);
}

static unsigned int get_first_fraction_digit(const decimal *value) {
  unsigned int digit = 0u;
  int scale = get_scale(value);
  if (scale > 0) {
    unsigned int w0 = (unsigned int)value->bits[0];
    unsigned int w1 = (unsigned int)value->bits[1];
    unsigned int w2 = (unsigned int)value->bits[2];
    for (int i = 0; i < scale - 1; i++) {
      (void)divide_by_10_u96(&w2, &w1, &w0);
    }
    digit = divide_by_10_u96(&w2, &w1, &w0);
  }
  return digit;
}

int floor_decimal(decimal value, decimal *result) {
  int status = 1;
  if (result) {
    decimal_zero(result);
    int scale = get_scale(&value);
    if (scale > 0) {
      floor_truncate(&value, result, scale);
    } else {
      *result = value;
      set_scale(result, 0);
      set_sign(result, get_sign(&value));
    }
    status = 0;
  }
  return status;
}

int round_decimal(decimal value, decimal *result) {
  int status = 1;
  if (result) {
    decimal_zero(result);
    int scale = get_scale(&value);
    int sign = get_sign(&value);
    if (scale > 0) {
      unsigned int first_frac = get_first_fraction_digit(&value);
      decimal tmp = value;
      drop_last_digits(&tmp, scale);
      *result = tmp;
      if (first_frac >= 5) {
        add_one(result);
      }
    } else {
      *result = value;
    }
    set_scale(result, 0);
    set_sign(result, sign);
    status = 0;
  }
  return status;
}

int truncate_decimal(decimal value, decimal *result) {
  int status = 1;
  if (result) {
    decimal_zero(result);
    int scale = get_scale(&value);
    int sign = get_sign(&value);
    decimal tmp = value;
    (void)drop_last_digits(&tmp, scale);
    *result = tmp;
    set_scale(result, 0);
    set_sign(result, sign);
    status = 0;
  }
  return status;
}

int negate_decimal(decimal value, decimal *result) {
  int status = 1;
  if (result) {
    decimal_zero(result);
    *result = value;
    int sign = get_sign(&value);
    set_sign(result, !sign);
    status = 0;
  }
  return status;
}

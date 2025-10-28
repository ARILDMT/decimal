#ifndef DECIMAL_H_
#define DECIMAL_H_

#define ARITHMETIC_OK 0
#define ARITHMETIC_BIG 1
#define ARITHMETIC_SMALL 2
#define ARITHMETIC_DIV_BY_ZERO 3
#define ARITHMETIC_BAD_INPUT 4

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  int bits[4];
} decimal;

int get_sign(const decimal *value);
void set_sign(decimal *value, int sign);
int get_scale(const decimal *value);
void set_scale(decimal *value, int scale);
void clear_service_bits(decimal *value);
int is_zero(decimal value);
void decimal_zero(decimal *value);
int get_bit(const decimal *value, int bit_position);
void set_bit(decimal *value, int bit_position, int bit_value);
int shift_left(decimal *value);
int shift_right(decimal *value);
int compare_abs(decimal value_1, decimal value_2);
int mul_by_ten(decimal *value);
void normalize(decimal *value_1, decimal *value_2);
int bank_round(decimal value, decimal *result);

int add(decimal value_1, decimal value_2, decimal *result);
int add_abs(decimal value_1, decimal value_2, decimal *result);
int sub(decimal value_1, decimal value_2, decimal *result);
int sub_abs(decimal value_1, decimal value_2, decimal *result);
int mul(decimal value_1, decimal value_2, decimal *result);
int mul_abs(decimal value_1, decimal value_2, decimal *result);
int div(decimal value_1, decimal value_2, decimal *result);
int div_abs(decimal dividend, decimal divisor, decimal *result);
int is_divisor_zero(decimal value);
int check_small_result(decimal value);
void process_multiplication(unsigned long long *temp, int i, int j,
                                unsigned int val1, unsigned int val2);
int process_division_bit(decimal *quotient, decimal remainder,
                             decimal divisor, decimal *result, int bit);
int handle_mul_scale(decimal *result, int scale1, int scale2);
int perform_multiplication(decimal value_1, decimal value_2,
                               decimal *result);
int finalize_division(decimal *result, int result_sign,
                          int result_scale);
int perform_division(decimal value_1, decimal value_2,
                         decimal *result);

int is_less(decimal, decimal);
int is_less_or_equal(decimal, decimal);
int is_greater(decimal, decimal);
int is_greater_or_equal(decimal, decimal);
int is_equal(decimal, decimal);
int is_not_equal(decimal, decimal);

int from_int_to_decimal(int src, decimal *dst);
int from_float_to_decimal(float src, decimal *dst);
int from_decimal_to_int(decimal src, int *dst);
int from_decimal_to_float(decimal src, float *dst);

int floor_decimal(decimal value, decimal *result);
int round_decimal(decimal value, decimal *result);
int truncate_decimal(decimal value, decimal *result);
int negate_decimal(decimal value, decimal *result);
#endif

#include <check.h>
#include <limits.h>

#include "decimal.h"

static decimal make_dec_int(int val, int scale) {
  decimal d = {{0, 0, 0, 0}};
  decimal_zero(&d);
  unsigned int mag =
      (val < 0) ? (unsigned int)(-1LL * (long long)val) : (unsigned int)val;
  d.bits[0] = (int)mag;
  set_scale(&d, scale);
  if (val < 0) set_sign(&d, 1);
  return d;
}

START_TEST(test_compare_equal_zeros) {
  decimal a = make_dec_int(0, 0);
  decimal b = make_dec_int(0, 5);
  set_sign(&b, 1);
  ck_assert_int_eq(is_equal(a, b), 1);
  ck_assert_int_eq(is_not_equal(a, b), 0);
  ck_assert_int_eq(is_greater(a, b), 0);
  ck_assert_int_eq(is_less(a, b), 0);
  ck_assert_int_eq(is_less_or_equal(a, b), 1);
  ck_assert_int_eq(is_greater_or_equal(a, b), 1);
}
END_TEST

START_TEST(test_compare_simple_signs) {
  decimal a = make_dec_int(1, 0);
  decimal b = make_dec_int(-1, 0);
  ck_assert_int_eq(is_greater(a, b), 1);
  ck_assert_int_eq(is_less(b, a), 1);
  ck_assert_int_eq(is_not_equal(a, b), 1);
}
END_TEST

START_TEST(test_compare_scale_normalization_equal) {
  decimal a = make_dec_int(123, 2);
  decimal b = make_dec_int(1230, 3);
  ck_assert_int_eq(is_equal(a, b), 1);
  ck_assert_int_eq(is_less(a, b), 0);
  ck_assert_int_eq(is_greater(a, b), 0);
}
END_TEST

START_TEST(test_compare_scale_normalization_order) {
  decimal a = make_dec_int(456, 2);
  decimal b = make_dec_int(4559, 3);
  ck_assert_int_eq(is_greater(a, b), 1);
  ck_assert_int_eq(is_less(b, a), 1);
  ck_assert_int_eq(is_not_equal(a, b), 1);
}
END_TEST

START_TEST(test_compare_negatives_order) {
  decimal a = make_dec_int(-3, 0);
  decimal b = make_dec_int(-2, 0);
  ck_assert_int_eq(is_less(a, b), 1);
  ck_assert_int_eq(is_greater(b, a), 1);
}
END_TEST

START_TEST(test_compare_mixed_scale_sign) {
  decimal a = make_dec_int(-150, 2);
  decimal b = make_dec_int(-1499, 3);
  ck_assert_int_eq(is_less(a, b), 1);
}
END_TEST

START_TEST(test_get_set_sign) {
  decimal v = {{0, 0, 0, 0}};
  ck_assert_int_eq(get_sign(&v), 0);
  set_sign(&v, 1);
  ck_assert_int_eq(get_sign(&v), 1);
  set_sign(&v, 0);
  ck_assert_int_eq(get_sign(&v), 0);
}
END_TEST

START_TEST(test_get_set_scale) {
  decimal v = {{0, 0, 0, 0}};
  ck_assert_int_eq(get_scale(&v), 0);
  set_scale(&v, 10);
  ck_assert_int_eq(get_scale(&v), 10);
  set_scale(&v, -5);
  ck_assert_int_eq(get_scale(&v), 0);
  set_scale(&v, 100);
  ck_assert_int_eq(get_scale(&v), 28);
}
END_TEST

START_TEST(test_clear_service_bits) {
  decimal v = {{0, 0, 0, 0}};
  v.bits[3] = 0xFFFFFFFF;
  set_scale(&v, 5);
  set_sign(&v, 1);
  clear_service_bits(&v);
  ck_assert_int_eq((v.bits[3] & ~0x80FF0000), 0);
  ck_assert_int_eq(get_scale(&v), 5);
  ck_assert_int_eq(get_sign(&v), 1);
}
END_TEST

START_TEST(test_is_zero_and_decimal_zero) {
  decimal v = {{1, 0, 0, 0}};
  ck_assert_int_eq(is_zero(v), 0);
  decimal_zero(&v);
  ck_assert_int_eq(is_zero(v), 1);
  ck_assert_int_eq(get_scale(&v), 0);
  ck_assert_int_eq(get_sign(&v), 0);
}
END_TEST

START_TEST(test_get_set_bit) {
  decimal v = {{0, 0, 0, 0}};
  set_bit(&v, 0, 1);
  ck_assert_int_eq(get_bit(&v, 0), 1);
  set_bit(&v, 0, 0);
  ck_assert_int_eq(get_bit(&v, 0), 0);
  set_bit(&v, 33, 1);
  ck_assert_int_eq(get_bit(&v, 33), 1);
}
END_TEST

START_TEST(test_shifts) {
  decimal v = {{1, 0, 0, 0}};
  int overflow = shift_left(&v);
  ck_assert_int_eq(overflow, 0);
  ck_assert_int_eq(v.bits[0], 2);
  int lost = shift_right(&v);
  ck_assert_int_eq(lost, 0);
  ck_assert_int_eq(v.bits[0], 1);
  v.bits[2] = 0x80000000;
  overflow = shift_left(&v);
  ck_assert_int_eq(overflow, 1);
}
END_TEST

START_TEST(test_compare_abs) {
  decimal a = {{10, 0, 0, 0}};
  decimal b = {{5, 0, 0, 0}};
  ck_assert_int_eq(compare_abs(a, b), 1);
  ck_assert_int_eq(compare_abs(b, a), -1);
  decimal c = {{10, 0, 0, 0}};
  ck_assert_int_eq(compare_abs(a, c), 0);
  set_sign(&c, 1);
  ck_assert_int_eq(compare_abs(a, c), 0);
}
END_TEST

START_TEST(test_mul_by_ten) {
  decimal v = {{1, 0, 0, 0}};
  int err = mul_by_ten(&v);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(v.bits[0], 10);
  v.bits[0] = 0xFFFFFFFF;
  v.bits[1] = 0xFFFFFFFF;
  v.bits[2] = 0xFFFFFFFF;
  err = mul_by_ten(&v);
  ck_assert_int_eq(err, 1);
}
END_TEST

START_TEST(test_normalize) {
  decimal a = {{1, 0, 0, 0}};
  set_scale(&a, 0);
  decimal b = {{2, 0, 0, 0}};
  set_scale(&b, 2);
  normalize(&a, &b);
  ck_assert_int_eq(get_scale(&a), get_scale(&b));
}
END_TEST

START_TEST(test_bank_round) {
  decimal v;
  decimal_zero(&v);
  v.bits[0] = 25;
  set_scale(&v, 1);
  decimal out;
  int err = bank_round(v, &out);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(out.bits[0], 2);
  ck_assert_int_eq(get_scale(&out), 0);
  v.bits[0] = 35;
  set_scale(&v, 1);
  err = bank_round(v, &out);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(out.bits[0], 4);
  v.bits[0] = 25;
  set_scale(&v, 1);
  set_sign(&v, 1);
  err = bank_round(v, &out);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(out.bits[0], 2);
  ck_assert_int_eq(get_sign(&out), 1);
}
END_TEST

START_TEST(test_conversions) {
  decimal v;
  int err = from_int_to_decimal(-123, &v);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(v.bits[0], 123);
  ck_assert_int_eq(get_sign(&v), 1);
  int out_int;
  err = from_decimal_to_int(v, &out_int);
  ck_assert_int_eq(err, 0);
  ck_assert_int_eq(out_int, -123);
  float out_float;
  err = from_decimal_to_float(v, &out_float);
  ck_assert_int_eq(err, 0);
  ck_assert_float_eq_tol(out_float, -123.0f, 1e-6f);
}
END_TEST

START_TEST(test_add_simple) {
  decimal a, b, result;

  from_int_to_decimal(123, &a);
  from_int_to_decimal(456, &b);
  int status = add(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 579);
}
END_TEST

START_TEST(test_add_negative) {
  decimal a, b, result;

  from_int_to_decimal(-123, &a);
  from_int_to_decimal(456, &b);
  int status = add(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 333);
}
END_TEST

START_TEST(test_add_zero) {
  decimal a, b, result;

  from_int_to_decimal(123, &a);
  decimal_zero(&b);
  int status = add(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 123);
}
END_TEST

START_TEST(test_add_overflow) {
  decimal a, b, result;

  a.bits[0] = 0xFFFFFFFF;
  a.bits[1] = 0xFFFFFFFF;
  a.bits[2] = 0xFFFFFFFF;
  a.bits[3] = 0;

  b.bits[0] = 1;
  b.bits[1] = 0;
  b.bits[2] = 0;
  b.bits[3] = 0;

  int status = add(a, b, &result);
  ck_assert_int_eq(status, ARITHMETIC_BIG);
}
END_TEST

START_TEST(test_add_null_result) {
  decimal a, b;
  from_int_to_decimal(123, &a);
  from_int_to_decimal(456, &b);

  int status = add(a, b, NULL);
  ck_assert_int_eq(status, ARITHMETIC_BAD_INPUT);
}
END_TEST

START_TEST(test_sub_simple) {
  decimal a, b, result;

  from_int_to_decimal(456, &a);
  from_int_to_decimal(123, &b);
  int status = sub(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 333);
}
END_TEST

START_TEST(test_sub_negative_result) {
  decimal a, b, result;

  from_int_to_decimal(123, &a);
  from_int_to_decimal(456, &b);
  int status = sub(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, -333);
}
END_TEST

START_TEST(test_sub_zero) {
  decimal a, b, result;

  decimal_zero(&a);
  from_int_to_decimal(123, &b);
  int status = sub(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, -123);
}
END_TEST

START_TEST(test_sub_equal_numbers) {
  decimal a, b, result;

  from_int_to_decimal(123, &a);
  from_int_to_decimal(123, &b);
  int status = sub(a, b, &result);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(is_zero(result), 1);
}
END_TEST

START_TEST(test_mul_simple) {
  decimal a, b, result;

  from_int_to_decimal(12, &a);
  from_int_to_decimal(13, &b);
  int status = mul(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 156);
}
END_TEST

START_TEST(test_mul_negative) {
  decimal a, b, result;

  from_int_to_decimal(-12, &a);
  from_int_to_decimal(13, &b);
  int status = mul(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, -156);
}
END_TEST

START_TEST(test_mul_zero) {
  decimal a, b, result;

  from_int_to_decimal(123, &a);
  decimal_zero(&b);
  int status = mul(a, b, &result);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(is_zero(result), 1);
}
END_TEST

START_TEST(test_mul_overflow) {
  decimal a, b, result;

  a.bits[0] = 0xFFFFFFFF;
  a.bits[1] = 0xFFFFFFFF;
  a.bits[2] = 0xFFFFFFFF;
  a.bits[3] = 0;

  b.bits[0] = 2;
  b.bits[1] = 0;
  b.bits[2] = 0;
  b.bits[3] = 0;

  int status = mul(a, b, &result);
  ck_assert_int_eq(status, ARITHMETIC_BIG);
}
END_TEST

START_TEST(test_mul_null_result) {
  decimal a, b;
  from_int_to_decimal(12, &a);
  from_int_to_decimal(13, &b);

  int status = mul(a, b, NULL);
  ck_assert_int_eq(status, ARITHMETIC_BAD_INPUT);
}
END_TEST

START_TEST(test_div_simple) {
  decimal a, b, result;

  from_int_to_decimal(156, &a);
  from_int_to_decimal(12, &b);
  int status = div(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 13);
}
END_TEST

START_TEST(test_div_negative) {
  decimal a, b, result;

  from_int_to_decimal(-156, &a);
  from_int_to_decimal(12, &b);
  int status = div(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, -13);
}
END_TEST

START_TEST(test_div_zero_dividend) {
  decimal a, b, result;

  decimal_zero(&a);
  from_int_to_decimal(123, &b);
  int status = div(a, b, &result);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(is_zero(result), 1);
}
END_TEST

START_TEST(test_div_by_zero) {
  decimal a, b, result;

  from_int_to_decimal(123, &a);
  decimal_zero(&b);
  int status = div(a, b, &result);

  ck_assert_int_eq(status, ARITHMETIC_DIV_BY_ZERO);
}
END_TEST

START_TEST(test_div_null_result) {
  decimal a, b;
  from_int_to_decimal(156, &a);
  from_int_to_decimal(12, &b);

  int status = div(a, b, NULL);
  ck_assert_int_eq(status, ARITHMETIC_BAD_INPUT);
}
END_TEST

START_TEST(test_arithmetic_with_scale) {
  decimal a, b, result;

  from_int_to_decimal(15, &a);
  set_scale(&a, 1);  // 1.5

  from_int_to_decimal(23, &b);
  set_scale(&b, 1);  // 2.3

  int status = add(a, b, &result);
  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result.bits[0], 38);
  ck_assert_int_eq(get_scale(&result), 1);
}
END_TEST

START_TEST(test_add_abs) {
  decimal a, b, result;

  from_int_to_decimal(123, &a);
  from_int_to_decimal(456, &b);
  int status = add_abs(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 579);
}
END_TEST

START_TEST(test_sub_abs) {
  decimal a, b, result;

  from_int_to_decimal(456, &a);
  from_int_to_decimal(123, &b);
  int status = sub_abs(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 333);
}
END_TEST

START_TEST(test_mul_abs) {
  decimal a, b, result;

  from_int_to_decimal(12, &a);
  from_int_to_decimal(13, &b);
  int status = mul_abs(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 156);
}
END_TEST

START_TEST(test_div_abs) {
  decimal a, b, result;

  from_int_to_decimal(156, &a);
  from_int_to_decimal(12, &b);
  int status = div_abs(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);

  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 13);
}
END_TEST

START_TEST(test_is_divisor_zero) {
  decimal zero, non_zero;

  decimal_zero(&zero);
  from_int_to_decimal(123, &non_zero);

  ck_assert_int_eq(is_divisor_zero(zero), 1);
  ck_assert_int_eq(is_divisor_zero(non_zero), 0);
}
END_TEST

START_TEST(test_check_small_result) {
  decimal small, normal;

  small.bits[0] = 5;
  small.bits[1] = 0;
  small.bits[2] = 0;
  small.bits[3] = 0;
  set_scale(&small, 28);

  from_int_to_decimal(123, &normal);

  ck_assert_int_eq(check_small_result(small), 1);
  ck_assert_int_eq(check_small_result(normal), 0);
}
END_TEST

START_TEST(test_arithmetic_edge_cases) {
  decimal a, b, result;

  a.bits[0] = 0xFFFFFFFF;
  a.bits[1] = 0xFFFFFFFF;
  a.bits[2] = 0xFFFFFFFF;
  a.bits[3] = 0;

  decimal_zero(&b);

  int status = add(a, b, &result);
  ck_assert_int_eq(status, ARITHMETIC_OK);

  status = mul(a, b, &result);
  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(is_zero(result), 1);
}
END_TEST

START_TEST(test_floor_positive_without_decimal) {
  decimal value = {.bits = {500, 0, 0, 0}};
  set_scale(&value, 2);
  decimal result;

  ck_assert_int_eq(floor_decimal(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 5);
}
END_TEST

START_TEST(test_floor_positive_with_decimal) {
  decimal value = {.bits = {575, 0, 0, 0}};
  set_scale(&value, 2);
  decimal result;

  ck_assert_int_eq(floor_decimal(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 5);
}
END_TEST

START_TEST(test_floor_negative_with_decimal) {
  decimal value = {.bits = {575, 0, 0, 0}};
  set_scale(&value, 2);
  set_sign(&value, 1);
  decimal result;

  ck_assert_int_eq(floor_decimal(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 6);
  ck_assert_int_eq(get_sign(&result), 1);
}
END_TEST

START_TEST(test_floor_zero) {
  decimal value = {.bits = {0, 0, 0, 0}};
  decimal result;

  ck_assert_int_eq(floor_decimal(value, &result), 0);
  ck_assert_int_eq(result.bits[0], 0);
}
END_TEST

START_TEST(test_round_positive) {
  decimal a, res;
  from_float_to_decimal(2.3f, &a);
  ck_assert_int_eq(round_decimal(a, &res), 0);
  float out = 0.0f;
  from_decimal_to_float(res, &out);
  ck_assert_float_eq(out, 2.0f);
}
END_TEST

START_TEST(test_round_half_up) {
  decimal a, res;
  a.bits[0] = 35;
  a.bits[1] = 0;
  a.bits[2] = 0;
  a.bits[3] = 0;
  set_scale(&a, 1);
  set_sign(&a, 0);

  ck_assert_int_eq(round_decimal(a, &res), 0);

  ck_assert_int_eq(res.bits[0], 4);
  ck_assert_int_eq(get_scale(&res), 0);
}
END_TEST

START_TEST(test_round_carry_propagation) {
  decimal q;
  decimal_zero(&q);
  q.bits[0] = (int)0xFFFFFFFFu;
  q.bits[1] = (int)0xFFFFFFFFu;
  q.bits[2] = 1;

  decimal t = q;
  int err = mul_by_ten(&t);
  ck_assert_int_eq(err, 0);
  for (int i = 0; i < 5; i++) {
    unsigned int prev0 = (unsigned int)t.bits[0];
    t.bits[0] += 1;
    if ((unsigned int)t.bits[0] < prev0) {
      unsigned int prev1 = (unsigned int)t.bits[1];
      t.bits[1] += 1;
      if ((unsigned int)t.bits[1] < prev1) {
        t.bits[2] += 1;
      }
    }
  }

  decimal value = t;
  set_scale(&value, 1);

  decimal res;
  ck_assert_int_eq(round_decimal(value, &res), 0);
  ck_assert_int_eq((unsigned int)res.bits[0], 0u);
  ck_assert_int_eq((unsigned int)res.bits[1], 0u);
  ck_assert_int_eq((unsigned int)res.bits[2], 2u);
  ck_assert_int_eq(get_scale(&res), 0);
}
END_TEST

START_TEST(test_truncate_positive_simple) {
  decimal a, res;
  from_float_to_decimal(3.7f, &a);
  ck_assert_int_eq(truncate_decimal(a, &res), 0);
  float out = 0.0f;
  from_decimal_to_float(res, &out);
  ck_assert_float_eq(out, 3.0f);
}
END_TEST

START_TEST(test_truncate_negative_simple) {
  decimal a, res;
  from_float_to_decimal(-3.7f, &a);
  ck_assert_int_eq(truncate_decimal(a, &res), 0);
  float out = 0.0f;
  from_decimal_to_float(res, &out);
  ck_assert_float_eq(out, -3.0f);
}
END_TEST

START_TEST(test_negate_positive_to_negative) {
  decimal a, res;
  from_float_to_decimal(1.5f, &a);
  ck_assert_int_eq(negate_decimal(a, &res), 0);
  int sign = get_sign(&res);
  ck_assert_int_eq(sign, 1);
}
END_TEST

START_TEST(test_from_int_to_decimal_positive) {
  int src = 123;
  decimal dst;
  int result = from_int_to_decimal(src, &dst);
  ck_assert_int_eq(result, 0);
  ck_assert_int_eq(dst.bits[0], 123);
  ck_assert_int_eq(dst.bits[1], 0);
  ck_assert_int_eq(dst.bits[2], 0);
  ck_assert_int_eq(get_scale(&dst), 0);
  ck_assert_int_eq(get_sign(&dst), 0);
}
END_TEST

START_TEST(test_from_int_to_decimal_negative) {
  int src = -456;
  decimal dst;
  int result = from_int_to_decimal(src, &dst);
  ck_assert_int_eq(result, 0);
  ck_assert_int_eq(dst.bits[0], 456);
  ck_assert_int_eq(get_sign(&dst), 1);
}
END_TEST

START_TEST(test_from_float_to_decimal_fraction) {
  float src = 123.456f;
  decimal dst;
  int result = from_float_to_decimal(src, &dst);
  ck_assert_int_eq(result, 0);
  float back;
  from_decimal_to_float(dst, &back);
  ck_assert_float_eq_tol(back, src, 0.001f);
}
END_TEST

START_TEST(test_from_decimal_to_int_positive) {
  decimal src = {{123, 0, 0, 0}};
  int dst;
  int result = from_decimal_to_int(src, &dst);
  ck_assert_int_eq(result, 0);
  ck_assert_int_eq(dst, 123);
}
END_TEST

START_TEST(test_from_decimal_to_float_integer) {
  decimal src = {{123, 0, 0, 0}};
  float dst;
  int result = from_decimal_to_float(src, &dst);
  ck_assert_int_eq(result, 0);
  ck_assert_float_eq_tol(dst, 123.0f, 1e-6f);
}
END_TEST

START_TEST(test_add_equal_magnitudes_different_signs) {
  decimal a, b, result;
  from_int_to_decimal(5, &a);
  from_int_to_decimal(-5, &b);
  int status = add(a, b, &result);
  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(is_zero(result), 1);
}
END_TEST

START_TEST(test_add_larger_magnitude_first) {
  decimal a, b, result;
  from_int_to_decimal(10, &a);
  from_int_to_decimal(-3, &b);
  int status = add(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);
  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 7);
  ck_assert_int_eq(get_sign(&result), 0);
}
END_TEST

START_TEST(test_sub_number_minus_zero) {
  decimal a, b, result;
  from_int_to_decimal(100, &a);
  decimal_zero(&b);
  int status = sub(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);
  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 100);
}
END_TEST

START_TEST(test_sub_different_signs_as_addition) {
  decimal a, b, result;
  from_int_to_decimal(5, &a);
  from_int_to_decimal(-3, &b);
  int status = sub(a, b, &result);
  int result_int;
  from_decimal_to_int(result, &result_int);
  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq(result_int, 8);
}
END_TEST

START_TEST(test_sub_with_borrow) {
  decimal a, b, result;
  a.bits[0] = 0;
  a.bits[1] = 1;
  a.bits[2] = 0;
  a.bits[3] = 0;

  b.bits[0] = 1;
  b.bits[1] = 0;
  b.bits[2] = 0;
  b.bits[3] = 0;

  int status = sub_abs(a, b, &result);
  ck_assert_int_eq(status, ARITHMETIC_OK);
  ck_assert_int_eq((unsigned int)result.bits[0], 0xFFFFFFFF);
  ck_assert_int_eq(result.bits[1], 0);
}
END_TEST

START_TEST(test_mul_large_scale) {
  decimal a, b, result;
  a.bits[0] = 123;
  a.bits[1] = 0;
  a.bits[2] = 0;
  a.bits[3] = 0;
  set_scale(&a, 15);

  b.bits[0] = 456;
  b.bits[1] = 0;
  b.bits[2] = 0;
  b.bits[3] = 0;
  set_scale(&b, 15);

  int status = mul(a, b, &result);
  ck_assert_int_eq(status, ARITHMETIC_OK);
}
END_TEST

static Suite *decimal_suite(void) {
  Suite *s = suite_create("decimal");

  TCase *tc_core = tcase_create("core");
  tcase_add_test(tc_core, test_get_set_sign);
  tcase_add_test(tc_core, test_get_set_scale);
  tcase_add_test(tc_core, test_clear_service_bits);
  tcase_add_test(tc_core, test_is_zero_and_decimal_zero);
  tcase_add_test(tc_core, test_get_set_bit);
  tcase_add_test(tc_core, test_shifts);

  tcase_add_test(tc_core, test_compare_equal_zeros);
  tcase_add_test(tc_core, test_compare_simple_signs);
  tcase_add_test(tc_core, test_compare_scale_normalization_equal);
  tcase_add_test(tc_core, test_compare_scale_normalization_order);
  tcase_add_test(tc_core, test_compare_negatives_order);
  tcase_add_test(tc_core, test_compare_mixed_scale_sign);

  tcase_add_test(tc_core, test_compare_abs);
  tcase_add_test(tc_core, test_mul_by_ten);
  tcase_add_test(tc_core, test_normalize);
  tcase_add_test(tc_core, test_bank_round);
  tcase_add_test(tc_core, test_conversions);
  suite_add_tcase(s, tc_core);

  TCase *tc_arithmetic = tcase_create("arithmetic");
  tcase_add_test(tc_arithmetic, test_add_simple);
  tcase_add_test(tc_arithmetic, test_add_negative);
  tcase_add_test(tc_arithmetic, test_add_zero);
  tcase_add_test(tc_arithmetic, test_add_overflow);
  tcase_add_test(tc_arithmetic, test_add_null_result);

  tcase_add_test(tc_arithmetic, test_sub_simple);
  tcase_add_test(tc_arithmetic, test_sub_negative_result);
  tcase_add_test(tc_arithmetic, test_sub_zero);
  tcase_add_test(tc_arithmetic, test_sub_equal_numbers);

  tcase_add_test(tc_arithmetic, test_mul_simple);
  tcase_add_test(tc_arithmetic, test_mul_negative);
  tcase_add_test(tc_arithmetic, test_mul_zero);
  tcase_add_test(tc_arithmetic, test_mul_overflow);
  tcase_add_test(tc_arithmetic, test_mul_null_result);

  tcase_add_test(tc_arithmetic, test_div_simple);
  tcase_add_test(tc_arithmetic, test_div_negative);
  tcase_add_test(tc_arithmetic, test_div_zero_dividend);
  tcase_add_test(tc_arithmetic, test_div_by_zero);
  tcase_add_test(tc_arithmetic, test_div_null_result);

  tcase_add_test(tc_arithmetic, test_arithmetic_with_scale);

  tcase_add_test(tc_arithmetic, test_add_abs);
  tcase_add_test(tc_arithmetic, test_sub_abs);
  tcase_add_test(tc_arithmetic, test_mul_abs);
  tcase_add_test(tc_arithmetic, test_div_abs);
  tcase_add_test(tc_arithmetic, test_is_divisor_zero);
  tcase_add_test(tc_arithmetic, test_check_small_result);

  tcase_add_test(tc_arithmetic, test_arithmetic_edge_cases);

  tcase_add_test(tc_arithmetic, test_add_equal_magnitudes_different_signs);
  tcase_add_test(tc_arithmetic, test_add_larger_magnitude_first);
  tcase_add_test(tc_arithmetic, test_sub_number_minus_zero);
  tcase_add_test(tc_arithmetic, test_sub_different_signs_as_addition);
  tcase_add_test(tc_arithmetic, test_sub_with_borrow);
  tcase_add_test(tc_arithmetic, test_mul_large_scale);

  suite_add_tcase(s, tc_arithmetic);

  TCase *tc_extra = tcase_create("extra");

  tcase_add_test(tc_extra, test_floor_positive_without_decimal);
  tcase_add_test(tc_extra, test_floor_positive_with_decimal);
  tcase_add_test(tc_extra, test_floor_negative_with_decimal);
  tcase_add_test(tc_extra, test_floor_zero);

  tcase_add_test(tc_extra, test_round_positive);
  tcase_add_test(tc_extra, test_round_half_up);
  tcase_add_test(tc_extra, test_round_carry_propagation);
  tcase_add_test(tc_extra, test_truncate_positive_simple);
  tcase_add_test(tc_extra, test_truncate_negative_simple);

  tcase_add_test(tc_extra, test_negate_positive_to_negative);

  suite_add_tcase(s, tc_extra);
  TCase *tc_conversion = tcase_create("conversion");

  tcase_add_test(tc_conversion, test_from_int_to_decimal_positive);
  tcase_add_test(tc_conversion, test_from_int_to_decimal_negative);
  tcase_add_test(tc_conversion, test_from_float_to_decimal_fraction);
  tcase_add_test(tc_conversion, test_from_decimal_to_int_positive);
  tcase_add_test(tc_conversion, test_from_decimal_to_float_integer);

  suite_add_tcase(s, tc_conversion);

  return s;
}

int main(void) {
  Suite *s = decimal_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_VERBOSE);
  int number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (number_failed == 0) ? 0 : 1;
}

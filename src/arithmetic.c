#include "decimal.h"

int add(decimal value_1, decimal value_2, decimal *result) {
  int flag = ARITHMETIC_OK;
  int sub = 0;
  if (result == NULL) return ARITHMETIC_BAD_INPUT;

  if (is_zero(value_1))
    *result = value_2;
  else if (is_zero(value_2))
    *result = value_1;
  else {
    normalize(&value_1, &value_2);
    decimal_zero(result);

    int sign1 = get_sign(&value_1);
    int sign2 = get_sign(&value_2);

    if (sign1 == sign2) {
      flag = add_abs(value_1, value_2, result);
      if (flag == ARITHMETIC_OK) {
        set_sign(result, sign1);
      }
    } else {
      int cmp = compare_abs(value_1, value_2);
      if (cmp == 0) {
        decimal_zero(result);
      } else if (cmp > 0) {
        sub = 1;
        flag = sub_abs(value_1, value_2, result);
      } else {
        sub = 2;
        flag = sub_abs(value_2, value_1, result);
      }
    }
    if (flag == ARITHMETIC_OK && sub == 1) {
      set_sign(result, sign1);
    } else if (flag == ARITHMETIC_OK && sub == 2) {
      set_sign(result, sign2);
    }
  }
  if (flag == ARITHMETIC_OK) {
    set_scale(result, get_scale(&value_1));
  }

  return flag;
}

int add_abs(decimal value_1, decimal value_2, decimal *result) {
  int flag = ARITHMETIC_OK;
  unsigned long long carry = 0;

  for (int i = 0; i < 3; i++) {
    unsigned long long sum = (unsigned long long)(unsigned int)value_1.bits[i] +
                             (unsigned long long)(unsigned int)value_2.bits[i] +
                             carry;
    result->bits[i] = (int)(sum & 0xFFFFFFFF);
    carry = sum >> 32;
  }

  if (carry) {
    flag = ARITHMETIC_BIG;
  }

  return flag;
}

int sub(decimal value_1, decimal value_2, decimal *result) {
  int flag = ARITHMETIC_OK;
  int sub = 0;
  if (result == NULL) return ARITHMETIC_BAD_INPUT;

  if (is_zero(value_1)) {
    *result = value_2;
    set_sign(result, !get_sign(&value_2));
  } else if (is_zero(value_2)) {
    *result = value_1;
  } else {
    normalize(&value_1, &value_2);
    decimal_zero(result);

    int sign1 = get_sign(&value_1);
    int sign2 = get_sign(&value_2);

    if (sign1 != sign2) {
      flag = add_abs(value_1, value_2, result);
      if (flag == ARITHMETIC_OK) {
        set_sign(result, sign1);
      }
    } else {
      int cmp = compare_abs(value_1, value_2);
      if (cmp == 0) {
        decimal_zero(result);
      } else if (cmp > 0) {
        sub = 1;
        flag = sub_abs(value_1, value_2, result);
      } else {
        sub = 2;
        flag = sub_abs(value_2, value_1, result);
      }
    }
    if (flag == ARITHMETIC_OK && sub == 1) {
      set_sign(result, sign1);
    } else if (flag == ARITHMETIC_OK && sub == 2) {
      set_sign(result, !sign1);
    }
  }

  if (flag == ARITHMETIC_OK) {
    set_scale(result, get_scale(&value_1));
  }

  return flag;
}

int sub_abs(decimal value_1, decimal value_2, decimal *result) {
  int flag = ARITHMETIC_OK;
  long long borrow = 0;

  for (int i = 0; i < 3; i++) {
    long long diff = (long long)(unsigned int)value_1.bits[i] -
                     (long long)(unsigned int)value_2.bits[i] - borrow;
    if (diff < 0) {
      diff += 0x100000000LL;
      borrow = 1;
    } else {
      borrow = 0;
    }
    result->bits[i] = (int)(diff & 0xFFFFFFFF);
  }

  return flag;
}

void process_multiplication(unsigned long long *temp, int i, int j,
                                unsigned int val1, unsigned int val2) {
  unsigned long long product =
      (unsigned long long)val1 * (unsigned long long)val2;
  temp[i + j] += product;
  if (temp[i + j] > 0xFFFFFFFFULL) {
    temp[i + j + 1] += temp[i + j] >> 32;
    temp[i + j] &= 0xFFFFFFFFULL;
  }
}

int mul_abs(decimal value_1, decimal value_2, decimal *result) {
  int flag = ARITHMETIC_OK;
  unsigned long long temp[6] = {0};

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      process_multiplication(temp, i, j, (unsigned int)value_1.bits[i],
                                 (unsigned int)value_2.bits[j]);
    }
  }

  if (temp[3] != 0 || temp[4] != 0 || temp[5] != 0) {
    flag = ARITHMETIC_BIG;
  } else {
    result->bits[0] = (int)temp[0];
    result->bits[1] = (int)temp[1];
    result->bits[2] = (int)temp[2];
  }

  return flag;
}

int handle_mul_scale(decimal *result, int scale1, int scale2) {
  int flag = ARITHMETIC_OK;
  int result_scale = scale1 + scale2;

  if (result_scale > 28) {
    decimal temp = *result;
    set_scale(&temp, result_scale);
    flag = bank_round(temp, result);
    result_scale = 0;
  }

  if (flag == ARITHMETIC_OK) {
    set_scale(result, result_scale);
  }

  return flag;
}

int perform_multiplication(decimal value_1, decimal value_2,
                               decimal *result) {
  int sign1 = get_sign(&value_1);
  int sign2 = get_sign(&value_2);
  int result_sign = sign1 ^ sign2;

  int flag = mul_abs(value_1, value_2, result);

  if (flag == ARITHMETIC_OK) {
    int scale1 = get_scale(&value_1);
    int scale2 = get_scale(&value_2);
    flag = handle_mul_scale(result, scale1, scale2);

    if (flag == ARITHMETIC_OK) {
      set_sign(result, result_sign);
    }
  }

  return flag;
}

int mul(decimal value_1, decimal value_2, decimal *result) {
  int flag = ARITHMETIC_OK;

  if (result == NULL) {
    flag = ARITHMETIC_BAD_INPUT;
  } else {
    decimal_zero(result);

    if (is_zero(value_1) || is_zero(value_2)) {
      decimal_zero(result);
    } else {
      flag = perform_multiplication(value_1, value_2, result);
    }
  }

  return flag;
}

int is_divisor_zero(decimal value) { return is_zero(value); }

int process_division_bit(decimal *quotient, decimal remainder,
                             decimal divisor, decimal *result,
                             int bit) {
  int continue_loop = 1;
  if (shift_left(quotient)) {
    continue_loop = 0;
  } else if (get_bit(&remainder, bit)) {
    set_bit(quotient, 0, 1);
    if (compare_abs(*quotient, divisor) >= 0) {
      sub_abs(*quotient, divisor, quotient);
      set_bit(result, bit, 1);
    }
  }
  return continue_loop;
}

int div_abs(decimal dividend, decimal divisor,
                decimal *result) {
  int flag = ARITHMETIC_OK;
  decimal_zero(result);

  if (compare_abs(dividend, divisor) < 0) {
    return flag;
  }

  decimal temp = divisor;
  decimal counter;
  decimal_zero(&counter);
  counter.bits[0] = 1;

  while (compare_abs(temp, dividend) <= 0) {
    decimal prev_temp = temp;
    decimal prev_counter = counter;

    if (shift_left(&temp) || shift_left(&counter)) {
      temp = prev_temp;
      counter = prev_counter;
      break;
    }
  }

  decimal remainder = dividend;
  while (counter.bits[0] || counter.bits[1] || counter.bits[2]) {
    if (compare_abs(remainder, temp) >= 0) {
      sub_abs(remainder, temp, &remainder);
      add_abs(*result, counter, result);
    }
    shift_right(&temp);
    shift_right(&counter);
  }

  return flag;
}

int check_small_result(decimal value) {
  int scale = get_scale(&value);
  return (scale == 28 && !is_zero(value) && value.bits[2] == 0 &&
          value.bits[1] == 0 && value.bits[0] < 10);
}

int finalize_division(decimal *result, int result_sign,
                          int result_scale) {
  int flag = ARITHMETIC_OK;

  if (result_scale < 0) result_scale = 0;
  if (result_scale > 28) result_scale = 28;

  set_sign(result, result_sign);
  set_scale(result, result_scale);

  if (check_small_result(*result)) {
    flag = ARITHMETIC_SMALL;
    decimal_zero(result);
  }

  return flag;
}

int perform_division(decimal value_1, decimal value_2,
                         decimal *result) {
  int sign1 = get_sign(&value_1);
  int sign2 = get_sign(&value_2);
  int result_sign = sign1 ^ sign2;

  normalize(&value_1, &value_2);

  int flag = div_abs(value_1, value_2, result);

  if (flag == ARITHMETIC_OK) {
    set_sign(result, result_sign);
    set_scale(result, 0);
  }

  return flag;
}

int div(decimal value_1, decimal value_2, decimal *result) {
  int flag = ARITHMETIC_OK;

  if (result == NULL) {
    flag = ARITHMETIC_BAD_INPUT;
  } else if (is_divisor_zero(value_2)) {
    flag = ARITHMETIC_DIV_BY_ZERO;
  } else {
    decimal_zero(result);

    if (is_zero(value_1)) {
      decimal_zero(result);
    } else {
      flag = perform_division(value_1, value_2, result);
    }
  }

  return flag;
}

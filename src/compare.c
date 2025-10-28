#include "decimal.h"

static int cmp_core(decimal a, decimal b) {
  if (is_zero(a) && is_zero(b)) return 0;

  int sa = get_sign(&a);
  int sb = get_sign(&b);

  if (sa != sb) {
    if (is_zero(a)) return sb ? 1 : -1;
    if (is_zero(b)) return sa ? -1 : 1;
    return sa ? -1 : 1;
  }

  normalize(&a, &b);

  int mag = compare_abs(a, b);
  return sa ? -mag : mag;
}

int is_less(decimal a, decimal b) { return cmp_core(a, b) < 0; }
int is_less_or_equal(decimal a, decimal b) {
  return cmp_core(a, b) <= 0;
}
int is_greater(decimal a, decimal b) { return cmp_core(a, b) > 0; }
int is_greater_or_equal(decimal a, decimal b) {
  return cmp_core(a, b) >= 0;
}
int is_equal(decimal a, decimal b) { return cmp_core(a, b) == 0; }
int is_not_equal(decimal a, decimal b) {
  return cmp_core(a, b) != 0;
}

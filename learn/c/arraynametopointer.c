/*
 * 6.3.2.1    Lvalues, arrays, and function designators
 *
 * 3  Except when it is the operand of the sizeof operator, or the unary &
 * operator, or is a string literal used to initialize an array, an expression
 * that has type "array of type" is converted to an expression with type
 * "pointer to type" that points to the initial element of the array object
 * and is not an lvalue. If the array object has register storage class, the
 * behavior is undefined.
 *
 * Note: The important part above is the: "array of type" is converted to an
 * expression with type "pointer to type" that points to the initial element of
 * the array object.
 *
 * In simple terms, it is saying that an array name is being implicitly converted
 * to a pointer.
 *
 * Refs:
 * https://www.open-std.org/jtc1/sc22/wg14/www/docs/n2310.pdf
 */

#include <stdio.h>

#define ARRAY_SIZE 5

int
main(void)
{
  int numbers[ARRAY_SIZE] = { 1, 2, 3, 4, 5 };
  int *int_ptr;

  int_ptr = numbers;
  printf("%d\n", int_ptr[0]);
  /*
   * Subscript expressions are often used to refer to array elements, but you
   * can apply a subscript operator [] to any pointer. As we did in int_ptr[0]
   * above.
   *
   * It works, because the subscript expression is evaluated by adding the
   * integral value to the pointer value, then applying the indirection
   * operator (*) to the result.
   *
   * As in:
   * numbers[0] is equivalent to *(numbers + 0)
   * int_ptr[0] is equivalent to *(int_ptr + 0)
   *
   * For what is worth, for a one-dimensional array, the following four
   * expressions are equivalent, assuming that a is a pointer and b is an
   * integer:
   *
   * a[b]
   * *(a + b)
   * *(b + a)
   * b[a]
   *
   * The conversion rules for the addition operator are given in
   * Additive Operators.
   *
   * Refs:
   * https://learn.microsoft.com/en-us/cpp/c-language/one-dimensional-arrays?view=msvc-170
   */
  return 0;
}

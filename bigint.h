/**
 * @file bigint.h
 * @brief Library for handling extremely large integers.
 * * This library provides a structure and functions for arbitrary-precision
 * arithmetic, including basic operations, modular arithmetic, and conversions.
 */

#ifndef BIGINT_H
#define BIGINT_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

extern bool g_error_already_printed;

/**
 * @struct BigInt
 * @brief Structure representing a large integer using signed-magnitude representation.
 * @var BigInt::sign Sign of the number (1 for positive, -1 for negative, 0 for zero).
 * @var BigInt::length Number of active 32-bit words in the digits array.
 * @var BigInt::capacity Total allocated size of the digits array in words.
 * @var BigInt::digits Pointer to the dynamically allocated array of 32-bit words.
 */
typedef struct
{
    int sign;
    size_t length;
    size_t capacity;
    uint32_t* digits;
} BigInt;

/**
 * @brief Allocates and initializes a new BigInt with value zero.
 * @return Pointer to the new BigInt, or NULL if allocation fails.
 */
BigInt* bi_create();

/**
 * @brief Safely deallocates a BigInt and its internal digit array.
 * @param num Pointer to the BigInt to be destroyed.
 */
void bi_destroy(BigInt* num);

/**
 * @brief Creates a deep copy of an existing BigInt.
 * @param original Pointer to the source BigInt.
 * @return Pointer to a new BigInt copy, or NULL if allocation fails.
 */
BigInt* bi_copy(const BigInt* original);

/**
 * @brief Compares absolute values of two BigInts.
 * @param a First operand.
 * @param b Second operand.
 * @return 1 if |a| > |b|, -1 if |a| < |b|, 0 if |a| == |b|.
 */
int bi_compare_abs(const BigInt* a, const BigInt* b);

/**
 * @brief Removes leading zero digits and ensures consistent zero representation.
 * @param num Pointer to the BigInt to normalize.
 */
void bi_normalize(BigInt* num);

/**
 * @brief Returns a new BigInt that is the negation of the input.
 * @param a Pointer to the input BigInt.
 * @return New BigInt with flipped sign.
 */
BigInt* bi_negate(const BigInt* a);

/**
 * @brief Efficiently multiplies the BigInt by 2 (bit shift left by 1).
 * @param n Pointer to the BigInt to be modified in-place.
 */
void bi_shift_left_one(BigInt* n);

/**
 * @brief Retrieves the value of a specific bit.
 * @param n Pointer to the BigInt.
 * @param k Index of the bit (0-indexed).
 * @return 1 if bit is set, 0 otherwise.
 */
int bi_get_bit(const BigInt* n, size_t k);

/**
 * @brief Calculates the number of bits required to represent the absolute value.
 * @param n Pointer to the BigInt.
 * @return Total bit length.
 */
size_t bi_bit_length(const BigInt* n);

/**
 * @brief Adds absolute values: |a| + |b|.
 * @param a First operand.
 * @param b Second operand.
 * @return New BigInt with the sum of absolute values.
 */
BigInt* bi_add_abs(const BigInt* a, const BigInt* b);

/**
 * @brief Subtracts absolute values: |a| - |b|. Assumes |a| >= |b|.
 * @param a First operand.
 * @param b Second operand.
 * @return New BigInt with the difference.
 */
BigInt* bi_sub_abs(const BigInt* a, const BigInt* b);

/**
 * @brief Core division algorithm providing both quotient and remainder for absolute values.
 * @param a Dividend.
 * @param b Divisor.
 * @param quotient Pointer where the result quotient will be stored.
 * @param remainder Pointer where the result remainder will be stored.
 */
void bi_div_mod_abs(const BigInt* a, const BigInt* b, BigInt** quotient, BigInt** remainder);

/**
 * @brief Ensures the internal array has at least the required capacity.
 * @param num BigInt to check/resize.
 * @param required_capacity Minimum required number of words.
 * @return true if successful, false on allocation failure.
 */
bool bi_resize(BigInt* num, size_t required_capacity);

/**
 * @brief Full signed addition: a + b.
 * @param a First operand.
 * @param b Second operand.
 * @return Result BigInt.
 */
BigInt* bi_add(const BigInt* a, const BigInt* b);

/**
 * @brief Full signed subtraction: a - b.
 * @param a First operand.
 * @param b Second operand.
 * @return Result BigInt.
 */
BigInt* bi_sub(const BigInt* a, const BigInt* b);

/**
 * @brief Full signed multiplication: a * b.
 * @param a First operand.
 * @param b Second operand.
 * @return Result BigInt.
 */
BigInt* bi_mul(const BigInt* a, const BigInt* b);

/**
 * @brief Full signed division: a / b.
 * @param a First operand.
 * @param b Second operand.
 * @return Quotient BigInt, or NULL if b is zero.
 */
BigInt* bi_div(const BigInt* a, const BigInt* b);

/**
 * @brief Full signed modulo: a % b.
 * @param a First operand.
 * @param b Second operand.
 * @return Remainder BigInt, or NULL if b is zero.
 */
BigInt* bi_mod(const BigInt* a, const BigInt* b);

/**
 * @brief Exponentiation: base ^ exponent.
 * @param base Base number.
 * @param exponent Unsigned 32-bit exponent.
 * @return Result BigInt.
 */
BigInt* bi_pow(const BigInt* base, BigInt* exponent);

/**
 * @brief Calculates factorial of n.
 * @param n Input value.
 * @return Resulting large integer (n!).
 */
BigInt* bi_fact(uint32_t n);

/**
 * @brief Generic string to BigInt converter (handles 0x, 0b, and dec).
 * @param str Input string.
 * @return New BigInt, or NULL on failure.
 */
BigInt* bi_from_str(const char* str);

/**
 * @brief Converts hexadecimal string to BigInt.
 * @param str Hex string (without 0x).
 * @return New BigInt.
 */
BigInt* bi_from_hex(const char* str);

/**
 * @brief Converts decimal string to BigInt.
 * @param str Decimal string.
 * @return New BigInt.
 */
BigInt* bi_from_dec(const char* str);

/**
 * @brief Converts binary string to BigInt. Handles Two's Complement for fixed lengths.
 * @param str Binary string (without 0b).
 * @return New BigInt.
 */
BigInt* bi_from_bin(const char* str);

/**
 * @brief Converts BigInt to hexadecimal string.
 * @param n BigInt to convert.
 * @return Dynamically allocated string. MUST be freed by caller!
 */
char* bi_to_hex(const BigInt* n);

/**
 * @brief Converts BigInt to decimal string.
 * @param n BigInt to convert.
 * @return Dynamically allocated string. MUST be freed by caller!
 */
char* bi_to_dec(const BigInt* n);

/**
 * @brief Converts BigInt to binary string.
 * @param n BigInt to convert.
 * @return Dynamically allocated string. MUST be freed by caller!
 */
char* bi_to_bin(const BigInt* n);

#endif // BIGINT_H

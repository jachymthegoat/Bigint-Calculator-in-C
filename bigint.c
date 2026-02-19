/**
 * @file bigint.c
 * @brief Implementation of high precision integer arithmetic.
 * * This file contains the core logic for basic BigInt operations.
 * Using 32-bit words (base 2^32) for better performance.
 */

#include "bigint.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <strings.h>

#define INITIAL_CAPACITY 4  /* Initial capacity for digits */
#define BASE_DEC 10         /* Decimal system base */
#define HEX_WIDTH 8         /* Number of digits for one 32bit word */
#define BITS_IN_WORD 32     /* Size of word in bits */
#define HEX_OFFSET 10       /* Shift for letters in hex */
#define DEC_DIVISOR 10      /* Constant for division */

/* HELP FUNCTIONS */

static int hex_value(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + HEX_OFFSET;
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + HEX_OFFSET;
    }
    return -1;
}

static char* custom_strdup(const char* s)
{
    size_t n;
    char* new_str;

    n = strlen(s) + 1;
    new_str = malloc(n);

    if (new_str)
    {
        memcpy(new_str, s, n);
    }
    return new_str;
}

static void bi_add_into_abs(BigInt* res, const BigInt* b)
{
    size_t max_n;
    uint64_t carry = 0;
    size_t i;
    uint32_t val_b;
    uint64_t sum;

    max_n = res->length;
    if (b->length > max_n)
    {
        max_n = b->length;
    }

    if (!bi_resize(res, max_n + 1)) return;

    for (i = 0; i < max_n || carry; i++)
    {
        val_b = 0;

        if (i >= res->length)
        {
            res->digits[i] = 0;
            res->length = i + 1;
        }

        if (i < b->length)
        {
            val_b = b->digits[i];
        }

        sum = (uint64_t)res->digits[i] + val_b + carry;
        res->digits[i] = (uint32_t)sum;
        carry = sum >> BITS_IN_WORD;
    }
    bi_normalize(res);
}

static void bi_sub_into_abs(BigInt* result, const BigInt* b)
{
    int borrow = 0;
    size_t i, j;
    uint64_t diff;

    for (i = 0; i < b->length; i++)
    {
        diff = (uint64_t)result->digits[i] - b->digits[i] - borrow;
        if (diff > result->digits[i])
        {
            borrow = 1;
        }
        else
        {
            borrow = 0;
        }
        result->digits[i] = (uint32_t)diff;
    }

    j = b->length;
    while (borrow && j < result->length)
    {
        if (result->digits[j] > 0)
        {
            result->digits[j] -= 1;
            borrow = 0;
        }
        else
        {
            result->digits[j] = 0xFFFFFFFF;
            borrow = 1;
        }
        j++;
    }
    bi_normalize(result);
}

static void bi_add_digit_into(BigInt* res, uint32_t val)
{
    uint64_t carry = val;
    size_t i;
    uint64_t sum;

    if (val == 0) return;

    if (res->sign == 0)
    {
        res->digits[0] = val;
        res->length = 1;
        res->sign = 1;
        return;
    }

    for (i = 0; i < res->length && carry; i++)
    {
        sum = (uint64_t)res->digits[i] + carry;
        res->digits[i] = (uint32_t)sum;
        carry = sum >> BITS_IN_WORD;
    }

    if (carry)
    {
        if (!bi_resize(res, res->length + 1)) return;
        res->digits[res->length] = (uint32_t)carry;
        res->length++;
    }
    bi_normalize(res);
}

static void bi_mul_digit_into(BigInt* res, uint32_t digit)
{
    uint64_t carry = 0;
    size_t i;
    uint64_t prod;

    if (res->sign == 0 || digit == 0)
    {
        res->digits[0] = 0;
        res->length = 1;
        res->sign = 0;
        return;
    }

    if (!bi_resize(res, res->length + 1)) return;

    for (i = 0; i < res->length; i++)
    {
        prod = (uint64_t)res->digits[i] * digit + carry;
        res->digits[i] = (uint32_t)prod;
        carry = prod >> BITS_IN_WORD;
    }

    if (carry)
    {
        res->digits[res->length] = (uint32_t)carry;
        res->length++;
    }
    bi_normalize(res);
}

static uint32_t bi_div10_into(BigInt* res)
{
    uint64_t remainder = 0;
    uint64_t current;
    int i;

    if (res->sign == 0) return 0;

    /* Iterate from MSB to LSB */
    for (i = (int)res->length - 1; i >= 0; i--)
    {
        current = (uint64_t)res->digits[i] + (remainder << BITS_IN_WORD);
        res->digits[i] = (uint32_t)(current / DEC_DIVISOR);
        remainder = current % DEC_DIVISOR;
    }

    bi_normalize(res);
    return (uint32_t)remainder;
}

/* CORE FUNCTIONS */

BigInt* bi_create()
{
    BigInt* num = (BigInt*)malloc(sizeof(BigInt));

    if (!num)
    {
        return NULL;
    }

    num->digits = (uint32_t*)calloc(INITIAL_CAPACITY, sizeof(uint32_t));

    if (!num->digits)
    {
        free(num);
        return NULL;
    }

    memset(num->digits, 0, INITIAL_CAPACITY * sizeof(uint32_t));

    num->sign = 0;
    num->length = 1;
    num->capacity = INITIAL_CAPACITY;
    num->digits[0] = 0;

    return num;
}

void bi_destroy(BigInt* num)
{
    if (!num) return;

    if (num->digits != NULL)
    {
        free(num->digits);
    }
    free(num);
}

BigInt* bi_copy(const BigInt* original)
{
    BigInt* copy;

    if (!original || !original->digits || original->capacity == 0)
    {
        return NULL;
    }

    copy = (BigInt*)malloc(sizeof(BigInt));
    if (!copy) return NULL;

    copy->digits = (uint32_t*)malloc(original->capacity * sizeof(uint32_t));
    if (!copy->digits)
    {
        free(copy);
        return NULL;
    }

    memcpy(copy->digits, original->digits, original->length * sizeof(uint32_t));

    copy->sign = original->sign;
    copy->length = original->length;
    copy->capacity = original->capacity;

    return copy;
}

bool bi_resize(BigInt* num, size_t required_capacity)
{
    size_t old_capacity;
    uint32_t* new_digits;

    if (!num) return false;

    if (num->capacity >= required_capacity)
    {
        return true;
    }

    old_capacity = num->capacity;
    new_digits = realloc(num->digits, required_capacity * sizeof(uint32_t));

    if (!new_digits)
    {
        return false;
    }

    memset(new_digits + old_capacity, 0, (required_capacity - old_capacity) * sizeof(uint32_t));

    num->digits = new_digits;
    num->capacity = required_capacity;
    return true;
}

void bi_normalize(BigInt* num)
{
    if (!num) return;
    if (!num->digits) return;

    /* Deleting null words */
    while (num->length > 1 && num->digits[num->length - 1] == 0)
    {
        num->length--;
    }

    if (num->length == 1 && num->digits[0] == 0)
    {
        num->sign = 0;
    }
}

/* HELP MATH FUNCTIONS */

int bi_compare_abs(const BigInt* a, const BigInt* b)
{
    size_t i;

    if (!a || !b) return 0;

    if (a->length > b->length) return 1;
    if (a->length < b->length) return -1;

    for (i = a->length; i > 0; i--)
    {
        if (a->digits[i - 1] > b->digits[i - 1]) return 1;
        if (a->digits[i - 1] < b->digits[i - 1]) return -1;
    }

    return 0;
}

int bi_get_bit(const BigInt* n, size_t k)
{
    if (!n) return 0;

    size_t digit_idx = k / BITS_IN_WORD;
    size_t bit_idx = k % BITS_IN_WORD;

    if (digit_idx < n->length) {
        uint32_t digit = n->digits[digit_idx];
        return (int)((digit >> bit_idx) & 1U);
    }

    return 0;
}

size_t bi_bit_length(const BigInt* n)
{
    size_t bits;
    uint32_t last_digit;

    if (!n) return 0;
    if (n->sign == 0) return 0;

    bits = (n->length - 1) * BITS_IN_WORD;
    last_digit = n->digits[n->length - 1];

    while (last_digit > 0)
    {
        last_digit >>= 1;
        bits++;
    }
    return bits;
}

void bi_shift_left_one(BigInt* n)
{
    size_t i;
    uint32_t carry = 0;
    uint32_t next_carry;

    if (!n) return;
    if (n->sign == 0) return;
    if (!bi_resize(n, n->length + 1)) return;

    for (i = 0; i < n->length; i++)
    {
        next_carry = n->digits[i] >> 31;
        n->digits[i] = (n->digits[i] << 1) | carry;
        carry = next_carry;
    }
    if (carry)
    {
        n->digits[n->length] = carry;
        n->length++;
    }
}

/* CORE MATH FUNCTIONS */

BigInt* bi_add_abs(const BigInt* a, const BigInt* b)
{
    BigInt* res = NULL;
    const BigInt* smaller = NULL;

    if (!a || !b) return NULL;

    if (bi_compare_abs(a, b) >= 0)
    {
        res = bi_copy(a);
        smaller = b;
    }
    else
    {
        res = bi_copy(b);
        smaller = a;
    }

    if (!res) return NULL;

    bi_add_into_abs(res, smaller);
    return res;
}

BigInt* bi_sub_abs(const BigInt* a, const BigInt* b)
{
    BigInt* result;

    if (!a || !b) return NULL;

    result = bi_copy(a);
    if (!result) return NULL;

    bi_sub_into_abs(result, b);
    return result;
}

void bi_div_mod_abs(const BigInt* a, const BigInt* b, BigInt** quotient, BigInt** remainder)
{
    BigInt* q;
    BigInt* r;
    size_t bits;
    int i;

    if (!a || !b || b->sign == 0)
    {
        *quotient = NULL;
        *remainder = NULL;
        return;
    }

    if (bi_compare_abs(a, b) < 0)
    {
        *quotient = bi_create();
        *remainder = bi_copy(a);
        return;
    }

    q = bi_create();
    r = bi_create();
    if (!q || !r) { return; }

    bits = bi_bit_length(a);

    for (i = (int)bits - 1; i >= 0; i--)
    {
        bi_shift_left_one(r);
        if (bi_get_bit(a, i))
        {
            r->digits[0] |= 1;
            r->sign = 1;
        }
        bi_normalize(r);

        if (bi_compare_abs(r, b) >= 0)
        {
            bi_sub_into_abs(r, b);

            if (!bi_resize(q, (i / BITS_IN_WORD) + 1))
            {
                bi_destroy(q);
                bi_destroy(r);
                *quotient = NULL;
                *remainder = NULL;
                return;
            }
            q->digits[i / BITS_IN_WORD] |= (1U << (i % BITS_IN_WORD));
            if (q->length <= (size_t)(i / BITS_IN_WORD)) q->length = (i / BITS_IN_WORD) + 1;
            q->sign = 1;
        }
    }
    bi_normalize(q);
    *quotient = q;
    *remainder = r;
}

BigInt* bi_add(const BigInt* a, const BigInt* b)
{
    int cmp;
    BigInt* result;

    if (!a || !b) return NULL;

    if (a->sign == 0) return bi_copy(b);
    if (b->sign == 0) return bi_copy(a);

    if (a->sign == b->sign)
    {
        result = bi_add_abs(a, b);
        if (!result) return NULL;
        result->sign = a->sign;
        return result;
    }

    cmp = bi_compare_abs(a, b);

    if (cmp == 0)
    {
        return bi_create();
    }

    if (cmp > 0)
    {
        result = bi_sub_abs(a, b);
        if (!result) return NULL;
        result->sign = a->sign;
    }
    else
    {
        result = bi_sub_abs(b, a);
        if (!result) return NULL;
        result->sign = b->sign;
    }

    return result;
}

BigInt* bi_sub(const BigInt* a, const BigInt* b)
{
    BigInt* neg_b;
    BigInt* result;

    if (!a || !b) return NULL;

    neg_b = bi_negate(b);
    if (!neg_b) return NULL;

    result = bi_add(a, neg_b);

    bi_destroy(neg_b);

    return result;
}

BigInt* bi_mul(const BigInt* a, const BigInt* b)
{
    size_t total_len;
    BigInt* res;
    size_t i, j, k;
    uint64_t carry;
    uint64_t current;
    uint64_t sum;
    uint64_t ripple;

    if (!a || !b) return NULL;
    if (a->sign == 0 || b->sign == 0) return bi_create();

    total_len = a->length + b->length;
    res = bi_create();

    if (!bi_resize(res, total_len))
    {
        bi_destroy(res);
        return NULL;
    }

    memset(res->digits, 0, total_len * sizeof(uint32_t));
    res->length = total_len;

    for (i = 0; i < a->length; i++)
    {
        carry = 0;
        for (j = 0; j < b->length; j++)
        {
            current = (uint64_t)a->digits[i] * b->digits[j] +
                res->digits[i + j] + carry;
            res->digits[i + j] = (uint32_t)current;
            carry = current >> BITS_IN_WORD;
        }

        if (carry)
        {
            sum = (uint64_t)res->digits[i + b->length] + carry;
            res->digits[i + b->length] = (uint32_t)sum;

            k = i + b->length + 1;
            ripple = sum >> BITS_IN_WORD;
            while (ripple > 0 && k < res->length)
            {
                sum = (uint64_t)res->digits[k] + ripple;
                res->digits[k] = (uint32_t)sum;
                ripple = sum >> BITS_IN_WORD;
                k++;
            }
        }
    }

    if (a->sign == b->sign)
    {
        res->sign = 1;
    }
    else
    {
        res->sign = -1;
    }

    bi_normalize(res);
    return res;
}

BigInt* bi_div(const BigInt* a, const BigInt* b)
{
    BigInt *q, *r;

    if (!a || !b) return NULL;
    if (b->sign == 0) return NULL;

    bi_div_mod_abs(a, b, &q, &r);
    bi_destroy(r);

    if (q)
    {
        if (a->sign == b->sign)
        {
            q->sign = 1;
        }
        else
        {
            q->sign = -1;
        }
        bi_normalize(q);
    }
    return q;
}

BigInt* bi_mod(const BigInt* a, const BigInt* b)
{
    BigInt *q, *r;

    if (!a || !b) return NULL;
    if (b->sign == 0) return NULL;

    bi_div_mod_abs(a, b, &q, &r);
    bi_destroy(q);

    if (r)
    {
        r->sign = a->sign;
        bi_normalize(r);
    }
    return r;
}

BigInt* bi_negate(const BigInt* a)
{
    BigInt* result;

    result = bi_copy(a);
    if (!result) return NULL;

    if (result->sign == 0)
    {
        return result;
    }

    result->sign = -result->sign;
    return result;
}

BigInt* bi_pow(const BigInt* base, BigInt* exponent)
{
    if (!base || !exponent) return NULL;

    /* Handle special cases */
    if (exponent->sign == 0) return bi_from_dec("1");
    if (base->sign == 0) return bi_from_dec("0");

    if (exponent->sign == -1) return bi_from_dec("0");

    BigInt* one = bi_from_dec("1");
    if (!one) return NULL;

    if (bi_compare_abs(base, one) == 0)
    {
        bi_destroy(one);
        if (base->sign == 1) return bi_from_dec("1");
        else
        {
            /* Check if exponent is even */
            BigInt* two = bi_from_dec("2");
            if (!two) return NULL;

            BigInt* mod = bi_mod(exponent, two);
            BigInt* result = NULL;

            if (mod && mod->sign == 0)
            {
                result = bi_from_dec("1");  /* Even */
            }
            else
            {
                result = bi_from_dec("-1"); /* Odd */
            }

            if (mod) bi_destroy(mod);
            bi_destroy(two);
            return result;
        }
    }
    bi_destroy(one);

    /* Exponentiation by squaring algorithm */
    BigInt* result = bi_from_dec("1");
    BigInt* current = bi_copy(base);
    BigInt* n = bi_copy(exponent);
    BigInt* zero = bi_from_dec("0");
    BigInt* two = bi_from_dec("2");

    if (!result || !current || !n || !zero || !two)
    {
        bi_destroy(result);
        bi_destroy(current);
        bi_destroy(n);
        bi_destroy(zero);
        bi_destroy(two);
        return NULL;
    }

    while (bi_compare_abs(n, zero) > 0)
    {
        BigInt* mod = bi_mod(n, two);
        if (!mod)
        {
            bi_destroy(result);
            bi_destroy(current);
            bi_destroy(n);
            bi_destroy(zero);
            bi_destroy(two);
            return NULL;
        }

        bool is_odd = (mod->sign != 0);
        bi_destroy(mod);

        if (is_odd)
        {
            BigInt* temp = bi_mul(result, current);
            if (!temp)
            {
                bi_destroy(result);
                bi_destroy(current);
                bi_destroy(n);
                bi_destroy(zero);
                bi_destroy(two);
                return NULL;
            }
            bi_destroy(result);
            result = temp;
        }

        /* n = n / 2 */
        BigInt* new_n = bi_div(n, two);
        if (!new_n)
        {
            bi_destroy(result);
            bi_destroy(current);
            bi_destroy(n);
            bi_destroy(zero);
            bi_destroy(two);
            return NULL;
        }
        bi_destroy(n);
        n = new_n;

        if (bi_compare_abs(n, zero) <= 0) break;

        /* current = current * current */
        BigInt* temp = bi_mul(current, current);
        if (!temp)
        {
            bi_destroy(result);
            bi_destroy(current);
            bi_destroy(n);
            bi_destroy(zero);
            bi_destroy(two);
            return NULL;
        }
        bi_destroy(current);
        current = temp;
    }

    bi_destroy(current);
    bi_destroy(n);
    bi_destroy(zero);
    bi_destroy(two);

    return result;
}

BigInt* bi_fact(uint32_t n)
{
    BigInt* res;
    uint32_t i;

    if (n == 0 || n == 1)
    {
        return bi_from_dec("1");
    }
    res = bi_create();
    res->digits[0] = 1;
    res->sign = 1;

    for (i = 2; i <= n; i++)
    {
        bi_mul_digit_into(res, i);
    }

    return res;
}

/* CONVERSIONS */

BigInt* bi_from_str(const char* str)
{
    BigInt* num = NULL;
    int sign = 1;
    const char* p = str;

    if (!str) return NULL;

    while (*p != '\0' && isspace((unsigned char)*p))
    {
        p++;
    }

    if (*p == '-')
    {
        sign = -1;
        p++;
    }
    else if (*p == '+')
    {
        p++;
    }

    while (*p != '\0' && isspace((unsigned char)*p))
    {
        p++;
    }

    if (p[0] == '0' && (p[1] == 'b' || p[1] == 'B'))
    {
        num = bi_from_bin(p + 2);
    }
    else if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
    {
        num = bi_from_hex(p + 2);
    }
    else
    {
        num = bi_from_dec(p);
    }

    if (!num) return NULL;

    if (num->sign != 0)
    {
        num->sign *= sign;
    }

    return num;
}

BigInt* bi_from_hex(const char* str)
{
    size_t n;
    size_t required_digits;
    BigInt* num;
    size_t i;
    size_t digit_index = 0;
    uint32_t current_digit;
    uint32_t shift;
    int j;
    int val;
    int first_val;
    size_t bits_used;
    uint32_t mask;
    size_t k;

    n = strlen(str);

    if (!str || *str == '\0') return NULL;

    for (i = 0; i < n; i++)
    {
        if (hex_value(str[i]) == -1)
        {
            return NULL;
        }
    }

    required_digits = (n + 7) / HEX_WIDTH;

    num = bi_create();
    if (!num) return NULL;

    if (!bi_resize(num, required_digits))
    {
        bi_destroy(num);
        return NULL;
    }
    memset(num->digits, 0, num->capacity * sizeof(uint32_t));
    num->length = required_digits;

    num->sign = 1;

    i = n;
    while (i > 0)
    {
        current_digit = 0;
        shift = 0;

        for (j = 0; j < HEX_WIDTH && i > 0; j++)
        {
            i--;
            val = hex_value(str[i]);
            if (val == -1)
            {
                bi_destroy(num);
                return NULL;
            }
            current_digit |= ((uint32_t)val << shift);
            shift += 4;
        }
        num->digits[digit_index++] = current_digit;
    }

    num->sign = 1;

    first_val = hex_value(str[0]);

    if (first_val >= 8)
    {
        bits_used = (n % HEX_WIDTH) * 4;
        if (bits_used == 0) bits_used = BITS_IN_WORD;

        if (bits_used < BITS_IN_WORD)
        {
            mask = 0xFFFFFFFF << bits_used;
            num->digits[num->length - 1] |= mask;
        }

        for (k = 0; k < num->length; k++)
        {
            num->digits[k] = ~num->digits[k];
        }

        bi_add_digit_into(num, 1);
        num->sign = -1;
    }

    bi_normalize(num);
    return num;
}

BigInt* bi_from_dec(const char* str)
{
    BigInt* res;
    const char* p;

    if (!str || *str == '\0') return NULL;

    res = bi_create();
    if (!res) return NULL;

    p = str;
    while (*p)
    {
        if (*p >= '0' && *p <= '9')
        {
            bi_mul_digit_into(res, BASE_DEC);
            bi_add_digit_into(res, *p - '0');
        }
        p++;
    }

    bi_normalize(res);
    return res;
}

BigInt* bi_from_bin(const char* str)
{
    const char* p;
    size_t n;
    BigInt* res;
    size_t needed_words;
    size_t i;
    size_t bit;
    size_t word_idx;
    size_t bit_idx;
    size_t z;

    if (!str) return NULL;

    p = str;
    if (p[0] == '0' && (p[1] == 'b' || p[1] == 'B')) p += 2;

    const char* check = p;
    while (*check != '\0')
    {
        if (*check != '0' && *check != '1')
        {
            return NULL;
        }
        check++;
    }

    n = strlen(p);
    if (n == 0) return NULL;

    res = bi_create();
    if (!res) return NULL;

    needed_words = (n + BITS_IN_WORD - 1) / BITS_IN_WORD;
    if (!bi_resize(res, needed_words))
    {
        bi_destroy(res);
        return NULL;
    }

    res->sign = 1;

    for (i = 0; i < n; i++)
    {
        bi_shift_left_one(res);
        if (p[i] == '1')
        {
            res->digits[0] |= 1;
            res->sign = 1;
        }
    }

    if (p[0] == '1')
    {
        for (bit = n; bit < res->capacity * BITS_IN_WORD; bit++)
        {
            word_idx = bit / BITS_IN_WORD;
            bit_idx = bit % BITS_IN_WORD;
            if (word_idx < res->length)
            {
                res->digits[word_idx] |= (1U << bit_idx);
            }
        }

        for (z = 0; z < res->length; z++)
        {
            res->digits[z] = ~res->digits[z];
        }

        bi_add_digit_into(res, 1);
        res->sign = -1;
    }

    bi_normalize(res);
    return res;
}

char* bi_to_dec(const BigInt* n)
{
    BigInt* copy;
    size_t max_digits;
    char* buffer;
    int written = 0;
    uint32_t remainder;
    char* result;
    int v = 0;
    int i;

    if (!n) return NULL;
    if (n->sign == 0) return custom_strdup("0");

    copy = bi_copy(n);
    if (!copy) return NULL;
    copy->sign = 1;

    max_digits = (n->length * BASE_DEC) + 2;
    buffer = (char*)malloc(max_digits);
    if (!buffer)
    {
        bi_destroy(copy);
        return NULL;
    }

    while (copy->sign != 0)
    {
        remainder = bi_div10_into(copy);
        buffer[written++] = (char)(remainder + '0');
    }

    result = (char*)malloc(written + 2);
    if (!result)
    {
        free(buffer);
        bi_destroy(copy);
        return NULL;
    }

    if (n->sign == -1) result[v++] = '-';

    for (i = written - 1; i >= 0; i--)
    {
        result[v++] = buffer[i];
    }
    result[v] = '\0';

    free(buffer);
    bi_destroy(copy);
    return result;
}

char* bi_to_hex(const BigInt* n)
{
    BigInt* working;
    size_t i;
    size_t hex_len;
    char* raw_buffer;
    int j;
    char* start;
    char next;
    bool next_is_negative_marker;
    char* result;

    if (!n) return NULL;
    if (n->sign == 0) return custom_strdup("0x0");

    working = bi_copy(n);
    if (!working) return NULL;

    if (n->sign == -1)
    {
        for (i = 0; i < working->length; i++)
        {
            working->digits[i] = ~working->digits[i];
        }
        bi_add_digit_into(working, 1);
    }

    hex_len = working->length * HEX_WIDTH;
    raw_buffer = (char*)malloc(hex_len + 1);
    if (!raw_buffer)
    {
        bi_destroy(working);
        return NULL;
    }

    for (j = (int)working->length - 1; j >= 0; j--)
    {
        sprintf(raw_buffer + (working->length - 1 - j) * HEX_WIDTH, "%08x", working->digits[j]);
    }

    start = raw_buffer;

    if (n->sign == 1)
    {
        char* original_start = start;
        /* Skip leading zeros but keep at least one digit */
        while (*start == '0' && *(start + 1) != '\0')
        {
            start++;
        }

        /* Check if first character is 8-f or A-F */
        char first_char = *start;
        int needs_leading_zero = 0;

        if ((first_char >= '8' && first_char <= '9') ||
            (first_char >= 'a' && first_char <= 'f') ||
            (first_char >= 'A' && first_char <= 'F'))
        {
            needs_leading_zero = 1;
        }

        if (needs_leading_zero && start != original_start)
        {
            start--;
        }
    }
    else
    {
        while (*start == 'f')
        {
            next = *(start + 1);

            if (next == '\0') break;

            next_is_negative_marker = (next >= '8' && next <= '9') ||
                (next >= 'a' && next <= 'f') ||
                (next >= 'A' && next <= 'F');

            if (next_is_negative_marker)
            {
                start++;
            }
            else
            {
                break;
            }
        }
    }

    result = (char*)malloc(strlen(start) + 3);
    if (!result)
    {
        free(raw_buffer);
        bi_destroy(working);
        return NULL;
    }
    sprintf(result, "0x%s", start);

    free(raw_buffer);
    bi_destroy(working);
    return result;
}

char* bi_to_bin(const BigInt* n)
{
    BigInt* working;
    char* buffer;
    char* start;
    char* result;
    size_t bit_len;
    size_t total_bits;
    int pos;
    int i;
    size_t z;

    if (!n) return NULL;
    if (n->sign == 0) return custom_strdup("0b0");

    working = bi_copy(n);
    if (!working) return NULL;

    if (n->sign == -1)
    {
        for (z = 0; z < working->length; z++)
        {
            working->digits[z] = ~working->digits[z];
        }
        bi_add_digit_into(working, 1);
        working->sign = 1;
    }

    bit_len = bi_bit_length(working);
    total_bits = bit_len + 1;

    buffer = (char*)malloc(total_bits + 1);
    if (!buffer)
    {
        bi_destroy(working);
        return NULL;
    }

    pos = 0;

    if (n->sign == 1)
    {
        buffer[pos] = '0';
    }
    else
    {
        buffer[pos] = '1';
    }
    pos++;

    for (i = (int)bit_len - 1; i >= 0; i--)
    {
        int bit = bi_get_bit(working, i);
        if (bit)
        {
            buffer[pos] = '1';
        }
        else
        {
            buffer[pos] = '0';
        }
        pos++;
    }
    buffer[pos] = '\0';

    start = buffer;

    if (n->sign == 1)
    {
        while (start[0] == '0' && start[1] == '0')
        {
            start++;
        }
    }
    else
    {
        while (start[0] == '1' && start[1] == '1')
        {
            start++;
        }
    }

    result = (char*)malloc(strlen(start) + 3);
    if (!result)
    {
        free(buffer);
        bi_destroy(working);
        return NULL;
    }

    sprintf(result, "0b%s", start);

    free(buffer);
    bi_destroy(working);
    return result;
}

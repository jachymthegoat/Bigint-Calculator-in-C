/**
* @file parser.c
 * @brief Mathematical expression parser and evaluator.
 * * Implements the Shunting-yard algorithm to convert infix expressions
 * to RPN and evaluates them using BigInt arithmetic.
 */

#include "parser.h"
#include "stack.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_STACK_SIZE 32
#define INITIAL_NUMBER_BUFFER_SIZE 32

static bool is_operator(char c)
{
    return c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '%' || c == '^' || c == '!' || c == '(' || c == ')';
}

static bool validate_expression_syntax(const char* input)
{
    if (!input) return false;

    int i = 0;
    bool expect_operand = true;
    bool last_was_operator = false;
    int paren_depth = 0;

    while (input[i] != '\0')
    {

        if (isspace(input[i]))
        {
            i++;
            continue;
        }

        /* Check for numbers */
        if (isdigit(input[i]))
        {
            if (!expect_operand)
            {
                /* Two operands in a row without operator */
                return false;
            }

            /* Parse the number strictly */
            if (input[i] == '0' && (input[i + 1] == 'x' || input[i + 1] == 'X'))
            {
                /* Hexadecimal */
                i += 2;
                if (!isxdigit(input[i])) return false;
                while (isxdigit(input[i])) i++;
            }
            else if (input[i] == '0' && (input[i + 1] == 'b' || input[i + 1] == 'B'))
            {
                /* Binary */
                i += 2;
                if (input[i] != '0' && input[i] != '1') return false;
                while (input[i] == '0' || input[i] == '1') i++;
            }
            else
            {
                /* Decimal */
                while (isdigit(input[i])) i++;
            }

            expect_operand = false;
            last_was_operator = false;
            continue;
        }

        /* Check for operators */
        if (is_operator(input[i]))
        {
            if (input[i] == '(')
            {
                if (!expect_operand)
                {
                    /* Something like "5(" is invalid */
                    return false;
                }
                i++;
                paren_depth++;
                expect_operand = true;
                last_was_operator = true;
                continue;
            }
            else if (input[i] == ')')
            {
                if (expect_operand || paren_depth <= 0)
                {
                    return false;
                }
                i++;
                paren_depth--;
                expect_operand = false;
                last_was_operator = false;
                continue;
            }
            else if (input[i] == '!')
            {
                if (expect_operand)
                {
                    return false;
                }
                i++;
                expect_operand = false;
                last_was_operator = false;
                continue;
            }
            else
            {
                /* Binary operators: +, -, *, /, %, ^ */
                if (input[i] == '-' || input[i] == '+')
                {
                    bool is_unary = true;
                    int j = i - 1;
                    while (j >= 0 && isspace(input[j])) j--;

                    if (j >= 0)
                    {
                        if (input[i] == '-')  /* Minus */
                        {
                            /* Minus can be unary after '(' or any operator */
                            if (input[j] != '(' && !is_operator(input[j]))
                            {
                                is_unary = false;
                            }
                            if (input[j] == '!')
                            {
                                is_unary = false;
                            }
                        }
                        else  /* Plus */
                        {
                            /* Plus can be unary only after '(' */
                            if (input[j] != '(')
                            {
                                is_unary = false;
                            }
                        }
                    }

                    if (is_unary)
                    {
                        i++;
                        last_was_operator = true;
                        continue;
                    }
                }

                /* Check for consecutive binary operators */
                if (last_was_operator && input[i] != '!')
                {
                    return false;
                }

                i++;
                expect_operand = true;
                last_was_operator = true;
                continue;
            }
        }
        return false;
    }

    if (paren_depth != 0)
    {
        return false;
    }

    if (expect_operand)
    {
        return false;
    }

    /* Check if expression ends with a binary operator */
    int len = (int)strlen(input);
    while (len > 0 && isspace(input[len - 1])) len--;
    if (len > 0 && strchr("+-*/%^", input[len - 1]))
    {
        return false;
    }

    return true;
}

static int get_priority(char op)
{
    switch (op)
    {
    case '+':
    case '-': return 1;
    case '*':
    case '/':
    case '%': return 2;
    case '^': return 3;
    case '!': return 4;
    case 'm': return 3; /* Unary minus */
    default: return 0;
    }
}

static bool apply_operation(BigIntStack* num_stack, char op, bool* error_already_printed)
{
    if (stack_is_empty(num_stack)) return false;
    BigInt* result = NULL;
    BigInt* right = stack_pop(num_stack);

    if (op == '!')
    {
        if (right->sign == -1)
        {
            printf("Input of factorial must not be negative!\n");
            *error_already_printed = true;
            bi_destroy(right);
            return false;
        }
        if (right->length > 1)
        {
            bi_destroy(right);
            return false;
        }
        result = bi_fact(right->digits[0]);
        bi_destroy(right);
    }
    else if (op == 'm')
    {
        BigInt* res = bi_negate(right);
        bi_destroy(right);
        if (res)
        {
            stack_push(num_stack, res);
            return true;
        }
        return false;
    }
    else
    {
        if (stack_is_empty(num_stack))
        {
            bi_destroy(right);
            return false;
        }
        BigInt* left = stack_pop(num_stack);

        /* Division by zero */
        if ((op == '/' || op == '%') && right->sign == 0)
        {
            printf("Division by zero!\n");
            *error_already_printed = true;

            bi_destroy(left);
            bi_destroy(right);
            return false;
        }

        switch (op)
        {
        case '+': result = bi_add(left, right);
            break;
        case '-': result = bi_sub(left, right);
            break;
        case '*': result = bi_mul(left, right);
            break;
        case '/': result = bi_div(left, right);
            break;
        case '%': result = bi_mod(left, right);
            break;
        case '^': result = bi_pow(left, right);
            break;

        default: break;
        }
        bi_destroy(left);
        bi_destroy(right);
    }

    if (result)
    {
        stack_push(num_stack, result);
        return true;
    }
    return false;
}

BigInt* eval_expression(const char* input, bool* error_already_printed)
{
    if (!input) return NULL;

    if (!validate_expression_syntax(input)) return NULL;

    BigIntStack* num_stack = stack_create(INITIAL_STACK_SIZE);
    if (!num_stack) return NULL;
    CharStack* op_stack = char_stack_create(INITIAL_STACK_SIZE);
    if (!op_stack)
    {
        stack_destroy(num_stack, true);
        return NULL;
    }

    bool can_be_sign = true;
    int i = 0;

    while (input[i] != '\0')
    {
        if (isspace(input[i]))
        {
            i++;
            continue;
        }

        if (isdigit(input[i]))
        {
            size_t capacity = INITIAL_NUMBER_BUFFER_SIZE;
            size_t length = 0;
            char* dynamic_tmp_buf = malloc(capacity);

            if (!dynamic_tmp_buf)
            {
                stack_destroy(num_stack, true);
                char_stack_destroy(op_stack);
                return NULL;
            }

            while (input[i] != '\0' && (isxdigit(input[i]) ||
                input[i] == 'x' || input[i] == 'X' ||
                input[i] == 'b' || input[i] == 'B'))
            {
                if (length + 1 >= capacity)
                {
                    capacity *= 2;
                    char* new_buf = realloc(dynamic_tmp_buf, capacity);
                    if (!new_buf)
                    {
                        free(dynamic_tmp_buf);
                        stack_destroy(num_stack, true);
                        char_stack_destroy(op_stack);
                        return NULL;
                    }
                    dynamic_tmp_buf = new_buf;
                }
                dynamic_tmp_buf[length++] = input[i++];
            }
            dynamic_tmp_buf[length] = '\0';

            stack_push(num_stack, bi_from_str(dynamic_tmp_buf));
            free(dynamic_tmp_buf);

            can_be_sign = false;
            continue;
        }

        if (input[i] == '(')
        {
            char_stack_push(op_stack, input[i++]);
            can_be_sign = true;
            continue;
        }

        if (input[i] == ')')
        {
            while (!char_stack_is_empty(op_stack) && char_stack_peek(op_stack) != '(')
            {
                if (!apply_operation(num_stack, char_stack_pop(op_stack),error_already_printed))
                {
                    stack_destroy(num_stack, true);
                    char_stack_destroy(op_stack);
                    return NULL;
                }
            }
            char_stack_pop(op_stack);
            i++;
            can_be_sign = false;
            continue;
        }

        /* Binary operators */
        if (input[i] == '+' || input[i] == '-' || input[i] == '*' ||
            input[i] == '/' || input[i] == '%' || input[i] == '^' || input[i] == '!')
        {
            char curr_op = input[i];

            /* Handle unary operators */
            if (can_be_sign)
            {
                if (curr_op == '-')
                {
                    curr_op = 'm';
                }
                else if (curr_op == '+')
                {
                    i++;
                    continue;
                }
                else
                {
                    /* *, /, %, ^, ! cannot be unary */
                    stack_destroy(num_stack, true);
                    char_stack_destroy(op_stack);
                    return NULL;
                }
            }

            while (!char_stack_is_empty(op_stack) &&
                get_priority(char_stack_peek(op_stack)) >= get_priority(curr_op))
            {
                if (curr_op == 'm') break;
                if (curr_op == '^' && char_stack_peek(op_stack) == '^') break;
                if (curr_op == '^' && get_priority(char_stack_peek(op_stack)) == get_priority(curr_op)) break;

                if (!apply_operation(num_stack, char_stack_pop(op_stack),error_already_printed))
                {
                    stack_destroy(num_stack, true);
                    char_stack_destroy(op_stack);
                    return NULL;
                }
            }

            char_stack_push(op_stack, curr_op);
            i++;
            if (curr_op == '!') can_be_sign = false;
            else can_be_sign = true;
            continue;
        }

        i++;
    }

    while (!char_stack_is_empty(op_stack))
    {
        if (!apply_operation(num_stack, char_stack_pop(op_stack),error_already_printed))
        {
            stack_destroy(num_stack, true);
            char_stack_destroy(op_stack);
            return NULL;
        }
    }

    BigInt* final_result = stack_pop(num_stack);

    stack_destroy(num_stack, true);
    char_stack_destroy(op_stack);
    return final_result;
}
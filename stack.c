/**
* @file stack.c
 * @brief Implementation of stack structures for operands and operators.
 * * Provides dynamic resizing stacks for BigInt pointers and characters
 * to facilitate the Shunting-yard algorithm and expression evaluation.
 */

#include "stack.h"
#include <stdlib.h>

/* BigInt Stack */

BigIntStack* stack_create(int capacity)
{
    BigIntStack* stack = (BigIntStack*)malloc(sizeof(BigIntStack));
    if (!stack) return NULL;

    stack->data = (BigInt**)malloc(capacity * sizeof(BigInt*));
    if (!stack->data)
    {
        free(stack);
        return NULL;
    }

    stack->top = -1;
    stack->capacity = capacity;
    return stack;
}

void stack_destroy(BigIntStack* stack, bool destroy_elements)
{
    if (!stack) return;

    if (destroy_elements)
    {
        while (!stack_is_empty(stack))
        {
            bi_destroy(stack_pop(stack));
        }
    }

    free(stack->data);
    free(stack);
}

bool stack_push(BigIntStack* stack, BigInt* val)
{
    if (!stack) return false;

    /* If full, double the size */
    if (stack->top >= stack->capacity - 1)
    {
        int new_capacity = stack->capacity * 2;
        BigInt** new_data = (BigInt**)realloc(stack->data, new_capacity * sizeof(BigInt*));

        if (!new_data)
        {
            return false;
        }

        stack->data = new_data;
        stack->capacity = new_capacity;
    }

    stack->top++;
    stack->data[stack->top] = val;
    return true;
}

BigInt* stack_pop(BigIntStack* stack)
{
    if (stack_is_empty(stack)) return NULL;
    return stack->data[stack->top--];
}

BigInt* stack_peek(const BigIntStack* stack)
{
    if (stack_is_empty(stack)) return NULL;
    return stack->data[stack->top];
}

bool stack_is_empty(const BigIntStack* stack)
{
    return stack->top == -1;
}

/* Char Stack */

CharStack* char_stack_create(int capacity)
{
    CharStack* stack = (CharStack*)malloc(sizeof(CharStack));
    if (!stack) return NULL;
    stack->data = (char*)malloc(capacity * sizeof(char));
    if (!stack->data)
    {
        free(stack);
        return NULL;
    }
    stack->top = -1;
    stack->capacity = capacity;
    return stack;
}

void char_stack_destroy(CharStack* stack)
{
    if (stack)
    {
        free(stack->data);
        free(stack);
    }
}

bool char_stack_push(CharStack* stack, char val)
{
    if (stack->top >= stack->capacity - 1)
    {
        int new_limit = stack->capacity * 2;
        char* new = (char*)realloc(stack->data, new_limit * sizeof(char));
        if (!new) return false;
        stack->data = new;
        stack->capacity = new_limit;
    }
    stack->data[++stack->top] = val;
    return true;
}

char char_stack_pop(CharStack* stack)
{
    if (char_stack_is_empty(stack))
        return '\0';
    else
        return stack->data[stack->top--];
}

char char_stack_peek(const CharStack* stack)
{
    if (char_stack_is_empty(stack))
        return '\0';
    else
        return stack->data[stack->top];
}

bool char_stack_is_empty(const CharStack* stack)
{
    return stack->top == -1;
}

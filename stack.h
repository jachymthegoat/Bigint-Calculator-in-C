/**
 * @file stack.h
 * @brief Header file for stack implementations used in expression parsing.
 * * Provides two types of stacks: one for BigInt pointers (operands)
 * and one for characters (operators).
 */

#ifndef STACK_H
#define STACK_H

#include "bigint.h"

/**
 * @struct BigIntStack
 * @brief Dynamic stack for storing pointers to BigInt structures.
 * @var BigIntStack::data Pointer to the array of BigInt pointers.
 * @var BigIntStack::top Index of the current top element (-1 if empty).
 * @var BigIntStack::capacity Total number of elements currently allocated.
 */
typedef struct
{
    BigInt** data;
    int top;
    int capacity;
} BigIntStack;

/**
 * @struct CharStack
 * @brief Simple dynamic stack for storing operator characters.
 * @var CharStack::data Pointer to the character array.
 * @var CharStack::top Index of the current top element.
 * @var CharStack::capacity Total number of characters currently allocated.
 */
typedef struct
{
    char* data;
    int top;
    int capacity;
} CharStack;

/* --- BigInt Stack Functions --- */

/**
 * @brief Creates a new BigIntStack with the specified initial capacity.
 * @param capacity Initial number of elements.
 * @return Pointer to the new stack, or NULL on allocation failure.
 */
BigIntStack* stack_create(int capacity);

/**
 * @brief Safely destroys the stack and optionally all BigInt elements within it.
 * @param stack Pointer to the stack.
 * @param destroy_elements If true, bi_destroy() is called on every item in the stack.
 */
void stack_destroy(BigIntStack* stack, bool destroy_elements);

/**
 * @brief Pushes a new BigInt pointer onto the stack. Resizes automatically if full.
 * @param stack Pointer to the stack.
 * @param val Pointer to the BigInt to store.
 * @return true if successful, false on memory failure.
 */
bool stack_push(BigIntStack* stack, BigInt* val);

/**
 * @brief Removes and returns the top element from the stack.
 * @param stack Pointer to the stack.
 * @return Pointer to the BigInt, or NULL if the stack is empty.
 */
BigInt* stack_pop(BigIntStack* stack);

/**
 * @brief Returns the top element without removing it.
 * @param stack Pointer to the stack.
 * @return Pointer to the BigInt, or NULL if the stack is empty.
 */
BigInt* stack_peek(const BigIntStack* stack);

/**
 * @brief Checks if the BigIntStack has no elements.
 * @param stack Pointer to the stack.
 * @return true if empty, false otherwise.
 */
bool stack_is_empty(const BigIntStack* stack);

/* --- Char Stack Functions --- */

/**
 * @brief Creates a new CharStack for operator storage.
 * @param capacity Initial number of characters.
 * @return Pointer to the new stack, or NULL on failure.
 */
CharStack* char_stack_create(int capacity);

/**
 * @brief Destroys the CharStack and frees its internal buffer.
 * @param stack Pointer to the stack.
 */
void char_stack_destroy(CharStack* stack);

/**
 * @brief Pushes a character onto the operator stack. Resizes if needed.
 * @param stack Pointer to the stack.
 * @param val The operator character.
 * @return true if successful, false on failure.
 */
bool char_stack_push(CharStack* stack, char val);

/**
 * @brief Removes and returns the top character.
 * @param stack Pointer to the stack.
 * @return The character, or '\0' if empty.
 */
char char_stack_pop(CharStack* stack);

/**
 * @brief Returns the top character without removing it.
 * @param stack Pointer to the stack.
 * @return The character, or '\0' if empty.
 */
char char_stack_peek(const CharStack* stack);

/**
 * @brief Checks if the CharStack is empty.
 * @param stack Pointer to the stack.
 * @return true if empty, false otherwise.
 */
bool char_stack_is_empty(const CharStack* stack);

#endif

/**
* @file parser.h
 * @brief Mathematical expression parser interface
 */

#ifndef PARSER_H
#define PARSER_H

#include "bigint.h"
#include "stack.h"

#define POW_DIGITS_LIMIT 10 /* Maximum exponent size for pow operation */

/**
 * @brief Evaluates a mathematical expression
 * @param input The expression string to evaluate
 * @param error_already_printed Pointer to flag indicating if error was printed
 * @return Result as BigInt, or NULL on error
 */
BigInt* eval_expression(const char* input, bool* error_already_printed);

#endif /* PARSER_H */
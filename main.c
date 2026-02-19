#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "bigint.h"
#include "stack.h"
#include "parser.h"

#define MAX_LINE_BUFFER 2048    /* Max size of row length*/
#define MAX_EXPR_BUFFER 16384   /* Max multi line expression length */
#define MAX_CMD_NAME 256        /* Max command name length */
#define BASE_DEC 10             /* Decimal output*/
#define BASE_HEX 16             /* Hexadecimal output */
#define BASE_BIN 2              /* Binary output */

/* Global output system */
int output_num_system = BASE_DEC;

void process_and_print(const char* row)
{
    if (!row || strlen(row) < 1) return;

    /* Ignoring number of rows */
    const char* p = row;
    while (*p && isspace(*p)) p++;

    if (strcmp(p, "quit") == 0) {
        printf("quit\n");
        return;
    }

    /* Command "out" writes current num system*/
    if (strstr(p, "out") == p)
    {
        if (output_num_system == BASE_HEX) printf("hex\n");
        else if (output_num_system == BASE_BIN) printf("bin\n");
        else printf("dec\n");
        return;
    }

    if (strstr(p, "hex") == p)
    {
        output_num_system = BASE_HEX;
        printf("hex\n");
        return;
    }

    if (strstr(p, "bin") == p)
    {
        output_num_system = BASE_BIN;
        printf("bin\n");
        return;
    }

    if (strstr(p, "dec") == p)
    {
        output_num_system = BASE_DEC;
        printf("dec\n");
        return;
    }


    /* If it starts with letter (not a number) it is invalid */
    if (isalpha((unsigned char)*p))
    {
        bool is_expression = false;
        size_t k;
        for (k = 0; p[k] != '\0'; k++)
        {
            /* If it is operator, leave it for parser */
            if (strchr("+-*/%^()!", p[k]))
            {
                is_expression = true;
                break;
            }
        }

        if (!is_expression)
        {
            /* Delete spaces */
            char cmd_name[MAX_CMD_NAME];
            strncpy(cmd_name, p, MAX_CMD_NAME - 1);
            cmd_name[MAX_CMD_NAME - 1] = '\0';

            size_t n = strlen(cmd_name);
            while (n > 0 && isspace((unsigned char)cmd_name[n - 1]))
            {
                cmd_name[n - 1] = '\0';
                n--;
            }

            printf("Invalid command \"%s\"!\n", cmd_name);
            return;
        }
    }

    bool error_already_printed = false;
    BigInt* result = eval_expression(p, &error_already_printed);

    if (result)
    {
        char* text;
        if (output_num_system == BASE_HEX) text = bi_to_hex(result);
        else if (output_num_system == BASE_BIN) text = bi_to_bin(result);
        else text = bi_to_dec(result);

        if (text)
        {
            printf("%s\n", text);
            free(text);
        }
        bi_destroy(result);
    }
    else
    {
        if (!error_already_printed)
        {
            printf("Syntax error!\n");
        }
    }
}

bool is_unfinished(const char* text)
{
    size_t n = strlen(text);
    char c;

    if (n == 0) return false;

    while (n > 0 && isspace((unsigned char)text[n - 1]))
    {
        n--;
    }
    if (n == 0) return false;

    c = text[n - 1];

    if (strchr("+-*/%^(", c))
    {
        return true;
    }

    if ((c == 'x' || c == 'X') && n > 1)
    {
        if (text[n - 2] == '0') return true;
    }

    if ((c == 'b' || c == 'B') && n > 1)
    {
        if (text[n - 2] == '0')
        {
            if (n == 2) return true;

            char prev_prev = text[n - 3];

            if (prev_prev == 'x' || prev_prev == 'X') return false;

            if (isxdigit(prev_prev)) return false;

            return true;
        }
    }

    return false;
}

int main(int argc, char* argv[])
{
    char row[MAX_LINE_BUFFER];
    /* Buffer for fusing multiple rows */
    char accumulated_row[MAX_EXPR_BUFFER];
    accumulated_row[0] = '\0';

    if (argc > 1)
    {
        FILE* f = fopen(argv[1], "r");
        if (!f)
        {
            printf("Invalid input file!\n");
            return EXIT_FAILURE;
        }

        while (fgets(row, sizeof(row), f))
        {
            /* Deleting the newline chars */
            row[strcspn(row, "\r\n")] = 0;

            if (strcmp(row, "quit") == 0) {
                printf("> quit\n");
                printf("quit\n");
                break;
            }

            /* If row is empty, and we have nothing ongoing, ignore it */
            if (strlen(row) == 0 && strlen(accumulated_row) == 0) continue;

            /* If not too big, fuse the rows */
            if (strlen(accumulated_row) + strlen(row) >= MAX_EXPR_BUFFER)
            {
                return EXIT_FAILURE;
            }
            strcat(accumulated_row, row);

            if (is_unfinished(accumulated_row))
            {
                /* In file mode: unfinished expression at line end is syntax error */
                printf("> %s\n", accumulated_row);
                printf("Syntax error!\n");
                accumulated_row[0] = '\0';
            }
            else
            {
                printf("> %s\n", accumulated_row);
                process_and_print(accumulated_row);
                accumulated_row[0] = '\0';
            }
        }
        
        /* After reading whole file, process any remaining incomplete expression */
        if (strlen(accumulated_row) > 0)
        {
            printf("> %s\n", accumulated_row);
            process_and_print(accumulated_row);
            accumulated_row[0] = '\0';
        }
        
        fclose(f);
    }

    /* Interaction mode */
    else
    {
        while (1)
        {
            /* If something is ongoing, write a different prompt */
            if (strlen(accumulated_row) > 0) printf("... ");
            else printf("> ");

            if (!fgets(row, sizeof(row), stdin)) break;
            row[strcspn(row, "\r\n")] = 0;

            if (strcmp(row, "quit") == 0) {
                printf("quit\n");
                break;
            }

            if (strlen(accumulated_row) + strlen(row) >= MAX_EXPR_BUFFER)
            {
                accumulated_row[0] = '\0';
                continue;
            }

            strcat(accumulated_row, row);

            if (is_unfinished(accumulated_row))
            {
                size_t n = strlen(accumulated_row);
                char last = accumulated_row[n - 1];

                if (last != 'x' && last != 'X' && last != 'b' && last != 'B')
                {
                    strcat(accumulated_row, " ");
                }

                continue;
            }

            process_and_print(accumulated_row);
            accumulated_row[0] = '\0';
        }
    }

    return EXIT_SUCCESS;
}
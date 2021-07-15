#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pat8.h"

char *ops[] = {
    "none",
    // prog_ops
    "return",
    "push",
    "pop",
    "rand",
    // normal ops
    "jump",
    "jump!",
    "jump_when",
    "jump_when!",
    "jump_if",
    "jump_if!",
    "add",
    "add!",
    "sub",
    "sub!",
    "mul",
    "mul!",
    "pow",
    "pow!",
    "lshift",
    "lshift!",
    "eq",
    "eq!",
    "neq",
    "neq!",
    "le",
    "le!",
    "nle",
    "nle!",
    "leq",
    "leq!",
    "nleq",
    "nleq!",
    "neg",
    "neg!",
    "get_key",
    "get_key!",
    "set_pix",
    "set_pix!",
};

// todo:
// split space - done
// perform preprocessing (i.e, record label positions, replace label occurances, replace macros)
// assemble (i.e, convert each word into its corresponding byte opcode) - done
// write to file
int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("usage: pat8asm FILE.p8\n");
        return 1;
    }

    printf("[p8asm log] num ops %i, prog ops %i\n", num_ops, num_prog_ops);

    char *buffer;
    long file_size;
    {
        FILE *file = fopen(argv[1], "r");
        if (file == NULL)
        {
            printf("[p8asm error] unable to open filename %s\n", argv[1]);
            return 1;
        }

        fseek(file, 0, SEEK_END);
        file_size = ftell(file);
        rewind(file);

        buffer = (char *)malloc(sizeof(char) * file_size);
        size_t result = fread(buffer, 1, file_size, file);
        fclose(file);
    }

    // replace newlines
    for (int i = 0; i < file_size; i++)
        if (buffer[i] == '\n')
            buffer[i] = ' ';
    buffer[file_size] = '\0';

    // split spaces
    char **words;
    int num_words = 0;
    {
        char *ptr = strtok(buffer, " ");
        while (ptr != NULL)
        {
            num_words++;
            words = realloc(words, num_words * sizeof(char *));
            words[num_words - 1] = malloc(strlen(ptr) * sizeof(char));
            strcpy(words[num_words - 1], ptr);

            ptr = strtok(NULL, " ");
        }
        free(buffer);
    }

    // assemble
    printf("[p8a log] num words %i\n", num_words);
    byte *program = malloc(num_words * sizeof(byte));
    for (int i = 0; i < num_words; i++)
    {
        program[i] = 0xff;

        char *word = words[i];
        int word_len = strlen(word);

        if (word_len == 0)
            break;

        if (word_len <= 2) // assuming only hex numbers will have 1 or 2 characters
        {
            if (isdigit(word[0]) || isalpha(word[0]))
            {
                if (word_len == 1)
                {
                    program[i] = (byte)strtol(word, NULL, 16);
                }
                else
                {
                    if (isdigit(word[1]) || isalpha(word[1]))
                    {
                        program[i] = (byte)strtol(word, NULL, 16);
                    }
                }
            }
        }
        else
        {
            byte k = word[word_len - 1] == '!';

            if (k)
            {
                for (int o = num_prog_ops; o < num_ops; o++)
                {
                    int oo = num_prog_ops + 2 * (o - num_prog_ops) + 1;
                    if (strcmp(word, ops[oo]) == 0)
                    {
                        program[i] = oo;
                        break;
                    }
                }
            }
            else
            {
                for (int o = 0; o < num_ops; o++)
                {
                    if (o < num_prog_ops)
                    {
                        if (strcmp(word, ops[o]) == 0)
                        {
                            program[i] = o;
                            break;
                        }
                    }
                    else
                    {
                        int oo = num_prog_ops + 2 * (o - num_prog_ops);
                        if (strcmp(word, ops[oo]) == 0)
                        {
                            program[i] = oo;
                            break;
                        }
                    }
                }
            }
        }
    }

    printf("[p8asm log] string, opcode pairs:\n");
    for (int i = 0; i < num_words; i++)
        printf("'%s', 0x%x\n", words[i], program[i]);

    // writing
    {
        FILE *file = fopen("output.p8a", "wb");
        if (file == NULL)
        {
            printf("[p8asm] error writing file!\n");
            goto cleanup;
        }

        fwrite(program, sizeof(byte), num_words, file);
        fclose(file);
    }

cleanup:
    for (int i = 0; i < num_words; i++)
        free(words[i]);
    free(words);
    free(program);

    return 0;
}
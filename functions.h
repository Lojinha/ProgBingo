#ifndef _FUNCTIONS_H_
#define _FUNCTIONS_H_

#include <malloc.h>

/* 1. STRING LENGTH */

int string_length (char * string)
{
    int i;

    for (i = 0; string[i] != '\0'; i++)
        /* Do nothing. */;

    return i;
}

/* 2. CHARACTER COUNT */

int character_count (char * string, char character)
{
    int i, counter = 0;

    for (i = 0; string[i] != '\0'; i++)
    {
        if (string[i] == character)
            counter++;
    }

    return counter;
}

/* 3. SPLIT STRING */

char ** split_string (char * input)
{
    int
    i, j,
    n_strings = character_count (input, ';') + 1;

    char ** output;

    output = (char **) malloc ( sizeof (char *) * n_strings );

    for (i = 0; i < n_strings; i++)
    {
        output[i] = (char *) malloc ( sizeof (char) * 3 );
    
        for (j = 0; j < 2; j++)
            output[i][j] = input[i * 3 + j];
        output[i][j] = '\0';
    }

    return output;
}

/* 4. CONCATENATE */

char * concatenate (char * string_1, char * string_2)
{
    int 
    i, j,
    destination_length = string_length (string_1) + 
        string_length (string_2) + 1,
    length_1 = string_length (string_1),
    length_2 = length_1 + string_length (string_2);

    char * output;
  
    output = (char *) malloc ( sizeof (char) * destination_length );

    if (length_1 > 0)
        for (i = 0; i < length_1; i++)
            output[i] = string_1[i];
    else
        i = 0;

    for (j = 0; j <= length_2; j++, i++)
        output[i] = string_2[j];

    return output;
}

/* 5. CREATE BOARD */

char * create_board (char * num)
{
    char ** numbers = split_string (num);
    char * board;
 
    char aux[25];

    board = concatenate ("", 
        "+----------------------------------+\n"
        "|      |      |      |      |      |\n|  ");
    sprintf (aux, "%s%s%s%s%s%s%s%s%s", 
        numbers[0], "  |  ",
        numbers[1], "  |  ",
        numbers[2], "  |  ",
        numbers[3], "  |  ",
        numbers[4]);
    board = concatenate (board, aux);
    /* ATE AQUI FUNCIONA (USANDO 'concatenate')*/
    board = strcat (board, 
        "  |\n|      |      |      |      |      |\n"
        "|----------------------------------|\n"
        "|      |      |      |      |      |\n|  ");
    /* AQUI JÁ NÃO FUNCIONA MAIS (USANDO 'concatenate') */
    sprintf (aux, "%s%s%s%s%s%s%s%s%s", 
        numbers[5], "  |  ",
        numbers[6], "  |  ",
        numbers[7], "  |  ",
        numbers[8], "  |  ",
        numbers[9]);
    board = strcat (board, aux);
    board = strcat (board, 
        "  |\n|      |      |      |      |      |\n"
        "|----------------------------------|\n"
        "|      |      |      |      |      |\n|  ");
    sprintf (aux, "%s%s%s%s%s%s%s%s", 
        numbers[10], "  |  ",
        numbers[11], "  |  ", 
        "XX  |  ",
        numbers[12], "  |  ",
        numbers[13]);
    board = strcat (board, aux);
    board = strcat (board, 
        "  |\n|      |      |      |      |      |\n"
        "|----------------------------------|\n"
        "|      |      |      |      |      |\n|  ");
    sprintf (aux, "%s%s%s%s%s%s%s%s%s", 
        numbers[14], "  |  ",
        numbers[15], "  |  ",
        numbers[16], "  |  ",
        numbers[17], "  |  ",
        numbers[18]);
    board = strcat (board, aux);
    board = strcat (board, 
        "  |\n|      |      |      |      |      |\n"
        "|----------------------------------|\n"
        "|      |      |      |      |      |\n|  ");
    /* ATE AQUI FUNCIONA (USANDO 'strcat') */
    printf("%s", board);
    sprintf (aux, "%s%s%s%s%s%s%s%s%s", 
        numbers[19], "  |  ",
        numbers[20], "  |  ",
        numbers[21], "  |  ",
        numbers[22], "  |  ",
        numbers[23]);
    /* AQUI JA NAO FUNCIONA MAIS (USANDO 'strcat') */
    /*printf("%s", board);*/
    board = strcat (board, aux);
    board = strcat (board, 
        "  |\n|      |      |      |      |      |\n"
        "+----------------------------------+");

    return board;
}

#endif

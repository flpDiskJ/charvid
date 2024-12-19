#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>

#define INPUT_MAX 500

bool strComp(char *input, char *comp) // compares strings
{
    bool same = true;
    for (int x = 0; x < strlen(comp); x++)
    {
        if (input[x] != comp[x])
        {
            same = false;
        }
    }
    return same;
}

void help()
{
    printf(" exit\n");
    printf("");
}

int main()
{
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("SDL Audio could not initialize! SDL Error: %s\n", SDL_GetError());
		return 1;
	}

    bool run = true;
    char input[INPUT_MAX];
    char command[INPUT_MAX];
    char arg[INPUT_MAX];
    char *token;
    while (run)
    {
        printf("command: ");
        fgets(input, INPUT_MAX, stdin);
        command[0] = '\0';
        arg[0] = '\0';
        token = strtok(input, " \n\t\r");
        strcpy(command, token);
        token = strtok(NULL, " \n\t\r");
        if (token != NULL)
        {
            strcpy(arg, token);
        }

        if (strComp(command, "exit\0"))
        {
            run = false;
        } else if (strComp(command, "help\0"))
        {
            help();
        }

        if (token != NULL)
        {
            free(token);
        }
    }

    SDL_Quit();
    return 0;
}

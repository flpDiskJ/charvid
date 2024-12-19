#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <SDL.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define INPUT_MAX 500

struct File_Header{
    uint16_t width;
    uint16_t height;
    uint8_t audio_khz;
    uint8_t chunk_size;
    uint8_t chunks_per_second;
};

int width = 0;
int height = 0;
unsigned char *video_data;
unsigned char *audio_data;

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
    printf(" convert (convert png image sequence and wav audio to character video)\n");
    printf(" play (play video currently in RAM)\n");
    printf(" load (arg: filename  load existing charvid file)\n");
    printf(" save (arg: filename  save charvid video currently in RAM)\n");
}

void convert()
{
    char filename[INPUT_MAX];
    unsigned long int count = 0;
    unsigned char *img;
    int img_w, img_h, channels;
    while(true)
    {
        memset(filename, 0, strlen(filename));
        sprintf(filename, "%04d.png", count);
        img = stbi_load(filename, &img_w, &img_h, &channels, 1);
        if (img == NULL)
        {
            printf("Couldn't load image %s!", filename);
            break;
        }
        free(img);
    }
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
        while(1)
        {
            printf("command: ");
            fgets(input, INPUT_MAX, stdin);
            if (input[0] == '\r' || input[0] == '\n')
            {
                break;
            }
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
            } else if (strComp(command, "convert\0"))
            {
                convert();
            }

            if (token != NULL)
            {
                free(token);
            }
            break;
        }
    }

    SDL_Quit();
    return 0;
}

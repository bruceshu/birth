

#include <stdio.h>
#include "./include/SDL2/SDL.h"


#define SCREEN_W    640          
#define SCREEN_H    360          
#define PIXEL_W     352          
#define PIXEL_H     288         
#define BPP         12           
#define BUF_LEN     ((PIXEL_W) * (PIXEL_H) * (BPP) / 8)   

const int bpp = BPP;
int screen_w = SCREEN_W;
int screen_h = SCREEN_H;
const int pixel_w = PIXEL_W;
const int pixel_h = PIXEL_H;

unsigned char buffer[BUF_LEN+1];


int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO)) {
        printf("Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *screen;
    screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        screen_w,screen_h,SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
    if(!screen) {
        printf("SDL: could not create window - exiting:%s\n",SDL_GetError());
        return -1;
    }

    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(screen, -1, 0);

    Uint32 pixformat=0;

    pixformat= SDL_PIXELFORMAT_YV12;        //YUV4:2:0

    SDL_Texture* sdlTexture = SDL_CreateTexture(sdlRenderer,pixformat, SDL_TEXTUREACCESS_STREAMING,pixel_w,pixel_h);

    FILE *fp=NULL;
    fp=fopen("akiyo_cif.yuv","rb+");
    if(fp==NULL){
        printf("cannot open this file\n");
        return -1;
    }

    SDL_Rect sdlRect;
    int i = 5;
    while(i >= 0){
            if (fread(buffer, 1, pixel_w*pixel_h*bpp/8, fp) != pixel_w*pixel_h*bpp/8){
                fseek(fp, 0, SEEK_SET);
                i--;
                continue;
            }

            SDL_UpdateTexture(sdlTexture, NULL, buffer, pixel_w);

            sdlRect.x = 0;
            sdlRect.y = 0;
            sdlRect.w = screen_w;
            sdlRect.h = screen_h;

            SDL_RenderClear( sdlRenderer );
            SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, &sdlRect);
            
            SDL_RenderPresent( sdlRenderer );
            
            SDL_Delay(40);
    }
    
    SDL_Quit();
    return 0;
}



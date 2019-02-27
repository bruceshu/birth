/* CopyRight (c) bruceshu 2018/01/30 3350207067@qq.com

description:调用sdl库，实现pcm文件播放
*/

#include "./../include/sdl/SDL.h"

#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
 
//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static  Uint8  *audio_chunk; 
static  Uint32  audio_len; 
static  Uint8  *audio_pos; 
 
/* The audio function callback takes the following parameters: 
 * stream: A pointer to the audio buffer to be filled 
 * len: The length (in bytes) of the audio buffer 
 * 回调函数
*/ 
void  fill_audio(void *udata,Uint8 *stream,int len){ 
    if(audio_len==0)
        return;
    
    len = (len > audio_len ? audio_len : len);
 
    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len; 
    audio_len -= len; 
} 
//-----------------
 
 
int main(int argc, char* argv[])
{
    int data_count = 0;

    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
        printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
        return -1;
    }

    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = 8000; 
    wanted_spec.format = AUDIO_S16MSB; 
    wanted_spec.channels = 1; 
    wanted_spec.silence = 0; 
    wanted_spec.samples = 0; 
    wanted_spec.callback = fill_audio;

    if (SDL_OpenAudio(&wanted_spec, NULL)<0){ 
        printf("can't open audio.\n"); 
        return -1; 
    }

    FILE *fp=fopen("eric_stb.pcm","rb+");
    if(fp==NULL){
        printf("cannot open this file\n");
        return -1;
    }

    int pcm_buffer_size=4096;
    char *pcm_buffer=(char *)malloc(pcm_buffer_size);
    if (!pcm_buffer) {
        printf("malloc size failed\n");
        return -1;
    }
    
    SDL_PauseAudio(0);
 
    while(1){
        if (!fread(pcm_buffer, 1, pcm_buffer_size, fp)){
            printf("fread error or read EOF\n");
	    break;
        }
        
        printf("Now Playing %10d Bytes data.\n",data_count);
        data_count += pcm_buffer_size;

        audio_chunk = (Uint8 *) pcm_buffer; 
        audio_len = pcm_buffer_size;
        audio_pos = audio_chunk;
        
        while(audio_len > 0)//Wait until finish
            SDL_Delay(1); 
    }
    
    free(pcm_buffer);
    SDL_Quit();
 
    return 0;
}



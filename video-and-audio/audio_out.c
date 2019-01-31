/* CopyRight (c) bruceshu 2018/01/30 3350207067@qq.com

description:调用sdl库，实现pcm文件播放
*/

#include "sdl/SDL.h"
 
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio
 
 
//Output PCM
#define OUTPUT_PCM 1

//Use SDL
#define USE_SDL 1
 
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
    
    len=(len>audio_len?audio_len:len);
 
    SDL_MixAudio(stream,audio_pos,len,SDL_MIX_MAXVOLUME);
    audio_pos += len; 
    audio_len -= len; 
} 
//-----------------
 
 
int main(int argc, char* argv[])
{
    char url[]="eric_stb.pcm";

    
#if OUTPUT_PCM
    FILE *pFile=NULL;
    pFile=fopen("output.pcm", "wb");
#endif
 
    
    //Out Audio Param
    uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO;
    //AAC:1024  MP3:1152
    int out_nb_samples=pCodecCtx->frame_size;
    AVSampleFormat out_sample_fmt=AV_SAMPLE_FMT_S16;
    int out_sample_rate=44100;
    int out_channels=av_get_channel_layout_nb_channels(out_channel_layout);
    //Out Buffer Size
    int out_buffer_size=av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1);
 
    uint8_t *out_buffer=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);
    
#if USE_SDL
    //Init
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
        printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
        return -1;
    }
    
    //SDL_AudioSpec
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = 44100; 
    wanted_spec.format = AUDIO_S16SYS; 
    wanted_spec.channels = 1; 
    wanted_spec.silence = 0; 
    wanted_spec.samples = out_nb_samples; 
    wanted_spec.callback = fill_audio; 
    wanted_spec.userdata = NULL; 
 
    if (SDL_OpenAudio(&wanted_spec, NULL)<0){ 
        printf("can't open audio.\n"); 
        return -1; 
    } 
#endif

    uint32_t ret,len = 0;
    int got_picture;
    int index = 0;
    
    //Play
    SDL_PauseAudio(0);
 
#if USE_SDL
    //Set audio buffer (PCM data)
    audio_chunk = (Uint8 *) out_buffer; 

    //Audio buffer length
    audio_len =out_buffer_size;

    audio_pos = audio_chunk;

    while(audio_len>0)//Wait until finish
        SDL_Delay(1); 
 
    SDL_CloseAudio();//Close SDL
    SDL_Quit();
#endif
 
#if OUTPUT_PCM
    fclose(pFile);
#endif
 
    return 0;
}

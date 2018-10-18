/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-10-18
 * Description:
 
*********************************/





void av_parser_close(AVCodecParserContext *s)
{
    if (s) {
        if (s->parser->parser_close)
            s->parser->parser_close(s);
        av_freep(&s->priv_data);
        av_free(s);
    }
}


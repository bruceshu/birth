/*********************************
 * Copyright (c) 2018 Bruceshu 3350207067@qq.com
 * Auther:Bruceshu
 * Date:  2018-09-28
 * Description:
 
*********************************/




#ifndef INTERNAL_H
#define INTERNAL_H


#define MAX_URL_SIZE 4096

/** size of probe buffer, for guessing file type from file contents */
#define PROBE_BUF_MIN 2048
#define PROBE_BUF_MAX (1 << 20)

#define MAX_PROBE_PACKETS 2500

#define SPACE_CHARS " \t\r\n"

#ifdef __GNUC__
#define dynarray_add(tab, nb_ptr, elem)\
do {\
    __typeof__(tab) _tab = (tab);\
    __typeof__(elem) _elem = (elem);\
    (void)sizeof(**_tab == _elem); /* check that types are compatible */\
    av_dynarray_add(_tab, nb_ptr, _elem);\
} while(0)
#else
#define dynarray_add(tab, nb_ptr, elem)\
do {\
    av_dynarray_add((tab), nb_ptr, (elem));\
} while(0)
#endif




#endif

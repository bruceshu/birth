/*
copyright bruceshu

author:bruceshu
date:2018-07-05
description:

*/

#ifndef DICT_H
#define DICT_H

#define AV_DICT_MATCH_CASE      1
#define AV_DICT_IGNORE_SUFFIX   2
#define AV_DICT_DONT_STRDUP_KEY 4
#define AV_DICT_DONT_STRDUP_VAL 8
#define AV_DICT_DONT_OVERWRITE 16
#define AV_DICT_APPEND         32
#define AV_DICT_MULTIKEY       64

typedef struct AVDictionaryEntry {
    char *key;
    char *value;
}AVDictionaryEntry;

typedef struct AVDictionary {
    int count;
    AVDictionaryEntry *elems;
}AVDictionary;

AVDictionaryEntry *av_dict_get(const AVDictionary *m, const char *key, const AVDictionaryEntry *prev, int flags);
int av_dict_set(AVDictionary **pm, const char *key, const char *value, int flags);
int av_dict_set_int(AVDictionary **pm, const char *key, int64_t value, int flags);
int av_dict_copy(AVDictionary **dst, const AVDictionary *src, int flags);
int av_dict_count(const AVDictionary *m);

void av_dict_free(AVDictionary **ppstAVDict);


#endif

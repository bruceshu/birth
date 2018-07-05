/*
copyright bruceshu

author:bruceshu
date:2018-07-05
description:

*/

#ifndef DICT_H
#define DICT_H


typedef struct AVDictionaryEntry {
    char *key;
    char *value;
}AVDictionaryEntry;

typedef struct AVDictionary {
    int count;
    AVDictionaryEntry *elems;
}AVDictionary;



#endif

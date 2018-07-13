


void av_dict_free(AVDictionary **ppstAVDict)
{
    AVDictionary *pstAVDict = *ppstAVDict;

    if (pstAVDict) {
        while (pstAVDict->count--) {
            av_freep(&pstAVDict->elems[pstAVDict->count].key);
            av_freep(&pstAVDict->elems[pstAVDict->count].value);
        }
        
        av_freep(&pstAVDict->elems);
    }
    
    av_freep(ppstAVDict);
}

AVDictionaryEntry *av_dict_get(const AVDictionary *m, const char *key, const AVDictionaryEntry *prev, int flags)
{
    unsigned int i, j;

    if (!m)
        return NULL;

    if (prev)
        i = prev - m->elems + 1;
    else
        i = 0;

    for (; i < m->count; i++) {
        const char *s = m->elems[i].key;
        if (flags & AV_DICT_MATCH_CASE)
            for (j = 0; s[j] == key[j] && key[j]; j++)
                ;
        else
            for (j = 0; av_toupper(s[j]) == av_toupper(key[j]) && key[j]; j++)
                ;
        if (key[j])
            continue;
        if (s[j] && !(flags & AV_DICT_IGNORE_SUFFIX))
            continue;
        return &m->elems[i];
    }
    return NULL;
}









int find_info_tag(char *arg, int arg_size, const char *tag, const char *info)
{
    const char *p;
    char *q;
    char tmp[128];

    p = info;
    if (*p == '?')
        p++;
    
    for(;;) {
        q = tmp;
        while (*p != '\0' && *p != '=' && *p != '&') {
            if ((q - tmp) < sizeof(tmp) - 1)
                *q++ = *p;
            p++;
        }
        *q = '\0';
        
        q = arg;
        if (*p == '=') {
            p++;
            while (*p != '&' && *p != '\0') {
                if ((q - arg) < arg_size - 1) {
                    if (*p == '+')
                        *q++ = ' ';
                    else
                        *q++ = *p;
                }
                p++;
            }
        }
        *q = '\0';
        
        if (!strcmp(tmp, tag))
            return 1;
        if (*p != '&')
            break;
        p++;
    }
    return 0;
}


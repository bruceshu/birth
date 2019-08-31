

#define PAD '='

static const char base64Encode[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g',
    'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '+', '/'
};

int Base64Encode(const char *in, unsigned int inlen, char *out, int *outlen)
{
    int outsize;
    int padTwoBytes;
    int i,j;
    char a1,a2,a3;
    int b1,b2,b3,b4;
    
    if (!in || !out)
        return -1;
    
    outsize = (inlen + 3 - 1)/3 * 4;
    if (outsize > *outlen)
        return -1;

    i = 0;
    j = 0;
    while(inlen > 2)
    {
        a1 = in[i++];
        a2 = in[i++];
        a3 = in[i++];

        b1 = (a1 >> 2) & 0x3f;
        b2 = ((a1 & 0x03) << 4) | ((a2 >> 4) & 0x0f);
        b3 = (a2 & 0x0f) << 2 | ((a3 >> 6) & 0x03);
        b4 = a3 & 0x3f;

        out[j++] = base64Encode[b1];
        out[j++] = base64Encode[b2];
        out[j++] = base64Encode[b3];
        out[j++] = base64Encode[b4];

        inlen -=3;
    }

    if (inlen)
    {
        padTwoBytes = (inlen == 2);
        
        a1 = in[i++];
        a2 = padTwoBytes ? in[i++] : 0;
        
        b1 = (a1 >> 2) & 0x3f;
        b2 = ((a1 & 0x03) << 4) | ((a2 >> 4) & 0x0f);
        b3 = (a2 & 0x0f) << 2;

        out[j++] = base64Encode[b1];
        out[j++] = base64Encode[b2];
        out[j++] = padTwoBytes ? base64Encode[b3] : PAD;
        out[j++] = PAD;
    }   
        
    *outlen = outsize;

    return 0;
}


int Base64Decode(const char *in, int inlen, char *out, int outlen)
{
    
}

static const char base64Decode[] = { 62, BAD, BAD, BAD, 63,   /* + starts at 0x2B */
                              52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
                              BAD, BAD, BAD, BAD, BAD, BAD, BAD,
                              0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
                              10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
                              20, 21, 22, 23, 24, 25,
                              BAD, BAD, BAD, BAD, BAD, BAD,
                              26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
                              36, 37, 38, 39, 40, 41, 42, 43, 44, 45,
                              46, 47, 48, 49, 50, 51
                            };


int Base64Decode(const char* in, int inLen, char * out, int * outLen)
{
    int i = 0;
    int j = 0;

    while (inLen > 3) {
        char b1, b2, b3;
        char e1 = in[j++];
        char e2 = in[j++];
        char e3 = in[j++];
        char e4 = in[j++];

        int pad3 = 0;
        int pad4 = 0;

        if (e1 == 0)            /* end file 0's */
            break;
        if (e3 == PAD)
            pad3 = 1;
        if (e4 == PAD)
            pad4 = 1;

        e1 = base64Decode[e1 - 0x2B];
        e2 = base64Decode[e2 - 0x2B];
        e3 = (e3 == PAD) ? 0 : base64Decode[e3 - 0x2B];
        e4 = (e4 == PAD) ? 0 : base64Decode[e4 - 0x2B];

        b1 = (e1 << 2) | (e2 >> 4);
        b2 = ((e2 & 0xF) << 4) | (e3 >> 2);
        b3 = ((e3 & 0x3) << 6) | e4;

        out[i++] = b1;
        if (!pad3)
            out[i++] = b2;
        if (!pad4)
            out[i++] = b3;
        else
            break;
        
        inLen -= 4;
        if (in[j] == ' ' || in[j] == '/r' || in[j] == '/n') {
            char endLine = in[j++];
            inLen--;
            while (endLine == ' ') {   /* allow trailing whitespace */
                endLine = in[j++];
                inLen--;
            }
            if (endLine == '/r') {
                endLine = in[j++];
                inLen--;
            }
            if (endLine != '/n')
                return -1;
        }
    }
    *outLen = i;
    return 0;
}


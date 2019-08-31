

#define PAD '='

static const char encodeBase64[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G',
    'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g',
    'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z'
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '+', '/'
};

int encoderBase64(const char *in, unsigned int inlen, char *out, unsigned int *outlen)
{
    int outsize;
    int padTwoBytes;
    int i,j;
    char a1,a2,a3;
    char b1,b2,b3,b4;
    
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

        out[j++] = encodeBase64[b1];
        out[j++] = encodeBase64[b2];
        out[j++] = encodeBase64[b3];
        out[j++] = encodeBase64[b4];
    }


    if (inlen)
    {
        padTwoBytes = (inlen == 2);
        
        a1 = in[j++];
        a2 = padTwoBytes ? in[j++] : 0;
        
        b1 = (a1 >> 2) & 0x3f;
        b2 = ((a1 & 0x03) << 4) | ((a2 >> 4) & 0x0f);
        b3 = (a2 & 0x0f) << 2;

        out[i++] = encodeBase64[b1];
        out[i++] = encodeBase64[b2];
        out[i++] = padTwoBytes ? encodeBase64[b3] : PAD;
        out[i++] = PAD;
    }   
        
    *outlen = outsize;

    return 0;
}




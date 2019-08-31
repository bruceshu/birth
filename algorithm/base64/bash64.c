#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
enum {
    BAD         = 0xFF,  /* invalid encoding */
    PAD         = '=',
    PEM_LINE_SZ = 64
};


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


static const char base64Encode[] = { 
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
    'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '+', '/'
    };


int Base64Encode(const char* in, int inLen, char* out, int* outLen)
{
    int i = 0,j = 0,n = 0;   /* new line counter */
    int outSz = (inLen + 3 - 1) / 3 * 4;

    if (outSz > *outLen) return -1;
    
    while (inLen > 2) {
        char b1 = in[j++];
        char b2 = in[j++];
        char b3 = in[j++];

        /* encoded idx */
        char e1 = b1 >> 2;
        char e2 = ((b1 & 0x3) << 4) | (b2 >> 4);
        char e3 = ((b2 & 0xF) << 2) | (b3 >> 6);
        char e4 = b3 & 0x3F;

        /* store */
        out[i++] = base64Encode[e1];
        out[i++] = base64Encode[e2];
        out[i++] = base64Encode[e3];
        out[i++] = base64Encode[e4];

        inLen -= 3;

        if ((++n % (PEM_LINE_SZ / 4)) == 0 && inLen)
            out[i++] = '/n';
    }

    /* last integral */
    if (inLen) {
        int twoBytes = (inLen == 2);

        char b1 = in[j++];
        char b2 = (twoBytes) ? in[j++] : 0;

        char e1 = b1 >> 2;
        char e2 = ((b1 & 0x3) << 4) | (b2 >> 4);
        char e3 =  (b2 & 0xF) << 2;

        out[i++] = base64Encode[e1];
        out[i++] = base64Encode[e2];
        out[i++] = (twoBytes) ? base64Encode[e3] : PAD;
        out[i++] = PAD;
    } 

    //out[i++] = '/n';
    if (i != outSz)
        return -1;
    *outLen = outSz;

    return 0; 
}


int main()
{
    char * sstr = "123Hello987World+=%456";
    char encrystr[256];
    int len = 256;
    memset(encrystr,'/0',256);
    Base64Encode(sstr,strlen(sstr),encrystr, &len);
    printf("estr : %s/n",encrystr);
    char decrystr[256];
    memset(decrystr,'/0',256);
    Base64Decode(encrystr,len,decrystr,&len);
    printf("dstr : %s/n",decrystr);
    return 0;
}

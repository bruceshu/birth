/*CopyRight (c) shuhuan 
* 描述：本文件是base64编码与解码算法原理
* 作者：舒焕
* 时间：2019/8/31
* 版本：1.0
*/

#define PAD '='
#define BAD 0xff

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

/*解码的时候，编码数组中的字符A~Z(65~90),a~z(97~122),0~9(48~57),+(43),/(47)为解码数组中的索引，
索引值为对应的ASCII值，也就是括号中的值。'+'ASCII值最小，所以在解码数组中，索引是第一个，
对应解码数组中的值是编码数组中的位置第62个。所以，解码数组定义如下：
index：A~Z，a~z,0~9,+,/字符的ASCII值，由于数组从0开始，所以全体字符ASCII值-43（0x2b）
value：A~Z，a~z,0~9,+,/字符在编码数组中的索引。
例如:A字符的index是 65-43，value是0。
H字符的index是72-43，value是7。
+字符的index是 43-43，value是62。
/字符的index是47-43，value是63。
+字符与/字符之间的符号不在编码数组中，所以这些符号对应的ASCII值作为index在解码数组中为BAD；
其他不在编码数组中的字符也类似。这些字符存在的区间是43（+）~z（122）。
*/
static const char base64Decode[] = {
    62, BAD, BAD, BAD, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
    BAD, BAD, BAD, BAD, BAD, BAD, BAD,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,
    BAD,BAD,BAD,BAD,BAD,BAD,
    26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51
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
    char a1,a2,a3,a4;
    char b1,b2,b3,b4;
    int i=0;
    int j=0;
    int pad = 0;

    if (!in || !out)
        return -1;

    if (inlen % 4)
        return -1;
    
    while(inlen > 0) 
    {
        a1 = in[i++];
        a2 = in[i++];
        a3 = in[i++];
        a4 = in[i++];

        if (a1 == 0)
        {
            return 0;
        }

        if (a3 == PAD)
        {
            pad = 2;
        }

        if (a4 == PAD)
        {
            pad = 1;
        }
        
        b1 = base64Decode[a1-0x2b];
        b2 = base64Decode[a2-0x2b];
        b3 = (a3 == PAD) ? 0 : base64Decode[a3-0x2b];    
        b4 = (a4 == PAD) ? 0 : base64Decode[a4-0x2b];

        out[j++] = (b1 << 2) | ((b2 & 0x30) >> 4);
        out[j++] = (pad == 2) ? 0 : ((b2 & 0x0f) << 4) | ((b3 & 0x3c) >> 2);
        out[j++] = (pad == 1) ? 0 : (b3 << 6) | (b4 & 0x3f);
        inlen -= 4;
    }
    
    return 0;
}

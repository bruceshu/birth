/*CopyRight (c) shuhuan 
* ���������ļ���base64����������㷨ԭ��
* ���ߣ����
* ʱ�䣺2019/8/31
* �汾��1.0
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

/*�����ʱ�򣬱��������е��ַ�A~Z(65~90),a~z(97~122),0~9(48~57),+(43),/(47)Ϊ���������е�������
����ֵΪ��Ӧ��ASCIIֵ��Ҳ���������е�ֵ��'+'ASCIIֵ��С�������ڽ��������У������ǵ�һ����
��Ӧ���������е�ֵ�Ǳ��������е�λ�õ�62�������ԣ��������鶨�����£�
index��A~Z��a~z,0~9,+,/�ַ���ASCIIֵ�����������0��ʼ������ȫ���ַ�ASCIIֵ-43��0x2b��
value��A~Z��a~z,0~9,+,/�ַ��ڱ��������е�������
����:A�ַ���index�� 65-43��value��0��
H�ַ���index��72-43��value��7��
+�ַ���index�� 43-43��value��62��
/�ַ���index��47-43��value��63��
+�ַ���/�ַ�֮��ķ��Ų��ڱ��������У�������Щ���Ŷ�Ӧ��ASCIIֵ��Ϊindex�ڽ���������ΪBAD��
�������ڱ��������е��ַ�Ҳ���ơ���Щ�ַ����ڵ�������43��+��~z��122����
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

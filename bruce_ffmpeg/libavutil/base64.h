

#ifndef BASE64_H
#define BASE64_H


#define AV_BASE64_SIZE(x)  (((x)+2) / 3 * 4 + 1)

#define AV_RB32(x)                          \
((((const uint8_t*)(x))[0] << 24) |        \
(((const uint8_t*)(x))[1] << 16) |        \
(((const uint8_t*)(x))[2] <<  8) |        \
((const uint8_t*)(x))[3])

#endif
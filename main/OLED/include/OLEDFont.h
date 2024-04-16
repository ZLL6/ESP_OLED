#ifndef __OLEDFONT_H__
#define __OLEDFONT_H__
#include "stdint.h"
#include "stdio.h"
#include "stddef.h"

#define WROD_WIDTH          16  /* width of word*/
#define WORD_HIGHT          2  /* height of word*/
typedef struct
{
	const uint16_t code;
	const uint8_t *FontHex;
}CodeIndex_typeDef;
// OLED 6*8字库
extern const unsigned char F6x8[][6];

extern const unsigned char BMP1[];
extern const unsigned char good[];
// OLED 8*16字库
extern const unsigned char F8X16[];

// OLED 中文字库
extern const unsigned char Hzk[][32];

void CodeTabInit(void);
const uint8_t* SearchCnCode(uint16_t code);
#endif /* __OLEDFONT_H__ */
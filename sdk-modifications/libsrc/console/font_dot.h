#ifndef __FONT_DOT_H__
#define __FONT_DOT_H__

//version 0.1
//modified ''', '(',')',';','?','[',']','^','_''`','{','}','~'

const unsigned char font_map[128][8] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x08, 0x08, 0x08, 0x08, 
        0x00, 0x00, 0x00, 0x78, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x00, 0x00, 0x00, 0x00, 
        0x08, 0x08, 0x08, 0x78, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 
        0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00, 0x00, 
        0x7c, 0x64, 0x44, 0x44, 0x64, 0x7c, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x08, 0x10, 0x70, 0x50, 0x50, 0x70, 0x00, 
        0x24, 0x24, 0x1c, 0x08, 0x3f, 0x08, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x16, 0x1a, 0x12, 0x12, 0x16, 0x34, 0x20, 0x00, 0x3c, 0x24, 0x24, 0x66, 0x24, 0x3c, 0x08, 0x08, 
        0x08, 0x08, 0x08, 0x7f, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x1c, 0x3c, 0x3c, 0x1c, 0x0c, 0x04, 0x00, 
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1c, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00, 0x00, 
        0x54, 0x54, 0x34, 0x14, 0x14, 0x14, 0x14, 0x14, 0x08, 0x08, 0x08, 0x7f, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x7f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x78, 0x08, 0x08, 0x08, 0x08, 
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 
        0x00, 0x00, 0x08, 0x7c, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x7c, 0x20, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00, 
        0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x14, 0x7e, 0x28, 0x7e, 0x28, 0x28, 0x00, 
        0x1c, 0x2c, 0x28, 0x18, 0x0c, 0x2c, 0x3c, 0x08, 0x64, 0x68, 0x68, 0x7c, 0x1c, 0x1c, 0x2c, 0x00, 
        0x30, 0x30, 0x3c, 0x28, 0x58, 0x50, 0x3c, 0x00, 0x30, 0x30, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, //'''
        0x04, 0x08, 0x10, 0x10, 0x10, 0x10, 0x08, 0x04, 0x40, 0x20, 0x10, 0x10, 0x10, 0x10, 0x20, 0x40, 
        0x10, 0x54, 0x38, 0x38, 0x54, 0x10, 0x00, 0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x40, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x20, 0x40, 
        0x18, 0x24, 0x24, 0x24, 0x24, 0x24, 0x18, 0x00, 0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 
        0x18, 0x24, 0x04, 0x08, 0x10, 0x20, 0x3c, 0x00, 0x18, 0x24, 0x04, 0x18, 0x04, 0x24, 0x18, 0x00, 
        0x08, 0x18, 0x28, 0x48, 0x7c, 0x08, 0x08, 0x00, 0x3c, 0x20, 0x38, 0x04, 0x04, 0x24, 0x18, 0x00, 
        0x38, 0x40, 0x40, 0x78, 0x44, 0x44, 0x38, 0x00, 0x3c, 0x08, 0x08, 0x10, 0x10, 0x10, 0x10, 0x00, 
        0x18, 0x24, 0x24, 0x18, 0x24, 0x24, 0x18, 0x00, 0x18, 0x24, 0x24, 0x1c, 0x04, 0x04, 0x18, 0x00, 
        0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x10, 0x10, 0x20, //';'
        0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x00, 0x7c, 0x00, 0x00, 
        0x20, 0x10, 0x08, 0x04, 0x08, 0x10, 0x20, 0x00, 0x10, 0x24, 0x24, 0x08, 0x10, 0x10, 0x00, 0x10, //'?'
        0x38, 0x4c, 0x54, 0x5c, 0x54, 0x44, 0x38, 0x00, 0x10, 0x10, 0x28, 0x28, 0x38, 0x28, 0x6c, 0x00, 
        0x78, 0x24, 0x38, 0x24, 0x24, 0x24, 0x78, 0x00, 0x3c, 0x44, 0x40, 0x40, 0x40, 0x44, 0x38, 0x00, 
        0x78, 0x24, 0x24, 0x24, 0x24, 0x24, 0x78, 0x00, 0x7c, 0x24, 0x20, 0x38, 0x20, 0x24, 0x7c, 0x00, 
        0x7c, 0x24, 0x28, 0x38, 0x28, 0x20, 0x70, 0x00, 0x38, 0x40, 0x40, 0x40, 0x5c, 0x48, 0x30, 0x00, 
        0x76, 0x24, 0x24, 0x3c, 0x24, 0x24, 0x76, 0x00, 0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 
        0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x60, 0x74, 0x28, 0x30, 0x30, 0x28, 0x28, 0x6c, 0x00, 
        0x70, 0x20, 0x20, 0x20, 0x20, 0x24, 0x7c, 0x00, 0x66, 0x3c, 0x3c, 0x3c, 0x34, 0x24, 0x66, 0x00, 
        0x6e, 0x24, 0x34, 0x34, 0x2c, 0x24, 0x74, 0x00, 0x38, 0x44, 0x44, 0x44, 0x44, 0x44, 0x38, 0x00, 
        0x78, 0x24, 0x24, 0x38, 0x20, 0x20, 0x70, 0x00, 0x38, 0x44, 0x44, 0x44, 0x74, 0x4c, 0x38, 0x0c, 
        0x78, 0x24, 0x38, 0x28, 0x24, 0x24, 0x76, 0x00, 0x1c, 0x24, 0x20, 0x18, 0x04, 0x24, 0x38, 0x00, 
        0x7c, 0x54, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x66, 0x24, 0x24, 0x24, 0x24, 0x24, 0x18, 0x00, 
        0x6c, 0x28, 0x28, 0x28, 0x28, 0x10, 0x10, 0x00, 0x7e, 0x52, 0x52, 0x2c, 0x2c, 0x24, 0x24, 0x00, 
        0x6c, 0x28, 0x28, 0x10, 0x28, 0x28, 0x6c, 0x00, 0x6c, 0x28, 0x28, 0x10, 0x10, 0x10, 0x38, 0x00, 
        0x7c, 0x48, 0x10, 0x10, 0x20, 0x24, 0x7c, 0x00, 0x00, 0x1c, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1c, 
        0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x08, 0x04, 0x00, 0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, //']'
        0x10, 0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 
        0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x48, 0x38, 0x48, 0x3c, 0x00, //'a'
        0x60, 0x20, 0x38, 0x24, 0x24, 0x24, 0x38, 0x00, 0x00, 0x00, 0x1c, 0x24, 0x20, 0x20, 0x1c, 0x00, 
        0x0c, 0x04, 0x1c, 0x24, 0x24, 0x24, 0x1e, 0x00, 0x00, 0x00, 0x18, 0x24, 0x3c, 0x20, 0x1c, 0x00, 
        0x0c, 0x10, 0x3c, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x3c, 0x28, 0x38, 0x20, 0x3c, 0x3c, 
        0x60, 0x20, 0x38, 0x24, 0x24, 0x24, 0x76, 0x00, 0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x38, 0x00, 
        0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x08, 0x38, 0x60, 0x20, 0x2c, 0x28, 0x30, 0x28, 0x6c, 0x00, 
        0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x78, 0x54, 0x54, 0x54, 0x54, 0x00, 
        0x00, 0x00, 0x78, 0x24, 0x24, 0x24, 0x76, 0x00, 0x00, 0x00, 0x38, 0x44, 0x44, 0x44, 0x38, 0x00, 
        0x00, 0x00, 0x78, 0x24, 0x24, 0x24, 0x38, 0x70, 0x00, 0x00, 0x1c, 0x24, 0x24, 0x24, 0x1c, 0x0e, 
        0x00, 0x00, 0x34, 0x18, 0x10, 0x10, 0x38, 0x00, 0x00, 0x00, 0x3c, 0x20, 0x18, 0x04, 0x3c, 0x00, 
        0x10, 0x10, 0x38, 0x10, 0x10, 0x10, 0x08, 0x00, 0x00, 0x00, 0x6c, 0x24, 0x24, 0x24, 0x1e, 0x00, 
        0x00, 0x00, 0x6c, 0x28, 0x28, 0x28, 0x10, 0x00, 0x00, 0x00, 0x6f, 0x2a, 0x2a, 0x36, 0x14, 0x00, 
        0x00, 0x00, 0x7c, 0x28, 0x10, 0x28, 0x7c, 0x00, 0x00, 0x00, 0x7c, 0x28, 0x10, 0x10, 0x20, 0x60, 
        0x00, 0x00, 0x3c, 0x08, 0x08, 0x10, 0x3c, 0x00, 0x00, 0x08, 0x10, 0x10, 0x20, 0x10, 0x10, 0x08, 
        0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x00, 0x20, 0x10, 0x10, 0x08, 0x10, 0x10, 0x20, 
        0x00, 0x00, 0x20, 0x54, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                              };

#endif //__FONT_DOT_H__

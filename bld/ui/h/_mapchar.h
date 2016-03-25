/*    enum                  linux           others          dbcs    charmap datamap */
pick( UI_RADIO_LEFT,        '(',            '(',            '(',    0xC6,   0x00,0x00,0x01,0x02,0x02,0x04,0x04,0x04, 0x04,0x02,0x02,0x01,0x00,0x00,0x00,0x00 )
pick( UI_RADIO_RIGHT,       ')',            ')',            ')',    0xEA,   0x00,0x00,0x00,0x80,0x80,0x40,0x40,0x40, 0x40,0x80,0x80,0x00,0x00,0x00,0x00,0x00 )
pick( UI_RADIO_FULL,        '*',            '*',            '*',    0xC7,   0x00,0xfe,0x01,0x00,0x7c,0xfe,0xfe,0xfe, 0xfe,0x7c,0x00,0x01,0xfe,0x00,0x00,0x00 )
pick( UI_RADIO_EMPTY,       ' ',            ' ',            ' ',    0xD0,   0x00,0xfe,0x01,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x01,0xfe,0x00,0x00,0x00 )
pick( UI_CHECKBOX_LEFT,     '[',            '[',            '[',    0xD1,   0x00,0x07,0x04,0x04,0x04,0x04,0x04,0x04, 0x04,0x04,0x04,0x04,0x07,0x00,0x00,0x00 )
pick( UI_CHECKBOX_RIGHT,    ']',            ']',            ']',    0xEB,   0x00,0xc0,0x40,0x40,0x40,0x40,0x40,0x40, 0x40,0x40,0x40,0x40,0xc0,0x00,0x00,0x00 )
pick( UI_CHECKBOX_FULL,     'X',            'X',            'X',    0xD2,   0x00,0xff,0x00,0xc6,0xee,0x7c,0x38,0x38, 0x7c,0xee,0xc6,0x00,0xff,0x00,0x00,0x00 )
pick( UI_CHECKBOX_EMPTY,    ' ',            ' ',            ' ',    0xD3,   0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0xff,0x00,0x00,0x00 )
pick( UI_BOX_TOP_LEFT,      UI_ULCORNER,    0xc9,           0x01,   0xD4,   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 )
pick( UI_BOX_TOP_RIGHT,     UI_URCORNER,    0xbb,           0x02,   0xCB,   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 )
pick( UI_BOX_BOTTOM_RIGHT,  UI_LRCORNER,    0xbc,           0x04,   0xCA,   0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0, 0xf0,0xf0,0xf0,0x00,0x00,0x00,0x00,0x00 )
pick( UI_BOX_BOTTOM_LEFT,   UI_LLCORNER,    0xc8,           0x03,   0xDE,   0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x00,0x00,0x00,0x00,0x00 )
pick( UI_BOX_TOP_LINE,      UI_DHLINE,      0xcd,           0x06,   0xCC,   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 )
pick( UI_BOX_RIGHT_LINE,    UI_DVLINE,      0xba,           0x05,   0xBA,   0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0, 0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0,0xf0 )
pick( UI_BOX_BOTTOM_LINE,   UI_DHLINE,      0xcd,           0x06,   0xCE,   0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00 )
pick( UI_BOX_LEFT_LINE,     UI_DVLINE,      0xba,           0x05,   0xCF,   0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07, 0x07,0x07,0x07,0x07,0x07,0x07,0x07,0x07 )
pick( UI_SHADOW_BOTTOM,     UI_UBLOCK,      0xdf,           ' ',    0xDF,   0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 )
pick( UI_SHADOW_B_LEFT,     ' ',            ' ',            ' ',    0xDC,   0x3f,0x3f,0x3f,0x3f,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 )
pick( UI_SHADOW_RIGHT,      UI_DBLOCK,      0xdc,           ' ',    0xFD,   0x00,0x00,0x00,0xe0,0xe0,0xe0,0xe0,0xe0, 0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0,0xe0 )
pick( UI_SHADOW_B_RIGHT,    UI_UBLOCK,      0xdf,           ' ',    0xF5,   0xe0,0xe0,0xe0,0xe0,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 )
#ifndef MAPCHARS
pick( UI_SBOX_TOP_LEFT,     UI_ULCORNER,    0xda,           0x01,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_TOP_RIGHT,    UI_URCORNER,    0xbf,           0x02,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_BOTTOM_RIGHT, UI_LRCORNER,    0xd9,           0x04,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_BOTTOM_LEFT,  UI_LLCORNER,    0xc0,           0x03,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_TOP_LINE,     UI_HLINE,       0xc4,           0x06,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_RIGHT_LINE,   UI_VLINE,       0xb3,           0x05,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_BOTTOM_LINE,  UI_HLINE,       0xc4,           0x06,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_LEFT_LINE,    UI_VLINE,       0xb3,           0x05,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_LEFT_TACK,    UI_LTEE,        0xc3,           0x19,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_RIGHT_TACK,   UI_RTEE,        0xb4,           0x17,   ,       , , , , , , , , , , , , , , , )
pick( UI_SBOX_HORIZ_LINE,   UI_HLINE,       0xc4,           0x06,   ,       , , , , , , , , , , , , , , , )
pick( UI_ARROW_DOWN,        UI_DARROW,      PC_arrowdown,   0x07,   ,       , , , , , , , , , , , , , , , )
pick( UI_POPUP_MARK,        UI_RPOINT,      PC_triangright, '>',    ,       , , , , , , , , , , , , , , , )
pick( UI_CHECK_MARK,        UI_ROOT,        PC_radical,     'X',    ,       , , , , , , , , , , , , , , , )
#endif

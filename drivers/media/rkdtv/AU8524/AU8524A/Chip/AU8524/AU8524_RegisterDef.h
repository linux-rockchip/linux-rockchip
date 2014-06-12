/**************************************************************************************

 *  CONFIDENTIAL AND PROPRIETARY SOFTWARE/DATA OF Auvitek International, Ltd.         * 


 *  Copyright (c) 2005-2018 Auvitek International, Ltd.  All Rights Reserved.         *

 *                                                                                    *

 *  Use of this Software/Data is subject to the terms and conditions of               *

 *  the applicable agreement between Auvitek International, Ltd, and receiving party. *

 *  In addition, this Software/Data is protected by copyright law and international   *

 *  treaties.                                                                         *

 *                                                                                    *

 *  The copyright notice(s) in this Software/Data does not indicate actual            *

 *  or intended publication of this Software/Data.                                    *

***************************************************************************************
**
**  Name: ConfigureCtr.h
**  Project:          
**  Description:
**       Chip 类型以AU开头：
**             AU8522 ---AU8522AA
**             AU8523 ---AU8523AA
**             AU8524 ---AU8524
**             AU8515 ---AU8515
**             AU8502 ---AU8502
**             AU8504 ---AU8504
**	
**       Tuner相关类型以TUN开头：
**			   TUNMT2130 --- MT-2130
**			   TUNMT2131 --- MT-2131
**			   TUNQJDTZ1W40x --- QJ-DTZ1W40x
**			   TUNNXPTDA18271C1 --- NXP-TDA18271C1 
**			   TUNNXPTDA18271C2 --- NXP-TDA18271C2 
**			   TUNPHILIPSTD1336 ---Philips-TD1336 
**			   TUNXGDVT6AW41F2 ---XG-DVT6AW41F2 
**			   TUNMXL50035 --- MXL-5003/5
**			   TUNXC5000  --- XC-5000 
**			   TUNTHDTT768xx --- Thomson-DTT768xx
**			   TUNNXPTD1636ALF --- NXP-TD1636ALF
**
**		 Frequency begin with "FRQ"：
**			   FRQVL --- VL section
**			   FRQVHU --- VH & U section
**
**       Mode 类型以MOD开头：
**             MODATSC --- ATSC
**             MODNTSC --- NTSC  
**			   MODSV ---  SVideo 
**			   MODCVBS --- Composit
**			   MODJ83B --- Common setting of J83B64 and J83B256
**			   MODJ83B64 --- J83B64 special setting execpt J83B Common Setting
**			   MODJ83B256 --- J83B256 special setting execpt J83B Common Setting
**			   MODPAL --- Common setting of PAL
**			   MODPALBG --- PAL_BG special setting execpt PAL
**			   MODPALDK --- PAL_DK  
**			   MODPALI --- PAL_I 
**			   MODSIF --- SIF
**			   MODFMRADIO --- FMRadio   
**
**
**
**  Date:	2009-03-31 10:23:30	
**
**  Version Number:	v2.28.11.41
**
**  Revision History:
**
**
*****************************************************************************/

#include "../Common/AUChipTypedef.h"

/*ATSC*/
UINT16 AU8524_TUNPHILIPSTD1336_MODATSC[]={0x262,0x20};

/*
UINT16 AU8524_TUNPHILIPSTD1336_MODATSC_FRQVL[]={};
*/
/*
UINT16 AU8524_TUNPHILIPSTD1336_MODATSC_FRQVHU[]={
												};
*/
UINT16 AU8524_TUNXGDVT6AW41F2_MODATSC[]={
										0x262,0x20,
										0x0a8,0x30,
										0x0aa,0xF0,
										};
/*
UINT16 AU8524_TUNXGDVT6AW41F2_MODATSC_FRQVL[]={
												};
*/
/*
UINT16 AU8524_TUNXGDVT6AW41F2_MODATSC_FRQVHU[]={
												};
*/	   
UINT16 AU8524_TUNQJDTZ1W40x_MODATSC[]={
										0x0ad,0x50,
										0x0a9,0x75,
										};

UINT16 AU8524_TUNQJDTZ1W40x_MODATSC_FRQVL[]={
										0x0aa,0xd0,
										0x0a8,0xff,
										};

UINT16 AU8524_TUNQJDTZ1W40x_MODATSC_FRQVHU[]={
										0x0aa,0x80,
										0x0a8,0xf0,
										};
	   
UINT16 AU8524_TUNNXPTDA18271C1_MODATSC[]={
										0x0ab,0x7b,				
										0x0ac,0x29,				
										0x0ad,0x58,				
										0x0a8,0x7b,				
										0x0a9,0x29,				
										0x0aa,0x58,
										0x0B8,0x30,
										0x0B9,0xE0,	
										0x0BA,0x00,	
										0x0BB,0x26,	
										0x0BC,0x80,	
										0x0BD,0x00,	
										0x0BE,0xE6,	
										0x0BF,0xBF,	
										0x0C0,0xFC,	
										0x0C1,0xA7,	
										0x0C2,0xBF,	
										0x0C3,0xF9,	
										0x0C4,0xB7,	
										0x0C5,0x0F,	
										0x0C6,0xFE,	
										0x0C7,0x15,	
										0x0C8,0xB0,	
										0x0C9,0x04,	
										0x0CA,0x58,	
										0x0CB,0xC0,	
										0x0CC,0x03,	
										0x0CD,0x05,	
										0x0CE,0x30,	
										0x0CF,0xFB,	
										0x0D0,0x73,	
										0x0D1,0xBF,	
										0x0D2,0xF6,	
										0x0D3,0x98,	
										0x0D4,0xEF,	
										0x0D5,0xFF,	
										0x0D6,0x80,	
										0x0D7,0x90,	
										0x250,0x0D,	
										0x251,0xE5,	
										0x252,0x20,	
										0x253,0x09,	
										0x254,0xDB,	
										0x255,0x5F,	
										0x256,0xF0,	
										0x257,0x6D,	
										0x258,0x5E,	
										0x259,0xE5,	
										0x25A,0x0A,	
										0x25B,0x7F,	
										0x25C,0x09,	
										0x25D,0xB0,	
										0x25E,0x82,	
										0x25F,0x4F,	
										0x260,0xEC,	
										0x261,0x06,
										};	

/*
UINT16 AU8524_TUNNXPTDA18271C1_MODATSC_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNNXPTDA18271C1_MODATSC_FRQVHU[]={										
										};	
*/

UINT16 AU8524_TUNNXPTDA18271C2_MODATSC[]={
										0x0ab,0x7b,				
										0x0ac,0x29,				
										0x0ad,0x58,				
										0x0a8,0x7b,				
										0x0a9,0x29,				
										0x0aa,0x58,
										0x0B8,0x30,
										0x0B9,0xE0,	
										0x0BA,0x00,	
										0x0BB,0x26,	
										0x0BC,0x80,	
										0x0BD,0x00,	
										0x0BE,0xE6,	
										0x0BF,0xBF,	
										0x0C0,0xFC,	
										0x0C1,0xA7,	
										0x0C2,0xBF,	
										0x0C3,0xF9,	
										0x0C4,0xB7,	
										0x0C5,0x0F,	
										0x0C6,0xFE,	
										0x0C7,0x15,	
										0x0C8,0xB0,	
										0x0C9,0x04,	
										0x0CA,0x58,	
										0x0CB,0xC0,	
										0x0CC,0x03,	
										0x0CD,0x05,	
										0x0CE,0x30,	
										0x0CF,0xFB,	
										0x0D0,0x73,	
										0x0D1,0xBF,	
										0x0D2,0xF6,	
										0x0D3,0x98,	
										0x0D4,0xEF,	
										0x0D5,0xFF,	
										0x0D6,0x80,	
										0x0D7,0x90,	
										0x250,0x0D,	
										0x251,0xE5,	
										0x252,0x20,	
										0x253,0x09,	
										0x254,0xDB,	
										0x255,0x5F,	
										0x256,0xF0,	
										0x257,0x6D,	
										0x258,0x5E,	
										0x259,0xE5,	
										0x25A,0x0A,	
										0x25B,0x7F,	
										0x25C,0x09,	
										0x25D,0xB0,	
										0x25E,0x82,	
										0x25F,0x4F,	
										0x260,0xEC,	
										0x261,0x06,
										};	

/*
UINT16 AU8524_TUNNXPTDA18271C2_MODATSC_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNNXPTDA18271C2_MODATSC_FRQVHU[]={										
										};	
*/
UINT16 AU8524_TUNTHDTT768xx_MODATSC[]={
										0x0B8,0x3F,
										0x0B9,0xC0,
										0x0BA,0x05,
										0x0BB,0x75,
										0x0BC,0xE0,
										0x0BD,0x06,
										0x0BE,0x3A,
										0x0BF,0x60,
										0x0C0,0xFE,
										0x0C1,0x8A,
										0x0C2,0xFF,
										0x0C3,0xF4,
										0x0C4,0x52,
										0x0C5,0xEF,
										0x0C6,0xF9,
										0x0C7,0x1F,
										0x0C8,0x90,
										0x0C9,0x0A,
										0x0CA,0x06,
										0x0CB,0x81,
										0x0CC,0x10,
										0x0CD,0xA1,
										0x0CE,0xD0,
										0x0CF,0xFE,
										0x0D0,0x28,
										0x0D1,0x0F,
										0x0D2,0xEA,
										0x0D3,0x93,
										0x0D4,0x7E,
										0x0D5,0xF1,
										0x0D6,0x08,
										0x0D7,0x20,
										0x0D8,0x14,
										0x250,0x11,
										0x251,0xC8,
										0x252,0x41,
										0x253,0x1D,
										0x254,0x1C,
										0x255,0x31,
										0x256,0xFD,
										0x257,0x74,
										0x258,0x6E,
										0x259,0xD9,
										0x25A,0xB7,
										0x25B,0xAD,
										0x25C,0xF0,
										0x25D,0x5C,
										0x25E,0xF1,
										0x25F,0x41,
										0x260,0x91,
										0x261,0x06,
										};   
/*
UINT16 AU8524_TUNTHDTT768xx_MODATSC_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNTHDTT768xx_MODATSC_FRQVHU[]={										
										};	
*/
UINT16 AU8524_TUNSANYOUBD00Ax_MODATSC[]={
										0x0B8,0x23,
										0x0B9,0x60,
										0x0BA,0x02,
										0x0BB,0x23,
										0x0BC,0xA0,
										0x0BD,0x00,
										0x0BE,0xE0,
										0x0BF,0x1F,
										0x0C0,0xFB,
										0x0C1,0x94,
										0x0C2,0xBF,
										0x0C3,0xF9,
										0x0C4,0xCC,
										0x0C5,0xAF,
										0x0C6,0x01,
										0x0C7,0x68,
										0x0C8,0x70,
										0x0C9,0x09,
										0x0CA,0x8B,
										0x0CB,0x20,
										0x0CC,0x04,
										0x0CD,0xCE,
										0x0CE,0x8F,
										0x0CF,0xF5,
										0x0D0,0x0E,
										0x0D1,0x6F,
										0x0D2,0xF1,
										0x0D3,0x7B,
										0x0D4,0x5F,
										0x0D5,0x02,
										0x0D6,0xE1,
										0x0D7,0x80,
										0x250,0x16,
										0x251,0x7B,
										0x252,0xA1,
										0x253,0x0F,
										0x254,0xF8,
										0x255,0xCF,
										0x256,0xEB,
										0x257,0xB2,
										0x258,0xDD,
										0x259,0xD4,
										0x25A,0xDD,
										0x25B,0x4D,
										0x25C,0xF7,
										0x25D,0xD1,
										0x25E,0x21,
										0x25F,0x47,
										0x260,0xB3,
										0x261,0x06,
										0x0ab,0xa0,
										};   

/*
UINT16 AU8524_TUNSANYOUBD00Ax_MODATSC_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNSANYOUBD00Ax_MODATSC_FRQVHU[]={										
										};	
*/
/*
UINT16 AU8524_TUNXC5000_MODATSC[]={
										};
*/
/*
UINT16 AU8524_TUNXC5000_MODATSC_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNXC5000_MODATSC_FRQVHU[]={										
										};	
*/
/*
UINT16 AU8524_TUNMT2130_MODATSC[]={
										};
*/
/*
UINT16 AU8524_TUNMT2130_MODATSC_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNMT2130_MODATSC_FRQVHU[]={										
										};	
*/
/*
UINT16 AU8524_TUNMT2131_MODATSC[]={
										};
*/
/*
UINT16 AU8524_TUNMT2131_MODATSC_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNMT2131_MODATSC_FRQVHU[]={										
										};	
*/
/*
UINT16 AU8524_TUNMXL50035_MODATSC[]={
										};
*/
/*
UINT16 AU8524_TUNMXL50035_MODATSC_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNMXL50035_MODATSC_FRQVHU[]={										
										};	
*/
UINT16 AU8524_TUNNXPTD1636ALF_MODATSC[]={0x0aa,0xa0,
										};
/*
UINT16 AU8524_TUNNXPTD1636ALF_MODATSC_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNNXPTD1636ALF_MODATSC_FRQVHU[]={										
										};
*/	

UINT16 AU8524_TUNQJDTZ0R401_MODATSC[]={
		0x0a8,0xf0,
		0x0a9,0x05,
		0x0aa,0xc0,
		0x0ab,0xf0,
		0x0ac,0x05,
		0x0ad,0x77,
		0x0af,0x23,
		0x21b,0xfc,
};

UINT16 AU8524_TUNQJDTZ0R401_MODATSC_FRQVL[]={
		0xa9, 0x5f,
};


UINT16 AU8524_TUNQJDTZ0R401_MODATSC_FRQVHU[]={
		0xa9, 0x3e,
};


/*J83*/
/*
UINT16 AU8524_TUNPHILIPSTD1336_MODJ83B[]={
										};
*/
/*

UINT16 AU8524_TUNPHILIPSTD1336_MODJ83B_FRQVL[]={
												};
*/
/*

UINT16 AU8524_TUNPHILIPSTD1336_MODJ83B_FRQVHU[]={
												};

*/


UINT16 AU8524_TUNXGDVT6AW41F2_MODJ83B[]={
										0xA8,0x30,
										0xAA,0xF0,
										};
/*
UINT16 AU8524_TUNXGDVT6AW41F2_MODJ83B_FRQVL[]={
												};
*/
/*

UINT16 AU8524_TUNXGDVT6AW41F2_MODJ83B_FRQVHU[]={
												};
*/

UINT16 AU8524_TUNQJDTZ1W40x_MODJ83B[]={
										0x0ad,0x50,
										0x0a9,0x75,
										};

UINT16 AU8524_TUNQJDTZ1W40x_MODJ83B_FRQVL[]={
										0x0aa,0xd0,
										0x0a8,0xff,
										};

UINT16 AU8524_TUNQJDTZ1W40x_MODJ83B_FRQVHU[]={
										0x0aa,0x80,
										0x0a8,0xf0,
										};

UINT16 AU8524_TUNNXPTDA18271C1_MODJ83B[]={   
										0x21a,0x00,
										0x0ab,0x7b,				
										0x0ac,0x29,				
										0x0ad,0x58,				
										0x0a8,0x7b,				
										0x0a9,0x29,				
										0x0aa,0x58,
										0x0B8,0x17,
										0x0B9,0x50,
										0x0BA,0x00,
										0x0BB,0x06,
										0x0BC,0x30,
										0x0BD,0xFF,
										0x0BE,0xDD,
										0x0BF,0xCF,
										0x0C0,0xFC,
										0x0C1,0xC2,
										0x0C2,0xAF,
										0x0C3,0xFC,
										0x0C4,0xE8,
										0x0C5,0xEF,
										0x0C6,0x00,
										0x0C7,0x36,
										0x0C8,0xE0,
										0x0C9,0x04,
										0x0CA,0x46,
										0x0CB,0xE0,
										0x0CC,0x01,
										0x0CD,0xE1,
										0x0CE,0x1F,
										0x0CF,0xFA,
										0x0D0,0x7B,
										0x0D1,0x9F,
										0x0D2,0xF8,
										0x0D3,0xCA,
										0x0D4,0x2F,
										0x0D5,0x03,
										0x0D6,0x9E,
										0x0D7,0xC0,
										0x250,0x0D,
										0x251,0xCA,
										0x252,0xC0,
										0x253,0x05,
										0x254,0xA6,
										0x255,0x2F,
										0x256,0xEE,
										0x257,0x64,
										0x258,0x4E,
										0x259,0xE7,
										0x25A,0x45,
										0x25B,0x6F,
										0x25C,0x0D,
										0x25D,0xEB,
										0x25E,0xF2,
										0x25F,0x51,
										0x260,0xF6,
										0x261,0x06,
										0x52d,0x0c,
										};
/*
UINT16 AU8524_TUNNXPTDA18271C1_MODJ83B_FRQVL[]={										
										};	
*/
/*

UINT16 AU8524_TUNNXPTDA18271C1_MODJ83B_FRQVHU[]={										
										};	
*/

UINT16 AU8524_TUNNXPTDA18271C2_MODJ83B[]={   
										0x21a,0x00,
										0x0ab,0x7b,				
										0x0ac,0x29,				
										0x0ad,0x58,				
										0x0a8,0x7b,				
										0x0a9,0x29,				
										0x0aa,0x58,
										0x0B8,0x17,
										0x0B9,0x50,
										0x0BA,0x00,
										0x0BB,0x06,
										0x0BC,0x30,
										0x0BD,0xFF,
										0x0BE,0xDD,
										0x0BF,0xCF,
										0x0C0,0xFC,
										0x0C1,0xC2,
										0x0C2,0xAF,
										0x0C3,0xFC,
										0x0C4,0xE8,
										0x0C5,0xEF,
										0x0C6,0x00,
										0x0C7,0x36,
										0x0C8,0xE0,
										0x0C9,0x04,
										0x0CA,0x46,
										0x0CB,0xE0,
										0x0CC,0x01,
										0x0CD,0xE1,
										0x0CE,0x1F,
										0x0CF,0xFA,
										0x0D0,0x7B,
										0x0D1,0x9F,
										0x0D2,0xF8,
										0x0D3,0xCA,
										0x0D4,0x2F,
										0x0D5,0x03,
										0x0D6,0x9E,
										0x0D7,0xC0,
										0x250,0x0D,
										0x251,0xCA,
										0x252,0xC0,
										0x253,0x05,
										0x254,0xA6,
										0x255,0x2F,
										0x256,0xEE,
										0x257,0x64,
										0x258,0x4E,
										0x259,0xE7,
										0x25A,0x45,
										0x25B,0x6F,
										0x25C,0x0D,
										0x25D,0xEB,
										0x25E,0xF2,
										0x25F,0x51,
										0x260,0xF6,
										0x261,0x06,
										0x52d,0x0c,
										};
/*
UINT16 AU8524_TUNNXPTDA18271C2_MODJ83B_FRQVL[]={										
										};	
*/
/*

UINT16 AU8524_TUNNXPTDA18271C2_MODJ83B_FRQVHU[]={										
										};	
*/

/*

UINT16 AU8524_TUNTHDTT768xx_MODJ83B[]={

										};   
*/
/*

UINT16 AU8524_TUNTHDTT768xx_MODJ83B_FRQVL[]={										
										};	
*/
/*

UINT16 AU8524_TUNTHDTT768xx_MODJ83B_FRQVHU[]={										
										};	
*/


UINT16 AU8524_TUNSANYOUBD00Ax_MODJ83B[]={
										0x0ab,0xa0,
										};   

/*

UINT16 AU8524_TUNSANYOUBD00Ax_MODJ83B_FRQVL[]={										
										};	
*/
/*

UINT16 AU8524_TUNSANYOUBD00Ax_MODJ83B_FRQVHU[]={										
										};	
*/


UINT16 AU8524_TUNXC5000_MODJ83B[]={		0x21a, 0x00,
										};

/*

UINT16 AU8524_TUNXC5000_MODJ83B_FRQVL[]={										
										};	
*/
/*

UINT16 AU8524_TUNXC5000_MODJ83B_FRQVHU[]={										
										};	
*/
UINT16 AU8524_TUNMXL5007_MODJ83B[]={
										0x21a,0x00,
};

/*

UINT16 AU8524_TUNMXL5007_MODJ83B_FRQVL[]={										
										};	
*/
/*

UINT16 AU8524_TUNMXL5007_MODJ83B_FRQVHU[]={										
										};	
*/
/*

UINT16 AU8524_TUNMT2130_MODJ83B[]={
										};
*/
/*

UINT16 AU8524_TUNMT2130_MODJ83B_FRQVL[]={										
										};	
*/
/*

UINT16 AU8524_TUNMT2130_MODJ83B_FRQVHU[]={										
										};	
*/
/*

UINT16 AU8524_TUNMT2131_MODJ83B[]={
										};
*/
/*

UINT16 AU8524_TUNMT2131_MODJ83B_FRQVL[]={										
										};	
*/
/*

UINT16 AU8524_TUNMT2131_MODJ83B_FRQVHU[]={										
										};	
*/


UINT16 AU8524_TUNMXL50035_MODJ83B[]={
	0x21a,0x00,
};

/*

UINT16 AU8524_TUNMXL50035_MODJ83B_FRQVL[]={										
										};	
*/
/*

UINT16 AU8524_TUNMXL50035_MODJ83B_FRQVHU[]={										
										};	
*/


UINT16 AU8524_TUNNXPTD1636ALF_MODJ83B[]={0x0aa,0xc0,
										 0x0ad,0x65,
										 0x0af,0x33,
										};
/*
UINT16 AU8524_TUNNXPTD1636ALF_MODJ83B_FRQVL[]={										
										};	
*/
/*
UINT16 AU8524_TUNNXPTD1636ALF_MODJ83B_FRQVHU[]={										
										};
*/								

UINT16 AU8524_TUNQJDTZ0R401_MODJ83B[]={
		0x0a8,0xf0,
		0x0a9,0x05,
		0x0aa,0xc0,
		0x0ab,0xf0,
		0x0ac,0x05,
		0x0ad,0x77,
		0x0af,0x23,
		0x21b,0xfc,
};


UINT16 AU8524_TUNQJDTZ0R401_MODJ83B_FRQVL[]={
		0xa9, 0x5f,
};


UINT16 AU8524_TUNQJDTZ0R401_MODJ83B_FRQVHU[]={
		0xa9, 0x3e,
};


/*define the Timer()
UINT16 TimerCallbackRoutine(){};
*/

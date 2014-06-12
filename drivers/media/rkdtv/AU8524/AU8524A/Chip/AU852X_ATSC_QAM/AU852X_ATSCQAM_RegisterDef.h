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

**************************************************************************************
**
**  Name: AU852X_ATSCQAM_RegisterDef.h
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
/*ATSC*/
UINT16 AU852X_MODATSC[]={
										0x0a4,0x00,
										0x081,0xC4,
										0x0a3,0x0C,
										0x0a5,0x40,	//0x0a5,0x40,TS Serial Master Module
										0x0a7,0x40,
										0x0a6,0x67,
										0x262,0x20,
										0x21c,0x30,
										0x0d8,0x1a,
										0x227,0xa0,
										0x121,0xff,
										0x0a8,0xf0,
										0x0a9,0x05,
										0x0aa,0x77,
										0x0ab,0xf0,
										0x0ac,0x05,
										0x0ad,0x77,
										0x0ae,0x41,
										0x0af,0x66,
										0x21b,0xcc,
										0x21d,0x80,
										0x231,0x13,
										0x21f,0x06,
										};

/*J83*/
UINT16 AU852X_MODJ83B[]={
										0x0A4,0x00,
										0x0A3,0x09,
										0x081,0xC4,
										0x0a5,0x40,
										0x0b5,0x00,
										0x0b6,0xbd,
										0x0b7,0xa1,
										0x0aa,0x77,
										0x0ad,0x77,
										0x0a6,0x67,   
										0x262,0x20,
										0x21c,0x30,
										0x0B8,0x3E,
										0x0B9,0xF0,
										0x0BA,0x01,
										0x0BB,0x18,
										0x0BC,0x50,
										0x0BD,0x00,
										0x0BE,0xEA,
										0x0BF,0xEF,
										0x0C0,0xFC,
										0x0C1,0xBD,
										0x0C2,0x1F,
										0x0C3,0xFC,
										0x0C4,0xDD,
										0x0C5,0xAF,
										0x0C6,0x00,
										0x0C7,0x38,
										0x0C8,0x30,
										0x0C9,0x05,
										0x0CA,0x4A,
										0x0CB,0xD0,
										0x0CC,0x01,
										0x0CD,0xD9,
										0x0CE,0x6F,
										0x0CF,0xF9,
										0x0D0,0x70,
										0x0D1,0xDF,
										0x0D2,0xF7,
										0x0D3,0xC2,
										0x0D4,0xDF,
										0x0D5,0x02,
										0x0D6,0x9A,
										0x0D7,0xD0,
										0x250,0x0D,
										0x251,0xCD,
										0x252,0xE0,
										0x253,0x05,
										0x254,0xA7,
										0x255,0xFF,
										0x256,0xED,
										0x257,0x5B,
										0x258,0xAE,
										0x259,0xE6,
										0x25A,0x3D,
										0x25B,0x0F,
										0x25C,0x0D,
										0x25D,0xEA,
										0x25E,0xF2,
										0x25F,0x51,
										0x260,0xF5,
										0x261,0x06,
										0x21A,0x01,
										0x546,0x40,
										0x121,0x04,
										0x122,0x04,
										0x52e,0x10,
										0x123,0x24,  
										0x0A7,0x40,
										};

UINT16 AU852X_MODJ83B64[]={
										0x210,0xC7,
										0x211,0xAA,
										0x212,0xAB,
										0x213,0x02, 
										0x502,0x00,
										0x526,0x00,
										};
					

UINT16 AU852X_MODJ83B256[]={
										0x210,0x26,    
										0x211,0xF6,    
										0x212,0x84,   
										0x213,0x02,   
										0x502,0x01,
										0x526,0x01,
										};

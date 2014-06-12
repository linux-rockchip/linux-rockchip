/****************************************************************************
 *
 *      Copyright (c) DiBcom SA.  All rights reserved.
 *
 *      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *      PURPOSE.
 *
 ****************************************************************************/

#ifndef MSG_DATA_IF_H
#define MSG_DATA_IF_H

/* Format of MsgData message */
#define SET_DATA_FORMAT(ItemIndex, Type, FirstFrag, LastFrag, NbRows, BlockType, BlockId, FrameId) (\
                       ((FrameId   & 0xFF) << 24) |\
                       ((NbRows    & 0x7)  << 21) |\
                       ((ItemIndex & 0x1F) << 16) |\
                       ((Type      & 0x1F) << 11) |\
                       ((LastFrag  & 1)    << 10) |\
                       ((FirstFrag & 1)    << 9)  |\
                       ((BlockType & 1)    << 8)  |\
                       ((BlockId   & 0xFF)))

#define MSG_DATA_FRAME_ID(format)     (uint8_t)      ((format & 0xFF000000) >> 24)    /* frame identifier, 8 bits */
#define MSG_DATA_NB_ROWS(format)      (uint8_t)      ((format & 0x00E00000) >> 21)    /* mpe Table Nb rows 3 bits */
#define MSG_DATA_ITEM_INDEX(format)   (uint8_t)      ((format & 0x001F0000) >> 16)    /* index of the item : 5 bits */
#define MSG_DATA_TYPE(format)         (uint8_t)      ((format & 0x0000F800) >> 11)    /* Type de Data on 5 bits */
#define MSG_DATA_LAST_FRAG(format)    (uint8_t)      ((format & 0x00000400) >> 10)    /* flag last fragment, 1 bit */
#define MSG_DATA_FIRST_FRAG(format)   (uint8_t)      ((format & 0x00000200) >> 9)     /* flag first fragment sur 1 bit */
#define MSG_DATA_BLOCK_TYPE(format)   (uint8_t)      ((format & 0x00000100) >> 8)     /* block Type: 0 for 4K, 1 for 64K */
#define MSG_DATA_BLOCK_ID(format)     (uint8_t)      ((format & 0x000000FF))          /* block identifier on 8 bits */

/*
 * dynamic message to move data between adt adn adst. see IN_MSG_FRAME_INFO
 */
#define SET_MSG_MOVE_DATA(Type, FirstCol, Table)         (((Type & 0xFFFF) <<  16) | ((FirstCol & 0xFF) <<  8) | (Table & 0xFF))
#define GET_MSG_INDEX(Info)                              (uint8_t)((Info&0xFF00) >> 8)
#define GET_MSG_TABLE(Info)                              (uint8_t)(Info&0xFF)
#define GET_MSG_TYPE(Info)                               (uint16_t)((Info&0xFFFF0000) >> 16)



#endif


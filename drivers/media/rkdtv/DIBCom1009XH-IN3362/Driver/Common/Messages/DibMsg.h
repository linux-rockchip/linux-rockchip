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

#ifndef DIBMSG_H_
#define DIBMSG_H_

#include "DibMsgDebug.h"
#include "DibMsgMac.h"
#include "DibMsgLayout.h"
#include "DibMsgData.h"
#include "DibMsgCas.h"

/* Add in this table the new Types of messages */
/*uint32_t tab_bits[] = {MAX_MSG_DATA_BITS, MAX_MSG_DEBUG_BITS, MAX_MSG_LAYOUT_BITS, MAX_MSG_MAC_BITS};

#define MAX_N(tab, N)      ({uint32_t MAX = tab[0]; \
                           uint32_t ii = 1; \
                           while(ii < N) \
                           { MAX = MAX_2(MAX, tab[ii]);\
                           ii++;}\
                           MAX;\
                           })
#define MAX_MSG_BITS       (MAX_N(tab_bits, (sizeof(tab_bits)/sizeof(uint32_t))))
*/

#define MAX_2(a, b)        ((a > b) ? (a) : (b))
#define MAX_4(a, b, c, d)  (MAX_2(MAX_2(a, b), MAX_2(c, d)))
#define MAX_5(a, b, c, d, e ) (MAX_2( a, MAX_4(b, c, d, e)))

#define MAX_MSG_BITS       (MAX_5(MAX_MSG_DATA_BITS, MAX_MSG_DEBUG_BITS, MAX_MSG_LAYOUT_BITS, MAX_MSG_MAC_BITS, MAX_MSG_CAS_BITS ))
#define MAX_MSG_WORDS      (GetWords((MAX_MSG_BITS), 32))

#endif /* DIBMSG_H_ */

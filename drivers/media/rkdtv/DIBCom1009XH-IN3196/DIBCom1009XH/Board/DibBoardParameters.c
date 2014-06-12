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

/**************************************************************************************************
* @file "DibBoardStk7078.h"
* @brief Test Bridge Interface.
*
***************************************************************************************************/
#include "DibConfig.h" /* Must be first include of all SDK files - Defines compilation options */

#include "DibBoardParameters.h"

/* PLL Configuration  With external clock = 30.000000 */
struct DibDemodPllCfg9000 DibPllConfig9080 =
{
   279620,       /* vcxo   */
   20452225,     /* timf   */
   30000000      /* XtalHz */
};

/* PLL Configuration  With external clock = 13.000000 */
struct DibDemodPllCfg9000 DibPllConfig13Mhz9080 =
{
   279039,       /* vcxo   */
   20409705,     /* timf   */
   13000000      /* XtalHz */
};

/* PLL Configuration for COFDM BW_MHz = 8.000000 With external clock = 12.000000 */
struct DibDemodPllCfg9000 DibPllConfig12Mhz9080 =
{
   279620,       /* vcxo    */
   20452225,     /* timf    */
   12000000      /* XtalHz  */
};

struct DibDemodPllCfg9000 DibPllConfig26Mhz9080 =
{
   286790,       /* vcxo             */
   20976641,     /* timf ADC: 14.625 */
   26000000      /* XtalHz           */
};

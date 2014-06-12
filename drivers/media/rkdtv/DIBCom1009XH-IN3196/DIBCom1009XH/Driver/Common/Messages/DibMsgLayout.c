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





#include "DibMsgLayout.h"

/*! \file DibMsgLayout.h */

/*! \brief Message to configure the Pll of DiBxxx9x-based devices
 *
 * This message is used to configure the PLL-parameters and thus the
 * supplied clock for DiBxxx9x-based devices.
 *
 * When this messages is sent the PLL and the internal clock-system is
 * reset. This message can only be sent at certain places during the
 * initialization phase of the chip.
 *
 * To find out the right settings for the hardware you're using please
 * refer to your contact at DiBcom.
 *
 * \warning Setting wrong settings can seriouly harm the hardware.
 */
void MsgLayoutDiBxxx9xPllConfigPack(struct MsgLayoutDiBxxx9xPllConfig *s, struct SerialBuf *ctx)
{
   /*! Message header */
   MsgHeaderPack(&s->Head, ctx);
   /*! The supplied clock-frequency in kHz */
   SerialBufWriteField(ctx, 32, s->clock_khz);
   /*! Is the PLL on bypassed and the clock is supplied directly. Default: 0 */
   SerialBufWriteField(ctx, 1, s->pll_bypass);
   /*! The PLL-range used. Default: */
   SerialBufWriteField(ctx, 1, s->pll_range);
   /*! The PLL-pre-divider. Default: 1 (for 30MHz crystals) */
   SerialBufWriteField(ctx, 6, s->pll_prediv);
   /*! The PLL-loop-divider. Default: 8 (for 30MHz crystals) */
   SerialBufWriteField(ctx, 6, s->pll_loopdiv);
   /*! ADC clock ratio. Default: 8 */
   SerialBufWriteField(ctx, 8, s->adc_clock_ratio);
   /*! PLL internal loop-filter. Default: 1. */
   SerialBufWriteField(ctx, 16, s->pll_int_loop_filt);
   /*! The input-clock is forwarded to the next chip if this field is set to 1. If set to 0 the clock is not available. */
   SerialBufWriteField(ctx, 1, s->clkouttobamse);
}

/*! Enumeration of supported GPIO functionalities */

/*! \brief Defining the usage of GPIO-pins
 *
 * This structure is used to define the usage of the GPIO-pins of a chipset.
 */
void MsgGpioDefinitionPack(struct MsgGpioDefinition *s, struct SerialBuf *ctx)
{
   /*! GPIO definition for this PIN */
   SerialBufWriteField(ctx, 8, s->GpioDef);
   /*! GPIO pin-number for which this definition is applied to */
   SerialBufWriteField(ctx, 16, s->GpioPin);
}

/*! \brief Message for the GPIO configuration of a chipset
 *
 * This message is sent initially to the chip to configure the usage of all GPIO-pins.
 */
void MsgGpioDefListPack(struct MsgGpioDefList *s, struct SerialBuf *ctx)
{
   int i;

   /*! Message Header */
   MsgHeaderPack(&s->Head, ctx);

   /*! Number of GPIO definitions filled in GpioDefList */
   SerialBufWriteField(ctx, 32, s->GpioSize);

   /*! Array of GPIO-definitions */
   for(i = 0; i < 16;i++ )
   {
      MsgGpioDefinitionPack(&s->GpioDefList[i], ctx);
   }
}

/*! \brief Message to control a GPIO-functionally of a chip
 *
 * This message can be used to activate or deactivate a single GPIO-functionally manually.
 * This message can be sent at any moment after the MsgGpioDefList has been sent.
 */
void MsgGpioActivationPack(struct MsgGpioActivation *s, struct SerialBuf *ctx)
{

   /*! Message Header */
   MsgHeaderPack(&s->Head, ctx);
   /*! GPIO definition to be controlled */
   SerialBufWriteField(ctx, 8, s->GpioDef);
   /*! Boolean value for the given GpioDef */
   SerialBufWriteField(ctx, 1, s->GpioSet);
}

/*! \brief Radio-frequency-switch range
 *
 * This structure is used to described the RF-switch 1 and 2 state for a Frequency range which ends at FreqMhzEnd.
 */

/*! \brief Message for the RF switch configuration
 *
 * This message is used to configure the logic behind the
 * eGPIO_CTRL_SW_-definitions.
 */
void MsgRfSwitchControlPack(struct MsgRfSwitchControl *s, struct SerialBuf *ctx)
{
   int i;

   /*! Message header */
   MsgHeaderPack(&s->Head, ctx);

   /*! List of RF-switch frequency ranges. A maximum of 6-entries are
    * possible. An entry which has FreqMHzEnd set to 0 is terminal.
    */
   for(i = 0; i < 6;i++ )
   {
      RfSwitchRangePack(&s->Range[i], ctx);
   }
}

/*! \brief Message used during handover add/del middle to enable/disable the diversity of a slave demod
 */
void MsgSetDiversityOutPack(struct MsgSetDiversityOut *s, struct SerialBuf *ctx)
{
   MsgHeaderPack(&s->Head, ctx);
   SerialBufWriteField(ctx, 8, s->Enable);
}

/*! \brief Message which acknowledges a previously sent configuration
 * message
 */
void MsgLayoutAckPack(struct MsgLayoutAck *s, struct SerialBuf *ctx)
{

   /*! Message header */
   MsgHeaderPack(&s->Head, ctx);
   /*! Return status */
   SerialBufWriteField(ctx, 32, s->Status);
}







/*! \file DibMsgLayout.h */

/*! \brief Message to configure the Pll of DiBxxx9x-based devices
 *
 * This message is used to configure the PLL-parameters and thus the
 * supplied clock for DiBxxx9x-based devices.
 *
 * When this messages is sent the PLL and the internal clock-system is
 * reset. This message can only be sent at certain places during the
 * initialization phase of the chip.
 *
 * To find out the right settings for the hardware you're using please
 * refer to your contact at DiBcom.
 *
 * \warning Setting wrong settings can seriouly harm the hardware.
 */
void MsgLayoutDiBxxx9xPllConfigUnpack(struct SerialBuf *ctx, struct MsgLayoutDiBxxx9xPllConfig *s)
{
   /*! Message header */
   MsgHeaderUnpack(ctx, &s->Head);
   /*! The supplied clock-frequency in kHz */
   s->clock_khz                   = SerialBufReadField(ctx, 32, 0);
   /*! Is the PLL on bypassed and the clock is supplied directly. Default: 0 */
   s->pll_bypass                  = SerialBufReadField(ctx, 1, 0);
   /*! The PLL-range used. Default: */
   s->pll_range                   = SerialBufReadField(ctx, 1, 0);
   /*! The PLL-pre-divider. Default: 1 (for 30MHz crystals) */
   s->pll_prediv                  = SerialBufReadField(ctx, 6, 0);
   /*! The PLL-loop-divider. Default: 8 (for 30MHz crystals) */
   s->pll_loopdiv                 = SerialBufReadField(ctx, 6, 0);
   /*! ADC clock ratio. Default: 8 */
   s->adc_clock_ratio             = SerialBufReadField(ctx, 8, 0);
   /*! PLL internal loop-filter. Default: 1. */
   s->pll_int_loop_filt           = SerialBufReadField(ctx, 16, 0);
   /*! The input-clock is forwarded to the next chip if this field is set to 1. If set to 0 the clock is not available. */
   s->clkouttobamse               = SerialBufReadField(ctx, 1, 0);
}

/*! Enumeration of supported GPIO functionalities */

/*! \brief Defining the usage of GPIO-pins
 *
 * This structure is used to define the usage of the GPIO-pins of a chipset.
 */
void MsgGpioDefinitionUnpack(struct SerialBuf *ctx, struct MsgGpioDefinition *s)
{
   /*! GPIO definition for this PIN */
   s->GpioDef                     = SerialBufReadField(ctx, 8, 1);
   /*! GPIO pin-number for which this definition is applied to */
   s->GpioPin                     = SerialBufReadField(ctx, 16, 0);
}

/*! \brief Message for the GPIO configuration of a chipset
 *
 * This message is sent initially to the chip to configure the usage of all GPIO-pins.
 */
void MsgGpioDefListUnpack(struct SerialBuf *ctx, struct MsgGpioDefList *s)
{
   int i;

   /*! Message Header */
   MsgHeaderUnpack(ctx, &s->Head);

   /*! Number of GPIO definitions filled in GpioDefList */
   s->GpioSize                    = SerialBufReadField(ctx, 32, 0);

   /*! Array of GPIO-definitions */
   for(i = 0; i < 16;i++ )
   {
      MsgGpioDefinitionUnpack(ctx, &s->GpioDefList[i]);
   }
}

/*! \brief Message to control a GPIO-functionally of a chip
 *
 * This message can be used to activate or deactivate a single GPIO-functionally manually.
 * This message can be sent at any moment after the MsgGpioDefList has been sent.
 */
void MsgGpioActivationUnpack(struct SerialBuf *ctx, struct MsgGpioActivation *s)
{

   /*! Message Header */
   MsgHeaderUnpack(ctx, &s->Head);
   /*! GPIO definition to be controlled */
   s->GpioDef                     = SerialBufReadField(ctx, 8, 1);
   /*! Boolean value for the given GpioDef */
   s->GpioSet                     = SerialBufReadField(ctx, 1, 0);
}

/*! \brief Radio-frequency-switch range
 *
 * This structure is used to described the RF-switch 1 and 2 state for a Frequency range which ends at FreqMhzEnd.
 */

/*! \brief Message for the RF switch configuration
 *
 * This message is used to configure the logic behind the
 * eGPIO_CTRL_SW_-definitions.
 */
void MsgRfSwitchControlUnpack(struct SerialBuf *ctx, struct MsgRfSwitchControl *s)
{
   int i;

   /*! Message header */
   MsgHeaderUnpack(ctx, &s->Head);

   /*! List of RF-switch frequency ranges. A maximum of 6-entries are
    * possible. An entry which has FreqMHzEnd set to 0 is terminal.
    */
   for(i = 0; i < 6;i++ )
   {
      RfSwitchRangeUnpack(ctx, &s->Range[i]);
   }
}

/*! \brief Message used during handover add/del middle to enable/disable the diversity of a slave demod
 */
void MsgSetDiversityOutUnpack(struct SerialBuf *ctx, struct MsgSetDiversityOut *s)
{
   MsgHeaderUnpack(ctx, &s->Head);
   s->Enable                      = SerialBufReadField(ctx, 8, 0);
}

/*! \brief Message which acknowledges a previously sent configuration
 * message
 */
void MsgLayoutAckUnpack(struct SerialBuf *ctx, struct MsgLayoutAck *s)
{

   /*! Message header */
   MsgHeaderUnpack(ctx, &s->Head);
   /*! Return status */
   s->Status                      = SerialBufReadField(ctx, 32, 0);
}




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

#ifndef __MSG_TYPE_LAYOUT__
#define __MSG_TYPE_LAYOUT__

#ifdef __cplusplus
#define EXTERN  extern "C"
#else
#define EXTERN
#endif


/* Max message length */
#define MAX_MSG_LAYOUT_BITS            448

/* Input message identifiers */
#define IN_MSG_LAYOUT_ACK              5
#define MAX_MSG_LAYOUT_IN              6

/* Output message identifiers */
#define OUT_MSG_LAYOUT_CLOCK           1
#define OUT_MSG_GPIO_DEF               3
#define OUT_MSG_GPIO_ACTIVATION        4
#define OUT_MSG_RF_SWITCH_CONTROL      8
#define OUT_MSG_SET_DIVERSITY_OUT      9
#define MAX_MSG_LAYOUT_OUT             10




#include "DibMsgTypes.h"
#include "DibMsgGeneric.h"
#include "DibMsgFields.h"
#include "DibMsgLayoutIf.h"
#include "DibStruct.h"
#include "DibExtAPI.h"


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
struct MsgLayoutDiBxxx9xPllConfig
{
   /*! Message header */
   struct MsgHeader Head;
   /*! The supplied clock-frequency in kHz */
   uint32_t clock_khz;
   /*! Is the PLL on bypassed and the clock is supplied directly. Default: 0 */
   uint8_t pll_bypass;
   /*! The PLL-range used. Default: */
   uint8_t pll_range;
   /*! The PLL-pre-divider. Default: 1 (for 30MHz crystals) */
   uint8_t pll_prediv;
   /*! The PLL-loop-divider. Default: 8 (for 30MHz crystals) */
   uint8_t pll_loopdiv;
   /*! ADC clock ratio. Default: 8 */
   uint8_t adc_clock_ratio;
   /*! PLL internal loop-filter. Default: 1. */
   uint16_t pll_int_loop_filt;
   /*! The input-clock is forwarded to the next chip if this field is set to 1. If set to 0 the clock is not available. */
   uint8_t clkouttobamse;
};
/*! Enumeration of supported GPIO functionalities */
enum GpioDefinition
{
   /*! Smartcard Clock */
   eGPIO_SMARTCARD_CLK = 0,
   /*! Smartcard Reset */
   eGPIO_SMARTCARD_RESET = 1,
   /*! Smartcard I/O */
   eGPIO_SMARTCARD_I_O = 2,
   /*! Smartcard detect presence low  */
   eGPIO_SMARTCARD_DETECTN = 3,
   /*! Smartcard detect presence high  */
   eGPIO_SMARTCARD_DETECT = 4,
   /*! Smartcard power-supply */
   eGPIO_SMARTCARD_VDD = 5,
   /*! Smartcard shutdown */
   eGPIO_SMARTCARD_SHUTDOWN = 6,
   /*! A led is connected to this GPIO */
   eGPIO_LED = 7,
   /*! Antenna power-supply */
   eGPIO_ANT_POWER_ON = 8,
   /*! DiBCtrl clock line */
   eGPIO_DIBCTRL_CLK = 9,
   /*! DiBCtrl data line */
   eGPIO_DIBCTRL_DATA = 10,
   /*! 0-wire - bus */
   eGPIO_0_WIRE = 11,
   /*! 1-wire - bus */
   eGPIO_1_WIRE = 12,
   /*! RF-switch 1 */
   eGPIO_CTRL_SW_1 = 13,
   /*! RF-switch 2 */
   eGPIO_CTRL_SW_2 = 14,
   /*! Spi Flash TX gpio */
   eGPIO_FLASH_TX = 15,
   /*! Spi Flash RX gpio */
   eGPIO_FLASH_RX = 16,
   /*! Tuner - reset */
   eGPIO_TUNER_RESET = 17,
   /*! User-controlled functionality 1 */
   eGPIO_USER1 = 18,
   /*! User-controlled functionality 2 */
   eGPIO_USER2 = 19,
   /*! User-controlled functionality 3 */
   eGPIO_USER3 = 20,
   /*! User-controlled functionality 4 */
   eGPIO_USER4 = 21,
   /*! SPI Flash clock  */
   eGPIO_FLASH_CLK = 22,
   /*! ChipSelect SPI flash */
   eGPIO_FLASH_CS = 23,
   /*! GPIO which is controlling the power-down/up of the slave devices */
   eGPIO_SLAVE_DEVICE_POWER_DOWN = 24,
   /*! A GPIO which is not connected */
   eGPIO_NOT_CONNECTED = 25,
   /*! Placeholder for future definitions */
   eGPIO_UNDEFINED = 26,
};
/*! \brief Defining the usage of GPIO-pins
 *
 * This structure is used to define the usage of the GPIO-pins of a chipset.
 */
struct MsgGpioDefinition
{
   /*! GPIO definition for this PIN */
   enum GpioDefinition GpioDef;
   /*! GPIO pin-number for which this definition is applied to */
   uint16_t GpioPin;
};
/*! \brief Message for the GPIO configuration of a chipset
 *
 * This message is sent initially to the chip to configure the usage of all GPIO-pins.
 */
struct MsgGpioDefList
{

   /*! Message Header */
   struct MsgHeader Head;

   /*! Number of GPIO definitions filled in GpioDefList */
   uint32_t GpioSize;

   /*! Array of GPIO-definitions */
   struct MsgGpioDefinition GpioDefList[16];
};
/*! \brief Message to control a GPIO-functionally of a chip
 *
 * This message can be used to activate or deactivate a single GPIO-functionally manually.
 * This message can be sent at any moment after the MsgGpioDefList has been sent.
 */
struct MsgGpioActivation
{

   /*! Message Header */
   struct MsgHeader Head;
   /*! GPIO definition to be controlled */
   enum GpioDefinition GpioDef;
   /*! Boolean value for the given GpioDef */
   uint8_t GpioSet;
};
/*! \brief Radio-frequency-switch range
 *
 * This structure is used to described the RF-switch 1 and 2 state for a Frequency range which ends at FreqMhzEnd.
 */
struct RfSwitchRange
{
   /*! Frequency in MHz which defines the end of a range */
   uint16_t FreqMHzEnd;
   /*! State of RF-switch 1 for this range - eGPIO_CTRL_SW_1 */
   uint8_t Sw1State;
   /*! State of RF-switch 2 for this range - eGPIO_CTRL_SW_2 */
   uint8_t Sw2State;
};
/*! \brief Message for the RF switch configuration
 *
 * This message is used to configure the logic behind the
 * eGPIO_CTRL_SW_-definitions.
 */
struct MsgRfSwitchControl
{

   /*! Message header */
   struct MsgHeader Head;

   /*! List of RF-switch frequency ranges. A maximum of 6-entries are
    * possible. An entry which has FreqMHzEnd set to 0 is terminal.
    */
   struct RfSwitchRange Range[6];
};
/*! \brief Message used during handover add/del middle to enable/disable the diversity of a slave demod
 */
struct MsgSetDiversityOut
{
   struct MsgHeader Head;
   uint8_t Enable;
};
/*! \brief Message which acknowledges a previously sent configuration
 * message
 */
struct MsgLayoutAck
{

   /*! Message header */
   struct MsgHeader Head;
   /*! Return status */
   uint32_t Status;
};






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
#define     MsgLayoutDiBxxx9xPllConfigBits 103
#define     MsgLayoutDiBxxx9xPllConfigPackInit(s, ctx)  SerialBufRestart(ctx); MsgLayoutDiBxxx9xPllConfigPack(s, ctx);
#define     MsgLayoutDiBxxx9xPllConfigUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgLayoutDiBxxx9xPllConfigUnpack(ctx, s);
EXTERN void MsgLayoutDiBxxx9xPllConfigPack(struct MsgLayoutDiBxxx9xPllConfig *s, struct SerialBuf *ctx);
EXTERN void MsgLayoutDiBxxx9xPllConfigUnpack(struct SerialBuf *ctx, struct MsgLayoutDiBxxx9xPllConfig *s);

/*! Enumeration of supported GPIO functionalities */

/*! \brief Defining the usage of GPIO-pins
 *
 * This structure is used to define the usage of the GPIO-pins of a chipset.
 */
#define     MsgGpioDefinitionBits 24
#define     MsgGpioDefinitionPackInit(s, ctx)  SerialBufRestart(ctx); MsgGpioDefinitionPack(s, ctx);
#define     MsgGpioDefinitionUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGpioDefinitionUnpack(ctx, s);
EXTERN void MsgGpioDefinitionPack(struct MsgGpioDefinition *s, struct SerialBuf *ctx);
EXTERN void MsgGpioDefinitionUnpack(struct SerialBuf *ctx, struct MsgGpioDefinition *s);

/*! \brief Message for the GPIO configuration of a chipset
 *
 * This message is sent initially to the chip to configure the usage of all GPIO-pins.
 */
#define     MsgGpioDefListBits 448
#define     MsgGpioDefListPackInit(s, ctx)  SerialBufRestart(ctx); MsgGpioDefListPack(s, ctx);
#define     MsgGpioDefListUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGpioDefListUnpack(ctx, s);
EXTERN void MsgGpioDefListPack(struct MsgGpioDefList *s, struct SerialBuf *ctx);
EXTERN void MsgGpioDefListUnpack(struct SerialBuf *ctx, struct MsgGpioDefList *s);

/*! \brief Message to control a GPIO-functionally of a chip
 *
 * This message can be used to activate or deactivate a single GPIO-functionally manually.
 * This message can be sent at any moment after the MsgGpioDefList has been sent.
 */
#define     MsgGpioActivationBits 41
#define     MsgGpioActivationPackInit(s, ctx)  SerialBufRestart(ctx); MsgGpioActivationPack(s, ctx);
#define     MsgGpioActivationUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgGpioActivationUnpack(ctx, s);
EXTERN void MsgGpioActivationPack(struct MsgGpioActivation *s, struct SerialBuf *ctx);
EXTERN void MsgGpioActivationUnpack(struct SerialBuf *ctx, struct MsgGpioActivation *s);

/*! \brief Radio-frequency-switch range
 *
 * This structure is used to described the RF-switch 1 and 2 state for a Frequency range which ends at FreqMhzEnd.
 */
#define     RfSwitchRangeBits 18
#define     RfSwitchRangePackInit(s, ctx)  SerialBufRestart(ctx); RfSwitchRangePack(s, ctx);
#define     RfSwitchRangeUnpackInit(ctx, s)  SerialBufRestart(ctx); RfSwitchRangeUnpack(ctx, s);
#define     RfSwitchRangePack(s, ctx)   SIPSIItemParametersPack((struct SIPSIItemParameters *)(s), ctx)
#define     RfSwitchRangeUnpack(ctx, s)   SIPSIItemParametersUnpack(ctx, (struct SIPSIItemParameters *)(s))

/*! \brief Message for the RF switch configuration
 *
 * This message is used to configure the logic behind the
 * eGPIO_CTRL_SW_-definitions.
 */
#define     MsgRfSwitchControlBits 140
#define     MsgRfSwitchControlPackInit(s, ctx)  SerialBufRestart(ctx); MsgRfSwitchControlPack(s, ctx);
#define     MsgRfSwitchControlUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgRfSwitchControlUnpack(ctx, s);
EXTERN void MsgRfSwitchControlPack(struct MsgRfSwitchControl *s, struct SerialBuf *ctx);
EXTERN void MsgRfSwitchControlUnpack(struct SerialBuf *ctx, struct MsgRfSwitchControl *s);

/*! \brief Message used during handover add/del middle to enable/disable the diversity of a slave demod
 */
#define     MsgSetDiversityOutBits 40
#define     MsgSetDiversityOutPackInit(s, ctx)  SerialBufRestart(ctx); MsgSetDiversityOutPack(s, ctx);
#define     MsgSetDiversityOutUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgSetDiversityOutUnpack(ctx, s);
EXTERN void MsgSetDiversityOutPack(struct MsgSetDiversityOut *s, struct SerialBuf *ctx);
EXTERN void MsgSetDiversityOutUnpack(struct SerialBuf *ctx, struct MsgSetDiversityOut *s);

/*! \brief Message which acknowledges a previously sent configuration
 * message
 */
#define     MsgLayoutAckBits 64
#define     MsgLayoutAckPackInit(s, ctx)  SerialBufRestart(ctx); MsgLayoutAckPack(s, ctx);
#define     MsgLayoutAckUnpackInit(ctx, s)  SerialBufRestart(ctx); MsgLayoutAckUnpack(ctx, s);
EXTERN void MsgLayoutAckPack(struct MsgLayoutAck *s, struct SerialBuf *ctx);
EXTERN void MsgLayoutAckUnpack(struct SerialBuf *ctx, struct MsgLayoutAck *s);



#endif /*__MSG_TYPE_LAYOUT__*/



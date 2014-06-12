/* drivers/adc/chips/rk29_hsadc.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
*/

#ifndef __ASM_RK30_HSADC_H
#define __ASM_RK30_HSADC_H

//hsadc register
#define         HSADC_CTRL                           0x00  /* control register */
#define         HSADC_IER                            0x04  /* interrupt enable/mask register */
#define         HSADC_ISR                            0x08  /* inputerrupt status register */
#define         HSADC_TS_FAIL                        0x0c  /* ts fail register */
#define         HSADC_CGCTL                          0x10  /* clock gating control register */
#define         HSADC_DATA                           0x20  /* data register */

//define almost full trigger level(HSADC_CTRL 27:24)
#define         HSADC_ALMOST_FULL_LEVEL64       (0<<24) 
#define         HSADC_ALMOST_FULL_LEVEL68       (1<<24)
#define         HSADC_ALMOST_FULL_LEVEL72       (3<<24) 
#define         HSADC_ALMOST_FULL_LEVEL76       (4<<24) 
#define         HSADC_ALMOST_FULL_LEVEL80       (5<<24) 
#define         HSADC_ALMOST_FULL_LEVEL84       (6<<24) 
#define         HSADC_ALMOST_FULL_LEVEL88       (7<<24) 
#define         HSADC_ALMOST_FULL_LEVEL92       (8<<24) 
#define         HSADC_ALMOST_FULL_LEVEL96       (9<<24) 
#define         HSADC_ALMOST_FULL_LEVEL100     (10<<24) 
#define         HSADC_ALMOST_FULL_LEVEL104     (11<<24)
#define         HSADC_ALMOST_FULL_LEVEL108     (12<<24)
#define         HSADC_ALMOST_FULL_LEVEL112     (13<<24)
#define         HSADC_ALMOST_FULL_LEVEL116     (14<<24)
#define         HSADC_ALMOST_FULL_LEVEL120     (15<<24)
#define         HSADC_ALMOST_FULL_LEVEL124     (16<<24)

//define almost empty trigger level(HSADC_CTRL 19:16)
#define         HSADC_ALMOST_EMPTY_LEVEL0       (0<<16) 
#define         HSADC_ALMOST_EMPTY_LEVEL4       (1<<16)
#define         HSADC_ALMOST_EMPTY_LEVEL8       (2<<16) 
#define         HSADC_ALMOST_EMPTY_LEVEL12     (3<<16) 
#define         HSADC_ALMOST_EMPTY_LEVEL16     (4<<16) 
#define         HSADC_ALMOST_EMPTY_LEVEL20     (5<<16) 
#define         HSADC_ALMOST_EMPTY_LEVEL24     (6<<16) 
#define         HSADC_ALMOST_EMPTY_LEVEL28     (7<<16) 
#define         HSADC_ALMOST_EMPTY_LEVEL32     (8<<16) 
#define         HSADC_ALMOST_EMPTY_LEVEL36     (9<<16) 
#define         HSADC_ALMOST_EMPTY_LEVEL40     (10<<16)
#define         HSADC_ALMOST_EMPTY_LEVEL44     (11<<16)
#define         HSADC_ALMOST_EMPTY_LEVEL48     (12<<16)
#define         HSADC_ALMOST_EMPTY_LEVEL52     (13<<16)
#define         HSADC_ALMOST_EMPTY_LEVEL56     (14<<16)
#define         HSADC_ALMOST_EMPTY_LEVEL60     (15<<16)
#define         HSADC_ALMOST_EMPTY_LEVEL64     (16<<16)

//TS sync interface enable(HSADC_CTRL 10)
#define         HSADC_SYNC_INTERFACE_DISABLE   (0<<10)
#define         HSADC_SYNC_INTERFACE_ENABLE           (1<<10)

//TS valid interface enable(HSADC_CTRL 9)
#define         HSADC_VALID_INTERFACE_DISABLE   (0<<9)
#define         HSADC_VALID_INTERFACE_ENABLE           (1<<9)

//MPEG transport stream data input select(HSADC_CTRL 8)
#define         HSADC_NOT_SELECT_MPEG_TRANSPORT_STREAM_INPUT   (0<<8)
#define         HSADC_SELECT_MPEG_TRANSPORT_STREAM_INPUT           (1<<8)

//data input channel select(HSADC_CTRL 7)
#define         HSADC_SINGLE_CHANNEL_DATA_INPUT       (0<<7)
#define         HSADC_DOUBLE_CHANNEL_DATA_INPUT      (1<<7)

//store mode select(HSADC_CTRL 6)
#define         HSADC_STORE_MODE_8BIT       (0<<6)
#define         HSADC_STORE_MODE_16BIT       (1<<6)

//DMA request mode(HSADC_CTRL 5)
#define         HSADC_ALMOST_FULL_DMA_REQUEST      (1<<5)
#define         HSADC_ALMOST_EMPTY_DMA_REQUEST         (0<<5)

//control the most significant bit negation(HSADC_CTRL 4)
#define         HSADC_MOST_SIGNIFICANT_BIT_NEGATION               (1<<4)
#define         HSADC_MOST_SIGNIFICANT_BIT_NOT_NEGATION      (0<<4)

//high 8-bit/10bit or low 8-bit/10bit select(HSADC_CTRL 3)
#define         HSADC_STORE_TO_HIGH_8_10_BIT      (1<<3)
#define         HSADC_STORE_TO_LOW_8_10_BIT      (0<<3)

//the data bus width of AD converter(HSADC_CTRL 2)
#define         HSADC_DATA_BUS_WIDTH_10_BIT       (1<<2)
#define         HSADC_DATA_BUS_WIDTH_8_BIT        (0<<2)

//GPS input select(HSADC_CTRL 1)
#define         HSADC_NOT_SELECT_GPS      (0<<1)
#define         HSADC_SELECT_GPS              (1<<1)

//HSADC interface unit enable bit(HSADC_CTRL 0)
#define         HSADC_INTERFACE_UNIT_ENABLE       (1<<0)
#define         HSADC_INTERFACE_UNIT_DISABLE      (0<<0)


//HSADC empty interrupt enable/disable  control(HSADC_IER 1)
#define         HSADC_EMPTY_INTERRUPT_ENABLE       (1<<1)
#define         HSADC_EMPTY_INTERRUPT_DISABLE      (0<<1)

//HSADC full interrupt enable/disable control(HSADC_IER 0)
#define         HSADC_FULL_INTERRUPT_ENABLE       (1<<0)
#define         HSADC_FULL_INTERRUPT_DISABLE     (0<<0)

//HSADC empty interrupt status(HSADC_ISR 1)
#define         HSADC_EMPTY_INTERRUPT_FLAG_GENERATE    (1<<1)
#define         HSADC_CLEAR_EMPTY_INTERRUPT_FLAG    (0<<1)

//HSADC full interrupt enable control(HSADC_ISR 0)
#define         HSADC_FULL_INTERRUPT_FLAG_GENERATE    (1<<0)
#define         HSADC_FULL_EMPTY_INTERRUPT_FLAG          (0<<0)

//HSADC ts fail register(HSADC_TS_FAIL 0)
#define         HSADC_TS_STREAM_DECODE_SUCCESS   (0<<0)
#define         HSADC_TS_STREAM_DECODE_FAIL          (1<<0)

//PID-FILTER register
#define         PIDF_GCTL                         0x00  /* global control register */
#define         PIDF_GSTA                         0x04   /* global status register */
#define         PIDF_ICTL                          0x10   /* interrupt control register */
#define         PIDF_IMR                           0x14   /* interrupt mask register */
#define         PIDF_ICLR                          0x18   /* interrupt clear register */
#define         PIDF_IRSR                          0x1C   /* Raw interrupt status(pre-masking) register */
#define         PIDF_ISR                            0x20   /* Interrupt status register */
#define         PIDF_CHID0                       0x80   /* Filter out ID & enable for channel 0 */
#define         PIDF_CHID1                       0x84   /* Filter out ID & enable for channel 1 */
#define         PIDF_CHID2                       0x88   /* Filter out ID & enable for channel 2 */
#define         PIDF_CHID63                     0x17C   /* Filter out ID & enable for channel 63 */
#define         PIDF_DR                             0x200   /* Data buffer write/read entrance */

#endif /* __ASM_RK30_HSADC_H */

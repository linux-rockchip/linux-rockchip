#include <adapter/frontend.h>
#include <tuner/dib0090.h>
#include "dib0090_priv.h"

#define EFUSE
#define TEST_EFUSE_VALUE

#define CAP_VALUE_MIN (uint8_t)  9
#define CAP_VALUE_MAX (uint8_t) 40
#define HR_MIN	      (uint8_t) 25
#define HR_MAX	      (uint8_t) 40
#define POLY_MIN      (uint8_t)  0
#define POLY_MAX      (uint8_t)  8

void dib0090_set_EFUSE(struct dib0090_state *state)
{
    uint8_t c,h,n;
    uint16_t e1, e2, e3, e4;
    uint16_t cal;

    e1 = dib0090_read_reg(state, EFUSE_1);
    e2 = dib0090_read_reg(state, EFUSE_2);
    e3 = dib0090_read_reg(state, EFUSE_3);
    e4 = dib0090_read_reg(state, EFUSE_4);

    //e2 = e4 = (5<<12) | (27<<6) | (0x18); /* Just for test */

    if ((state->identity.version == P1D_E_F) || // All P1F uses the internal calibration
        (state->identity.version == P1G) || (e2 == 0xffff)) {	//W0090G11R1 and W0090G11R1-D  : We will find the calibration Value of the Baseband

#ifdef CONFIG_TUNER_DIB0090_DEBUG
        dbgpl(&dib0090_dbg, "************It is a P1G needing a cal because of faulty EFUSE************");
#endif

        dib0090_write_reg(state,CALIBRATE,0x10); //Start the Calib
        cal = (dib0090_read_reg(state,CALIBRATE)>>6) & 0x3ff;

        if ((cal<670) || (cal==1023)) //Cal at 800 would give too high value for the n
            cal=850; //Recenter the n to 32
        n = 165 - ((cal * 10)>>6) ;
#ifdef CONFIG_TUNER_DIB0090_DEBUG
        dbgpl(&dib0090_dbg, "cal has been done = %d , and N_CAP = %d",cal,n);
#endif
        e2 = e4 = (3<<12) | (34<<6) | (n);
    }

#ifdef CONFIG_TUNER_DIB0090_DEBUG
    dbgpl(&dib0090_dbg, "************EFUSE upload************");
    dbgpl(&dib0090_dbg, "************e2=%x,e4=%x*************",e2,e4);
#endif
    if (e2!=e4) {
        e2 &= e4; /* Remove the redundancy  */
#ifdef CONFIG_TUNER_DIB0090_DEBUG
        dbgpl(&dib0090_dbg, "************Redundancy used*************");
#endif
    }

    if (e2 != 0xffff) {
        c = e2 & 0x3f;
        n = (e2 >> 12) & 0xf;
        h= (e2 >> 6) & 0x3f;

	if ((c >= CAP_VALUE_MAX) || (c <= CAP_VALUE_MIN))
	    c=32;
    else {
#ifdef CONFIG_TUNER_DIB0090_DEBUG
        dbgpl(&dib0090_dbg, "************ADD 14*************");
#endif
        c += 14;
    }
	if ((h >= HR_MAX) || (h <= HR_MIN))
	    h=34;
	if ((n >= POLY_MAX) || (n <= POLY_MIN))
	    n=3;

#ifdef TEST_EFUSE_VALUE
	if ((c >= CAP_VALUE_MIN) && (c <= CAP_VALUE_MAX) && (h >= HR_MIN) && (h <= HR_MAX) && (n >= POLY_MIN) && (n <= POLY_MAX))
#endif
	{
            dib0090_write_reg(state,CTRL_BIAS, (h << 10)) ;
            e2 = (n<<11) | ((h>>2)<<6) | (c);
            dib0090_write_reg(state,CTRL_BB_2, e2) ; /* Load the BB_2 */
        }
#ifdef CONFIG_TUNER_DIB0090_DEBUG
        dbgpl(&dib0090_dbg, "************EFUSE c=%d,n=%d,h=%d ************",c,n,h);
#endif
        /* test EFUSE value */
        //e3 = 0xd6d6;
        if (e1 != e3) {
#ifdef CONFIG_TUNER_DIB0090_DEBUG
            dbgpl(&dib0090_dbg, "************There is a Temperature calibration from Production************");
#endif
            if ((e3 >> 8) == (e3 & 0xff)) // The Temperature calibration is Valid from prod
                state->ptat_ref_offset = e3 & 0xff;
            else
                state->ptat_ref_offset = 0;
        } else
            state->ptat_ref_offset = 0;
#ifdef CONFIG_TUNER_DIB0090_DEBUG
        dbgpl(&dib0090_dbg, "************ptat_ref_offset=%d************",state->ptat_ref_offset);
#endif
    }
#ifdef CONFIG_TUNER_DIB0090_DEBUG
    else
        dbgpl(&dib0090_dbg, "************EFUSE is empty************");
#endif
}
//#endif

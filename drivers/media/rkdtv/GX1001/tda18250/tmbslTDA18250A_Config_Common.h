/*
Copyright (C) 2010 NXP B.V., All Rights Reserved.
This source code and any compilation or derivative thereof is the proprietary
information of NXP B.V. and is confidential in nature. Under no circumstances
is this software to be  exposed to or placed under an Open Source License of
any type without the expressed written permission of NXP B.V.
*
* \file          tmbslTDA18250A_Config.h
*
* \date          %modify_time%
*
* \author        Alexandre TANT
*
* \brief         Describe briefly the purpose of this file.
*
* REFERENCE DOCUMENTS :
*                TDA18250A_Driver_User_Guide.pdf
*
* \section info Change Information
*
*/

#ifndef _TDA18250A_CONFIG_COMMON_H
#define _TDA18250A_CONFIG_COMMON_H


#ifdef __cplusplus
extern "C"
{
#endif

    /*============================================================================*/
    /* Types and defines:                                                         */
    /*============================================================================*/

    /* Driver settings version definition */
#define TDA18250A_SETTINGS_CUSTOMER_NUM      (0)                     /* SW Settings Customer Number */
#define TDA18250A_SETTINGS_PROJECT_NUM       (0)                     /* SW Settings Project Number  */
#define TDA18250A_SETTINGS_MAJOR_VER         (1)                     /* SW Settings Major Version   */
#define TDA18250A_SETTINGS_MINOR_VER         (1)                     /* SW Settings Minor Version   */

#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_REF_GAIN_MAX 19
#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC1_MAX 12
#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC2_MAX 14
#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC3_MAX 8

#define TDA18250A_RF_MAX         1200000000 

#define TDA18250A_BP_FILTER_1      39424000 
#define TDA18250A_BP_FILTER_2      61952000
#define TDA18250A_BP_FILTER_3      86528000
#define TDA18250A_BP_FILTER_4     123392000
#define TDA18250A_BP_FILTER_5     172544000
#define TDA18250A_BP_FILTER_6     244222000
#define TDA18250A_BP_FILTER_7     320000000

/* {    X     ,    X    ,   X  ,           R           },         */
/* {    0     ,    1    ,   2  ,           F           },         */
/* {          ,         ,      ,                       },         */
/* {    x     ,    x    ,   x  ,                       },         */
/* {    1     ,    1    ,   1  ,           M           },         */
/* {    0     ,    0    ,   0  ,           H           },         */
/* {    0     ,    0    ,   0  ,           z           },         */
/* {    0     ,    0    ,   0  ,                       },         */
/* {    0     ,    0    ,   0  ,                       },         */
/* {    0     ,    0    ,   0  ,                       },         */
/* {    0     ,    0    ,   0  ,                       },         */
/* {          ,         ,   0  ,                       },         */
/* {          ,         ,      ,                       },         */
/* {          ,         ,      ,                       },         */
#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_REF_GAIN_MAX_NB    \
{                                                              \
   { 50544946,   314452,  -25806,     TDA18250A_BP_FILTER_1  }, \
   { 50544946,   314452,  -25806,     TDA18250A_BP_FILTER_2  }, \
   { 51239049,   232294,  -13936,     TDA18250A_BP_FILTER_3  }, \
   { 52388115,   156738,   -7029,     TDA18250A_BP_FILTER_4  }, \
   { 53961378,    99065,   -3425,     TDA18250A_BP_FILTER_5  }, \
   { 59788149,     6642,    -150,     TDA18250A_BP_FILTER_6  }, \
   { 53121988,    47726,    -805,     TDA18250A_BP_FILTER_7  }, \
   { 60101688,      140,    -154,     335360000              }, \
   { 65022437,   -32531,     334,     390656000              }, \
   { 54268908,    22541,    -372,     430592000              }, \
   { 73524942,   -64465,     641,     473500000              }, \
   { 52585046,    24369,    -326,     535040000              }, \
   { 68451433,   -35142,     254,     562688000              }, \
   { 69310100,   -38493,     264,     600000000              }, \
   { 50755238,    18868,    -176,     654848000              }, \
   { 42143765,    45877,    -383,     720384000              }, \
   { 92511767,   -93070,     592,     793088000              }, \
   { 56275795,     4022,     -69,     865792000              }, \
   { 47648303,    24175,    -174,     TDA18250A_RF_MAX}         \
}

/* {    X     ,    X    ,   X  ,           A           },                */
/* {    0     ,    1    ,   2  ,           G           },                */
/* {          ,         ,      ,           C           },                */
/* {    x     ,    x    ,   x  ,           1           },                */
/* {    1     ,    1    ,   1  ,           N           },                */
/* {    0     ,    0    ,   0  ,           B           },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {          ,         ,   0  ,                       },                */
/* {          ,         ,      ,                       },                */
/* {          ,         ,      ,                       },                */
/* {          ,         ,      ,                       },                */
#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC1NB_TUNER_LT_OFF_ST_OFF        \
{                                                                                \
   { -353296473,      15022,   -135,      TDA18250AAGC1_GAIN_Minus_11dB   }, \
   { -326948330,       8002,    -40,      TDA18250AAGC1_GAIN_Minus_8dB    }, \
   { -295755910,     -14866,    198,      TDA18250AAGC1_GAIN_Minus_5dB    }, \
   { -265929070,      -6964,    115,      TDA18250AAGC1_GAIN_Minus_2dB    }, \
   { -230494030,       5669,    -10,      TDA18250AAGC1_GAIN_Plus_1dB         }, \
   { -204057517,     -13333,    181,      TDA18250AAGC1_GAIN_Plus_4dB         }, \
   { -172095183,     -14280,    203,      TDA18250AAGC1_GAIN_Plus_7dB         }, \
   { -140936067,     -15536,    240,      TDA18250AAGC1_GAIN_Plus_10dB        }, \
   { -106777320,     -17578,    230,      TDA18250AAGC1_GAIN_Plus_13dB        }, \
   { -79847410,       14190,    -96,      TDA18250AAGC1_GAIN_Plus_16dB        }, \
   { -45311661,        9327,    -25,      TDA18250AAGC1_GAIN_Plus_19dB       }, \
   {         0,           0,      0,      TDA18250AAGC1_GAIN_Plus_22dB       }  \
}

#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC1NB_TUNER_LT_OFF_ST_ON          \
{                                                                                \
   { -352797083,       9716,    -88,      TDA18250AAGC1_GAIN_Minus_11dB   }, \
   { -327520540,       3161,     18,      TDA18250AAGC1_GAIN_Minus_8dB    }, \
   { -296171260,     -17660,    223,      TDA18250AAGC1_GAIN_Minus_5dB    }, \
   { -266310830,     -10556,    147,      TDA18250AAGC1_GAIN_Minus_2dB    }, \
   { -231036293,       2548,     14,      TDA18250AAGC1_GAIN_Plus_1dB         }, \
   { -204386737,     -18262,    222,      TDA18250AAGC1_GAIN_Plus_4dB         }, \
   { -172356037,     -19467,    236,      TDA18250AAGC1_GAIN_Plus_7dB         }, \
   { -141172937,     -22381,    283,      TDA18250AAGC1_GAIN_Plus_10dB        }, \
   { -106660657,     -27852,    295,      TDA18250AAGC1_GAIN_Plus_13dB        }, \
   { -79232070,        -341,     -6,      TDA18250AAGC1_GAIN_Plus_16dB        }, \
   { -44114841,      -11686,    102,      TDA18250AAGC1_GAIN_Plus_19dB       }, \
   {   2544050,      -36685,    247,      TDA18250AAGC1_GAIN_Plus_22dB       }  \
}

#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC1NB_TUNER_LT_ON_ST_ON          \
{                                                                                \
   { -354109980,       9060,    -90,      TDA18250AAGC1_GAIN_Minus_11dB   }, \
   { -328349970,       2598,     -2,      TDA18250AAGC1_GAIN_Minus_8dB    }, \
   { -296721540,     -20309,    228,      TDA18250AAGC1_GAIN_Minus_5dB    }, \
   { -267087650,     -13325,    148,      TDA18250AAGC1_GAIN_Minus_2dB    }, \
   { -231461830,      -1953,     21,      TDA18250AAGC1_GAIN_Plus_1dB         }, \
   { -204928130,     -23081,    223,      TDA18250AAGC1_GAIN_Plus_4dB         }, \
   { -172642377,     -26687,    244,      TDA18250AAGC1_GAIN_Plus_7dB         }, \
   { -141464157,     -28576,    280,      TDA18250AAGC1_GAIN_Plus_10dB        }, \
   { -106945840,     -34895,    305,      TDA18250AAGC1_GAIN_Plus_13dB        }, \
   { -79208435,       -8696,     19,      TDA18250AAGC1_GAIN_Plus_16dB        }, \
   { -43828137,      -21667,    145,      TDA18250AAGC1_GAIN_Plus_19dB       }, \
   {   2842769,      -48852,    325,      TDA18250AAGC1_GAIN_Plus_22dB       }  \
}

#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC1NB_MASTER_LT_OFF_ST_ON          \
{                                                                                \
   { -157256753,      17527,   -268,      TDA18250AAGC1_GAIN_Minus_11dB   }, \
   { -131424170,      12492,   -223,      TDA18250AAGC1_GAIN_Minus_8dB    }, \
   {  -99878629,      -9616,     -6,      TDA18250AAGC1_GAIN_Minus_5dB    }, \
   {  -70569703,        343,   -123,      TDA18250AAGC1_GAIN_Minus_2dB    }, \
   {  -34899443,      12381,   -263,      TDA18250AAGC1_GAIN_Plus_1dB         }, \
   {   -8394208,      -7670,    -65,      TDA18250AAGC1_GAIN_Plus_4dB         }, \
   {   23527409,      -8799,    -51,      TDA18250AAGC1_GAIN_Plus_7dB         }, \
   {   54525144,     -10098,    -24,      TDA18250AAGC1_GAIN_Plus_10dB        }, \
   {   88588461,     -12752,    -41,      TDA18250AAGC1_GAIN_Plus_13dB        }, \
   {  115477980,      17416,   -364,      TDA18250AAGC1_GAIN_Plus_16dB        }, \
   {  149721600,      11315,   -308,      TDA18250AAGC1_GAIN_Plus_19dB       }, \
   {  194569980,      -3087,   -269,      TDA18250AAGC1_GAIN_Plus_22dB       }  \
}

#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC1NB_MASTER_LT_ON_ST_ON          \
{                                                                                \
   { -159939870,     18757,    -328,      TDA18250AAGC1_GAIN_Minus_11dB   }, \
   { -133171943,      9275,    -238,      TDA18250AAGC1_GAIN_Minus_8dB    }, \
   { -102226642,    -10213,     -51,      TDA18250AAGC1_GAIN_Minus_5dB    }, \
   { -72577822,      -3199,    -140,      TDA18250AAGC1_GAIN_Minus_2dB    }, \
   { -37130337,       9232,    -293,      TDA18250AAGC1_GAIN_Plus_1dB         }, \
   { -10339955,     -11934,     -98,      TDA18250AAGC1_GAIN_Plus_4dB         }, \
   {  21728243,     -14937,     -86,      TDA18250AAGC1_GAIN_Plus_7dB         }, \
   {  53095912,     -15887,     -51,      TDA18250AAGC1_GAIN_Plus_10dB        }, \
   {  87477003,     -19799,     -47,      TDA18250AAGC1_GAIN_Plus_13dB        }, \
   { 114890843,       8832,    -349,      TDA18250AAGC1_GAIN_Plus_16dB        }, \
   { 149495227,       1330,    -272,      TDA18250AAGC1_GAIN_Plus_19dB       }, \
   { 194827333,     -16445,    -186,      TDA18250AAGC1_GAIN_Plus_22dB       }  \
}

/* {    X     ,    X    ,   X  ,           A           },                */
/* {    0     ,    1    ,   2  ,           G           },                */
/* {          ,         ,      ,           C           },                */
/* {    x     ,    x    ,   x  ,           2           },                */
/* {    1     ,    1    ,   1  ,           N           },                */
/* {    0     ,    0    ,   0  ,           B           },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {          ,         ,   0  ,                       },                */
/* {          ,         ,      ,                       },                */
/* {          ,         ,      ,                       },                */
/* {          ,         ,      ,                       },                */
#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC2NB  \
{                                                    \
   { -262186,    2085,    -120,       10    },       \
   { -264194,    2079,    -120,       21    },       \
   { -266973,    2079,    -119,       32    },       \
   { -269833,    2078,    -116,       43    },       \
   { -272289,    2076,    -114,       54    },       \
   { -274733,    2070,    -99,        65    },       \
   { -277588,    2067,    -81,        76    },       \
   { -280562,    2063,    -62,        87    },       \
   { -282784,    2054,    -43,        98    },       \
   { -284701,    2043,    -30,       109    },       \
   { -284809,    2027,    -22,       120    },       \
   { -283553,    2007,    -14,       131    },       \
   {  -281681,    1984,     0,       142    }        \
}

/* {    X     ,    X    ,   X  ,           A           },                */
/* {    0     ,    1    ,   2  ,           G           },                */
/* {          ,         ,      ,           C           },                */
/* {    x     ,    x    ,   x  ,           3           },                */
/* {    1     ,    1    ,   1  ,           N           },                */
/* {    0     ,    0    ,   0  ,           B           },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {    0     ,    0    ,   0  ,                       },                */
/* {          ,         ,   0  ,                       },                */
/* {          ,         ,      ,                       },                */
/* {          ,         ,      ,                       },                */
/* {          ,         ,      ,                       },                */
#define TDA18250A_INSTANCE_CUSTOM_COMMON_LVL_AGC3NB						        \
{                                                                               \
   { -208116100,   -688,     2,    TDA18250AAGC3_GAIN_Minus_6dB      }, \
   { -178917030,   -732,     3,    TDA18250AAGC3_GAIN_Minus_3dB      }, \
   { -148438740,   -798,     3,    TDA18250AAGC3_GAIN_0dB            }, \
   { -118542815,   -777,     3,    TDA18250AAGC3_GAIN_Plus_3dB      }, \
   {  -88269478,   -778,     2,    TDA18250AAGC3_GAIN_Plus_6dB       }, \
   {  -58950548,   -606,     1,    TDA18250AAGC3_GAIN_Plus_9dB       }, \
   {  -29234591,   -457,     1,    TDA18250AAGC3_GAIN_Plus_12dB      }, \
   {          0,      0,     0,    TDA18250AAGC3_GAIN_Plus_15dB      }  \
}

/* Default configuration */
#define TDA18250A_CONFIG_CURPOWERSTATE_DEF tmPowerMax
#define TDA18250A_CONFIG_CURLLPOWERSTATE_DEF TDA18250A_PowerMax
#define TDA18250A_CONFIG_RF_DEF 0
#define TDA18250A_CONFIG_PROG_RF_DEF 0
#define TDA18250A_CONFIG_STANDARDMODE_DEF TDA18250A_StandardMode_Max

    /* Power settings */
#define TDA18250A_CONFIG_POWER_DEF_MASTER \
    TDA18250A_CONFIG_CURPOWERSTATE_DEF,                     /* curPowerState */       \
    TDA18250A_CONFIG_CURLLPOWERSTATE_DEF,                   /* curLLPowerState */     \
    {                                                       /* mapLLPowerState */     \
    TDA18250A_PowerNormalMode,                          /* tmPowerOn (D0) */      \
    TDA18250A_PowerStandbyWithLtOnWithXtalOn,           /* tmPowerStandby (D1) */ \
    TDA18250A_PowerStandbyWithLtOnWithXtalOn,           /* tmPowerSuspend (D2) */ \
    TDA18250A_PowerStandbyWithLtOnWithXtalOn            /* tmPowerOff (D3) */     \
}

#define TDA18250A_CONFIG_POWER_DEF_SLAVE \
    TDA18250A_CONFIG_CURPOWERSTATE_DEF,                     /* curPowerState */       \
    TDA18250A_CONFIG_CURLLPOWERSTATE_DEF,                   /* curLLPowerState */     \
    {                                                       /* mapLLPowerState */     \
    TDA18250A_PowerNormalMode,                          /* tmPowerOn (D0) */      \
    TDA18250A_PowerStandbyWithXtalOn,			        /* tmPowerStandby (D1) */ \
    TDA18250A_PowerStandbyWithXtalOn,					/* tmPowerSuspend (D2) */ \
    TDA18250A_PowerStandbyWithXtalOn					/* tmPowerOff (D3) */     \
}

    /* Standard Presets Aggregation: */
#define TDA18250A_CONFIG_STD_DEF_VCO_PULLING_MINUS \
    { \
    TDA18250A_CONFIG_STD_QAM_6MHZ_VCO_PULLING_MINUS,    \
    TDA18250A_CONFIG_STD_QAM_8MHZ_VCO_PULLING_MINUS,    \
    TDA18250A_CONFIG_STD_DVBT_1_7MHZ_VCO_PULLING_MINUS, \
    TDA18250A_CONFIG_STD_DVBT_6MHZ_VCO_PULLING_MINUS,   \
    TDA18250A_CONFIG_STD_DVBT_7MHZ_VCO_PULLING_MINUS,   \
    TDA18250A_CONFIG_STD_DVBT_8MHZ_VCO_PULLING_MINUS,   \
    TDA18250A_CONFIG_STD_DVBT_10MHZ_VCO_PULLING_MINUS,  \
    TDA18250A_CONFIG_STD_DTMB_8MHz_VCO_PULLING_MINUS,   \
    TDA18250A_CONFIG_STD_ISDBT_6MHZ_VCO_PULLING_MINUS,  \
}

#define TDA18250A_CONFIG_STD_DEF_VCO_PULLING_PLUS \
    { \
    TDA18250A_CONFIG_STD_QAM_6MHZ_VCO_PULLING_PLUS,    \
    TDA18250A_CONFIG_STD_QAM_8MHZ_VCO_PULLING_PLUS,    \
    TDA18250A_CONFIG_STD_DVBT_1_7MHZ_VCO_PULLING_PLUS, \
    TDA18250A_CONFIG_STD_DVBT_6MHZ_VCO_PULLING_PLUS,   \
    TDA18250A_CONFIG_STD_DVBT_7MHZ_VCO_PULLING_PLUS,   \
    TDA18250A_CONFIG_STD_DVBT_8MHZ_VCO_PULLING_PLUS,   \
    TDA18250A_CONFIG_STD_DVBT_10MHZ_VCO_PULLING_PLUS,  \
    TDA18250A_CONFIG_STD_DTMB_8MHz_VCO_PULLING_PLUS,   \
    TDA18250A_CONFIG_STD_ISDBT_6MHZ_VCO_PULLING_PLUS,  \
}

#define TDA18250A_CONFIG_REGMAP_DEF \
    { \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, \
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0           \
}


#define TDA18250A_CONFIG_MASTER \
    TDA18250A_CONFIG_POWER_DEF_MASTER, \
    TDA18250A_CONFIG_RF_DEF,                                /* uRF */                            \
    TDA18250A_CONFIG_PROG_RF_DEF,                           /* uProgRF */                        \
    TDA18250A_CONFIG_STANDARDMODE_DEF,                      /* StandardMode */                   \
    Null,                                                   /* pStandard */                      \
    False,                                                  /* bBufferMode */                    \
    False,                                                  /* Manual PLL Calc */                \
    TDA18250A_LoopThrough_Enabled,                          /* single app 18274 */               \
    TDA18250A_Mode_Dual,                                    /* single app */                     \
    TDA18250A_XtalFreq_16000000,                            /* eXtalFreq */                      \
    TDA18250A_XTOUT_Amp_400mV,                              /* eXTOutAmp */                      \
    TDA18250A_XTOUT_Sinusoidal,                             /* eXTOut */                         \
    TDA18250A_LNA_RANGE_Minus8dB_Plus22dB,					/* eLNA_Range */                     \
    TDA18250A_IRQ_POLARITY_LOW,                             /* eIRQPolarity */                   \
    TDA18250A_HwState_InitNotDone,                          /* eHwState */                       \
    TDA18250A_CONFIG_STD_DEF_VCO_PULLING_MINUS, \
    TDA18250A_CONFIG_REGMAP_DEF

#define TDA18250A_CONFIG_SLAVE \
    TDA18250A_CONFIG_POWER_DEF_SLAVE, \
    TDA18250A_CONFIG_RF_DEF,                                /* uRF */                            \
    TDA18250A_CONFIG_PROG_RF_DEF,                           /* uProgRF */                        \
    TDA18250A_CONFIG_STANDARDMODE_DEF,                      /* StandardMode */                   \
    Null,                                                   /* pStandard */                      \
    True,                                                   /* bBufferMode */                    \
    False,                                                  /* Manual PLL Calc */                \
    TDA18250A_LoopThrough_Disabled,                         /* single app 18274 */               \
    TDA18250A_Mode_Dual,                                    /* single app */                     \
    TDA18250A_XtalFreq_16000000,                            /* eXtalFreq */                      \
    TDA18250A_XTOUT_Amp_400mV,                              /* eXTOutAmp */                      \
    TDA18250A_XTOUT_Sinusoidal,                             /* eXTOut */                         \
    TDA18250A_LNA_RANGE_Minus8dB_Plus22dB,					/* eLNA_Range */                     \
    TDA18250A_IRQ_POLARITY_LOW,                             /* eIRQPolarity */                   \
    TDA18250A_HwState_InitNotDone,                          /* eHwState */                       \
    TDA18250A_CONFIG_STD_DEF_VCO_PULLING_PLUS, \
    TDA18250A_CONFIG_REGMAP_DEF


    /* Custom Driver Instance Parameters: (Path 0) */
#define TDA18250A_CONFIG_0 \
    TDA18250A_CONFIG_MASTER

    /* Custom Driver Instance Parameters: (Path 1) */
#define TDA18250A_CONFIG_1 \
    TDA18250A_CONFIG_SLAVE


#ifdef __cplusplus
}
#endif

#endif /* _TDA18250A_CONFIG_COMMON_H */


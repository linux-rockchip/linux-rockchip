#ifndef ANALOG_H
#define ANALOG_H

#define	AnalogVideo_None		0x00000000 // This is a digital sensor
#define	AnalogVideo_NTSC_M		0x00000001 // 75 IRE Setup
#define	AnalogVideo_NTSC_M_J	0x00000002 // Japan, 0 IRE Setup
#define	AnalogVideo_NTSC_433	0x00000004
#define	AnalogVideo_PAL_B		0x00000010
#define	AnalogVideo_PAL_D		0x00000020
#define	AnalogVideo_PAL_G		0x00000040
#define	AnalogVideo_PAL_H		0x00000080
#define	AnalogVideo_PAL_I		0x00000100
#define	AnalogVideo_PAL_M		0x00000200
#define	AnalogVideo_PAL_N		0x00000400
#define	AnalogVideo_PAL_60		0x00000800
#define	AnalogVideo_SECAM_B		0x00001000
#define	AnalogVideo_SECAM_D		0x00002000
#define AnalogVideo_SECAM_G		0x00004000
#define	AnalogVideo_SECAM_H		0x00008000
#define	AnalogVideo_SECAM_K		0x00010000
#define	AnalogVideo_SECAM_K1	0x00020000
#define	AnalogVideo_SECAM_L		0x00040000
#define	AnalogVideo_SECAM_L1	0x00080000
#define	AnalogVideo_PAL_N_COMBO 0x00100000
#define AnalogVideo_PAL_B_NICAM 0x00200000
#define AnalogVideo_PAL_G_NICAM 0x00400000
#define AnalogVideo_PAL_I1      0x00800000
#define AnalogRadio_FM          0x40000000

#define	AnalogVideo_NTSC_Mask	0x00000007
#define	AnalogVideo_PAL_Mask	0x00F00FF0
#define	AnalogVideo_SECAM_Mask	0x000FF000

#define TVAUDIO_MODE_MONO		0x00000001
#define TVAUDIO_MODE_STEREO     0x00000002
#define TVAUDIO_MODE_LANG_A     0x00000010
#define TVAUDIO_MODE_LANG_B     0x00000020
#define TVAUDIO_MODE_LANG_C     0x00000040

#define CTRY_DEFAULT 0
#define CTRY_ALBANIA 355 // Albania
#define CTRY_ALGERIA 213 // Algeria
#define CTRY_ARGENTINA 54 // Argentina
#define CTRY_ARMENIA 374 // Armenia
#define CTRY_AUSTRALIA 61 // Australia
#define CTRY_AUSTRIA 43 // Austria
#define CTRY_AZERBAIJAN 994 // Azerbaijan
#define CTRY_BAHRAIN 973 // Bahrain
#define CTRY_BELARUS 375 // Belarus
#define CTRY_BELGIUM 32 // Belgium
#define CTRY_BELIZE 501 // Belize
#define CTRY_BOLIVIA 591 // Bolivia
#define CTRY_BRAZIL 55 // Brazil
#define CTRY_BRUNEI_DARUSSALAM 673 // Brunei Darussalam
#define CTRY_BULGARIA 359 // Bulgaria
#define CTRY_CANADA 2 // Canada
#define CTRY_CARIBBEAN 1 // Caribbean
#define CTRY_CHILE 56 // Chile
#define CTRY_COLOMBIA 57 // Colombia
#define CTRY_COSTA_RICA 506 // Costa Rica
#define CTRY_CROATIA 385 // Croatia
#define CTRY_CZECH 420 // Czech Republic
#define CTRY_DENMARK 45 // Denmark
#define CTRY_DOMINICAN_REPUBLIC 1 // Dominican Republic
#define CTRY_ECUADOR 593 // Ecuador
#define CTRY_EGYPT 20 // Egypt
#define CTRY_EL_SALVADOR 503 // El Salvador
#define CTRY_ESTONIA 372 // Estonia
#define CTRY_FAEROE_ISLANDS 298 // Faeroe Islands
#define CTRY_FINLAND 358 // Finland
#define CTRY_FRANCE 33 // France
#define CTRY_GEORGIA 995 // Georgia
#define CTRY_GERMANY 49 // Germany
#define CTRY_GREECE 30 // Greece
#define CTRY_GUATEMALA 502 // Guatemala
#define CTRY_HONDURAS 504 // Honduras
#define CTRY_HONG_KONG 852 // Hong Kong S.A.R., P.R.C.
#define CTRY_HUNGARY 36 // Hungary
#define CTRY_ICELAND 354 // Iceland
#define CTRY_INDIA 91 // India
#define CTRY_INDONESIA 62 // Indonesia
#define CTRY_IRAN 981 // Iran
#define CTRY_IRAQ 964 // Iraq
#define CTRY_IRELAND 353 // Ireland
#define CTRY_ISRAEL 972 // Israel
#define CTRY_ITALY 39 // Italy
#define CTRY_JAMAICA 1 // Jamaica
#define CTRY_JAPAN 81 // Japan
#define CTRY_JORDAN 962 // Jordan
#define CTRY_KAZAKSTAN 7 // Kazakstan
#define CTRY_KENYA 254 // Kenya
#define CTRY_KUWAIT 965 // Kuwait
#define CTRY_KYRGYZSTAN 996 // Kyrgyzstan
#define CTRY_LATVIA 371 // Latvia
#define CTRY_LEBANON 961 // Lebanon
#define CTRY_LIBYA 218 // Libya
#define CTRY_LIECHTENSTEIN 41 // Liechtenstein
#define CTRY_LITHUANIA 370 // Lithuania
#define CTRY_LUXEMBOURG 352 // Luxembourg
#define CTRY_MACAU 853 // Macau S.A.R., PRC
#define CTRY_MACEDONIA 389 // Former Yugoslav Republic of Macedonia
#define CTRY_MALAYSIA 60 // Malaysia
#define CTRY_MALDIVES 960 // Maldives
#define CTRY_MEXICO 52 // Mexico
#define CTRY_MONACO 33 // Principality of Monaco
#define CTRY_MONGOLIA 976 // Mongolia
#define CTRY_MOROCCO 212 // Morocco
#define CTRY_NETHERLANDS 31 // Netherlands
#define CTRY_NEW_ZEALAND 64 // New Zealand
#define CTRY_NICARAGUA 505 // Nicaragua
#define CTRY_NORWAY 47 // Norway
#define CTRY_OMAN 968 // Oman
#define CTRY_PAKISTAN 92 // Islamic Republic of Pakistan
#define CTRY_PANAMA 507 // Panama
#define CTRY_PARAGUAY 595 // Paraguay
#define CTRY_PERU 51 // Peru
#define CTRY_PHILIPPINES 63 // Republic of the Philippines
#define CTRY_POLAND 48 // Poland
#define CTRY_PORTUGAL 351 // Portugal
#define CTRY_PRCHINA 86 // People's Republic of China
#define CTRY_PUERTO_RICO 1 // Puerto Rico
#define CTRY_QATAR 974 // Qatar
#define CTRY_ROMANIA 40 // Romania
#define CTRY_RUSSIA 7 // Russia
#define CTRY_SAUDI_ARABIA 966 // Saudi Arabia
#define CTRY_SERBIA 381 // Serbia
#define CTRY_SINGAPORE 65 // Singapore
#define CTRY_SLOVAK 421 // Slovak Republic
#define CTRY_SLOVENIA 386 // Slovenia
#define CTRY_SOUTH_AFRICA 27 // South Africa
#define CTRY_SOUTH_KOREA 82 // Korea
#define CTRY_SPAIN 34 // Spain
#define CTRY_SWEDEN 46 // Sweden
#define CTRY_SWITZERLAND 41 // Switzerland
#define CTRY_SYRIA 963 // Syria
#define CTRY_TAIWAN 886 // Taiwan
#define CTRY_TATARSTAN 7 // Tatarstan
#define CTRY_THAILAND 66 // Thailand
#define CTRY_TRINIDAD_Y_TOBAGO 1 // Trinidad y Tobago
#define CTRY_TUNISIA 216 // Tunisia
#define CTRY_TURKEY 90 // Turkey
#define CTRY_UAE 971 // U.A.E.
#define CTRY_UKRAINE 380 // Ukraine
#define CTRY_UNITED_KINGDOM 44 // United Kingdom
#define CTRY_UNITED_STATES 1 // United States
#define CTRY_URUGUAY 598 // Uruguay
#define CTRY_UZBEKISTAN 7 // Uzbekistan
#define CTRY_VENEZUELA 58 // Venezuela
#define CTRY_VIET_NAM 84 // Viet Nam
#define CTRY_YEMEN 967 // Yemen
#define CTRY_ZIMBABWE 263 // Zimbabwe


#endif

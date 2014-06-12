#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <iostream>

#include "api\IT9133.h"
using namespace std;

StreamType streamType = StreamType_DVBT_PARALLEL;
extern Handle I2c_handle;


DefaultDemodulator demod = {
	NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    20480,
    20480000,
    StreamType_DVBT_PARALLEL,
    8000,
    642000,
    0x00000000,
	{False, False, 0, 0},
    0,
    False,
    False,
	0,
	User_I2C_ADDRESS,
	False
};

void Example_initialize (Demodulator *demod) {
    Dword error = Error_NO_ERROR;
    Dword start = 0, end = 0;

    //Initialize with StreamType (StreamType_DVBT_PARALLEL or StreamType_DVBT_SERIAL)
    start = GetTickCount();
    error = Demodulator_initialize (demod, streamType);
    end = GetTickCount();
	if (error){
		printf("Initialize failed.0x%08x\n", error);
		return;
	}
    printf("Initialize Time: %.02fsec\n", ((double)(end-start))/1000);
    printf("Initialize successful.\n");
}

void Example_finalize (Demodulator *demod) {
    Dword error = Error_NO_ERROR;

    //Finialize demodulator and Tuner when device is off
    error = Demodulator_finalize (demod);
	if (error){
		printf("Finalize failed.0x%08x\n", error);
		return;
	}

    printf("Finalize successful.\n");
}
void Example_OnOffPowerControl (Demodulator *demod, Byte bOn) {
    Dword error = Error_NO_ERROR;

    //Enable Power control function
    error = Demodulator_enableControlPowerSaving(demod, bOn);
    if (error) goto exit;        

exit:
    if (error) printf("Error = 0x%08x\n", error);
    else printf("Power Control is %s Successfully! \n", bOn?"On":"Off");
}

void Example_PowerControl (Demodulator *demod, Byte enable) {
    Dword error = Error_NO_ERROR;

    error = Demodulator_controlPowerSaving(demod, enable);
    if (error) goto exit;        

exit:
    if (error) printf("Error = 0x%08x\n", error);
    else printf("Power Control %s Successfully! \n", enable?"Enable":"Disable");
}

void Example_acquireChannel (Demodulator *demod, Word bandwidth, Dword freq) {
    Dword error = Error_NO_ERROR;
    Bool locked;
    Bool found = False;

    //Set Channel
    error = Demodulator_acquireChannel (demod, bandwidth, freq);
    if (error) goto exit;
    
    //Wait demodulator ready and Check if Locked
    error = Demodulator_isLocked (demod, &locked);
    if (error) goto exit;

    printf("Channel is %slocked!\n", locked?"":"not ");

exit:
    if (error) printf("Error = 0x%08x\n", error);
}

void Example_monitorVersion (Demodulator *demod) {
    Dword error = Error_NO_ERROR;
	Dword version = 0;

	printf("API Version = %04x.%08x.%02x\n", Version_NUMBER, Version_DATE, Version_BUILD);

	error = Demodulator_getFirmwareVersion (demod, Processor_LINK, &version);
	if (error) {
		printf("Demodulator_getFirmwareVersion(LINK) failed! Error = 0x%08x\n", error);
		return;
	} else {
		printf("LINK FW Version = 0x%08x\n", version);
	}
	error = Demodulator_getFirmwareVersion (demod, Processor_OFDM, &version);
	if (error) {
		printf("Demodulator_getFirmwareVersion(OFDM) failed! Error = 0x%08x\n", error);
		return;
	} else {
		printf("OFDM FW Version = 0x%08x\n", version);
	}
}

void Example_monitorStatistic (Demodulator *demod) {
    
    Dword error = Error_NO_ERROR;
    Statistic stat;
    Dword postErrorCount=0;
    Dword postBitCount=0;
    Word  abortCount=0;
	Word  cnt=0;
    Long dbm = 0;
    
	do{
		error = Demodulator_getStatistic(demod, &stat);
		if (error) {
			printf("Demodulator_getStatistic failed!Error = 0x%08x\n", error);
			break;
		} else {
			printf("Signal Present = %s, Locked = %s\n", stat.signalPresented?"TRUE":"FALSE", stat.signalLocked?"TRUE":"FALSE");
			printf("Signal Quality = %d, Strength = %d\n", stat.signalQuality, stat.signalStrength);
		}

		error = Demodulator_getSignalStrengthDbm(demod, 18, 18, &dbm);
		if (error) {
			printf("Demodulator_getSignalStrengthDbm failed!Error = 0x%08x\n", error);
			break;
		} else {
			printf("Signal Strength = %d(DBm)\n", dbm);
		}   

		error = Demodulator_getPostVitBer (demod, &postErrorCount, &postBitCount, &abortCount);
		if (error) {
			printf("Demodulator_getPostVitBer failed!Error = 0x%08x\n", error);
			break;
		} else {
			if(postErrorCount){ 
				while(postErrorCount<postBitCount){
					postErrorCount = postErrorCount*10;
					cnt++;
				}
			}
			printf("BER = %.03f+E%d, Abort Cnt  = %d\n", (double)postErrorCount/(double)postBitCount,cnt, abortCount);
		}

	}while(FALSE);
}


int main(int argc, char *argv[])
{
    char c = 0;
    Dword val = 0;
    Dword reg = 0;
    Dword processor = 0;
    Dword freq = 0;
	Dword bw = 6000;
    Dword error = 0;
    Dword i2caddr = 0;
    
    printf ("Please wait for initialize complete .....\n");
    
	/** Initialize device when it is first used */
    //Example_initialize ((Demodulator*)&demod);                


	while (1) {
        printf ("i : initialize chips\n");
        printf ("w : write register\n");
        printf ("r : read register\n");
        printf ("a : acquire channels\n");
        printf ("m : monitor statistic\n");
        printf ("v : monitor version\n");
        printf ("s : Enable/Disable Device Power Control\n");
        printf ("p : Device Power Control\n");
        printf ("x : exit\n");
        printf ("Please enter your command:\n");
        c = _getch();

        switch (c) {
            case 'i' :
            case 'I' :
                /**
                 * DVB-T: Initialize chip.
                 */
                printf ("i2c address?(HEX)\n");
                scanf ("%02X", &i2caddr);
                demod.demodAddr = i2caddr;
                printf ("i2caddr = %02X\n", i2caddr);
                
                Example_initialize ((Demodulator*)&demod);                
                break;
            case 'w' :
            case 'W' :                
				printf ("i2c address?(HEX)\n");
                scanf ("%02X", &i2caddr);
                demod.demodAddr = i2caddr;
                printf ("i2caddr = %02X\n", i2caddr);
                
                printf ("which processor to write? 0: LINK, 1:OFDM\n");
                scanf ("%d", &processor);

                printf ("which register to write?(HEX)\n");
                scanf ("%08X", &reg);

                printf ("value?(HEX)\n");
                scanf ("%02X", &val);

                if (processor == 0)
                {
                    error = Demodulator_writeRegister((Demodulator*)&demod, Processor_LINK, reg, (Byte)val);
                    if (error) printf("write register(link) failed! Error = 0x%08x\n", error);
                }
                else
                {
                    error = Demodulator_writeRegister((Demodulator*)&demod, Processor_OFDM, reg, (Byte)val);
                    if (error) printf("write register(ofdm) failed! Error = 0x%08x\n", error);
                }
                break;
            case 'r' :
            case 'R' :
                printf ("which processor to read? 0: LINK, 1:OFDM\n");
                scanf ("%d", &processor);

                printf ("which register to read?\n");
                scanf ("%08X", &reg);

                if (processor == 0)
                {
                    error = Demodulator_readRegister((Demodulator*)&demod, Processor_LINK, reg, (Byte *)&val);
                    if (error) printf("read register failed! Error = %08x\n", error);
                    else printf ("value = %02x\n", (val & 0x000000FF));
                }
                else
                {
                    error = Demodulator_readRegister((Demodulator*)&demod, Processor_OFDM, reg, (Byte *)&val);
                    if (error) printf("read register failed! Error = %08x\n", error);
                    else printf ("value = %02x\n", (val & 0x000000FF));
                }
                break;
            case 'a' :
            case 'A' :
                /**
                 * DVB-T: Use Demodulator_acquireChannel to tune channel to specified
                 *        frequency and bandwidth.
                 */
                printf ("what channel to acquire? (KHz)\n");
                scanf ("%d", &freq);
                printf ("what BW to acquire? (KHz)\n");
                scanf ("%d", &bw);
                Example_acquireChannel ((Demodulator*)&demod, (Word)bw, freq);
                break;
            case 'm' :
            case 'M' :
                /**
                 * DVB-T: Monitor the statistics of chip.               
                 */
                Example_monitorStatistic ((Demodulator*)&demod);
                break;
            case 'v' :
            case 'V' :
                /**
                 * DVB-T: Monitor the Version of chip.               
                 */
                Example_monitorVersion ((Demodulator*)&demod);
                break;
            case 'p' :
            case 'P' :
				printf ("Power off or not? 0: off, 1:On\n");
                scanf ("%d", &bw);
                Example_PowerControl((Demodulator*)&demod, (Byte)bw);
                break;
            case 's' :
            case 'S' :
				printf ("Power Control enable or disable? 0: disable, 1:Enable\n");
                scanf ("%d", &bw);
                Example_OnOffPowerControl((Demodulator*)&demod, (Byte)bw);
                break;                 
            case 'x' :
            case 'X' :
                goto exit;
        }
    }

exit :
	Example_finalize((Demodulator*)&demod);
    /** Unload Inpout32.dll */
    if(I2c_handle){
        FreeLibrary( (HINSTANCE)I2c_handle );
    }
    system("PAUSE");
    return EXIT_SUCCESS;
}

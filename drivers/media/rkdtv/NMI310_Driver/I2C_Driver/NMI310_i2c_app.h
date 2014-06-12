#ifndef __NMI310_I2C_HHHHHHHHHH
#define __NMI310_I2C_HHHHHHHHHH

//////////////////////////////////////
//include the .h file here


int NPM_bus_write(unsigned int  adr, unsigned char *b, unsigned int sz);
int NPM_bus_read(unsigned int adr, unsigned char *b, unsigned int  sz);

#endif


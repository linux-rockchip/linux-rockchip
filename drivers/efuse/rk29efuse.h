#if (CONFIG_RK_EFUSE)
//parm:   pdata:output buffer,buffer length 128Byte
//return: 0 is success,other error.
//note :   buffer must be >= 128Byte
 int efuse_read(uint8_t *pdata);
#endif

/*
				Copyright (c) 2006 Siano Mobile Silicon Ltd. All rights reserved
				
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  

*/
#if 0
#include "osw.h"
#include <string.h>
#if 1
unsigned char	g_Cwd[1];
#else
unsigned char   g_Cwd[MAX_DIR_NAME_SIZE];

UINT32 OSW_FS_Init(UINT32 arg)
{
	ARGUSE(arg);

	memset(g_Cwd, 0, MAX_DIR_NAME_SIZE);

	return OSW_OK;
}
#endif
UINT32 OSW_FS_SetCwd(const char* cwd)
{
	if (cwd == NULL)
	{
		return OSW_OK;
	}

	if (memcpy( (void*)g_Cwd, (void*) cwd, (UINT32)strlen(cwd)) != NULL)
	{
		return OSW_OK;
	}
	else
	{
		return OSW_ERROR;
	}
}

OSW_FILEHANDLE OSW_FS_Open(const char* filename, const char* attributes)
{
	OSW_FILEHANDLE		hFile;
	size_t srcLen = strlen(filename);
	size_t cwdLen = strlen((char*)g_Cwd);
	unsigned char* dstString = (unsigned char*)OSW_MemAlloc( (UINT32)(srcLen + cwdLen + 2) );

	if (dstString != NULL)
	{
        strcpy((char *)dstString, (char*)g_Cwd);
		strcat((char *)dstString, "/");
		strcat((char *)dstString, filename);
        hFile = fopen((char*)dstString, attributes);

		OSW_MemFree((void *)dstString);

        return hFile;
	}
	else
	{
		return NULL;
	}
}

UINT32 OSW_FS_Close(OSW_FILEHANDLE hFile)
{
	return fclose(hFile);
}

UINT32 OSW_FS_Write(OSW_FILEHANDLE hFile, void* pBuffer, UINT32 buffLen)
{
	size_t size = fwrite(pBuffer, sizeof(unsigned char), buffLen, hFile);

	return (UINT32)size;
}

UINT32 OSW_FS_Read(OSW_FILEHANDLE hFile, void* pBuffer, UINT32 buffLen)
{
	size_t size = fread(pBuffer, sizeof(unsigned char), buffLen, hFile);

	return (UINT32)size;
}

UINT32 OSW_FS_Delete(const char* filename)
{
	UINT32 retCode = OSW_ERROR;
    OSW_FILEHANDLE		hFile;

	size_t srcLen = strlen(filename);
	size_t cwdLen = strlen((char*)g_Cwd);
	unsigned char* dstString = (unsigned char*)OSW_MemAlloc( (UINT32)(srcLen + cwdLen + 2) );

	if (dstString != NULL)
	{
        strcpy((char *)dstString, (char*)g_Cwd);
		strcat((char *)dstString, "/");
		strcat((char *)dstString, filename);
        hFile = fopen((char*)dstString, "wb");
        if (hFile != 0)
            fclose(hFile);
		if (hFile == 0)
		{
			retCode = OSW_ERROR;
		}
		else
		{
			retCode = OSW_OK;
		}

		OSW_MemFree((void *)dstString);
		
	}
	return (UINT32) retCode;
}

#ifdef SMS_OSW_FS_EXTENSIONS

UINT32 OSW_FS_Tell(	OSW_FILEHANDLE	hFile)
{
	return ftell(hFile); 
}



UINT32 OSW_FS_Printf(OSW_FILEHANDLE	hFile, const char *fmt, ...)
{

	UINT32 NumWritten; 
	va_list Args;

	va_start(Args, fmt);
	NumWritten = vfprintf(hFile, fmt, Args);
	va_end(Args);

	return NumWritten; 
}



BOOL OSW_FS_Flush(OSW_FILEHANDLE hFile)
{
	return fflush(hFile) == 0; 
}


BOOL OSW_FS_Seek(OSW_FILEHANDLE hFile, UINT32 offset, INT32 whence)
{
	const int WHENCE_VALS[] = {SEEK_SET, SEEK_CUR, SEEK_END};  

	return fseek(hFile, offset, WHENCE_VALS[whence]) == 0; 
}


BOOL OSW_FS_Putc(OSW_FILEHANDLE hFile, INT32 c)
{
	return fputc(c, hFile) != EOF; 
}


OSW_FILEHANDLE OSW_FS_Dopen(INT32 fd, const char *attributes)
{
	return (OSW_FILEHANDLE)fdopen(fd, attributes); 
}



void OSW_FS_ClearErr(OSW_FILEHANDLE hFile)
{
	clearerr(hFile); 
}

#endif
#endif //jan

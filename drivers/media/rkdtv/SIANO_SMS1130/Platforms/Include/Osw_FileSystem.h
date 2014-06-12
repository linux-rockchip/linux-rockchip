/*!  

	\file		Osw_FileSystem.h
	
	\brief		OS Wrapper - File System abstraction
	
	\par		Copyright (c) 2006 Siano Mobile Silicon Ltd. All rights reserved	
	
				PROPRIETARY RIGHTS of Siano Mobile Silicon are involved in the 
				subject matter of this material.  All manufacturing, reproduction, 
				use, and sales rights pertaining to this subject matter are governed 
				by the license agreement.  The recipient of this software implicitly 
				accepts the terms of the license.	  

*/

#ifndef __OSW_FS_H
#define __OSW_FS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../../include_internal/Include/sms_common.h"

/*!
	Initialize the OSW_FS local variables

	\param[in]  arg        Initialization parameter
	\return                OSW_OK if OK - or a different value in case of error
*/
UINT32 OSW_FS_Init( UINT32 arg );

/*!
	Set the OSW_FS Sandbox directory

	\param[in]  cwd		New working directory
	
	\return             OSW_OK if OK - or a different value in case of error
*/

UINT32 OSW_FS_SetCwd( const char* cwd );

/*!
	Open a file using the given attributes

	\param[in]  filename	The name of the file to open							
	\param[in]	attributes	The attributes to use when opening the file
	
	\return					Handle to the opened file. NULL if file not opened.
*/

OSW_FILEHANDLE OSW_FS_Open(	const char* filename,
					   	const char* attributes );

/*!
	Close the file

	\param[in]  hFile	Handle to the file to close
	\return             Zero if OK - and your OS unique error code number otherwise
*/

UINT32 OSW_FS_Close(OSW_FILEHANDLE hFile);

/*!
	Write buffer to file

	\param[in]  hFile	Handle to the file to write to
	\param[in]	pBuffer	Pointer to the buffer containing the bytes to write
	\param[in]	buffLen	The number of bytes to write
	
	\return     Returns the number of bytes actually written
*/

UINT32 OSW_FS_Write(OSW_FILEHANDLE hFile,
					void*      pBuffer,
					UINT32	   buffLen);

/*!
	Read bytes from the file to a buffer

	\param[in]  hFile	Handle to the file to read from
	\param[out]	pData	Pointer to the buffer to write to
	\param[in]	dataLen	Number of bytes to read
	
	\return    Returns the actual number of bytes read
*/
UINT32 OSW_FS_Read(	OSW_FILEHANDLE	hFile,
				   	void*   	pData,
				   	UINT32		dataLen);


/*!
	Delete the file

	\param[in]  filename	The name of the file to delete
	
	\return		Returns OSW_OK if OK, or OSW_ERROR in case of error
*/
UINT32 OSW_FS_Delete(const char* filename);




//File system abstraction extensions declarations. 
//Platform dependent implementation of these functions is optional. 
//When SMS_OSW_FS_EXTENSIONS compilation flag is set, platform dependent code must define these functions. 
//When SMS_OSW_FS_EXTENSIONS compilation flag is not set, hostlib will be linked with stubs.
//Implementing the extensions is required for the imported zlib code to work 
//(currently used only by CMMB ESG parser). 


/*!
Tells file position.

\param[in]  hFile	Handle to the file to read from

\return    Returns file position. 
*/
UINT32 OSW_FS_Tell(	OSW_FILEHANDLE	hFile);


/*!
Outputs formated string to a file.

\param[in]  hFile	Handle to the file to write to
\param[in]  fmt	    Format string

\return    Returns number of characters printed to file. 
*/
UINT32 OSW_FS_Printf(OSW_FILEHANDLE	hFile, const char *fmt, ...);


/*!
Flushes a file.

\param[in]  hFile	Handle to the file.

\return    Returns success indication. 
*/
BOOL OSW_FS_Flush(OSW_FILEHANDLE hFile);

/*!
Moves to a specified position in the file.

\param[in]  hFile	Handle to the file.
\param[in]  offset Offset in the file.
\param[in]  whence (OSW_FS_SEEK_SET/OSW_FS_SEEK_CUR/OSW_FS_SEEK_END)

\return    Returns success indication. 
*/
#define OSW_FS_SEEK_SET 0
#define OSW_FS_SEEK_CUR 1
#define OSW_FS_SEEK_END 2
BOOL OSW_FS_Seek(OSW_FILEHANDLE hFile, UINT32 offset, INT32 whence);

/*!
Writes single character to a file.

\param[in]  hFile	Handle to the file.
\param[in]  c	    Character to write.

\return    Returns success indication. 
*/
BOOL OSW_FS_Putc(OSW_FILEHANDLE hFile, INT32 c);

/*!
Opens a file from a Unix-compliant file handle.

\param[in]  fd	        Unix-compliant file handle.
\param[in]  attributes	File attributes. 

\return    Returns success indication. 
*/
OSW_FILEHANDLE OSW_FS_Dopen(INT32 fd, const char *attributes); 



/*!
Clears end-of file error indication from a file.

\param[in]  hFile	Handle to the file.

*/
void OSW_FS_ClearErr(OSW_FILEHANDLE hFile);


#ifdef __cplusplus
}
#endif
#endif


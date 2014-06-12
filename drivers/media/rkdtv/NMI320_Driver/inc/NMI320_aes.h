/****************************************************************************************
File: AES.h

Description: Implementation of the AES Algorithm.

Component: Media FLO Transport Layer.

Module: Stream Encryption Layer

Description:  

Author: Samer Sarhan

Date: 12/06/2007.

Comment: reference: FIPS-197 Advaced Encryption Standard (AES).

Version: 1.0
****************************************************************************************/


#ifndef _AES_H_
#define _AES_H_

#include "include.h"
//typedef unsigned long	uint32_t;
//typedef unsigned short	uint16_t;
//typedef unsigned char	uint8_t;

typedef unsigned char	bool_t;

#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

typedef uint32_t word;
typedef uint8_t byte;

// Either AES_128, AES_192 or AES_256 is defined at the compiler 
// preprocessor flags.
#if !defined ( AES_128 ) && !defined ( AES_192 ) && !defined ( AES_256 )
#define AES_128
#endif

#if defined ( AES_128 )
#define	Nk	(4)
#define Nb	(4)
#define Nr	(10) 
#elif  defined ( AES_192 )
#define	Nk	(6)
#define Nb	(4)
#define Nr	(12) 
#elif  defined ( AES_256 )
#define	Nk	(8)
#define Nb	(4)
#define Nr	(14)
#endif
/******************************************************************************
Function: AES_ECB_Encrypt

Description: 
Encrypts an input plain text buffer using a certain key.

NOTE: 

Return: none.

Author: Samer Sarhan

Version: 1.0

Date: 12/06/2007
******************************************************************************/
/*!Description:  Encrypts an input plain text buffer using a certain key. */
void AES_ECB_Encrypt(
					 /*!
					 The encryption key of length 16, 24 or  32 bytes for modes AES-128,
					 AES-192 or AES-256 repectively.
					 */
					 uint8_t key[4*Nk], 
					 /*! The input buffer of plain text of  arbitrary length. */
					 const uint8_t * in, 
					 /*! The output cipher text buffer of the SAME length as the input buffer. */
					 uint8_t * out, 
					 /*! 
					 Length of the input/output buffers. Length must be 
					 greater than one block size, i.e. greater than 16 Bytes  
					 otherwise an assertion error happens. 
						*/
						uint32_t length);

/******************************************************************************
Function: AES_ECB_Decrypt

Description: 
Decrypts an input cipher text buffer using a certain key.

NOTE: 

Return: none.

Author: Samer Sarhan

Version: 1.0

Date: 12/06/2007
******************************************************************************/
/*!Description: Decrypts an input cipher text buffer using a certain key. */
void AES_ECB_Decrypt(
					 /*!
					 The encryption key of length 16, 24 or  32 bytes for modes AES-128,
					 AES-192 or AES-256 repectively.
					 */
					 uint8_t key[4*Nk], 
					 /*! The input buffer of cipher text of arbitrary length. */
					 const uint8_t * in, 
					 /*! The output plain text buffer of the SAME length as the input buffer. */
					 uint8_t * out, 
					 /*! 
					 Length of the input/output buffers. Length must be 
					 greater than one block size, i.e. greater than 16 Bytes  
					 otherwise an assertion error happens. 
						*/
						uint32_t length);

/******************************************************************************
Function: AES_CTR_EncDec

Description: 
Encrypts/decrypts an input plain/cipher text buffer using a 
certain key and a 32*Nb (128, 192 or 256) bit counter.

NOTE: 

Return: none.

Author: Samer Sarhan

Version: 1.0

Date: 12/06/2007
******************************************************************************/
/*!Description: 
Encrypts/decrypts an input plain/cipher text buffer using a 
certain key and a 32*Nb (128, 192 or 256) bit counter.
*/
void AES_CTR_EncDec(
					/*!
					The encryption key of length 16, 24 or  32 bytes for 
					modes AES-128, AES-192 or AES-256 repectively.
					*/
					uint8_t key[4*Nk], 
					/*!
					The CTR mode counter. Same initial value shall be used 
					for both encryption and decryption. 
					*/
					uint32_t counter[Nb], 
					/*! The input buffer of plain/cipher text of arbitrary length. */
					const uint8_t * in, 
					/*! 
					The output plain cipher/plain text buffer of the SAME 
					length as the input buffer. 
					*/
					uint8_t * out, 
					/*! 
					Length of the input/output buffers. Length must be 
					greater than one block size, i.e. greater than 16 Bytes  
					otherwise an assertion error happens. 
					*/
					uint32_t length);


#define AES_CTR_Encrypt(key, counter, in, out, length) \
	AES_CTR_EncDec(key, counter, in, out, length) 

#define AES_CTR_Decrypt(key, counter, in, out, length) \
	AES_CTR_EncDec(key, counter, in, out, length) 


// General: 
void IncCounter(uint32_t counter[Nb]);
void DecCounter(uint32_t counter[Nb]);
void AddRoundKey(byte state[4][Nb], word * w, uint32_t start, uint32_t end);
word SubWord(word a);
byte ModMultiply(byte a, byte b);
void GenericSubBytes(byte state[4][Nb], byte s_box[16][16]) ;
void KeyExpansion(byte key[4*Nk], word w[Nb*(Nr+1)]);
// Cipher: 
void Cipher(const byte *in/*[4*Nb]*/, byte *out/*[4*Nb]*/, word *w/*[Nb*(Nr+1)]*/);
void SubBytes(byte state[4][Nb]);
void ShiftRows(byte state[4][Nb]);
void MixColumns(byte state[4][Nb]);

// InvCipher: 
void InvCipher(const byte *in/*[4*Nb]*/, byte *out/*[4*Nb]*/, word *w/*[Nb*(Nr+1)]*/);
void InvSubBytes(byte state[4][Nb]) ;
void InvShiftRows(byte state[4][Nb]);
void InvMixColumns(byte state[4][Nb]);


#endif

/****************************************************************************
 *
 *      Copyright (c) DiBcom SA.  All rights reserved.
 *
 *      THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *      PURPOSE.
 *
 ****************************************************************************/

// Dta.h : main header file for the Dta application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

//#ifdef STANDARDSHELL_UI_MODEL
#include "resource.h"
//#endif

#include "DibExtDefines.h"

// CDtaApp:
// See Dta.cpp for the implementation of this class
//

class CDtaApp : public CWinApp
{
public:
	CDtaApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
public:
	afx_msg void OnBoard9080();
	afx_msg void OnBoard9090();
public:
	afx_msg void OnFileOpen();
public:
	afx_msg void OnAppAbout();
public:
	afx_msg void OnAppExit();
public:
	void InitBoard(DibBoardType BId);

	DECLARE_MESSAGE_MAP()
};

extern CDtaApp theApp;

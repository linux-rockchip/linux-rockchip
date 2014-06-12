// DtaDoc.cpp : implementation of the CDtaDoc class
//

#include "stdafx.h"
#include "Dta.h"

#include "DtaDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDtaDoc

IMPLEMENT_DYNCREATE(CDtaDoc, CDocument)

BEGIN_MESSAGE_MAP(CDtaDoc, CDocument)
END_MESSAGE_MAP()

// CDtaDoc construction/destruction

CDtaDoc::CDtaDoc()
{
	// TODO: add one-time construction code here

}

CDtaDoc::~CDtaDoc()
{
}

BOOL CDtaDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

// CDtaDoc serialization

#ifndef _WIN32_WCE_NO_ARCHIVE_SUPPORT
void CDtaDoc::Serialize(CArchive& ar)
{
	(ar);
}
#endif // !_WIN32_WCE_NO_ARCHIVE_SUPPORT


// CDtaDoc diagnostics

#ifdef _DEBUG
void CDtaDoc::AssertValid() const
{
	CDocument::AssertValid();
}
#endif //_DEBUG


// CDtaDoc commands


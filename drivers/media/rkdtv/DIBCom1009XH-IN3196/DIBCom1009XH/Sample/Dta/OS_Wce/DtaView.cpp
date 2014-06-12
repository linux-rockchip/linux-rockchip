// DtaView.cpp : implementation of the CDtaView class
//

#include "stdafx.h"
#include "Dta.h"

#include "DtaDoc.h"
#include "DtaView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDtaView

IMPLEMENT_DYNCREATE(CDtaView, CView)

BEGIN_MESSAGE_MAP(CDtaView, CView)
END_MESSAGE_MAP()

// CDtaView construction/destruction

CDtaView::CDtaView()
{
	// TODO: add construction code here

}

CDtaView::~CDtaView()
{
}

BOOL CDtaView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}


// CDtaView drawing
void CDtaView::OnDraw(CDC* /*pDC*/)
{
	CDtaDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}



// CDtaView diagnostics

#ifdef _DEBUG
void CDtaView::AssertValid() const
{
	CView::AssertValid();
}

CDtaDoc* CDtaView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDtaDoc)));
	return (CDtaDoc*)m_pDocument;
}
#endif //_DEBUG


// CDtaView message handlers

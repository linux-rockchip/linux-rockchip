// Dta.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Dta.h"
#include "MainFrm.h"
#include "DtaDoc.h"
#include "DtaView.h"

#ifdef __cplusplus 
extern "C" {
#endif

#include "OsTypes.h"
#include "OsDebug.h"
#include "DibExtDefines.h"
#include "DibExtAPI.h"
#include "DtaCmds.h"
#include "DtaMain.h"

char logfilepath[30];

#ifdef __cplusplus 
}
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// board type definition
DibBoardType BoardId = eBOARD_DEFAULT;

// CDtaApp

BEGIN_MESSAGE_MAP(CDtaApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CDtaApp::OnAppAbout)
	ON_COMMAND(ID_APP_EXIT, &CDtaApp::OnAppExit)
	ON_COMMAND(ID_FILE_OPEN, &CDtaApp::OnFileOpen)
	ON_COMMAND(ID_BOARD_9080, &CDtaApp::OnBoard9080)
	ON_COMMAND(ID_BOARD_9090, &CDtaApp::OnBoard9090)
END_MESSAGE_MAP()


// Overwrite of OnFileOpen
void CDtaApp::OnFileOpen()
{
	CFileDialog fileDlg (TRUE, NULL, NULL,
		OFN_FILEMUSTEXIST|OFN_EXPLORER, L"All Files (*.*)|*.*||");
	FILE * fb;
	char ffullname[100];
	char fname[30];
	char bcmd[100];

	// Display the file dialog. When user clicks OK, fileDlg.DoModal() 
   // returns IDOK.
   if( fileDlg.DoModal ()==IDOK )
   {

	   if (BoardId == eBOARD_DEFAULT)
	   {
		   MessageBox(NULL,L"First Select a Board",L"Warning",MB_OK|MB_ICONWARNING);
		   return;
	   }

	   CString CsFileFullName = fileDlg.GetPathName();
	   WideCharToMultiByte(CP_ACP, 0, CsFileFullName, -1, ffullname, 100, 0, 0);
	   strcpy(logfilepath, ffullname);

	   CString CsFileName = fileDlg.GetFileName();
	   WideCharToMultiByte(CP_ACP, 0, CsFileName, -1, fname, 30, 0, 0);

	   char * fpath_end = strstr (logfilepath, fname);
	   *fpath_end = 0;

	   fb = fopen(ffullname, "r");

       if(!fb)
       {
		   MessageBox(NULL,L"*** Error: Cannot open file",NULL,MB_OK);
		   return;
	   }

	   MessageBox(NULL,L"Press OK to run the test",L"Status",MB_OK|MB_ICONINFORMATION);

	   while(fscanf(fb, "%s", bcmd) != EOF)
	   {
		   DtaProcessCommand(bcmd, 1, fb);
	   }

	   fclose(fb);
	   
	   DtaDeInit();
	   BoardId = eBOARD_DEFAULT;

	   MessageBox(NULL,L"Batch test completed",L"Status",MB_OK|MB_ICONINFORMATION);
   }
}

void CDtaApp::InitBoard(DibBoardType BId)
{
	DtaInitSoft(BId, 0);
	DtaInit();
}

void CDtaApp::OnBoard9080()
{
	BoardId = eBOARD_9080;
	CDtaApp::InitBoard(BoardId);
	MessageBox(NULL,L"Board ITF 9080 Selected",L"Status",MB_OK|MB_ICONINFORMATION);
}

// CDtaApp construction
CDtaApp::CDtaApp()
	: CWinApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDtaApp object
CDtaApp theApp;

// CDtaApp initialization

BOOL CDtaApp::InitInstance()
{

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDtaDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDtaView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	return TRUE;
}


// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
#ifdef _DEVICE_RESOLUTION_AWARE
	afx_msg void OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/);
#endif
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;	// return TRUE unless you set the focus to a control
			// EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
#ifdef _DEVICE_RESOLUTION_AWARE
	ON_WM_SIZE()
#endif
END_MESSAGE_MAP()

#ifdef _DEVICE_RESOLUTION_AWARE
void CAboutDlg::OnSize(UINT /*nType*/, int /*cx*/, int /*cy*/)
{
	if (AfxIsDRAEnabled())
    	{
		DRA::RelayoutDialog(
			AfxGetResourceHandle(), 
			this->m_hWnd, 
			DRA::GetDisplayMode() != DRA::Portrait ? MAKEINTRESOURCE(IDD_ABOUTBOX_WIDE) : MAKEINTRESOURCE(IDD_ABOUTBOX));
	}
}
#endif

// App command to run the dialog
void CDtaApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

void CDtaApp::OnAppExit()
{
	if(BoardId != eBOARD_DEFAULT)
		   DtaDeInit();
	CWinApp::OnAppExit();
}

void CDtaApp::OnBoard9090()
{
	BoardId = eBOARD_9090;
	CDtaApp::InitBoard(BoardId);
	MessageBox(NULL,L"Board ITF 9090 Selected",L"Status",MB_OK|MB_ICONINFORMATION);

}

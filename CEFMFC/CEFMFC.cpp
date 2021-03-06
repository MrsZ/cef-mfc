
// CEFMFC.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "CEFMFC.h"
#include "MainFrm.h"

#include "CEFMFCDoc.h"
#include "CEFMFCView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCEFMFCApp

BEGIN_MESSAGE_MAP(CCEFMFCApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CCEFMFCApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
END_MESSAGE_MAP()


// CCEFMFCApp construction

CCEFMFCApp::CCEFMFCApp()
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("CEFMFC.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance

	m_bCEFInitialized = FALSE;

	m_szHomePage = _T("https://www.google.com");
}

// The one and only CCEFMFCApp object

CCEFMFCApp theApp;


// The CefDoMessageLoopWork needs to be in the message loop to draw the window
BOOL CCEFMFCApp::PumpMessage()
{
	// do CEF message loop
	if( m_bCEFInitialized )
		CefDoMessageLoopWork();

	return CWinAppEx::PumpMessage();
}


// CCEFMFCApp initialization

BOOL CCEFMFCApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	// initialize CEF.
	m_cefApp = new ClientApp();

	// get arguments
	CefMainArgs main_args(GetModuleHandle(NULL));

	// Execute the secondary process, if any.
	int exit_code = CefExecuteProcess(main_args, m_cefApp.get(), NULL);
	if (exit_code >= 0)
		return exit_code;

	// setup settings
	CString szCEFCache;
	CString szPath;
	INT nLen = GetTempPath( 0, NULL ) + 1;
	GetTempPath( nLen, szPath.GetBuffer( nLen ));

	// save path
	szCEFCache.Format( _T("%scache\0\0"), szPath );

	CefSettings settings;
	//settings.no_sandbox = TRUE;
	//settings.multi_threaded_message_loop = FALSE;

	CefString(&settings.cache_path) = szCEFCache;

	void* sandbox_info = NULL;
#if CEF_ENABLE_SANDBOX
	// Manage the life span of the sandbox information object. This is necessary
	// for sandbox support on Windows. See cef_sandbox_win.h for complete details.
	CefScopedSandboxInfo scoped_sandbox;
	sandbox_info = scoped_sandbox.sandbox_info();
#endif

	//CEF Initiaized
	m_bCEFInitialized = CefInitialize(main_args, settings, m_cefApp.get(), sandbox_info);


	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	EnableTaskbarInteraction(FALSE);

	// AfxInitRichEdit2() is required to use RichEdit control	
	// AfxInitRichEdit2();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(0);  // Load standard INI file options (including MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CCEFMFCDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CCEFMFCView));
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

	//no show title	so we can set our own and SES_USEAIMM for the Global IME.
	m_pMainWnd->ModifyStyle( FWS_ADDTOTITLE, SES_USEAIMM );
	m_pMainWnd->SetWindowText( _T("CEFMFC") );

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand

	// create CEF Browser
	CCEFMFCView* pView = (CCEFMFCView*)((CFrameWnd*)m_pMainWnd)->GetActiveView();
	if( pView )
	{
		// get rect
		CRect rect;
		pView->GetClientRect( rect );

		// set browser
		if( !m_cefApp->CreateBrowser( pView->GetSafeHwnd(), rect, m_szHomePage ))
		{
			AfxMessageBox(_T("Failed to create CEF Browser Window. Applications is exiting"));
			return FALSE;
		}
	}

	return TRUE;
}

int CCEFMFCApp::ExitInstance()
{
	// shutdown CEF
	if( m_bCEFInitialized )
	{
		// closing stop work loop
		m_bCEFInitialized = FALSE;
		// release CEF app
		m_cefApp = NULL;
		// shutdown CEF
		CefShutdown();
	}

	//TODO: handle additional resources you may have added
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CCEFMFCApp message handlers


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// App command to run the dialog
void CCEFMFCApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CCEFMFCApp customization load/save methods

void CCEFMFCApp::PreLoadState()
{
}

void CCEFMFCApp::LoadCustomState()
{
}

void CCEFMFCApp::SaveCustomState()
{
}

// CCEFMFCApp message handlers

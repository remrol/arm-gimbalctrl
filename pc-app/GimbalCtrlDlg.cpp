
// GimbalCtrlDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GimbalCtrl.h"
#include "GimbalCtrlDlg.h"
#include "afxdialogex.h"
#include "enumser.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGimbalCtrlDlg dialog




CGimbalCtrlDlg::CGimbalCtrlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGimbalCtrlDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGimbalCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_COMBO_COMPORTS, m_comboComPorts);
  DDX_Control(pDX, IDC_BUTTON_CONNECT, m_buttonConnect);
}

BEGIN_MESSAGE_MAP(CGimbalCtrlDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CGimbalCtrlDlg::OnBnClickedButtonConnect)
END_MESSAGE_MAP()


// CGimbalCtrlDlg message handlers

BOOL CGimbalCtrlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	CEnumerateSerial::CPortsArray ports;
	CEnumerateSerial::CNamesArray names;

	L_ << "Start enumeratung COM ports";
	CEnumerateSerial::UsingSetupAPI1(ports, names);
	L_ << "Finished enumeratung COM ports, found " << names.size();

	for( size_t i = 0; i < names.size(); ++i )
	{
		std::stringstream ss;
		ss << "COM" << ports[i] << " (" << names[i] << ")";
		int item = m_comboComPorts.AddString( ss.str().c_str());
		m_comboComPorts.SetItemData( item, ports[i] );
	}

	if( !names.empty() )
	{
		m_comboComPorts.SetCurSel(0);
	}

//	GetDlgItem(IDC_EDIT_DEVICE_INFO)->SetWindowText("Disconnected");

//	m_timer = SetTimer( WORK_TIMER_ID, WORK_TIMER_INTERVAL_MS, nullptr );
//	L_ << "Starting timer id " << WORK_TIMER_ID << " interval " << WORK_TIMER_INTERVAL_MS << " [ms] status " << m_timer;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGimbalCtrlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGimbalCtrlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGimbalCtrlDlg::OnBnClickedButtonConnect()
{
	L_ << "OnBnClickedButtonConnect begin";

	// Get selected item.
	int comboSelected = m_comboComPorts.GetCurSel();
	if( !m_device.isOpened() && comboSelected != CB_ERR )
	{
		// Open serial connection
		std::string status;
		if( m_device.open( (int) m_comboComPorts.GetItemData(comboSelected), 19200, status) )
		{
			assert( m_device.isOpened() );
			status = std::string( "Connected, " ) + status;
			m_buttonConnect.SetWindowText("Disconnect");

//			readConfig();
		}
		else
		{
			assert( !m_device.isOpened() );
			// Not valid, reset connection.
			status = std::string( "Disconnected, " ) + status;
		}

		GetDlgItem(IDC_EDIT_DEVICE_INFO)->SetWindowText( status.c_str() );
	}
	else
	{
		m_device.close();
		assert( !m_device.isOpened() );
		
		m_buttonConnect.SetWindowText("Connect");
		GetDlgItem(IDC_EDIT_DEVICE_INFO)->SetWindowText("Disconnected");
	}

	L_ << "OnBnClickedButtonConnect end";
}
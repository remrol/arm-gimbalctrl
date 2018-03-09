
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
  , m_servoMin(0)
  , m_servoDbandLo(0)
  , m_servoDbandHi(0)
  , m_servoMax(0)
  , m_timer(0)
  , m_motorPower(0)
  , m_pwmScaleFactor(0)
  , m_expo(0)
  , m_intervalProcessPulseMs(0)
  , m_intervalProcessSpeedSmooth(0)
  , m_timeoutMotorStopIfNoPulse(0)
  , m_timeoutMotorShutdownIfNoPulse(0)
  , m_speedSmoothRatio(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGimbalCtrlDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_COMPORTS, m_comboComPorts);
    DDX_Control(pDX, IDC_BUTTON_CONNECT, m_buttonConnect);
    DDX_Text(pDX, IDC_EDIT_SERVO_MIN, m_servoMin);
    DDX_Text(pDX, IDC_EDIT_SERVO_DBANDLO, m_servoDbandLo);
    DDX_Text(pDX, IDC_EDIT_SERVO_DBANDHI, m_servoDbandHi);
    DDX_Text(pDX, IDC_EDIT_SERVO_MAX, m_servoMax);
    DDX_Control(pDX, IDC_LIST_DIAGNOSTICS, m_listDiagnostics);
    DDX_Text(pDX, IDC_EDIT_MOTOR_POWER, m_motorPower);
    DDX_Text(pDX, IDC_EDIT_PWMSCALE, m_pwmScaleFactor);
    DDX_Text(pDX, IDC_EDIT_EXPO, m_expo);
    DDX_Text(pDX, IDC_EDIT_INTERVAL_PROCESSPULSE, m_intervalProcessPulseMs);
    DDX_Text(pDX, IDC_EDIT_INTERVAL_PROCESSPEEDSMOOTH, m_intervalProcessSpeedSmooth);
    DDX_Text(pDX, IDC_EDIT_TIMEOUT_STOPNOPULSE, m_timeoutMotorStopIfNoPulse);
    DDX_Text(pDX, IDC_EDIT_TIMEOUT_DISABLENOPULSE, m_timeoutMotorShutdownIfNoPulse);
    DDX_Text(pDX, IDC_EDIT_PROCESSPEEDSMOOTH_RATIO, m_speedSmoothRatio);
}

BEGIN_MESSAGE_MAP(CGimbalCtrlDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CGimbalCtrlDlg::OnBnClickedButtonConnect)
    ON_BN_CLICKED(IDC_BUTTON_READ_CONFIG, &CGimbalCtrlDlg::readConfig)
    ON_BN_CLICKED(IDC_BUTTON_READ_STATE, &CGimbalCtrlDlg::readState)
    ON_BN_CLICKED(IDC_BUTTON_SERVO_GET, &CGimbalCtrlDlg::readServoRanges)
    ON_BN_CLICKED(IDC_BUTTON_SERVO_SET, &CGimbalCtrlDlg::OnBnClickedButtonServoSet)
    ON_BN_CLICKED(IDC_BUTTON_CONFIG_SAVEEEPROM, &CGimbalCtrlDlg::OnBnClickedButtonConfigSaveeeprom)
    ON_BN_CLICKED(IDC_BUTTON_MOTORPARAMS_GET, &CGimbalCtrlDlg::readMotorParams)
    ON_BN_CLICKED(IDC_BUTTON_MOTORPARAMS_SET, &CGimbalCtrlDlg::OnBnClickedButtonMotorParamsSet)
    ON_BN_CLICKED(IDC_BUTTON_PROCESSINGINTERVALS_GET, &CGimbalCtrlDlg::readProcessing)
    ON_BN_CLICKED(IDC_BUTTON_PROCESSINGINTERVALS_SET, &CGimbalCtrlDlg::OnBnClickedButtonProcessingintervalsSet)
    ON_BN_CLICKED(IDC_BUTTON_MOTORTIMEOUTS_GET, &CGimbalCtrlDlg::readMotorTimeouts)
    ON_BN_CLICKED(IDC_BUTTON_MOTORTIMEOUTS_SET, &CGimbalCtrlDlg::OnBnClickedButtonMotortimeoutsSet)
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

	GetDlgItem(IDC_EDIT_DEVICE_INFO)->SetWindowText("Disconnected");

	m_timer = SetTimer( WORK_TIMER_ID, WORK_TIMER_INTERVAL_MS, nullptr );
	L_ << "Starting timer id " << WORK_TIMER_ID << " interval " << WORK_TIMER_INTERVAL_MS << " [ms] status " << m_timer;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGimbalCtrlDlg::OnDestroy()
{
	if( m_timer )
	{
		KillTimer( m_timer );
		L_ << "Stopped timer id " << m_timer;
	}

	CDialogEx::OnDestroy();
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

			readServoRanges();
			readConfig();
			readMotorParams();
			readProcessing();
			readMotorTimeouts();
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


void CGimbalCtrlDlg::readConfig()
{
    Config config;
    if( m_device.getConfig(config))
    {
        GetDlgItem(IDC_EDIT_CONFIG)->SetWindowText(config.toDisplayableString().c_str());
    }
}


void CGimbalCtrlDlg::readState()
{
    State state;
    if( m_device.getState(state))
    {
        GetDlgItem(IDC_EDIT_STATE)->SetWindowText(state.toDisplayableString().c_str());
    }
}

void CGimbalCtrlDlg::readServoRanges()
{
	if( m_device.getServoRange(m_servoMin, m_servoDbandLo, m_servoDbandHi, m_servoMax ) )
	{
		UpdateData(FALSE);
	}
}

void CGimbalCtrlDlg::OnBnClickedButtonServoSet()
{
	UpdateData(TRUE);

	if( m_device.setServoRange( m_servoMin, m_servoDbandLo, m_servoDbandHi, m_servoMax ) )
	{
		readConfig();
	}
	else
	{
		readServoRanges();
	}
}

void CGimbalCtrlDlg::OnBnClickedButtonConfigSaveeeprom()
{
	m_device.configSaveToEeprom();
}

void CGimbalCtrlDlg::OnTimer(UINT_PTR nIDEvent)
{
	if( nIDEvent == WORK_TIMER_ID )
	{
		if( m_device.isOpened() )
		{
			readState();
			readDiagnostics();
/*
			double timeNow = TimeMeasure::now();
			if( m_lastMeasureTime == 0 || timeNow >= m_lastMeasureTime + m_measureUpdateIntervalSec )
			{
				m_lastMeasureTime = timeNow;
				readMeasurements();
			} */
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CGimbalCtrlDlg::readDiagnostics()
{
    int diag0, diag1;

    if( m_device.getDiagnostics( diag0, diag1 ) )
    {
        std::stringstream ss;
        ss << diag0 << "," << diag1;
	    m_listDiagnostics.InsertString( 0, ss.str().c_str() );
    }
}


void CGimbalCtrlDlg::readMotorParams()
{
	if( m_device.getMotorParams( m_motorPower, m_pwmScaleFactor, m_expo ) )
	{
		UpdateData(FALSE);
	}
}


void CGimbalCtrlDlg::OnBnClickedButtonMotorParamsSet()
{
	UpdateData(TRUE);

	if( m_device.setMotorParams( m_motorPower, m_pwmScaleFactor, m_expo ) )
	{
		readConfig();
	}
	else
	{
		readMotorParams();
	}
}

void CGimbalCtrlDlg::readProcessing()
{
    if( m_device.getProcessing( m_intervalProcessPulseMs, m_intervalProcessSpeedSmooth, m_speedSmoothRatio ) )
	{
		UpdateData(FALSE);
	}
}


void CGimbalCtrlDlg::OnBnClickedButtonProcessingintervalsSet()
{
	UpdateData(TRUE);

	if( m_device.setProcessing( m_intervalProcessPulseMs, m_intervalProcessSpeedSmooth, m_speedSmoothRatio ) )
	{
		readConfig();
	}
	else
	{
		readProcessing();
	}
}


void CGimbalCtrlDlg::readMotorTimeouts()
{
    if( m_device.getTimeouts( m_timeoutMotorStopIfNoPulse, m_timeoutMotorShutdownIfNoPulse ) )
    {
        UpdateData(FALSE);
    }
}


void CGimbalCtrlDlg::OnBnClickedButtonMotortimeoutsSet()
{
	if( m_device.setTimeouts( m_timeoutMotorStopIfNoPulse, m_timeoutMotorShutdownIfNoPulse ) )
	{
		readConfig();
	}
	else
	{
		readMotorTimeouts();
	}
}

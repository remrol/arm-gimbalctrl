
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
  , m_yawPID_P(0)
  , m_yawPID_I(0)
  , m_yawPID_D(0)
  , m_yawSpeedSmoothFactor(0)
  , m_st32UpdateIntervalMs(0)
  , m_yawMaxSpeed(0)
  , m_listUpdateIntervalMs(1000)
  , m_listLastUpdateTime( TimeMeasure::now())
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
	DDX_Text(pDX, IDC_EDIT_YAWPID_P, m_yawPID_P);
	DDX_Text(pDX, IDC_EDIT_YAWPID_I, m_yawPID_I);
	DDX_Text(pDX, IDC_EDIT_YAWPID_D, m_yawPID_D);
	DDX_Text(pDX, IDC_EDIT_YAWSTAB_SPDSMOOTH, m_yawSpeedSmoothFactor);
	DDX_Text(pDX, IDC_EDIT_ST32UPDINTVL_MS, m_st32UpdateIntervalMs);
	DDX_Text(pDX, IDC_EDIT_YAWMAXSPEED, m_yawMaxSpeed);
	DDX_Text(pDX, IDC_EDIT_LIST_UPDATEINTERVAL, m_listUpdateIntervalMs);
	DDX_Control(pDX, IDC_COMBO_LIST_DATASOURCE, m_comboListDataSource);
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
	ON_BN_CLICKED(IDC_BUTTON_STORM32_GETDATA, &CGimbalCtrlDlg::OnBnClickedButtonStorm32Getdata)
	ON_BN_CLICKED(IDC_BUTTON_YAW_SET, &CGimbalCtrlDlg::OnBnClickedButtonYawSet)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_LOADDEFAULTS, &CGimbalCtrlDlg::OnBnClickedButtonConfigLoaddefaults)
	ON_BN_CLICKED(IDC_BUTTON_LIST_CLEAR, &CGimbalCtrlDlg::OnBnClickedButtonListClear)
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


	m_comboListDataSource.AddString( "Debug data" );
	m_comboListDataSource.AddString( "Storm32 live data" );
	m_comboListDataSource.AddString( "Sensors data" );
	m_comboListDataSource.AddString( "None" );
	m_comboListDataSource.SetCurSel(0);

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
//			readState();
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
	int selectedData = m_comboListDataSource.GetCurSel();

	if( selectedData == 0 )
	{
		std::stringstream ss;

		std::vector< int > values;
		values.resize(8);

		for( size_t i = 0; i < values.size(); ++i )
		{
			m_device.getDebug(i, values[i] );
			ss << i << ":" << values[i] << "   ";
		}

		m_listDiagnostics.InsertString( 0, ss.str().c_str() );	
	}
	else if( selectedData == 1 )
	{
		double pTS, mTS, mpuTS;
		int pT, pP, mX, mY, mZ, aX, aY, aZ, gX, gY, gZ;
		if( m_device.getSensors( 
			pTS, pT, pP, 
			mTS, mX, mY, mZ,
			mpuTS, aX, aY, aZ, gX, gY, gZ ) )
		{
			std::stringstream ss;
			ss << 
				"P: " << pTS << " " << pT / 10.0 << " " << pP << 
				" M: " << mTS << " " << mX << " " << mY << " " << mZ <<
				" A: " << aX << " " << aY << " " << aZ <<
				" G: " << gX << " " << gY << " " << gZ;

			m_listDiagnostics.InsertString( 0, ss.str().c_str() );
		}
	}
	else if( selectedData == 2 )
	{
		std::vector< int > values;

		if( m_device.getStorm32( values ) )
		{
			std::stringstream ss;

			for( int i = 7; i < values.size() - 2; i += 3 )
				ss << "  " << i << ":" << values[i] << " " << values[i+1] << " " << values[i+2];

			m_listDiagnostics.InsertString( 0, ss.str().c_str() );
		}
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
	UpdateData(TRUE);

	if( m_device.setTimeouts( m_timeoutMotorStopIfNoPulse, m_timeoutMotorShutdownIfNoPulse ) )
	{
		readConfig();
	}
	else
	{
		readMotorTimeouts();
	}
}


void CGimbalCtrlDlg::OnBnClickedButtonStorm32Getdata()
{
	m_device.getYawConfig( m_yawPID_P, m_yawPID_I, m_yawPID_D, m_yawSpeedSmoothFactor, m_st32UpdateIntervalMs, m_yawMaxSpeed );
    UpdateData(FALSE);

}


void CGimbalCtrlDlg::OnBnClickedButtonYawSet()
{
	UpdateData(TRUE);
	m_device.setYawConfig( m_yawPID_P, m_yawPID_I, m_yawPID_D, m_yawSpeedSmoothFactor, m_st32UpdateIntervalMs, m_yawMaxSpeed );


}


void CGimbalCtrlDlg::OnBnClickedButtonConfigLoaddefaults()
{
	// TODO: Add your control notification handler code here
}


void CGimbalCtrlDlg::OnBnClickedButtonListClear()
{
	m_listDiagnostics.ResetContent();
}

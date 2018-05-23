
// GimbalCtrlDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Device.h"


// CGimbalCtrlDlg dialog
class CGimbalCtrlDlg : public CDialogEx
{
// Construction
public:
	CGimbalCtrlDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_GIMBALCTRL_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	static const int WORK_TIMER_ID = 1;
	static const int WORK_TIMER_INTERVAL_MS = 1000;

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()
public:
    void readDiagnostics();
    afx_msg void OnBnClickedButtonConnect();
    afx_msg void readConfig();
    afx_msg void readState();
    afx_msg void readServoRanges();
    afx_msg void OnBnClickedButtonServoSet();
    afx_msg void OnBnClickedButtonConfigSaveeeprom();
    afx_msg void readMotorParams();
    afx_msg void OnBnClickedButtonMotorParamsSet();
    afx_msg void readProcessing();
    afx_msg void OnBnClickedButtonProcessingintervalsSet();
    afx_msg void readMotorTimeouts();
    afx_msg void OnBnClickedButtonMotortimeoutsSet();

    Device		m_device;
    CComboBox	m_comboComPorts;
    CButton		m_buttonConnect;
	UINT_PTR    m_timer;
    int			m_servoMin;
    int			m_servoDbandLo;
    int			m_servoDbandHi;
    int			m_servoMax;
    CListBox	m_listDiagnostics;
    int			m_motorPower;
    int			m_pwmScaleFactor;
    int			m_expo;
    int			m_intervalProcessPulseMs;
    int			m_intervalProcessSpeedSmooth;
    int			m_timeoutMotorStopIfNoPulse;
    int			m_timeoutMotorShutdownIfNoPulse;
    int			m_speedSmoothRatio;
	afx_msg void OnBnClickedButtonStorm32Getdata();
};

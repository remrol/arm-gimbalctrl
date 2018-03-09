
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
	static const int WORK_TIMER_INTERVAL_MS = 200;

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
    Device				m_device;
    CComboBox m_comboComPorts;
    afx_msg void OnBnClickedButtonConnect();
    CButton m_buttonConnect;
    afx_msg void readConfig();
    afx_msg void readState();
    afx_msg void readServoRanges();
    afx_msg void OnBnClickedButtonServoSet();
    int m_servoMin;
    int m_servoDbandLo;
    int m_servoDbandHi;
    int m_servoMax;
    afx_msg void OnBnClickedButtonConfigSaveeeprom();
    void readDiagnostics();

	UINT_PTR            m_timer;

    CListBox m_listDiagnostics;
    int m_motorPower;
    int m_pwmScaleFactor;
    afx_msg void readMotorParams();
    afx_msg void OnBnClickedButtonMotorParamsSet();
    int m_expo;
    int m_intervalProcessPulseMs;
    int m_intervalProcessSpeedSmooth;
    afx_msg void readProcessing();
    afx_msg void OnBnClickedButtonProcessingintervalsSet();
    int m_timeoutMotorStopIfNoPulse;
    int m_timeoutMotorShutdownIfNoPulse;
    afx_msg void readMotorTimeouts();
    afx_msg void OnBnClickedButtonMotortimeoutsSet();
    int m_speedSmoothRatio;
};

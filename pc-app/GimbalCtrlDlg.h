
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
  afx_msg void OnBnClickedButtonReadConfig();
  afx_msg void OnBnClickedButtonReadState();
  afx_msg void OnBnClickedButtonServoGet();
  afx_msg void OnBnClickedButtonServoSet();
  int m_servoMin;
  int m_servoDbandLo;
  int m_servoDbandHi;
  int m_servoMax;
  afx_msg void OnBnClickedButtonConfigSaveeeprom();
  void readDiagnostics();

	UINT_PTR            m_timer;

  CListBox m_listDiagnostics;
};

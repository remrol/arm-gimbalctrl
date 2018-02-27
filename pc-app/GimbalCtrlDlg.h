
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


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  Device				m_device;
  CComboBox m_comboComPorts;
  afx_msg void OnBnClickedButtonConnect();
  CButton m_buttonConnect;
  afx_msg void OnBnClickedButtonReadConfig();
  afx_msg void OnBnClickedButtonReadState();
};

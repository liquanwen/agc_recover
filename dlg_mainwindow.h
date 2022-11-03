/*

*
* 文件名称: dlg_mainwindow.h
* 摘    要: 主窗口类声明文件
*

*/
#ifndef __DLG_MAINWINDOW_H__
#define __DLG_MAINWINDOW_H__

#include "ui_mainwindow.h"
#include <QtWidgets>


enum mainstate
{
	mainstate_null,
	mainstate_apn = 1,
	mainstate_LTEmodem = 2,
};

class CMainWndDlg:public QMainWindow,  public Ui::CMainWndDlg_Base
{
	Q_OBJECT
public:
	CMainWndDlg(QWidget* parent = NULL);
	~CMainWndDlg();
public:
private:
	void InitWidget();
    void setTableWdgetAttribute();
	void createLogo();
    void createBaseParam();
    void createInvParam();

protected slots:
	void Loadvideo2();
	void receiveInfo();
	void onTimer();
    void slot_TimeTCP();
    void slot_TimeDone();
    void slot_TimeData();
    void onMqttConnect();
    void onMqttDisConnect();
    void onReadLog();
    void onCloseLog();
    void onReboot();
    void onSetParam();
    void onReadParam();
    void onUpTool();
    void onDownTool();
    void onUnpackParam();
    void onUnpackSetParam();
    void onUnpackSetReboot();
    void onUnpackRealData();

    void onCopyInv();
    void onDelInv();
    void onAddInv();
    void oncellChanged(int row, int column);
	
	void showVolEleEff(); //界面显示节点电压、电流和效率
	void analysisAgcStatus();
	void agcStatusShowInit();
private:
	//Ui::CMainWndDlg_Base* ui;
	QSplitter *         m_pSpliter;
	QSplitter *         m_pSpliterLeft;
	QSplitter *         m_pSpliterLeftRight;
	QSplitter*          m_pVSpliterDwon;
	QSplitter *         m_pSpliterUp1;
	QSplitter *         m_pSpliterRirht1;
	
	QStatusBar*		    m_statusbar;
	QLabel*			    m_statusSpace;
	QToolBar*		    m_pLinkStateToolBar;
	QComboBox*		    m_pCBTerminalInfo;					//切换终端

    //QTimer	m_mqttConnectTimer;         // 终端连接状态

	QTimer	m_dataTimer;	// 实时数据刷新定时器
	QTimer  *m_TCPTimer;	// 连接状态刷新定时器
	QTimer *m_pTestResultShowtimer;
	QTimer *m_pTimer;
	QTimer *m_pDataTimer;
	int m_mainstate;
	int m_mainstate2;
	int m_timeinter;
	bool m_bResult;
    bool m_lastFlag;
};

class CLastPath : public QObject
{
	Q_OBJECT
public:
	CLastPath(QObject *parent = 0);
	~CLastPath(void);
	QString GetLastPath(QString Key);
	void SetLastPath(QString Key, QString Path);

private:
	QSettings		*m_Settings;
};




#endif

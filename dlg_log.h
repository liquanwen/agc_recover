#ifndef DLG_LOG_H
#define DLG_LOG_H

#include "ui_dlg_log.h"


class CDlgLOG:public QDialog,Ui_LogDlg
{
	Q_OBJECT

public:
	CDlgLOG(QWidget* parent = 0);
	~CDlgLOG();

private slots:

	void onTimer();
	void onTimer2();
	void onExit();


private:
	void Initial();

private:

	QTimer *m_pTimer;
	QTimer *m_pTimer2;
	int		m_state;
};



#endif //DLG_LOG_H


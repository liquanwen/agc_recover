#include <QtNetwork>
#include "dlg_log.h"
//#include "win_main.h"
#include <QMessageBox> 
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
//#include <atlbase.h>
#include "meg_std.h"



CDlgLOG::CDlgLOG(QWidget* parent)
: QDialog(parent)
, m_pTimer(NULL)
{

	setupUi(this);
	Initial();
	connect(pushButton,SIGNAL(clicked()),this,SLOT(onExit()));

	pushButton->setEnabled(false);
	m_state = 90;
	this->setWindowTitle(QString::fromLocal8Bit("输出日志"));
	m_pTimer = new QTimer(this);
	m_pTimer->setInterval(1000);//1s
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
	m_pTimer->start();

	m_pTimer2 = new QTimer(this);
	m_pTimer2->setInterval(50);//1s
	connect(m_pTimer2, SIGNAL(timeout()), this, SLOT(onTimer2()));
	m_pTimer2->start();

}

CDlgLOG::~CDlgLOG()
{
	if (m_pTimer != NULL)
	{
		m_pTimer->stop();
		delete m_pTimer;
	}

	if (m_pTimer2 != NULL)
	{
		m_pTimer2->stop();
		delete m_pTimer2;
	}

	m_state = 90;
}

//界面初始化
void CDlgLOG::Initial()
{
	//m_pComm = CClientCommController::Instance();
	//m_pProxy = CAccessProxy::Instance();
	this->setWindowFlags(Qt::Window | Qt::WindowTitleHint);
}

void CDlgLOG::onExit()
{

	this->accept();

}
void CDlgLOG::onTimer2()
{
	std::string str;
	//if (CManager::Instance().Get_RealItem(str))
	//{
		QString Qstr = QString::fromLocal8Bit(str.c_str());
		//ATlog->append(Qstr);
		textBrowser->moveCursor(QTextCursor::End);
		textBrowser->insertPlainText(Qstr);
	//}	
}

		
void CDlgLOG::onTimer()
{
	m_state = m_state--;
	QString str = QString::fromLocal8Bit("重启倒计时: %1").arg(m_state);
	label->setText(str);
	if (!m_state)
	{
		m_pTimer->stop();
		pushButton->setEnabled(true);
	}

}

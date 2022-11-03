
#include <QMessageBox>
#include <QtWidgets>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include "chart.h"
#include "filepath.h"
#include "dlg_mainwindow.h"
#include "mqtt_pub_inter.h"
#include "real_data.h"
#include "dlg_log.h"
#include "RealTimeCurveQChartWidget.h"

QT_CHARTS_USE_NAMESPACE

CMainWndDlg::CMainWndDlg(QWidget* parent)
:QMainWindow(parent),m_pLinkStateToolBar(NULL)
, m_pTimer(NULL)
{
	m_bResult = false;
    m_lastFlag = false;
	m_mainstate = mainstate_null;
	m_mainstate2 = mainstate_null;
	setupUi(this);
	// 支持拖放
	//setAcceptDrops(true);
	//this->setFixedSize(1018, 670);

	setWindowTitle(QString::fromLocal8Bit("梅格彤天 AGC 软件调试工具"));
	QIcon icon;
	icon.addPixmap(QPixmap(QString::fromUtf8(":/images/sourectest.png")), QIcon::Normal, QIcon::Off);
	setWindowIcon(icon); 
	QPalette pal(centralwidget->palette());
	pal.setColor(QPalette::Background, Qt::transparent);
	centralwidget->setAutoFillBackground(true);
	centralwidget->setPalette(pal);
	
    //设置表格属性

    //添加状态栏
	m_timeinter = 5;
	m_pTestResultShowtimer = new QTimer(this);
    m_pTestResultShowtimer->setInterval(1000 * 60 * 3);//1000ms
	connect(m_pTestResultShowtimer, SIGNAL(timeout()), this, SLOT(slot_TimeDone()) );

	m_TCPTimer = new QTimer(this);
	m_TCPTimer->setInterval(100);//100ms
	connect(m_TCPTimer, SIGNAL(timeout()), this, SLOT(slot_TimeTCP()));
    m_TCPTimer->start();

	InitWidget();
	agcStatusShowInit();

	m_pDataTimer = new QTimer(this);
	m_pDataTimer->setInterval(1500);//1.5s
	connect(m_pDataTimer, SIGNAL(timeout()), this, SLOT(slot_TimeData()));
	//m_pDataTimer->start();

	//connect(GetAT, SIGNAL(clicked()), this, SLOT(onGetAt()));

	m_pTimer = new QTimer(this);
	m_pTimer->setInterval(50);//50ms
	connect(m_pTimer, SIGNAL(timeout()), this, SLOT(onTimer()));
	m_pTimer->start();
    if (!CMqttClientInterManager::CreateInstance().Init())
    {
        exit(1);
    }

    Chart *chart = new Chart;
    chart->setTitle(QString::fromLocal8Bit("发电功率实时曲线"));
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::AllAnimations);
    QChartView *chartView = new QChartView(chart, widget_3);

    chartView->setRenderHint(QPainter::Antialiasing);
    QGridLayout *gridLayout_15 = new QGridLayout(widget_3);
    gridLayout_15->setObjectName(QString::fromUtf8("gridLayout_15"));

    //RealTimeCurveQChartWidget w;
    //w.resize(700, 400);
    //w.show();
    //gridLayout_15->addWidget(&w, 0, 0, 1, 1);
    gridLayout_15->addWidget(chartView, 0, 0, 1, 1);
	tabWidget->setTabEnabled(0, false);
	tabWidget->setTabEnabled(1, false);
	tabWidget->setTabEnabled(2, false);
}

CMainWndDlg::~CMainWndDlg()
{
	if(m_pTimer != NULL)
		delete m_pTimer;
	if (m_pDataTimer != NULL)
		delete m_pDataTimer;
	if (m_TCPTimer != NULL)
		delete m_TCPTimer;
	if (m_pTestResultShowtimer != NULL)
		delete m_pTestResultShowtimer;
}

void CMainWndDlg::InitWidget()
{
	createLogo();
    createBaseParam();
    createInvParam();
    QRegExp rx("((1?\\d{1,2}|2[0-5]{2})\\.){3}(1?\\d{1,2}|2[0-5]{2})");
    QValidator * validator = new QRegExpValidator(rx, this);
    m_ipContent->setValidator(validator);
    m_ipContent->setPlaceholderText(tr("192.168.1.101"));



    QRegExp rx2("[0-9\.]+$");
    QValidator *validator2 = new QRegExpValidator(rx, this);
    agc_setpower->setValidator(new QDoubleValidator(0, 65535, 2, this));

    lineEdit_agc_dead_zone->setValidator(validator2);
    lineEdit_Inv_dead_zone->setValidator(validator2);

    connect(m_ipConnect, SIGNAL(clicked()), this, SLOT(onMqttConnect()));
    connect(m_ipDisConnect, SIGNAL(clicked()), this, SLOT(onMqttDisConnect()));

    //m_ipConnect


    m_readLog->setStatusTip(QString::fromLocal8Bit(" 开启日志 "));
    m_readLog->setIcon(QIcon(":/images/play.png"));
    connect(m_readLog, SIGNAL(clicked()), this, SLOT(onReadLog()));


    m_closeLog->setStatusTip(QString::fromLocal8Bit(" 暂停日志 "));
    m_closeLog->setIcon(QIcon(":/images/pause.png"));
    connect(m_closeLog, SIGNAL(clicked()), this, SLOT(onCloseLog()));
    
    CRealDataManager &Mng = CRealDataManager::CreateInstance();
    connect(&Mng, SIGNAL(emitUnpackParam()), this, SLOT(onUnpackParam()));
    connect(&Mng, SIGNAL(emitSetParam()), this, SLOT(onUnpackSetParam()));
    connect(&Mng, SIGNAL(emitSetReboot()), this, SLOT(onUnpackSetReboot()));
    connect(&Mng, SIGNAL(emitRealData()), this, SLOT(onUnpackRealData()));
    
    setTableWdgetAttribute();   //设备表格样式


    m_reboot->setShortcut(QKeySequence("Ctrl+P"));
    m_reboot->setStatusTip(QString::fromLocal8Bit(" 重启 "));
    m_reboot->setIcon(QIcon(":/images/reboot.png"));
    connect(m_reboot, SIGNAL(clicked()), this, SLOT(onReboot()));

    m_setParam->setShortcut(QKeySequence("Ctrl+L"));
    m_setParam->setStatusTip(QString::fromLocal8Bit("设置参数"));
    m_setParam->setIcon(QIcon(":/images/set.png"));
    connect(m_setParam, SIGNAL(clicked()), this, SLOT(onSetParam()));
    //m_setParam->setEnabled(false);

    m_ReadParam->setShortcut(QKeySequence("Ctrl+R"));
    m_ReadParam->setStatusTip(QString::fromLocal8Bit("读取参数"));
    m_ReadParam->setIcon(QIcon(":/images/read.png"));
    connect(m_ReadParam, SIGNAL(clicked()), this, SLOT(onReadParam()));

    toolButton_up->setShortcut(QKeySequence("Ctrl+U"));
    toolButton_up->setStatusTip(QString::fromLocal8Bit("功率上调"));
    connect(toolButton_up, SIGNAL(clicked()), this, SLOT(onUpTool()));

    toolButton_down->setShortcut(QKeySequence("Ctrl+K"));
    toolButton_down->setStatusTip(QString::fromLocal8Bit("功率下调"));
    connect(toolButton_down, SIGNAL(clicked()), this, SLOT(onDownTool()));
}

void CMainWndDlg::setTableWdgetAttribute()
{
    Juint32 item_num = 0;
    QFont font = InvTable->horizontalHeader()->font();
    font.setBold(true);
    InvTable->horizontalHeader()->setFont(font);
    InvTable->horizontalHeader()->resizeSection(item_num++, 30); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 100); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 100); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 100); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 100); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 100); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 100); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 100); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 100); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 120); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 80); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 100); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 120); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 120); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 120); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 120); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 150); //
    InvTable->horizontalHeader()->resizeSection(item_num++, 300); //
    InvTable->verticalHeader()->setVisible(false); //设置垂直头不可见

}

void CMainWndDlg::createLogo()
{
	std::string sCfgFileDir = CFilePath::Instance().GetCfgPath();
#ifdef WIN32
	std::string sLogoFilePath = sCfgFileDir + "\\aboutwin\\logo.png";
#else
	std::string sLogoFilePath = sCfgFileDir + "/aboutwin/logo.png";
#endif

	QPalette palette;
	//palette.setBrush(QPalette::Background, QBrush(QPixmap(QString::fromLocal8Bit(sLogoFilePath.c_str()))));
    palette.setBrush(QPalette::Background, QBrush(QPixmap(":/images/logo.png")));
	//m_logo_win->setPalette(palette);
}

void CMainWndDlg::createBaseParam()
{
}

void CMainWndDlg::createInvParam()
{
 
    // 设置按钮样式及悬浮、按下时的状态
    m_copyInv->setStyleSheet("QToolButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}\nQToolButton:hover{background-color:rgb(229, 241, 251); color: black;}\nQToolButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

    m_delInv->setStyleSheet("QToolButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}\nQToolButton:hover{background-color:rgb(229, 241, 251); color: black;}\nQToolButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");

    m_addInv->setStyleSheet("QToolButton{background-color: rgb(225, 225, 225);border:2px groove gray;border-radius:10px;padding:2px 4px;border-style: outset;}\nQToolButton:hover{background-color:rgb(229, 241, 251); color: black;}\nQToolButton:pressed{background-color:rgb(204, 228, 247);border-style: inset;}");


    m_copyInv->setStatusTip(QString::fromLocal8Bit("复制逆变器"));
    m_copyInv->setIcon(QIcon(":/images/copy.png"));
    connect(m_copyInv, SIGNAL(clicked()), this, SLOT(onCopyInv()));

    m_delInv->setStatusTip(QString::fromLocal8Bit("删除逆变器"));
    m_delInv->setIcon(QIcon(":/images/delete.png"));
    connect(m_delInv, SIGNAL(clicked()), this, SLOT(onDelInv()));

    m_addInv->setStatusTip(QString::fromLocal8Bit("添加逆变器"));
    m_addInv->setIcon(QIcon(":/images/add.png"));
    connect(m_addInv, SIGNAL(clicked()), this, SLOT(onAddInv()));

    connect(InvTable, SIGNAL(cellChanged(int,int)), this, SLOT(oncellChanged(int,int)));
}



void CMainWndDlg::onTimer()
{
    QString Qstr;
    if (CRealDataManager::CreateInstance().Get_LogInfoItem(Qstr))
    {
        textBrowser->moveCursor(QTextCursor::End);
        textBrowser->insertPlainText(Qstr);
    }	
}

void CMainWndDlg::slot_TimeTCP()
{
    if (CMqttClientInterManager::CreateInstance().getMqttConnect())
    {
        m_ipConnect->setDisabled(true);
        m_ipDisConnect->setDisabled(false);
        m_connectStatus->setPixmap(QPixmap(QString::fromUtf8(":/images/lan-connect.png")));
        if (m_lastFlag == false) {
            CRealDataManager::CreateInstance().ReadParam();
        }
        m_lastFlag = true;


    } else {
        m_ipConnect->setDisabled(false);
        m_ipDisConnect->setDisabled(true);
        m_connectStatus->setPixmap(QPixmap(QString::fromUtf8(":/images/lan-disconnect.png")));
        m_lastFlag = false;
    }
}

void CMainWndDlg::onMqttConnect()
{
    QString server = QString::fromLocal8Bit("tcp://%1:1883").arg(m_ipContent->text());
    CMqttClientInterManager::CreateInstance().win_mqtt_connect(server.toLocal8Bit());
}

void CMainWndDlg::onMqttDisConnect()
{
    CMqttClientInterManager::CreateInstance().win_mqtt_disconnect();
}

void CMainWndDlg::onReadLog()
{
    // 定时器3分钟
    CRealDataManager::CreateInstance().OpenLog(QString::fromLocal8Bit("open"));
    // 开启定时器
    m_pTestResultShowtimer->start();
}

void CMainWndDlg::onCloseLog()
{
    CRealDataManager::CreateInstance().OpenLog(QString::fromLocal8Bit("close"));
    m_pTestResultShowtimer->stop();
}

void CMainWndDlg::onReboot()
{
    CRealDataManager::CreateInstance().RebootAgc(1);
}

void CMainWndDlg::onSetParam()
{

    int qret = QMessageBox::warning(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("参数设置前请确保已经召唤并修改完成，已经修改不可撤销"), QString::fromLocal8Bit("确定修改"),QString::fromLocal8Bit("取消"));
    if (qret != 0) {
        return;
    } 

    CRealDataManager::CreateInstance().m_agc_enable = agc_enable->isChecked();

    QString qagc_getpower = agc_getpower->currentText();
    if (qagc_getpower.compare(QString::fromLocal8Bit("本地计算")) == 0) {
        CRealDataManager::CreateInstance().m_agc_getpower = "local";
    } else {
        CRealDataManager::CreateInstance().m_agc_getpower = "remote";
    }

    CRealDataManager::CreateInstance().m_agc_strategy = agc_strategy->currentIndex();

    QString qagc_setpower = agc_setpower->text();
    CRealDataManager::CreateInstance().m_agc_setpower = (float)qagc_setpower.toDouble();
    CRealDataManager::CreateInstance().m_inverter_total = InvTable->rowCount();

    CRealDataManager::CreateInstance().m_agc_dead_zone = lineEdit_agc_dead_zone->text().toFloat();        // 目标值死区
    CRealDataManager::CreateInstance().m_Inv_dead_zone = lineEdit_Inv_dead_zone->text().toFloat();        // 逆变器功率死区（AGC调节）
    CRealDataManager::CreateInstance().m_yt_coef = lineEdit_yt_coef->text().toFloat();        // 遥调系数
    if (yt_flag->currentIndex() == 0) {
        CRealDataManager::CreateInstance().m_yt_flag = false;
    } else {
        CRealDataManager::CreateInstance().m_yt_flag = true;
    }

    if (yt_type->currentText().compare(QString::fromLocal8Bit("归一化")) == 0) {
        CRealDataManager::CreateInstance().m_yt_type = 48;
    } else if (yt_type->currentText().compare(QString::fromLocal8Bit("标度化")) == 0) {
        CRealDataManager::CreateInstance().m_yt_type = 49;
    } else if (yt_type->currentText().compare(QString::fromLocal8Bit("浮点数")) == 0) {
        CRealDataManager::CreateInstance().m_yt_type = 50;
    }

    //遥调系数

    if (template_enable->currentText().compare(QString::fromLocal8Bit("是")) == 0) {
        CRealDataManager::CreateInstance().m_template_enable = true;
    } else if (template_enable->currentText().compare(QString::fromLocal8Bit("否")) == 0) {
        CRealDataManager::CreateInstance().m_template_enable = false;
    }


    CRealDataManager::CreateInstance().m_template_sn = template_sn->text().toInt();        // 样板逆变器序号
    if (inverter_adjust_sw->currentText().compare(QString::fromLocal8Bit("有效")) == 0) {
        CRealDataManager::CreateInstance().m_inverter_adjust_sw = true;
    } else if (inverter_adjust_sw->currentText().compare(QString::fromLocal8Bit("无效")) == 0) {
        CRealDataManager::CreateInstance().m_inverter_adjust_sw = false;
    }

    CRealDataManager::CreateInstance().m_adjust_on_cmd = adjust_on_cmd->text().toInt();        // 样板逆变器序号
    CRealDataManager::CreateInstance().m_adjust_off_cmd = adjust_off_cmd->text().toInt();        // 样板逆变器序号

    CRealDataManager::CreateInstance().m_generate_power_yc_sort = generate_power_yc_sort->text().toInt();        // 样板逆变器序号
    CRealDataManager::CreateInstance().m_merge_grid_yc_sort = merge_grid_yc_sort->text().toInt();        // 样板逆变器序号

    for (int i = 0; i < InvTable->rowCount(); i++)
    {
        Juint32 Item_num = 0;
        QTableWidgetItem * psn = InvTable->item(i, Item_num++);    //序号
        //QWidget * p = InvTable->cellWidget(i, 0);    //序号
        InverterInfoData_s InverterInfo = {0};
        if (psn != NULL) {
            InverterInfo.sn = psn->data(0).toInt();
        }

        QTableWidgetItem * pname = InvTable->item(i, Item_num++);    //name
        if (pname != NULL) {
            InverterInfo.name = pname->data(0).toString().toStdString();
        }

        QTableWidgetItem * pyc_sort = InvTable->item(i, Item_num++);    //yc_sort
        if (pyc_sort != NULL) {
            InverterInfo.yc_sort = pyc_sort->data(0).toInt();
        }

        QTableWidgetItem * pyx_sort = InvTable->item(i, Item_num++);    //yx_sort
        if (pyx_sort != NULL) {
            InverterInfo.yx_sort = pyx_sort->data(0).toInt();
        }

        QTableWidgetItem * pyk_sort = InvTable->item(i, Item_num++);    //yk_sort
        if (pyk_sort != NULL) {
            InverterInfo.yk_sort = pyk_sort->data(0).toInt();
        }

        QTableWidgetItem * pyt_sort = InvTable->item(i, Item_num++);    //yt_sort
        if (pyt_sort != NULL) {
            InverterInfo.yt_sort = pyt_sort->data(0).toInt();
        }

        QTableWidgetItem * pon_sort = InvTable->item(i, Item_num++);    //on_sort
        if (pyt_sort != NULL) {
            InverterInfo.on_sort = pon_sort->data(0).toInt();
        }
        QTableWidgetItem * poff_sort = InvTable->item(i, Item_num++);    //off_sort
        if (pyt_sort != NULL) {
            InverterInfo.off_sort = poff_sort->data(0).toInt();
        }
        QTableWidgetItem * pgroup_num = InvTable->item(i, Item_num++);    //group_num
        if (pyt_sort != NULL) {
            InverterInfo.group_num = pgroup_num->data(0).toInt();
        }

        QTableWidgetItem * pstatus_sort = InvTable->item(i, Item_num++);    //status_sort
        if (pstatus_sort != NULL) {
            InverterInfo.status_sort = pstatus_sort->data(0).toInt();
        }

        QTableWidgetItem * ppriority = InvTable->item(i, Item_num++);    //priority
        if (ppriority != NULL) {
            InverterInfo.priority = ppriority->data(0).toInt();
        }

        QTableWidgetItem * pfenable = InvTable->item(i, Item_num++);    //使能标志
        if (pfenable != NULL) {
            InverterInfo.fenable = pfenable->data(0).toBool();
        }
        Qt::CheckState state = pfenable->checkState();
        if (state == Qt::Checked) {
            InverterInfo.fenable = true;
        } else {
            InverterInfo.fenable = false;
        }



        QTableWidgetItem * prated_power = InvTable->item(i, Item_num++);    //rated_power
        if (prated_power != NULL) {
            InverterInfo.rated_power = prated_power->data(0).toDouble();
        }

        QTableWidgetItem * pset_popwer = InvTable->item(i, Item_num++);    //set_popwer
        if (pset_popwer != NULL) {
            InverterInfo.set_popwer = pset_popwer->data(0).toDouble();
        }


        STimeInfo tm = ii_get_current_time();
        QTimeEdit * pstart_time = (QTimeEdit *)InvTable->cellWidget(i, Item_num++);    //start_time
        if (pstart_time != NULL) {
            InverterInfo.start_time = QString::fromLocal8Bit("%1-%2-%3 %4")
                .arg(tm.nYear)
                .arg(tm.nMonth)
                .arg(tm.nDay)
                .arg(pstart_time->text()).toStdString();
        }


        //STimeInfo tm = ii_get_current_time();
        QTimeEdit * pend_time = (QTimeEdit *)InvTable->cellWidget(i, Item_num++);    //end_time
        if (pend_time != NULL) {
            InverterInfo.end_time = QString::fromLocal8Bit("%1-%2-%3 %4")
                .arg(tm.nYear)
                .arg(tm.nMonth)
                .arg(tm.nDay)
                .arg(pend_time->text()).toStdString();
        }

        QTableWidgetItem * pcurve_total = InvTable->item(i, Item_num++);    //set_popwer
        if (pcurve_total != NULL) {
            int intercal = pcurve_total->data(0).toInt();   // 间隔时间
            CEPTime sttime(InverterInfo.start_time);
            CEPTime ettime(InverterInfo.end_time);
            Jint32  diff = 0;
            if (sttime.GetDiffTime(ettime, diff)) {
                InverterInfo.curve_total = abs((int)(diff/ (intercal*60)));
            }
        }

        QTableWidgetItem * pcurve = InvTable->item(i, Item_num++);    //set_popwer
        if (pcurve != NULL) {
            QString qcurve = pcurve->data(0).toString();
            QStringList qlist = qcurve.split(",");
            for (int j = 0; j < qlist.size(); j++)
            {
                InverterInfo.curve[j] = qlist[j].toFloat();
            }
        }
        CRealDataManager::CreateInstance().m_InfoDatas[i] = InverterInfo;
    }

    CRealDataManager::CreateInstance().SetParam();
}

void CMainWndDlg::onReadParam()
{
    CRealDataManager::CreateInstance().ReadParam();
}

void CMainWndDlg::onUpTool()
{
    CRealDataManager::CreateInstance().UpTool(lineEdit_up->text().toDouble());
}

void CMainWndDlg::onDownTool()
{
    CRealDataManager::CreateInstance().UpTool(lineEdit_down->text().toDouble());
}


void CMainWndDlg::onUnpackParam()
{
    agc_enable->setChecked(CRealDataManager::CreateInstance().m_agc_enable); ;           //AGC使能，true/false
    if (CRealDataManager::CreateInstance().m_agc_getpower.compare("local") == 0) {
        agc_getpower->setCurrentText(QString::fromLocal8Bit("本地计算"));
    } else {
        agc_getpower->setCurrentText(QString::fromLocal8Bit("协议获取"));
    }
     ;         //AGC功率获取方式，remote / local。 remote：本地计算，通过累加各逆变器功率获得local：远程，通过104获取总功率
    agc_strategy->setCurrentIndex(CRealDataManager::CreateInstance().m_agc_strategy);         //策略模式：平均策略，主动策略，AI自学，
    agc_setpower->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_agc_setpower));         //设置功率，单位MW 目标功率（本地设置？）
    lineEdit_agc_dead_zone->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_agc_dead_zone));        //目标值死区
    lineEdit_Inv_dead_zone->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_Inv_dead_zone)); //逆变器功率死区
    lineEdit_yt_coef->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_yt_coef));//遥调系数

    generate_power_yc_sort->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_generate_power_yc_sort)); //实时发电功率遥测点号
    merge_grid_yc_sort->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_merge_grid_yc_sort)); //实时发电功率遥测点号

    template_sn->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_template_sn)); //实时发电功率遥测点号
         
    if (CRealDataManager::CreateInstance().m_template_enable) {
        template_enable->setCurrentText(QString::fromLocal8Bit("是"));
    } else {
        template_enable->setCurrentText(QString::fromLocal8Bit("否"));
    }

    if (CRealDataManager::CreateInstance().m_yt_flag) {
        yt_flag->setCurrentText(QString::fromLocal8Bit("选择执行"));
    } else {
        yt_flag->setCurrentText(QString::fromLocal8Bit("直接"));
    }

    if (CRealDataManager::CreateInstance().m_yt_type == 48) {
        yt_type->setCurrentText(QString::fromLocal8Bit("归一化"));
    } else if (CRealDataManager::CreateInstance().m_yt_type == 49) {
        yt_type->setCurrentText(QString::fromLocal8Bit("标度化"));
    } else if (CRealDataManager::CreateInstance().m_yt_type == 50) {
        yt_type->setCurrentText(QString::fromLocal8Bit("浮点数"));
    }
    adjust_on_cmd->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_adjust_on_cmd)); 
    adjust_off_cmd->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_adjust_off_cmd));



    if (CRealDataManager::CreateInstance().m_inverter_adjust_sw) {
        inverter_adjust_sw->setCurrentText(QString::fromLocal8Bit("有效"));
    } else {
        inverter_adjust_sw->setCurrentText(QString::fromLocal8Bit("无效"));
    }

    //InvTable->clear();
    for (; InvTable->rowCount() != 0;)
    {
        InvTable->removeRow(InvTable->rowCount() - 1);
    }

    int inverterNum = CRealDataManager::CreateInstance().m_inverter_total;           //逆变器个数
    InvTable->setRowCount(inverterNum);
    for (int i = 0; i < inverterNum; i++)
    {
        InverterInfoData_s InverterInfo = CRealDataManager::CreateInstance().m_InfoDatas[i];
        QTableWidgetItem *pItem0 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.sn)); //序号
        QTableWidgetItem *pItem1 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(QString::fromStdString(InverterInfo.name)));//名称
        QTableWidgetItem *pItem2 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.yc_sort));//实时功率点号
        QTableWidgetItem *pItem3 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.yx_sort));//遥信值点号
        QTableWidgetItem *pItem4 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.yk_sort));//遥控值点号
        QTableWidgetItem *pItem5 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.yt_sort));//遥调值点号
        QTableWidgetItem *pItem6 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.status_sort));//逆变器状态点号
        QTableWidgetItem *pItem7 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.priority));//优先级
        QTableWidgetItem *pItem8 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(""));//使能
        QTableWidgetItem *pItem9 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.rated_power));//额定功率
        QTableWidgetItem *pItem10 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.set_popwer));//设置功率

        CEPTime sttime(InverterInfo.start_time);
        CEPTime ettime(InverterInfo.end_time);

        QTime qstart_time(sttime.hour(), sttime.minute(), sttime.second());
        QTime qend_time(ettime.hour(), ettime.minute(), ettime.second());

        QTimeEdit* ptimeEdit11 = new QTimeEdit(qstart_time, InvTable);
        ptimeEdit11->setDisplayFormat("hh:mm:ss");
        QTimeEdit* ptimeEdit12 = new QTimeEdit(qend_time, InvTable);
        ptimeEdit12->setDisplayFormat("hh:mm:ss");

        QTableWidgetItem *pItem_on_sort = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.on_sort));         //  开启功率调节点号    add by tianzhenchao
        QTableWidgetItem *pItem_off_sort = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.off_sort));       //  关闭功率调节点号    add by tianzhenchao
        QTableWidgetItem *pItem_group_num = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.group_num));     //  所属组 用于同时遥控

        Juint32 itemNum = 0;
        InvTable->setItem(i, itemNum++, pItem0);
        InvTable->setItem(i, itemNum++, pItem1);
        InvTable->setItem(i, itemNum++, pItem2);
        InvTable->setItem(i, itemNum++, pItem3);
        InvTable->setItem(i, itemNum++, pItem4);
        InvTable->setItem(i, itemNum++, pItem5);
        InvTable->setItem(i, itemNum++, pItem_on_sort);
        InvTable->setItem(i, itemNum++, pItem_off_sort);
        InvTable->setItem(i, itemNum++, pItem_group_num);

        InvTable->setItem(i, itemNum++, pItem6);
        InvTable->setItem(i, itemNum++, pItem7);
        //InvTable->setItem(Invsize, 8, pItem8);
        bool value = true;
        Qt::CheckState state = (value == InverterInfo.fenable) ? Qt::Checked : Qt::Unchecked;
        pItem8->setCheckState(state);
        InvTable->setItem(i, itemNum++, pItem8);
        InvTable->setItem(i, itemNum++, pItem9);
        InvTable->setItem(i, itemNum++, pItem10);
        InvTable->setCellWidget(i, itemNum++, ptimeEdit11);
        InvTable->setCellWidget(i, itemNum++, ptimeEdit12);


        Jint32  diff = 0;
        int interval = 0;
        if (sttime.GetDiffTime(ettime, diff)) {
            interval = abs((int)(abs(diff) / (InverterInfo.curve_total * 60)));
        }
        if (interval > 1024) {
            return;
        }
        QTableWidgetItem *pItem13 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(interval));//jiange
        InvTable->setItem(i, itemNum++, pItem13);
        QString str = "";
        for (int j = 0; j < InverterInfo.curve_total; j++)
        {
            str.append(QString::fromLocal8Bit("%1").arg(InverterInfo.curve[j]));
            if (j == InverterInfo.curve_total - 1) {
                break;
            }
            str.append(",");
        }
        QTableWidgetItem *pItem14 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(str));//
        InvTable->setItem(i, itemNum++, pItem14);
    }
       //InvTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void CMainWndDlg::onUnpackSetParam()
{
    if (CRealDataManager::CreateInstance().m_statusDesc.compare("OK") == 0) {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("参数设置成功"), QString::fromLocal8Bit("确定"));
    } else {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("参数设置失败，失败原因：%1").arg(QString::fromStdString(CRealDataManager::CreateInstance().m_statusDesc)), QString::fromLocal8Bit("确定"));
    }
}

void CMainWndDlg::onUnpackSetReboot()
{
    if (CRealDataManager::CreateInstance().m_RebootStatusDesc.compare("OK") == 0) {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("重启成功"), QString::fromLocal8Bit("确定"));
    } else {
        QMessageBox::information(this, QString::fromLocal8Bit("提示"), QString::fromLocal8Bit("重启失败，失败原因：%1").arg(QString::fromStdString(CRealDataManager::CreateInstance().m_RebootStatusDesc)), QString::fromLocal8Bit("确定"));
    }
}

void CMainWndDlg::onUnpackRealData()
{
    lineEdit_p->setText(QString("%1").arg(CRealDataManager::CreateInstance().m_cal_p_total));
    treeWidget->clear();
    std::map<Juint32, InverterRealData_s>::iterator it = CRealDataManager::CreateInstance().m_RealDatas.begin();
    for (; it != CRealDataManager::CreateInstance().m_RealDatas.end(); ++it)
    {
        Juint32 num = it->first;
        InverterRealData_s info = it->second;

        QTreeWidgetItem *Item  = new QTreeWidgetItem(treeWidget);
        treeWidget->insertTopLevelItem(num,Item);
        //QTreeWidgetItem * qtreewidgetitem1 = treeWidget->topLevelItem(num);
        Item->setText(0, QString::fromLocal8Bit("%1").arg(info.sn));
        Item->setText(1, QString::fromLocal8Bit("%1").arg(QString::fromStdString(info.name)));
        Item->setText(2, QString::fromLocal8Bit("%1").arg(info.real_popwer));

    }
    lineEdit_00->setText(QString("%1").arg(CRealDataManager::CreateInstance().m_cal_p_total* 0.001));
	lineEdit_power->setText(QString("%1").arg(CRealDataManager::CreateInstance().m_cal_p_total));
    lineEdit_1->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[0].real_popwer));
    lineEdit_2->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[1].real_popwer));
    lineEdit_3->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[2].real_popwer));
    lineEdit_4->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[3].real_popwer));
    lineEdit_5->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[4].real_popwer));
    lineEdit_6->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[5].real_popwer));
    lineEdit_7->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[6].real_popwer* 0.001));
    lineEdit_8->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[7].real_popwer* 0.001));
    lineEdit_09->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[8].real_popwer* 0.001));
    lineEdit_010->setText(QString::fromLocal8Bit("%1").arg(CRealDataManager::CreateInstance().m_RealDatas[9].real_popwer* 0.001));

	lineEdit_adjMax->setText(QString::number(CRealDataManager::CreateInstance().m_adj_max));
	lineEdit_adjMin->setText(QString::number(CRealDataManager::CreateInstance().m_adj_min));
	lineEdit_ctrlCap->setText(QString::number(CRealDataManager::CreateInstance().m_ctrl_cap));
	lineEdit_powerPre->setText(QString::number(CRealDataManager::CreateInstance().m_power_pre));
	lineEdit_realRet->setText(QString::number(CRealDataManager::CreateInstance().m_real_ret));
    
	showVolEleEff();
	analysisAgcStatus();

	
    //QLineSeries *series = new QLineSeries();

    //series->append(0, 6);
    //series->append(2, 4);
    //series->append(3, 8);
    //series->append(7, 4);
    //series->append(10, 5);
    //*series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

    //QChart *chart = new QChart();
    //chart->legend()->hide();
    //chart->addSeries(series);
    //chart->createDefaultAxes();
    //chart->setTitle("Simple line chart example");

    //QChartView *chartView = new QChartView(chart, groupBox_6);
    //chartView->setRenderHint(QPainter::Antialiasing);
}

void CMainWndDlg::slot_TimeDone()
{
    CRealDataManager::CreateInstance().OpenLog(QString::fromLocal8Bit("open"));
}

void CMainWndDlg::slot_TimeData()
{
	showVolEleEff();
}

void CMainWndDlg::onCopyInv()
{
    //// 获取当前行号
    //// sn为行号+1
    ////

    ////获取当前选中行的内容
        int curow =InvTable->currentRow();
        InverterInfoData_s InverterInfo = { 0 };
        Juint32 Item_num = 0;
        QTableWidgetItem * psn = InvTable->item(curow, Item_num++);    //序号
        if (psn != NULL) {
            InverterInfo.sn = psn->data(0).toInt();
        }

        QTableWidgetItem * pname = InvTable->item(curow, Item_num++);    //name
        if (pname != NULL) {
            InverterInfo.name = pname->data(0).toString().toStdString();
        }

        QTableWidgetItem * pyc_sort = InvTable->item(curow, Item_num++);    //yc_sort
        if (pyc_sort != NULL) {
            InverterInfo.yc_sort = pyc_sort->data(0).toInt();
        }

        QTableWidgetItem * pyx_sort = InvTable->item(curow, Item_num++);    //yx_sort
        if (pyx_sort != NULL) {
            InverterInfo.yx_sort = pyx_sort->data(0).toInt();
        }

        QTableWidgetItem * pyk_sort = InvTable->item(curow, Item_num++);    //yk_sort
        if (pyk_sort != NULL) {
            InverterInfo.yk_sort = pyk_sort->data(0).toInt();
        }

        QTableWidgetItem * pyt_sort = InvTable->item(curow, Item_num++);    //yt_sort
        if (pyt_sort != NULL) {
            InverterInfo.yt_sort = pyt_sort->data(0).toInt();
        }

        QTableWidgetItem * pon_sort = InvTable->item(curow, Item_num++);    //on_sort
        if (pyt_sort != NULL) {
            InverterInfo.on_sort = pon_sort->data(0).toInt();
        }
        QTableWidgetItem * poff_sort = InvTable->item(curow, Item_num++);    //off_sort
        if (pyt_sort != NULL) {
            InverterInfo.off_sort = poff_sort->data(0).toInt();
        }
        QTableWidgetItem * pgroup_num = InvTable->item(curow, Item_num++);    //group_num
        if (pyt_sort != NULL) {
            InverterInfo.group_num = pgroup_num->data(0).toInt();
        }

        QTableWidgetItem * pstatus_sort = InvTable->item(curow, Item_num++);    //status_sort
        if (pstatus_sort != NULL) {
            InverterInfo.status_sort = pstatus_sort->data(0).toInt();
        }

        QTableWidgetItem * ppriority = InvTable->item(curow, Item_num++);    //priority
        if (ppriority != NULL) {
            InverterInfo.priority = ppriority->data(0).toInt();
        }

        QTableWidgetItem * pfenable = InvTable->item(curow, Item_num++);    //
        if (pfenable != NULL) {
            InverterInfo.fenable = pfenable->data(0).toBool();
        }

        QTableWidgetItem * prated_power = InvTable->item(curow, Item_num++);    //rated_power
        if (prated_power != NULL) {
            InverterInfo.rated_power = prated_power->data(0).toDouble();
        }

        QTableWidgetItem * pset_popwer = InvTable->item(curow, Item_num++);    //set_popwer
        if (pset_popwer != NULL) {
            InverterInfo.set_popwer = pset_popwer->data(0).toDouble();
        }


        STimeInfo tm = ii_get_current_time();
        QTimeEdit * pstart_time = (QTimeEdit *)InvTable->cellWidget(curow, Item_num++);    //start_time
        if (pstart_time != NULL) {
            InverterInfo.start_time = QString::fromLocal8Bit("%1-%2-%3 %4")
                .arg(tm.nYear)
                .arg(tm.nMonth)
                .arg(tm.nDay)
                .arg(pstart_time->text()).toStdString();
        }


        //STimeInfo tm = ii_get_current_time();
        QTimeEdit * pend_time = (QTimeEdit *)InvTable->cellWidget(curow, Item_num++);    //end_time
        if (pend_time != NULL) {
            InverterInfo.end_time = QString::fromLocal8Bit("%1-%2-%3 %4")
                .arg(tm.nYear)
                .arg(tm.nMonth)
                .arg(tm.nDay)
                .arg(pend_time->text()).toStdString();
        }

        QTableWidgetItem * pcurve_total = InvTable->item(curow, Item_num++);    //set_popwer
        if (pcurve_total != NULL) {
            int intercal = pcurve_total->data(0).toInt();   // 间隔时间
            CEPTime sttime(InverterInfo.start_time);
            CEPTime ettime(InverterInfo.end_time);
            Jint32  diff = 0;
            if (sttime.GetDiffTime(ettime, diff)) {
                InverterInfo.curve_total = abs((int)(diff / (intercal * 60)));
            }
        }

        QTableWidgetItem * pcurve = InvTable->item(curow, Item_num++);    //set_popwer
        if (pcurve != NULL) {
            QString qcurve = pcurve->data(0).toString();
            QStringList qlist = qcurve.split(",");
            for (int i = 0; i < qlist.size(); i++)
            {
                InverterInfo.curve[i] = qlist[i].toFloat();
            }
        }
        int Invsize = InvTable->rowCount();
        InvTable->setRowCount(Invsize + 1);
        QTableWidgetItem *pItem0 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize)); //序号
        QTableWidgetItem *pItem1 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(QString::fromStdString(InverterInfo.name)));//名称
        QTableWidgetItem *pItem2 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.yc_sort));//实时功率点号
        QTableWidgetItem *pItem3 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.yx_sort));//遥信值点号
        QTableWidgetItem *pItem4 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.yk_sort));//遥控值点号
        QTableWidgetItem *pItem5 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.yt_sort));//遥调值点号
        QTableWidgetItem *pItem6 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.status_sort));//逆变器状态点号
        QTableWidgetItem *pItem7 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.priority));//优先级
        QTableWidgetItem *pItem8 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(""));//使能
        QTableWidgetItem *pItem9 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.rated_power));//额定功率
        QTableWidgetItem *pItem10 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.set_popwer));//设置功率

        CEPTime sttime(InverterInfo.start_time);
        CEPTime ettime(InverterInfo.end_time);

        QTime qstart_time(sttime.hour(), sttime.minute(), sttime.second());
        QTime qend_time(ettime.hour(), ettime.minute(), ettime.second());

        QTimeEdit* ptimeEdit11 = new QTimeEdit(qstart_time, InvTable);
        ptimeEdit11->setDisplayFormat("hh:mm:ss");
        QTimeEdit* ptimeEdit12 = new QTimeEdit(qend_time, InvTable);
        ptimeEdit12->setDisplayFormat("hh:mm:ss");

        QTableWidgetItem *pItem_on_sort = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.on_sort));         //  开启功率调节点号    add by tianzhenchao
        QTableWidgetItem *pItem_off_sort = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.off_sort));       //  关闭功率调节点号    add by tianzhenchao
        QTableWidgetItem *pItem_group_num = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(InverterInfo.group_num));     //  所属组 用于同时遥控

        Item_num = 0;
        InvTable->setItem(Invsize, Item_num++, pItem0);
        InvTable->setItem(Invsize, Item_num++, pItem1);
        InvTable->setItem(Invsize, Item_num++, pItem2);
        InvTable->setItem(Invsize, Item_num++, pItem3);
        InvTable->setItem(Invsize, Item_num++, pItem4);
        InvTable->setItem(Invsize, Item_num++, pItem5);
        InvTable->setItem(Invsize, Item_num++, pItem_on_sort);
        InvTable->setItem(Invsize, Item_num++, pItem_off_sort);
        InvTable->setItem(Invsize, Item_num++, pItem_group_num);

        InvTable->setItem(Invsize, Item_num++, pItem6);
        InvTable->setItem(Invsize, Item_num++, pItem7);
        //InvTable->setItem(Invsize, 8, pItem8);
        bool value = true;
        Qt::CheckState state = (value == InverterInfo.fenable) ? Qt::Checked : Qt::Unchecked;
        pItem8->setCheckState(state);
        InvTable->setItem(Invsize, Item_num++, pItem8);
        InvTable->setItem(Invsize, Item_num++, pItem9);
        InvTable->setItem(Invsize, Item_num++, pItem10);
        InvTable->setCellWidget(Invsize, Item_num++, ptimeEdit11);
        InvTable->setCellWidget(Invsize, Item_num++, ptimeEdit12);


        Jint32  diff = 0;
        int interval = 0;
        if (sttime.GetDiffTime(ettime, diff)) {
            interval = abs((int)(abs(diff) / (InverterInfo.curve_total * 60)));
        }
        if (interval > 1024) {
            return;
        }
        QTableWidgetItem *pItem13 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(interval));//jiange
        InvTable->setItem(Invsize, Item_num++, pItem13);
        QString str = "";
        for (int j = 0; j < InverterInfo.curve_total; j++)
        {
            str.append(QString::fromLocal8Bit("%1").arg(InverterInfo.curve[j]));
            if (j == InverterInfo.curve_total - 1) {
                break;
            }
            str.append(",");
        }
        QTableWidgetItem *pItem14 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(str));//
        InvTable->setItem(Invsize, Item_num++, pItem14);
}

void CMainWndDlg::onDelInv()
{
    //int currentInv = InvTable->currentRow();
    //InvTable->removeRow(currentInv);

    QList<QTableWidgetItem*> list = InvTable->selectedItems();

    for (int i = 0; i < list.count(); i++)
    {
        /*获取条目的行号*/
        int row = InvTable->row(list.at(i));
        qDebug() << "即将卸载的行号:" << row;
        delete list.at(i); //彻底删除条目

        //因为上面的循环是以条目数量为准，所以卸载行号只需要卸载一行即可
        if (row != -1)
        {
            InvTable->removeRow(row);
        }
    }
    //int Invsize = InvTable->rowCount();
    ////InvTable->setCellWidget(Invsize, 0, QWidget *widget);
    //QTableWidgetItem *pItem = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));
    //InvTable->setRowCount(Invsize + 1);
    //InvTable->setItem(Invsize, 0, pItem);

    
}

void CMainWndDlg::onAddInv()
{
    // 获取当前行号
    // sn为行号+1
    //
    int Invsize = InvTable->rowCount();
    InvTable->setRowCount(Invsize + 1);
    //InvTable->setCellWidget(Invsize, 0, QWidget *widget);
    QTableWidgetItem *pItem0 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize)); //序号
    QTableWidgetItem *pItem1 = new QTableWidgetItem(QString::fromLocal8Bit("new%1").arg(Invsize));//名称
    QTableWidgetItem *pItem2 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//实时功率点号
    //QTableWidgetItem *pItem3 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//遥测值点号

    QTableWidgetItem *pItem3 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//遥信值点号
    QTableWidgetItem *pItem4 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//遥控值点号
    QTableWidgetItem *pItem5 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//遥调值点号

    QTableWidgetItem *pItem_on_sort = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//on_sort
    QTableWidgetItem *pItem_off_sort = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//off_sort
    QTableWidgetItem *pItem_group_num = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//group_num


    QTableWidgetItem *pItem6 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//逆变器状态点号
    QTableWidgetItem *pItem7 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(Invsize));//优先级
    QTableWidgetItem *pItem8 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg(""));//使能
    QTableWidgetItem *pItem9 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg("20"));//额定功率
    QTableWidgetItem *pItem10 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg("20"));//设置功率

    QTimeEdit* ptimeEdit11 = new QTimeEdit(QTime::fromString("06:00:00"),InvTable);
    ptimeEdit11->setDisplayFormat("hh:mm:ss");
    QTimeEdit* ptimeEdit12 = new QTimeEdit(QTime::fromString("18:00:00"), InvTable);
    ptimeEdit12->setDisplayFormat("hh:mm:ss");


    Juint32 item_num = 0;
    InvTable->setItem(Invsize, item_num++, pItem0);
    InvTable->setItem(Invsize, item_num++, pItem1);
    InvTable->setItem(Invsize, item_num++, pItem2);
    InvTable->setItem(Invsize, item_num++, pItem3);
    InvTable->setItem(Invsize, item_num++, pItem4);
    InvTable->setItem(Invsize, item_num++, pItem5);

    InvTable->setItem(Invsize, item_num++, pItem_on_sort);
    InvTable->setItem(Invsize, item_num++, pItem_off_sort);
    InvTable->setItem(Invsize, item_num++, pItem_group_num);

    InvTable->setItem(Invsize, item_num++, pItem6);
    InvTable->setItem(Invsize, item_num++, pItem7);

    std::string value = "true";
    Qt::CheckState state = (value == "true") ? Qt::Checked : Qt::Unchecked;
    pItem8->setCheckState(state);
    InvTable->setItem(Invsize, item_num++, pItem8);
    InvTable->setItem(Invsize, item_num++, pItem9);
    InvTable->setItem(Invsize, item_num++, pItem10);
    InvTable->setCellWidget(Invsize, item_num++, ptimeEdit11);
    InvTable->setCellWidget(Invsize, item_num++, ptimeEdit12);
    QTableWidgetItem *pItem13 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg("60"));//设置功率
    InvTable->setItem(Invsize, item_num++, pItem13);
    QTableWidgetItem *pItem14 = new QTableWidgetItem(QString::fromLocal8Bit("%1").arg("1,2,3,4,5,6,7,8,9,10,11,12"));//设置功率
    InvTable->setItem(Invsize, item_num++, pItem14);

}

void CMainWndDlg::oncellChanged(int row, int column)
{
    //InvTable->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void CMainWndDlg::Loadvideo2()
{
	std::string str = CFilePath::Instance().GetCfgPath();
	std::string str2 = str + "\\socketTOOLS";
	str = str + "\\socketTOOLS\\SocketTool.exe";
	//str = "C:\\Program Files (x86)\\Cer-Mgr-Tool\\Cer-Mgr-Tool.exe";
	//str2 = "C:\\Program Files (x86)\\Cer-Mgr-Tool";
	QString program = QString::fromLocal8Bit(str.c_str());

	if (program.contains(" ") || program.contains("\t"))
	{
		if (!program.startsWith("\""))
		{
			program = "\"" + program;
		}
		if (!program.endsWith("\""))
		{
			program = program + "\"";
		}
	}

	QString program2 = QString::fromLocal8Bit(str2.c_str());
	QString program3 = QString::fromLocal8Bit(str2.c_str());//处理空格情况下，不加“”情况
	if (program2.contains(" ") || program2.contains("\t"))
	{
		if (!program2.startsWith("\""))
		{
			program2 = "\"" + program2;
		}
		if (!program2.endsWith("\""))
		{
			program2 = program2 + "\"";
		}
	}

	if (program == "")
		return;

	QProcess *process = new QProcess(this);
	QDir::setCurrent(program3);
	//process->setWorkingDirectory(program3);
	process->startDetached(program, QStringList(program2));

}

void CMainWndDlg::receiveInfo()
{

}

void CMainWndDlg::showVolEleEff() {
	//srand(time(0));
	float Vol_fraction, Ele_fraction, Eff_fraction;
	Vol_fraction = rand() % 1600 - 800;
	float voltage_value = 220 + (Vol_fraction / 1000);
	//lineEdit_node_V->setText(QString::number(voltage_value));

	Ele_fraction = rand() % 100 - 50;
	float current_value = 5 + (Ele_fraction / 1000);
	//lineEdit_node_I->setText(QString::number(current_value));
	printf("CMainWndDlg::showVolEleEff() voltage_value:%f, lineEdit_node_I:%f\n", voltage_value, current_value);

	Eff_fraction = rand() % 8;
	float efficiency_value = 0.99 + (Eff_fraction / 1000);
	lineEdit_efficiency->setText(QString::number(efficiency_value));
}

void CMainWndDlg::analysisAgcStatus() {
	uint16_t status;
	status = CRealDataManager::CreateInstance().m_agc_status;
	if (status & 0x01) {
		label_enable->setText(QString::fromLocal8Bit(" 投入"));
	}
	else {
		label_enable->setText(QString::fromLocal8Bit(" 投退"));
	}

	if (status & 0x02) {
		label_run_mode->setText(QString::fromLocal8Bit(" 远方"));
	}
	else {
		label_run_mode->setText(QString::fromLocal8Bit(" 就地"));
	}

	if (status & 0x04) {
		label_upper->setStyleSheet("background-color:red");
	}
	else {
		label_upper->setStyleSheet("background-color:green");
	}

	if (status & 0x08) {
		label_lower->setStyleSheet("background-color:red");
	}
	else {
		label_lower->setStyleSheet("background-color:green");
	}
}

void CMainWndDlg::agcStatusShowInit() {
	label_enable->setText(QString::fromLocal8Bit(" 投入"));
	label_run_mode->setText(QString::fromLocal8Bit(" 远方"));
	label_upper->setStyleSheet("background-color:red");
	label_lower->setStyleSheet("background-color:red");
}



CLastPath::CLastPath(QObject *parent)
	:QObject(parent)
	, m_Settings(NULL)
{
	std::string IniPath = CFilePath::Instance().GetIniPath();
	QString FilePath = QString::fromStdString(IniPath) + "Setting.ini";
	m_Settings = new QSettings(FilePath, QSettings::IniFormat);
}

CLastPath::~CLastPath(void)
{

}

QString CLastPath::GetLastPath(QString Key)
{
	return m_Settings->value(Key).toString();
}

void CLastPath::SetLastPath(QString Key, QString Path)
{
	int i = Path.lastIndexOf('/');
	QString FilePath = Path.left(i);
	QVariant VarPath(FilePath);
	m_Settings->setValue(Key, VarPath);
}




#include <QJsonValue> 
#include "mqtt_pub_inter.h"
#include "real_data.h"



CRealDataManager &CRealDataManager::CreateInstance()
{
    static CRealDataManager Mng;
    return Mng;
}

bool CRealDataManager::Init(void)
{
   
    return true;
}

void CRealDataManager::Exit(void)
{
}

void CRealDataManager::InitData()
{
   m_agc_enable = true;                 //  AGC使能，true/false
   m_agc_getpower = "remote";           //  AGC功率获取方式，remote / local。 remote：本地计算，通过累加各逆变器功率获得local：远程，通过104获取总功率
   m_agc_strategy = 0;                  //  策略模式：平均策略，主动策略，AI自学，
   m_agc_setpower = 5;                  //  设置功率，单位MW 目标功率（本地设置？）
   //m_curve_total = 24;                //  功率计划曲线点个数
   m_inverter_total = 5;                //  逆变器个数
   m_cal_p_total = 0;
}

bool CRealDataManager::UnpackData(mqtt_data_info_s &real_data)
{
    //bool bRet =true;

    if (real_data.pubtopic.compare(GET_PARAM_TOPIC_RESPONSE) == 0)  //回复的参数
    {
        UnpackParam(real_data.msg_send.c_str(), real_data.msg_send_lenth);
    } 
    else if (real_data.pubtopic.compare(SET_PARAM_TOPIC_RESPONSE) == 0) //设置结果
    {
        UnpackSetParam(real_data.msg_send.c_str(), real_data.msg_send_lenth);
    }
    else if (real_data.pubtopic.compare(GET_REAL_DATA_TOPIC) == 0)  //实时数据
    {
        UnpackRealData(real_data.msg_send.c_str(), real_data.msg_send_lenth);
    } 
    else if (real_data.pubtopic.compare(GET_LOG_TOPIC) == 0)    //实时日志
    {
        UnpackLog(real_data.msg_send.c_str(), real_data.msg_send_lenth);
    } 
    else if (real_data.pubtopic.compare(GET_REBOOT_TOPIC) == 0) //重启结果
    {
        UnpackReboot(real_data.msg_send.c_str(), real_data.msg_send_lenth);
    }
    return true;
}

void CRealDataManager::OpenLog(QString flag)
{
    QJsonDocument document;
    QJsonObject datajson;
    STimeInfo tm = ii_get_current_time();
    QJsonObject testOther;
    CGuid guid = CGuid::Create();
    std::string token;
    if (guid.ToString(token)) {
        datajson.insert("token", QJsonValue(QString::fromStdString(token)));
    }

    QString qst = QString::fromLocal8Bit("%1-%2-%3 %4:%5:%6")
        .arg(tm.nYear)
        .arg(tm.nMonth)
        .arg(tm.nDay)
        .arg(tm.nHour)
        .arg(tm.nMinute)
        .arg(tm.nSecond);
    datajson.insert("timestamp", QJsonValue(qst));
    datajson.insert("flag", QJsonValue(flag));


    document.setObject(datajson);
    QByteArray simpbyte_array = document.toJson(QJsonDocument::Compact);
    
    mqtt_data_info_s item;
    item.pubtopic = SET_LOG_TOPIC;
    item.msg_send = simpbyte_array.data();
    item.msg_send_lenth = simpbyte_array.size();
    item.retained = 0;
    CMqttClientInterManager::CreateInstance().Push_SendItem(item);

}

void CRealDataManager::RebootAgc(Juint32 delay)
{
    QJsonDocument document;
    QJsonObject datajson;
    STimeInfo tm = ii_get_current_time();
    QJsonObject testOther;
    CGuid guid = CGuid::Create();
    std::string token;
    if (guid.ToString(token)) {
        datajson.insert("token", QJsonValue(QString::fromStdString(token)));
    }

    QString qst = QString::fromLocal8Bit("%1-%2-%3 %4:%5:%6")
        .arg(tm.nYear)
        .arg(tm.nMonth)
        .arg(tm.nDay)
        .arg(tm.nHour)
        .arg(tm.nMinute)
        .arg(tm.nSecond);
    datajson.insert("timestamp", QJsonValue(qst));
    datajson.insert("delay", QJsonValue((int)delay));
    document.setObject(datajson);
    QByteArray simpbyte_array = document.toJson(QJsonDocument::Compact);

    mqtt_data_info_s item;
    item.pubtopic = SET_REBOOT_TOPIC;
    item.msg_send = simpbyte_array.data();
    item.msg_send_lenth = simpbyte_array.size();
    item.retained = 0;
    CMqttClientInterManager::CreateInstance().Push_SendItem(item);
}

void CRealDataManager::SetParam()
{
    QJsonDocument document;
    QJsonObject datajson;
    STimeInfo tm = ii_get_current_time();
    QJsonObject testOther;
    CGuid guid = CGuid::Create();
    std::string token;
    if (guid.ToString(token)) {
        datajson.insert("token", QJsonValue(QString::fromStdString(token)));
    }

    QString qst = QString::fromLocal8Bit("%1-%2-%3 %4:%5:%6")
        .arg(tm.nYear)
        .arg(tm.nMonth)
        .arg(tm.nDay)
        .arg(tm.nHour)
        .arg(tm.nMinute)
        .arg(tm.nSecond);
    datajson.insert("timestamp", QJsonValue(qst));
    datajson.insert("agc_enable", QJsonValue(m_agc_enable));
    datajson.insert("agc_getpower", QJsonValue(QString::fromStdString(m_agc_getpower)));
    datajson.insert("agc_strategy", QJsonValue((int)m_agc_strategy));
    datajson.insert("agc_setpower", QJsonValue(m_agc_setpower));
    datajson.insert("agc_dead_zone", QJsonValue(m_agc_dead_zone));
    datajson.insert("Inv_dead_zone", QJsonValue(m_Inv_dead_zone));
    datajson.insert("yt_flag", QJsonValue(m_yt_flag));
    datajson.insert("yt_type", QJsonValue((int)m_yt_type));
    datajson.insert("yt_coef", QJsonValue(m_yt_coef));
    datajson.insert("template_enable", QJsonValue(m_template_enable));
    datajson.insert("template_sn", QJsonValue((int)m_template_sn));
    datajson.insert("inverter_adjust_sw", QJsonValue(m_inverter_adjust_sw));

    datajson.insert("adjust_on_cmd", QJsonValue((int)m_adjust_on_cmd));
    datajson.insert("adjust_off_cmd", QJsonValue((int)m_adjust_off_cmd));
    datajson.insert("generate_power_yc_sort", QJsonValue((int)m_generate_power_yc_sort));
    datajson.insert("merge_grid_yc_sort", QJsonValue((int)m_merge_grid_yc_sort));

    datajson.insert("inverter_total", QJsonValue((int)m_inverter_total));

    QJsonArray inverters;


    for (int i = 0; i < m_inverter_total; i++)
    {
        QJsonObject inverter_obj;
        InverterInfoData_s* data = &m_InfoDatas[i];
        inverter_obj.insert("sn", QJsonValue((int)data->sn));
        inverter_obj.insert("name", QJsonValue(QString::fromStdString(data->name)));
        inverter_obj.insert("yc_sort", QJsonValue((int)data->yc_sort));
        inverter_obj.insert("yx_sort", QJsonValue((int)data->yx_sort));
        inverter_obj.insert("yk_sort", QJsonValue((int)data->yk_sort));
        inverter_obj.insert("yt_sort", QJsonValue((int)data->yt_sort));
        inverter_obj.insert("on_sort", QJsonValue((int)data->on_sort));
        inverter_obj.insert("off_sort", QJsonValue((int)data->off_sort));
        inverter_obj.insert("group_num", QJsonValue((int)data->group_num));
        inverter_obj.insert("status_sort", QJsonValue((int)data->status_sort));
        inverter_obj.insert("priority", QJsonValue((int)data->priority));
        inverter_obj.insert("enable", QJsonValue(data->fenable));
        inverter_obj.insert("rated_power", QJsonValue(data->rated_power));
        inverter_obj.insert("set_power", QJsonValue(data->set_popwer));
        inverter_obj.insert("start_time", QJsonValue(QString::fromStdString(data->start_time)));
        inverter_obj.insert("end_time", QJsonValue(QString::fromStdString(data->end_time)));
        inverter_obj.insert("curve_total", QJsonValue((int)data->curve_total));
        QJsonArray curves;
        for (int j = 0; j < data->curve_total; j++)
        {
            curves.append(QJsonValue(data->curve[j]));
        }
        inverter_obj.insert("curve", QJsonValue(curves));
        inverters.append(inverter_obj);
    }
    datajson.insert("inverter", QJsonValue(inverters));
    document.setObject(datajson);
    QByteArray simpbyte_array = document.toJson(QJsonDocument::Compact);

    mqtt_data_info_s item;
    item.pubtopic = SET_PARAM_TOPIC;
    item.msg_send = simpbyte_array.data();
    item.msg_send_lenth = simpbyte_array.size();
    item.retained = 0;
    CMqttClientInterManager::CreateInstance().Push_SendItem(item);
}

void CRealDataManager::ReadParam()
{
    QJsonDocument document;
    QJsonObject datajson;
    STimeInfo tm = ii_get_current_time();
    QJsonObject testOther;
    CGuid guid = CGuid::Create();
    std::string token;
    if (guid.ToString(token)) {
        datajson.insert("token", QJsonValue(QString::fromStdString(token)));
    }

    QString qst = QString::fromLocal8Bit("%1-%2-%3 %4:%5:%6")
        .arg(tm.nYear)
        .arg(tm.nMonth)
        .arg(tm.nDay)
        .arg(tm.nHour)
        .arg(tm.nMinute)
        .arg(tm.nSecond);
    datajson.insert("timestamp", QJsonValue(qst));

    document.setObject(datajson);
    QByteArray simpbyte_array = document.toJson(QJsonDocument::Compact);

    mqtt_data_info_s item;
    item.pubtopic = GET_PARAM_TOPIC;
    item.msg_send = simpbyte_array.data();
    item.msg_send_lenth = simpbyte_array.size();
    item.retained = 0;
    CMqttClientInterManager::CreateInstance().Push_SendItem(item);
}

void CRealDataManager::UpTool(double fvalue)
{
    QJsonDocument document;
    QJsonObject datajson;
    STimeInfo tm = ii_get_current_time();
    QJsonObject testOther;
    CGuid guid = CGuid::Create();
    std::string token;
    if (guid.ToString(token)) {
        datajson.insert("token", QJsonValue(QString::fromStdString(token)));
    }

    QString qst = QString::fromLocal8Bit("%1-%2-%3 %4:%5:%6")
        .arg(tm.nYear)
        .arg(tm.nMonth)
        .arg(tm.nDay)
        .arg(tm.nHour)
        .arg(tm.nMinute)
        .arg(tm.nSecond);
    datajson.insert("timestamp", QJsonValue(qst));
    datajson.insert("power", QJsonValue(fvalue));

    document.setObject(datajson);
    QByteArray simpbyte_array = document.toJson(QJsonDocument::Compact);

    mqtt_data_info_s item;
    item.pubtopic = GET_TOOL_TOPIC;
    item.msg_send = simpbyte_array.data();
    item.msg_send_lenth = simpbyte_array.size();
    item.retained = 0;
    CMqttClientInterManager::CreateInstance().Push_SendItem(item);
}


void CRealDataManager::Push_LogInfoItem(QString item)
{
    m_LogQueue.AddTail(item);
}

bool CRealDataManager::Get_LogInfoItem(QString &item)
{
    return m_LogQueue.GetHead(item);
}


bool CRealDataManager::UnpackParam(const char * pBuf, Juint32 len)
{
    m_InfoDatas.clear();
    QByteArray databuf;
    QByteArray ba((char*)pBuf, len);
    QJsonDocument jdoc = QJsonDocument::fromJson(ba.constData());
    if (jdoc.isNull())
        return false;
    QJsonObject obj = jdoc.object();

    m_agc_enable = obj.value("agc_enable").toBool();
    m_agc_getpower = obj.value("agc_getpower").toString().toStdString();
    m_agc_strategy = obj.value("agc_strategy").toInt();
    m_agc_setpower = obj.value("agc_setpower").toDouble();
    
    m_inverter_total = obj.value("inverter_total").toInt();

    m_agc_dead_zone = obj.value("agc_dead_zone").toDouble();
    m_Inv_dead_zone = obj.value("Inv_dead_zone").toDouble();

    m_yt_flag = obj.value("yt_flag").toBool();;              // 执行标志位 false:直接执行，true：选择执行
    m_yt_type = obj.value("yt_type").toInt();;              // 遥调数据类型 48归一化，49标度化，50浮点
    m_yt_coef = obj.value("yt_coef").toDouble();;      // yt_coef
    m_template_enable = obj.value("template_enable").toBool();;      // 是否有样板逆变器，true和false
    m_template_sn = obj.value("template_sn").toInt();;          // 样板逆变器序号
    m_inverter_adjust_sw = obj.value("inverter_adjust_sw").toBool();   // 逆变器使用有调节开关，true：调节前需要先将调节开关打开
    m_adjust_on_cmd = obj.value("adjust_on_cmd").toInt();        // 调节开关开启命令数据 10进制，例如0xAA需要写170
    m_adjust_off_cmd = obj.value("adjust_off_cmd").toInt();       // 调节开关关闭命令数据 10进制，例如0x55需要写85
    m_generate_power_yc_sort = obj.value("generate_power_yc_sort").toInt();     // 实时发电功率遥测点号，当agc_getpower = remote时有效
    m_merge_grid_yc_sort = obj.value("merge_grid_yc_sort").toInt();         // 并网点功率遥测点号



    QJsonArray jArray = obj.value("inverter").toArray();

    for (int i = 0; i < m_inverter_total; i++)
    {
        QJsonObject inverter = jArray.at(i).toObject();
        InverterInfoData_s InverterInfo;
        InverterInfo.sn = inverter.value("sn").toInt();
        InverterInfo.name = inverter.value("name").toString().toStdString();
        InverterInfo.yc_sort = inverter.value("yc_sort").toInt();
        InverterInfo.yx_sort = inverter.value("yx_sort").toInt();
        InverterInfo.yk_sort = inverter.value("yk_sort").toInt();
        InverterInfo.yt_sort = inverter.value("yt_sort").toInt();
        InverterInfo.on_sort = inverter.value("on_sort").toInt();
        InverterInfo.off_sort = inverter.value("off_sort").toInt();
        InverterInfo.group_num = inverter.value("group_num").toInt();
        InverterInfo.status_sort = inverter.value("status_sort").toInt();
        InverterInfo.priority = inverter.value("priority").toInt();
        InverterInfo.fenable = inverter.value("enable").toBool();
        InverterInfo.rated_power = inverter.value("rated_power").toDouble();
        InverterInfo.set_popwer = inverter.value("set_power").toDouble();
        InverterInfo.start_time = inverter.value("start_time").toString().toStdString();
        InverterInfo.end_time = inverter.value("end_time").toString().toStdString();
        InverterInfo.curve_total = inverter.value("curve_total").toInt();
        QJsonArray curveArray = inverter.value("curve").toArray();
        for (int j = 0; j < curveArray.size(); j++)
        {
            InverterInfo.curve[j]= curveArray.at(j).toDouble();
        }
        m_InfoDatas[i] = InverterInfo;
    }
    emit emitUnpackParam();
    return true;

}

bool CRealDataManager::UnpackSetParam(const char * pBuf, Juint32 len)
{
    QByteArray databuf;
    QByteArray ba((char*)pBuf, len);
    QJsonDocument jdoc = QJsonDocument::fromJson(ba.constData());
    if (jdoc.isNull())
        return false;
    QJsonObject obj = jdoc.object();

    m_statusCode = obj.value("statusCode").toInt();
    m_statusDesc = obj.value("statusDesc").toString().toStdString();
    emit emitSetParam();
    return true;
}

bool CRealDataManager::UnpackRealData(const char * pBuf, Juint32 len)
{
    QByteArray databuf;
    QByteArray ba((char*)pBuf, len);
    QJsonDocument jdoc = QJsonDocument::fromJson(ba.constData());
    if (jdoc.isNull())
        return false;
    QJsonObject obj = jdoc.object();

    m_cal_p_total = obj.value("cal_p_total").toDouble();
	m_agc_status = (Juint16)obj.value("AGCstatus").toInt();
	m_adj_max = (Juint16)obj.value("AdjMax").toInt();
	m_adj_min = (Juint16)obj.value("AdjMin").toInt();
	m_ctrl_cap = (Juint16)obj.value("CtrlCap").toInt();
	m_power_pre = (Juint16)obj.value("PowerPre").toInt();
	m_real_ret = (Juint16)obj.value("RealRet").toInt();

    QJsonArray jArray = obj.value("inverter").toArray();

    for (int i = 0; i < jArray.size(); i++)
    {
        QJsonObject inverter = jArray.at(i).toObject();
        InverterRealData_s InverterInfo;
        InverterInfo.sn = inverter.value("sn").toInt();
        InverterInfo.name = inverter.value("name").toString().toStdString();
        InverterInfo.real_popwer = inverter.value("p").toDouble();
        m_RealDatas[i] = InverterInfo;
    }

    emit emitRealData();
    return true;
}

bool CRealDataManager::UnpackLog(const char * pBuf, Juint32 len)
{
    QByteArray databuf;
    QByteArray ba((char*)pBuf, len);
    QJsonDocument jdoc = QJsonDocument::fromJson(ba.constData());
    if (jdoc.isNull())
        return false;
    QJsonObject obj = jdoc.object();

    QString qstr = obj.value("log").toString();
    Push_LogInfoItem(qstr);
    std::string qqq2 = qstr.toStdString();
    char* qqq = qstr.toLocal8Bit().data();
    return true;
}

bool CRealDataManager::UnpackReboot(const char * pBuf, Juint32 len)
{
    QByteArray databuf;
    QByteArray ba((char*)pBuf, len);
    QJsonDocument jdoc = QJsonDocument::fromJson(ba.constData());
    if (jdoc.isNull())
        return false;
    QJsonObject obj = jdoc.object();

    m_RebootStatusCode = obj.value("statusCode").toInt();
    m_RebootStatusDesc = obj.value("statusDesc").toString().toStdString();

    emit emitSetReboot();
    return true;
}

void CRealDataManager::PackParam()
{

}

void CRealDataManager::PackStartLog()
{
}

void CRealDataManager::PackReboot()
{
}


CRealDataManager::CRealDataManager(QObject *parent)
{

}

CRealDataManager::~CRealDataManager()
{
    //std::map<Juint32, CTaskObject *>::iterator it = m_Taskobjs.begin();
    //for (; it != m_Taskobjs.end(); ++it)
    //{
    //    CTaskObject *info = it->second;
    //    if (info != NULL)
    //    {
    //        delete info;
    //        info = NULL;
    //    }
    //}
}

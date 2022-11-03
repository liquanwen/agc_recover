#include <QDebug> 
#include "real_data.h"
#include "mqtt_pub_inter.h"

volatile MQTTClient_deliveryToken deliveredtoken_inter;
MQTTClient_connectOptions m_conn_opts_inter = MQTTClient_connectOptions_initializer;

CMqttClientInterThread::CMqttClientInterThread(void)
: m_pMng(NULL)
{

}

CMqttClientInterThread::~CMqttClientInterThread()
{
}

void CMqttClientInterThread::SetPackageAcquireManager(CMqttClientInterManager * p)
{
	m_pMng = p;
}

void CMqttClientInterThread::run(void)
{
	m_pMng->thread_prev();
	while (CSL_thread::IsEnableRun())
	{
		m_pMng->thread_func();
	}
	m_pMng->thread_exit();
}

std::string CMqttClientInterThread::ThreadName(void) const
{
	return std::string("MQTT 内部Broker线程");
}

void CMqttClientInterThread::RecordLog(const std::string & sMsg)
{
    Std_LogWrite(eRunType, "CMqttClientInterThread RecordLog(%s).", sMsg.c_str());
}

//***************************************************************
//报文获取管理者
//***************************************************************
CMqttClientInterManager& CMqttClientInterManager::CreateInstance()
{
	static CMqttClientInterManager Mng;
	return Mng;
}

void CMqttClientInterManager::agent_Delivered(void *context, MQTTClient_deliveryToken dt)
{
    //SGDEV_INFO(SYSLOG_LOG, SGDEV_MODULE, "Message with token value %d delivery confirmed.", dt);
    deliveredtoken_inter = dt;
}

void CMqttClientInterManager::agent_ConnLost(void *context, char *cause)
{
    if (cause != NULL) 
    {
        Std_LogWrite(eErrType, "connLost(cause = %s).", cause);
        CMqttClientInterManager::CreateInstance().m_connectFlag = false;
    }
}

bool CMqttClientInterManager::Init(void)
{
	bool bRet(false);
	CIIPara myParam;
	std::string sTempValue;
    m_topics.clear();
    //m_topics.push_back(SET_PARAM_TOPIC);
    m_topics.push_back(SET_PARAM_TOPIC_RESPONSE);
   // m_topics.push_back(GET_PARAM_TOPIC);
    m_topics.push_back(GET_PARAM_TOPIC_RESPONSE);

    m_topics.push_back(GET_REAL_DATA_TOPIC);
    //m_topics.push_back(SET_LOG_TOPIC);
    m_topics.push_back(GET_LOG_TOPIC);
    //m_topics.push_back(SET_REBOOT_TOPIC);
    m_topics.push_back(GET_REBOOT_TOPIC);


    //m_topics.push_back("#");

    char str[256] = {0};
    SMTimeInfo local_time = ii_get_current_mtime();
    Juint32 iport = 1883;
    sprintf(str, "megskyagc%d%d%dstd", iport, local_time.nSecond, local_time.nMSecond);
    m_clientid = str;
    m_port = "1883";
	char server_uri[256] = {0};
	sprintf(server_uri, "tcp://%s:%s",m_server_addr.c_str(), m_port.c_str());

    Std_LogWrite(eRunType, "mqtt connect url =(%s).", server_uri);

    if (agent_mqtt_init((const char *)server_uri, m_clientid.c_str()) != true) {
        Std_LogWrite(eErrType, "agent mqtt init failed.");
        bRet = false;
    }

    //if (agent_mqtt_connect()) {    // 初始化成功后尝试一次连接
    //    if (agent_creatSubTopic()) {
    //        m_connectFlag = true;
    //    }
    //}

	bRet = m_Thread.start();
	return bRet;
    //return true;
}
bool CMqttClientInterManager::agent_mqtt_init(const char *server_uri, const char* client_id)
{
    CIIString iisTemp;

    int mqtt_ret;
    if (server_uri == NULL || client_id == NULL) {
        Std_LogWrite(eErrType, "MQTT Init param failed.");
        return false;
    }

    m_conn_opts_inter.keepAliveInterval = SG_KEEP_ALIVE_INTERVAL;
    m_conn_opts_inter.cleansession = 1;
    //options.setAutomaticReconnect(true);
    //if (m_user.size() != 0 && m_password.size() != 0) {  
    //    m_conn_opts_inter.username = m_user.c_str();
    //    m_conn_opts_inter.password = m_password.c_str();
    //}

    mqtt_ret = MQTTClient_create(&m_client, server_uri, client_id, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        Std_LogWrite(eErrType, "MQTTClient create failed(server_uri = %s,clientid = %s,ret = %d).", server_uri, client_id, mqtt_ret);
        return false;
    }

    Std_LogWrite(eRunType, "MQTTClient create success(server_uri = %s,clientid = %s).", server_uri, client_id);

    mqtt_ret = MQTTClient_setCallbacks(m_client, NULL, agent_ConnLost, agent_MqttMsgArrvd, agent_Delivered);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        agent_mqtt_destroy();
        return false;
    }

    Std_LogWrite(eRunType, "agent mqtt init success(server_uri = %s,clientid = %s).", server_uri, client_id);
    return true;
}
void CMqttClientInterManager::Exit(void)
{	
	m_Thread.close();
	m_Thread.wait_for_end();
	agent_destrySubTopic();
    agent_mqtt_disconnect();
    agent_mqtt_destroy();
	m_mqttSendQueue.Clear();
	m_mqttRecvQueue.Clear();
}

void CMqttClientInterManager::thread_prev(void)
{

}

void CMqttClientInterManager::thread_func(void)
{
	OnRun();
}

void CMqttClientInterManager::thread_exit(void)
{

}
void CMqttClientInterManager::OnRun(void)
{
	mqtt_data_info_s Sitem;
	mqtt_data_info_s Ritem;

    if (m_StartconnectFlag && (!getMqttConnect()))
    {
        agent_mqtt_connect(m_server_addr.c_str());
    } 
    if ((!m_StartconnectFlag) && getMqttConnect())
    {
        agent_mqtt_disconnect();
    }

    if(Get_SendItem(Sitem))
	{
        agent_MqttMsgPub((char*)Sitem.msg_send.c_str(), (char*)Sitem.pubtopic.c_str(),Sitem.retained); //取到发送队列数据后发送
	}

    while (Get_RecvItem(Ritem))        
    {
        CRealDataManager::CreateInstance().UnpackData(Ritem);        //处理收到的数据

        qDebug() << "Date:" << CEPTime::LocalTime().ToString().ToChar();
        qDebug() << "msg_send:" << Ritem.msg_send.c_str();
        qDebug() << "pubtopic:" << Ritem.pubtopic.c_str();
    }
    ii_sleep(10);

}

void CMqttClientInterManager::Start(void)
{

}

void CMqttClientInterManager::Stop(void)
{
	m_mqttSendQueue.Clear();
	m_mqttRecvQueue.Clear();

}

bool CMqttClientInterManager::getMqttConnect(void)
{
    if (!MQTTClient_isConnected(m_client))
    {
        m_connectFlag = false;
    }
    else 
    {
        m_connectFlag = true;
    }
    return m_connectFlag;
}

bool CMqttClientInterManager::agent_mqtt_connect(const char *server_uri)
{
    int mqtt_ret = VOS_OK;
    m_connectFlag = false;
    if (m_client == NULL) {
        Std_LogWrite(eErrType, "mqtt client id handle invalid).");
        return false;
    }
    char server_addr[256] = { 0 };
    sprintf(server_addr, "%s", m_server_addr.c_str());
    m_serverURIs[0] = (char*)server_addr;
  
    m_conn_opts_inter.connectTimeout = 5;
    m_conn_opts_inter.serverURIcount = 1;
    m_conn_opts_inter.serverURIs = m_serverURIs;
    //m_conn_opts_inter.returned.serverURI = server_uri;
    Std_LogWrite(eRunType, "mqtt connecting ....");
    mqtt_ret = MQTTClient_connect(m_client, &m_conn_opts_inter);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        Std_LogWrite(eErrType,"mqtt connect failed(ret = %d)).", mqtt_ret);
        m_connectFlag = false;
        return false;
    }
    agent_creatSubTopic();
    Std_LogWrite(eRunType, "mqtt connect success.");
    m_connectFlag = true;
    return true;
}

void CMqttClientInterManager::agent_mqtt_disconnect(void)
{
    int mqtt_ret;

    if (m_client != NULL) {
        mqtt_ret = MQTTClient_disconnect(m_client, MQTT_DISCONNECT_TIMEOUT);
        m_connectFlag = false;
        if (mqtt_ret != MQTTCLIENT_SUCCESS) {
            Std_LogWrite(eRunType, "mqtt disconnect error.");
        }
    }
}

void CMqttClientInterManager::agent_mqtt_destroy(void)
{
    if (m_client != NULL) {
        MQTTClient_destroy(&m_client);
        Std_LogWrite(eRunType, "mqtt destroy.");
        m_client = NULL;
    }
}

bool CMqttClientInterManager::agent_MqttMsgPub(char* msg_send, char* pub_topic, int retained)
{
    int mqtt_ret = 0;
    int msglen = 0;
    CIIString iisTemp;
    MQTTClient_deliveryToken token;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    if (msg_send == NULL || pub_topic == NULL) {
        Std_LogWrite(eErrType, "mqtt publish param msg_send or pubtopicinvalid.");
        return false;
    }

    if (m_client == NULL) {
        Std_LogWrite(eErrType, "mqtt client id handle invalid.");
        return false;
    }

    if (!MQTTClient_isConnected(m_client))
    {
        Std_LogWrite(eErrType, "mqtt client disconnected.");
        m_connectFlag = false;
    }
    else {
        m_connectFlag = true;
    }
    
    if (m_connectFlag == false)
    {
        return true;
    }
    msglen = (int)strlen(msg_send) + 1;
    pubmsg.payload = msg_send;
    pubmsg.payloadlen = msglen;
    pubmsg.qos = QOS;
    // if (strcmp(pub_topic, "") == 0)
    pubmsg.retained = retained;

    mqtt_ret = MQTTClient_publishMessage(m_client, pub_topic, &pubmsg, &token);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        iisTemp.Format("mqtt publish failed.(topic=%s, ret=%d)", pub_topic, mqtt_ret);
        Std_LogWrite(eErrType, iisTemp.GetBuf());
        return false;
    }

    mqtt_ret = MQTTClient_waitForCompletion(m_client, token, TIMEOUT);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        iisTemp.Format("MQTTClient waitForCompletion ret=%d).", mqtt_ret);
        Std_LogWrite(eErrType, iisTemp.GetBuf());
        return false;
    }
    return true; 
}

int CMqttClientInterManager::agent_MqttMsgArrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    CIIAutoMutex lock(&CMqttClientInterManager::CreateInstance().m_cs);
    char *content_str = NULL;
    if (message == NULL) {
        Std_LogWrite(eErrType, "mqtt msg is null.");
        MQTTClient_free(topicName);
        return 1;
    }

    if (message->payloadlen > 0) 
    {
        content_str = (char *)malloc((size_t)(message->payloadlen + 1));
        if (content_str) {
            memcpy(content_str, message->payload, (size_t)(message->payloadlen));
            content_str[message->payloadlen] = 0;
            mqtt_data_info_s item = {0};
            item.pubtopic = topicName;
            item.msg_send = content_str;
            item.msg_send_lenth = message->payloadlen;
            CMqttClientInterManager::CreateInstance().Push_RecvItem(item); 
            (void)memset(content_str, 0, (size_t)(message->payloadlen + 1));
            (void)memset(message->payload, 0, (size_t)(message->payloadlen));
            free(content_str);
            content_str = NULL;
        } else {
            Std_LogWrite(eErrType, "mqtt msg malloc failed.");
        }

    }
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

bool CMqttClientInterManager::agent_mqtt_msg_subscribe(char* topic, int qos)
{
    int mqtt_ret;
    if (topic == NULL) {
        Std_LogWrite(eErrType, "mqtt subscribe topic invalid.");
        return false;
    }

    if (strlen(topic) > 256) {
        Std_LogWrite(eErrType, "mqtt subscribe topic over max 256.");
        return false;
    }

    if (m_client == NULL) {
        Std_LogWrite(eErrType, "mqtt subscribe client id handle invalid).");
        return false;
    }

    mqtt_ret = MQTTClient_subscribe(m_client, topic, qos);
    if (mqtt_ret != MQTTCLIENT_SUCCESS) {
        Std_LogWrite(eErrType,"mqtt subscribe failed(ret = %d,pub_topic = %s)).", mqtt_ret, topic);
        return false;
    }
    Std_LogWrite(eRunType, "mqtt subscribe succeed(pub_topic = %s)).", topic);
    return true;
}

bool CMqttClientInterManager::agent_creatSubTopic(void)
{
    bool ret = true;
    std::list<std::string>::iterator it = m_topics.begin();
    for (; it != m_topics.end(); ++it)
    {
        if (!agent_mqtt_msg_subscribe((char *)(*it).c_str(), QOS)) 
        {
            ret = false;
        }
    }

    return ret;
}
void CMqttClientInterManager::agent_destrySubTopic()
{
    std::list<std::string>::iterator it = m_topics.begin();
    for (; it != m_topics.end(); ++it)
    {
        MQTTClient_unsubscribe(m_client, (*it).c_str());
    }

}



Juint32 CMqttClientInterManager::MqttGetRandomUuid(char * pucUuid, Juint32 ulLen)
{
    Juint32 ulIndex;
    Juint32 randomNum;
    char * pucUuidItem = pucUuid;
    
    if (NULL == pucUuid)
    {
        Std_LogWrite(eErrType, "VOS_GetRandomUuid in put invalid.\n");
        return VOS_ERR;
    }
    
    srand(time(0));
    for (ulIndex = 0; ulIndex < 16; ulIndex++)
    {
        randomNum = (rand()%255);
        sprintf(pucUuidItem, "%02x", randomNum);
        pucUuidItem+=2;

        switch(ulIndex)
        {
            /* 3 5 7 9 都会执行添加- */
            case 3:
            case 5:
            case 7:
            case 9:
                *pucUuidItem++ = '-';
                break;
        }
    }
    *pucUuidItem = '\0';

    return VOS_OK;
}
/*****************************************************************************
 函 数 名  : MqttGenerateToken
 功能描述  : 生成token
 输入参数  : 
 输出参数  : 
 返 回 值  : 
 调用函数  :
 被调函数  :
*****************************************************************************/
int CMqttClientInterManager::MqttGenerateToken(char* token, size_t buf_len)
{

    if (MQTT_OK != MqttGetRandomUuid(token, buf_len))
    {
        Std_LogWrite(eErrType, "generate token failed.");
        return MQTT_ERR;
    }
    //MG_LOG("generated token [%s]\n", token);
    return MQTT_OK;
}
int CMqttClientInterManager::MqttHeaderFill(mqtt_header_s* header, const char* request_token)
{
    //time_t now_time;
    int ret = MQTT_ERR;

    if (request_token) //如果请求token非空，说明现在填充的是响应中的token，响应token与请求token相同
    {
        (void)memcpy(header->token, request_token, sizeof(header->token));
    }

    else
    {
        //MG_LOG("\nheader token len is %d.\n", sizeof(header->token));
        ret = MqttGenerateToken(header->token, sizeof(header->token));
        if (ret != MQTT_OK)
        {
            Std_LogWrite(eErrType, "\nFill json header failed for generate token.\n");
            return MQTT_ERR;
        }
    }
    
    //使用UTC时间
    ret = MqttTimeStr(header->timestamp, sizeof(header->timestamp));
    if (ret != MQTT_OK)
    {
        Std_LogWrite(eErrType, "\nFill json header failed for generate time str.\n");
        return MQTT_ERR;
    }
    return MQTT_OK;
}


/*****************************************************************************
 函 数 名  : MqttTimeStr
 功能描述  : 生成时间戳
 输入参数  : 
 输出参数  : 
 返 回 值  : 
 调用函数  :
 被调函数  :
*****************************************************************************/
int CMqttClientInterManager::MqttTimeStr(char* time_buff, size_t buff_len)
{
    int ret;     

    
    SMTimeInfo local_time = ii_get_current_mtime();
    ret = snprintf(time_buff,buff_len,"%04d-%02d-%02dT%02d:%02d:%02d.%03d+0800", 
    local_time.nYear,
    local_time.nMonth,
    local_time.nDay,
    local_time.nHour,
    local_time.nMinute,
    local_time.nSecond,
    local_time.nMSecond);

    return ret >0 ? MQTT_OK : MQTT_ERR;
}

void CMqttClientInterManager::Push_RecvItem(mqtt_data_info_s item)
{
	m_mqttRecvQueue.AddTail(item);
}

bool CMqttClientInterManager::Get_RecvItem(mqtt_data_info_s& item)
{
	return m_mqttRecvQueue.GetHead(item);
}

void CMqttClientInterManager::Push_SendItem(mqtt_data_info_s item)
{
	m_mqttSendQueue.AddTail(item);
}

bool CMqttClientInterManager::Get_SendItem(mqtt_data_info_s& item)
{
	return m_mqttSendQueue.GetHead(item);
}

bool CMqttClientInterManager::win_mqtt_connect(const char * server_uri)
{
    m_server_addr = server_uri;
    m_StartconnectFlag = true;
    return true;
}

void CMqttClientInterManager::win_mqtt_disconnect(void)
{
    m_StartconnectFlag = false;
}

CMqttClientInterManager::CMqttClientInterManager()
{
	m_Thread.SetPackageAcquireManager(this);
	m_mqttSendQueue.Clear();
	m_mqttRecvQueue.Clear();
    m_topics.clear();   

    m_serverURIs = (char**)malloc(sizeof(char*) * 2);
	m_T.SetParam(200* 1000); // 默认200秒
    m_client = NULL;
}

CMqttClientInterManager::~CMqttClientInterManager()
{
    if (m_serverURIs != NULL) {
        free(m_serverURIs);
    }
}

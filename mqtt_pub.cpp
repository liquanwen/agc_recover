
#include "mqtt_pub.h"
#include <QDebug> 

volatile MQTTClient_deliveryToken deliveredtoken;

CPackageMQTTThread::CPackageMQTTThread(void)
: m_pMng(NULL)
{

}

CPackageMQTTThread::~CPackageMQTTThread()
{
}

void CPackageMQTTThread::SetPackageAcquireManager(CPackageMQTT * p)
{
	m_pMng = p;
}

void CPackageMQTTThread::run(void)
{
	m_pMng->thread_prev();
	while (CSL_thread::IsEnableRun())
	{
		m_pMng->thread_func();
	}
	m_pMng->thread_exit();
}

std::string CPackageMQTTThread::ThreadName(void) const
{
	return std::string("报文获取线程");
}

void CPackageMQTTThread::RecordLog(const std::string & sMsg)
{
}


//***************************************************************
//报文获取管理者
//***************************************************************
CPackageMQTT& CPackageMQTT::CreateInstance()
{
	static CPackageMQTT Mng;
	return Mng;
}

void CPackageMQTT::Delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("\nMessage with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

void CPackageMQTT::ConnLost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    int rc = 0;
    if (cause)
		printf("     cause: %s\n", cause);

	printf("Reconnecting\n");

    // CPackageMQTT::CreateInstance().destrySubTopic();
    // MQTTClient_disconnect(CPackageMQTT::CreateInstance().m_client, 10000);

    // MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    // conn_opts.keepAliveInterval = 20;
    // conn_opts.cleansession = 1;
    
    // if ((rc = MQTTClient_connect(CPackageMQTT::CreateInstance().m_client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    // {
    //     printf("\nFailed to connect, return code %d\n", rc);
    // }

	// CPackageMQTT::CreateInstance().creatSubTopic();
}

bool CPackageMQTT::Init(void)
{

	bool bRet(false);
	// int nTempValue(0);
	CIIPara myParam;
	std::string sTempValue;

	std::string szIniFilePath = "mqtt_proxy_config.ini";
	myParam.SetFileName(szIniFilePath.c_str());


	sTempValue = myParam.GetString("mqtt", "port", "1883", &bRet).GetBuf();
	if (!bRet)
		myParam.SetString("mqtt" , "port", "1883");
	m_port = sTempValue;

	sTempValue = myParam.GetString("mqtt", "IP", "127.0.0.1", &bRet).GetBuf();
	if (!bRet)
		myParam.SetString("mqtt" , "IP", "127.0.0.1");
	m_ip = sTempValue;


	
	char szTemp[256] = {0};
	sprintf(szTemp, "tcp://%s:%s","192.168.1.101", m_port.c_str());
    printf("url == %s\n",szTemp);
	int rc;
    MQTTClient_create(&m_client, szTemp, G_CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_setCallbacks(m_client, NULL, ConnLost, MqttMsgArrvd, Delivered);

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(m_client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("\nFailed to connect, return code %d\n", rc);
        return bRet;
    }


	//订阅 
	creatSubTopic();
	bRet = m_Thread.start();
	return bRet;
}

void CPackageMQTT::Exit(void)
{	

	m_Thread.close();
	m_Thread.wait_for_end();

	destrySubTopic();
	MQTTClient_disconnect(m_client, 10000);
    MQTTClient_destroy(&m_client);


}

void CPackageMQTT::thread_prev(void)
{

}

void CPackageMQTT::thread_func(void)
{
	OnRun();
}

void CPackageMQTT::thread_exit(void)
{

}
void CPackageMQTT::OnRun(void)
{

 //   Juint8 type;
 //   Juint32 len;
	//// char* asd = "11";
 //   std::string topicstr;
 //   if(Get_SendItem(Sitem))
	//{
	//	MqttMsgPub(Sitem.msg_send,Sitem.pubtopic);
	//}

	//if(Get_RecvItem(Ritem))
 //   {
 //       topicstr = Ritem.pubtopic;
 //       if(topicstr.compare("/v1/esn/device/elv") == 0)
 //       {
 //           type = CMD_ELV;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/gps") == 0)
 //       {
 //           type = CMD_GPS;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("esdk/get/response/ethapp/deviceInfo") == 0)
 //       {
 //           type = CMD_DEV;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/lte") == 0)
 //       {
 //           type = CMD_LTE;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/hplc") == 0)
 //       {
 //           type = CMD_HPLC;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/jc") == 0)
 //       {
 //           type = CMD_JC;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/other") == 0)
 //       {
 //           type = CMD_OTHER;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/ble") == 0)
 //       {
 //           type = CMD_BLE;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/manual") == 0)
 //       {
 //           type = CMD_MAN;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/cmdret/elv") == 0)
 //       {
 //           type = CMD_RET_ELV;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/cmdret/gps") == 0)
 //       {
 //           type = CMD_RET_GPS;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/cmdret/ble") == 0)
 //       {
 //           type = CMD_RET_BLE;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/cmdret/lte") == 0)
 //       {
 //           type = CMD_RET_LTE;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/cmdret/hplc") == 0)
 //       {
 //           type = CMD_RET_HPLC;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/cmdret/jc") == 0)
 //       {
 //           type = CMD_RET_JC;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/cmdret/other") == 0)
 //       {
 //           type = CMD_RET_OTHER;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //       else if(topicstr.compare("/v1/esn/device/cmdret/manual") == 0)
 //       {
 //           type = CMD_RET_MAN;
 //           len = CItgRunDiagnotor::Instance().PackFrame(type,(Juint8*)Ritem.msg_send,Ritem.msg_send_lenth);
 //       }
 //   }
 //   
	ii_sleep(50);

}

void CPackageMQTT::Start(void)
{

}

void CPackageMQTT::Stop(void)
{
	//m_mqttSendQueue.Clear();
	//m_mqttRecvQueue.Clear();

}

void CPackageMQTT::MqttMsgPub(char* msg_send,char* pubtopic)
{
	int rc;
	int msglen;
	// char msg_send[MSG_ARRVD_MAX_LEN]={0};
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	MQTTClient_deliveryToken token; 
	// char pubtopic[256] = {0};  
	msglen = (int)strlen(msg_send);
	pubmsg.payload = msg_send;
	pubmsg.payloadlen = msglen;
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	MQTTClient_publishMessage(m_client, pubtopic, &pubmsg, &token);
	printf("\nPublishing topic %s: %s,len is %d.\n\n", pubtopic, (char *)pubmsg.payload, pubmsg.payloadlen);
	rc = MQTTClient_waitForCompletion(m_client, token, TIMEOUT);
	printf("\nMessage with delivery token %d delivered\n\n", token);  
}

int CPackageMQTT::MqttMsgArrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{

    int i;
    char* payloadptr;
    payloadptr = (char*)message->payload; 

    printf("\nMessage arrived\n");
    printf("topic: %s\n", topicName);
	qDebug() << payloadptr << topicName << endl;
 //	mqtt_data_info_s item;

	//sprintf(item.pubtopic,"%s",topicName);
	//sprintf(item.msg_send,"%s",payloadptr);
 //   item.msg_send_lenth = message->payloadlen;
	//收到主题
	//CPackageMQTT::CreateInstance().Push_RecvItem(item);


    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    printf("\n\n");
    
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}



void CPackageMQTT::creatSubTopic()
{
	bool bRet(false);
	// int nTempValue(0);
	CIIPara myParam;
	std::string sTempValue;

	std::string szIniFilePath = "mqtt_proxy_config.ini";
	myParam.SetFileName(szIniFilePath.c_str());


	sTempValue = myParam.GetString("mqtt", "version", "v1", &bRet).GetBuf();
	if (!bRet)
		myParam.SetString("mqtt" , "version", "v1");
	m_ver = sTempValue;

	sTempValue = myParam.GetString("mqtt", "deviceId", "T2312560300720200220012", &bRet).GetBuf();
	if (!bRet)
		myParam.SetString("mqtt" , "deviceId", "T2312560300720200220012");
	m_devid = sTempValue;

	
    
	sprintf(m_topic_elv, "SdeeJc/notify/event/database/ADC/ADC_5c313e0d2d6ea361"); 		
	//printf("\nSubscribing to topic :%s\n", m_topic_elv);
    MQTTClient_subscribe(m_client, m_topic_elv, QOS);

	sprintf(m_topic_gps, "/v1/esn/device/gps"); 			
    //printf("\nSubscribing to topic :%s\n", m_topic_gps);
    MQTTClient_subscribe(m_client, m_topic_gps, QOS);

    sprintf(m_topic_deviceInfo, "esdk/get/response/ethapp/deviceInfo"); 			
    //printf("\nSubscribing to topic :%s\n", m_topic_deviceInfo);
    MQTTClient_subscribe(m_client, m_topic_deviceInfo, QOS);

    sprintf(m_topic_deviceInfopub, "ethapp/get/request/esdk/deviceInfo"); 	


    sprintf(m_topic_hplc, "/v1/esn/device/hplc"); 			
    //printf("\nSubscribing to topic :%s\n", m_topic_hplc);
    MQTTClient_subscribe(m_client, m_topic_hplc, QOS);

    sprintf(m_topic_lte, "/v1/esn/device/lte"); 			
    //printf("\nSubscribing to topic :%s\n", m_topic_lte);
    MQTTClient_subscribe(m_client, m_topic_lte, QOS);

    sprintf(m_topic_jc, "/v1/esn/device/jc"); 			
    //printf("\nSubscribing to topic :%s\n", m_topic_jc);
    MQTTClient_subscribe(m_client, m_topic_jc, QOS);

    sprintf(m_topic_other, "/v1/esn/device/other"); 			
    //printf("\nSubscribing to topic :%s\n", m_topic_other);
    MQTTClient_subscribe(m_client, m_topic_other, QOS);

    sprintf(m_topic_ble, "/v1/esn/device/ble"); 			
    //printf("\nSubscribing to topic :%s\n", m_topic_ble);
    MQTTClient_subscribe(m_client, m_topic_ble, QOS);

    sprintf(m_topic_manual, "/v1/esn/device/manual"); 			
    //printf("\nSubscribing to topic :%s\n", m_topic_manual);
    MQTTClient_subscribe(m_client, m_topic_manual, QOS);

    MQTTClient_subscribe(m_client, "/v1/esn/device/cmdret/elv", QOS); 
    MQTTClient_subscribe(m_client, "/v1/esn/device/cmdret/gps", QOS);  
    MQTTClient_subscribe(m_client, "/v1/esn/device/cmdret/ble", QOS); 
    MQTTClient_subscribe(m_client, "/v1/esn/device/cmdret/hplc", QOS);   
    MQTTClient_subscribe(m_client, "/v1/esn/device/cmdret/lte", QOS);  
    MQTTClient_subscribe(m_client, "/v1/esn/device/cmdret/jc", QOS); 
    MQTTClient_subscribe(m_client, "/v1/esn/device/cmdret/other", QOS); 
    MQTTClient_subscribe(m_client, "/v1/esn/device/cmdret/manual", QOS); 

}

void CPackageMQTT::destrySubTopic()
{
    MQTTClient_unsubscribe(m_client, m_topic_elv);
    MQTTClient_unsubscribe(m_client, m_topic_gps);
    MQTTClient_unsubscribe(m_client, m_topic_deviceInfo);
    MQTTClient_unsubscribe(m_client, m_topic_hplc);
    MQTTClient_unsubscribe(m_client, m_topic_lte);
    MQTTClient_unsubscribe(m_client, m_topic_jc);
    MQTTClient_unsubscribe(m_client, m_topic_other);
    MQTTClient_unsubscribe(m_client, m_topic_manual);
    MQTTClient_unsubscribe(m_client, m_topic_ble);

    MQTTClient_unsubscribe(m_client, "/v1/esn/device/cmdret/elv"); 
    MQTTClient_unsubscribe(m_client, "/v1/esn/device/cmdret/gps");  
    MQTTClient_unsubscribe(m_client, "/v1/esn/device/cmdret/ble"); 
    MQTTClient_unsubscribe(m_client, "/v1/esn/device/cmdret/hplc");   
    MQTTClient_unsubscribe(m_client, "/v1/esn/device/cmdret/lte");  
    MQTTClient_unsubscribe(m_client, "/v1/esn/device/cmdret/jc"); 
    MQTTClient_unsubscribe(m_client, "/v1/esn/device/cmdret/other"); 
    MQTTClient_unsubscribe(m_client, "/v1/esn/device/cmdret/manual"); 
}

CPackageMQTT::CPackageMQTT()
{
	m_Thread.SetPackageAcquireManager(this);




}

CPackageMQTT::~CPackageMQTT()
{

}

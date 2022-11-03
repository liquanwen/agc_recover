

/*=====================================================================
 * 文件：mqtt_pub_inter.h
 *
 * 描述：mqtt订阅和发布线程处理
 *
 * 作者：田振超			2021年4月29日09:53:29
 * 
 * 修改记录：
 =====================================================================*/


#ifndef MQTT_PUB_INTER_H
#define MQTT_PUB_INTER_H

#include "public_struct.h"
#include "MQTTClient.h"
#include "pub_thread.h"
#include "pub_logfile.h"
#include "queue_model.h"

//====================================================================================
//线程
//=====================================================================================
class CMqttClientInterManager;
class CMqttClientInterThread : public CSL_thread
{
public:
	CMqttClientInterThread();
	virtual~CMqttClientInterThread();
	void SetPackageAcquireManager(CMqttClientInterManager * p);

	virtual void run(void);
	virtual std::string ThreadName(void) const;
	virtual void RecordLog(const std::string & sMsg);

private:
	CMqttClientInterManager * m_pMng;
};

//=====================================================================================
//管理者
//=====================================================================================
class CMqttClientInterManager
{
public:
	static CMqttClientInterManager & CreateInstance();

	bool Init(void);
	void Exit(void);

	void thread_prev(void);
	void thread_func(void);
	void thread_exit(void);

	void OnRun(void);
	void Start(void);
	void Stop(void);

    bool getMqttConnect(void);

	void Push_RecvItem(mqtt_data_info_s item);		//回调函数中使用
	bool Get_RecvItem(mqtt_data_info_s& item);		//主逻辑线程中调用

	void Push_SendItem(mqtt_data_info_s item);	    //逻辑转换时调用，先调用mqtt组帧接口
	bool Get_SendItem(mqtt_data_info_s& item);	    //mqtt发布线程调用

    bool win_mqtt_connect(const char *server_uri);
    void win_mqtt_disconnect(void);

    bool agent_mqtt_connect(const char *server_uri);
    void agent_mqtt_disconnect(void);
	bool agent_creatSubTopic();
    bool agent_MqttMsgPub(char* msg_send,char* pubtopic, int retained);
	bool agent_mqtt_msg_subscribe(char* topic, int qos);
    void agent_destrySubTopic();
private:

    bool agent_mqtt_init(const char *server_uri, const char* client_id);
	static int agent_MqttMsgArrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);

	static void agent_Delivered(void *context, MQTTClient_deliveryToken dt);
	static void agent_ConnLost(void *context, char *cause);
    void agent_mqtt_destroy(void);
	CMqttClientInterManager();
	~CMqttClientInterManager();

	CMqttClientInterManager& operator = (const CMqttClientInterManager&);
	CMqttClientInterManager(const CMqttClientInterManager&);

public:
	Juint32 MqttGetRandomUuid(char * pucUuid, Juint32 ulLen);
	int MqttGenerateToken(char* token, size_t buf_len);
	int MqttTimeStr(char* time_buff, size_t buff_len);
public:
	int MqttHeaderFill(mqtt_header_s* header, const char* request_token);

public:
	MQTTClient 								m_client;
	bool 								    m_connectFlag;
private:
	CIIMutex	                   		 	m_cs;               //  备用
	CMqttClientInterThread           		m_Thread;           //  线程     
    CpacketQueue<mqtt_data_info_s>		   	m_mqttSendQueue;    //mqtt发送数据队列
    CpacketQueue<mqtt_data_info_s>			m_mqttRecvQueue;	//mqtt接收数据队列
    char**                                  m_serverURIs;
    std::string                             m_server_addr;
    bool 								    m_StartconnectFlag;
    CTimerCnt                               m_T;				// 计时器 备用
    std::string                             m_ip;
    std::string 							m_port;
    std::string 							m_user;
    std::string								m_password;
    std::string								m_clientid;
public:
    std::list<std::string>                  m_topics;           // 读取参数 获取所有订阅的主题列表
    char 									m_topic_other[256];

    friend class CMqttClientInterThread;

};

#endif

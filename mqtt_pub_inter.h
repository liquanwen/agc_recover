

/*=====================================================================
 * �ļ���mqtt_pub_inter.h
 *
 * ������mqtt���ĺͷ����̴߳���
 *
 * ���ߣ�����			2021��4��29��09:53:29
 * 
 * �޸ļ�¼��
 =====================================================================*/


#ifndef MQTT_PUB_INTER_H
#define MQTT_PUB_INTER_H

#include "public_struct.h"
#include "MQTTClient.h"
#include "pub_thread.h"
#include "pub_logfile.h"
#include "queue_model.h"

//====================================================================================
//�߳�
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
//������
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

	void Push_RecvItem(mqtt_data_info_s item);		//�ص�������ʹ��
	bool Get_RecvItem(mqtt_data_info_s& item);		//���߼��߳��е���

	void Push_SendItem(mqtt_data_info_s item);	    //�߼�ת��ʱ���ã��ȵ���mqtt��֡�ӿ�
	bool Get_SendItem(mqtt_data_info_s& item);	    //mqtt�����̵߳���

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
	CIIMutex	                   		 	m_cs;               //  ����
	CMqttClientInterThread           		m_Thread;           //  �߳�     
    CpacketQueue<mqtt_data_info_s>		   	m_mqttSendQueue;    //mqtt�������ݶ���
    CpacketQueue<mqtt_data_info_s>			m_mqttRecvQueue;	//mqtt�������ݶ���
    char**                                  m_serverURIs;
    std::string                             m_server_addr;
    bool 								    m_StartconnectFlag;
    CTimerCnt                               m_T;				// ��ʱ�� ����
    std::string                             m_ip;
    std::string 							m_port;
    std::string 							m_user;
    std::string								m_password;
    std::string								m_clientid;
public:
    std::list<std::string>                  m_topics;           // ��ȡ���� ��ȡ���ж��ĵ������б�
    char 									m_topic_other[256];

    friend class CMqttClientInterThread;

};

#endif



/*=====================================================================
 * �ļ���mqtt_pub.h
 *
 * ������mqtt�����߳�
 *
 * ���ߣ�����			2020��9��15��10:18:38
 * 
 * �޸ļ�¼��
 =====================================================================*/


#ifndef MQTT_PUB_H
#define MQTT_PUB_H


#include "MQTTClient.h"
#include "pub_thread.h"

#define ADDRESS     "tcp://127.0.0.1:1883"
#define QOS         0
#define TIMEOUT     10000L
#define G_CLIENTID  "msk_data_pub" 
#define MSG_ARRVD_MAX_LEN   3*1024

//====================================================================================
//�߳�
//=====================================================================================
class CPackageMQTT;
class CPackageMQTTThread : public CSL_thread
{
public:
	CPackageMQTTThread();
	virtual~CPackageMQTTThread();
	void SetPackageAcquireManager(CPackageMQTT * p);

	virtual void run(void);
	virtual std::string ThreadName(void) const;
	virtual void RecordLog(const std::string & sMsg);

private:
	CPackageMQTT * m_pMng;
};
//=====================================================================================
//������
//=====================================================================================
class CPackageMQTT
{
public:
	static CPackageMQTT & CreateInstance();

	bool Init(void);
	void Exit(void);

	void thread_prev(void);
	void thread_func(void);
	void thread_exit(void);

	void OnRun(void);
	void Start(void);
	void Stop(void);


private:

    void destrySubTopic();
	void creatSubTopic();
	static int MqttMsgArrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
	void MqttMsgPub(char* msg_send,char* pubtopic);
	static void Delivered(void *context, MQTTClient_deliveryToken dt);
	static void ConnLost(void *context, char *cause);

	
	//����
	CPackageMQTT();
	~CPackageMQTT();

	CPackageMQTT& operator = (const CPackageMQTT&);
	CPackageMQTT(const CPackageMQTT&);
    
public:
	MQTTClient 								m_client;

private:
	CIIMutex	                   		 	m_cs;
	CPackageMQTTThread           		m_Thread;           //���Ĳɼ��߳�

	//ÿһ������һ�����У�



	// static  volatile MQTTClient_deliveryToken 		m_deliveredtoken;

	std::string 							m_ip;
	std::string 							m_port;
	std::string 							m_ver;
	std::string								m_devid;
	//���ݽṹ����
	friend class CPackageMQTTThread;
public:	

	char 									m_topic_elv[256];
	char 									m_topic_gps[256];
	char 									m_topic_deviceInfo[256];
	char 									m_topic_hplc[256];
	char 									m_topic_lte[256];
	char 									m_topic_jc[256];
	char 									m_topic_other[256];
    char 									m_topic_ble[256];
    char 									m_topic_manual[256];
    char                                    m_topic_deviceInfopub[256];
	
};

#endif

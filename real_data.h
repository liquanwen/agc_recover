

/*=====================================================================
 * �ļ���real_data.h
 *
 * ���������ݴ洢
 *
 * ���ߣ�����       2022��3��17��08:45:43
 *
 * �޸ļ�¼��
 =====================================================================*/

#ifndef REAL_DATA_H
#define REAL_DATA_H

#include "public_struct.h"
#include "pub_thread.h"
#include "queue_model.h"
#include <QtWidgets>
#include <QObject>

// ������ṹ
typedef struct InverterInfoData
{
	Juint32					sn;             //  �������ţ���1��ʼ
	std::string 			name;           //  ��������ƣ�Ӣ�ģ�
	Juint32					yc_sort;        //  �����ң��ֵ���0��ʼ�������ʵʱ����
	Juint32					yx_sort;        //  �����ң��ֵ���0��ʼ��ң��Ϊ1���ָ澯��������������п���
	Juint32					yk_sort;        //  �����ң��ֵ���0��ʼ������������Ƿ�����
	Juint32					yt_sort;        //  �����ң��ֵ���0��ʼ
    Juint32                 on_sort;        //  �������ʵ��ڵ��    add by tianzhenchao
    Juint32                 off_sort;       //  �رչ��ʵ��ڵ��    add by tianzhenchao
    Juint32                 group_num;      //  ������ ����ͬʱң��
	Juint32					status_sort;    //  �����״̬��ң�ŵ��
	Juint32 				priority;       //  ��������ȼ���1���
	bool 			        fenable;        //  �����ʹ��true:�ɿأ�false:���ɿ�
	Jfloat 					rated_power;    //  ����������
	Jfloat 					set_popwer;     //  ��������ù���
    Juint32					curve_total;    //  �������ʷ�������ߵ����
    std::string				start_time;     //  �������ʷ�������߿�ʼʱ��
    std::string				end_time;       //  �������ʷ�������߽���ʱ��
	Jfloat					curve[1024];    //  ��ʷ�������ߣ�����"curve": [100, 100, 100������]
}InverterInfoData_s;

// ������ṹ
typedef struct InverterRealData
{
    Juint32					sn;             //  �������ţ���1��ʼ
    std::string 			name;           //  ��������ƣ�Ӣ�ģ�
    Jfloat 					real_popwer;     //  ���������
}InverterRealData_s;


//=====================================================================================
//ʵʱ���ݴ洢������
//=====================================================================================
//class CRealDataManager : public QObject
class CRealDataManager : public QObject
{
    Q_OBJECT
public:
    static CRealDataManager & CreateInstance();

    bool Init(void);
    void Exit(void);

    //��ʼ�������� 
    void InitData();
    bool UnpackData(mqtt_data_info_s &real_data);

	 //������־
    void OpenLog(QString flag);
	 //�����豸
    void RebootAgc(Juint32 delay);
	 //�·�����
    void SetParam();
    void ReadParam();
    void UpTool(double fvalue);

    void Push_LogInfoItem(QString item);    // 
    bool Get_LogInfoItem(QString& item);


signals:
    void emitUnpackParam();
    void emitSetParam();
    void emitRealData();
    void emitSetReboot();
    void onDataRefreshSuccess(const char* data, int len);
private:
    bool UnpackParam(const char * pBuf, Juint32 len);
    bool UnpackSetParam(const char * pBuf, Juint32 len);
    bool UnpackRealData(const char * pBuf, Juint32 len);
    bool UnpackLog(const char * pBuf, Juint32 len);
    bool UnpackReboot(const char * pBuf, Juint32 len);

    void PackParam();
    void PackStartLog();
    void PackReboot();

private:
    CRealDataManager(QObject *parent = 0);
    ~CRealDataManager();

    CRealDataManager& operator = (const CRealDataManager&);
    CRealDataManager(const CRealDataManager&);

private:
    CpacketQueue<QString>         	    m_LogQueue;          //  ��־����

public:
	bool									    m_agc_enable;           // AGCʹ�ܣ�true/false
    Juint32 									m_agc_strategy;         // ����ģʽ��ƽ�����ԣ��������ԣ�AI��ѧ��
    std::string 							    m_agc_getpower;         // AGC���ʻ�ȡ��ʽ��remote / local�� remote�����ؼ��㣬ͨ���ۼӸ���������ʻ��local��Զ�̣�ͨ��104��ȡ�ܹ���
    Jfloat 									    m_agc_setpower;         // ���ù��ʣ���λMW Ŀ�깦�ʣ�������վ���õĹ��ʣ�
    Jfloat 									    m_agc_dead_zone;        // Ŀ��ֵ����
    Jfloat 									    m_Inv_dead_zone;        // ���������������AGC���ڣ�
    bool                                        m_yt_flag;              // ִ�б�־λ false:ֱ��ִ�У�true��ѡ��ִ��
    Juint32                                     m_yt_type;              // ң���������� 48��һ����49��Ȼ���50����
    Jfloat                                      m_yt_coef;              //ң��ϵ��
    bool                                        m_template_enable;      // �Ƿ��������������true��false
    Juint32                                     m_template_sn;          // ������������
    bool                                        m_inverter_adjust_sw;   // �����ʹ���е��ڿ��أ�true������ǰ��Ҫ�Ƚ����ڿ��ش�
    Juint32                                     m_adjust_on_cmd;        // ���ڿ��ؿ����������� 10���ƣ�����0xAA��Ҫд170
    Juint32                                     m_adjust_off_cmd;       // ���ڿ��عر��������� 10���ƣ�����0x55��Ҫд85
    Juint32                                     m_generate_power_yc_sort;     // ʵʱ���繦��ң���ţ���agc_getpower = remoteʱ��Ч
    Juint32                                     m_merge_grid_yc_sort;         // �����㹦��ң����
    Juint32                                     m_inverter_total;           // ���������
    std::map<Juint32, InverterInfoData_s>       m_InfoDatas;                // ��ź�����

    //ʵʱ����
    Jfloat                                      m_cal_p_total;              // �ܹ���
    std::map<Juint32, InverterRealData_s>       m_RealDatas;                // ÿ̨�����ʵʱ����
    Juint32                                     m_statusCode;
    std::string                                 m_statusDesc;
    Juint32                                     m_RebootStatusCode;
    std::string                                 m_RebootStatusDesc;

	Juint16										m_agc_status;
	Juint16										m_adj_max;
	Juint16										m_adj_min;
	Juint16										m_ctrl_cap;
	Juint16										m_power_pre;
	Juint16										m_real_ret;
};

#endif

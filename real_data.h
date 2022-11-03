

/*=====================================================================
 * 文件：real_data.h
 *
 * 描述：数据存储
 *
 * 作者：田振超       2022年3月17日08:45:43
 *
 * 修改记录：
 =====================================================================*/

#ifndef REAL_DATA_H
#define REAL_DATA_H

#include "public_struct.h"
#include "pub_thread.h"
#include "queue_model.h"
#include <QtWidgets>
#include <QObject>

// 逆变器结构
typedef struct InverterInfoData
{
	Juint32					sn;             //  逆变器序号，从1开始
	std::string 			name;           //  逆变器名称（英文）
	Juint32					yc_sort;        //  逆变器遥测值点号0开始，逆变器实时功率
	Juint32					yx_sort;        //  逆变器遥信值点号0开始，遥信为1则发现告警，该逆变器不进行控制
	Juint32					yk_sort;        //  逆变器遥控值点号0开始，控制逆变器是否运行
	Juint32					yt_sort;        //  逆变器遥调值点号0开始
    Juint32                 on_sort;        //  开启功率调节点号    add by tianzhenchao
    Juint32                 off_sort;       //  关闭功率调节点号    add by tianzhenchao
    Juint32                 group_num;      //  所属组 用于同时遥控
	Juint32					status_sort;    //  逆变器状态的遥信点号
	Juint32 				priority;       //  逆变器优先级，1最高
	bool 			        fenable;        //  逆变器使能true:可控，false:不可控
	Jfloat 					rated_power;    //  逆变器额定功率
	Jfloat 					set_popwer;     //  逆变器设置功率
    Juint32					curve_total;    //  逆变器历史功率曲线点个数
    std::string				start_time;     //  逆变器历史功率曲线开始时间
    std::string				end_time;       //  逆变器历史功率曲线结束时间
	Jfloat					curve[1024];    //  历史功率曲线，例如"curve": [100, 100, 100………]
}InverterInfoData_s;

// 逆变器结构
typedef struct InverterRealData
{
    Juint32					sn;             //  逆变器序号，从1开始
    std::string 			name;           //  逆变器名称（英文）
    Jfloat 					real_popwer;     //  逆变器功率
}InverterRealData_s;


//=====================================================================================
//实时数据存储及解析
//=====================================================================================
//class CRealDataManager : public QObject
class CRealDataManager : public QObject
{
    Q_OBJECT
public:
    static CRealDataManager & CreateInstance();

    bool Init(void);
    void Exit(void);

    //初始化点数据 
    void InitData();
    bool UnpackData(mqtt_data_info_s &real_data);

	 //开启日志
    void OpenLog(QString flag);
	 //重启设备
    void RebootAgc(Juint32 delay);
	 //下发参数
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
    CpacketQueue<QString>         	    m_LogQueue;          //  日志队列

public:
	bool									    m_agc_enable;           // AGC使能，true/false
    Juint32 									m_agc_strategy;         // 策略模式：平均策略，主动策略，AI自学，
    std::string 							    m_agc_getpower;         // AGC功率获取方式，remote / local。 remote：本地计算，通过累加各逆变器功率获得local：远程，通过104获取总功率
    Jfloat 									    m_agc_setpower;         // 设置功率，单位MW 目标功率（调度主站设置的功率）
    Jfloat 									    m_agc_dead_zone;        // 目标值死区
    Jfloat 									    m_Inv_dead_zone;        // 逆变器功率死区（AGC调节）
    bool                                        m_yt_flag;              // 执行标志位 false:直接执行，true：选择执行
    Juint32                                     m_yt_type;              // 遥调数据类型 48归一化，49标度化，50浮点
    Jfloat                                      m_yt_coef;              //遥调系数
    bool                                        m_template_enable;      // 是否有样板逆变器，true和false
    Juint32                                     m_template_sn;          // 样板逆变器序号
    bool                                        m_inverter_adjust_sw;   // 逆变器使用有调节开关，true：调节前需要先将调节开关打开
    Juint32                                     m_adjust_on_cmd;        // 调节开关开启命令数据 10进制，例如0xAA需要写170
    Juint32                                     m_adjust_off_cmd;       // 调节开关关闭命令数据 10进制，例如0x55需要写85
    Juint32                                     m_generate_power_yc_sort;     // 实时发电功率遥测点号，当agc_getpower = remote时有效
    Juint32                                     m_merge_grid_yc_sort;         // 并网点功率遥测点号
    Juint32                                     m_inverter_total;           // 逆变器个数
    std::map<Juint32, InverterInfoData_s>       m_InfoDatas;                // 序号和内容

    //实时数据
    Jfloat                                      m_cal_p_total;              // 总功率
    std::map<Juint32, InverterRealData_s>       m_RealDatas;                // 每台逆变器实时数据
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

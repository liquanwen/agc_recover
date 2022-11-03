/*=====================================================================
 * 文件：public_struct.h
 *
 * 描述：公共定义的头文件定义
 *
 * 作者：田振超			2021年5月14日13:54:32
 * 
 * 修改记录：
 =====================================================================*/

#ifndef PUBLIC_STRUCT_H
#define PUBLIC_STRUCT_H


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "pub_std.h"
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MQTT订阅发布相关 */
#define QOS         0
#define TIMEOUT     10000L
#define MSG_ARRVD_MAX_LEN   32*1024
#define PATH_MAX 1024


const Juint32	QUEUE_LEN = 8192;

#define TCP_CLIENT_T 0
#define TCP_SERVER_T 1


#define XMAXVALUE 300
#define XMAXCOUNT 31
#define HorizontalAxis 15
#define VerticalAxis 12

int memcpy_safe(void *s1, int s1_size, void *s2, int s2_size, int n);
int memcpy_safe_revese(void *s1, int s1_size, void *s2, int s2_size, int n);
void Std_LogWrite(ELogType eLogType, const char * format, ...);

/* 基本数据类型和常量定义 */
#define TOKEN_RELEASE 0
#define TOKEN_SHUT 1


#define MQTT_OK 0
#define MQTT_ERR 1
#define VOS_OK 0
#define VOS_ERR 1
#define DEVICE_DELETE       2
#define DEVICE_ONLINE       1
#define DEVICE_OFFLINE      0

#define DEV_INFO_MSG_BUFFER_LEN 1024
#define DATA_BUF_F256_SIZE 256
#define SG_KEEP_ALIVE_INTERVAL (60)
#define MQTT_DISCONNECT_TIMEOUT 10000

#define LOG_FILE_SIZE 5 //单位MB

/* 此APP名称 */
#define CFG_APP_NAME "HomeAGC"
#define CFG_AGC_NAME "AGC"

/* 设备重启延时时间，表示在多少秒之后重启，取值范围1~60s */
#define CFG_RST_DELAY 1

#define FLAG_NULL   0
#define FLAG_FAULT  1
#define FLAG_RECOVER  2

#define QUERY_PARAM   0
#define QUERY_DD  1
#define QUERY_YCYXOCT  2

#define EVENT_MORMAL   0
#define EVENT_NULL   1

//主题定义

//const char*  SET_PARAM_TOPIC = CFG_AGC_NAME "/set/request/" CFG_APP_NAME  "/param";
//const char*  GET_PARAM_TOPIC = CFG_AGC_NAME "/set/response/" CFG_APP_NAME "/param";
//const char*  GET_REAL_DATA_TOPIC =  CFG_AGC_NAME "/cycle/" CFG_APP_NAME "//data";
//const char*  SET_LOG_TOPIC =  CFG_AGC_NAME "/get/request/" CFG_APP_NAME "/log";
//const char*  GET_LOG_TOPIC =  CFG_AGC_NAME "/get/response/" CFG_APP_NAME "/log";
//const char*  SET_REBOOT_TOPIC = CFG_AGC_NAME "/set/request/" CFG_APP_NAME "/reboot";
//const char*  GET_REBOOT_TOPIC = CFG_AGC_NAME "/set/response/" CFG_APP_NAME "/reboot";

#define SET_PARAM_TOPIC CFG_APP_NAME "/set/request/" CFG_AGC_NAME  "/param"
#define SET_PARAM_TOPIC_RESPONSE CFG_AGC_NAME "/set/response/" CFG_APP_NAME "/param"

#define GET_PARAM_TOPIC CFG_APP_NAME "/get/request/" CFG_AGC_NAME  "/param"
#define GET_PARAM_TOPIC_RESPONSE CFG_AGC_NAME "/get/response/" CFG_APP_NAME "/param"

#define GET_TOOL_TOPIC CFG_APP_NAME "/set/request/" CFG_AGC_NAME "/power"
#define GET_TOOLRE_TOPIC CFG_AGC_NAME "/set/response/" CFG_APP_NAME "/power"


#define GET_REAL_DATA_TOPIC CFG_AGC_NAME "/cycle/" CFG_APP_NAME "/data"
#define SET_LOG_TOPIC CFG_APP_NAME "/get/request/" CFG_AGC_NAME "/log"
#define GET_LOG_TOPIC  CFG_AGC_NAME "/get/response/" CFG_APP_NAME "/log"
#define SET_REBOOT_TOPIC CFG_APP_NAME "/set/request/" CFG_AGC_NAME "/reboot"
#define GET_REBOOT_TOPIC CFG_AGC_NAME "/set/response/" CFG_APP_NAME "/reboot"

/*Start================MQTT定义=======================*/

/* MQTT消息通用部分结构体 */
typedef struct mqtt_header
{
    char        token[40];
    char        timestamp[32];
}mqtt_header_s;

//typedef struct mqtt_data_info
//{
//    Juint32 msg_send_lenth;
//	char msg_send[MSG_ARRVD_MAX_LEN];
//    char pubtopic[256];
//    int  retained;                  // 可保留的
//}mqtt_data_info_s;

typedef struct mqtt_data_info
{
    Juint32 msg_send_lenth;
	std::string  msg_send;
    std::string  pubtopic;
    int  retained;                  // 可保留的
}mqtt_data_info_s;

/*End================设备信息定义=======================*/

#define F_DESC(x) 1
#define SEM_MAX_WAIT_TIMES 20
#define MAX_QUE_DEPTH 4096
#define MSECOND 1000000
#define TEN_MSECOND 100000
#define REQUEST_TYPE 77
#define JSON_BUF_SIZE 256


//json文件格式存储

//设备结构体
typedef struct devBaseInfo_104
{
    Juint32         devNo;
	std::string     port;   //端口
    Juint32         addr;   //地址
    std::string     model;  //模型
    std::string     desc;
    std::string     protocol;   //协议MODBUS/DLT645/IEC101/IEC104/1376.1/1376.2/698.45
    std::string     manuID;     //厂商ID
    bool            isreport;          //整个设备数据是否上报标志 0 -不上报 1-上报
}devBaseInfo_104_s;

enum ETagType
{
    ETagType_Error = 0,
    ETagType_Boolean = 1,				        //布尔型
    ETagType_Tiny = 43,					        //小整型
    ETagType_UTiny = 32,				        //无符号小整型
    ETagType_Short = 33,				        //短整型
    ETagType_UShort = 45,				        //无符号短整型
    ETagType_Int = 2,					        //整型
    ETagType_Uint = 35,					        //无符号整型
    ETagType_Long = 36,					        //长整型
    ETagType_Ulong = 37,				        //无符号长整型
    ETagType_Float = 38,				        //单精度浮点
    ETagType_Double = 39,				        //双精度浮点
    ETagType_OcterString = 8,					//可变 OcterString    浙江新增
    ETagType_String = 4,					    //字符串类型
};

enum EParamResult
{
    EParamResult_FAILURE = 0,				// 失败
    EParamResult_SUCCESS = 1,				// 成功
    EParamResult_NULL = 2,				    // 空
};

enum TIMEUNIT
{
    TIMEUNIT_NULL = 0,				    // 
    TIMEUNIT_SECOND = 1,				    // 秒
    TIMEUNIT_MINUTE = 2,				    // 分钟
    TIMEUNIT_HOUR= 3,				        // 小时
    TIMEUNIT_DAY = 4,				    // 月
    TIMEUNIT_MOUNTH= 5,				        // 年
};


#ifdef __cplusplus
}
#endif

#endif



#include "public_struct.h"

static bool g_printf_enable = false;
void Std_LogWrite(ELogType eLogType, const char * format, ...)
{
    char m_str[256] = { 0 };
    if (format != NULL) {
        va_list ap;
        va_start(ap, format);
        vsprintf(m_str, format, ap);
        va_end(ap);
    }
    CLogFile::Instance().Write(eLogType, (const char*)m_str);
}

/****************************************************************************
功能:不重叠的内存拷贝
输入:s1:目的buf; s1_size:s1的尺寸; s2:源buf;s2_size:s2的尺寸; n:拷贝的字节数
返回:0 成功,-1 失败
****************************************************************************/
int memcpy_safe(void *s1, int s1_size, void *s2, int s2_size, int n)
{
    if (s1_size > 0 && s2_size > 0 && n <= s1_size && n <= s2_size) {
        memcpy(s1, s2, n);
        return 0;
    } else {
        Std_LogWrite(eErrType,"memcpy_safe :%s(%d)---fatal error:s1_size=%d,s2_size=%d,n=%d,exit\n", __FILE__, __LINE__, s1_size, s2_size, n);

        //exit(0);
       return -1;
        
    }
}

int memcpy_safe_revese(void *s1, int s1_size, void *s2, int s2_size, int n)
{

    if (s1_size > 0 && s2_size > 0 && n <= s1_size && n <= s2_size) {
        for (int i = 0; i < n; i++)
        {
            memcpy((char*)s1 + i, (char*)s2 + (n - i - 1), 1);
        }
        return 0;
    } else {
        Std_LogWrite(eErrType,"memcpy_safe_revese :%s(%d)---fatal error:s1_size=%d,s2_size=%d,n=%d,exit\n", __FILE__, __LINE__, s1_size, s2_size, n);

        //exit(0);
       return -1;
        
    }
}

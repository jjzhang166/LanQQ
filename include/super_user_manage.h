#ifndef _SUPER_USER_MANAGE_H_
#define _SUPER_USER_MANAGE_H_

#define NAME_LENGTH 20  //名字的长度
#define PASSWD_LENGTH 20 //密码的长度

#define USER_NUMBER 100 //用户的数目
#define GROUP_NUMBER 20 //群的数量

#define ERROR_MSG_LENGTH 200  //错误信息的字符串最大长度

/*函数功能：服务器的日志函数
	  参数：InLog要打印的日志*/
void serverlLOG(const char* InLog);

/*函数功能：服务器的日志函数
	  参数：InLog要打印的日志*/
void sys_log(const char* str);

/**函数功能：初始化tcp服务器
参数：
	OutSocketFd 返回的文件描述符
返回值：0 函数执行成功 
		1 函数执行失败 
*/
int initTcpServer(int* OutSocketFd);


#endif
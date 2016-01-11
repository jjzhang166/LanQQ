#ifndef _SUPER_USER_MANAGE_H_
#define _SUPER_USER_MANAGE_H_

#define NAME_LENGTH 20  //名字的长度
#define PASSWD_LENGTH 20 //密码的长度

#define USER_NUMBER 100 //用户的数目
#define GROUP_NUMBER 20 //群的数量

/*函数功能：服务器的日志函数
	  参数：InLog要打印的日志*/
void serverlLOG(char* InLog);

/*函数功能：服务器的日志函数
	  参数：InLog要打印的日志*/
void sys_log(char* str);


#endif
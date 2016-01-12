#include "super_user_manage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include "mysqlqq.h"

/*函数功能：服务器的日志函数
参数：InLog要打印的日志*/
void serverlLOG(const char* InLog)
{
	printf("%s\n", InLog);
	int fd = 0, save_fd = 0;

	/* 打开文件，在已经有的文件结尾添加信息 */
	fd = open("../log/server_log.txt", O_WRONLY | O_APPEND, 0);

	if (fd < 0)
	{
		return;
	}

	save_fd = dup(STDOUT_FILENO);
	dup2(fd, STDOUT_FILENO);
	close(fd);
	system("date +%F:%H:%M:%S");
	printf("%s\n\n", InLog);
	dup2(save_fd, STDOUT_FILENO);
	close(save_fd);
	return;
}

/*函数功能：服务器的日志函数
参数：InLog要打印的日志*/
void sys_log(const char* str)
{
	serverlLOG(str);
}

/**函数功能：初始化tcp服务器
参数：
	OutSocketFd 返回的文件描述符
返回值：0 函数执行成功 
		1 函数执行失败 
*/
int initTcpServer(int* OutSocketFd)
{
	int nSocketFd = 0;
	struct sockaddr_in stServAddr;
	int nRet = 0;
	int isReuse = 1;
	char errorMsg[ERROR_MSG_LENGTH] = {0};

	/** 产生一个套接口的描数字 */
	nSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == nSocketFd)
	{
		memset(errorMsg,0,ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"initTcpServer-->socket:get the socketfd failed");
		serverlLOG(errorMsg);
		
		return 1;
	}

	memset(&stServAddr, 0, sizeof(struct sockaddr_in));

	stServAddr.sin_family = AF_INET;
	stServAddr.sin_addr.s_addr = htonl(INADDR_ANY);//接收任何客户端的连接
	stServAddr.sin_port = htons(8000);//端口默认都是8000

	/** 处于TIME_WAIT状态的端口也可以使用 */
	setsockopt(nSocketFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&isReuse, sizeof(isReuse));

	/** 把这个套接字描述符和本地地址绑定起来 */
	nRet = bind(nSocketFd, (struct sockaddr*)&stServAddr, sizeof(stServAddr));
	if (-1 == nRet)
	{	
		memset(errorMsg,0,ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"initTcpServer-->bind:bind the socket failed");
		serverlLOG(errorMsg);
		close(nSocketFd);
		return 1;
	}

	/** 设置该套接口的监听状态， */
	listen(nSocketFd, 100);

	*OutSocketFd = nSocketFd;

	return 0;
}

/*函数功能：获取管理员登录的用户名
参数：str 客户端发过来的字符串
返回值：0 函数执行成功 1 函数执行失败 */
int get_name(const char* str, char* name)
{
	int i = 0;
	char name_length[2] = {0};
	int name_l = 0;
	

	for (i = 0; i<2; i++)
	{
		name_length[i] = *(str + i);
	}
	name_l = atoi(name_length);

	for (; i<name_l + 2; i++)
	{
		name[i - 2] = *(str + i);
	}
	return i;
	
	return i;
}

/*函数功能：超级用户登录
参数：
str 客户端发过来的字符串
返回值：0 函数执行成功 1 函数执行失败 */
int root_login(const char* str, int InsocketFd)
{
	char name[NAME_LENGTH] = { 0 };
	char passwd[PASSWD_LENGTH] = { 0 };
	int point = 2, n = 0, a = 0;
	char szBuff[100] = { 0 };
	char errorMsg[ERROR_MSG_LENGTH] = {0};

	//printf("str: %s\n", str);

	/* 把超级用户的登录名取出来 */
	point += get_name((str + point), name);
	//printf("name:%s\n", name);
	printf("user %s login !\n\n", name);

	/* 判断是否是超级用户 */
	IsSuperUser(name, &n);

	if (n == 1)
	{
		memset(errorMsg,0,ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"root_login:bu shi chao ji yong hu");
		serverlLOG(errorMsg);
		
		szBuff[0] = '3';
		szBuff[1] = '0';
		szBuff[2] = '4';
		szBuff[3] = '\0';
		write(InsocketFd, szBuff, 4);
		return 1;
	}

	point += get_name((str + point), passwd);
//	printf("passwd:%s\n", passwd);

	/* 判断登录的密码是否正确 */
	isSpuerUserPasswordTrue(name, passwd, &a);

	/* 密码正确 */
	if (a == 0)
	{
		szBuff[0] = '3';
		szBuff[1] = '0';
		szBuff[2] = '1';
		szBuff[3] = '\0';
		write(InsocketFd, szBuff, 4);

		/* 延时200ms */

		usleep(200000);
		return 0;
	}

	/* 密码错误 */
	if (a == 1)
	{
		memset(errorMsg,0,ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"root_login:mi ma cuo wu");
		serverlLOG(errorMsg);
		
		szBuff[0] = '3';
		szBuff[1] = '0';
		szBuff[2] = '2';
		szBuff[3] = '\0';
		write(InsocketFd, szBuff, 4);
		return 1;
	}
	return 0;
}



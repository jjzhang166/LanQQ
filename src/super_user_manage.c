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

	/** 产生一个套接口的描数字 */
	nSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == nSocketFd)
	{
		serverlLOG("initTcpServer-->socket:get the socketfd failed");
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
		serverlLOG("initTcpServer-->bind:bind the socket failed");
		close(nSocketFd);
		return 1;
	}

	/** 设置该套接口的监听状态， */
	listen(nSocketFd, 100);

	*OutSocketFd = nSocketFd;

	return 0;
}


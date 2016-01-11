#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "mysqlqq.h"
#include "super_user_manage.h"
#include "normal_server.h"

/* 可重入的线程函数,服务器和客户端所有消息的入口 */
void* PthreadReadClientMsg(void* arg)
{
	int nSocketFd = *((int*)arg);
	char errorMsg[ERROR_MSG_LENGTH] = {0};
	int nReadLen = 0;
	//int nRet = 0;
	char szBuff[BUFSIZ] = { 0 };
	char szMode[3] = { 0 };
	char szCmd[200] = {0};
	
	/*设置自己的线程是分离的,自动回收线程*/
	if (pthread_detach(pthread_self()))
	{
		memset(errorMsg,0,ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"set detach failed");
		serverlLOG(errorMsg);
	}
	
	while(1)
	{
		memset(szBuff, 0, BUFSIZ);
		
		/** 接收客户端发过来的信息 */
		nReadLen = read(nSocketFd, szBuff, BUFSIZ);
		
		/* 该客户端和服务器断开 */
		if (nReadLen == 0)
		{
			sprintf(szCmd,"%s%d%s%d%s","UPDATE tbl_register_users SET socketfd=",0,",is_online=0 WHERE socketfd='",nSocketFd,"'");
			execCmdToMysql(szCmd);
			return (void*)0;
		}
		else if (nReadLen > 0)
		{
			strncpy(szMode, szBuff, 2);
			
		}
	}
	
	return (void*)0;
}

int main()
{
	int nSocketFd = 0, nClientFd = 0;
	int nRet = 0;
	struct sockaddr_in stClientAddr;
	socklen_t socketAddrLen;
	pthread_t tid;
	char errorMsg[ERROR_MSG_LENGTH] = {0};
	
	/* 初始化TCp连接，获得文件描述符 */
	nRet = initTcpServer(&nSocketFd);
	if (0 != nRet)
	{
		return 1;
	}
	
	while (1)
	{
		/* 监听连接，如果有主机要连接过来，则建立套接口连接 */
		socketAddrLen = sizeof(struct sockaddr_in);
		nClientFd = accept(nSocketFd, (struct sockaddr*)&stClientAddr, &socketAddrLen);
		if (-1 == nClientFd)
		{
			memset(errorMsg,0,ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"accept the client error");
			serverlLOG(errorMsg);
			return 1;
		}
		else
		{
			printf("commect %s %d successful\n", inet_ntoa(stClientAddr.sin_addr), ntohs(stClientAddr.sin_port));//ntohs(stClientAddr.sin_port)
		}
		
		/* 创建线程，等待客户端发过来的信息 */
		nRet = pthread_create(&tid, NULL, PthreadReadClientMsg, (void*)&nClientFd);
		if (-1 == nRet)
		{
			memset(errorMsg,0,ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"function call pthread_create error");
			serverlLOG(errorMsg);
			return -1;
		}
	}
	return 0;
	
	return 0;
}
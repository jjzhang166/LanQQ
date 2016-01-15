#include "normal_server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "mysqlqq.h"

/*函数功能：服务器的日志函数
参数：InLog要打印的日志*/
void normal_serverlLOG(const char* InLog)
{
	printf("%s\n", InLog);
	int fd = 0, save_fd = 0;

	/* 打开文件，在已经有的文件结尾添加信息 */
	fd = open("../log/normal_server_log.txt", O_WRONLY | O_APPEND, 0);

	if (fd < 0)
	{
		return;
	}

	save_fd = dup(STDOUT_FILENO);
	dup2(fd, STDOUT_FILENO);
	close(fd);
	system("date +%F:%H:%M:%S: ");
	printf("%s\n", InLog);
	dup2(save_fd, STDOUT_FILENO);
	close(save_fd);
	return;
}

/*函数功能：系统日志函数
参数：Inlog：所要记录的日志信息

返回值：NULL */
void My_log(const char* Inlog)
{
	normal_serverlLOG(Inlog);
	return;
}

/*函数功能：解析了前2个字节判断是注册信息后调用该函数解析注册信息
返回给客户端的协议：111,注册成功 112,该用户已注册，失败 113,永远不能注册的用户
参数：
sockfd:注册用户连入服务器的的clifd，用来读写数据，并放于注册数据库表中
所用数据库函数
使用的数据库函数
int isUserAlreadyResiger(char* InUserName,int* OutIsResiger);
int execCmdToMysql(char* InExecCmd);
int isAlreadyBeDelete(char* InUserName,int* OutIsDelete); 判断用户是否已结被删除过，删除过就不能再注册了
返回值：0 函数执行成功 1 函数执行失败 */
int AnalyRegister(const char* buff, int sockfd)
{
	char *username = NULL;
	char *password = NULL;
	char *security = NULL;

	char szcmd[300] = { 0 };
	char pusrlen[3] = {0}, ppswlen[3] = {0}, seclen[3] = {0};//长度信息两个字节还要加个'\0'
	int usr_len = 0, psw_len = 0, sec_len = 0;
	int OutIsResiger = 0, err = 0, OutIsDelete = 0;
	char answer[4] = {0};
	char errorMsg[SERVER_ERROR_MSG_LENGTH] = {0};

	/* 得到注册的用户名 */
	strncpy(pusrlen, buff, 2);
	pusrlen[2] = '\0';
	usr_len = atoi(pusrlen);//这时得到了用户名长度
	username = (char*)malloc(usr_len + 1);
	if (username == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"malloc username 错误");
		My_log(errorMsg);
		return 1;
	}
	strncpy(username, buff + 2, usr_len);//这时要从buff+2处开始拷贝
	username[usr_len] = '\0';//得到了用户名

	/* 得到注册的密码 */
	strncpy(ppswlen, buff + 2 + usr_len, 2);
	ppswlen[2] = '\0';
	psw_len = atoi(ppswlen);
	password = (char*)malloc(psw_len + 1);
	if (password == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"malloc password error.");
		My_log(errorMsg);
		
		free(username);
		return 1;
	}
	strncpy(password, buff + 2 + usr_len + 2, psw_len);
	password[psw_len] = '\0';

	/* 得到密保 */
	strncpy(seclen, buff + 2 + usr_len + 2 + psw_len, 2);//要读密保长度
	seclen[2] = '\0';
	sec_len = atoi(seclen);
	security = (char*)malloc(sec_len + 1);
	if (security == NULL)
	{	
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"malloc security error:");
		My_log(errorMsg);
		
		free(username);
		free(password);
		return 1;
	}
	strncpy(security, buff + 2 + usr_len + 2 + psw_len + 2, sec_len);
	security[sec_len] = '\0';//得到了密保

	/*如果根据用户名是在永远无法注册数据库里，要回客户端113*/
	err = isAlreadyBeDelete(username, &OutIsDelete);
	if (err == -1)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyRegister 执行 isAlreadyBeDelete 失败.");
		My_log(errorMsg);
		
		free(username);
		free(password);
		free(security);
		return 1;
	}

	if (OutIsDelete == 1)//此账号被删除过
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyRegister 这账号被删除过.");
		My_log(errorMsg);
		
		sprintf(answer, "%s", "113");

		err = write(sockfd, answer, 4);
		if (err < 0)
		{	
			memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyRegister 回复给客户端113失败.");
			My_log(errorMsg);
			
			free(username);
			free(password);
			free(security);
			return 1;
		}
		return 0;//这时没事做，返回
	}

	/*能执行到这一步说明账号没有被注册过*/
	err = isUserAlreadyResiger(username, &OutIsResiger);

	/* 如果函数执行失败 */
	if (err == 1)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyRegister 执行 isUserAlreadyResiger 失败.");
		My_log(errorMsg);
		
		free(username);
		free(password);
		free(security);
		return 1;
	}

	if (OutIsResiger == 1)//注册过
	{
		sprintf(answer, "%s", "112");

		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s %s %s",__FILE__,__LINE__,"用户", username, " 已注册.");
		My_log(errorMsg);

		write(sockfd, answer, 4);
		free(username);
		free(password);
		free(security);

		return 0;//这时没事做了，返回
	}
	else if (OutIsResiger == 0)//没注册过
	{
		sprintf(szcmd, "%s%s%s%s%s%d%s%s%s", "INSERT INTO tbl_register_users(name,passwd,socketfd,is_online,is_used,securiety,isOfflineMsg,isofflineGroupMsg,isOfflineBroadMsg) VALUES ('" \
											                 ,username, "','", password, "',", sockfd, ",0,0,'", security, "',0,0,0)");//刚注册默认是在线，sockfd没有被用，没有离线消息

		err = execCmdToMysql(szcmd);//执行插入命令
		if (err == -1)
		{
			memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyRegister 执行 isUserAlreadyResiger 失败.");
			My_log(errorMsg);
			
			free(username);
			free(password);
			free(security);
			return 1;
		}
		
		usleep(100000);//等待0.1秒
		sprintf(answer, "%s", "111");//给客户端回注册成功

		write(sockfd, answer, 4);
		
		/* 创建单聊的离线文件 */
		char szFile[100] = {0};
		sprintf(szFile,"%s%s%s","touch ../offlineMsgFile/",username,".txt");
		system(szFile);
		
		/* 创建群聊的离线文件 */
		memset(szFile,0,100);
		sprintf(szFile,"%s%s%s","touch ../offlineMsgFile/",username,"Group.txt");
		system(szFile);
		
		/* 创建聊天文件 */
		memset(szFile,0,100);
		sprintf(szFile,"%s%s%s","touch ../offlineMsgFile/",username,"chat.txt");
		system(szFile);
		
		/* 创建自己的广播文件 */
		memset(szFile,0,100);
		sprintf(szFile,"%s%s%s","touch ../offlineMsgFile/",username,"broadcast.txt");
		system(szFile);
		
		free(username);
		free(password);
		free(security);
		
		return 0;//完事，直接返回
	}

	return 0;
}


/*函数功能：解析了前2个字节判断是登陆信息后调用该函数解析登陆信息，每次登陆sockfd都不一样，要据此实时改掉注册数据库中该用户的sockfd，登陆了要先返回给
客户端它所有的好友，所有的群，如果它有离线消息要把离线消息返回给它
登陆成功和失败的区别：
如果登陆成功了告诉客户端它所有的好友和群号，这样客户端就可以据此发送私聊和群聊消息了（要有好友名和群号名），返回
如果失败了就回给客户端失败了，直接返回，什么也不做
返回给客户端的协议：121,登陆成功 122,用户名或者密码错误 123,用户已经在线 124,用户被删除（在登陆这个可以不要的）
参数：
sockfd:登陆用户连入服务器的的clifd，用来读写数据
调用的数据库函数:
int execCmdToMysql(char* InExecCmd);
int isUserAlreadyOnline(char* InUserName,int* OutIsOnline);
int isUserPasswordTrue(char* InUserName,char* InPassword,int* OutIsTrue);
int execCmdToMysql(char* InExecCmd);//执行更新注册数据库命令
int getUserFriends(char* InUserName,char OutFriendsName[][20],int* OutFriendNums);
int getUserGroups(char* InUserName,char OutGroupsName[][20],int* OutGroupNums);
int isUserAlreadyOnline(char* InUserName,int* OutIsOnline);
int isUserSocketfdUsed(char* InUserName,int* OutIsSocketfdUsed);每次写的时候都要判断该sockfd是否被占用
返回值：0 函数执行成功 1 函数执行失败 */

int AnalyLogin(const char* buff,int sockfd)
{
	char *username = NULL;
	char *password = NULL;
	char pusrlen[3] = {0}, ppswlen[3] = {0};//长度信息两个字节还要加个'\0'
	
	char answer[4] = {0};
	char szcmd[300] = {0};
	int usr_len = 0, psw_len = 0;
	int err = 0, OutIsOnline = 0, OutIsTrue = 0;
	char errorMsg[SERVER_ERROR_MSG_LENGTH] = {0};
	
	/* 得到登录的用户名 */
	strncpy(pusrlen, buff, 2);
	pusrlen[2] = '\0';
	usr_len = atoi(pusrlen);//得到了用户名长度
	username = (char*)malloc(usr_len + 1);
	if (username == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"malloc Login username 失败.");
		My_log(errorMsg);
		return 1;
	}
	
	strncpy(username, buff + 2, usr_len);
	username[usr_len] = '\0';
	
	/* 得到登录的密码 */
	strncpy(ppswlen, buff + 2 + usr_len, 2);
	ppswlen[2] = '\0';
	psw_len = atoi(ppswlen);//得到了密码长度
	password = (char *)malloc(psw_len + 1);
	if (password == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"malloc Login password 失败.");
		My_log(errorMsg);
		free(username);
		return 1;
	}
	
	strncpy(password, buff + 2 + usr_len + 2, psw_len);
	password[psw_len] = '\0';//得到了密码
	
	int isUserExist = 0;
	/*判断用户名是否存在*/
	isUserAlreadyResiger(username,&isUserExist);
	
	/* 如果没有注册 */
	if(isUserExist == 0)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"this user name is not exist.");
		My_log(errorMsg);
		write(sockfd, "124", 4); //给客户端返回信息
		return 1;
	}
	
	/* 查询数据库判断密码是否正确 */
	err = isUserPasswordTrue(username, password, &OutIsTrue);//判断用户名密码是否正确
	if (err == 1)
	{
		My_log("In AnalyLogin 执行 isUserPasswordTrue 失败");
		return 1;
	}
	
	if (OutIsTrue == 1)//用户名密码错误
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin 用户名或者密码错误.");
		My_log(errorMsg);
		
		/* 给客户端返回用户名或密码错误的信息 */
		sprintf(answer, "%s", "122");	
		err = write(sockfd, answer, 4);
		if (err < 0)
		{
			memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin 回复用户名或密码错误 失败.");
			My_log(errorMsg);
			return 1;
		}
		
		free(username);
		free(password);
		return 0;//这里要返回，没事做了
	}
	else if (OutIsTrue == 0)
	{
		/* 判断该用户是否已经在线 */
		err = isUserAlreadyOnline(username, &OutIsOnline);
		if (err == 1)
		{
			memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin 执行 isUserAlreadyOnline 失败.");
			My_log(errorMsg);
			return 1;
		}
		
		/* 如果登录的用户已经在线 */
		if (OutIsOnline == 1)//在线
		{
			memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin 该用户已经在线.");
			My_log(errorMsg);
			
			/* 给客户端返回信息 */
			sprintf(answer, "%s", "123");
			err = write(sockfd, answer, 4);
			if (err < 0)
			{
				memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
				sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin 回复该用户已在线 信息 失败.");
				My_log(errorMsg);
				return 1;
			}
			
			free(username);
			free(password);
			return 0;//**
		}
		if (OutIsOnline == 0)//不在线，可以登陆
		{		
			sprintf(answer, "%s", "121");					
			write(sockfd, answer, 4);
			
			memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s %s %s",__FILE__,__LINE__, "用户 ", username, "登陆成功了.");
			My_log(errorMsg);
			
			/*登陆成功要把该用户在注册数据库中的sockfd和is_online更新*/
			/*UPDATE tbl_register_users SET socketfd=sockfd ,is_online=1 WHERE name='username' */
			sprintf(szcmd,"%s%d%s%s%s","UPDATE tbl_register_users SET socketfd=",sockfd,",is_online=1 WHERE name='",username,"'");
			execCmdToMysql(szcmd);
			
			/*要把该用户的所有好友和群提取出来发送给该用户*/
			char OutFriendsName[100][20] = {0};
			char OutGroupsName[50][20] = {0};//先默认定义这么大的二维数组用于存好友和群
			int OutGroupNums = 0, OutFriendNums = 0;
			
			/* 得到用户的所有的好友 */
			err = getUserFriends(username, OutFriendsName, &OutFriendNums);
			if (err == 1)
			{	
				memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
				sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin exec getUserFriends error.");
				My_log(errorMsg);

				return 1;
			}
			
			/* 得到用户所有的群 */
			err = getUserGroups(username, OutGroupsName, &OutGroupNums);
			if (err == 1)
			{
				memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
				sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin exec getUserGroups error.");
				My_log(errorMsg);

				return 1;
			}
			
			char send_buff[200] = {0};
			int i = 0;
			
			/* 把用户的所有好友的信息发送给用户 */
			for (i = 0; i < OutFriendNums; i++)
			{
				/* 判断该用户是否在线 */
				int isUserOnline = 0;
				err = isUserAlreadyOnline(OutFriendsName[i], &isUserOnline);
				if (err == 1)
				{
					memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
					sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin 执行 isUserAlreadyOnline 失败.");
					My_log(errorMsg);
					return 1;
				}
				
				bzero(send_buff, sizeof(send_buff));
				
				if (isUserOnline == 1)//在线
				{
					strncpy(send_buff,"181",3);				
				}
				else if (isUserOnline == 0)//不在线
				{
					strncpy(send_buff,"182",3);	
				}
				
				send_buff[3] = strlen(OutFriendsName[i]) / 10 + 0x30;
				send_buff[4] = strlen(OutFriendsName[i]) % 10 + 0x30;
				strcat(send_buff,OutFriendsName[i]);
				/* 给用户发送好友列表 */
				err = write(sockfd, send_buff, strlen(send_buff)+1);
				if (err == -1)
				{
					memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
					sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin write User's friend error.");
					My_log(errorMsg);

					return 1;
				}
				usleep(200000);
			}
			/* 把用户的群发给该用户 */
			for (i = 0; i < OutGroupNums; i++)
			{
				bzero(send_buff, sizeof(send_buff));
				
				strncpy(send_buff,"20",2);
				send_buff[2] = strlen(OutGroupsName[i]) / 10 + 0x30;
				send_buff[3] = strlen(OutGroupsName[i]) % 10 + 0x30;
				strcat(send_buff,OutGroupsName[i]);
				
				err = write(sockfd, send_buff, strlen(send_buff)+1);
				if (err == -1)
				{
					memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
					sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In AnalyLogin write User's group error.");
					My_log(errorMsg);

					return 1;
				}	
				usleep(200000);
			}
#if 0			
			/*发完了该登陆用户的好友和群消息，如果该用户有离线消息要给该用户发送离线消息*/
			/**************/
			/*发完了离线消息*/
			
			/* 判断自己是否有离线的单聊的消息 */
			int IsOfflineMssage1 = 0;
			int fd1 = 0;
			int readLen1 = 0;
			int p1 = 0;
			char szBuffer1[BUFSIZ] = {0};
			char filename1[100] = {0};
			char szStrLen1[3] = {0};
			int len1 = 0;
			char szSend1[100] = {0};
			char szUserName1[20] = {0};
			
			char szMsgLen[3] = {0};
			int msgLen = 0;
			char szMsginfo[100] = {0};
			
			
			isOfflineMsg(username,&IsOfflineMssage1);
			
			if(IsOfflineMssage1 == 1)
			{
				sprintf(filename1,"%s%s%s", "../offlineMsgFile/",username,".txt");
				
				/* 打开自己的文件 */
				fd1 = open(filename1,O_RDWR);
				if (fd1 < 0)
				{
					My_log("打开单聊的，打开广播文件失败");
				}
				/* 读取文件的内容 */
				readLen1 = read(fd1,szBuffer1,BUFSIZ);
				printf("readLen = %d   %s\n",readLen1,szBuffer1);
				if(readLen1>0)
				{
					while(1)
					{
						/* 把长度的两个字节取出来 */
						memset(szUserName1,0,20);
						memset(szStrLen1,0,3);
						memset(szSend1,0,100);
						memset(szMsginfo,0,100);
						
						strncpy(szStrLen1,szBuffer1,2);
						szStrLen1[2] = '\0';
						p1 += 2; 
						
						/* 获得发送方的姓名长度 */
						len1 = atoi(szStrLen1);
						
						/* 把用户名取出来 */
						strncpy(szUserName1,szBuffer1+p1,len1);
						
						p1 = p1 + len1;
						
						/* 把消息长度取出来 */
						
						
						
						
						strncpy(szMsgLen,szBuffer1+p1,2);
						msgLen = atoi(szMsgLen);
						
						p1 = p1+2;
						
						strncpy(szMsginfo,szBuffer1+p1,msgLen);
						p1 = p1+msgLen;
						
						/* 多输入一个换行符 */
						p1 = p1 + 1;
						
						/* 把信息发出去 */
						szSend1[0] = '1';
						szSend1[1] = '4';
						
						szSend1[2] = len1 / 10 + 0x30;
						szSend1[3] = len1 % 10 + 0x30;
						
						strcat(szSend1, szUserName1);
						
						szSend1[4 + len1] = usr_len / 10 + 0x30;
						szSend1[5 + len1] = usr_len % 10 + 0x30;
						
						strcat(szSend1, username);
						
						szSend1[6 + len1+usr_len] = msgLen / 10 + 0x30;
						szSend1[7 + len1+usr_len] = msgLen % 10 + 0x30;
						
						strcat(szSend1, szMsginfo);

						write(sockfd, szSend1, strlen(szSend1) + 1);
						
						
						
						if(p1 >= readLen1)
						{
							break;
						}
						usleep(200000);
						
					}
					/* 清空文件 */
					ftruncate(fd1,0);
					lseek(fd1,0,SEEK_SET);
					/* 更新数据库 */
					sprintf(szcmd, "%s%s%s%s", "UPDATE tbl_register_users SET isOfflineMsg=0"," WHERE name='", username, "'");
					execCmdToMysql(szcmd);
					close(fd1);
				}
			}
			
			/* 判断自己是否有离线的广播消息 */
			int IsOfflineMssage = 0;
			int fd = 0;
			int readLen = 0;
			int p = 0;
			char szBuffer[BUFSIZ] = {0};
			char filename[100] = {0};
			char szStrLen[3] = {0};
			int len = 0;
			char szMsg[100] = {0};
			char szSend[100] = {0};
			
			
			isOfflineBroadcastMsg(username,&IsOfflineMssage);//判断该用户是否有离线消息
			
			if(IsOfflineMssage == 1)
			{
				sprintf(filename,"%s%s%s", "../offlineMsgFile/",username,"broadcast.txt");
				
				/* 打开自己的文件 */
				fd = open(filename,O_RDWR);
				if (fd < 0)
				{
					My_log("打开广播文件失败");
				}
				else
				{
					readLen = read(fd,szBuffer,BUFSIZ);
					if(read > 0)
					{
					//	printf("readlen 3333: %d  mes: %s",readLen,szBuffer);
						while(1)
						{
							/* 把长度的两个字节取出来 */
							strncpy(szStrLen,szBuffer,2);
							szStrLen[2] = '\0';
							p += 2; 
							len = atoi(szStrLen);
							
							/* 把广播信息取出来 */
							memset(szMsg,0,100);
							strncpy(szMsg,szBuffer+p,len);
							szMsg[len] = '\0';
							p = p + len;
							
							memset(szSend,0,100);
							strncpy(szSend,"40",2);
							szSend[2] = len / 10 + 0x30;
							szSend[3] = len % 10 + 0x30;
							
							strcat(szSend,szMsg);
							
							write(sockfd,szSend,strlen(szSend)+1);
							
							/* 清空文件 */
							ftruncate(fd,0);
							lseek(fd,0,SEEK_SET);
							
							if(p >= readLen)
							{
								break;
							}
						}
						
						
						close(fd);
						/* 更新数据库 */
						sprintf(szcmd, "%s%s%s%s", "UPDATE tbl_register_users SET isOfflineBroadcastMsg=0"," WHERE name='", username, "'");
						execCmdToMysql(szcmd);
					}
				}
				close(fd);
				
			}
			
#endif			

			free(username);
			free(password);
			return 0;
		}
	}
	return 0;
}


/*函数功能：解析了前2个字节判断是要找回密码的信息的信息，根据客户端发送的信息解析出密保正确与否，正确就发送给他密保
每次return 都要free

接收客户端信息的协议：
类型（03）  发送方名字长度（2字节）  发送方名字    密保长度（2字节）  密保
发送给客户端的协议：
找回成功 131
找回失败 132

参数：
sockfd:
发送方的sockfd，用于与服务器之间收发数据

使用的数据库函数：
int isSecurietyTrue(char* InUserName,char* Insecuriety,int* OutIsTrue,char* OutPasswd);判断密保是否正确，如果正确直接把密码返回
int isUserSocketfdUsed(char* InUserName,int* OutIsSocketfdUsed);每次写的时候都要判断该sockfd是否被占用
int execCmdToMysql(char* InExecCmd);
返回值：0 函数执行成功 1 函数执行失败 */

int Retrieve_psw(const char* buff,const int sockfd)
{
	char *username = NULL, *securiety = NULL, *Newpasswd = NULL;
	char p_username_len[3] = {0}, p_securiety_len[3] = {0}, p_Newpasswd_len[3] = {0};
	int username_len = 0, securiety_len = 0, Newpasswd_len = 0;
	int IsTrue = 0;
	char errorMsg[SERVER_ERROR_MSG_LENGTH] = {0};
	char szcmd[200] = {0};

	/* 读取用户名 */
	strncpy(p_username_len, buff, 2);
	p_username_len[2] = '\0';
	username_len = atoi(p_username_len);//得到用户名长度
	username = (char *)malloc(username_len + 1);
	if (username == NULL)
	{
		My_log("In Retrieve_psw malloc username 失败");
		return 1;
	}
	strncpy(username, buff + 2, username_len);
	username[username_len] = '\0';//得到了用户名
	
	/* 读取密保 */
	strncpy(p_securiety_len, buff + 2 + username_len, 2);
	p_securiety_len[2] = '\0';
	securiety_len = atoi(p_securiety_len);//得到密保长度
	securiety = (char *)malloc(securiety_len + 1);
	if (securiety == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Retrieve_psw malloc securiety 失败.");
		My_log(errorMsg);
		
		free(username);
		return 1;
	}
	strncpy(securiety, buff + 2 + username_len + 2, securiety_len);
	securiety[securiety_len] = '\0';//得到了密保
	
	/* 得到新的密码 */
	strncpy(p_Newpasswd_len, buff + 2 + username_len + 2 + securiety_len, 2);
	p_Newpasswd_len[2] = '\0';
	Newpasswd_len = atoi(p_Newpasswd_len);//得到了新密码长度
	Newpasswd = (char *)malloc(Newpasswd_len + 1);
	if (Newpasswd == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Retrieve_psw malloc Newpasswd 失败.");
		My_log(errorMsg);
		
		free(username);
		free(securiety);
		return 1;
	}
	strncpy(Newpasswd, buff + 2 + username_len + 2 + securiety_len + 2, Newpasswd_len);
	Newpasswd[Newpasswd_len] = '\0';//得到了新密码
	
	int isUserExist = 0;
	isUserAlreadyResiger(username,&isUserExist);
	if(isUserExist == 0)
	{	
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s %s %s",__FILE__,__LINE__, "找回密码失败，用户 ", username," 不存在.");
		My_log(errorMsg);
		
		write(sockfd, "133", 4);
		return 0;
	}
	
	/*0  用户名不存在 1 密保正确 2  密保不正确*/
	isSecurietyTrue(username, securiety, &IsTrue);//passwd只是作为一个指针传入，并没有 什么实际意义
																
	if (IsTrue == 2)//密保错误
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s %s %s",__FILE__,__LINE__, "找回密码失败，用户 ", username," 的密保错误.");
		My_log(errorMsg);
		
		write(sockfd, "132", 4);
		
		free(username);
		free(securiety);
		return 0;//这里要直接返回了
	}
	
	
	/*这里就是密保正确了，要回找回成功，并且把新密码更新在数据库里*/
	
	/*UPDATE tbl_register_users SET passwd='Newpasswd' WHERE name='username'*/
	
	sprintf(szcmd,"%s%s%s%s%s","UPDATE tbl_register_users SET passwd='",Newpasswd,"' WHERE name='" ,username,"'");
	execCmdToMysql(szcmd);


	write(sockfd, "131", 4);
	
	free(username);
	free(securiety);
	return 0;//这里要直接返回了
}

/*函数功能：解析了前2个字节判断是发送方A要加接收方B好友的消息，要解析出A,B用户名，并且给B发：A要加你好友
每次return 都要free

接收客户端A信息的协议：
类型（07）  发送方名字长度（2字节）  发送方名字（A）  接收方名字长度（2字节）  接收方名字（B）
发送给客户端B的协议：
类型（41）  请求加好友方A名字长度（2字节） 请求加好友方名字（A）

参数：
sockfd:
发送方的sockfd，用于与服务器之间收发数据

使用的数据库函数：
int getUserSocketFd(char* InUserName,int* OutSocketFd);根据用户名把该用户服务器与客户端的的socketfd找出来
int isAlreadyBeDelete(char* InUserName,int* OutIsDelete);判断用户是否已结被删除过，
int isUserSocketfdUsed(char* InUserName,int* OutIsSocketfdUsed);每次写的时候都要判断该sockfd是否被占用
int execCmdToMysql(char* InExecCmd);
int isUserNameExist(char* InUserName,int* OutIsUsernameExist);//判断用户名是否存在 1,存在  0，不存在
返回值：0 函数执行成功 1 函数执行失败 */

int Add_friend_forward(const char* buff,int sockfd)
{
	char *send_username = NULL, *rec_username = NULL;
	char p_send_username_len[3] = {0}, p_rec_username_len[3] = {0};
	char send_buff[100] = {0};
	int send_username_len = 0, rec_username_len = 0;
	int err = 0, rec_sockfd = 0,  IsUsernameExist = 0;
	char errorMsg[SERVER_ERROR_MSG_LENGTH] = {0};
	int OutIsOnline = 0;

	/* 把A的名字取出来 */
	strncpy(p_send_username_len, buff, 2);
	p_send_username_len[2] = '\0';
	send_username_len = atoi(p_send_username_len);//得到了发送方名字长度
	send_username = (char *)malloc(send_username_len + 1);
	if (send_username == NULL)
	{	
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_forward malloc send_username 失败.");
		My_log(errorMsg);
		
		return 1;
	}
	strncpy(send_username, buff + 2, send_username_len);
	send_username[send_username_len] = '\0';//得到了发送方的名字
	
	/* 把B的名字取出来 */
	strncpy(p_rec_username_len, buff + 2 + send_username_len, 2);
	p_rec_username_len[2] = '\0';
	rec_username_len = atoi(p_rec_username_len);//得到了接收方名字长度
	rec_username = (char *)malloc(rec_username_len + 1);
	if (rec_username == NULL)
	{		
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_forward malloc rec_username 失败.");
		My_log(errorMsg);
		
		free(send_username);
		return 1;
	}
	strncpy(rec_username, buff + 2 + send_username_len + 2, rec_username_len);
	rec_username[rec_username_len] = '\0';//得到了接收方的名字
	
	/* 得到B是否已经注册 */
	err = isUserAlreadyResiger(rec_username, &IsUsernameExist);
	if (err == 1)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_forward 执行 isUserNameExist 失败.");
		My_log(errorMsg);
		
		free(send_username);
		free(rec_username);
		return 1;
	}
	
	/* 如果B用户名不存在 */
	if (IsUsernameExist == 0)//不存在
	{		
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s %s %s",__FILE__,__LINE__,"添加好友失败，要添加的好友 ", rec_username ," 不存在.");
		My_log(errorMsg);
		
		/* 给A发送要加的B用户名不存在 */
		write(sockfd,"99",3);
		
		free(send_username);
		free(rec_username);
		return 0;//直接返回
	}
	
	/* 要加的好友B是存在的，先判断B是否在线 */
	err = isUserAlreadyOnline(rec_username, &OutIsOnline);
	if (err == 1)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_forward 执行 isUserAlreadyOnline 失败.");
		My_log(errorMsg);
		
		return 1;
	}
		
	/* 如果B在线 */
	if (OutIsOnline == 1)//在线
	{
		/* 服务器给B发送有人加自己好友，首先获得B和服务器连接的socketfd */
		err = getUserSocketFd(rec_username, &rec_sockfd);
		if (err == 1)
		{
			memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_forward 执行 getUserSocketFd 失败.");
			My_log(errorMsg);
			
			free(send_username);
			free(rec_username);
			return 1;
		}
		
		/* 组建数据包 */
		memset(send_buff,0,100);
		send_buff[0] = '4';
		send_buff[1] = '1';
		send_buff[2] = strlen(send_username) / 10 + 0x30;
		send_buff[3] = strlen(send_username) % 10 + 0x30;
		strcat(send_buff,send_username);
		
		write(rec_sockfd, send_buff, strlen(send_buff) + 1);
	}
	else if (OutIsOnline == 0)//不在线
	{
		/* 组建数据包 */
		memset(send_buff,0,100);
		send_buff[0] = '8';
		send_buff[1] = '0';
		send_buff[2] = '\0';
		write(sockfd, send_buff, strlen(send_buff) + 1);
	}		
	
	free(send_username);
	free(rec_username);
	return 0;
}


/*函数功能：解析了前2个字节判断是发送方B同意加A好友的信息，要解析出A,B用户名，并且给A发：B同意加你好友
每次return 都要free

接收客户端A信息的协议：
类型（39）  发送方名字长度（2字节）  发送方名字（B）  接收方名字长度（2字节）  接收方名字（A）
发送给客户端B的协议：
类型（43）  同意加好友方（B）名字长度（2字节） 同意加好友方（B）名字

参数：
char *buff:
读出来的buff信息，包括了前2个类型字节

使用的数据库函数：
int getUserSocketFd(char* InUserName,int* OutSocketFd);根据用户名把该用户服务器与客户端的的socketfd找出来
int isAlreadyBeDelete(char* InUserName,int* OutIsDelete);判断用户是否已结被删除过，
int isUserSocketfdUsed(char* InUserName,int* OutIsSocketfdUsed);每次写的时候都要判断该sockfd是否被占用
int execCmdToMysql(char* InExecCmd);
int isUserNameExist(char* InUserName,int* OutIsUsernameExist);//判断用户名是否存在 1,存在  0，不存在//这里要不要判断的？
返回值：0 函数执行成功 1 函数执行失败 
*/
int Add_friend_agree(const char* buff)
{
	char *send_username = NULL, *rec_username = NULL;
	char p_send_username_len[3] = {0}, p_rec_username_len[3] = {0};
	
	char szcmd[200] = {0};
	int send_username_len = 0, rec_username_len = 0;
	int err = 0, rec_sockfd = 0;
	char errorMsg[SERVER_ERROR_MSG_LENGTH] = {0};
	
	/* 解析B的名字 */
	strncpy(p_send_username_len, buff + 2, 2);
	p_send_username_len[2] = '\0';
	send_username_len = atoi(p_send_username_len);//得到了发送方名字长度
	send_username = (char *)malloc(send_username_len + 1);
	if (send_username == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_agree malloc send_username 失败.");
		My_log(errorMsg);
		
		free(send_username);
		free(rec_username);
		return 1;
	}
	strncpy(send_username, buff + 2 + 2, send_username_len);
	send_username[send_username_len] = '\0';//得到了发送方的名字
	
	/* 得到A的名字 */
	strncpy(p_rec_username_len, buff + 2 + 2 + send_username_len, 2);
	p_rec_username_len[2] = '\0';
	rec_username_len = atoi(p_rec_username_len);//得到了接收方名字长度
	rec_username = (char *)malloc(rec_username_len + 1);
	if (rec_username == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_forward malloc rec_username 失败.");
		My_log(errorMsg);
		
		free(send_username);
		return 1;
	}
	strncpy(rec_username, buff + 2 + 2 + send_username_len + 2, rec_username_len);
	rec_username[rec_username_len] = '\0';//得到了接收方的名字
	
	
	/*这里要同时更新tbl_users_friends 表中用户与他好友关系，添加两条
	"INSERT INTO tbl_users_friends(name,friend_name) VALUES('A','B')";
	"INSERT INTO tbl_users_friends(name,friend_name) VALUES('B','A')";*/
	
	memset(szcmd,0,200);
	sprintf(szcmd,"%s%s%s%s%s","INSERT INTO tbl_users_friends(name,friend_name) VALUES ('",send_username,"','",rec_username,"')");
	printf("agree:send_username = %s rec_username= %s %s\n",send_username,rec_username,szcmd);
	err = execCmdToMysql(szcmd);
	if (err == 1)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_agree 执行 execCmdToMysql 失败.");
		My_log(errorMsg);
		
		free(send_username);
		free(rec_username);
		return 1;
	}
	
	
	bzero(szcmd, sizeof(szcmd));//要清空

	sprintf(szcmd,"%s%s%s%s%s","INSERT INTO tbl_users_friends(name,friend_name) VALUES ('",rec_username,"','",send_username,"')");
	printf("agree:send_username = %s rec_username= %s %s\n",send_username,rec_username,szcmd);
	err = execCmdToMysql(szcmd);
	if (err == 1)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_agree 执行 execCmdToMysql 失败.");
		My_log(errorMsg);
		
		free(send_username);
		free(rec_username);
		return 1;
	}
	
	/* 得到和A的socket连接 */
	err = getUserSocketFd(rec_username, &rec_sockfd);
	if (err == 1)
	{		
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_agree 执行 getUserSocketFd 失败.");
		My_log(errorMsg);
		
		free(send_username);
		free(rec_username);
		return 1;
	}
	
	/* 给A回复B同意加自己好友 */
	write(rec_sockfd, "421", 4);
	
	free(send_username);
	free(rec_username);
	return 0;
}


/*函数功能：解析了前2个字节判断是发送方B拒绝加A好友的信息，要解析出A,B用户名，并且给A发：B拒绝加你好友
每次return 都要free

接收客户端A信息的协议：
类型（40）  发送方（B）名字长度（2字节）  发送方名字（B）  接收方（A）名字长度（2字节）  接收方名字（A）
发送给客户端B的协议：
类型（42）  拒绝加好友方（B）名字长度（2字节） 拒绝加好友方（B）名字

参数：
char *buff:
读出来的buff信息，包括了前2个类型字节

使用的数据库函数：
int getUserSocketFd(char* InUserName,int* OutSocketFd);根据用户名把该用户服务器与客户端的的socketfd找出来
int isAlreadyBeDelete(char* InUserName,int* OutIsDelete);判断用户是否已结被删除过，
int isUserSocketfdUsed(char* InUserName,int* OutIsSocketfdUsed);每次写的时候都要判断该sockfd是否被占用
int execCmdToMysql(char* InExecCmd);
int isUserNameExist(char* InUserName,int* OutIsUsernameExist);//判断用户名是否存在 1,存在  0，不存在//这里要不要判断的？
返回值：0 函数执行成功 1 函数执行失败 */

int Add_friend_refuse(const char* buff)
{
	char *send_username = NULL, *rec_username = NULL;
	char p_send_username_len[3] = {0}, p_rec_username_len[3] = {0};
	
	int send_username_len = 0, rec_username_len = 0;
	int err = 0, rec_sockfd = 0;
	char errorMsg[SERVER_ERROR_MSG_LENGTH] = {0};
	
	/* 给B的名字取出来 */
	strncpy(p_send_username_len, buff + 2, 2);
	p_send_username_len[2] = '\0';
	send_username_len = atoi(p_send_username_len);//得到了发送方名字长度
	send_username = (char *)malloc(send_username_len + 1);
	if (send_username == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_refuse malloc send_username 失败.");
		My_log(errorMsg);
		return 1;
	}
	strncpy(send_username, buff + 2 + 2, send_username_len);
	send_username[send_username_len] = '\0';//得到了发送方的名字
	
	/* 给A的名字取出来 */
	strncpy(p_rec_username_len, buff + 2 + 2 + send_username_len, 2);
	p_rec_username_len[2] = '\0';
	rec_username_len = atoi(p_rec_username_len);//得到了接收方名字长度
	rec_username = (char *)malloc(rec_username_len + 1);
	if (rec_username == NULL)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_refuse malloc rec_username 失败.");
		My_log(errorMsg);
		
		free(send_username);
		return 1;
	}
	strncpy(rec_username, buff + 2 + 2 + send_username_len + 2, rec_username_len);
	rec_username[rec_username_len] = '\0';//得到了接收方的名字
	
	/*得到A的socketfd*/
	err = getUserSocketFd(rec_username, &rec_sockfd);
	if (err == 1)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"In Add_friend_refuse 执行 getUserSocketFd 失败.");
		My_log(errorMsg);
		
		free(send_username);
		free(rec_username);
		return 1;
	}
	
	write(rec_sockfd, "420", 4);
	
	free(send_username);
	free(rec_username);
	return 0;
}



/*函数功能：解析了前2个字节判断是要私聊的信息，发送给客户端消息时要根据对方是否在线，1，在线（在登陆时已经更新了在线信息）就直接根据
要接收数据方姓名从数据库中取出它的sockfd，再根据此sockfd将信息发送给他，2，不在线，实时更新，将数据库中该用户的离线消息字段置1，
将消息保存在本地文件中（文件名：要接收方用户名.txt）  每次return 都要free

接收客户端信息的协议：
类型（04）  发送方名字长度（2字节）  发送方名字    接收方名字长度（2字节）   接收方名字  信息长度（2字节）   信息
发送给客户端的协议：
类型（14）  发送方名字长度（2字节）  发送方名字  信息长度（2字节）   信息
参数：
sockfd:
发送方的sockfd，用于与服务器之间收发数据

使用的数据库函数：
int isSensitiveWords(char* InWords,int* OutIsSensWord);判断输入的字符串是否含有敏感词，若有敏感词要改造该条信息
int isUserByGag(char* InUserName,int* OutIsGag,int* OutgagMinutes);判断用户是否被禁言
int isUserAlreadyOnline(char* InUserName,int* OutIsOnline);根据用户名判断用户是否在线
int getUserSocketFd(char* InUserName,int* OutSocketFd);根据用户名把该用户服务器与客户端的的socketfd找出来
int isUserSocketfdUsed(char* InUserName,int* OutIsSocketfdUsed);每次写的时候都要判断该sockfd是否被占用
int execCmdToMysql(char* InExecCmd);

如果要接收消息方的用户是被永远删除的，那么服务器会把该消息发送给他的每一个朋友，协议是**（2字节） 所删好友名字长度（2字节） 所删好友名字  那么每一个客户端就会把该人从
好友列表中去除，所以好友列表的好友都是存在于数据库中的
返回值：0 函数执行成功 1 函数执行失败 */

int Private_chat(const char* buff, int sockfd)
{
	char *send_username = NULL, *rec_username = NULL, *imfor = NULL;
	char p_send_user_len[3] = {0}, p_rec_user_len[3] = {0}, p_imfor_len[3] = {0};//长度信息两个字节还要加个'\0'

	char sens_log[30] = { 0 };
	char szcmd[100] = { 0 };
	char send_buff[200] = { 0 };
	int fd = 0;//存离线消息
	char filename[30] = { 0 };//存放离线消息的文件的名字
	int rec_sockfd = 0;
	int send_usr_len = 0, rec_usr_len = 0, imfor_len = 0;
	int   OutIsSensWord = 0, rec_IsOnline = 0;
	char errorMsg[SERVER_ERROR_MSG_LENGTH] = {0};

	/* 获得A的用户名 */
	strncpy(p_send_user_len, buff, 2);
	p_send_user_len[2] = '\0';
	send_usr_len = atoi(p_send_user_len);//发送方名字长度
	send_username = (char*)malloc(send_usr_len + 1);
	strncpy(send_username, buff + 2, send_usr_len);
	send_username[send_usr_len] = '\0';//得到发送方名字

	/* 获得B的用户名 */
	strncpy(p_rec_user_len, buff + 2 + send_usr_len, 2);
	p_rec_user_len[2] = '\0';
	rec_usr_len = atoi(p_rec_user_len);//得到接收方名字长度
	rec_username = (char*)malloc(rec_usr_len + 1);
	strncpy(rec_username, buff + 2 + send_usr_len + 2, rec_usr_len);
	rec_username[rec_usr_len] = '\0';//得到接收方名字

	/* 得到发送的信息 */
	strncpy(p_imfor_len, buff + 2 + send_usr_len + 2 + rec_usr_len, 2);
	imfor_len = atoi(p_imfor_len);//得到信息长度
	imfor = (char *)malloc(imfor_len + 1);
	strncpy(imfor, buff + 2 + send_usr_len + 2 + rec_usr_len + 2, imfor_len);//这里要不要imfor_len+1？等待调试
	imfor[imfor_len] = '\0';
	
	/* 获取系统当前的时间 */
	time_t timep;
	time (&timep);
	
	/* 首先写到A的聊天文件中去 */
	char filename1[100] = {0};
	int fd1 = 0,fd2 = 0;
	sprintf(filename1,"%s%s%s", "../offlineMsgFile/",rec_username,"chat.txt");
	
	fd1 = open(filename1,O_RDWR);
	if (fd1 < 0)
	{
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"打开消息文本失败.");
		My_log(errorMsg);
		
		free(send_username);
		free(rec_username);
		free(imfor);
		return 1;
	}
	
	/** 设置写文件的位置 */
	lseek(fd1,0,SEEK_END);
	char lineMsg1[100] = {0};
	
	sprintf(lineMsg1,"%s%s%s%s%s",asctime(gmtime(&timep)),"receive from ",send_username,": ",imfor);					
	write(fd1, lineMsg1, strlen(lineMsg1));//把聊天消息写到文件中
	write(fd1, "\n",1);//把聊天消息写到文件中
	close(fd1);
	
	/* 写到B的聊天文件中去 */
	memset(lineMsg1,0,100);
	sprintf(filename1,"%s%s%s", "../offlineMsgFile/",send_username,"chat.txt");
	fd2 = open(filename1,O_RDWR);
	if (fd1 < 0)
	{		
		memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"打开消息文本失败.");
		My_log(errorMsg);
		
		free(send_username);
		free(rec_username);
		free(imfor);
		return 1;
	}
	/** 设置写文件的位置 */
	lseek(fd2,0,SEEK_END);
	sprintf(lineMsg1,"%s%s%s%s%s",asctime(gmtime(&timep)),"send to  ",rec_username,": ",imfor);	
	write(fd2, lineMsg1, strlen(lineMsg1));//把离线消息写到文件中
	write(fd2, "\n",1);//把离线消息写到文件中
	close(fd2);
	
	//单聊的时候，不会检查是否被禁言
	{
		/* 是否有敏感词 */
		isSensitiveWords(imfor, &OutIsSensWord);

		if (OutIsSensWord == 0)//有敏感词
		{
			sprintf(sens_log, "%s%s", send_username, "发送的信息有敏感词");//记录谁发的信息有敏感词
			memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
			sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,sens_log);
			My_log(errorMsg);
			
			/* 把敏感词去掉之后，再过滤 */
			changeSensitiveWords(imfor);
		}
	
		/* 扎到接收方的socketfd */
		getUserSocketFd(rec_username, &rec_sockfd);
		
		isUserAlreadyOnline(rec_username, &rec_IsOnline);//要先检查下接收方是否在线
	
		if (rec_IsOnline == 0)//接收方没有在线
		{	
			/*UPDATE tbl_register_users SET isOfflineMsg=1 WHERE name='rec_username' */
			sprintf(szcmd, "%s%s%s%s", "UPDATE tbl_register_users SET isOfflineMsg=1"," WHERE name='", rec_username, "'");
			printf("%s\n",szcmd);
			execCmdToMysql(szcmd);
				
			sprintf(filename,"%s%s%s", "../offlineMsgFile/",rec_username,".txt");
			
			/* 以读写的方式打开这个文件 */
			fd = open(filename,O_RDWR);
			if (fd < 0)
			{				
				memset(errorMsg,0,SERVER_ERROR_MSG_LENGTH);
				sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"打开离线文本失败");
				My_log(errorMsg);
				
				free(send_username);
				free(rec_username);
				free(imfor);
				return 1;
			}
			
			/** 设置写文件的位置 */
			lseek(fd,0,SEEK_END);
			
			char lineMsg[100] = {0};
			
			lineMsg[0] = send_usr_len/10+ 0x30;
			lineMsg[1] = send_usr_len%10+ 0x30;
			strcat(lineMsg,send_username);
			lineMsg[2+send_usr_len] = strlen(imfor)/10+ 0x30;
			lineMsg[3+send_usr_len] = strlen(imfor)%10+ 0x30;
			
			strcat(lineMsg,imfor);
			
			write(fd, lineMsg, strlen(lineMsg));//把离线消息写到文件中
			write(fd, "\n",1);//把离线消息写到文件中
			close(fd);
			free(send_username);
			free(rec_username);
			free(imfor);
			return 0;//直接返回，没有在线就不要发消息了
		}
		else if (rec_IsOnline == 1)//接收方在线
		{
			send_buff[0] = '1';
			send_buff[1] = '4';
			
			send_buff[2] = strlen(send_username) / 10 + 0x30;
			send_buff[3] = strlen(send_username) % 10 + 0x30;
			
			strcat(send_buff, send_username);
			
			send_buff[4 + send_usr_len] = rec_usr_len / 10 + 0x30;
			send_buff[5 + send_usr_len] = rec_usr_len % 10 + 0x30;
			
			strcat(send_buff, rec_username);
			
			
			send_buff[6 + send_usr_len+rec_usr_len] = imfor_len / 10 + 0x30;
			send_buff[7 + send_usr_len+rec_usr_len] = imfor_len % 10 + 0x30;
			
			strcat(send_buff, imfor);

			write(rec_sockfd, send_buff, strlen(send_buff) + 1);
			
			//printf("baowen:%s\n",send_buff);

			/*此时发送成功了 *send_username,*rec_username,*imfor;*/
			free(send_username);
			free(rec_username);
			free(imfor);
			return 0;
		}
	}
	return 0;
}



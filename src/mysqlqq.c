#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "mysql.h"
#include "mysqlqq.h"

/*
tbl_register_users ：存放所有已经注册用户的信息
tbl_gag_users ： 存放被禁言的用户，当禁言时间到了之后，就要把用户从禁言列表里面删除
tbl_del_users ： 存放被管理员删除的用户，已经被加入该列表的用户，不能再注册
tbl_users_friends ： 存放某个用户和他的好友的信息
tbl_users_groups ： 存放某个用户和他的QQ群的信息
tbl_sensitive_words ： 存放敏感词的信息
tbl_super_users:超级用户的数据库表
*/

/*函数功能：MySQL的日志函数
参数：InLog要打印的日志
返回值：0 函数执行成功 1 函数执行失败*/
void mysqlLOG(const char* InLog)
{
	printf("%s\n", InLog);
	int fd = 0, save_fd = 0;

	/* 打开文件，在已经有的文件结尾添加信息 */
	fd = open("../log/mysql_log.txt", O_WRONLY | O_APPEND, 0);

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

/*函数功能：执行数据库命令，可以插入、删除、更新
InExecCmd 要执行的命令
返回值：0 函数执行成功 1 函数执行失败
示例："INSERT INTO tbl_gag_users(gag_name,gag_minutes) VALUES('user1de9',5)"
"DELETE FROM tbl_gag_users WHERE gag_name='user1de5' "*/
int execCmdToMysql(const char* InExecCmd)
{
	int nRet = 0;
	MYSQL my_connection;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"connect mysql failed");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 执行命令 */
	nRet = mysql_query(&my_connection, InExecCmd);
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s%s%s",__FILE__,__LINE__,"exec cmd failed(",InExecCmd,").");
		mysqlLOG(errorMsg);
		
		mysql_close(&my_connection);
		return 1;
	}

	/* 关闭数据库连接 */
	mysql_close(&my_connection);
	return 0;
}


/*函数功能：判断该用户是否是超级用户
参数：
InName 输入的用户名
OutIsSuperUser 返回是否是超级用户 0 是 1 不是
返回值：0 函数执行成功 1 函数执行失败 */
int IsSuperUser(const char* InName, int* OutIsSuperUser)
{
	int nRet = 0;
	MYSQL my_connection;
	MYSQL_RES*  result;
	int nRows = 0;
	char szCmd[100] = { 0 };
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"connect mysql failed");
		mysqlLOG(errorMsg);
		return 1;
	}
	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT * FROM tbl_super_users WHERE super_name='", InName, "'");
	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"exec cmd failed");
		mysqlLOG(errorMsg);
		
		mysql_close(&my_connection);
		return 1;
	}

	/* 获得查找结果有几行记录 */
	result = mysql_store_result(&my_connection);
	nRows = mysql_num_rows(result);

	if (nRows == 0)
	{
		/* 不是超级用户 */
		*OutIsSuperUser = 1;
	}
	else
	{
		/* 是超级用户 */
		*OutIsSuperUser = 0;
	}

	/* 释放空间，关闭与数据库的连接 */
	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}

/*函数功能：判断超级管理员用户的密码是否正确
参数：InSpuerUserName 输入的用户名
InPassword 输入的密码
OutIsTrue 输出是否正确，0 正确 1 错误
返回值：0 函数执行成功 1 函数执行失败 */
int isSpuerUserPasswordTrue(const char* InSuperUserName,const char* InPassword, int* OutIsTrue)
{
	int nRet = 0;
	MYSQL my_connection;
	char szCmd[100] = { 0 };
	MYSQL_RES*  result;
	MYSQL_ROW row;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"connect mysql failed");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT passwd FROM tbl_super_users WHERE super_name='", InSuperUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"exec cmd failed");
		mysqlLOG(errorMsg);
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);

	if ((row = mysql_fetch_row(result)))
	{
		if (strncmp(InPassword, row[0], 20) == 0)
		{
			/* 密码正确 */
			*OutIsTrue = 0;
		}
		else
		{
			/* 密码错误 */
			*OutIsTrue = 1;
		}
	}
	else
	{
		/* 密码错误 */
		*OutIsTrue = 1;
	}

	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}

/*函数功能：判断用户是否已结被删除过，删除过就不能再注册了
参数：InUserName 输入的用户名
OutIsDelete 输出是否被禁言，0 没有被删除过，1 被删除过
返回值：0 函数执行成功 1 函数执行失败 */
int isAlreadyBeDelete(const char* InUserName, int* OutIsDelete)
{
	int nRet = 0;
	MYSQL my_connection;
	char szCmd[100] = { 0 };
	MYSQL_RES*  result;
	int count = 0;
	MYSQL_ROW row;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isUserByGag-->mysql_real_connect: connect mysql failed");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT * FROM tbl_del_users WHERE del_name='", InUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isUserByGag-->mysql_query: exec cmd failed");
		mysqlLOG(errorMsg);
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);

	while ((row = mysql_fetch_row(result)))
	{
		count++;
	}

	if (count == 0)
	{
		/* 没有被删除过 */
		*OutIsDelete = 0;
	}
	else
	{
		/* 被删除过 */
		*OutIsDelete = 1;
	}
	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}



/*函数功能：根据用户名判断用户是否已经注册过
参数：InUserName 输入的用户名
OutIsResiger 输出是否已经注册，0 没有注册，1 已经注册
返回值：0 函数执行成功 1 函数执行失败 */
int isUserAlreadyResiger(const char* InUserName, int* OutIsResiger)
{
	int nRet = 0;
	MYSQL_RES*  result;
	MYSQL my_connection;
	int nRows = 0;
	char szCmd[100] = { 0 };
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{		
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isUserAlreadyResiger-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);
		return 1;
	}

	sprintf(szCmd, "%s%s%s", "SELECT * FROM tbl_register_users WHERE name='", InUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isUserAlreadyResiger-->mysql_query: exec cmd failed.");
		mysqlLOG(errorMsg);
		
		mysql_close(&my_connection);
		return 1;
	}

	/* 获得查找结果有几行记录 */
	result = mysql_store_result(&my_connection);
	nRows = mysql_num_rows(result);

	if (nRows == 0)
	{
		/* 没有注册 */
		*OutIsResiger = 0;
	}
	else
	{
		/* 已经注册 */
		*OutIsResiger = 1;
	}
	/* 关闭数据库连接 */
	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}
/*函数功能：判断用户的密码是否正确
参数：InUserName 输入的用户名
InPassword 输入的密码
OutIsTrue 输出是否正确，0 正确 1 错误
返回值：0 函数执行成功 1 函数执行失败 */
int isUserPasswordTrue(const char* InUserName, const char* InPassword, int* OutIsTrue)
{
	int nRet = 0;
	MYSQL my_connection;
	char szCmd[100] = { 0 };
	MYSQL_RES*  result;
	MYSQL_ROW row;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isUserPasswordTrue-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);
		
		return 1;
	}

	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT passwd FROM tbl_register_users WHERE name='", InUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isUserPasswordTrue-->mysql_query: exec cmd failed.");
		mysqlLOG(errorMsg);
		mysql_close(&my_connection);
		
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);
	row = mysql_fetch_row(result);

	if (strncmp(InPassword, row[0], 20) == 0)
	{
		/* 密码正确 */
		*OutIsTrue = 0;
	}
	else
	{
		/* 密码错误 */
		*OutIsTrue = 1;
	}

	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}


/*函数功能：根据用户名判断用户是否在线
参数：InUserName 输入的用户名
OutIsOnline 输出是否在线，0 没有在线，1 在线
返回值：0 函数执行成功 1 函数执行失败 */
int isUserAlreadyOnline(const char* InUserName, int* OutIsOnline)
{
	int nRet = 0;
	MYSQL my_connection;
	char szCmd[100] = { 0 };
	MYSQL_RES*  result;
	MYSQL_ROW row;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isUserAlreadyOnline-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT is_online FROM tbl_register_users WHERE name='", InUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{
		mysqlLOG("isUserAlreadyOnline-->mysql_query: exec cmd failed");
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);
	row = mysql_fetch_row(result);

	if (row[0][0] == '0')
	{
		/* 没有在线 */
		*OutIsOnline = 0;
	}
	else
	{
		/* 在线 */
		*OutIsOnline = 1;
	}
	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}
/*函数功能：判断密保是否正确，
参数：InUserName 用户名
Insecuriety输入的密保
OutIsTrue 密保是否正确 0  用户名不存在 1 密保正确 2  密保不正确
返回值：0 函数执行成功 1 函数执行失败 */
int isSecurietyTrue(const char* InUserName, const char* Insecuriety, int* OutIsTrue)
{
	int nRet = 0;
	MYSQL my_connection;
	char szCmd[200] = { 0 };
	MYSQL_RES*  result;
	MYSQL_ROW row;
	int count = 0;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{	
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isSecurietyTrue-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT securiety, passwd FROM tbl_register_users WHERE name='", InUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{		
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isSecurietyTrue-->mysql_query: exec cmd failed.");
		mysqlLOG(errorMsg);
		
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);
	if ((row = mysql_fetch_row(result)))
	{
		count++;
	}
	
	if (count == 0)
	{
		/* 用户名不存在 */
		*OutIsTrue = 0;
	}
	else if (strncmp(Insecuriety, row[0], 20) == 0)
	{
		/* 密保正确 */
		*OutIsTrue = 1;

	}
	else
	{
		/* 密保错误 */
		*OutIsTrue = 2;
	}

	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}
/*函数功能：根据用户名把该用户服务器与客户端的的socketfd找出来
参数：InUserName 输入的用户名
OutSocketFd 返回通信的文件描述符
返回值：0 函数执行成功 1 函数执行失败 */
int getUserSocketFd(const char* InUserName, int* OutSocketFd)
{
	int nRet = 0;
	MYSQL my_connection;
	char szCmd[200] = { 0 };
	MYSQL_RES*  result;
	MYSQL_ROW row;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getUserSocketFd-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT socketfd FROM tbl_register_users WHERE name='", InUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getUserSocketFd-->mysql_query: exec cmd faile.");
		mysqlLOG(errorMsg);
		
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);
	row = mysql_fetch_row(result);

	/* 把字符串转换为int数数值 */
	*OutSocketFd = atoi(row[0]);

	/* 释放空间，关闭与数据库的连接 */
	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}


/*函数功能：根据用户名把该用户所有的好友提取出来
参数：InUserName 输入的用户名
OutFriendsName 返回的用户名，放入一个数组中
OutFriendNums 返回的好友的个数
返回值：0 函数执行成功 1 函数执行失败 */
int getUserFriends(const char* InUserName, char OutFriendsName[][20], int* OutFriendNums)
{
	int nRet = 0;
	MYSQL my_connection;
	char szCmd[100] = { 0 };
	MYSQL_RES*  result;
	MYSQL_ROW row;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getUserFriends-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT friend_name FROM tbl_users_friends WHERE name='", InUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{	
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getUserFriends-->mysql_query: exec cmd failed.");
		mysqlLOG(errorMsg);
		
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);

	/* 把用户的所有的好友查出来 */
	*OutFriendNums = 0;
	while ((row = mysql_fetch_row(result)))
	{
		strncpy(OutFriendsName[*OutFriendNums], row[0], 20);
		(*OutFriendNums) = (*OutFriendNums) + 1;
	}

	/* 释放空间，关闭与数据库的连接 */
	mysql_free_result(result);
	mysql_close(&my_connection);
	
	return 0;
}


/*函数功能：根据用户名把该用户所有的群提取出来
参数：InUserName 输入的用户名
OutGroupsName 返回的用户的群名，放入一个数组中
OutGroupNums 返回的群组的个数
返回值：0 函数执行成功 1 函数执行失败 */
int getUserGroups(const char* InUserName, char OutGroupsName[][20], int* OutGroupNums)
{
	int nRet = 0;
	MYSQL my_connection;
	char szCmd[100] = { 0 };
	MYSQL_RES*  result;
	MYSQL_ROW row;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};
	
	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{	
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getUserGroups-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);

		return 1;
	}

	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT group_name FROM tbl_users_groups WHERE name='", InUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getUserGroups-->mysql_query: exec cmd failed.");
		mysqlLOG(errorMsg);
		
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);

	/* 把用户的所有的好友查出来 */
	*OutGroupNums = 0;
	while ((row = mysql_fetch_row(result)))
	{
		strncpy(OutGroupsName[*OutGroupNums], row[0], 20);
		(*OutGroupNums) = (*OutGroupNums) + 1;
	}

	/* 释放空间，关闭与数据库的连接 */
	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}


/*函数功能：判断用户是否被禁言,如果被禁言的话，直接可以把禁言的时间查出来
参数：InUserName 输入的用户名
OutIsGag 输出是否被禁言，0 没有被禁言，1 被禁言
OutgagMinutes 被禁言的时间
返回值：0 函数执行成功 1 函数执行失败 */
int isUserByGag(const char* InUserName, int* OutIsGag, int* OutgagMinutes)
{
	int nRet = 0;
	MYSQL my_connection;
	char szCmd[100] = { 0 };
	MYSQL_RES*  result;
	int count = 0;
	MYSQL_ROW row;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{		
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isUserByGag-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 组合查找命令 */
	sprintf(szCmd, "%s%s%s", "SELECT gag_minutes FROM tbl_gag_users WHERE gag_name='", InUserName, "'");

	/* 执行命令 */
	nRet = mysql_query(&my_connection, szCmd);
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"isUserByGag-->mysql_query: exec cmd failed.");
		mysqlLOG(errorMsg);
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);

	while ((row = mysql_fetch_row(result)))
	{
		*OutgagMinutes = atoi(row[0]);
		count++;
	}

	if (count == 0)
	{
		/* 没有被禁言 */
		*OutIsGag = 0;
		*OutgagMinutes = 0;
	}
	else
	{
		/* 被禁言 */
		*OutIsGag = 1;
	}
	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}

/*函数功能：判断输入的字符串是否含有敏感词
参数：
InWords 聊天的一句话
OutIsSensWord 是否含有敏感词 0 含有 1 没有
返回值：0 函数执行成功 1 函数执行失败 */
int isSensitiveWords(const char* InWords, int* OutIsSensWord)
{
	int nRet = 0;
	MYSQL my_connection;

	MYSQL_RES*  result;
	MYSQL_ROW row;
	char OutSensWords[20][20];
	int OutWordNums = 0;
	/* 初始化数据库 */
	mysql_init(&my_connection);
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getSensitiveWords-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 执行命令 */
	nRet = mysql_query(&my_connection, "SELECT * FROM tbl_sensitive_words");
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getSensitiveWords-->mysql_query: exec cmd failed.");
		mysqlLOG(errorMsg);
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);

	/* 把所有的敏感词查出来*/
	OutWordNums = 0;
	while ((row = mysql_fetch_row(result)))
	{
		strncpy(OutSensWords[OutWordNums], row[0], 20);
		(OutWordNums) = (OutWordNums)+1;
	}

	int i = 0, j = 0;
	int nCount = 0;

	/* 每一个敏感词都要判断 */
	for (i = 0; i < OutWordNums; i++)
	{
		for (j = 0; j < strlen(InWords); j++)
		{
			if (InWords[j] == OutSensWords[i][0])
			{
				if (strncmp(InWords + j, OutSensWords[i], strlen(OutSensWords[i])) == 0)
				{
					nCount++;
					break;
				}

			}

		}
		if (nCount != 0)
		{
			break;
		}
	}
	if (nCount == 0)
	{
		/* 没有含有敏感词 */
		*OutIsSensWord = 1;
	}
	else
	{
		/* 含有敏感词 */
		*OutIsSensWord = 0;
	}

	/* 释放空间，关闭与数据库的连接 */
	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}


/*函数功能：把含有敏感词的字符串改造
参数：
InOutWords 聊天的一句话,直接在原来的字符串上修改
OutChangeWord 改造之后的一句话
例子：原句：你是，滚蛋吧
改造：你是，**吧
返回值：0 函数执行成功 1 函数执行失败 */
int changeSensitiveWords(char* InOutWords)
{
	int nRet = 0;
	MYSQL my_connection;

	MYSQL_RES*  result;
	MYSQL_ROW row;
	char OutSensWords[20][20];
	int OutWordNums = 0;
	int i = 0, j = 0;
	char errorMsg[MYSQL_ERROR_MSG_LENGTH] = {0};
	
	/* 初始化数据库 */
	mysql_init(&my_connection);

	/* 连接数据库 */
	if (!mysql_real_connect(&my_connection, "localhost", "root", "passwd", "db_bsqq", 0, NULL, 0))
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getSensitiveWords-->mysql_real_connect: connect mysql failed.");
		mysqlLOG(errorMsg);
		return 1;
	}

	/* 执行命令 */
	nRet = mysql_query(&my_connection, "SELECT * FROM tbl_sensitive_words");
	if (0 != nRet)
	{
		memset(errorMsg,0,MYSQL_ERROR_MSG_LENGTH);
		sprintf(errorMsg,"%s %d: %s",__FILE__,__LINE__,"getSensitiveWords-->mysql_query: exec cmd failed.");
		mysqlLOG(errorMsg);
		
		mysql_close(&my_connection);
		return 1;
	}

	/* 把查询到的数据取出来 */
	result = mysql_store_result(&my_connection);

	/* 把所有的敏感词查出来*/
	OutWordNums = 0;
	while ((row = mysql_fetch_row(result)))
	{
		strncpy(OutSensWords[OutWordNums], row[0], 20);
		(OutWordNums) = (OutWordNums)+1;
	}

	/* 每一个敏感词都要判断 */
	for (i = 0; i < OutWordNums; i++)
	{
		for (j = 0; j < strlen(InOutWords); j++)
		{
			if (InOutWords[j] == OutSensWords[i][0])
			{
				if (strncmp(InOutWords + j, OutSensWords[i], strlen(OutSensWords[i])) == 0)
				{
					strncpy(InOutWords + j, "****************", strlen(OutSensWords[i]));
				}

			}

		}
	}

	/* 释放空间，关闭与数据库的连接 */
	mysql_free_result(result);
	mysql_close(&my_connection);
	return 0;
}


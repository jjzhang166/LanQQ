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


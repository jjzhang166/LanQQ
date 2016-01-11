#ifndef _MYSQLQQ_H_
#define _MYSQLQQ_H_

#define MYSQL_ERROR_MSG_LENGTH 300  //错误信息的字符串最大长度

/*函数功能：MySQL的日志函数
	  参数：InLog要打印的日志
    返回值：0 函数执行成功 1 函数执行失败*/
void mysqlLOG(const char* InLog);

/*函数功能：执行数据库命令，可以插入、删除、更新
InExecCmd 要执行的命令
返回值：0 函数执行成功 1 函数执行失败
示例："INSERT INTO tbl_gag_users(gag_name,gag_minutes) VALUES('user1de9',5)"
"DELETE FROM tbl_gag_users WHERE gag_name='user1de5' "*/
int execCmdToMysql(const char* InExecCmd);



#endif
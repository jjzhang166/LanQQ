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

/*函数功能：判断该用户是否是超级用户
参数：
InName 输入的用户名
OutIsSuperUser 返回是否是超级用户 0 是 1 不是
返回值：0 函数执行成功 1 函数执行失败 */
int IsSuperUser(const char* InName, int* OutIsSuperUser);

/*函数功能：判断超级管理员用户的密码是否正确
参数：InSpuerUserName 输入的用户名
InPassword 输入的密码
OutIsTrue 输出是否正确，0 正确 1 错误
返回值：0 函数执行成功 1 函数执行失败 */
int isSpuerUserPasswordTrue(const char* InSuperUserName,const char* InPassword, int* OutIsTrue);

/*函数功能：判断用户是否已结被删除过，删除过就不能再注册了
参数：InUserName 输入的用户名
OutIsDelete 输出是否被禁言，0 没有被删除过，1 被删除过
返回值：0 函数执行成功 1 函数执行失败 */
int isAlreadyBeDelete(const char* InUserName, int* OutIsDelete);

/*函数功能：根据用户名判断用户是否已经注册过
参数：InUserName 输入的用户名
OutIsResiger 输出是否已经注册，0 没有注册，1 已经注册
返回值：0 函数执行成功 1 函数执行失败 */
int isUserAlreadyResiger(const char* InUserName, int* OutIsResiger);

/*函数功能：判断用户的密码是否正确
参数：InUserName 输入的用户名
InPassword 输入的密码
OutIsTrue 输出是否正确，0 正确 1 错误
返回值：0 函数执行成功 1 函数执行失败 */
int isUserPasswordTrue(const char* InUserName, const char* InPassword, int* OutIsTrue);

/*函数功能：根据用户名判断用户是否在线
参数：InUserName 输入的用户名
OutIsOnline 输出是否在线，0 没有在线，1 在线
返回值：0 函数执行成功 1 函数执行失败 */
int isUserAlreadyOnline(const char* InUserName, int* OutIsOnline);

/*函数功能：判断密保是否正确，
参数：InUserName 用户名
Insecuriety输入的密保
OutIsTrue 密保是否正确 0  用户名不存在 1 密保正确 2  密保不正确
返回值：0 函数执行成功 1 函数执行失败 */
int isSecurietyTrue(const char* InUserName, const char* Insecuriety, int* OutIsTrue);

/*函数功能：根据用户名把该用户服务器与客户端的的socketfd找出来
参数：InUserName 输入的用户名
OutSocketFd 返回通信的文件描述符
返回值：0 函数执行成功 1 函数执行失败 */
int getUserSocketFd(const char* InUserName, int* OutSocketFd);

#endif
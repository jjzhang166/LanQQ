#ifndef _NORMAL_SERVER_H_
#define _NORMAL_SERVER_H_

#define SERVER_ERROR_MSG_LENGTH 300  //错误信息的字符串最大长度

/*函数功能：服务器的日志函数
	  参数：InLog要打印的日志*/
void normal_serverlLOG(const char* InLog);

/*函数功能：系统日志函数
          参数：Inlog：所要记录的日志信息

    返回值：NULL */
void My_log(const char* Inlog);

/*函数功能：解析了前2个字节判断是注册信息后调用该函数解析注册信息
返回给客户端的协议：111,注册成功 112,该用户已注册，失败 113,永远不能注册的用户
参数：
sockfd:注册用户连入服务器的的clifd，用来读写数据，并放于注册数据库表中
所用数据库函数
使用的数据库函数
int isUserAlreadyResiger(char* InUserName,int* OutIsResiger);
int execCmdToMysql(char* InExecCmd);
int isAlreadyBeDelete(char* InUserName,int* OutIsDelete); 判断用户是否已结被删除过，删除过就不能再注册了
int isUserSocketfdUsed(char* InUserName,int* OutIsSocketfdUsed);每次写的时候都要判断该sockfd是否被占用
返回值：0 函数执行成功 1 函数执行失败 */
int AnalyRegister(const char* buff, int sockfd);

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

int AnalyLogin(const char* buff,int sockfd);

#endif
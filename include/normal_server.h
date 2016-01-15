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

int Retrieve_psw(const char* buff,const int sockfd);

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

int Add_friend_forward(const char* buff,int sockfd);

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
int Add_friend_agree(const char* buff);

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
返回值：0 函数执行成功 1 函数执行失败 */
int Add_friend_refuse(const char* buff);

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

int Private_chat(const char* buff, int sockfd);

#endif
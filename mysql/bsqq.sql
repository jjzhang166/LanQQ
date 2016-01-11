

-- 存放所有已经注册用户的信息
CREATE TABLE tbl_register_users
(
	name VARCHAR(20) NOT NULL PRIMARY KEY,  -- 主键，用户的名字
	passwd VARCHAR(20) NOT NULL,   -- 用户的密码，MD5加密过
	socketfd INT NOT NULL,        -- 客户端与服务器通信的fd
	is_online  TINYINT  NOT NULL,-- 是否在线 0 不在线  1 在线
	is_used TINYINT NOT NULL, -- 该文件描述符现在是否被用 0 没有被占用 1 被占有
    securiety VARCHAR(30) NOT NULL,  -- 密保
    isOfflineMsg TINYINT NOT NULL, -- 是否有离线的单聊消息 0 没有, 1 有离线消息
	isofflineGroupMsg TINYINT NOT NULL, -- 是否有离线的群的消息 0 没有 1 有
	isOfflineBroadMsg TINYINT NOT NULL -- 是否有离线的广播消息，0 没有 1有
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- 存放被禁言的用户，当禁言时间到了之后，就要把用户从禁言列表里面删除
CREATE TABLE tbl_gag_users
(
	gag_name VARCHAR(20) NOT NULL PRIMARY KEY,  -- 主键
	gag_minutes INT NOT NULL -- 禁言时间
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';


-- 存放被管理员删除的用户，已经被加入该列表的用户，不能再注册
CREATE TABLE tbl_del_users
(
	del_name VARCHAR(20) NOT NULL PRIMARY KEY -- 主键
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- 存放某个用户和他的好友的信息
CREATE TABLE tbl_users_friends
(
	name VARCHAR(20) NOT NULL ,  	
	friend_name VARCHAR(20) NOT NULL,	
	CONSTRAINT pk_friend_name PRIMARY KEY (name,friend_name)  -- 组合键作为主键
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';


-- 存放某个用户和他的QQ群的信息
CREATE TABLE tbl_users_groups
(
	name VARCHAR(20) NOT NULL ,  
	group_name VARCHAR(20) NOT NULL,	
	CONSTRAINT pk_group_name PRIMARY KEY (name,group_name)  -- 组合键作为主键
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- 存放敏感词的信息
CREATE TABLE tbl_sensitive_words
(
	sensitive_word VARCHAR(30) NOT NULL PRIMARY KEY -- 主键
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- 超级用户数据库表
CREATE TABLE tbl_super_users
(
	super_name VARCHAR(20) NOT NULL PRIMARY KEY, -- 主键
	passwd VARCHAR(20) NOT NULL
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- INSERT INTO tbl_sensitive_words VALUES('ddd'),('ssss'),('eee');
INSERT INTO tbl_super_users VALUES('root','111111');  -- 插入一个默认的管理员账户



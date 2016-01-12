

-- ��������Ѿ�ע���û�����Ϣ
CREATE TABLE tbl_register_users
(
	name VARCHAR(20) NOT NULL PRIMARY KEY,  -- �������û�������
	passwd VARCHAR(20) NOT NULL,   -- �û������룬MD5���ܹ�
	socketfd INT NOT NULL,        -- �ͻ����������ͨ�ŵ�fd
	is_online  TINYINT  NOT NULL,-- �Ƿ����� 0 ������  1 ����
	is_used TINYINT NOT NULL, -- ���ļ������������Ƿ��� 0 û�б�ռ�� 1 ��ռ��
    securiety VARCHAR(30) NOT NULL,  -- �ܱ�
    isOfflineMsg TINYINT NOT NULL, -- �Ƿ������ߵĵ�����Ϣ 0 û��, 1 ��������Ϣ
	isofflineGroupMsg TINYINT NOT NULL, -- �Ƿ������ߵ�Ⱥ����Ϣ 0 û�� 1 ��
	isOfflineBroadMsg TINYINT NOT NULL -- �Ƿ������ߵĹ㲥��Ϣ��0 û�� 1��
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- ��ű����Ե��û���������ʱ�䵽��֮�󣬾�Ҫ���û��ӽ����б�����ɾ��
CREATE TABLE tbl_gag_users
(
	gag_name VARCHAR(20) NOT NULL PRIMARY KEY,  -- ����
	gag_minutes INT NOT NULL -- ����ʱ��
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';


-- ��ű�����Աɾ�����û����Ѿ���������б���û���������ע��
CREATE TABLE tbl_del_users
(
	del_name VARCHAR(20) NOT NULL PRIMARY KEY -- ����
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- ���ĳ���û������ĺ��ѵ���Ϣ
CREATE TABLE tbl_users_friends
(
	name VARCHAR(20) NOT NULL ,  	
	friend_name VARCHAR(20) NOT NULL,	
	CONSTRAINT pk_friend_name PRIMARY KEY (name,friend_name)  -- ��ϼ���Ϊ����
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';


-- ���ĳ���û�������QQȺ����Ϣ
CREATE TABLE tbl_users_groups
(
	name VARCHAR(20) NOT NULL ,  
	group_name VARCHAR(20) NOT NULL,	
	CONSTRAINT pk_group_name PRIMARY KEY (name,group_name)  -- ��ϼ���Ϊ����
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- ������дʵ���Ϣ
CREATE TABLE tbl_sensitive_words
(
	sensitive_word VARCHAR(30) NOT NULL PRIMARY KEY -- ����
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- �����û����ݿ��
CREATE TABLE tbl_super_users
(
	super_name VARCHAR(20) NOT NULL PRIMARY KEY, -- ����
	passwd VARCHAR(20) NOT NULL
)ENGINE=InnoDB,CHARACTER SET 'gbk' COLLATE 'gbk_chinese_ci';

-- INSERT INTO tbl_sensitive_words VALUES('ddd'),('ssss'),('eee');
INSERT INTO tbl_super_users VALUES('root','111111');  -- ����һ��Ĭ�ϵĹ���Ա�˻�



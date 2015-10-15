//sqltextdefine.h
//数据库操作常用的sql语句都在这里定义
#ifndef _db_operate_sql_
#define _db_operate_sql_

// 创建角色条件：账号已经被当前服务器锁定，角色名不能重复，同一账号下索引号不能重复
#define INSERT_PROPERTY_ONLYNAME    " insert into property ( account, rolename, roleindex ) " \
                                    " select '%s', '%s', %d from dual where exists " \
                                    " ( select 1 from warehouse where account = '%s' and serverid = %d ) " /*\
                                    " and not exists ( select 1 from property where rolename = '%s' or ( account = '%s' and roleindex = %d ) ) "*/

#define SELECT_PROPERTY_ROLELIST    " select rolebuf, roleindex from property where account = '%s' " \
                                    " and exists ( select 1 from warehouse where account = '%s' and serverid = %d ) "

#define SELECT_PROPERTY_ROLEID      " select roleid from property where rolename = '%s' "

#define SELECT_ALL_PROPERTY_DATA    " select roleid, rolename, databuf, verify from property where account = '%s' and roleindex = %d " \
                                    " and exists ( select 1 from warehouse where account = '%s' and serverid = %d ) "

#define UPDATE_PROPERTY_LOGINTIME   " update property set logintime = now() where roleid = %u " \
                                    " and exists ( select 1 from warehouse where account = '%s' and serverid = %d ) "

#define DELETE_PROPERTY_ROLE        " delete from property where roleid = %u " \
                                    " and exists ( select 1 from warehouse where account = '%s' and serverid = %d ) "

#define UPDATE_PROPERTY_ONLYBUF     " update property set databuf = ?, rolebuf = ?, verify = ?, Level = ?, RankList = ?, tollgate = ?, viplv = ?, guideID = ?, savetime = now() where roleid = %u " \
                                    " and exists ( select 1 from warehouse where account = '%s' and serverid = %d ) "

#define UPDATE_PLAYER_CONFIG   " update property set PlayerConfig ='%s' where account = '%s' "

#define CHECK_PROPERTY_EXIST         " select RoleID from property where account = '%s' "
// ################# accounts #############################################

#define INSERT_WAREHOUSE            " insert into warehouse ( account, serverid, lastModify,registerTime ) " \
                                    " select '%s', %d, now(), now() from dual where not exists " \
                                    " ( select 1 from warehouse where account = '%s' ) "

//#define INSERT_WAREHOUSE            " insert into warehouse ( account, serverid, lastModify ) " \
//                                    " values( '%s', %d, now() )"

#define SELECT_WAREHOUSE            " select locked, databuf from warehouse where account = '%s' and serverid = %d "

//#define SELECT_WAREHOUSE            " select locked, databuf from warehouse where account = '%s' and name = '%s' and serverid = %d "

#define UPDATE_WAREHOUSE            " update warehouse set locked = %d, databuf = ?, lastModify = now() where account = '%s' and serverid = %d "

//#define UPDATE_WAREHOUSE            " update warehouse set locked = %d, databuf = ?, lastModify = now() where account = '%s' and name = '%s' and serverid = %d "

#define CHECK_ACCOUNT_EXIST         " select serverid from warehouse where account = '%s' "

#define UPDATE_ACCOUNT_LOCK         " update warehouse set serverid = %d where account = '%s' and serverid = 0 "

#define UPDATE_ACCOUNT_UNLOCK       " update warehouse set serverid = 0 where account = '%s' and serverid = %d "

#define CHECK_ACCOUNT_FROZEN         " select Frozen  from warehouse where account = '%s' "

// ########################################################################

#define INSERT_FACTION              " insert into factionlist ( updatetime, data, serverid, factionname, factionid ) values ( ?, ?, ?, ?, ? ) "

#define UPDATE_FACTION              " update factionlist set data = ? where serverid = ? and factionid = ? "

#define GET_FACTIONLIST				" select data from factionlist where serverid = %u and updatetime = ( select max( updatetime ) from factionlist where serverid = %u ) "

#define DELETE_FACTION               "delete from factionlist where serverid = %d and factionid = %d "

#define SAVE_SCRIPTDATA             "insert into scriptdata ( data ) values (?)"

#define GET_SCRIPTDATA              " select data from scriptdata "

#define UPDATE_SCRIPTDATA             "update scriptdata set data = ? "

// ########################################################################

#define GET_PLAYERNAME              " select rolename from property where roleid = %u "

#define GET_UNIONLIST               " select title, suffix, roleid1, roleid2, roleid3, roleid4, roleid5, roleid6, roleid7, roleid8, exp1, exp2, exp3, exp4, exp5, exp6, exp7, exp8 from unions where updatetime = ( select max( updatetime ) from unions  ) "

#define UPDATE_UNION                " update unions set roleid1 = ?, roleid2 = ?, roleid3 = ?, roleid4 = ?, roleid5 = ?, roleid6 = ?, roleid7 = ?, roleid8 = ?, exp1 = ?, exp2 = ?, exp3 = ?, exp4 = ?, exp5 = ?, exp6 = ?, exp7 = ?, exp8 = ?, updatetime = ? where title = ? and suffix = ?"

#define INSERT_UNION                " insert into unions( title, suffix, ctime, roleid1, roleid2, roleid3, roleid4, roleid5, roleid6, roleid7, roleid8, exp1, exp2, exp3, exp4, exp5, exp6, exp7, exp8, updatetime ) values( ?, ?, now(), ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ? ) "
// ########################################################################

#define INSERT_BUILDING             " insert into buildinglist ( updatetime, buffer, serverid, parentid, uniqueid ) values ( ?, ?, ?, ?, ? ) "

#define UPDATE_BUILDING             " update buildinglist set updatetime = ?, buffer = ? where serverid = ? and parentid = ? and uniqueid = ? "

#define GET_BUILDINGLIST			" select buffer from buildinglist where serverid = %u and parentid = %u and updatetime = " \
                                    " ( select max( updatetime ) from buildinglist where serverid = %u and parentid = %u ) "

// ########################################################################

#define SELECT_PROCEDURE_PARAMLIST  " select replace( replace( cast( param_list as char ), char( 13 ), '' ), char( 10 ), '' ) " \
                                    " as paramlist from mysql.proc where db = '%s' and name = '%s' "

//#define UPDATE_SCORELIST            " update scorelist_raw set level = %d, money = %I64d, xvalue = %d, name = '%s', updatetime = now() where roleid = %d "
//
//#define INSERT_SCORELIST            " insert into scorelist_raw( level, money, xvalue, name, roleid, updatetime ) values ( %d, %I64d, %d, '%s', %d, now() ) "


// ########################################################################
#define INSERT_SCORELIST            " insert into scorelist_raw( level, money, xvalue, name, roleid, updatetime, factionname, killmonster, killplayer, deadtimes, onlinetime, openbox, usexydpoint, school, ambit ) values ( %d, %I64d, %d, '%s', %d, now(), '%s', %d, %d, %d, %d, %d, %d, %d, %d ) "

#define UPDATE_SCORELIST            " update scorelist_raw set level = %d, money = %I64d, xvalue = %d, name = '%s', updatetime = now(), factionname='%s', killmonster=%d, killplayer=%d, deadtimes=%d, onlinetime=%d, openbox=%d, usexydpoint= %d, school = %d, ambit = %d where roleid = %d "


//////////////////////

//提取数据相关
// ########################################################################

#define SELECT_ALL_PLAYER_DATA    " select account, rolename, rolebuf, databuf,roleindex from property "
#define SELECT_ALL_PLAYER_RANKLIST "select RankList from property where level >= %d"


//三国 2015.1.12 wk 邮件系统 存储过程调用##begin###################################################################

//#define CALL_SEND_MAIL   "call p_EmailSend (%d, '%s', %d, ?, %d, %d, %d)"
//#define CALL_GET_MAIL   "call p_EmailGetIsNew (%d, %d, %d, %d)"
//#define CALL_change_name   "call ChangeRoleName ('%s','%s','%s', %d, %d,@%s)"

//根据id发邮件
#define INSERT_SEND_MAIL           " insert into  emailmsg  (content,sendername, receiverid,emailtype, itemid,itemnum,pubid,isrecvattach,isread,inreceivebox,isdeleted,inserttime,serverid,expiretime) values (?,'', %d, %d,  0, 0, 0, 0, 0, 0, 0, now(), %d, date_add(now(), INTERVAL %d minute))"
//根据名字发邮件
#define INSERT_SEND_MAIL_BYNAME           " insert into  emailmsg  (content,sendername, receiverid,emailtype, itemid,itemnum,pubid,isrecvattach,isread,inreceivebox,isdeleted,inserttime,serverid,expiretime) values (?,'', %d, %d,  0, 0, 0, 0, 0, 0, 0, now(), %d, date_add(now(), INTERVAL %d minute))"

#define SELECT_MAILINFO_LIST    " select content, id,isread from emailmsg where receiverid=%d and isread<%d and expiretime>now()  "
#define SELECT_MAILINFO_ONE    " select content,isread from emailmsg where id=%d "
#define UPDATE_MAIL_ISREAD    " update emailmsg set  isread=%d  where id=%d"
#define CALL_SETMAILSTATE   "call p_EmailSetRead(%d,%d,@ret)"
#define CALL_SEND_MAIL   "call p_EmailSend (%d, '%s', %d, ?, %d, %d, %s)"
#define CALL_SEND_MAILBYNAME   "call p_EmailSendByName (%d, '%s', %d, ?, %d, %d, %s)"

#define CALL_GETMAILFROM_MAILSYS   "call p_EmailGetIsNew (%d, %d, 0 , %s)"

//三国 2015.1.12 wk 邮件系统 存储过程调用##end###################################################################
#define CALL_POINT_LOG   "call p_pointlog (%d,  '%s',%d,%d,%d,%d,%d,%d)"

#endif
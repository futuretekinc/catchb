#include <common.h>
#include <libdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <mysql/mysql.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <poll.h>


int db_open(DB *mysql)
{
#if 0
    mysql_init(mysql);
    if (!mysql_real_connect(mysql, NULL, DB_USER, DB_PASS, DB_NAME, DB_PORT,(char *)NULL, 0)) {

        printf("%s\n", mysql_error(mysql));
        return WDB_ERROR;
    }
#endif
    return WDB_SUCCESS;
}

int init_db_open(DB *mysql)
{
#if 0
    mysql_init(mysql);
    if (!mysql_real_connect(mysql, NULL, DB_USER, DB_PASS, NULL, DB_PORT,(char *)NULL, 0)) {

        printf("%s\n", mysql_error(mysql));
        return WDB_ERROR;
    }
#endif
    return WDB_SUCCESS;
}

 int db_close(DB *mysql) 
{
#if 0
    mysql_close(mysql);
#endif
    return WDB_SUCCESS;
}
int create_db()
{
#if 0
    int rc;
    DB mysql;

    rc = init_db_open(&mysql);

    if (rc != WDB_SUCCESS) {
        printf("%s\n", mysql_error(&mysql));
        db_close(&mysql);
        return;
    }

    if (mysql_query(&mysql, CREATE_DB)) {
        printf("%s\n", mysql_error(&mysql));
        return WDB_ERROR;
    }

    mysql_close(&mysql);

#endif
    return WDB_SUCCESS;
}

int init_data(DB *mysql)
{
#if 0
    if (mysql_query(mysql, INSERT_TB_CODEINFO_VALUE_1)) {
        printf("11%s\n", mysql_error(mysql));
    }

    if (mysql_query(mysql, INSERT_TB_CODEINFO_VALUE_2)) {
        printf("11%s\n", mysql_error(mysql));
    }
    if (mysql_query(mysql, INSERT_TB_CODEINFO_VALUE_3)) {
        printf("11%s\n", mysql_error(mysql));
    }

    if (mysql_query(mysql, INSERT_TB_CODEINFO_SWITCH_DASAN_VALUE)) {
        printf("11%s\n", mysql_error(mysql));
    }

    if (mysql_query(mysql, INSERT_TB_CODEINFO_SWITCH_JUNIPER_VALUE)) {
        printf("11%s\n", mysql_error(mysql));
    }

    if (mysql_query(mysql, INSERT_TB_CODEINFO_SWITCH_NST_VALUE)) {
        printf("11%s\n", mysql_error(mysql));
    }
#endif
	return	WDB_SUCCESS;
}

int init_create_table(DB *mysql)
{
#if 0
    if (mysql_query(mysql, CREATE_SYSTEM_ADMIN_TABLE)) {
        printf("11%s\n", mysql_error(mysql));
    }
    if (mysql_query(mysql, CREATE_CHECK_RESULT_INFO)) {
        printf("33%s\n", mysql_error(mysql));
    }
    //if (mysql_query(mysql,CREATE_CHECK_IP_RESULT )) {
    //    printf("%s\n", mysql_error(mysql));
    //}
    if (mysql_query(mysql,CREATE_CHECK_DENY_IP )) {
        printf("55%s\n", mysql_error(mysql));
    }
    if (mysql_query(mysql, CREATE_CHECK_LINK_INFO)) {
        printf("66%s\n", mysql_error(mysql));
    }
    if (mysql_query(mysql, CREATE_CHECK_ALARM_INFO)) {
        printf("77%s\n", mysql_error(mysql));

    }

    if (mysql_query(mysql, CREATE_CHECK_SWITCH_IP_DETECTION_INFO)) {
        printf("77%s\n", mysql_error(mysql));

    }

    if (mysql_query(mysql, CREATE_CHECK_SYSTEM_RESOURCE)) {
        printf("88%s\n", mysql_error(mysql));

    }
   if (mysql_query(mysql,CREATE_CHECK_UPDATE )) {
        printf("99%s\n", mysql_error(mysql));

    }
   if (mysql_query(mysql,CREATE_CHECK_CODE_INFO )) {
        printf("00%s\n", mysql_error(mysql));

    }

  if (mysql_query(mysql, CREATE_CHECK_IP_INFORMATION)) {
        printf("22--%s\n", mysql_error(mysql));
    }

if (mysql_query(mysql, CREATE_CHECK_IP_RESULT_LOG)) {
        printf("44%s\n", mysql_error(mysql));
    }

#endif

    return WDB_SUCCESS;
}


void initdb()
{
#if 0
    int rc;
    DB mysql;

    rc = db_open(&mysql);

    if (rc != WDB_SUCCESS) {
        printf("%s\n", mysql_error(&mysql));
        db_close(&mysql);
        return;
    }

   init_create_table(&mysql);

   init_data(&mysql);

   db_close(&mysql);
#endif
}

const char*	CREATE_DB="CREATE DATABASE IF NOT EXISTS `CCTV_CHECK_DB` /*!40100 DEFAULT CHARACTER SET utf8 */;";

const char*  CREATE_CHECK_RESULT_INFO="CREATE TABLE IF NOT EXISTS `tb_ck_result_info` (`C_ID` int(11) NOT NULL AUTO_INCREMENT COMMENT 'idx',`C_CCTV_IDX` int(11) DEFAULT NULL COMMENT 'tb_ck_ip_info',`C_CCTV_ID` varchar(24) NOT NULL COMMENT 'CCTV ID', `C_CCTV_IP` varchar(24) NOT NULL COMMENT 'CCTV_IP',`C_CCTV_SIGNATURE` varchar(128) NOT NULL COMMENT 'CCTV SIGNATURE','CODE' NOT NULL COMMENT 'CODE',PRIMARY KEY(`C_ID`)) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8 COMMENT='CCTV RESULT INFO';";

const char*  CREATE_SYSTEM_ADMIN_TABLE="CREATE TABLE IF NOT EXISTS `tb_admin` (`C_IDX` int(11) NOT NULL AUTO_INCREMENT COMMENT 'idx',`C_ID` varchar(20) NOT NULL DEFAULT '0' COMMENT 'AH ID',`C_PASSWORD` varchar(128) NOT NULL DEFAULT '0' COMMENT 'AH PASSWORD', PRIMARY KEY (`C_IDX`), UNIQUE KEY `index 2` (`C_ID`)) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COMMENT='AH TABLE';";

const char*  CREATE_CHECK_ALARM_INFO="CREATE TABLE IF NOT EXISTS `tb_ck_alarm_info` (  `C_ID` int(11) NOT NULL AUTO_INCREMENT COMMENT 'idx',    `C_ALARM_NAME` varchar(64) NOT NULL DEFAULT '0' COMMENT '°ü¸®ÀÚ¸í',      `C_ALARM_MAIL_ADDRESS` varchar(24) NOT NULL DEFAULT '0' COMMENT '°ü¸®ÀÚ ÀÌ¸ÞÀÏ',        PRIMARY KEY (`C_ID`)    ) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8 COMMENT='¾Ë¶÷ °ü¸® Å×ÀÌºí';";////

const char*  CREATE_CHECK_DENY_IP="CREATE TABLE IF NOT EXISTS `tb_ck_deny_ip` (  `C_ID` int(11) NOT NULL AUTO_INCREMENT COMMENT 'Á¤Ã¥ ID',    `C_SWITCH_ID` varchar(50) NOT NULL COMMENT 'Switch ID',  `C_SWITCH_LOGIN_ID` varchar(20) NOT NULL DEFAULT '0' COMMENT 'LOGIN ID',    `C_SWITCH_PASSWORD` varchar(128) NOT NULL COMMENT 'Switch Password',  `C_SWITCH_NAME` varchar(64) DEFAULT NULL COMMENT 'Switch Name',          `C_SWITCH_IP` varchar(24) NOT NULL COMMENT 'Switch IP', `C_SWITCH_TYPE` tinyint(4) NOT NULL DeFAuLT '0' COMMENT 'switch type',`C_SWITCH_COMMENT` varchar(64) DEFAULT NULL COMMENT '¼³¸í', `C_SWITCH_KIND` int(11) NOT NULL DEFAULT '0' COMMENT 'SWITCH KIND',   PRIMARY KEY (`C_ID`)    ) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8 COMMENT='Â÷´ÜÀåºñ Å×ÀÌºí';";////

const char*  CREATE_CHECK_IP_INFORMATION="CREATE TABLE IF NOT EXISTS `tb_ck_ip_info` (  `C_ID` int(11) NOT NULL AUTO_INCREMENT COMMENT 'CCTV idx',    `C_CCTV_ID` varchar(24) NOT NULL COMMENT 'CCTV ID¸í',   `C_CCTV_IP` varchar(24) NOT NULL COMMENT 'CCTV IP',  `C_CCTV_COMMENT` varchar(64) DEFAULT NULL COMMENT 'CCTV ¼³¸í',   `C_CCTV_TIME` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'CCTV µî·Ï½Ã°£',            PRIMARY KEY (`C_ID`)    ) ENGINE=InnoDB AUTO_INCREMENT=19 DEFAULT CHARSET=utf8 COMMENT='CCTV Á¤º¸ Å×ÀÌºí';";////

const char*  CREATE_CHECK_IP_RESULT_LOG="CREATE TABLE IF NOT EXISTS `tb_ck_ip_result_log` (  `C_ID` int(11) NOT NULL AUTO_INCREMENT COMMENT 'idx',`C_CCTV_IDX` int(11) DEFAULT NULL COMMENT 'tb_ck_ip_infoÀÇ idx¿Ü·¡',           `C_CCTV_ID` varchar(24) NOT NULL COMMENT 'ID', `C_RE_CCTV_IP` varchar(24) NOT NULL COMMENT 'IP', `C_RE_CCTV_SIGNATURE` varchar(128) NOT NULL COMMENT 'CCTV °íÀ¯ °ª',      `C_RE_CCTV_TIME` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT 'µî·Ï½Ã°£',      `C_RE_CCTV_STATUS` tinyint(4) NOT NULL DEFAULT '0' COMMENT '»óÅÂ',    `C_RE_CCTV_LOG` varchar(64) DEFAULT NULL COMMENT '·Î±×³»¿ë',        PRIMARY KEY (`C_ID`)) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COMMENT='·Î±× Å×ÀÌºí';";/////

const char*  CREATE_CHECK_LINK_INFO="CREATE TABLE IF NOT EXISTS `tb_ck_link_info` (  `C_ID` int(11) NOT NULL AUTO_INCREMENT COMMENT 'idx',    `C_LINK_IP_SERVER_NAME` varchar(64) NOT NULL DEFAULT '0' COMMENT '¿ÜºÎÀåºñ ¼­¹ö¸í',              `C_LINK_IP` varchar(24) NOT NULL DEFAULT '0' COMMENT '¿ÜºÎÀåºñ ip',           `C_LINK_IP_COMMENT` varchar(64) DEFAULT '0' COMMENT '¿ÜºÎÀåºñ ¼³¸í',                PRIMARY KEY (`C_ID`)    ) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COMMENT='¿ÜºÎ Àåºñ °ü¸® Å×ÀÌºí';;";//////


const char*  CREATE_CHECK_SWITCH_IP_DETECTION_INFO="CREATE TABLE IF NOT EXISTS `tb_ck_switch_detection_info` ( `C_ID` int(11) NOT NULL AUTO_INCREMENT    COMMENT 'idx', `C_DETECTION_IP` varchar(24) NOT NULL DEFAULT '0' COMMENT 'switch ip', `C_RULE_NUM` int NOT NULL COMMENT 'rule num',`C_RULE_STATE` int NOT NULL DEFAULT '0' COMMENT 'rule state' ,PRIMARY KEY (`C_ID`)) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COMMENT='switch info';";

const char*  CREATE_CHECK_SYSTEM_RESOURCE="CREATE TABLE IF NOT EXISTS `tb_ck_system_resource` (  `C_ID` int(11) NOT NULL AUTO_INCREMENT COMMENT 'idx',`C_DATE` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '¼öÁý½Ã°£',        `C_CPU_RATE` float NOT NULL DEFAULT '0' COMMENT 'CPU »ç¿ëÀ²',   `C_MEMORY_RATE` float NOT NULL DEFAULT '0' COMMENT 'Memory »ç¿ëÀ²', `C_TX_BYTES` int(11) NOT NULL DEFAULT '0', `C_RX_BYTES` int(11) NOT NULL DEFAULT '0',          PRIMARY KEY (`C_ID`)    ) ENGINE=InnoDB AUTO_INCREMENT=478 DEFAULT CHARSET=utf8 COMMENT='¼­¹ö ½Ã½ºÅÛ ¸®¼Ò½º °ª';";

const char*  CREATE_CHECK_UPDATE="CREATE TABLE IF NOT EXISTS `tb_ck_update` (  `C_ID` int(11) NOT NULL AUTO_INCREMENT,    `C_FILE_NAME` varchar(50) NOT NULL DEFAULT '0',`C_SAVE_DATE` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,`C_FILE_SIZE` int(11) NOT NULL DEFAULT '0',PRIMARY KEY (`C_ID`)    ) ENGINE=InnoDB AUTO_INCREMENT=8 DEFAULT CHARSET=utf8 COMMENT='¾÷µ¥ÀÌÆ®';";

const char*  CREATE_CHECK_CODE_INFO="CREATE TABLE IF NOT EXISTS `tb_code_info` (  `code` varchar(12) NOT NULL COMMENT 'ÄÚµå °ª',          `name` varchar(50) NOT NULL COMMENT 'ÄÚµå ¸í',      `parents_code` varchar(50) DEFAULT NULL COMMENT 'ºÎ¸ð ÄÚµå °ª',UNIQUE INDEX(name) ) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='ÄÚµå °ü¸® Å×ÀÌºí';";

const char*  INSERT_TB_CODEINFO_VALUE_1="INSERT INTO tb_code_info VALUES('A01', 'Status', null);";
const char*  INSERT_TB_CODEINFO_VALUE_2="INSERT INTO tb_code_info VALUES('1', 'normal', 'A01');";
const char*  INSERT_TB_CODEINFO_VALUE_3="INSERT INTO tb_code_info VALUES('2', 'Abnormal', 'A01');";

const char*  INSERT_TB_CODEINFO_SWITCH_DASAN_VALUE="INSERT INTO tb_code_info VALUES(0, 'DASAN', 'A02');";
const char*  INSERT_TB_CODEINFO_SWITCH_JUNIPER_VALUE="INSERT INTO tb_code_info VALUES(1, 'JUNIPER', 'A02');";
const char*  INSERT_TB_CODEINFO_SWITCH_NST_VALUE="INSERT INTO tb_code_info VALUES(2, 'NST', 'A02');";

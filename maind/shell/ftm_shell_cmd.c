#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_catchb.h"
#include "ftm_shell.h"
#include "ftm_db.h"
#include "ftm_mem.h"

FTM_RET	FTM_SHELL_CMD_alarm(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTM_SHELL_CMD_analyzer(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTM_SHELL_CMD_db(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTM_SHELL_CMD_cctv(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTM_SHELL_CMD_logger(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTM_SHELL_CMD_notifier(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTM_SHELL_CMD_switch(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTM_SHELL_CMD_trace(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);

FTM_SHELL_CMD	pCatchBShellCmdList[] = 
{
	{
		.pString	= "alarm",
		.function	= FTM_SHELL_CMD_alarm,
		.pShortHelp	= "Alarm",
		.pHelp		= "Alarm",
		.pData		= NULL
	},
	{
		.pString	= "analyzer",
		.function	= FTM_SHELL_CMD_analyzer,
		.pShortHelp	= "analyzer settings.",
		.pHelp		= "<cmd>\n"
					  "  Analyzer settings.\n"
					  "COMMANDs:\n"
					  "  cctv\n"
					  "    - Show CCTV list being monitored.\n",
		.pData		= NULL
	},
	{
		.pString	= "cctv",
		.function	= FTM_SHELL_CMD_cctv,
		.pShortHelp	= "CCTV(IP Camera) management.",
		.pHelp		= "<cmd>\n"
					  "  CCTV(IP Camera) management.\n"
					  "COMMANDS:\n"
					  "  list\n"
					  "    - Show CCTV list.\n"
					  "  add <ID> <IP>\n"
					  "    - Add new CCTV.\n"
					  "  del <ID>\n"
					  "    - Delete CCTV.\n"
					  "PARAMETERS:\n"
					  "  ID     CCTV Identifier\n"
					  "  IP     CCTV IP Address\n",
		.pData		= NULL
	},
	{
		.pString	= "db",
		.function	= FTM_SHELL_CMD_db,
		.pShortHelp	= "Database management.",
		.pHelp		= "<cmd>\n"
					  "  Database management.\n"
					  "COMMANDS:\n"
					  "  cctv\n"
					  "    - CCTV table management.\n",
		.pData		= NULL
	},
	{
		.pString	= "logger",
		.function	= FTM_SHELL_CMD_logger,
		.pShortHelp	= "Log management.",
		.pHelp		= "<cmd>\n"
					  "  Logger settings.\n"
					  "COMMANDS:\n"
					  "  show\n"
					  "    - Show the last 100 logs.\n"
					  "  show <COUNT>\n"
					  "    - Show the last <COUNT> logs.\n"
					  "  show <INDEX> <COUNT>\n"
					  "    - Show logs from <INDEX>.\n"
					  "  del id <ID> [<COUNT>]\n"
					  "    - Delete CCTV log for the ID.\n"
					  "  del ip <IP> [<COUNT>]\n"
					  "    - Delete CCTV log for the IP.\n",
					  "  del range <Start Time> <End Time>\n"
					  "    - Delete CCTV log for the time period.\n",
		.pData		= NULL
	},
	{
		.pString	= "notifier",
		.function	= FTM_SHELL_CMD_notifier,
		.pShortHelp	= "Alarm message settings.",
		.pHelp		= "<cmd>\n"
					  "  Alarm message settings.\n"
					  "COMMANDS:\n"
					  "  add <ID> <EMAIL> <MESSAGE>\n"
					  "    - Add Manager to receive alarms.\n"
					  "  del <ID>\n"
					  "    - Delete Manager.\n"
					  "PARAMETERS:\n"
					  "  ID      Manager name.\n"
					  "  EMAIL   e-mail to receive alarm.\n"
					  "  MESSAGE Alarm Message.\n",
		.pData		= NULL
	},
	{
		.pString	= "switch",
		.function	= FTM_SHELL_CMD_switch,
		.pShortHelp	= "Switch management.",
		.pHelp		= "<cmd>\n"
					  "  Switch configuration.\n"
					  "COMMANDS:\n"
					  "  add <ID> <MODEL> <IP> <USER> <PASSWD>\n"
					  "    - Add switch\n"
					  "  del <ID>\n"
					  "    - Delete switch\n"
					  "PARAMETERS:\n"
					  "  ID     Switch Identifier\n"
					  "  MODEL  Switch Model\n"
					  "  IP     Switch IP Address\n"
					  "  USER   User ID for switch management\n"
					  "  PASSWD Passwd for switch managment\n"
					  "MODEL:\n"
					  "  nst    NST\n"
					  "  dasan  Dasan networks.\n" ,
		.pData		= NULL
	},
	{
		.pString	= "trace",
		.function	= FTM_SHELL_CMD_trace,
		.pShortHelp	= "trace",
		.pHelp		= "trace",
		.pData		= NULL
	},
};

FTM_UINT32	ulCatchBShellCmdCount = sizeof(pCatchBShellCmdList) / sizeof(FTM_SHELL_CMD);

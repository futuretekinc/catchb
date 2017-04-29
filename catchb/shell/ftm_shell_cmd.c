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
FTM_RET	FTM_SHELL_CMD_config(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTM_SHELL_CMD_net(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);

FTM_SHELL_CMD	pCatchBShellCmdList[] = 
{
	{
		.pString	= "alarm",
		.function	= FTM_SHELL_CMD_alarm,
		.pShortHelp	= "alarm settings.",
		.pHelp		= "<cmd>\n"
					  "  Alarm settings.\n"
					  "COMMANDs:\n"
					  "  add <NAME> <EMAIL> <MESSAGE>\n"
					  "    - Add new notification email.\n"
					  "  del <NAME>\n"
					  "    _ Delete notification email.\n"
					  "  get <NAME>\n"
					  "    _ Show notification alarm information.\n"
					  "  set <NAME> --email <EMAIL> --message <MESSAGE>\n"
					  "    _ Delete notification email.\n"
					  "PARAMETERS:\n"
					  "  NAME   User name\n"
					  "  EMAIL	Destination email address.\n",
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
					  "  add <ID> [--ip <IP>] [--switchid <SWITCH_ID>] ...\n"
					  "    - Add new CCTV.\n"
					  "  del <ID>\n"
					  "    - Delete CCTV.\n"
					  "  get <ID>\n"
					  "    - show CCTV properties.\n"
					  "  set <ID> [--ip <IP>] [--switchid <SWITCH_ID] ...\n"
					  "    - Set CCTV properties.\n"
					  "PARAMETERS:\n"
					  "  ID     CCTV Identifier\n"
					  "OPTIONS:\n"
					  "  --ip <IP>\n"
					  "    CCTV IP Address\n"
					  "  --switchid <SWITCH_ID>\n"
					  "    The switch id to which the CCTV is connected.\n"
					  "  --comment <COMMENT>\n"
					  "    Comment.\n",
		.pData		= NULL
	},
	{
		.pString	= "config",
		.function	= FTM_SHELL_CMD_config,
		.pShortHelp	= "Configuration settings.",
		.pHelp		= "<cmd>\n"
					  "  Configuration settings.\n"
					  "COMMANDs:\n"
					  "  save [<file>]\n"
					  "    - Save configuration to file.\n",
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
		.pString	= "log",
		.function	= FTM_SHELL_CMD_logger,
		.pShortHelp	= "Log management.",
		.pHelp		= "<cmd>\n"
					  "  Log settings.\n"
					  "COMMANDS:\n"
					  "  show [--index <INDEX>] [--count <COUNT>]\n"
					  "    - Show logs from <INDEX>.\n"
					  "  del [--index <INDEX>] [--count <COUNT>]\n"
					  "    - Delete log from <INDEX>.\n",
		.pData		= NULL
	},
	{
		.pString	= "net",
		.function	= FTM_SHELL_CMD_net,
		.pShortHelp	= "Network management.",
		.pHelp		= "<cmd>\n"
					  "  Network settings.\n"
					  "COMMANDS:\n"
					  "  info\n"
					  "    - Show network.\n",
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
					  "  add <ID> <MODEL> [--ip <IP>] ...\n"
					  "    - Add switch\n"
					  "  del <ID>\n"
					  "    - Delete switch\n"
					  "  get <ID>\n"
					  "    - Show switch properties\n"
					  "  set <ID> [--ip <IP>] [--userid <USERID>] ...\n"
					  "    - Set switch properties(ip, userid, passwd, comment)\n"
					  "PARAMETERS:\n"
					  "  MODEL  Switch model\n"
					  "  ID     Switch Identifier\n"
					  "OPTIONS:\n"
					  "  --ip <IP>\n"
					  "    Switch IP Address\n"
					  "  --userid <USERID>\n"
					  "    User ID for switch management\n"
					  "  --passwd <PASSWD>\n"
					  "    Passwd for switch managment\n"
					  "  --comment <COMMENT>\n"
					  "    Comment\n"
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

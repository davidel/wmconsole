//    Copyright 2023 Davide Libenzi
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
// 


#include "wmc-includes.h"



static char const szHelp[] = "WmConsole supported commands:\n\n"
	"pwd                       = Prints the current directory\n"
	"cd DIR                    = Change current directory to DIR (.. is allowed)\n"
	"ls [DIR/FILE]             = Lists the current directory or the specified path\n"
	"ps                        = Lists currently running processes\n"
	"lsmod [MATCH]             = Lists modules loaded by processes whose name match MATCH\n"
	"wld MATCH                 = Lists all processes that load modules matching MATCH\n"
	"kill PID                  = Terminates the process identified by PID\n"
	"run EXE ...               = Starts the exe with the specified parameters\n"
	"exec EXE ...              = Starts the exe with the specified parameters and links to\n"
	"                            the process standard output/input. This differs from the 'run'\n"
	"                            command because it waits for the program termintation\n"
	"df [DIR]                  = Prints the disk space statistics for DIR (or object store\n"
	"                            in case DIR is not specified\n"
	"mem                       = Displays memory usage statistics\n"
	"lssvc                     = Lists device services\n"
	"svc -{S,T} SVC            = Controls services functionality. Supported actions are:\n"
	"                              S : Start service SVC\n"
	"                              T : Stop service SVC\n"
	"lsdev                     = Lists system devices\n"
	"mkdir [-p] DIR            = Create directory DIR. Allowed flags for the command are:\n"
	"                              p : Create all the intermediate directories, if missing\n"
	"rmdir DIR                 = Remove directory DIR (must be empty)\n"
	"rm FILE                   = Remove file FILE\n"
	"mv OLD NEW                = Renames files or directories from OLD to NEW\n"
	"cp SRC DST                = Copy SRC file onto DST file. If SRC is a directory, the whole\n"
	"                            SRC tree will be copied\n"
	"rmtree DIR                = Remove directory DIR and all its subtree (dangerous)\n"
	"chmod MODE FILE           = Change the access mode of FILE applying MODE changes\n"
	"                            ([+-][hws]+ | h=hidden w=write s=system)\n"
	"exit                      = Exit session\n"
	"shutdown                  = Exit session and shutdown the WmConsole server\n"
	"reboot                    = Exit session and reboot the device\n"
	"cat FILE                  = Dump the content of remote file FILE on screen\n"
	"find [-1s] PATH MATCH     = Recursively searches PATH for MATCH file or directory name\n"
	"                            (wildcards allowed in MATCH). Allowed flags for the command are:\n"
	"                              1 : Do not recurse inside subdirectories\n"
	"                              s : Print only the file path\n"
	"put [-Rf] REM LOC         = Copies the local file(s) LOC to the remote file/path (on device) REM.\n"
	"                            Wildcards allowed in LOC. Allowed flags for the command are:\n"
	"                              R : Recurse to subdirectories\n"
	"                              f : Forces the missing subdirectories creation on the device\n"
	"get [-R] REM LOC          = Copies the remote file(s) (on device) REM to local file/path LOC.\n"
	"                            Wildcards allowed in REM. Allowed flags for the command are:\n"
	"                              R : Recurse to subdirectories\n"
	"rpwd                      = Prints the current registry key\n"
	"rcd KEY                   = Change current registry key to KEY (.. is allowed)\n"
	"rls [KEY/RVAL]            = Lists the current registry key or the specified KEY\n"
	"rcat RVAL                 = Dump the content of registry path RVAL on screen\n"
	"rfind KEY MATCH           = Recursively searches KEY path for MATCH value or key name\n"
	"rrm RVAL                  = Remove the registry value RVAL\n"
	"rrmtree KEY               = Remove the registry key KEY and all its subtree (dangerous)\n"
	"rmkkey KEY                = Creates the registry key KEY\n"
	"rset RVAL TYPE VALUE      = Sets the registry value RVAL to VALUE, with data type TYPE.\n"
	"                            Allowed data types for the command are:\n"
	"                              SZ : String value\n"
	"                              DW : DWord value\n"
	"                              BI : Binary value (VALUE is a sequence of 2 hexdigit numbers,\n"
	"                                   comma separated, like: 03,c2,fa,...)\n"
	"rdig [-i] KEY TYPE DATA   = Recursively searches the KEY registry path for DATA bytes. Allowed\n"
	"                            data types for the command are:\n"
	"                              SZ : String value\n"
	"                              DW : DWord value\n"
	"                              BI : Binary value (VALUE is a sequence of 2 hexdigit numbers,\n"
	"                                   comma separated, like: 03,c2,fa,...)\n"
	"                            The DATA parameter format depends on the TYPE. Allowed\n"
	"                            flags for the command are:\n"
	"                              i : Ignore case during string match\n"
	"rexp [-R] KEY/RVAL FILE   = Exports the registry value RVAL or key KEY, to the file path FILE.\n"
	"                            Allowed flags for the command are:\n"
	"                              R : In case a registry key KEY is specified, exports the whole subtree\n"
	"rimp FILE                 = Imports the registry file FILE into the device registry. New keys\n"
	"                            will be created if missing, and existing values will be overwritten\n"
	"help                      = Prints this help screen\n";




int WmcCmd_help(WmcCtx *pCtx, char **ppszArgsA) {

	if (WmcChanPrintf(pCtx->pCh, WMC_CHAN_CTRL, "%s\n", szHelp) < 0 ||
	    WmcPktSend(pCtx->pCh, WMC_CHAN_CTRL, NULL, 0) < 0)
		return -1;

	return 0;
}


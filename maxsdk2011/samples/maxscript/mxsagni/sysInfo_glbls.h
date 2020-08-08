define_struct_global ("processAffinity",	"sysInfo",		getProcessAffinity,		setProcessAffinity);
define_struct_global ("systemAffinity",		"sysInfo",		getSystemAffinity,		setSystemAffinity);

define_struct_global ("DesktopSize",		"sysInfo",		getDesktopSize,			NULL);
define_struct_global ("DesktopBPP",			"sysInfo",		getDesktopBPP,			NULL);
define_struct_global ("MAXPriority",		"sysInfo",		getMAXPriority,			setMAXPriority);
define_struct_global ("username",			"sysInfo",		getusername,			NULL);
define_struct_global ("computername",		"sysInfo",		getcomputername,		NULL);
define_struct_global ("windowsdir",			"sysInfo",		getWinDirectory,		NULL);
define_struct_global ("systemdir",			"sysInfo",		getSystemDirectory,		NULL);
define_struct_global ("tempdir",			"sysInfo",		getTempDirectory,		NULL);
define_struct_global ("currentdir",			"sysInfo",		getCurrentDirectory,	setCurrentDirectory);
define_struct_global ("cpucount",			"sysInfo",		getCPUcount,			NULL);

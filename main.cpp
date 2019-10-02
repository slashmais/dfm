
#include "dfm.h"
#include <utilfuncs/utilfuncs.h>

bool Initialize()
{
	bool bret=false;
	if (GetAppHoMConfig(GetExeFilePath().ToStd(), DFMConf))
	{
		bret=DFMConf.Load();
		if (!DFMConf.haskey("dotdfm"))
		{
			std::string spath{homedir()};
			spath=path_append(spath, ".DFM");
			if (path_realize(spath))
			{
				DFMConf.setval("dotdfm", spath);
				spath=path_append(spath, "Dumpster");
				if (path_realize(spath)) { DFMConf.setval("dumpster", spath); }
				else { bret=tellerror("Cannot access '", spath, "'"); }
			}
			else { bret=tellerror("Cannot access '", spath, "'"); }
		}
	}
	else if (!bret) tellerror("Cannot configure dfm: ", HoM_Message());
	return bret;
}

void Terminate()
{
	DFMConf.Save();
}



GUI_APP_MAIN
{
	if (Initialize())
	{
		DFM dfm(CommandLine());
		dfm.Run();
		Terminate();
	}
	else telluser("Cannot initialize dfm");
}



#include "processtab.h"
#include <watcher/watcher.h>
#include <utilfuncs/utilfuncs.h>
#include "pictures.h"
#include <runner/runner.h>
#include "dfm.h"
#include <map>
#include <sys/types.h>
#include <signal.h>
#include <cerrno>
#include <cstring>

//---------------------------------------------------
ProcessTab::~ProcessTab()
{
	PL.clear();
}

ProcessTab::ProcessTab(const String &sname)
{
	clear();
	sName=sname;
	TG.UseTGHelpMenu(false);
	TG.WhenMenuBar = THISBACK(OnMenuBar);
}

void ProcessTab::Layout() { Add(TG.HSizePosZ().VSizePosZ()); TG.Layout(); }

void ProcessTab::OnTabClosing()
{
	watcher::StopPoll();
}

void ProcessTab::RefreshPane(bool bIcons) { if (!bIcons) ShowProcesses(); }

void ProcessTab::Setup(const String &sdata)
{
	TG.AddColumn("Process", 250).Sorting();
	TG.AddColumn("PID", 80).Align(ALIGN_CENTER).Sorting();
	TG.AddColumn("Command-line", 500).Sorting();
	TG.AddColumn("UID", 80).Sorting();
	TG.AddColumn("User", 100).Sorting();
	TG.ShowHeader(true);
	
	ResetProcesses(); //sdata);
	
	//watcher::StartPoll("/sys/block", [&](std::string s){ ResetDevices(s); });
	//watcher::StartPoll("/proc/mounts", [&](std::string s){ ResetDevices(s); });

	
}

//std::atomic<bool> busyatm{false};

void ProcessTab::ResetProcesses(std::string)
{
	PL.fillpl();
	ShowProcesses();
}

void ProcessTab::ShowProcesses()
{
	PNode F=TG.GetFocusNode();
	std::string sf{};
	if (F) sf=F->GetInfo();

	auto getprocpic=[&](const std::string &st)->Image
			{
				Image img;
				//if (sieqs(st, "sata-disk")) img=GetPic(PICDISK);
				//else if (sieqs(st, "usb-disk")) img=GetPic(PICUSB);
				//else if (iscd(st)) img=GetPic(PICCDDVD);
				//else img=GetPic(PICUNKNOWN);
				img=GetPic(PICEXECUTABLE);
				return img;
			};
	
	TG.ClearTree();
	for (auto& P:PL)
	{
		//Process &P=p.second;
		PNode N=TG.AddNode(0, getprocpic(P.cmdline), P.appname.c_str(), "",
										ttos<int>(P.pid).c_str(),
										P.cmdline.c_str(),
										ttos<int>(P.uid),
										P.owner.c_str());
		N->SetInfo(ttos<int>(P.pid));
	}
//	for (size_t i=0;i<TG.RootCount();i++) TG.Expand(TG.RootAt(i));
	F=(!sf.empty())?TG.GetInfoNode(sf.c_str()):TG.RootAt(0);
	TG.SetFocusNode(F);
	TG.RefreshTreeGrid();

}

void ProcessTab::OnMenuBar(Bar &bar)
{
	PNode N=TG.GetFocusNode();
	if (N)
	{
		int pid=stot<int>(N->GetInfo());
		std::string appname=N->CellAt(0).GetData();
		bool bCLI=(appname.find("//*cli-login-prompt")!=std::string::npos);
		bar.Add("kill", [this,pid,bCLI]{ OnKill(pid, bCLI); });
		bar.Separator();
		bar.Add("Refresh", [this]{ ResetProcesses(); });
		//bar.Separator();
		//bar.Add("Properties", [this,N]{ OnProperties(N); });
	//===============================
//		bar.Separator();
//		bar.Add("show watches", [](){ watcher::show_active_watches(); });
		
	//===============================
	}

}

void ProcessTab::OnKill(int pid, bool bcli)
{
	if (bcli&&!PromptYesNo("This will [* immediately kill your current session. ]"
							"*** &[* Nothing will be saved ] ***"
							"&You will be back at the login-prompt."
							"& &Do you [* REALLY ] want to kill everything?"))
		return;
	int r=kill(pid, SIGKILL);
	if (r<0) tellerror("Failed to kill process: ", std::strerror(errno));
	else ResetProcesses();
}

//void ProcessTab::OpenInWindow(PNode N)
//{
//	std::string sd=N->CellAt(0).GetData();
//	std::string smt=getmountpoint(sd);
//	pDFM->OpenInNewWindow(smt.c_str());
//}


void ProcessTab::TabMenu(Bar &bar) { OnMenuBar(bar); }

void ProcessTab::SetTools()
{
	if (pDFM)
	{
		pDFM->toolpane.WhenTool = [](int){};
		pDFM->ShowCurFocus("");
		pDFM->toolpane.Enable(ToolPane::TID_FIND);
		pDFM->toolpane.Enable(ToolPane::TID_DIFF);
		pDFM->toolpane.Enable(ToolPane::TID_PATH, false);
		pDFM->toolpane.Enable(ToolPane::TID_TOP, false);
		pDFM->toolpane.Enable(ToolPane::TID_BOT, false);
		pDFM->toolpane.Enable(ToolPane::TID_AZ, false);
	}
}

void ProcessTab::OnProperties(PNode N)
{
	
	to_do //what prop's?
	
}


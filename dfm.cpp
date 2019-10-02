
#include "dfm.h"
#include <watcher/watcher.h>
#include <utilfuncs/utilfuncs.h>
#include <runner/runner.h>
#include <uppfun/uppfun.h>
#include <thread>
#include "VersionRBT.h"


const std::string DFM_BACKUP_DIR{"/////nonesuchyet/////"};

AppConfig DFMConf;

//---------------------------------------------------------------------------------------------------
ToolPane::~ToolPane() {}

ToolPane::ToolPane()
{
	AddFrame(BlackFrame());
	ebPath.Clear();
	Add(btnFind.SetPic(GetPic(PICFIND)).SetSData(ttos(TID_FIND)).LeftPos(3,23).TopPos(3,23));
	Add(btnDiff.SetPic(GetPic(PICDIFF)).SetSData(ttos(TID_DIFF)).LeftPos(29,23).TopPos(3,23));
	Add(lblPath.SetLabel("Path:").LeftPos(60,35).TopPos(5));
	Add(ebPath.HSizePosZ(75, 64).TopPos(1));
	Add(btnTop.SetPic(GetPic(PICNODESTOP)).SetSData(ttos(TID_TOP)).RightPos(55,23).TopPos(3,23));
	Add(btnBot.SetPic(GetPic(PICNODESBOT)).SetSData(ttos(TID_BOT)).RightPos(29,23).TopPos(3,23));
	Add(btnAZ.SetPic(GetPic(PICNODESNONE)).SetSData(ttos(TID_AZ)).RightPos(3,23).TopPos(3,23));
	
	btnFind.WhenClick = THISFN(PBClicked);
	btnDiff.WhenClick = THISFN(PBClicked);
	ebPath.WhenEnter = [&]{ DoCall(TID_PATH); };
	btnTop.WhenClick = THISFN(PBClicked);
	btnBot.WhenClick = THISFN(PBClicked);
	btnAZ.WhenClick = THISFN(PBClicked);
}

void ToolPane::SetPath(std::string sp)	{ ebPath.SetData(sp.c_str()); }
std::string ToolPane::GetPath()			{ return ebPath.GetData().ToString().ToStd(); }
void ToolPane::Paint(Draw &drw)			{ drw.DrawRect(GetSize(), SColorFace()); }
void ToolPane::DoCall(int tid)			{ if ((tid>0)&&WhenTool) WhenTool(tid); }
void ToolPane::PBClicked(PicButton *p)	{ int tid=stot<int>(p->Data()); DoCall(tid); }
void ToolPane::Enable(int toolid, bool b)
{
	switch(toolid)
	{
		case TID_FIND: btnFind.Enable(b); break;
		case TID_DIFF: btnDiff.Enable(b); break;
		case TID_PATH: ebPath.Enable(b); break;
		case TID_TOP: btnTop.Enable(b); break;
		case TID_BOT: btnBot.Enable(b); break;
		case TID_AZ: btnAZ.Enable(b); break;
	}
}
void ToolPane::Select(int toolid, bool b)
{
	if (toolid==TID_TOP) { btnTop.Select(b); if (b) { btnBot.Select(false); btnAZ.Select(false); }}
	if (toolid==TID_BOT) { btnBot.Select(b); if (b) { btnTop.Select(false); btnAZ.Select(false); }}
	if (toolid==TID_AZ) { btnAZ.Select(b); if (b) { btnTop.Select(false); btnBot.Select(false); }}
}


//---------------------------------------------------------------------------------------------------
bool b_stop_scan_mapps{false};
bool b_scan_mapps_busy{false};
void scan_mapps(DFM *pdfm, bool bShowProgress=true)
{
	b_scan_mapps_busy=true;
	pdfm->ShowStatus("scanning icons..");
	if (bShowProgress) pdfm->Sync();
	pdfm->mime.Scan(Mime::SCAN_ALL, pdfm, bShowProgress);
	if (!b_stop_scan_mapps)
	{
		pdfm->RefreshAllTabIcons();
		pdfm->ShowStatus("scanning done");
	}
	b_scan_mapps_busy=false;
}

DFM::~DFM()
{
//aaa
	watcher::StopAllWatches();
	Tabs.RemoveAllTabs();
}

void DFM::init_dfm()
{
	Icon(GetPic(PICDFMICON));
	std::string stitle=spf("DFM ", V_VERSION, " on ", hostname());
	Title(stitle.c_str());
	SetRect(0,0,1250,800);
	Sizeable();
	int Y=0, H=31;

//	{
//		x; //bug-check
//		std:: string s{"History\n"};
//		for (auto p:VersionHistory<>) { s+="    ["; s+=p.first; s+="] -> "; s+=p.second; s+="\n"; }
//		PromptOK(DeQtf(s.c_str()));
//		std:: string s{};
//		for (auto p:VersionHistory<>) s+=spf("    [", p.first, "] -> ", p.second, "\n");
//		telluser("History\n", s, "\n");
//	}

	WaitCursor WC; WC.Show();

	AddFrame(mainmenu);
	mainmenu.Set([=](Bar& bar) { MakeMenu(bar); });
	
	AddFrame(statusbar);
	if (has_root_access()) { Add(urootwarn.HSizePosZ().TopPos(Y, 15)); Y+=15; }

	toolpane.pDFM=this;
	Add(toolpane.HSizePosZ().TopPos(Y, H));
	Y+=H-4;
	
	Tabs.pDFM=this;
	Add(Tabs.HSizePosZ().VSizePosZ(Y));
	Tabs.WhenSet = THISFN(OnTab);

	create_dir_tabs();

	std::thread(scan_mapps, this, false).detach();

}

DFM::DFM() { init_dfm(); }

DFM::DFM(const Vector<String> &cmdln)
{
	init_dfm();
	for (int i=0;i<cmdln.size();i++) { String SD=cmdln[i]; if (dir_exist(SD.ToStd())) { Tabs.AddTab<DirTab>(SD, SD); }}
}

void DFM::Close()
{
	if (b_scan_mapps_busy)
	{
		Size sz=GetSize();
		Add(BPBusy.LeftPos((sz.cx-250)/2, 250).TopPos((sz.cy-50)/2, 50));
		Sync();
		b_stop_scan_mapps=true;
		while (b_scan_mapps_busy) kipm(50);
	}
	Ctrl::Close();
}

void DFM::create_dir_tabs()
{
//aaa - check if removing tab is chaining to stopwatch(..)
	watcher::StopAllWatches();
	Tabs.RemoveAllTabs();
	Tabs.AddTab<DeviceTab>("Devices");
	Tabs.AddTab<ProcessTab>("Processes");
	Tabs.AddTab<RootTab>("Root", "/");
	
	show_pinned_tabs();

}

void DFM::show_pinned_tabs()
{
	std::string sp=DFMConf.getval("pinned");
	std::vector<std::string> vs;
	splitslist(sp, ':', vs, false);
	for (auto s:vs) Tabs.AddTab<DirTab>(s.c_str(), s.c_str());
}

void DFM::RemoveTab(const String &sname)						{ Tabs.RemoveTab(sname); }
void DFM::RenameTab(const String &sold, const String &snew)		{ Tabs.RenameTab(sold, snew); }
void DFM::RefreshTabIcons(TabType *ptab)						{ ptab->RefreshPane(true); }
void DFM::RefreshAllTabIcons()									{ for (auto& p:Tabs.TabPanes) { RefreshTabIcons(p.second); }}
bool DFM::IsTab(const String &sname)							{ return Tabs.HasTab(sname); }
void DFM::ShowStatus(const String &S)							{ statusbar=S; }
void DFM::ClearStatus()											{ statusbar="Ready"; }
void DFM::OnTab()												{ TabType *p=Tabs.FocusTab(); if (p) { p->SetTools(); }}
void DFM::ShowCurFocus(const String &S)							{ toolpane.SetPath(S.ToStd()); }
void DFM::ClearCurFocus()										{ toolpane.SetPath(""); }

void DFM::MakeMenu(Bar &bar)
{
	bar.Sub("Meta", [=](Bar& bar) { MetaMenu(bar); });
	bar.Sub("Context", [=](Bar& bar) { ContextMenu(bar); });
	bar.Sub("Tools", [=](Bar& bar) { ToolsMenu(bar); });
	bar.Sub("Help", [=](Bar& bar) { HelpMenu(bar); });
}

void DFM::MetaMenu(Bar &bar)
{
	bar.Add("Preferences..", [=](){ telluser("to do: (behaviour)"); }); //dir-order, init-tabs to open, app-assoc's,
	bar.Add("Customize..", [=](){ telluser("to do: (appearance)"); }); //?
	bar.Separator();
	bar.Sub("Rescan Application-associations", [&](Bar& bar) { AppAssocsMenu(bar); });
	bar.Separator();
	bar.Add("Exit", [=](){ Close(); });
}

void DFM::ContextMenu(Bar &bar)
{
	TabType *ptab=Tabs.FocusTab();
	ptab->TabMenu(bar);
}

void DFM::ToolsMenu(Bar &bar)
{
	std::string s, sdev="/seleced/device";
	s=spf("Inspect ", sdev);
	bar.Add(!sdev.empty(), s.c_str(), [=](){ telluser("to do (SMART?/..)"); });
	bar.Separator();
	bar.Add("Partitioning..", [=](){ telluser("to do: call gparted..?.."); }); //runner::SudoRun("/usr/sbin/gparted"); }); doesn't work
	bar.Separator();
	bar.Add("Recovery..", [=](){ telluser("to do: call testdisk/photorec..?"); });
}

void DFM::HelpMenu(Bar &bar)
{
	bar.Add("Topics", [=](){ telluser("to do"); });
	bar.Add("Find ..", [=](){ telluser("to do (keyword-search)"); });
	bar.Separator();
	bar.Add("About", [=](){ telluser("to do"); });
}

void DFM::AppAssocsMenu(Bar &bar)
{
	bar.Add("Scan All", [&](){ mime.Scan(Mime::SCAN_ALL, this); });
	bar.Add("Scan System", [&](){ mime.Scan(Mime::SCAN_SYSTEM, this); });
	bar.Add("Scan Local", [&](){ mime.Scan(Mime::SCAN_USER, this); });
	bar.Add("Scan Custom", [&](){ mime.Scan(Mime::SCAN_CUSTOM, this); });
	bar.Separator();
	bar.Add("Custom associations..", [&](){ CustomAssocs(); });
	//String S=SelectDirectory(); if (!S.IsEmpty()) { Mapps.pDFM=this; Mapps.CustomDesktops=S.ToStd(); Mapps.Scan(MimeApps::SCAN_CUSTOM); }});
}

void DFM::OpenInNewTab(const String &sname, const String &sdata)
{
	Tabs.AddTab<DirTab>(sname, sdata);
}

bool DFM::OpenInNewWindow(const String &SDir)
{
	String SApp=GetExeFilePath();
	runner::Arguments A;
	runner::Environment E;
	A.add(SDir.ToStd());
	return runner::Run(SApp.ToStd(), A, E);
}

bool DFM::BackupToDumpster(std::string sfd)
{
	bool b=false;
	std::string dumpster=DFMConf.getval("dumpster");
	if (dir_exist(dumpster))
	{
		if (issubdir(sfd, dumpster)) { return false; }
		//if (issubdir(sfd, dumpster)) { return tellerror("You must use Erase to delete items in dumpster"); }
		if (isdir(sfd)) b=dir_copy(sfd, dumpster);
		else if (isfile(sfd)) b=file_copy(sfd, dumpster);
	}
	//if (!b) tellerror("'", sfd, "' not backed-up to dumpster");
	return b;
}

void DFM::CustomAssocs()
{
//[Desktop Entry]
//Name=NNNNN
//Comment=CCCCC
//Exec=XXXXX %f
//Icon=IIIII ...copied from XXXXX if exist else cut
//Type=Application
//MimeType=...copied from XXXXX's if exist and adding?/prepending? assoc's...

	to_do

}
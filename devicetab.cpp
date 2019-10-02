
#include "devicetab.h"
#include <watcher/watcher.h>
#include <utilfuncs/utilfuncs.h>
#include "pictures.h"
#include <runner/runner.h>
#include "dfm.h"
#include <atomic>
#include "cd_oc.h"
#include <thread>
#include <map>

//---------------------------------------------------
DeviceTab::~DeviceTab()
{
	DL.clear();
}

DeviceTab::DeviceTab(const String &sname)
{
	clear();
	sName=sname;
	TG.UseTGHelpMenu(false);
	TG.WhenMenuBar = THISBACK(OnMenuBar);
}

void DeviceTab::Layout() { Add(TG.HSizePosZ().VSizePosZ()); TG.Layout(); }

void DeviceTab::OnTabClosing()
{
	watcher::StopPoll();
}

void DeviceTab::RefreshPane(bool bIcons) { if (!bIcons) ShowDevices(); }

void DeviceTab::Setup(const String &sdata)
{
	TG.AddColumn("Device", 180);
	TG.AddColumn("Identity", 300);
	TG.AddColumn("Label", 150);
	TG.AddColumn("Type", 100);
	TG.AddColumn("Mount", 150);
	TG.AddColumn("Size", 100);
	TG.AddColumn("Used", 100);
	TG.AddColumn("Free", 100);
	TG.ShowHeader(true);
	
	ResetDevices(); //sdata);
	
	watcher::StartPoll("/sys/block", [&](std::string s){ ResetDevices(s); });
	watcher::StartPoll("/proc/mounts", [&](std::string s){ ResetDevices(s); });

	//battle to get cd-change data...
	//watcher::StartPoll("/dev/disk/by-label", [&](std::string s){ ResetDevices(s); });
	watcher::StartPoll("/dev/disk/by-uuid", [&](std::string s){ ResetDevices(s); });
	
}

std::atomic<bool> busyatm{false};

void DeviceTab::ResetDevices(std::string)
{
	getdiskdata(DL);
	ShowDevices();
}

void DeviceTab::ShowDevices()
{
	PNode F=TG.GetFocusNode();
	std::string sf{};
	if (F) sf=F->GetInfo();
	if (std::atomic_exchange(&busyatm, true)) return;
	auto iscd=[&](std::string s)->bool{ std::string sl{s}; tolcase(sl); return (sl.find("cdrom")!=std::string::npos); };
	auto getdevpic=[&](const std::string &st)->Image
			{
				Image img;
				if (sieqs(st, "sata-disk")) img=GetPic(PICDISK);
				else if (sieqs(st, "usb-disk")) img=GetPic(PICUSB);
				else if (iscd(st)) img=GetPic(PICCDDVD);
				else img=GetPic(PICUNKNOWN);
				return img;
			};
	
	TG.ClearTree();
	for (auto& d:DL)
	{
		Device &D=d.second;
		PNode N=TG.AddNode(0, getdevpic(D.stype), D.sname.c_str(), "",
								D.sident.c_str(), /*D.slabel.c_str()*/ "", D.stype.c_str(),
								/*D.smount.c_str()*/ "", D.stotal.c_str(), /*D.sused.c_str()*/ "",
								/*D.sfree.c_str()*/ "")->Expandable();
		N->SetInfo(D.sname);
		for (auto& p:D.partitions)
		{
			auto& pp=p.second;
			PNode P=TG.AddNode(N, (sieqs(pp.stype, "swap")?GetPic(PICSWAPPARTITION):(pp.smount.empty())?GetPic(PICLOCKED):GetPic(PICPARTITION)),
							  pp.sname.c_str(), "", pp.sident.c_str(), pp.slabel.c_str(),
							  pp.stype.c_str(), pp.smount.c_str(), pp.stotal.c_str(),
							  pp.sused.c_str(), pp.sfree.c_str());
			P->SetInfo(pp.sname);
		}
	}
	std::atomic_exchange(&busyatm, false);
	for (size_t i=0;i<TG.RootCount();i++) TG.Expand(TG.RootAt(i));
	F=(!sf.empty())?TG.GetInfoNode(sf.c_str()):TG.RootAt(0);
	TG.SetFocusNode(F);
	TG.RefreshTreeGrid();
}

void DeviceTab::OnMenuBar(Bar &bar)
{
	PNode N=TG.GetFocusNode();
	if (N)
	{
		std::string sd=N->CellAt(0).GetData();
		std::string sm=getmountpoint(sd);
		bool b=(N->Parent()!=0);
		bool bm=!sm.empty();
		bar.Add(b&&!bm, "mount", [this,N]{ OnMount(N); });
		bar.Add(b&&bm, "un-mount", [this,N]{ OnUnmount(N); });
		bar.Separator();
		bar.Add(b&&bm, "Open in tab", [this,N]{ OpenInTab(N); });
		bar.Add(b&&bm, "Open in window", [this,N]{ OpenInWindow(N); });
		if (IsCDDrive(sd))
		{
			int cdt=CDTrayState(sd);
			bar.Separator();
			bar.Add(((cdt==CD_OPEN)||(cdt==CD_NO_INFO)), "Close Tray", [this,sd]{ CloseCDTray(sd); });
			bar.Add(((cdt==CD_CLOSED)||(cdt==CD_NO_INFO)), "Open Tray", [this,sd]{ OpenCDTray(sd); });
		}
		bar.Separator();
		bar.Add("Refresh", [this]{ CheckReset(); });
		bar.Separator();
		//bar.Add("Properties", [this,N]{ OnProperties(N); });
	//===============================
//		bar.Separator();
//		bar.Add("show watches", [](){ watcher::show_active_watches(); });
		
	//===============================
	}
}

void DeviceTab::OnMount(PNode N)
{
	if (N->Parent()==0) { telluser("Cannot mount raw device ", N->CellAt(0).GetData()); return; }
	std::string sd=N->CellAt(0).GetData();
	std::string st=N->CellAt(3).GetData();
	std::string smt=getmountpoint(sd);
	if (!smt.empty()) { telluser(sd, " already mounted at ", smt); return; }
	if (st.empty()) { telluser("Cannot mount unknown filesystem type.\nDo 'Refresh' and try again."); return ; }
	int n=0; do { smt=spf("/media/dfm", ++n); if (!ismountpoint(smt)) break; } while (dir_exist(smt));
	std::string sc("");
	sc=spf("mkdir ", smt, " ; sudo mount -t ", st, " -o defaults,noatime,nodiratime ", sd, " ", smt);
	bool b;
	if (!(b=runner::SudoRun(sc))) telluser("mount failed: ", runner::get_run_error());
	Sync();
	if (b) { int tries=10; while ((tries-->0)&&!ismountpoint(smt)) kipm(500); }
	if (ismountpoint(smt)) { N->CellAt(4).SetData(smt); TG.RefreshTreeGrid(); }
}

void DeviceTab::OnUnmount(PNode N)
{
	std::string sdev=N->CellAt(0).GetData();
	std::string sm=getmountpoint(sdev);
	if (sm.empty()) { telluser(sdev, " is not mounted"); return; }
	std::string sc;
	sc=spf("umount ", sm, " ; if [ \"`lsblk -pno NAME,MOUNTPOINT | grep '", sm, "'`\" == \"\" ] ; then sudo rm -r ", sm, " ; fi");
	if (!runner::SudoRun(sc)) telluser("umount failed: ", runner::get_run_error());
	Sync();
	kipm(2000);
	if (ismountpoint(sm)) telluser("cannot umount ", sdev);
	else { pDFM->RemoveTab(sm); N->CellAt(4).SetData(""); TG.RefreshTreeGrid(); }
}

void DeviceTab::OpenInTab(PNode N)
{
	std::string sd=N->CellAt(0).GetData();
	std::string smt=getmountpoint(sd);
	pDFM->OpenInNewTab(smt.c_str(), smt.c_str());
}

void DeviceTab::OpenInWindow(PNode N)
{
	std::string sd=N->CellAt(0).GetData();
	std::string smt=getmountpoint(sd);
	pDFM->OpenInNewWindow(smt.c_str());
}


void DeviceTab::CheckReset()
{
	Devices devs;
	getdiskdata(devs);
	if (DL==devs) return; //don't update GUI needlessly
	DL=devs;
	ShowDevices();
}

void DeviceTab::CloseCDTray(std::string sd) { CDClose(sd); }

void DeviceTab::OpenCDTray(std::string sd) { CDOpen(sd); }

void DeviceTab::TabMenu(Bar &bar) { OnMenuBar(bar); }

void DeviceTab::SetTools()
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

void DeviceTab::OnProperties(PNode N)
{
	
	to_do //what prop's?
	
}


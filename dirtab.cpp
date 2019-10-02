
#include <HoM/HoM.h>
#include "dirtab.h"
#include "dfm.h"
#include "pictures.h"
#include <runner/runner.h>
//#include "mimeappico.h"
#include <uppfun/uppfun.h>
#include <mutex>
#include "finddlg.h"
#include <filedirdiff/filedirdiff.h>
#include "dirprops.h"
#include "secdel.h"
#include <fstream>

//==================================================================================================
std::mutex MUX_DIRTAB; //prevent clashes between this and watcher..
enum //timeouts
{
	TO_UPDATECB=17,
	TO_DOEXPAND=23,
	TO_DOCONTRACT=31,
	TO_MENUPASTE=37,
	TO_MENUMOVE=43,
	TO_MENUNEWDIR=13,
	TO_MENUNEWTEXTFILE=11,
	TO_MENUDELETE=59,
};

#define KIPP(n) kipu(n)

//==================================================================================================

int get_type_pic(const std::string &se, FSYS::file_type ft)
{
	switch(ft)
	{
		case FSYS::file_type::directory:	return PICDIR;
		case FSYS::file_type::regular:		return (canexecute(se))?PICFILERUN:PICFILE; //todo: get_filetype_pic(se); todo todo todo
		case FSYS::file_type::fifo:			return PICPIPE;
		case FSYS::file_type::symlink:
			{
				std::string sfe=getsymlinktarget(se); //FSYS::read_symlink(se);
				if (!fsexist(sfe)) sfe=path_append(path_path(se), sfe);
				FSYS::file_type	t=getfsfiletype(sfe);
				if (isdirtype(t)) return PICLINKDIR;
				else if (isfiletype(t)) return PICLINKFILE;
				else return PICLINK;
			}
		case FSYS::file_type::block:		return PICBLOCKDEVICE;
		case FSYS::file_type::character:	return PICCHARDEVICE;
		case FSYS::file_type::socket:		return PICSOCKET;
		case FSYS::file_type::none:
		case FSYS::file_type::not_found:
		case FSYS::file_type::unknown:			return PICUNKNOWN;
	}
	return PICUNKNOWN;
}

bool getFSMETA(const std::string &se, FSMETA &fsmeta)
{
	fsmeta.clear();
	if (fsexist(se))
	{
		FSYS::file_type	t=getfsfiletype(se);
		fsmeta.item=path_name(se);
		fsmeta.type=(int)t;
		fsmeta.bdir=isdirtype(t);
		fsmeta.rights=getfullrights(se); //getrightsRWX(se);
		fsmeta.dtmodified=path_time_h(se);
		fsmeta.owner=get_owner(se);
		fsmeta.size=fssize(se);
		fsmeta.picid=get_type_pic(se, t);
		return true;
	}
	return false;
}

bool FillVFSMETA(const std::string &sdir, VFSMETA &vfsmeta)
{
	DirEntries de{};
	std::string se{};
	vfsmeta.clear();
	dir_read(sdir, de);
	for (auto p:de)
	{
		FSMETA content{};
		se=path_append(sdir, p.first);
		if (getFSMETA(se, content)) vfsmeta.push_back(content);
	}
	return (vfsmeta.size()>0);
}


//==================================================================================================
//const std::string GetOpenWith(const std::string &sf, const Mime::AppPath &ap)
const std::string GetOpenWith(const std::string &sf, const Apps &ap) //const Mime::AppPath &ap)
{
	struct DlgOpenWith : public TopWindow
	{
		typedef DlgOpenWith CLASSNAME;
	
		//Mime::AppPath apps{};
		Apps apps{};
		ArrayCtrl a;
		Label o, u;
		EditString e, p;
		Button c, k, x;
		DocEdit d;
		Option chkDefault;
		bool b;
	
//		DlgOpenWith(const Mime::AppPath &mal)
		DlgOpenWith(const Apps &mal)
		{
			apps=mal;
			Title("Open with..");
			SetRect(0, 0, 296, 312);
			Sizeable();
			b=false;
			a.Clear();
			a.NoHeader().NoGrid();
			a.AddColumn();
			for (auto pp:apps) { a.Add(pp.first.c_str()); }
			a.WhenSel << [&]{ e.Clear(); p.SetData(apps[a.Get(0).ToString().ToStd()].papp.c_str()); };
			e.WhenAction << [&]{ p.Clear(); std::string s=e.GetData().ToString().ToStd(); if (!(s=runner::getwhich(s)).empty()) p.SetData(s.c_str()); };
			a.WhenLeftDouble = THISFN(ok);
			k.WhenAction = THISFN(ok);
			c.WhenAction = THISFN(quit);
			x.WhenAction = THISFN(browse);

			Add(a.HSizePosZ(8, 8).VSizePosZ(28, 88));
			Add(o.SetLabel(t_("Open with:")).LeftPosZ(8, 148).TopPosZ(8, 19));
			Add(u.SetLabel(t_("Use:")).SetAlign(ALIGN_RIGHT).LeftPosZ(8, 24).BottomPosZ(61, 19));
			Add(e.HSizePosZ(32, 32).BottomPosZ(61, 19));
			Add(c.SetLabel(t_("Cancel")).RightPosZ(8, 60).BottomPosZ(8, 20));
			Add(k.SetLabel(t_("OK")).RightPosZ(84, 60).BottomPosZ(8, 20));
			Add(x.SetLabel(t_("...")).RightPosZ(8, 20).BottomPosZ(61, 19));
			Add(d.HSizePosZ(8, 8).VSizePosZ(8, 88));
			Add(p.SetEditable(false).HSizePosZ(8, 8).BottomPosZ(37, 19));
			Add(chkDefault.SetLabel(t_("Use as Default")).LeftPosZ(8, 100).TopPosZ(284, 16));
			
			if (apps.empty()) { a.Hide(); d.Show(); } else { a.Show(); d.Hide(); }
			
		}
		virtual ~DlgOpenWith() {}
		void ok() { std::string s=e.GetData().ToString().ToStd(); b=true; Close(); }
		void quit() { b=false; Close(); }
		bool Key(dword key, int) { if (key==K_ENTER) { ok(); return true; } else if (key==K_ESCAPE) { quit(); return true; } return false; }
		void browse() { String S=SelectFileOpen("All\t*"); std::string s=S.ToStd(); if (isexe(s)) { e.SetData(s.c_str()); }}
	};

	DlgOpenWith dlg(ap);
	std::string sexe("");
	dlg.Execute();
	if (dlg.b) sexe=dlg.p.GetData().ToString().ToStd();
	return sexe;
}

//==================================================================================================

DirectoryTab::~DirectoryTab()
{
	watcher::StopWatches(sName.ToStd());
//	if (pDFM) pDFM->toolpane.WhenTool = [](int){return;};
//	TG.Clear();
}

DirectoryTab::DirectoryTab(const String &sname)
{
	sName=sname;
	TG.UseTGHelpMenu(false);
	TG.WhenFocus = THISBACK(OnWhenFocus);
}

void DirectoryTab::Layout()
{
	Add(TG.HSizePosZ().VSizePosZ());
	TG.ListExpandingNodes(EXD_FIRST);
	TG.Layout();
	
	TG.WhenBeforeNodeExpand = THISBACK(OnDoExpand);
	TG.WhenAfterNodeContract = THISBACK(OnDoContract);
	
	TG.WhenMenuBar = THISBACK(OnMenuBar);
	
}

void DirectoryTab::OnTabClosing()
{
	watcher::StopWatches(sName.ToStd());
	if (pDFM) pDFM->toolpane.WhenTool = [](int){return;};
}


bool DirectoryTab::DoWatchUpdateCB(std::string sk, watcher::FSACTION act, std::string sdf, std::string sdt)
{
	//caveat: using logger(..) or similar to do output here to a file in a monitored dir may(will) cause infinite tail-chasing
	//try not to update anything in a monitored dir from this function - just reflect what is there
	bool bgotmux=false;
	int tries=10;
	while ((tries>0)&&!bgotmux) { KIPP(TO_UPDATECB); tries--; bgotmux=MUX_DIRTAB.try_lock(); }
	if (bgotmux)
	{
		try
		{
			PNode WatchNode=TG.GetNode(sk);
			if (!WatchNode) { MUX_DIRTAB.unlock(); return true; }
			PNode P=nullptr;
			std::string swn=WatchNode->GetInfo();
			if (seqs(swn, sdf)) P=WatchNode; else P=WatchNode->GetInfoNode(sdf);
			if (act==watcher::FS_UPDATED)
			{
				if (P)
				{
					std::string se=P->GetInfo();
					FSMETA fsmeta;
					if (getFSMETA(se, fsmeta))
					{
						P->CellAt(0).SetData(fsmeta.item.c_str());
						P->CellAt(1).SetData(to_sKMGT(fsmeta.size).c_str());
						P->CellAt(2).SetData(gettypename(((FSYS::file_type)fsmeta.type)).c_str());
						P->CellAt(3).SetData(fsmeta.rights.c_str());
						P->CellAt(4).SetData(fsmeta.owner.c_str());
						P->CellAt(5).SetData(fsmeta.dtmodified.c_str());
						P->SetPic(GetPic(fsmeta.picid));
					}
				}
			}
			else if (act==watcher::FS_RENAMED)
			{
				if (P&&!sdt.empty())
				{
					if (pDFM&&pDFM->IsTab(sdf)) { MUX_DIRTAB.unlock(); pDFM->RenameTab(sdf, sdt); MUX_DIRTAB.lock(); }
					P->SetInfo(sdt);
					P->CellAt(0).SetData(path_name(sdt).c_str());
				}
			}
			else
			{
				if (act==watcher::FS_CREATED)
				{
					P=WatchNode;
					//PNode F=TG.GetFocusNode();
					if (P&&!sdf.empty())
					{
						FSMETA fsmeta;
						if (getFSMETA(sdf, fsmeta))
						{
							std::string sd, st, tn=gettypename(((FSYS::file_type)fsmeta.type));
							if (fsmeta.bdir)
							{
								sd=path_append(path_path(sdf), fsmeta.item);
								PNode T=TG.AddNode(P, GetPic(fsmeta.picid), fsmeta.item.c_str(), "", to_sKMGT(fsmeta.size).c_str(), tn.c_str(), fsmeta.rights.c_str(), fsmeta.owner.c_str(), fsmeta.dtmodified.c_str())->Expandable();
								T->SetInfo(sd);
								TG.AddNode(T, "<place-holder>"); //filled & watched when expanded
							}
							else
							{
								st=fsmeta.item;
								sd=path_append(path_path(sdf), fsmeta.item);
								if (((FSYS::file_type)fsmeta.type)==FSYS::file_type::symlink) { st+=" -> "; st+=FSYS::read_symlink(sdf); }
								Image ico;
								//if (isfile(sd)) ico=get_item_icon(sd); else ico=GetPic(fsmeta.picid);
								ico=GetFDPic(sd, pDFM, true, false);
								PNode E=TG.AddNode(P, ico, st.c_str(), "", to_sKMGT(fsmeta.size).c_str(), tn.c_str(), fsmeta.rights.c_str(), fsmeta.owner.c_str(), fsmeta.dtmodified.c_str());
								E->SetInfo(sd.c_str());
							}
						}
						//TG.SetFocusNode(F, false);
					}
				}
				else if ((act==watcher::FS_DELETED)||(act==watcher::FS_REMOVED))
				{
					if (P)
					{
						if (pDFM&&pDFM->IsTab(P->GetInfo())) { MUX_DIRTAB.unlock(); pDFM->RemoveTab(P->GetInfo()); MUX_DIRTAB.lock(); }
						watcher::StopWatch(sName.ToStd(), P->GetKey());
						TG.DeleteNode(P, (act==watcher::FS_DELETED));
					}
				}
				TG.RefreshTreeGrid();
			}
		} catch(std::exception &e)
		{
			MUX_DIRTAB.unlock();
			telluser("Exception in DoWatchUpdateCB(", sk, ", ", watcher::act_name(act), ", ", sdf, ", ", sdt, ")\n", e.what(), "\n");
			return true;
		}
		MUX_DIRTAB.unlock();
		return true;
	}
	return false;
}

void DirectoryTab::RefreshPane(bool bIcons)
{
	if (!bIcons) { Setup(sData); ShowDir(); }
	else
	{
		size_t i=0, n=TG.RootCount();
		while (i<n)
		{
			PNode pn=TG.RootAt(i);
			while ((pn=TG.GetNextNode(pn)))
			{
				if (isfile(pn->GetInfo()))
				{
					Image ico=GetFDPic(pn->GetInfo(), pDFM, true, false);//true); //get_item_icon(pn->GetInfo());
					pn->SetPic(ico);
				}
			}
			i++;
		}
		TG.RefreshTreeGrid();
	}
}

void DirectoryTab::Paint(Draw &)//drw) pedantic
{
	//nothing to do - treegrid covers all
}

void DirectoryTab::Setup(const String &sdir)
{
	sData=sdir;
	TG.AddColumn("Item", 550).Sorting();
	TG.AddColumn("Size", 100).Align(ALIGN_RIGHT).Sorting();
	TG.AddColumn("Type", 150).Align(ALIGN_CENTER).Sorting();
	TG.AddColumn("Rights", 100).Align(ALIGN_CENTER);
	TG.AddColumn("Owner", 100).Align(ALIGN_CENTER).Sorting();
	TG.AddColumn("Modified", 150).Align(ALIGN_LEFT).Sorting();
	TG.ShowHeader(true);
	
	ShowDir();
	
}

void DirectoryTab::ShowDir()
{
	TG.ClearTree();
	std::string d=Data().ToStd();
	if (!d.empty())
	{
		PNode D=TG.AddNode(0, GetPic(PICDIR), d, "", to_sKMGT(dir_size(d)).c_str(), "directory", getfullrights(d).c_str(), get_owner(d).c_str(), path_time_h(d).c_str())->Expandable();
		D->SetInfo(d);
		TG.AddNode(D, "<place-holder>");
		//TG.LockNode(D); //default parameter: bExpand=true (callback invokes OnDoExpand(..))
		TG.Expand(D);
	}
	
	TG.ListExpandingNodes(EXD_FIRST);
	if (pDFM) pDFM->toolpane.Select(ToolPane::TID_TOP);
	
	TG.SetFocusNode(TG.RootAt(0));
	if (pDFM) pDFM->toolpane.WhenTool = THISFN(ToolCall);
}

void DirectoryTab::OnDoExpand(PNode N)
{
	if (!N) return;
	bool bgotmux=false;
	int tries=10;
	while ((tries>0)&&!bgotmux) { KIPP(TO_DOEXPAND); tries--; bgotmux=MUX_DIRTAB.try_lock(); }
	if (bgotmux)
	{
		WaitCursor WC; WC.Show();
		try
		{
			VFSMETA v;
			std::string sd, st, sdir=N->GetInfo();//.ToStd();
			N->ClearNodes();
						
			if (FillVFSMETA(sdir, v))
			{
				for (auto e:v)
				{
					std::string tn=gettypename(((FSYS::file_type)e.type));
					if (e.bdir)
					{
						sd=path_append(sdir, e.item);
						PNode T=TG.AddNode(N, GetPic(e.picid), e.item.c_str(), "", to_sKMGT(e.size).c_str(), tn.c_str(), e.rights.c_str(), e.owner.c_str(), e.dtmodified.c_str())->Expandable();
						T->SetInfo(sd);
						TG.AddNode(T, "<place-holder>");
					}
					else
					{
						st=e.item;
						sd=path_append(sdir, e.item);
						if (((FSYS::file_type)e.type)==FSYS::file_type::symlink) { st+=" -> "; st+=getsymlinktarget(sd); }
						Image ico=GetFDPic(sd, pDFM, true, false); //(isfile(sd))?get_item_icon(sd):GetPic(e.picid);
						PNode E=TG.AddNode(N, ico, st.c_str(), "", to_sKMGT(e.size).c_str(), tn.c_str(), e.rights.c_str(), e.owner.c_str(), e.dtmodified.c_str());
						E->SetInfo(sd.c_str());
					}
				}
			}
			else TG.AddNode(N, "<empty>");
			watcher::StartWatch(sName.ToStd(), N->GetKey(), N->GetInfo(), [this](std::string k, watcher::FSACTION a, std::string s, std::string t)->bool{ return DoWatchUpdateCB(k, a, s, t); });
		} catch(...){}
		MUX_DIRTAB.unlock();
	}
}

void DirectoryTab::OnDoContract(PNode N)
{
	if (!N) return;
	bool bgotmux=false;
	int tries=10;
	while ((tries>0)&&!bgotmux) { KIPP(TO_DOCONTRACT); tries--; bgotmux=MUX_DIRTAB.try_lock(); }
	if (bgotmux)
	{
		try
		{
			watcher::StopWatch(sName.ToStd(), N->GetKey());
			N->ClearNodes();
			TG.AddNode(N, "<place-holder>");
		} catch(...){}
		MUX_DIRTAB.unlock();
	}
}

void DirectoryTab::OnMenuBar(Bar &bar)
{
	PNode N=TG.GetFocusNode();

	if (pDFM) pDFM->ClearStatus();
	
	if (N&&N->IsLocked()&&N->Parent()) { bar.Add("Open", THISBACK1(OnMenuOpen,N)).Key(K_CTRL|K_O); return; } //???

	if (N)
	{
		std::string s, sF;
		//bar.Clear();
		
		sF=N->GetInfo();
		
		bool bIsRootNode=!N->Parent();
		bool bCanR=canread(sF);
		bool bCanW=canwrite(sF);
		bool bCanX=canexecute(sF);
		bool bIsDir=isdir(sF);
		bool bIsFile=isfile(sF);

		if (bCanX)
		{
			if (bIsFile) bar.Add("Run", [&, N]{ OnMenuRun(N); }).Key(K_CTRL|K_R);
			else if (bIsDir)
			{
				if (!bIsRootNode)
				{
					bar.Add("Open", [&, N]{ OnMenuOpen(N); }).Key(K_CTRL|K_O);
					bar.Add("Open in new Tab", [&, N]{ OnMenuOpenTab(N);}).Key(K_CTRL|K_T);
				}
				bar.Add("Open in new Window", [&, N]{ OnMenuOpenWindow(N);}).Key(K_CTRL|K_W);
			}
			bar.Separator();
		}

		if (bCanR||bCanW)
		{
			if (bIsFile)
			{
				bar.Add("Open..", [&]{ OnMenuOpenFile();}).Key(K_CTRL|K_O);
				//bar.Add("Open with ..", .....(OnOpenWith,N) ).Key(K_CTRL|K_P);
				bar.Add("View/Edit", [&, N]{ OnViewFile(N);}).Key(K_CTRL|K_F);
				bar.Separator();
			}
			bar.Add("Copy (Select)", [&]{ OnMenuCopy();}).Key(K_CTRL|K_C);
			//if (bCanW&&IsClipboardAvailable("files")) //cb_has_files())
			if (bCanW&&cb_has_files())
			{
				bar.Add("Paste", [&, N]{ OnMenuPaste(N);}).Key(K_CTRL|K_V);
				if (!N->IsLocked()) bar.Add("Move here", [&, N]{ OnMenuMove(N);}).Key(K_CTRL|K_H);
			}
			bar.Separator();
		}

		if (bCanW)
		{
			if (!N->IsLocked())
			{
				bar.Add("Rename ..", [&, N]{ OnMenuRename(N);}).Key(K_F2);
				bar.Separator();
				bar.Add("Delete", [&]{ OnMenuDelete();}).Key(K_DELETE); //moved to dumpster
				bar.Add("Erase", [&]{ OnMenuErase(N);}).Key(K_CTRL|K_SHIFT|K_DELETE);
				bar.Separator();
				bar.Add("New Directory", [&, N]{ OnMenuNewDir(N);}).Key(K_CTRL|K_N);
				bar.Add("New Textfile", [&, N]{ OnMenuNewTextFile(N);}).Key(K_CTRL|K_E);
				bar.Separator();
				bar.Add("Set Access/Modify Datetime..", [&, N]{ TouchFD(N);});
				bar.Separator();
			}
		}

		bar.Sub(bIsFile||bIsDir, "Encryption", THISFN(EncryptionMenu));
		bar.Sub(bIsFile||bIsDir, "Compression", THISFN(CompressionMenu));
		bar.Separator();

		bar.Add("Properties", [&, N]{ OnMenuProperties(N);}).Key(K_F3);
		bar.Separator();
		bar.Add("Refresh", [this](){ RefreshPane(); });
	}
}

void DirectoryTab::EncryptionMenu(Bar &bar)
{
	PNode N=TG.GetFocusNode();
	if (N)
	{
		std::string s, sF=N->GetInfo();
		bool bCanR=canread(sF);
		bool bIsDir=isdir(sF);
		bool bIsFile=isfile(sF);
		bool bIsEnc=false; //isencrypted(sF);
		bool bCan=(bCanR&&(bIsDir||bIsFile));

		bar.Add(!bIsEnc&&bCan, spf("Encrypt ", sF).c_str(), [=](){ to_do });
		bar.Add(bIsEnc&&bCan, spf("Decrypt ", sF).c_str(), [=](){ to_do });
	}
	else
	{
		bar.Add(false, "Encrypt ", [=](){ to_do });
		bar.Add(false, "Decrypt ", [=](){ to_do });
	}
}

void DirectoryTab::CompressionMenu(Bar &bar)
{
	PNode N=TG.GetFocusNode();
	if (N)
	{
		std::string s, sF=N->GetInfo();
		bool bCanR=canread(sF);
		bool bIsDir=isdir(sF);
		bool bIsFile=isfile(sF);
		bool bIsCompressed=false; //isencrypted(sF);
		bool bCan=(bCanR&&(bIsDir||bIsFile));

		bar.Add(!bIsCompressed&&bCan, spf("Compress ", sF).c_str(), [=](){ to_do });
		bar.Add(bIsCompressed&&bCan, spf("Decompress ", sF).c_str(), [=](){ to_do });
	}
	else
	{
		bar.Add(false, "Compress ", [=](){ to_do });
		bar.Add(false, "Decompress ", [=](){ to_do });
	}
}

void DirectoryTab::OnMenuOpen(PNode N) { TG.Expand(N); }

void DirectoryTab::OnMenuOpenTab(PNode N)
{
	String S=N->GetInfo();
	if (pDFM) pDFM->OpenInNewTab(S, S);
	TG.Expand(N, false);
}

void DirectoryTab::OnMenuOpenWindow(PNode N) { if (pDFM) pDFM->OpenInNewWindow(N->GetInfo()); }

void DirectoryTab::OnMenuOpenFile()//PNode N)
{
	std::vector<PNode> v{};
	if (TG.GetSelectionNodes(v)==0) { if (TG.GetFocusNode()) v.push_back(TG.GetFocusNode()); }
	if (v.empty()) return;
	//Mime::AppPath ap{};
	Apps ap;
	for (auto n:v) { pDFM->mime.GetAppList(n->GetInfo(), ap); } // ap should be adjusted to show only
	std::string sx=GetOpenWith(v[0]->GetInfo(), ap); //todo .. common apps for all selected types: get_common_apps(v, &apps_list)
	if (!sx.empty())
	{
		runner::Arguments args{};
		for (auto n:v) args.add(n->GetInfo());
		runner::Run(sx, args);
	}
	
}

void DirectoryTab::OnMenuRun(PNode N)
{
	std::string sf=N->GetInfo();
	if (isscript(sf)) runner::Terminal(sf.c_str());
	else if (isexe(sf)) { runner::Arguments args; runner::Environment env; runner::Run(sf, args, env); }
	
}

void DirectoryTab::OnViewFile(PNode N)
{
	std::string sApp="/usr/bin/leafpad";																			//todo...use fix built-in viewer
	std::string sf=N->GetInfo();
	runner::Arguments args(sf);
	runner::Environment env;
	runner::Run(sApp, args, env);
}

void DirectoryTab::OnMenuRename(PNode N)
{
	std::string sold=N->GetInfo();
	std::string snew=GetNewName("Rename", path_name(sold));
	if (!snew.empty()) Rename(sold, snew);
}

void DirectoryTab::normalize_selection(std::vector<PNode> &v)
{
	//remove subs if parent is in selection
	if (v.size()<=1) return;
	size_t i=0;
	volatile size_t n;
	std::sort(v.begin(), v.end());
	n=v.size();
	while (i<(n-1))
	{
		if (issubdir(v[i+1]->GetInfo(), v[i]->GetInfo()))
		{
			auto it=(v.begin()+i+1);
			v.erase(it);
			n=v.size();
		}
		else i++;
	}
}

void DirectoryTab::ToolCall(int tn)
{
	switch(tn)
	{
		case ToolPane::TID_FIND: do_find_tool(); break;
		case ToolPane::TID_DIFF: do_compare_tool(); break;
		case ToolPane::TID_PATH:	{
										//to_do break; //<enter> pressed - chk tabs & focus-node
										telluser("tool-path: ", pDFM->toolpane.GetPath(), " - chk tabs & focus-node");
										
									} break;
		case ToolPane::TID_TOP: { TG.ListExpandingNodes(EXD_FIRST); if (pDFM) pDFM->toolpane.Select(ToolPane::TID_TOP); } break;
		case ToolPane::TID_BOT: { TG.ListExpandingNodes(EXD_LAST); if (pDFM) pDFM->toolpane.Select(ToolPane::TID_BOT); } break;
		case ToolPane::TID_AZ: { TG.ListExpandingNodes(EXD_NONE); if (pDFM) pDFM->toolpane.Select(ToolPane::TID_AZ); } break;
		//..more tools..
	}
}

void DirectoryTab::OnMenuCopy()
{
	std::string sfiles="", sformat="text/uri-list";
	std::vector<PNode> v;
	v.clear();
	if (pDFM) pDFM->ClearStatus();
	if (TG.GetSelectionNodes(v)==0) { PNode N=TG.GetFocusNode(); if (N) v.push_back(N); }
	normalize_selection(v);
	if (v.size()>0)
	{
		ClearClipboard();
		for (auto n:v) { sfiles+=spf("file://", n->GetInfo(), "\n"); }
		WriteClipboard(sformat.c_str(), sfiles.c_str());
	}
	v.clear();
}

void DirectoryTab::OnMenuPaste(PNode N)
{
	if (!N) return;
	bool bgotmux=false;
	int tries=10;
	while ((tries>0)&&!bgotmux) { KIPP(TO_MENUPASTE); tries--; bgotmux=MUX_DIRTAB.try_lock(); }
	if (bgotmux)
	{
		watcher::PauseWatcher();
		try
		{
			std::string dest;
			int ncopies=0;
			PNode T=((isdir(N->GetInfo()))?N:N->Parent());
			Vector<String> Vf;
			if (pDFM) pDFM->ClearStatus();
			Vf=GetFiles(Clipboard()); if (Vf.IsEmpty()) { watcher::PauseWatcher(false); return; }
			dest=T->GetInfo();
			WaitCursor WC; WC.Show();
		//	bool bX=T->nodes.IsExpanded();
		//	if (bX) TG.Expand(T, false);
			for (auto S:Vf)
			{
				std::string src=S.ToStd();
				if (isdir(src)) { dir_copy(src, dest); ncopies++; } //											todo - handle errors...
				else { file_copy(src, dest); ncopies++; } //													todo - handle errors...
			}
		//	if (bX) TG.Expand(T);
			dest=spf("Copied ", ncopies, (ncopies==1)?" file/directory":" files/directories");
			if (pDFM) pDFM->ShowStatus(dest.c_str());
		} catch(...){}
		MUX_DIRTAB.unlock();
		watcher::PauseWatcher(false);
		//RefreshPane();
	}
}

void DirectoryTab::OnMenuMove(PNode N)
{
	if (!N) return;
	bool bgotmux=false;
	int tries=10;
	while ((tries>0)&&!bgotmux) { KIPP(TO_MENUMOVE); tries--; bgotmux=MUX_DIRTAB.try_lock(); }
	if (bgotmux)
	{
		watcher::PauseWatcher();
		try
		{
			std::string tdir;
			int ncopies=0;
			PNode T=((isdir(N->GetInfo()))?N:N->Parent());
			Vector<String> Vf;
			if (pDFM) pDFM->ClearStatus();
			Vf=GetFiles(Clipboard()); if (Vf.IsEmpty()) { MUX_DIRTAB.unlock(); watcher::PauseWatcher(false); return; }
			tdir=T->GetInfo();
			WaitCursor WC; WC.Show();
		//	bool bX=T->nodes.IsExpanded();
		//	if (bX) TG.Expand(T, false);
			for (auto S:Vf)
			{
				if (isdir(S.ToStd())) { dir_move(S.ToStd(), tdir); ncopies++; } //											todo - check errors...
				else { file_move(S.ToStd(), tdir); ncopies++; } //															todo - check errors...
			}
		//	if (bX) TG.Expand(T);
			tdir=spf(tdir, "Moved ", ncopies, (ncopies==1)?" file/directory":" files/directories");
			if (pDFM) pDFM->ShowStatus(tdir.c_str());
		} catch(...){}
		MUX_DIRTAB.unlock();
		watcher::PauseWatcher(false);
		//RefreshPane();
	}
}

void DirectoryTab::OnMenuNewDir(PNode N)
{
	if (!N) return;
	std::string sp=N->GetInfo();
	std::string snew{};
	if (!isdir(sp)) sp=path_path(sp);
	snew=GetNewName("New Directory");
	if (!snew.empty()) NewDir(sp, snew);
}

void DirectoryTab::OnMenuNewTextFile(PNode N)
{
	if (!N) return;
	std::string sp=N->GetInfo();
	std::string snew;
	if (!isdir(sp)) sp=path_path(sp);
	snew=GetNewName("New Textfile");
	if (!snew.empty()) NewTextFile(sp, snew);
}

void DirectoryTab::OnMenuDelete()
{
	bool bgotmux=false;
	int tries=10;
	while ((tries>0)&&!bgotmux) { KIPP(TO_MENUDELETE); tries--; bgotmux=MUX_DIRTAB.try_lock(); }
	if (bgotmux)
	{
		watcher::PauseWatcher();
		try
		{
			std::vector<String> vk{}, vi{}; //nodes are reallocated & keys change on delete, so use info...
			if (!TG.GetSelectionKeys(vk)) { if (TG.GetFocusNode()) vk.push_back(TG.GetFocusNode()->GetKey().c_str()); }
			if (vk.size()!=0)//&&valid_for_delete(v)) todo?
			{
				//if (PromptOKCancel("&Are you sure you want to delete the selection?&"))
				//{
					WaitCursor WC; WC.Show();
					vi.clear();
					for (auto k:vk) { vi.push_back(TG.GetNode(k)->GetInfo()); }
					for (auto e:vi)
					{
						PNode Ne=TG.GetInfoNode(e);
						if (pDFM) pDFM->BackupToDumpster(e.ToStd()); //todo if not...
						TG.DeleteNode(Ne);
						if (isdir(e.ToStd())) dir_delete(e.ToStd()); else file_delete(e.ToStd());
					}
				//}
			}
		} catch(...){}
		MUX_DIRTAB.unlock();
		watcher::PauseWatcher(false);
		//RefreshPane();
	}
}

void DirectoryTab::OnMenuErase(PNode N)
{
	std::string s=spf("The selection will be permanently removed!\nYou will not be able to recover it by any means.\n\nDo you want to continue?");
	if (PromptYesNo(DeQtf(s.c_str())))
	{
		bool bgotmux=false;
		int tries=10;
		while ((tries>0)&&!bgotmux) { KIPP(TO_MENUDELETE); tries--; bgotmux=MUX_DIRTAB.try_lock(); }
		if (bgotmux)
		{
			watcher::PauseWatcher();
			try
			{
				std::vector<String> vk{}, vi{}; //nodes are reallocated & keys change on delete, so use info...
				if (!TG.GetSelectionKeys(vk)) { if (TG.GetFocusNode()) vk.push_back(TG.GetFocusNode()->GetKey().c_str()); }
				if (vk.size()!=0)
				{
					WaitCursor WC; WC.Show();
					vi.clear();
					for (auto k:vk) { vi.push_back(TG.GetNode(k)->GetInfo()); }
					for (auto e:vi) { SecureDelete(e.ToStd()); }
				}
			} catch(...){}
			MUX_DIRTAB.unlock();
			watcher::PauseWatcher(false);
			//RefreshPane();
		}
	}
}

void DirectoryTab::OnMenuProperties(PNode N)
{
	ShowProperties(N);
}

void DirectoryTab::SetTools()
{
	if (pDFM)
	{
		pDFM->toolpane.WhenTool = THISFN(ToolCall);
		PNode N=TG.GetFocusNode();
		if (N) pDFM->ShowCurFocus(N->GetInfo().c_str());
		
		pDFM->toolpane.Enable(ToolPane::TID_FIND);
		pDFM->toolpane.Enable(ToolPane::TID_DIFF);
		pDFM->toolpane.Enable(ToolPane::TID_PATH);
		pDFM->toolpane.Enable(ToolPane::TID_TOP);
		pDFM->toolpane.Enable(ToolPane::TID_BOT);
		pDFM->toolpane.Enable(ToolPane::TID_AZ);

		EXDisp ex=TG.ListExpandingNodes();
		if (ex==EXD_FIRST) pDFM->toolpane.Select(ToolPane::TID_TOP);
		else if (ex==EXD_LAST) pDFM->toolpane.Select(ToolPane::TID_BOT);
		else if (ex==EXD_NONE) pDFM->toolpane.Select(ToolPane::TID_AZ);
	}
}

void DirectoryTab::OnWhenFocus(PNode N) { if (pDFM) pDFM->ShowCurFocus(N->GetInfo()); }

bool DirectoryTab::cb_has_files()
{
	PasteClip pc;
	bool b{false};
	pc=Clipboard();
	b=pc.IsAvailable("files");
	return b;
}

void DirectoryTab::TabMenu(Bar &bar) { OnMenuBar(bar); }

void DirectoryTab::ShowProperties(PNode N)
{
	DirPropsDlg dlg(N);
	dlg.Execute();
}

void DirectoryTab::Rename(std::string sold, std::string snew)
{
	if (!snew.empty()&&!fsexist(snew)&&fsexist(sold))
	{
		watcher::PauseWatcher();
		try
		{
			if (isdir(sold)) dir_rename(sold, snew);
			else file_rename(sold, snew);
		} catch(...){ tellerror("Rename failed: ", filesys_error()); }
		watcher::PauseWatcher(false);
	}
}

void DirectoryTab::NewDir(std::string spath, std::string snew)
{
	std::string newdir=path_append(spath, snew);
	if (snew.empty()||fsexist(newdir)) { tellerror("NewDir:Invalid name: ", newdir); return; }
	bool bgotmux=false;
	int tries=10;
	while ((tries>0)&&!bgotmux) { KIPP(TO_MENUNEWDIR); tries--; bgotmux=MUX_DIRTAB.try_lock(); }
	if (bgotmux)
	{
		watcher::PauseWatcher();
		try
		{
			if (!dir_create(newdir)) tellerror("NewDir:Failed to create directory: ", filesys_error());
		} catch(...){}
		watcher::PauseWatcher(false);
		MUX_DIRTAB.unlock();
	}
}

void DirectoryTab::NewTextFile(std::string spath, std::string snew)
{
	std::string newfile=path_append(spath, snew);
	if (snew.empty()||fsexist(newfile)) { tellerror("NewTextFile:Invalid name: ", newfile); return; }
	bool bgotmux=false;
	int tries=10;
	while ((tries>0)&&!bgotmux) { KIPP(TO_MENUNEWTEXTFILE); tries--; bgotmux=MUX_DIRTAB.try_lock(); }
	if (bgotmux)
	{
		watcher::PauseWatcher();
		try
		{
			//runner::Touch(newfile);
			std::ofstream(newfile) << " ";
			if (!fsexist(newfile)) tellerror("NewTextFile:Failed to create file: ", filesys_error());
		} catch(...){}
		MUX_DIRTAB.unlock();
		watcher::PauseWatcher(false);
		//RefreshPane();
	}
}

void DirectoryTab::TouchFD(PNode N)
{
	if (!N) return;
	std::string sfd=N->GetInfo();
	std::string sdt=GetDTStamp();
	if (!sdt.empty()) runner::Touch(sfd, sdt);
}

void DirectoryTab::do_find_tool()
{
	//to_do
	FindDlg fdlg(pDFM);
	
	fdlg.Execute();
	
}

void DirectoryTab::do_compare_tool()
{
	//to_do
	//FileDiff ddlg;
	//ddlg.Execute();
	
	ShowFileDirDiff();
	
}



//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX


//==================================================================================================
DirTab::~DirTab() {}

DirTab::DirTab(const String &sname) : DirectoryTab(sname)
{
	auto isp=[&](std::string sd)->bool
		{
			bool b{false};
			std::string sp=DFMConf.getval("pinned");
			std::vector<std::string> vs;
			splitslist(sp, ':', vs, false);
			for (auto s:vs) { if ((b=seqs(s, sd))) break; }
			return b;
		};
	bPinable=true;
	bPinned=isp(sname.ToStd());

}

void DirTab::SetPin(bool bpin)
{
	if (bPinable) bPinned=bpin;
	bool b{false};
	std::string sd=sName.ToStd();
	pDFM->Tabs.DropTabXButton(sName);
	std::string sp=DFMConf.getval("pinned");
	std::vector<std::string> vs;
	splitslist(sp, ':', vs, false);
	auto it=vs.begin();
	while (!b&&(it!=vs.end())) { if ((b=seqs((*it), sd))) { if (!bpin) { vs.erase(it); } break; } else it++; }
	if (b&&!bpin)
	{
		sp.clear();
		for (auto s:vs) { spfs(sp, ":", s); } LTRIM(sp, ":"); DFMConf.setval("pinned", sp);
		 moveable=closeable=true;
	}
	else if (!b&&bpin)
	{
		spfs(sp, ":", sd); LTRIM(sp, ":"); DFMConf.setval("pinned", sp);
		moveable=closeable=false;
	}
	pDFM->Tabs.SetTabXPinPic(sName, bpin);
	if (!bpin) pDFM->Tabs.SetTabXButton(sName);
}

void DirTab::Setup(const String &sdir)
{
	moveable=closeable=true;
	DirectoryTab::Setup(sdir);
	if (IsPinned()) { moveable=closeable=false; pDFM->Tabs.SetTabXPinPic(sdir); }
	else pDFM->Tabs.SetTabXButton(sdir);
}

void DirTab::RefreshPane() { Setup(Data()); ShowDir(); }

void DirTab::TabMenu(Bar &bar) { DirectoryTab::TabMenu(bar); }







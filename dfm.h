#ifndef _dfm_dfm_h
#define _dfm_dfm_h

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include <HoM/HoM.h>
#include "pictures.h"
#include "tabber.h"
#include "devicetab.h"
#include "processtab.h"
#include "dirtab.h"
#include "mimeappico.h"
#include "picbutton.h"
#include <vector>
#include <map>
#include <functional>


extern AppConfig DFMConf;

struct UROOTWarn : public Ctrl
{
	using CLASSNAME=UROOTWarn;
	virtual ~UROOTWarn() {}
	void Paint(Draw &drw)
	{
		Size tsz, sz=GetSize();
		int x, y;
		String S="Careful: superuser access";
		drw.DrawRect(sz, LtRed());
		tsz=GetTextSize(S, StdFont().Bold());
		x=(sz.cx-tsz.cx)/2;
		y=(sz.cy-tsz.cy)/2;
		drw.DrawText(x, y, S, StdFont().Bold(), White());
	}
};

struct ToolPane : public Ctrl
{
	typedef ToolPane CLASSNAME;
	
	DFM *pDFM;
	PicButton btnFind;
	PicButton btnDiff;
	Label lblPath;
	EditString ebPath;
	PicButton btnTop;
	PicButton btnBot;
	PicButton btnAZ;

	enum { TID_FIND=1, TID_DIFF, TID_PATH, TID_TOP, TID_BOT, TID_AZ, };

	Event<int> WhenTool;

	virtual ~ToolPane();
	ToolPane();

	void SetPath(std::string sp);
	std::string GetPath();
	void Paint(Draw &drw);
	void DoCall(int tid);
	void PBClicked(PicButton *p);
	void Enable(int toolid, bool b=true);
	void Select(int toolid, bool b=true);

};

struct BusyPopup : public Label
{
	using CLASSNAME=BusyPopup;
	virtual ~BusyPopup(){}
	virtual void Paint(Draw &drw)
	{
		Size sz=GetSize();
		drw.DrawRect(sz, Green());
		String S="..stopping startup tasks..";
		Size szt=GetTextSize(S, StdFont().Bold());
		drw.DrawText((sz.cx-szt.cx)/2, (sz.cy-szt.cy)/2, S, StdFont().Bold(), White());
	}
};

struct DFM : public TopWindow
{
	typedef DFM CLASSNAME;

	BusyPopup BPBusy;
	Mime mime;
	StatusBar statusbar;
	MenuBar mainmenu;
	UROOTWarn urootwarn;
	ToolPane toolpane;
	Tabber Tabs;
	bool bHasFocus{false};
	
	void RefreshFileMappings();
	void init_dfm();
	
	virtual ~DFM();
	DFM();
	DFM(const Vector<String> &cmdln);

	virtual void Close();
	virtual void GotFocus() { bHasFocus=true; }
	virtual void LostFocus() { bHasFocus=false; }
	
	void create_dir_tabs();
	void show_pinned_tabs();
	void RemoveTab(const String &sname);
	void RenameTab(const String &sold, const String &snew);
	void RefreshTabIcons(TabType *ptab);
	void RefreshAllTabIcons();
	bool IsTab(const String &sname);
	
	void ShowStatus(const String &S);
	void ClearStatus();
	void OnTab();
	void ShowCurFocus(const String &S);
	void ClearCurFocus();
	
	void MakeMenu(Bar &bar);
	void MetaMenu(Bar &bar);
	void ContextMenu(Bar &bar);
	void ToolsMenu(Bar &bar);
	void HelpMenu(Bar &bar);
	void AppAssocsMenu(Bar &bar);
	
	void OpenInNewTab(const String &sname, const String &sdata);
	bool OpenInNewWindow(const String &SDir);
	
	bool BackupToDumpster(std::string sfd);
	void CustomAssocs();
};


#endif

#ifndef _dfm_dirpane_h_
#define _dfm_dirpane_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include "tabtype.h"
#include <string>
#include <vector>
#include <utilfuncs/utilfuncs.h>
#include <TreeGrid/treegrid.h>
#include <functional>
#include <watcher/watcher.h>
#include "mimeappico.h"


struct FSMETA
{
	bool bdir{false};
	std::string item{};
	size_t size{};
	int type{};
	std::string rights{};
	std::string owner{};
	std::string dtmodified{};
	int picid{};
	void clear() { bdir=false; item.clear(); size=0; type=0; rights.clear(); owner.clear(); dtmodified.clear(); picid=0; }
	virtual ~FSMETA() {}
	FSMETA() {}
	FSMETA(const FSMETA &F) { *this=F; }
	FSMETA& operator=(const FSMETA &F)
	{
		bdir=F.bdir; item=F.item; size=F.size; type=F.type; rights=F.rights;
		owner=F.owner; dtmodified=F.dtmodified; picid=F.picid;
		return *this;
	}
};

bool getFSMETA(const std::string &se, FSMETA &fsmeta);

typedef std::vector<FSMETA> VFSMETA;
bool FillVFSMETA(const std::string &sdir, VFSMETA &vfsmeta);

struct DirectoryTab : public TabType
{
	typedef DirectoryTab CLASSNAME;
	TreeGrid TG;
	
///	Image get_item_icon(std::string sd);

	virtual ~DirectoryTab();
	DirectoryTab(const DirectoryTab &T) { *this=T; }
	DirectoryTab(const String &sname=PANAMERR);

	DirectoryTab operator=(const DirectoryTab &T) { ((TabType)*this)=((TabType)T); TG=T.TG; return *this; }
	
	virtual void OnTabClosing();

	virtual bool DoWatchUpdateCB(std::string sk, watcher::FSACTION act, std::string sdf, std::string sdt);
	
	virtual void RefreshPane(bool bIcons=false);
	virtual void Layout();
	virtual void Setup(const String &sdir);
	virtual void Paint(Draw &drw);
	virtual void ShowDir();
	virtual void OnDoExpand(PNode N);
	virtual void OnDoContract(PNode N);
	virtual void OnMenuBar(Bar &bar);
	virtual void EncryptionMenu(Bar &bar);
	virtual void CompressionMenu(Bar &bar);
	
	virtual void OnMenuOpen(PNode N);
	virtual void OnMenuOpenTab(PNode N);
	virtual void OnMenuOpenWindow(PNode N);
	virtual void OnMenuOpenFile();//PNode N);
	virtual void OnMenuRun(PNode N);
	virtual void OnViewFile(PNode N);
	virtual void OnMenuRename(PNode N);
	virtual void OnMenuCopy();
	virtual void OnMenuPaste(PNode N);
	virtual void OnMenuMove(PNode N);
	virtual void OnMenuNewDir(PNode N);
	virtual void OnMenuNewTextFile(PNode N);
	virtual void OnMenuDelete();
	virtual void OnMenuErase(PNode N);
	virtual void OnMenuProperties(PNode N);

	virtual void SetTools();
	void OnWhenFocus(PNode N);

	bool cb_has_files();
	void normalize_selection(std::vector<PNode> &v);
	
	void ToolCall(int tn);
	
	virtual void TabMenu(Bar &bar);

	void ShowProperties(PNode N);
	void Rename(std::string sold, std::string snew);
	void NewDir(std::string spath, std::string snew);
	void NewTextFile(std::string spath, std::string snew);
	void TouchFD(PNode N);

	void do_find_tool();
	void do_compare_tool();

	
};

typedef DirectoryTab RootTab;

struct DirTab : public DirectoryTab
{
	typedef DirTab CLASSNAME;

	virtual ~DirTab();// {};
	DirTab(const String &sname);// : DirectoryTab(sname) { bPinable=true; bPinned=false; } //todo read from config...
	
	virtual bool IsPinable() { return bPinable; }
	virtual bool IsPinned() { return bPinned; }
	virtual void SetPin(bool bpin=true);
	virtual void Setup(const String &sdir);
	virtual void RefreshPane();
	virtual void TabMenu(Bar &bar);

};


#endif



#ifndef _dfm_processtab_h_
#define _dfm_processtab_h_

#include "tabtype.h"
#include "processes.h"
#include <TreeGrid/treegrid.h>


struct ProcessTab : public TabType
{
	typedef ProcessTab CLASSNAME;
	
	ProcessList PL{};
	
	TreeGrid TG{};

	virtual void SetPin(bool bpin=true) {}

	void clear() { } //DL.clear(); TG.Clear(); } //TabType::reset(); }
	
	virtual ~ProcessTab();
	ProcessTab(const String &sname=PANAMERR);
	ProcessTab(const ProcessTab &T) { *this=T; }

	ProcessTab operator=(const ProcessTab &T) { ((TabType)*this)=((TabType)T); PL=T.PL; TG=T.TG; return *this; }

	virtual void OnTabClosing();
	virtual void RefreshPane(bool bIcons=false);
	virtual void Layout();
	virtual void Setup(const String &sdata);

	void ResetProcesses(std::string sp="");
	void ShowProcesses();
	
	virtual void OnMenuBar(Bar &bar);
	void OnKill(int pid, bool bcli=false);
	void OnProperties(PNode N);

	virtual void TabMenu(Bar &bar);
	virtual void SetTools();

//?	void ShowProperties(std::string sd);

};


#endif

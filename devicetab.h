#ifndef _dfm_testpane_h_
#define _dfm_testpane_h_

#include "tabtype.h"
#include <TreeGrid/treegrid.h>
#include "devices.h"


struct DeviceTab : public TabType
{
	typedef DeviceTab CLASSNAME;
	
	Devices DL{};
	TreeGrid TG{};

	//bool IsPinable() { return bPinable; }
	//bool IsPinned() { return bIsPinned; }
	
	virtual void SetPin(bool bpin=true) {}

	void clear() { } //DL.clear(); TG.Clear(); } //TabType::reset(); }
	
	virtual ~DeviceTab();
	DeviceTab(const String &sname=PANAMERR);
	DeviceTab(const DeviceTab &T) { *this=T; }

	DeviceTab operator=(const DeviceTab &T) { ((TabType)*this)=((TabType)T); DL=T.DL; TG=T.TG; return *this; }

	virtual void OnTabClosing();
	virtual void RefreshPane(bool bIcons=false);
	virtual void Layout();
	virtual void Setup(const String &sdata);

	void ResetDevices(std::string sp="");
	void ShowDevices();
	
	virtual void OnMenuBar(Bar &bar);
	void OnMount(PNode N);
	void OnUnmount(PNode N);
	void OpenInTab(PNode N);
	void OpenInWindow(PNode N);

	void CheckReset();
	void CloseCDTray(std::string sd);
	void OpenCDTray(std::string sd);
	void OnProperties(PNode N);

	virtual void TabMenu(Bar &bar);
	virtual void SetTools();

	void ShowProperties(std::string sd);
};


#endif

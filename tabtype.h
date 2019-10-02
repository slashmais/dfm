#ifndef _tabtype_h_
#define _tabtype_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

//#include <functional>
#include <utilfuncs/utilfuncs.h>

const std::string PANAMERR=std::string("<ERROR>");

//============================
struct DFM;
struct TabType : public Ctrl
{
	typedef TabType CLASSNAME;
	
	DFM *pDFM;
	String sName{};
	String sData{};
	bool moveable{false};
	bool closeable{false};
	bool bPinable{false};
	bool bPinned{false};
	
	virtual ~TabType() {}
	TabType() {}
	TabType(const TabType &T) { *this=T; }
	TabType(const String &sname) { sName=sname; }

	TabType operator=(const TabType &T) { pDFM=T.pDFM; sName=T.sName; sData=T.sData; moveable=T.moveable; closeable=T.closeable; return *this; }
	
	virtual bool IsPinable() { return bPinable; }
	virtual bool IsPinned() { return bPinned; }
	virtual void SetPin(bool bpin=true) {}

	void reset() { pDFM=nullptr; sName=PANAMERR.c_str(); sData.Clear(); moveable=closeable=false; }
	virtual void OnTabClosing() {}
	virtual void RefreshPane(bool bIcons=false) {}
	virtual void Setup(const String&) {}
	String Name()					{ return sName; }
	void Name(const String sname)	{ sName=sname; RefreshPane(); }
	String Data()					{ return sData; }
	void Data(const String sdata)	{ sData=sdata; RefreshPane(); }
	
	virtual void TabMenu(Bar &bar)	{}
	virtual void SetTools() {}

	
};





#endif

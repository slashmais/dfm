#ifndef _dfm_tabber_h_
#define _dfm_tabber_h_

#include <CtrlLib/CtrlLib.h>

using namespace Upp;

 #include "tabtype.h"
#include <map>
#include "picbutton.h"


struct DFM;
struct Tabber : public TabCtrl
{
	typedef Tabber CLASSNAME;
	
	DFM *pDFM;
	std::map<String, TabType* > TabPanes{};
	std::map<String, PicButton> tabxs{};

	//Event<> WhenTab; //clicked/maybe changed

	virtual ~Tabber();
	Tabber();
	
	void AddTab(TabType *pPane);

	template<typename P> P* CreateTab(const String &sname) { return new P(sname); }
	template<typename P> void DestroyTab(P *p) { if (p) delete p; }
//	template<typename P> P* CreateTab(const String &sname) { P *p=new P(sname); return p; } // ((TabType*)p)->deleter=[p](){ delete (P*)p; }; return p; }
//	template<typename P> void DestroyTab(P *p) { std::function<void(void)> f=((TabType*)p)->deleter; f(); }

	template<typename T>void AddTab(const String &sname, const String &sdata="")
		{
			if (!HasTab(sname))
			{
				TabType *p=CreateTab<T>(sname);
				p->pDFM=pDFM;
				AddTab(p);
				p->Setup(sdata);
			}
			FocusTab(sname);
		}
	
	void RemoveTab(const String &sName);
	void RemoveAllTabs();
	bool HasTab(const String &sname);
	void FocusTab(const String &sname);
	TabType* FocusTab();
	void RenameTab(const String &curname, const String &newname);
	void SetTabXPinPic(const String &sname, bool bset=true);
	void SetTabXButton(const String &sname);
	void DropTabXButton(const String &sname);
	virtual void RightDown(Point p, dword keyflags);
	virtual void MouseEnter(Point p, dword keyflags);
	virtual void MouseLeave();

};



#endif

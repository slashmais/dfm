
#include "dfm.h"
#include "tabber.h"
#include <utilfuncs/utilfuncs.h>
#include "pictures.h"


Tabber::~Tabber() { } //RemoveAllTabs(); }

Tabber::Tabber() {}

void Tabber::AddTab(TabType *pTT)
{
	TabPanes[pTT->Name()]=pTT;
	auto& item=Add((*pTT).HSizePosZ().VSizePosZ(), pTT->Name());
	pTT->Tip(pTT->Name());
}

bool Tabber::HasTab(const String &sname) { return (TabPanes.find(sname)!=TabPanes.end()); }

void Tabber::FocusTab(const String &sname) { if (TabPanes.find(sname)!=TabPanes.end()) TabCtrl::Set(*(TabPanes[sname])); }

TabType* Tabber::FocusTab() { int idx=TabCtrl::Get(); if (idx>=0) { TabCtrl::Item &IT=GetItem(idx); return (TabType*)IT.GetSlave(); } return nullptr; }

void Tabber::RenameTab(const String &curname, const String &newname)
{
	if (HasTab(newname)||!HasTab(curname)) return;
	RemoveTab(curname);
	AddTab<DirTab>(newname, newname);
}

void Tabber::RemoveTab(const String &sname)
{
	if (HasTab(sname))
	{
		TabPanes[sname]->OnTabClosing();
		DropTabXButton(sname);
		TabCtrl::Remove(*TabPanes[sname]);
		DestroyTab(TabPanes[sname]);
		TabPanes.erase(sname);
	}
}

void Tabber::RemoveAllTabs() { while (!TabPanes.empty()) { RemoveTab(TabPanes.begin()->first); }}

void Tabber::SetTabXPinPic(const String &sname, bool bset)
{
	if (HasTab(sname))
	{
		TabCtrl::Item &IT=GetItem(Find(*(TabPanes[sname])));
		if (bset) IT.SetImage(GetPic(PICXPIN));
		else IT.SetImage(Image());
	}
}

void Tabber::SetTabXButton(const String &sname)
{
	if (HasTab(sname))
	{
		TabCtrl::Item &IT=GetItem(Find(*(TabPanes[sname])));
		tabxs[sname]=PicButton(GetPic(PICX));
		tabxs[sname].Data(sname.ToStd());
		tabxs[sname].WhenClick = [&](PicButton *pX) { if (pX) RemoveTab(pX->Data().c_str()); };
		IT.SetCtrl(tabxs[sname]);
	}
}

void Tabber::DropTabXButton(const String &sname)
{
	if (HasTab(sname))
	{
		TabCtrl::Item &IT=GetItem(Find(*(TabPanes[sname])));
		IT.SetCtrl(nullptr);
		tabxs.erase(sname);
	}
}

void Tabber::RightDown(Point p, dword) // keyflags)
{
	int t=TabCtrl::GetTab(p);
	if (t>=0)
	{
		MenuBar mb;
		TabType *pTT=(TabType*)(TabCtrl::GetItem(t).GetSlave());
		String S=pTT->Name();
		mb.Add(pTT->moveable, spf("Move '", S.ToStd(), "'-tab to a new window").c_str(), [&]{ pDFM->OpenInNewWindow(pTT->Data()); });
		mb.Add(pTT->closeable, spf("Close '", S.ToStd(), "'-tab").c_str(), [&]{ pDFM->RemoveTab(S); });
		if (pTT->IsPinable())
		{
			mb.Separator();
			if (pTT->IsPinned()) mb.Add("Unpin this tab", [&]{ pTT->SetPin(false); });
			else mb.Add("Pin this tab", [&]{ pTT->SetPin(); });
		}
		mb.Separator();
		mb.Add("Refresh", [&]{ pTT->RefreshPane(); });
		mb.Execute();
	}
}

void Tabber::MouseEnter(Point p, dword keyflags)
{
	int t=TabCtrl::GetTab(p);
	if (t>=0)
	{
		TabType *pTT=(TabType*)(TabCtrl::GetItem(t).GetSlave());
		pTT->MouseEnter(p, keyflags);
		//String S=pTT->Name();
		//pTT->ShowToolTip();
		//ToolTip TT;
		//TT.Set(pTT->Name());
		//TT.PopUp(pTT, p, false);
	}
}

void Tabber::MouseLeave()
{
	//
}


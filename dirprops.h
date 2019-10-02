#ifndef _dfm_dirprops_h_
#define _dfm_dirprops_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

//struct DFM;
struct DirectoryTab;

struct PicBox : public Ctrl
{
	using CLASSNAME=PicBox;
	Image pic;
	virtual ~PicBox(){}
	PicBox(){}
	PicBox(Image img) { pic=img; }
	//cctor..
	//operator=
	void Pic(Image img) { pic=img; Refresh(); }
	Image Pic() { return pic; }
	virtual void Paint(Draw &drw);
};

#define LAYOUTFILE <dfm/props.lay>
#include <CtrlCore/lay.h>

#include <TreeGrid/treegrid.h>
#include <string>

struct DirPropsDlg : public WithdirpropsLayout<TopWindow>
{
	using CLASSNAME=DirPropsDlg;
	
	PNode N;
	std::string sDE;
	bool bdirty;
	
	void init_dlg();
	
	virtual ~DirPropsDlg();
	DirPropsDlg();
	DirPropsDlg(PNode P);
	void ShowMeta();
	void ShowDatesRights();
	void on_change();
	void OnApply();
	void OnReset();
	void OnClose();
};






#endif

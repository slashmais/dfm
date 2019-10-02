#ifndef _dfm_finddlg_h_
#define _dfm_finddlg_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include <utilfuncs/utilfuncs.h>
#include <string>
#include <vector>

#include <CtrlLib/CtrlLib.h>
using namespace Upp;


#define LAYOUTFILE <dfm/dfm.lay>
#include <CtrlCore/lay.h>


struct DFM;


//---------------------------------------------------------------------------------------------------
struct FindDlg : public WithfindLayout<TopWindow>
{
	using CLASSNAME=FindDlg;

	DFM *pDFM;
	std::vector<std::string> vFound;

	~FindDlg();
	FindDlg(DFM *pdfm=nullptr);

	void Add(std::string sfd);
	void cancel_find_thread();
	void OnBar(Bar &bar);
	//void OpenInTab();
	void OnBrowse();
	void OnClose();
	void OnFind();
	//void DoFind();
	void ShowFiles();
	
};




#endif


#include "finddlg.h"
#include "dfm.h"
#include <utilfuncs/utilfuncs.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <thread>
#include <runner/runner.h>
#include <uppfun/uppfun.h>


bool bcancel_find_thread;


/*
//---------------------------------------------------------------------------------------------------
bool b_fg_terminated{false};

//---------------------------------------------------------------------------------------------------
void findfiles(const std::string sDir, bool bSubdirs, const std::string sNeedle, bool bCaseSensitive, std::vector<std::string> *pvFound, VCB pvcb)
{
	std::string sc;
	spfs(sc, "find -P \"%s\"%s-%s \"%s\"", sDir.c_str(), (!bSubdirs)?" -maxdepth 1 ":" ", (bCaseSensitive)?"name":"iname", sNeedle.c_str());
	b_fg_terminated=false;
	sc=SysCmd(sc);
	pvFound->clear();
	splitslist(sc, '\n', *pvFound, false);
	if (pvcb&&!b_fg_terminated) pvcb();
}

//---------------------------------------------------------------------------------------------------
void grepfiles(const std::string sDir, bool bSubdirs, const std::string sNeedle, bool bCaseSensitive, std::vector<std::string> *pvFound, VCB pvcb)
{
	std::string sc;
	mais::spf(sc, "cd \"%s\" ; grep -ls -m 1 %s%s\"%s\"", sDir.c_str(),
														  (bSubdirs)?"-r ":"",
														  (bCaseSensitive)?" ":"-i ",
														  sNeedle.c_str());
	b_fg_terminated=false;
	sc=SysCmd(sc);
	pvFound->clear();
	splitslist(sc, '\n', *pvFound, false);
	sc=sDir; sc+=(sc[sc.size()-1]=='/')?"":"/";
	for (auto &f:(*pvFound)) mais::spf(f, "%s%s", sc.c_str(), f.c_str());
	if (pvcb&&!b_fg_terminated) pvcb();
}
*/


//---------------------------------------------------------------------------------------------------
//bool bTTT{false};
//
//void TTT(FindDlg *pf)
//{
//	FindProgress *fp=new FindProgress;
//	fp->Show();
//	fp->TopMost();
//	fp->bGo=true;
//	for (int i=1; (i<=10)&&fp->bGo; i++)
//	 {
//	  fp->ShowCur(spf("..searching..", i));
//	  fp->ShowProgress(i, 10);
//	   if ((i%2)==0)
//	   {
//	    pf->arFind.Add(spf("found ", i/2).c_str(), "size", "type", "sRights", "sOwner", "sDate");
//	    pf->arFind.Refresh();
//	   }
//	    pf->Refresh();
//	    pf->Sync();
//	    fp->Sync();
//	     kipm(500);
//	      }
//	fp->bGo=false;
//	fp->Close();
//	delete fp;
//}


//---------------------------------------------------------------------------------------------------
FindDlg::~FindDlg()
{
	bcancel_find_thread=true;
	vFound.clear();
}

FindDlg::FindDlg(DFM *pdfm)
{
	CtrlLayout(*this, "Find");
	SetRect(0, 0, 900, 700);
	Sizeable();
	CenterScreen();

	pDFM=pdfm;
	vFound.clear();
	lblWhat.Hide();
	prog.Hide();
	bcancel_find_thread=true;

	std::string sd{"/"};
	if (pDFM) { sd=pDFM->toolpane.GetPath(); if (fsexist(sd)) { if (!isdir(sd)) { sd=path_path(sd); } if (!fsexist(sd)) { sd="/"; }}}
	ebDir.SetData(sd.c_str());
	optSubs.Set(true);
	radFG <<= 0;
	ebFind.SetData("");
	optCase.Set(false);

	arFind.AddColumn("Found", 300).Sorting();
	arFind.AddColumn("Size", 80).Sorting();
	arFind.AddColumn("Type", 80).Sorting();
	arFind.AddColumn("Rights", 100);
	arFind.AddColumn("Owner", 80).Sorting();
	arFind.AddColumn("Date", 150).Sorting(); //string-sort is fine
	arFind.WhenBar << THISBACK(OnBar);

	btnBrowse.WhenPush << THISBACK(OnBrowse);
	btnClose.WhenPush << THISBACK(OnClose);
	btnFind.SetLabel("Find");
	btnFind.WhenPush << THISBACK(OnFind);
}

void FindDlg::Add(std::string sfd)
{
	//get sfd info ... todo...
	arFind.Add(sfd.c_str(), "size", "type", "sRights", "sOwner", "sDate");
}

void FindDlg::OnBar(Bar &bar)
{
//	if (pDC&&(arFind.GetCursor()>=0)) bar.Add("Show in Tab", THISBACK(OpenInTab));
}

//void FindDlg::OpenInTab()
//{
//	if (!pDC) return;
//	int cr;
//	std::string s, sn, sk;
//	if ((cr=arFind.GetCursor())>=0)
//	{
//		s=arFind.Get(cr, 0).ToString().ToStd();
//		sk=path_path(s);
//		sn=path_name(sk);
//		spf(s, "N%s", s.c_str());
//		pDC->SetFocus();
//		pDC->WhenOpenTab(sn, sk, true, s);
//	}
//}

void FindDlg::OnBrowse()
{
	std::string s=SelectDirectory().ToStd();
	if (dir_exist(s)) ebDir.SetData(s.c_str());
}

void FindDlg::OnClose()
{
//	mais::b_vcb_is_gone=true;
	
	bcancel_find_thread=true;
	vFound.clear();
	Close();
}

//void FindDlg::kill_search_thread()
//{
//	if (!bIs_Searching) return;
//	if (!mais::system_has_tool("awk")) { SysMsg("Need 'awk' which is not available."); return; }
//
//	std::vector<std::string> vg, vv;
//	std::string sr="", sc="ps -ef | grep grep | grep '";
//	sc+=sFindString; sc+="' | awk '{print $0}'";
//
//	sr=mais::SysCmd(sc);
//	if (desv(sr, '\n', vg, false)>1)
//	{
//		if (desv(vg[1], ' ', vv, false)>1)
//		{
//			sc="kill -9 "; sc+=vv[1];
//			mais::SysCmd(sc);
//			lblSearching.Hide();
//			bIs_Searching=false;
//			btnFind.SetLabel("Find");
//		}
//	}
//}


void find_thread(FindDlg *pFD)
{
	ProgressBox *pPB=new ProgressBox(pFD);
	pPB->WhenCancel = [&]{ bcancel_find_thread=true; }; //pFD->cancel_find_thread(); };
	int i=1;
	while (!bcancel_find_thread&&(i<=20))
	{
		if (bcancel_find_thread) break;
		pPB->ShowProgress(i, 20, spf("..counting..", i).c_str());
		if ((i%3)==0) { std::string s=spf("found ", i/3+1); pFD->Add(s); }
		pFD->Sync();
		kipm(500);
		i++;
	}
	delete pPB;
	bcancel_find_thread=true;
}

void FindDlg::cancel_find_thread() { bcancel_find_thread=true; }

void FindDlg::OnFind()
{
	bcancel_find_thread=false;
	arFind.Clear();
	vFound.clear();
	std::thread(find_thread, this).detach();
}

//void FindDlg::DoFind()
//void DoFind(FindDlg *pFD)
//{
//	//ProgressBox pop(this);
//	//pop.Pop(this);
//	//pop.ShowProgress(0,0);
//
//	btnFind.SetLabel("Cancel");
//	lblWhat.Show();
//	lblWhat.SetLabel("");
//	prog.Show();
//	prog.Set(0,0);
//	CancelModeDeep();
//	NoIgnoreMouse();
//	for (int i=0; i<20; i++)
//	{
//		if (bCancel) break;
//		lblWhat.SetLabel(spf("..counting..", i).c_str());
//		prog.Set(i, 20);
//		if ((i%3)==0) { arFind.Add(spf("found ", i/3+1).c_str(), "size", "type", "sRights", "sOwner", "sDate"); }
//		Sync();
//		kipm(500);
//	}
//	lblWhat.Hide();
//	prog.Hide();
//	btnFind.SetLabel("Find");


//	if (bIs_Searching) { kill_search_thread(); return; }
//	
//	std::string sD=ebDir.GetData().ToString().ToStd(); //base-dir
//	sFindString=ebFind.GetData().ToString().ToStd();
//	bool bS=optSubs.Get(); //subdirs
//	bool bC=optCase.Get(); //case-sensitive
//	bool bF=(~radFG==0); //file/grep
//	
//	TRIM(sD); TRIM(sFindString); TRIM(sFindString, "*?");
//	if (!dir_exist(sD)) { PromptOK("Invalid directory"); return; }
//	if (sFindString.empty()) { PromptOK("Nothing to find!"); return; }
//	
//	arFind.Clear();
//	Add(lblSearching); lblSearching.Show();
//	btnFind.SetLabel("Stop");
//	mais::b_vcb_is_gone=false;
//	bIs_Searching=true;
//
//	if (bF)
//	{
//		spf(sFindString, "*%s*", sFindString.c_str());
//		std::thread(findfiles, sD, bS, sFindString, bC, &vFound, fdcb).detach();
//	}
//	else std::thread(mais::grepfiles, sD, bS, sFindString, bC, &vFound, fdcb).detach();
//}

void FindDlg::ShowFiles()
{
	arFind.Clear();
	if (vFound.empty()) arFind.Add(" - nothing found - ");
	else
	{
		struct stat STAT;
		struct tm *ptm;
		std::stringstream ss;
		std::string sSize, sType, sRights, sOwner, sDate;
		
		for (auto f:vFound)
		{
			sSize=sType=sRights=sOwner=sDate="";
			if (lstat(f.c_str(), &STAT)==0)
			{
				sOwner=username(STAT.st_uid);
				sSize=ttos<size_t>(STAT.st_size);
				ss << ((STAT.st_mode & S_IRUSR)?"r":"_") << ((STAT.st_mode & S_IWUSR)?"w":"_") << ((STAT.st_mode & S_IXUSR)?"x":"_"); //speed
				ss << ((STAT.st_mode & S_IRGRP)?"r":"_") << ((STAT.st_mode & S_IWGRP)?"w":"_") << ((STAT.st_mode & S_IXGRP)?"x":"_");
				ss << ((STAT.st_mode & S_IROTH)?"r":"_") << ((STAT.st_mode & S_IWOTH)?"w":"_") << ((STAT.st_mode & S_IXOTH)?"x":"_");
				sRights=ss.str(); ss.str(""); ss.flush();

				ptm = localtime(&STAT.st_mtime);
				ss << (ptm->tm_year+1900) //a #define or variable somewhere?
					<< "-" << (((ptm->tm_mon+1)<=9)?"0":"") << (ptm->tm_mon+1)
					<< "-" << ((ptm->tm_mday<=9)?"0":"") << ptm->tm_mday
					<< " " << ((ptm->tm_hour<=9)?"0":"") << ptm->tm_hour
					<< ":" << ((ptm->tm_min<=9)?"0":"") << ptm->tm_min
					<< ":" << ((ptm->tm_sec<=9)?"0":"") << ptm->tm_sec;
				sDate=ss.str(); ss.str(""); ss.flush();

//				switch(mais::getdetype(f))
//				{
//					case mais::DET_FILE: sType="file"; break;
//					case mais::DET_DIR: sType="dir"; break;
//					case mais::DET_DEVICE: sType="device"; break;
//					case mais::DET_PIPE: sType="pipe"; break;
//					case mais::DET_LINK: sType="link"; break;
//					case mais::DET_SOCKET: sType="socket"; break;
//					default: sType="unknown";
//				}
sType="unknown";

			}
			arFind.Add(f.c_str(), sSize.c_str(), sType.c_str(), sRights.c_str(), sOwner.c_str(), sDate.c_str());
		}
	}
}




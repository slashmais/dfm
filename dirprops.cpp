
#include "dirprops.h"
#include "dirtab.h"
#include <utilfuncs/utilfuncs.h>
#include <sys/stat.h>
#include <runner/runner.h>
#include <pwd.h>
#include "pictures.h"


void PicBox::Paint(Draw &drw)
{
	Size sz=GetSize();
	drw.DrawRect(sz, White());
	Image I=pic;
	if (I.IsEmpty()) I=GetPic(PICDEFAULT);
	elastic_resize(I, sz.cx, sz.cy);
	drw.DrawImage(0, 0, I);
}

DirPropsDlg::~DirPropsDlg()		{ if (bdirty&&PromptYesNo("Do you want to apply changes?")) OnApply(); }

void DirPropsDlg::init_dlg()
{
	CtrlLayout(*this, "Properties");
	Sizeable();

	btnApply.WhenAction << THISFN(OnApply);
	btnReset.WhenAction << THISFN(OnReset);
	btnClose.WhenAction << THISFN(OnClose);

	ShowMeta();
	ShowDatesRights();
}

DirPropsDlg::DirPropsDlg()
{
	N=nullptr;
	sDE.clear();
	bdirty=false;
	init_dlg();
}

DirPropsDlg::DirPropsDlg(PNode P)
{
	N=P;
	sDE=N->GetInfo();
	bdirty=false;
	init_dlg();
}

void DirPropsDlg::ShowMeta()
{
	if (!sDE.empty())
	{
		std::string s="", st;
		std::stringstream ss;
		struct stat STAT;
		struct passwd *pw;
	
		lstat(sDE.c_str(), &STAT); //---links
		ebName.SetData(sDE.c_str());
		ebSize.SetData(ttos<size_t>(STAT.st_size).c_str());
		pw=getpwuid(STAT.st_uid);
		if (pw) ebOwner.SetData(pw->pw_name); else ebOwner.SetData(ttos(STAT.st_uid).c_str());
		ebOwner.SetData(username(STAT.st_uid).c_str());
		st=runner::getFILEinfo(sDE);
		ebType.SetData(st.c_str());
		if ((STAT.st_mode&S_IFMT)==S_IFLNK)
		{
			char buf[1024];
			int n=readlink(sDE.c_str(), (char*)&buf, 1023);
			buf[n]=0;
			ebLink.SetData((char*)buf);
		}
		else ebLink.Clear();
		picbox.Pic(GetFDPic(sDE));
	}
}

void DirPropsDlg::ShowDatesRights()
{
	struct stat STAT;
	lstat(sDE.c_str(), &STAT);
	bool bchg;
	std::string sdt;
	auto tts=[](const time_t *t, std::string &sDate)
		{
			struct tm *ptm;
			std::stringstream ss;
			ptm = localtime(t);
			ss << (ptm->tm_year+1900) //a #define or variable somewhere?
				<< "-" << (((ptm->tm_mon+1)<=9)?"0":"") << (ptm->tm_mon+1)
				<< "-" << ((ptm->tm_mday<=9)?"0":"") << ptm->tm_mday
				<< " " << ((ptm->tm_hour<=9)?"0":"") << ptm->tm_hour
				<< ":" << ((ptm->tm_min<=9)?"0":"") << ptm->tm_min
				<< ":" << ((ptm->tm_sec<=9)?"0":"") << ptm->tm_sec;
			sDate=ss.str(); ss.str(""); ss.flush();
		};

	tts(&STAT.st_atime, sdt); ebAccessed.SetData(sdt.c_str());
	tts(&STAT.st_mtime, sdt); ebModified.SetData(sdt.c_str());
	tts(&STAT.st_ctime, sdt); ebStatusChanged.SetData(sdt.c_str());
	ebCreated.SetData("-");
	
	int ownr_id=STAT.st_uid;
	bchg=(has_root_access()||(realuid()==ownr_id)||(effectiveuid()==ownr_id));
	
	chkOwnR=((STAT.st_mode&S_IRUSR)==S_IRUSR); chkOwnR.Enable(bchg); chkOwnR.WhenPush << THISBACK(on_change);
	chkOwnW=((STAT.st_mode&S_IWUSR)==S_IWUSR); chkOwnW.Enable(bchg); chkOwnW.WhenPush << THISBACK(on_change);
	chkOwnX=((STAT.st_mode&S_IXUSR)==S_IXUSR); chkOwnX.Enable(bchg); chkOwnX.WhenPush << THISBACK(on_change);

	chkGrpR=((STAT.st_mode&S_IRGRP)==S_IRGRP); chkGrpR.Enable(bchg); chkGrpR.WhenPush << THISBACK(on_change);
	chkGrpW=((STAT.st_mode&S_IWGRP)==S_IWGRP); chkGrpW.Enable(bchg); chkGrpW.WhenPush << THISBACK(on_change);
	chkGrpX=((STAT.st_mode&S_IXGRP)==S_IXGRP); chkGrpX.Enable(bchg); chkGrpX.WhenPush << THISBACK(on_change);

	chkOthR=((STAT.st_mode&S_IROTH)==S_IROTH); chkOthR.Enable(bchg); chkOthR.WhenPush << THISBACK(on_change);
	chkOthW=((STAT.st_mode&S_IWOTH)==S_IWOTH); chkOthW.Enable(bchg); chkOthW.WhenPush << THISBACK(on_change);
	chkOthX=((STAT.st_mode&S_IXOTH)==S_IXOTH); chkOthX.Enable(bchg); chkOthX.WhenPush << THISBACK(on_change);
	
	btnReset.Enable(bdirty);
	btnApply.Enable(bdirty);
}

void DirPropsDlg::on_change() { bdirty=true; btnApply.Enable(true); btnReset.Enable(true); }

void DirPropsDlg::OnApply()
{
	if (bdirty)
	{
		std::string sc, sm="0", sr="";
		int n=0;
		
		if (chkOwnR) { n+=4; sr="r"; } else sr="-";
		if (chkOwnW) { n+=2; sr+="w"; } else sr+="-";
		if (chkOwnX) { n+=1; sr+="x"; } else sr+="-";
		sm+=ttos<int>(n); //"07"
		sr+=" ";
		n=0;
		if (chkGrpR) { n=4; sr+="r"; } else sr+="-";
		if (chkGrpW) { n+=2; sr+="w"; } else sr+="-";
		if (chkGrpX) { n+=1; sr+="x"; } else sr+="-";
		sm+=ttos<int>(n); //"077"
		sr+=" ";
		n=0;
		if (chkOthR) { n=4; sr+="r"; } else sr+="-";
		if (chkOthW) { n+=2; sr+="w"; } else sr+="-";
		if (chkOthX) { n+=1; sr+="x"; } else sr+="-";
		sm+=ttos<int>(n); //"0777"
		
		sc=spf("chmod ", sm.c_str(), " \"", sDE, "\"");
		runner::SysRun(sc);
		N->SetCellData(3, sr);
		OnReset();
	}
}


void DirPropsDlg::OnReset()
{
	bdirty=false;
	ShowDatesRights();
	
}

void DirPropsDlg::OnClose() { Close(); }


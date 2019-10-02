
#include "mimeappico.h"
#include <utilfuncs/utilfuncs.h>
#include <runner/runner.h>
#include "dfm.h"


void Mime::scan_desktops(const std::string &dir, ProgressBox *pPB, size_t curcount, size_t curmax)
{
	DirEntries de;
	if (dir_read(dir, de))
	{
		const std::string kM{"mimetype"}, kI{"icon"}, kN{"name"}, kX{"exec"};
		std::map<std::string, std::string> minx{}; //kM-kI-kN-kX
		std::vector<std::string> V{};
		//std::string f{}, sd{}, sk{}, sv{};
		std::string sd{}, sk{}, sv{};
		size_t n=0, tot;
		double perc;
		
		auto isdesktop=[](auto p)->bool{ return (isfiletype(p.second)&&seqs(".desktop", p.first.substr(p.first.size()-sizeof(".desktop")+1))); };
		
		tot=de.size();
		for (auto e:de)
		{
			n++;
			
			if (pPB) //popup-progress..
			{
				perc=(((double(n)/double(tot))*(curmax-curcount))+curcount);
				pPB->ShowProgress((int)perc, 100, e.first);
				if (pCaller) pCaller->Sync();
			}
			
			if (isdesktop(e))
			{
				file_read(path_append(dir, e.first), sd);
				splitslist(sd, '\n', V, false);
				minx.clear();
				for (auto kv:V)
				{
					splitslr(kv, '=', sk, sv);
					if (sieqs(sk, kM)||sieqs(sk, kI)||sieqs(sk, kN)||sieqs(sk, kX)) minx[lcase(sk)]=sv;
				}

				minx[kX]=runner::getwhich(minx[kX]);
				if (!minx[kX].empty())
				{
					if (minx[kM].empty()) { minx[kM]=GetMimetype(minx[kX]); }
					if (!minx[kM].empty())
					{
						if (minx[kN].empty()) { minx[kN]=path_name(minx[kX]); }
						if (!file_exist(minx[kI])) { sv=find_icon_path(minx[kI]); if (sv.empty()) { sv=find_icon_path(minx[kX]); } minx[kI]=sv; }
						V.clear(); splitslist(minx[kM], ';', V, false);
						for (auto s:V)
						{
							std::string mj, mi;
							size_t p;
							if ((p=s.find('/'))!=std::string::npos) { mj=s.substr(0, p); mi=s.substr(p+1); } else mj=mi=s;
							AppPI ap;
							ap.papp=minx[kX];
							ap.picon=minx[kI];
							apps[minx[kN]]=ap;
							mimes[mj][mi].push_back(minx[kN]);
						}
					}
				}
			}
		}
	}
}

/*
void load_mappings(Mime *self, Mime::ScanScope scanscope)
{
	b_stop_scan_desktops=false;
	auto ps=[&](std::string s){ if (self->pDFM) self->pDFM->ShowStatus(s); };
	auto ss=[scanscope](Mime::ScanScope s)->bool{ return ((scanscope&s)==s); };
	ps("Scanning .Desktops..");
	self->majors.clear();

//	if (self->pDFM)
//	{
//	if (DFM is starting-up) kipm(250); //give it some quarter..
//	}

	if (!b_stop_scan_desktops&&ss(Mime::SCAN_SYSTEM)) { ps("Scanning System .Desktops.."); self->scan_desktops(self->SystemDesktops); }
	if (!b_stop_scan_desktops&&ss(Mime::SCAN_USER)) { ps("Scanning User .Desktops.."); self->scan_desktops(self->LocalDesktops); }
	if (!b_stop_scan_desktops&&ss(Mime::SCAN_CUSTOM)) { ps("Scanning Custom .Desktops.."); self->scan_desktops(self->CustomDesktops); }
	if (b_stop_scan_desktops) ps("Desktop scanning stopped..");
	else ps("Desktop-mappings loaded");
	b_stop_scan_desktops=true;
	
//	std::string sdbg{};
//	size_t mi=0, ma=0;
//	for (auto p:self->majors)
//	{
//		mi+=p.second.size();
//		for (auto pp:p.second) ma+=pp.second.size();
//	}
//	sdbg=spf("maj#=", self->majors.size(), "\nmin#=", mi, "\nma#=", ma);

}
*/

Mime::~Mime() { clear(); }

Mime::Mime() //todo: below should be stored/retrieved from config or db..
{
	SystemDesktops="/usr/share/applications";
	LocalDesktops=path_append(homedir(), ".local/share/applications");
	CustomDesktops.clear();
	mimes.clear();
	apps.clear();
	pCaller=nullptr;
}

Mime::Mime(const Mime &M) { *this=M; }

Mime& Mime::operator=(const Mime &M)
{
	SystemDesktops=M.SystemDesktops;
	LocalDesktops==M.LocalDesktops;
	CustomDesktops=M.CustomDesktops;
	mimes=M.mimes;
	apps=M.apps;
	pCaller=nullptr; //acquired with Scan(..)
	return *this;
}

void Mime::clear() { mimes.clear(); }

bool Mime::GetAppList(std::string sf, Apps &rapps)
{
	std::string sc, st, mj, mi;
	size_t p;
	rapps.clear();
	st=GetMimetype(sf);
	if (!st.empty())
	{
		if ((p=st.find('/'))!=std::string::npos)
		{
			mj=st.substr(0, p);
			mi=st.substr(p+1);
			if ((mimes.find(mj)!=mimes.end())&&(mimes[mj].find(mi)!=mimes[mj].end()))
			{
				for (auto s:mimes[mj][mi]) { if (apps.find(s)!=apps.end()) rapps[s]=apps[s]; }
			}
		}
		else
		{
			mi=st;
			for (auto p:mimes)
			{
				if (p.second.find(mi)!=p.second.end()) { for (auto s:p.second[mi]) { if (apps.find(s)!=apps.end()) rapps[s]=apps[s]; }}
			}
		}
	}
	return (rapps.size()>0);
}

void Mime::Scan(ScanScope ss, Ctrl *pCtrl, bool bShowCtrl)
{
	pCaller=pCtrl;
	auto SS=[=](ScanScope s)->bool{ return ((ss&s)==s); };
	mimes.clear();
	apps.clear();

	ProgressBox *pPB=nullptr;
	if (pCaller&&bShowCtrl) pPB=new ProgressBox(pCaller);
	if (SS(SCAN_SYSTEM)) { if (pPB&&bShowCtrl) pPB->ShowProgress(0,100,"Scanning System .Desktops.."); scan_desktops(SystemDesktops, pPB, 0, 33); }
	if (SS(SCAN_USER)) { if (pPB&&bShowCtrl) pPB->ShowProgress(33,100,"Scanning User .Desktops.."); scan_desktops(LocalDesktops, pPB, 33, 66); }
	if (SS(SCAN_CUSTOM)) { if (pPB&&bShowCtrl) pPB->ShowProgress(66,100,"Scanning Custom .Desktops.."); scan_desktops(CustomDesktops, pPB, 66, 100); }
	if (pPB) delete pPB;
}

bool Mime::GetIcon(std::string se, Image &img)
{
	img.Clear();
	std::string sn=path_name(se);
	if (apps.find(sn)!=apps.end()) img=StreamRaster::LoadFileAny(apps[sn].picon.c_str()); // sn.c_str());
	return !img.IsNullInstance();
}

std::string GetMimetype(std::string sfile)
{
	std::string st{};
	if (file_exist(sfile))
	{
		std::string sc;
		sc=spf("file -b --mime-type '", sfile, "'");
		st=runner::SysCall(sc);
		TRIM(st);
	}
	return st;
}

std::string find_icon_path(std::string sn) //name | name.ext | /a/b/c/name.ext
{
	if (fsexist(sn)) return sn;
	std::vector<std::string> vpaths{ "/usr/share/pixmaps", "/usr/share/icons/hicolor/32x32/apps" };
	// todo .. srch /usr/share/icons and subs for ../apps/ & just srch this .../apps/-sub for icons
	std::string sfound{};
	std::string sname=file_name_noext(sn);
	std::string sf;
	sf=spf(sname, ".png");
	for (auto p:vpaths) { if (findfile(p, sf, sfound)) return sfound; }
	sf=spf(sname, ".xpm");
	for (auto p:vpaths) { if (findfile(p, sf, sfound)) return sfound; }
	sf=spf(sname, ".svg");
	for (auto p:vpaths) { if (findfile(p, sf, sfound)) return sfound; }
	return "";
}

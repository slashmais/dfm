#ifndef _dfm_resources_h_
#define _dfm_resources_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

#include <string>
#include <map>
#include <vector>
#include <uppfun/uppfun.h>


struct AppPI
{
	std::string papp;
	std::string picon;
	void clear() { papp.clear(); picon.clear(); }
	~AppPI(){}
	AppPI() : papp{}, picon{} {}
	AppPI(const AppPI &A) { *this=A; }
	AppPI& operator=(const AppPI &A) { papp=A.papp; picon=A.picon; return *this; }
};

template<typename K, typename V> struct MSV : public std::map<K, V>
{
	using M = std::map<K, V>;
	void clear() { for (auto& p:((M)(*this))) p.second.clear(); }
	~MSV() { clear(); }
	MSV() {}
	MSV(const MSV &m) { *this=m; }
	MSV& operator=(const MSV &m) { for (auto& p:((M)m)) ((M)(*this))[p.first]=p.second; return *this; }
};

using Apps=MSV<std::string, AppPI>; //[app-name]=..
using Minors=MSV<std::string, std::vector<std::string> >; //[mime-minor]=(list of app-names)
using Mimes=MSV<std::string, Minors>; //[mime-major]=..(map of mime-minors each with their list of associated app-names)

struct Mime // Mime-Apps-Icons
{
	enum ScanScope { SCAN_SYSTEM=1, SCAN_USER=2, SCAN_CUSTOM=4, SCAN_ALL=(SCAN_SYSTEM|SCAN_USER|SCAN_CUSTOM), };
	//struct PathIcon { std::string apppath; std::string iconpath; }; //Image icon; };
//	typedef std::map<std::string, std::string> AppIcon; //[app-path]=app-icon-path
//	typedef std::map<std::string, std::string> AppPath; //[app-name]=app-path
//	typedef std::map<std::string, AppPath> Minors; //[mime-minor]=..
//	typedef std::map<std::string, Minors> Majors; //[mime-major]=..

	std::string SystemDesktops; //todo:...should be user-definable... retrieve from config..
	std::string LocalDesktops;
	std::string CustomDesktops;
	Apps apps{};
	Mimes mimes{};
	Ctrl *pCaller;
	
	void scan_desktops(const std::string &dir, ProgressBox *pPB, size_t curcount, size_t curmax);
	
	~Mime();
	Mime();
	Mime(const Mime &M);
	Mime& operator=(const Mime &M);
	void clear();
	bool GetAppList(std::string sf, Apps &rapps); //apps applicable to file 'sf'
	void Scan(ScanScope ss=SCAN_ALL, Ctrl *pCtrl=nullptr, bool bShowCtrl=true);
	bool GetIcon(std::string se, Image &img);
};

std::string GetMimetype(std::string sfile);
//void RefreshMimeMappings(MimeApps &MA, MimeApps::ScanScope scanscope=MimeApps::SCAN_ALL, DFM *p=nullptr);
//void StopMimeMappings();
//bool IsMimeMapping(); //busy?

std::string find_icon_path(std::string sn); //sn={name | name.ext | /a/b/c/name.ext }


#endif

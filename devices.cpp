
#include "devices.h"
#include <utilfuncs/utilfuncs.h>
#include <runner/runner.h>


bool operator==(const DeviceData &l, const DeviceData &r) { return seqs(l.sname, r.sname); }

template<typename M> bool mkmatch(const M &l, const M &r)
{
	return ((l.size()==r.size())&&std::equal(l.begin(), l.end(), r.begin(), [](auto ll, auto rr)->bool{ return (ll.first==rr.first); }));
}

bool operator==(const Device &l, const Device &r)
{
	if (!seqs(l.sname, r.sname)) return false;
	return mkmatch(l.partitions, r.partitions);
}

bool operator==(const Devices &l, const Devices &r)
{
	return ((l.size()==r.size())&&std::equal(l.begin(), l.end(), r.begin()));
}

void getdiskdata(Devices &devs)
{
	
	
//	get from:
//		/proc/mounts
//		/proc/mountinfo
//		/proc/mountstats
//		/proc/partitions
//		...
// look in fstab for user's mountpoints ... why? it'll be mounted anyway & in /proc/mounts&mountinfo&mountstats&partitions
//		/dev/disk/by-id/...
//		/dev/disk/by-label/...
//		/dev/disk/by-uuid/...
//

//	/proc/diskstats -> lists present block devices in 3rd column
//	/proc/partitions -> lists sizes(kilobytes) in 3rd column, name in 4th
	
	
	
	std::vector<std::string> vlines{};
	std::string sc{};
	typedef std::map<std::string, std::string> HMAP;
	auto tomap=[](const std::string sr, HMAP &hm)
		{
			std::vector<std::string> vdat{};
			std::string sk, sv;
			hm.clear();
			vdat.clear(); splitsslist(sr, "\" ", vdat, false);
			for (auto s:vdat)
			{
				size_t p=s.find('=');
				if (p>0) { sk=s.substr(0, p); TRIM(sk, " \"\'\t"); sv=s.substr(p+1); TRIM(sv, " \"\'\t"); hm[sk]=sv; }
			}
			//return hm;
		};
	auto uf=[](const std::string &D, const std::string &T, std::string &U, std::string &F)
		{	//fix this to get size, used, free values																todo
			std::string sc, s=T;
			char c;
			std::vector<std::string> v;
			size_t nt, nf;//, n;
			c=s[s.size()-1]; s=s.substr(0, s.size()-1);
			nt=stot<size_t>(s); nt*=((c=='K')?1024:(c=='M')?1048576:(c=='G')?1073741824:1);
			sc=spf("df | grep '", D, "'");
			sc=runner::SysCall(sc.c_str());
			TRIM(sc);
			if (!sc.empty())
			{
				v.clear(); splitslist(sc, ' ', v, false);
				//nt=(stot<size_t>(v[1])*1024); //T=to_sKMGT(nt);
				nf=(stot<size_t>(v[3])*1024); F=to_sKMGT(nf);
				U=to_sKMGT(nt-nf);
			}
		};

	devs.clear();

	sc="lsblk -Pdno NAME,FSTYPE,MOUNTPOINT,LABEL,UUID,MODEL,SIZE,TRAN,GROUP";
	sc=runner::SysCall(sc.c_str());
	vlines.clear(); splitslist(sc, '\n', vlines, false);
	for (auto s:vlines)
	{
		HMAP hm; tomap(s, hm);
		DeviceData &D=devs[hm["NAME"]];
		D.sname=hm["NAME"];
		D.sident=hm["MODEL"];
		D.slabel=hm["LABEL"];
		D.stype=spf(hm["TRAN"], "-", hm["GROUP"]);
		D.smount=hm["MOUNTPOINT"];
		D.stotal=hm["SIZE"];
		D.sused="";
		D.sfree="";
	}

	if (!devs.empty())
	{
		for (auto& p:devs)
		{
			sc=spf("lsblk -Ppno NAME,FSTYPE,MOUNTPOINT,LABEL,UUID,SIZE", " /dev/", p.first);
			sc=runner::SysCall(sc.c_str());
			vlines.clear(); splitslist(sc, '\n', vlines, false);
			if (vlines.size()>1) vlines.erase(vlines.begin());
			for (auto s:vlines)
			{
				HMAP hm; tomap(s, hm);
				DeviceData &dd=devs[p.first].partitions[hm["NAME"]];
				dd.sname=hm["NAME"];
				dd.sident=hm["UUID"];
				dd.slabel=hm["LABEL"];
				dd.stype=hm["FSTYPE"];
				dd.smount=hm["MOUNTPOINT"];
				dd.stotal=hm["SIZE"];
				dd.sused="";
				dd.sfree="";
				if (!dd.smount.empty()) uf(dd.sname, dd.stotal, dd.sused, dd.sfree);
				//else { dd.sused=""; dd.sfree=""; }
			}
		}
	}
}

void getdevicedata(Devices &devs, std::string sdev, DeviceData &dd)
{
	//Devices devs;
	getdiskdata(devs);
	if (devs.find(sdev)!=devs.end()) dd=(DeviceData)devs[sdev];
	else
	{
		bool b{false};
		for (auto p:devs)
		{
			for (auto pp:p.second.partitions)
			{
				if ((b=(pp.first==sdev))) { dd=pp.second; break; }
			}
			if (b) break;
		}
	}
}

std::string getmountpoint(std::string sdev)
{
	//std::vector<std::string> v;
	std::string sp{};
	std::string sc{};
	sc=spf("lsblk -pno NAME,MOUNTPOINT ", sdev);
	sc=runner::SysCall(sc.c_str());
	//v.clear(); splitslist(sc, ' ', v);//, false);
	if (!sc.empty()) { sp=sc.substr(sdev.size()); LTRIM(sp); RTRIM(sp, "\n"); } //trailing ' ' (spaces)?
	//if (v.size()>1) return v[1];
	//return "";
	return sp;
}

bool ismountpoint(std::string smt)
{
	
//	get from:
//		/proc/mounts

	
	std::vector<std::string> v;
	std::string sc;
	sc=spf("lsblk -no MOUNTPOINT");
	sc=runner::SysCall(sc.c_str());
	v.clear(); splitslist(sc, '\n', v, false);
	bool b=false;
	auto it=v.begin();
	while (!b&&it!=v.end()) { if ((b=seqs(smt, (*it)))) break; it++; }
	return b;
}


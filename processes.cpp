
#include "processes.h"
#include <utilfuncs/utilfuncs.h>
#include <runner/runner.h>

ProcessList::ProcessList()
{
	fillpl();
}

void ProcessList::fillpl()
{
	clear();
	DirEntries de;
	std::string procdir{"/proc"};
	dir_read(procdir, de);
	
	auto name=[](std::string cmd, std::string &n)
			{
				std::string s, sc{cmd};
				size_t p=sc.find(' ');
				if (p!=std::string::npos)
				{
					s=sc.substr(0, p);
					///todo: check if exist or name contain spaces ...
				}
				else s=sc;
				s=path_name(s);
		if (!s.empty()) {
				n.clear();
		}
				size_t i=0; while (s[i]) { n+=s[i]; i++; } //got weird 0-terminated string with "-bash"
				TRIM(n, "-: ");
			};
	
	auto user=[procdir](int pid, std::string &su, int &uid)
	{
		std::string s{}, sp=path_append(procdir, ttos<int>(pid));
		sp=path_append(sp, "status");
		file_read(sp, s);
		size_t p=s.find("Uid:"); //todo check npos...
		size_t e=s.find('\n', p);
		s=s.substr(p+5, e-p);
		TRIM(s);
		if ((p=s.find(' '))==std::string::npos) p=s.find('\t');
		if (p!=std::string::npos) s=s.substr(0, p);
		if (!s.empty()) { uid=stot<int>(s); su=username(uid); }
	};
	
	for (auto p:de)
	{
		std::string sp=path_append(procdir, p.first);
		if (IsInteger(p.first)&&isdir(sp))
		{
			std::string sc, t, c, a;
			sp=path_append(sp, "cmdline");
			if (file_read(sp, sc))
			{
				TRIM(sc);
				if (!sc.empty())
				{
					Process P;
					int uid;
					P.pid=stot<int>(p.first);
					name(sc, P.appname);
					if (sc[0]=='-') { P.appname=spf(P.appname, "  //*cli-login-prompt"); LTRIM(sc, "-"); }
					if (sc[0]!='/')
					{
						size_t p=sc.find(' ');
						if (p!=std::string::npos) { a=sc.substr(p); t=sc.substr(0, p); }
						else { a.clear(); t=sc; }
						if (t[t.size()-1]==':') { t=t.substr(0, t.size()-1); a.insert(a.begin(), ':'); }
						c=runner::getwhich(t);
						if (c.empty()) { c=path_append("/usr/sbin", t); if (!file_exist(c)) c.clear(); }
						if (!c.empty()) sc=spf(c, a);
					}
					P.cmdline=sc;
					user(P.pid, P.owner, P.uid);
					pl::push_back(P);
				}
			}
		}
	}
}

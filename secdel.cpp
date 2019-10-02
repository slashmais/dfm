#include "secdel.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <random>
#include <dirent.h>


namespace secdel
{

#define NBUF 512

bool isfile(std::string s)
{
	struct stat st;
	if (stat(s.c_str(), &st)>=0) { return S_ISREG(st.st_mode); }
	return false;
}

bool isdir(std::string s)
{
	struct stat st;
	if (stat(s.c_str(), &st)>=0) { return S_ISDIR(st.st_mode); }
	return false;
}

void randomize(char *p, size_t n)
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<std::mt19937::result_type> d(0, 255);
	for (size_t i=0; i<n; i++) p[i]=d(g);
}

bool fillfile(std::string sf, size_t nf)
{
	bool bret{false};
	char *buf=(char*)malloc(NBUF);
	auto wf=[](std::string s, char *b, size_t N)->bool
			{
				int r, f=open(s.c_str(), O_WRONLY);
				if (f>=0)
				{
					size_t n=0;
					while ((r=write(f, b, (((N-n)>=NBUF)?NBUF:(N-n))))>0) n+=r;
					fsync(f); close(f);
					return true;
				}
				return false;
			};

	if (buf)
	{
		randomize(buf, NBUF);
		if ((bret=wf(sf, buf, nf)))
		{
			memset(buf, 0, NBUF);
			bret=wf(sf, buf, nf);
		}
		free(buf);
	}
	return bret;
}

bool sec_del_file(std::string sf)
{
    struct stat st;
    if (stat(sf.c_str(), &st)>=0)
    {
		size_t n=st.st_size;
		if (fillfile(sf, n)) { return (remove(sf.c_str())>=0); }
    }
    return false;
}

bool sec_del_dir(std::string sd)
{
	struct dirent *pe;
	std::string sfd;
	DIR *pD;

	if ((pD=opendir(sd.c_str())))
	{
		while ((pe=readdir(pD)))
		{
			sfd=sd; sfd+=(sd[sd.size()]=='/')?"":"/"; sfd+=(char*)(pe->d_name);
			SecureDelete(sfd);
		}
		closedir(pD);
		remove(sd.c_str());
	}
	return false;
}

} //namespace

bool SecureDelete(std::string sfd)
{
	if (secdel::isfile(sfd)) return secdel::sec_del_file(sfd);
	if (secdel::isdir(sfd)) return secdel::sec_del_dir(sfd);
	return remove(sfd.c_str()); //pipes/links/sockets/devices
}

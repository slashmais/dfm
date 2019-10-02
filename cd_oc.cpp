
#include "cd_oc.h"
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/cdrom.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <utilfuncs/utilfuncs.h>

//SCSI ioctl's (?which header?)
#define CDIOCEJECT 0x20006318
#define CDIOCCLOSE 0x2000631c

std::string cd_oc_err{};
std::string get_cd_oc_err() { return cd_oc_err; }

bool open_fcd(std::string sdev, int &fcd)
{
	if (sdev.empty()) { cd_oc_err="no cd-device"; return false; }
	bool b{false};
	if ((b=!((fcd=open(sdev.c_str(), O_RDONLY|O_NONBLOCK))<0))) cd_oc_err=spf("cannot access ", sdev, ": ", std::strerror(errno));
	return b;
}

inline void close_fcd(int fcd) { close(fcd); }

bool IsCDDrive(std::string sdev)
{
	bool b{false};
	int fcd;
	if ((b=open_fcd(sdev, fcd))) { b=(ioctl(fcd, CDROM_DRIVE_STATUS, 0)>=0); close_fcd(fcd); }
	return b;
}

int GetCDState(std::string sdev)
{
	if (!IsCDDrive(sdev)) return CD_ERROR;
	int fcd, n=0, tries=10;
	bool b{false};
	while (!b&&(tries-->0))
	{
		if (open_fcd(sdev, fcd))
		{
			n=ioctl(fcd, CDROM_DRIVE_STATUS);
			b=(n!=CDS_DRIVE_NOT_READY);
			close_fcd(fcd);
		}
		kips(5);
	}
	return n;
}

int CDTrayState(std::string sdev)
{
	if (sdev.empty()) { cd_oc_err="no cd-device"; return false; }
	int fcd;
	int cdt=CD_ERROR;
	if (open_fcd(sdev, fcd))
	{
		int n=ioctl(fcd, CDROM_DRIVE_STATUS, 0);
		if (n==CDS_TRAY_OPEN) cdt=CD_OPEN;
		else if (n!=CDS_NO_DISC) cdt=CD_CLOSED;
		else cdt=CD_NO_INFO;
		close_fcd(fcd);
	}
	return cdt;
}

bool open_tray(int fcd)
{
	int n=0;
    cd_oc_err.clear();
	ioctl(fcd, CDROM_LOCKDOOR, 0); //I wonder why this is needed
	if ((n=ioctl(fcd, CDROMEJECT))<0)
	{
		cd_oc_err=spf("open tray failed: (cdrom-error): ", std::strerror(errno));
		if ((n=ioctl(fcd, CDIOCEJECT))<0) { cd_oc_err+=spf(" (SCSI-error): ", std::strerror(errno)); }
	}
	return (n>=0);
}

bool close_tray(int fcd)
{
	int n=0;
    cd_oc_err.clear();
	if ((n=ioctl(fcd, CDROMCLOSETRAY))<0)
	{
		cd_oc_err=spf("close tray failed: (cdrom-error): ", std::strerror(errno));
		if ((n=ioctl(fcd, CDIOCCLOSE))<0) { cd_oc_err+=spf(" (SCSI-error): ", std::strerror(errno)); }
	}
	return (n>=0);
}

bool CDOpen(std::string sdev)
{
	bool b{false};
	int fcd;
	if ((b=open_fcd(sdev, fcd))) { b=open_tray(fcd); close_fcd(fcd); }
	return b;
}

bool CDClose(std::string sdev)
{
	bool b{false};
	int fcd;
	if ((b=open_fcd(sdev, fcd))) { b=close_tray(fcd); close_fcd(fcd); }
	return b;
}

bool HasCD(std::string sdev)
{
	int fcd, tries=5;
	bool b{false};
	while ((tries-->0)&&!b)
	{
		if (open_fcd(sdev, fcd))
		{
			int n;
			if ((n=ioctl(fcd, CDROM_DRIVE_STATUS)!=CDS_DRIVE_NOT_READY)) b=(n==CDS_DISC_OK);
			close_fcd(fcd);
		}
		if (!b) kipm(200);
	}
	return b;
}





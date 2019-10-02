#ifndef _devices_h_
#define _devices_h_

#include <string>
#include <vector>
#include <map>


struct DeviceData
{
	std::string sname{}; //device or partition; sdx or /dev/sdxn; sr0 or /dev/sr0; ...
	std::string sident{}; //model or uuid
	std::string slabel{};
	std::string stype{}; //dev or part; sata-disk/usb-disk/usb-cdrom or ext3/iso9660/vfat
	std::string smount{}; //mountpoint-path
	std::string stotal{}; //nnnG, nnnM, nnnT, ...
	std::string sused{};
	std::string sfree{};
	void clear() { sname=sident=slabel=stype=smount=stotal=sused=sfree=""; }
	virtual ~DeviceData() {}
	DeviceData() {}
	DeviceData(const DeviceData &DD) { *this=DD; }
	DeviceData& operator=(const DeviceData &DD)
	{
		sname=DD.sname; sident=DD.sident; slabel=DD.slabel; stype=DD.stype;
		smount=DD.smount; stotal=DD.stotal; sused=DD.sused; sfree=DD.sfree;
		return *this;
	}
	
};

bool operator==(const DeviceData &l, const DeviceData &r);

struct Device : public DeviceData //e.g.: /dev/sda containing bunch of partitions
{
	std::map<std::string, DeviceData> partitions{};
	void clear() { partitions.clear(); DeviceData::clear(); }
	virtual ~Device() {}
	Device() : DeviceData() { clear(); }
	Device(const Device &D) : DeviceData() { *this=D; }
	Device& operator=(const Device &D) { ((DeviceData)*this)=D; partitions=D.partitions; return *this; }
};

bool operator==(const Device &l, const Device &r);

typedef std::map<std::string, Device> Devices;

bool operator==(const Devices &l, const Devices &r);


void getdiskdata(Devices &devs);
void getdevicedata(Devices &devs, std::string sdev, DeviceData &dd);
std::string getmountpoint(std::string sdev);
bool ismountpoint(std::string smt);

#endif

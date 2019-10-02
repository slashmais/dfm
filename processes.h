#ifndef _dfm_processes_h_
#define _dfm_processes_h_

#include <string>
#include <vector>

struct Process
{
	int pid;
	std::string cmdline;
	std::string appname;
	int uid; //owner
	std::string owner;
};

struct ProcessList : public std::vector<Process>
{
	using pl=std::vector<Process>;
	void clear() { pl::clear(); }
	~ProcessList() { clear(); }
	ProcessList();
	void fillpl();
};

#endif

#ifndef _dfm_cd_oc_h_
#define _dfm_cd_oc_h_

#include <string>

std::string get_cd_oc_err();

enum { CD_ERROR=(-1), CD_NO_INFO=0, CD_NO_DISC, CD_DISC, CD_OPEN, CD_CLOSED, };
//CD_ERROR can be: not a cd-drive, cannot access

bool IsCDDrive(std::string sdev);
int GetCDState(std::string sdev);
bool CDOpen(std::string sdev);
bool CDClose(std::string sdev);
int CDTrayState(std::string sdev);
bool HasCD(std::string sdev);


#endif

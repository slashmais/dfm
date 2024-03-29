#ifndef _dfm_pictures_h_
#define _dfm_pictures_h_

#include <CtrlLib/CtrlLib.h>
using namespace Upp;

struct DFM;

enum
{
	PICX,
	PICXPIN,
	PICDFMICON,
	PICEMPTY,
	PICNOENTRY,
	PICLOCKED,
	PICBLOCKDEVICE,
	PICCHARDEVICE,
	PICSOCKET,
	PICPIPE,
	PICLINK,
	PICLINKDIR,
	PICLINKFILE,
	PICDISK,
	PICPARTITION,
	PICSWAPPARTITION,
	PICUSB,
	PICCDDVD,
	PICDIR,
	PICNEWDIR,
	PICHOME,
	PICFILE,
	PICFILERUN,
	PICTEXT,
	PICSCRIPT,
	PICUNKNOWN,
	PICDEFAULT=PICUNKNOWN,
	PICBINARYFILE,
	PICEXECUTABLE,
	PICELF,
	PICMASKEXEC,
	PICAUDIO,
	PICVIDEO,
	PICGRAPHIC,
	PICDOC,
	PICPDF,
	PICCODE,
	PICWEB,
	PICENCRYPTED,
	//PICARCHIVE,
	PICCOMPRESSED,
	PICDATA,
	PICXML,
	PICFIND,
	PICDIFF,
	PICUPARROW,
	PICRIGHTARROW,
	PICLEFTARROW,
	PICREFRESH,
	PICGO,
	PICNODESTOP,
	PICNODESBOT,
	PICNODESNONE,
};

Image GetPic(int npic);
Image GetFDPic(std::string sfd, DFM *pdfm=nullptr, bool bIco=false, bool bImg=true);
void elastic_resize(Image &pic, int w, int h);


#endif

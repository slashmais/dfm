
#include "pictures.h"

#define  IMAGEFILE  <dfm/dfm.iml>
#define  IMAGECLASS DFMImg
#include <Draw/iml.h>

#include <utilfuncs/utilfuncs.h>
#include "mimeappico.h"
#include "dfm.h"

Image GetPic(int npic)
{
	switch(npic)
	{
		case PICX:				return DFMImg::PicX(); break;
		case PICXPIN:			return DFMImg::PicXPin(); break;
		case PICDFMICON:		return DFMImg::DFMICON(); break;
		case PICEMPTY:			return DFMImg::PicEmpty(); break;
		case PICNOENTRY:		return DFMImg::PicNoEntry(); break;
		case PICLOCKED:			return DFMImg::PicLocked(); break;
		case PICBLOCKDEVICE:	return DFMImg::PicBlockDevice(); break;
		case PICCHARDEVICE:		return DFMImg::PicCharDevice(); break;
		case PICSOCKET:			return DFMImg::PicSocket(); break;
		case PICPIPE:			return DFMImg::PicPipe(); break;
		case PICLINK:			return DFMImg::PicLink(); break;
		case PICLINKDIR:		return DFMImg::PicLinkDir(); break;
		case PICLINKFILE:		return DFMImg::PicLinkFile(); break;
		case PICDISK:			return DFMImg::PicDisk(); break;
		case PICPARTITION:		return DFMImg::PicPartition(); break;
		case PICSWAPPARTITION:	return DFMImg::PicSwapPartition(); break;
		case PICUSB:			return DFMImg::PicUSB(); break;
		case PICCDDVD:			return DFMImg::PicCDDVD(); break;
		case PICDIR:			return DFMImg::PicDir(); break;
		case PICNEWDIR:			return DFMImg::PicNewDir(); break;
		case PICHOME:			return DFMImg::PicHome(); break;
		case PICFILE:			return DFMImg::PicFile(); break;
		//case PICFILERUN:		return DFMImg::PicFileRun(); break;
		case PICTEXT:			return DFMImg::PicText(); break;
		case PICSCRIPT:			return DFMImg::PicScript(); break;
		case PICUNKNOWN:		return DFMImg::PicUnknown(); break;
		case PICBINARYFILE:		return DFMImg::PicBinaryFile(); break;
		case PICEXECUTABLE:		return DFMImg::PicExecutable(); break;
		case PICELF:			return DFMImg::PicELF(); break;
		case PICMASKEXEC:		return DFMImg::PicMaskExec(); break;
		case PICAUDIO:			return DFMImg::PicAudio(); break;
		case PICVIDEO:			return DFMImg::PicVideo(); break;
		case PICGRAPHIC:		return DFMImg::PicGraphic(); break;
		case PICDOC:			return DFMImg::PicDoc(); break;
		case PICPDF:			return DFMImg::PicPDF(); break;
		case PICCODE:			return DFMImg::PicCode(); break;
		case PICWEB:			return DFMImg::PicWEB(); break;
		case PICENCRYPTED:		return DFMImg::PicEncrypted(); break;
		//case PICARCHIVE:		return DFMImg::PicArchive(); break;
		case PICCOMPRESSED:		return DFMImg::PicCompressed(); break;
		case PICDATA:			return DFMImg::PicData(); break;
		case PICXML:			return DFMImg::PicData(); break; //for now
		case PICFIND:			return DFMImg::PicFind(); break;
		case PICDIFF:			return DFMImg::PicDiff(); break;
		case PICUPARROW:		return DFMImg::PicUpArrow(); break;
		case PICRIGHTARROW:		return DFMImg::PicRightArrow(); break;
		case PICLEFTARROW:		return DFMImg::PicLeftArrow(); break;
		case PICREFRESH:		return DFMImg::PicRefresh(); break;
		case PICGO:				return DFMImg::PicGo(); break;
		case PICNODESTOP:		return DFMImg::PicNodesTop(); break;
		case PICNODESBOT:		return DFMImg::PicNodesBot(); break;
		case PICNODESNONE:		return DFMImg::PicNodesNone(); break;
	}
	return DFMImg::PicPlaceHolder(); //DFMImg::PicUnknown();
}

Image GetFDPic(std::string sfd, DFM *pdfm, bool bIco, bool bImg)
{
	//Mime mime; --- if !pdfm then maybe + use WaitCursor
	//mime.Scan(); ... or make mime global - part of DFMConf?
	Image pic;
	if (isdir(sfd)) pic=GetPic(PICDIR);
	else if (pdfm) pdfm->mime.GetIcon(sfd, pic);
	if (pic.IsEmpty())
	{
		if (ispicture(sfd))
		{
			if (bImg) pic=StreamRaster::LoadFileAny(sfd.c_str());
			if (pic.IsNullInstance()) pic=GetPic(PICGRAPHIC);
		}
		else if (issound(sfd)) pic=GetPic(PICAUDIO);
		else if (isvideo(sfd)) pic=GetPic(PICVIDEO);
		else if (isarchive(sfd)) pic=GetPic(PICCOMPRESSED);
		else if (ispdf(sfd)) pic=GetPic(PICPDF);
		else if (isdocument(sfd)) pic=GetPic(PICDOC);
		else if (isdatabase(sfd)) pic=GetPic(PICDATA);
		else if (issourcecode(sfd)) pic=GetPic(PICCODE);
		else if (iswebfile(sfd)) pic=GetPic(PICWEB);
		else if (canexecute(sfd)) pic=GetPic(PICFILERUN);
		else pic=GetPic(PICFILE);
	}
	if (bIco) elastic_resize(pic, 20, 20);
	return pic;
}

void er_contract(Image &pic, int W)
{
	if (pic.IsNullInstance()||(W<=0)) return;
	int x, y, i, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(W, h);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (y=0;y<h;y++)
	{
		for (x=0;x<w;x++)
		{
			i=(x*W)/w;
			pib[(y*W)+i]=pic[y][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_extend(Image &pic, int W) //widening pic
{
	if (pic.IsNullInstance()||(W<=0)) return;
	int x, i, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(W, h);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (j=0;j<h;j++)
	{
		for (i=0;i<W;i++)
		{
			x=(i*w)/W;
			pib[(j*W)+i]=pic[j][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_shorten(Image &pic, int H) //shortening pic
{
	if (pic.IsNullInstance()||(H<=0)) return;
	int x, y, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(w, H);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (y=0;y<h;y++) //some source-pixels will be go to same target-pixels (many src to 1 tgt)
	{
		for (x=0;x<w;x++)
		{
			j=(y*H)/h;
			pib[(j*w)+x]=pic[y][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_highten(Image &pic, int H) //lengthening pic
{
	if (pic.IsNullInstance()||(H<=0)) return;
	int y, i, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(w, H);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (j=0;j<H;j++) //some target-pixels will be from same source-pixels (1 src to many tgt)
	{
		for (i=0;i<w;i++)
		{
			y=(j*h)/H;
			pib[(j*w)+i]=pic[y][i];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_shrink(Image &pic, int W, int H)
{
	if (pic.IsNullInstance()||(W<=0)||(H<=0)) return;
	int x, y, i, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(W, H);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (y=0;y<h;y++) //some source-pixels will be go to same target-pixels (many src to 1 tgt)
	{
		for (x=0;x<w;x++)
		{
			i=(x*W)/w;
			j=(y*H)/h;
			pib[(j*W)+i]=pic[y][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void er_grow(Image &pic, int W, int H)
{
	if (pic.IsNullInstance()||(W<=0)||(H<=0)) return;
	int x, y, i, j, w=pic.GetWidth(), h=pic.GetHeight();
	ImageBuffer ib(W, H);
	Fill(~ib, RGBAZero(), ib.GetLength());
	RGBA *pib=~ib;
	for (j=0;j<H;j++)
	{
		for (i=0;i<W;i++)
		{
			x=(i*w)/W;
			y=(j*h)/H;
			pib[(j*W)+i]=pic[y][x];
		}
	}
	pic.Clear();
	pic=ib;
}

void elastic_resize(Image &pic, int w, int h)
{
	//idea from: [dev_hold]tapov::void TAMap::PaintPic(Draw &drw, TATopic &T, int CX, int CY)
	//gives even spreading-out / stretching / squashing of picture - not reversible
	//((can make a class that retains copy of original pic then it can be sized at will))

	/*
		for each pixel of target, fetch corresponding pixel from source
		enlarging => same source pixel can fill several target positions
		shrinking => some source pixels will be lost
	*/

	int pw=pic.GetWidth();
	int ph=pic.GetHeight();
	if ((pw==w)&&(ph==h)) return;
	if (pw<w)
	{
		if (ph<h) er_grow(pic, w, h);
		else if (ph==h) er_extend(pic, w);
		else { er_extend(pic, w); er_shorten(pic, h); }
	}
	else if (pw==w)
	{
		if (ph<h) er_highten(pic, h);
		else if (ph>h) er_shorten(pic, h);
	}
	else //pw>w
	{
		if (ph<h) { er_highten(pic, h); er_contract(pic, w); }
		else if (ph==h) er_contract(pic, w);
		else er_shrink(pic, w, h);
	}
}


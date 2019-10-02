#ifndef _picbutton_h_
#define _picbutton_h_


#include <CtrlLib/CtrlLib.h>

using namespace Upp;

//#define LAYOUTFILE <sfm/picbut.lay>
//#include <CtrlCore/lay.h>

#include <string>

struct PicButton : public Ctrl
{
	typedef PicButton CLASSNAME;

	Image pic;
	std::string sData;
	bool ldn, bsel;
	
	PicButton();
	PicButton(const PicButton &PB) { pic=PB.pic; SetRect(PB.GetRect()); sData=""; ldn=false; }
	PicButton(const Image &img);
	
	PicButton& operator=(const PicButton &PB) { pic=PB.pic; SetRect(PB.GetRect()); sData=PB.sData; ldn=false; return *this; }
	
	PicButton& SetPic(const Image &img);
	PicButton& SetSData(std::string sdata);
	
	void Paint(Draw& w);
	
	const std::string Data() { return sData; }
	void Data(const std::string &data) { sData=data; }
	
	void Select(bool b=true);

	virtual void MouseEnter(Point p, dword keyflags);
	virtual void MouseLeave();
	virtual void LeftDown(Point p, dword keyflags);
	virtual void LeftUp(Point p, dword keyflags);

	Event<PicButton*> WhenClick;
};





#endif

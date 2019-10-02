
#include "picbutton.h"


PicButton::PicButton() { SetRect(0,0,24,28); sData=""; ldn=bsel=false; }
PicButton::PicButton(const Image &img) { pic=img; SetRect(pic.GetSize()); }

PicButton& PicButton::SetPic(const Image &img)
{
	pic=img;
	Refresh();
	return *this;
}

PicButton& PicButton::SetSData(std::string sdata) { sData=sdata; return *this; }

void PicButton::Paint(Draw& w)
{
	Size sz = GetSize();
	Size psz = pic.GetSize();
    w.DrawRect(sz, SColorFace());
    Color lt=White(), br=Gray();
    Image img=pic;
    
	if (ldn) { lt=Gray(); br=White(); }
	else if (bsel) { lt=br=Black(); }

	if (!IsEnabled())
	{
		ImageBuffer ib(img);
		int H=pic.GetHeight();
		int W=pic.GetWidth();
		for (int y=0; y<H; y++)
		{
			RGBA *p=ib[y];
			for (int x=0; x<W; x++)
			{
				if (!y||!x||(y==(H-1))||(x==(W-1))) p[x]=Gray();
				else if (((x+y)%2)==0) p[x]=LtGray();
				//else if (((y%2)==0)&&((x%2)==0)) p[x]=LtGray();
				//p[x].a*=100;
				//p[x].r*=2;
				//p[x].g*=2;
				//p[x].b*=2;
			}
		}
		img=ib;
	}

	w.DrawImage((sz.cx-psz.cx)/2, (sz.cy-psz.cy)/2, img);
	w.DrawLine(0, 0, sz.cx-1, 0, 1, lt);
	w.DrawLine(0, 0, 0, sz.cy-1, 1, lt);
	w.DrawLine(0, sz.cy-1, sz.cx-1, sz.cy-1, 1, br);
	w.DrawLine(sz.cx-1, 0, sz.cx-1, sz.cy-1, 1, br);

}

void PicButton::Select(bool b) { bsel=b; Refresh(); }
void PicButton::LeftDown(Point p, dword keyflags) { ldn=true; Refresh(); }
void PicButton::LeftUp(Point p, dword keyflags) { ldn=false; Refresh(); WhenClick(this); }
void PicButton::MouseEnter(Point p, dword keyflags) { /*...*/ }
void PicButton::MouseLeave() { ldn=false; Refresh(); }


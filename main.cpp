#include "BMP.h"
int main()
{
	BMP sample;
	sample.SetWidth(640);
	sample.SetWidth(480);
	for(int py=0; py<480; py++)
	{
		for(int px=0; px<640; px++)
		{
			PIXEL p;
			p.R = 128;
			p.G = 128;
			p.B = 128;
			sample.SetPixel(px, py, p);
		}
	}
	sample.WriteBMPFile("./output/sample.bmp");
	return 0;
}
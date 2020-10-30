#ifndef BMP_H
#define BMP_H

#include "BMP_TypeDef.h"
#include <string>

#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif
using namespace std;

extern const double RGB2YUVCoef_709[3][3];
extern const double YUV2RGBCoef_709[3][3];
extern const double RGB2YUVCoef_601[3][3];
extern const double YUV2RGBCoef_601[3][3];

class BMP {
// MEMBER VARIABLE
public :
	// STANDARD SEL
	static const unsigned int BT601 = 0;
	static const unsigned int BT709 = 1;
	// OPERATE VECTOR SEL
	static const unsigned int VECTOR_RGB = 0;
	static const unsigned int VECTOR_YUV = 1;
	// DIMENSION SEL
	static const unsigned int DIR_H = 0;
	static const unsigned int DIR_V = 1;

private :
	BMP_HEADER* Header    = NULL;
	PIXEL**     RGBVector = NULL;
	YUV_PIXEL** YUVVector = NULL;
	HSI_PIXEL** HSIVector = NULL;
	HISTOGRAM*  Histogram = NULL;

// MEMBER FUNCTION
public :
	BMP();
	BMP(string);
	~BMP();
	bool ReadBMPFile();
	bool ReadBMPFile(string);
	bool WriteBMPFile();
	bool WriteBMPFile(string);
	bool SetPixel(const unsigned int, const unsigned int, PIXEL&);
	bool SetPixel(const unsigned int, const unsigned int, YUV_PIXEL&);
	bool GenYHistogram(const unsigned int, const unsigned int, const unsigned int, const unsigned int);
	bool GetPixel(const unsigned int, const unsigned int, PIXEL&);
	bool GetPixel(const unsigned int, const unsigned int, YUV_PIXEL&);
	bool GetPixel(const unsigned int, const unsigned int, HSI_PIXEL&);
	bool RGB2YUV_OverWrite(const unsigned int);
	bool YUV2RGB_OverWrite(const unsigned int);
	bool RGB2HSI_OverWrite();
	bool HSI2RGB_OverWrite();
	bool CutHorizontalEdge(const unsigned int, const unsigned int);
	bool CutVerticalEdge(const unsigned int, const unsigned int);
	UINT32 GetYHistogram(const unsigned int);
	UINT32 GetWidth();
	UINT32 GetHeight();
	bool SetWidth(const unsigned int);
	bool SetHeight(const unsigned int);
	BMP_HEADER GetHeader();
	void PrintInfo2Stream(ostream&);
	bool Filter(const unsigned int*, const unsigned int, const unsigned int, const unsigned int);
	void DuplicateField(bool);
	void UpsideDown();
	void Mirror();
	void CutByRegion(unsigned int , unsigned int, unsigned int, unsigned int);
	void CutByIdx(unsigned int , unsigned int, unsigned int, unsigned int);
	void ScaleUp_Duplicate(unsigned int);
	void PQ_HPF();

private :
	bool ReadBMPData(string);
	bool WriteBMPData(string);
	void DeleteUserData();
	void ResetUserData();
	void Put1Byte(ofstream&, UINT8);
	void Put2Byte(ofstream&, UINT16);
	void Put3Byte(ofstream&, UINT32);
	void Put4Byte(ofstream&, UINT32);
};

#endif
#include "BMP.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>

const double RGB2YUVCoef_709[3][3] =
{
    {0.2126,0.7152,0.0722},
    {-0.09991,-0.33609,0.436},
    {0.615,-0.55861,-0.05639}
};
const double YUV2RGBCoef_709[3][3] =
{
    {1,0,1.28033},
    {1,-0.21482,-0.38059},
    {1,2.12798,0}
};

const double RGB2YUVCoef_601[3][3] =
{
    {0.299,0.587,0.114},
    {-0.14713,-0.28886,0.436},
    {0.615,-0.51499,-0.10001}
};
const double YUV2RGBCoef_601[3][3] =
{
    {1, 0, 1.13983},
    {1,-0.39465,-0.58060},
    {1,2.03211,0}
};

#define SIZE_PTR sizeof(void*)
#define UNIT_BYTE_TYPE char
// Reference : http://smalldd.pixnet.net/blog/post/10953132-c%2B%2B-%E5%8B%95%E6%85%8B%E9%85%8D%E7%BD%AE%E4%BA%8C%E7%B6%AD%E9%99%A3%E5%88%97
#define NEW2D(Dim1, Dim2, TYPE) (TYPE**)GeneralNew2D( Dim1, Dim2, sizeof(TYPE) )
void** GeneralNew2D(const unsigned int Dim1, const unsigned int Dim2, const unsigned int ElementSize)
{
    void **p = (void**) new UNIT_BYTE_TYPE [ Dim1*SIZE_PTR + Dim1*Dim2*ElementSize ];
    for( int i=0; i<Dim1; i++ )
    {
        p[i] = (void*) ( (char*)p+Dim1*SIZE_PTR + i*Dim2*ElementSize );
    }
    // INITIALIZATION
    memset((char*)p+Dim1*SIZE_PTR, 0, Dim1*Dim2*ElementSize);
    return p;
}

#define NEW3D(Dim1, Dim2, Dim3, TYPE) (TYPE***)GeneralNew3D( Dim1, Dim2, Dim3, sizeof(TYPE) )
void*** GeneralNew3D(const unsigned int Dim1, const unsigned int Dim2, const unsigned int Dim3, const unsigned int ElementSize)
{
    void ***p = (void***) new UNIT_BYTE_TYPE [ Dim1*SIZE_PTR + Dim1*Dim2*SIZE_PTR + Dim1*Dim2*Dim3*ElementSize ];
    for( int i=0; i<Dim1; i++ )
    {
        //                BASE                     OFFSET
        p[i] = (void**) ( (char*)p+Dim1*SIZE_PTR + i*Dim2*SIZE_PTR );
        for( int j=0; j<Dim2; j++ )
        {
            //                  BASE                                        OFFSET
            p[i][j] = (void*) ( (char*)p+Dim1*SIZE_PTR+Dim1*Dim2*SIZE_PTR + (i*Dim2+j)*(Dim3*ElementSize) );
        }
    }
    // INITIALIZATION
    memset((char*)p+Dim1*SIZE_PTR+Dim1*Dim2*SIZE_PTR, 0, Dim1*Dim2*Dim3*ElementSize);
    return p;
}

#define FILE_NAME_MAX_SIZE  256
#define PI                  3.14159265359
#define R_RAD2DEGREE        180/PI
#define R_DEGREE2RAD        PI/180
#define DEBUG_INFO          0             // PRINT THE FUNCTION FLOW TO DEBUG
using namespace std;

// operation overloading
ostream& operator<<(ostream& os, const PIXEL& dat)
{
  os << "R:[" << dat.R << "] G:[" << dat.G << "] B:[" << dat.B << "]";
  return os;
}

ostream& operator<<(ostream& os, const YUV_PIXEL& dat)
{
  os << "Y:[" << dat.Y << "] U:[" << dat.U << "] V:[" << dat.V << "]";
  return os;
}

ostream& operator<<(ostream& os, const HSI_PIXEL& dat)
{
  os << "H:[" << dat.H << "] S:[" << dat.S << "] I:[" << dat.I << "]";
  return os;
}

ostream& operator<<(ostream& os, const INFO_HEADER& dat)
{
	os << "=====[ INFO HEADER ]=====" << endl;
	os << "BitmapHeaderSize = " <<      (unsigned int)dat.BitmapHeaderSize << endl;
	os << "Width = " <<                 (unsigned int)dat.Width << endl;
	os << "Height = " <<                (int)         dat.Height << endl;
	os << "Planes = " <<                (unsigned int)dat.Planes << endl;
	os << "BitPerPixel = " <<           (unsigned int)dat.BitPerPixel << endl;
	os << "Compression = " <<           (unsigned int)dat.Compression << endl;
	os << "BitmapDataSize = " <<        (unsigned int)dat.BitmapDataSize << endl;
	os << "HorizontalResolution = " <<  (unsigned int)dat.HorizontalResolution << endl;
	os << "VerticalResolution = " <<    (unsigned int)dat.VerticalResolution << endl;
	os << "UsedColors = " <<            (unsigned int)dat.UsedColors << endl;
	os << "ImportanColors = " <<        (unsigned int)dat.ImportanColors;
	return os;
}

ostream& operator<<(ostream& os, const FILE_HEADER& dat)
{
	os << "=====[ FILE HEADER ]=====" << endl;
	os << "Identifier = " <<        (unsigned int)dat.Identifier << endl;
	os << "FileSize = " <<          (unsigned int)dat.FileSize << endl;
	os << "Reserved = " <<          (unsigned int)dat.Reserved << endl;
	os << "BitmapDataOffset = " <<  (unsigned int)dat.BitmapDataOffset;
	return os;
}

ostream& operator<<(ostream& os, const BMP_HEADER& dat)
{
	os << (*dat.FileHeader) << endl << (*dat.InfoHeader);
	return os;
}

ostream& operator<<(ostream& os, BMP_HEADER& dat)
{
	os << *(dat.FileHeader) << endl << *(dat.InfoHeader);
	return os;
}

ostream& operator<<(ostream& os, BMP& dat)
{
	dat.PrintInfo2Stream(os);
	return os;
}

FILE_HEADER& FILE_HEADER::operator=( const FILE_HEADER& rhs )
{
	this->Identifier = rhs.BitmapDataOffset;
	this->FileSize = rhs.FileSize;
	this->Reserved = rhs.Reserved;
	this->BitmapDataOffset = rhs.BitmapDataOffset;
	return *this;
}

INFO_HEADER& INFO_HEADER::operator=( const INFO_HEADER& rhs)
{
	this->BitmapHeaderSize = rhs.BitmapHeaderSize;
	this->Width = rhs.Width;
	this->Height = rhs.Height;
	this->Planes = rhs.Planes;
	this->BitPerPixel = rhs.BitPerPixel;
	this->Compression = rhs.Compression;
	this->BitmapDataSize = rhs.BitmapDataSize;
	this->HorizontalResolution = rhs.HorizontalResolution;
	this->VerticalResolution = rhs.VerticalResolution;
	this->UsedColors = rhs.UsedColors;
	this->ImportanColors = rhs.ImportanColors;
	return *this;
}

BMP_HEADER& BMP_HEADER::operator=( const BMP_HEADER& rhs)
{
	*(this->FileHeader) = *(rhs.FileHeader);
	*(this->InfoHeader) = *(rhs.InfoHeader);
	return *this;
}

PIXEL& PIXEL::operator=( const PIXEL& rhs )
{
	this->R = rhs.R;
	this->G = rhs.G;
	this->B = rhs.B;
	this->Res = rhs.Res;
	return *this;
}

YUV_PIXEL& YUV_PIXEL::operator=( const YUV_PIXEL& rhs )
{
	this->Y = rhs.Y;
	this->U = rhs.U;
	this->V = rhs.V;
	this->Res = rhs.Res;
	return *this;
}

// PUBLIC FUNCTION
BMP::BMP()
{
    Header = new BMP_HEADER;
    Header->FileHeader = new FILE_HEADER;
    Header->InfoHeader = new INFO_HEADER;
    RGBVector = NEW2D(Header->InfoHeader->Width, Header->InfoHeader->Height, PIXEL);
    YUVVector = NEW2D(Header->InfoHeader->Width, Header->InfoHeader->Height, YUV_PIXEL);
}

BMP::BMP(string FileName)
{
    ReadBMPFile(FileName);
}

BMP::~BMP()
{
    DeleteUserData();
    delete Header->FileHeader;
    delete Header->InfoHeader;
    delete Header;
}

bool BMP::ReadBMPFile()
{
    string FileName;
    cout << "Please enter file name : ";
    cin >> FileName;
    return ReadBMPData(FileName);
}

bool BMP::ReadBMPFile(string FileName)
{
    return ReadBMPData(FileName);
}

bool BMP::WriteBMPFile()
{
    string FileName;
    cout << "Please enter output file name : ";
    cin >> FileName;
    WriteBMPData(FileName);
}

bool BMP::WriteBMPFile(string FileName)
{
    return WriteBMPData(FileName);
}

bool BMP::SetPixel(const unsigned int x, const unsigned int y, YUV_PIXEL& p)
{
    if( YUVVector == NULL ) return false;
    else if( x>=Header->InfoHeader->Width || y>=Header->InfoHeader->Height ) return false;
    else
    {
        YUVVector[x][y].Y = p.Y;
        YUVVector[x][y].U = p.U;
        YUVVector[x][y].V = p.V;
        return true;
    }
}

bool BMP::SetPixel(const unsigned int x, const unsigned int y, PIXEL& p)
{
    if( RGBVector == NULL ) return false;
    else if( x>=Header->InfoHeader->Width || y>=Header->InfoHeader->Height ) return false;
    else
    {
        RGBVector[x][y].ByteValue = p.ByteValue;
        return true;
    }
}

bool BMP::GetPixel(const unsigned int x, const unsigned int y, PIXEL& p)
{
    if( RGBVector == NULL ) return false;
    else if( x>=Header->InfoHeader->Width || y>=Header->InfoHeader->Height ) return false;
    else
    {
        p.R = RGBVector[x][y].R;
        p.G = RGBVector[x][y].G;
        p.B = RGBVector[x][y].B;
        p.Res = RGBVector[x][y].Res;
        return true;
    }
}

bool BMP::GetPixel(const unsigned int x, const unsigned int y, YUV_PIXEL& p)
{
    if( YUVVector == NULL ) return false;
    else if( x>=Header->InfoHeader->Width || y>=Header->InfoHeader->Height ) return false;
    else
    {
        p.Y = YUVVector[x][y].Y;
        p.U = YUVVector[x][y].U;
        p.V = YUVVector[x][y].V;
        return true;
    }
}

bool BMP::GetPixel(const unsigned int x, const unsigned int y, HSI_PIXEL& p)
{
    if( HSIVector == NULL ) return false;
    else if( x>=Header->InfoHeader->Width || y>=Header->InfoHeader->Height ) return false;
    else
    {
        p.H = HSIVector[x][y].H;
        p.S = HSIVector[x][y].S;
        p.I = HSIVector[x][y].I;
        return true;
    }
}

bool BMP::YUV2RGB_OverWrite(const unsigned int Standard)
{
    if( YUVVector == NULL )
    {
        cout << "Please convert RGB data to YUV first" << endl;
        return false;
    }

    if(DEBUG_INFO) cout << "Over write RGB value by RGB from YUV" << endl;

    double ConvertTable[3][3] = {0};
    for( int i=0; i<3; i++ )
    {
        for( int j=0; j<3; j++ )
        {
            if( Standard == BT709 )      ConvertTable[i][j] = YUV2RGBCoef_709[i][j];
            else if( Standard == BT601 ) ConvertTable[i][j] = YUV2RGBCoef_601[i][j];
        }
    }

    for( int y=0; y<GetHeight(); y++ )
    {
        for( int x=0; x<GetWidth(); x++ )
        {
            double TmpY = YUVVector[x][y].Y;
            double TmpU = YUVVector[x][y].U;
            double TmpV = YUVVector[x][y].V;

            double TmpR = TmpY*ConvertTable[0][0] + (TmpU-128)*ConvertTable[0][1] + (TmpV-128)*ConvertTable[0][2];
            double TmpG = TmpY*ConvertTable[1][0] + (TmpU-128)*ConvertTable[1][1] + (TmpV-128)*ConvertTable[1][2];
            double TmpB = TmpY*ConvertTable[2][0] + (TmpU-128)*ConvertTable[2][1] + (TmpV-128)*ConvertTable[2][2];

            TmpR = (TmpR > 255)? 255 : ((TmpR < 0)? 0 : TmpR);
            TmpG = (TmpG > 255)? 255 : ((TmpG < 0)? 0 : TmpG);
            TmpB = (TmpB > 255)? 255 : ((TmpB < 0)? 0 : TmpB);

            RGBVector[x][y].R = TmpR;
            RGBVector[x][y].G = TmpG;
            RGBVector[x][y].B = TmpB;
        }
    }
    return true;
}

bool BMP::RGB2YUV_OverWrite(const unsigned int Standard)
{
    if( RGBVector == NULL )
    {
        cout << "Please read BMP file first" << endl;
        return false;
    }

    if(DEBUG_INFO) cout << "Over write YUV value by YUV from RGB" << endl;
    if( YUVVector != NULL ) delete [] YUVVector;
        YUVVector = NEW2D(Header->InfoHeader->Width, Header->InfoHeader->Height, YUV_PIXEL);

    double ConvertTable[3][3] = {0};
    for( int i=0; i<3; i++ )
    {
        for( int j=0; j<3; j++ )
        {
            if( Standard == BT709 )      ConvertTable[i][j] = RGB2YUVCoef_709[i][j];
            else if( Standard == BT601 ) ConvertTable[i][j] = RGB2YUVCoef_601[i][j];
        }
    }

    for( int y=0; y<Header->InfoHeader->Height; y++ )
    {
        for( int x=0; x<Header->InfoHeader->Width; x++ )
        {
            double TmpR = RGBVector[x][y].R;
            double TmpG = RGBVector[x][y].G;
            double TmpB = RGBVector[x][y].B;

            double TmpY = TmpR*ConvertTable[0][0] + TmpG*ConvertTable[0][1] + TmpB*ConvertTable[0][2] + 16;
            double TmpU = TmpR*ConvertTable[1][0] + TmpG*ConvertTable[1][1] + TmpB*ConvertTable[1][2] + 128;
            double TmpV = TmpR*ConvertTable[2][0] + TmpG*ConvertTable[2][1] + TmpB*ConvertTable[2][2] + 128;

            TmpY = (TmpY > 255)? 255 : ((TmpY < 0)? 0 : TmpY);
            TmpU = (TmpU > 255)? 255 : ((TmpU < 0)? 0 : TmpU);
            TmpV = (TmpV > 255)? 255 : ((TmpV < 0)? 0 : TmpV);

            YUVVector[x][y].Y = TmpY;
            YUVVector[x][y].U = TmpU;
            YUVVector[x][y].V = TmpV;
        }
    }
    return true;
}

bool BMP::RGB2HSI_OverWrite()
{
    // FORMULA : http://www.had2know.com/technology/hsi-rgb-color-converter-equations.html
    if( RGBVector == NULL )
    {
        cout << "Please read BMP file first" << endl;
        return false;
    }

    if(DEBUG_INFO) cout << "Over write HSI value by HSI from RGB" << endl;
    if( HSIVector != NULL ) delete [] HSIVector;
        HSIVector = NEW2D(Header->InfoHeader->Width, Header->InfoHeader->Height, HSI_PIXEL);

    for( int y=0; y<Header->InfoHeader->Height; y++)
    {
        for( int x=0; x<Header->InfoHeader->Width; x++ )
        {

            double TmpR = RGBVector[x][y].R;
            double TmpG = RGBVector[x][y].G;
            double TmpB = RGBVector[x][y].B;

            // CONVERT I
            HSIVector[x][y].I = (TmpR+TmpG+TmpB)/3;

            // CONVERT S
            if( HSIVector[x][y].I == 0 ) HSIVector[x][y].S = 0;
            else HSIVector[x][y].S = 1 - ( fmin(TmpR, fmin(TmpG, TmpB)) / HSIVector[x][y].I );

            // CONVERT H
            if( TmpR == TmpG && TmpR == TmpB ) HSIVector[x][y].H = 0;
            else
            {
                double Dividen = TmpR - (TmpG/2) - (TmpB/2);
                double Divisor = sqrt( pow(TmpR ,2) + pow(TmpG ,2) + pow(TmpB ,2) - TmpR*TmpG - TmpR*TmpB - TmpG*TmpB );
                if( TmpG >= TmpB ) HSIVector[x][y].H = R_RAD2DEGREE*acos(Dividen/Divisor);
                else HSIVector[x][y].H = 360 - R_RAD2DEGREE*acos(Dividen/Divisor);
            }

        }
    }

    return true;
}

bool BMP::HSI2RGB_OverWrite()
{
    if( HSIVector == NULL )
    {
        cout << "Please convert RGB data to HSI first" << endl;
        return false;
    }

    for( int y=0; y<Header->InfoHeader->Height; y++ )
    {
        for( int x=0; x<Header->InfoHeader->Width; x++ )
        {

            double TmpR = 255;
            double TmpG = 255;
            double TmpB = 255;

            if( HSIVector[x][y].H == 0 || HSIVector[x][y].H == 360 )
            {
                TmpR = HSIVector[x][y].I + 2*HSIVector[x][y].I*HSIVector[x][y].S;
                TmpG = HSIVector[x][y].I - HSIVector[x][y].I*HSIVector[x][y].S;
                TmpB = HSIVector[x][y].I - HSIVector[x][y].I*HSIVector[x][y].S;
            }
            else if( HSIVector[x][y].H > 0 && HSIVector[x][y].H < 120 )
            {
                TmpR = HSIVector[x][y].I + HSIVector[x][y].I*HSIVector[x][y].S *      cos(HSIVector[x][y].H*R_DEGREE2RAD)/cos((60-HSIVector[x][y].H)*R_DEGREE2RAD);
                TmpG = HSIVector[x][y].I + HSIVector[x][y].I*HSIVector[x][y].S * (1 - cos(HSIVector[x][y].H*R_DEGREE2RAD)/cos((60-HSIVector[x][y].H)*R_DEGREE2RAD));
                TmpB = HSIVector[x][y].I - HSIVector[x][y].I*HSIVector[x][y].S;
            }
            else if( HSIVector[x][y].H == 120 )
            {
                TmpR = HSIVector[x][y].I - HSIVector[x][y].I*HSIVector[x][y].S;
                TmpG = HSIVector[x][y].I + 2*HSIVector[x][y].I*HSIVector[x][y].S;
                TmpB = HSIVector[x][y].I - HSIVector[x][y].I*HSIVector[x][y].S;
            }
            else if( HSIVector[x][y].H > 120 && HSIVector[x][y].H < 240 )
            {
                TmpR = HSIVector[x][y].I - HSIVector[x][y].I*HSIVector[x][y].S;
                TmpG = HSIVector[x][y].I + HSIVector[x][y].I*HSIVector[x][y].S *      cos((HSIVector[x][y].H-120)*R_DEGREE2RAD)/cos((180-HSIVector[x][y].H)*R_DEGREE2RAD);
                TmpB = HSIVector[x][y].I + HSIVector[x][y].I*HSIVector[x][y].S * (1 - cos((HSIVector[x][y].H-120)*R_DEGREE2RAD)/cos((180-HSIVector[x][y].H)*R_DEGREE2RAD));
            }
            else if( HSIVector[x][y].H == 240 )
            {
                TmpR = HSIVector[x][y].I - HSIVector[x][y].I*HSIVector[x][y].S;
                TmpG = HSIVector[x][y].I - HSIVector[x][y].I*HSIVector[x][y].S;
                TmpB = HSIVector[x][y].I + 2*HSIVector[x][y].I*HSIVector[x][y].S;
            }
            else if( HSIVector[x][y].H > 240 && HSIVector[x][y].H < 360 )
            {
                TmpR = HSIVector[x][y].I + HSIVector[x][y].I*HSIVector[x][y].S * (1 - cos((HSIVector[x][y].H-240)*R_DEGREE2RAD)/cos((300-HSIVector[x][y].H)*R_DEGREE2RAD));
                TmpG = HSIVector[x][y].I - HSIVector[x][y].I*HSIVector[x][y].S;
                TmpB = HSIVector[x][y].I + HSIVector[x][y].I*HSIVector[x][y].S *      cos((HSIVector[x][y].H-240)*R_DEGREE2RAD)/cos((300-HSIVector[x][y].H)*R_DEGREE2RAD);
            }
            // CLAMP TO 255
            TmpR = (TmpR > 255)? 255: (TmpR<0)? 0: TmpR;
            TmpG = (TmpG > 255)? 255: (TmpG<0)? 0: TmpG;
            TmpB = (TmpB > 255)? 255: (TmpB<0)? 0: TmpB;

            RGBVector[x][y].R = (UINT8)(TmpR);
            RGBVector[x][y].G = (UINT8)(TmpG);
            RGBVector[x][y].B = (UINT8)(TmpB);

        }
    }

    return true;
}

bool BMP::GenYHistogram(const unsigned int Bin, const unsigned int MinValue, const unsigned int MaxValue, const unsigned int Standard)
{
    if( (Bin & Bin-1) != 0 )
    {
        cout << "Histogram bin num should be 2^x" << endl;
        return false;
    }
    else if( YUVVector == NULL )
    {
        if( RGB2YUV_OverWrite(Standard) == false ) return false;
    }

    if( Histogram != NULL ) delete Histogram;

    Histogram = new HISTOGRAM;
    Histogram->HistogramBin = Bin;
    Histogram->MinValue = MinValue;
    Histogram->MaxValue = MaxValue;
    Histogram->HistogramVector = new UINT32 [Histogram->HistogramBin];
    for( int i=0; i<Histogram->HistogramBin; i++) Histogram->HistogramVector[i] = 0;

    unsigned int IndexShift = 8;
    unsigned int TmpBin = Bin;
    while( TmpBin!=1 )
    {
        TmpBin = TmpBin >> 1;
        IndexShift--;
    }

    for( int y=0; y<Header->InfoHeader->Height; y++ )
    {
        for( int x=0; x<Header->InfoHeader->Width; x++ )
        {
            if( YUVVector[x][y].Y >= Histogram->MinValue && YUVVector[x][y].Y <= Histogram->MaxValue )
            {
                Histogram->HistogramVector[YUVVector[x][y].Y >> IndexShift]++;
            }
        }
    }

    return true;
}

UINT32 BMP::GetYHistogram(const unsigned int Index)
{
    if( Histogram == NULL )
    {
        if(DEBUG_INFO) cout << "Please generate histogram first" << endl;
        return 0;
    }
    else if( Index >= Histogram->HistogramBin )
    {
        if(DEBUG_INFO) cout << "Read histogram index out of range" << endl;
        return 0;
    }
    else return Histogram->HistogramVector[Index];
}

bool BMP::Filter(const unsigned int* FilterVector, const unsigned int FilterSize, const unsigned int VectorSel, const unsigned int Dir)
{
    if( VectorSel == VECTOR_YUV && YUVVector == NULL )
    {
    if(DEBUG_INFO) cout << "Please convert RGB to YUV first" << endl;
    return false;
    }
    else if( Dir != DIR_H && Dir != DIR_V )
    {
    if( DEBUG_INFO ) cout << "Unknow filter direction" << endl;
    return false;
    }

    unsigned int FilterSum = 0;
    for( int i=0; i<FilterSize; i++ ) FilterSum += FilterVector[i];

    if( VectorSel == VECTOR_YUV )
    {


    }
    else if( VectorSel == VECTOR_RGB )
    {
        PIXEL** ProcVector = NEW2D(Header->InfoHeader->Width, Header->InfoHeader->Height, PIXEL);
        for( int y=0; y<Header->InfoHeader->Height; y++ )
        {
            for( int x=0; x<Header->InfoHeader->Width; x++ )
            {
                double Result_R = 0, Result_G = 0, Result_B = 0;
                if( Dir == DIR_H )
                {
                    // Horizontal
                    for( int i=0; i<FilterSize; i++ )
                    {
                        int Idx = x-((int)FilterSize>>1)+i;
                        if( Idx < 0 || Idx >= Header->InfoHeader->Width )
                        {
                            Result_R = Result_R + 0;
                            Result_G = Result_G + 0;
                            Result_B = Result_B + 0;
                        }
                        else
                        {
                            Result_R = Result_R + RGBVector[Idx][y].R*FilterVector[i];
                            Result_G = Result_G + RGBVector[Idx][y].G*FilterVector[i];
                            Result_B = Result_B + RGBVector[Idx][y].B*FilterVector[i];
                        }
                    }
                    ProcVector[x][y].R = (UINT8)(Result_R/FilterSum + 0.5);
                    ProcVector[x][y].G = (UINT8)(Result_G/FilterSum + 0.5);
                    ProcVector[x][y].B = (UINT8)(Result_B/FilterSum + 0.5);
                }
                else if( Dir == DIR_V )
                {
                    // Vertical
                    for( int i=0; i<FilterSize; i++ )
                    {
                        int Idx = y-((int)FilterSize>>1)+i;
                        if( Idx < 0 || Idx >= Header->InfoHeader->Height )
                        {
                            Result_R = Result_R + 0;
                            Result_G = Result_G + 0;
                            Result_B = Result_B + 0;
                        }
                        else
                        {

                            Result_R = Result_R + RGBVector[x][Idx].R*FilterVector[i];
                            Result_G = Result_G + RGBVector[x][Idx].G*FilterVector[i];
                            Result_B = Result_B + RGBVector[x][Idx].B*FilterVector[i];
                        }
                    }
                    ProcVector[x][y].R = (UINT8)(Result_R/FilterSum + 0.5);
                    ProcVector[x][y].G = (UINT8)(Result_G/FilterSum + 0.5);
                    ProcVector[x][y].B = (UINT8)(Result_B/FilterSum + 0.5);
                }
            }
        }

        memcpy( (void*)&RGBVector[0][0], (void*)&ProcVector[0][0], sizeof(PIXEL)*Header->InfoHeader->Width*Header->InfoHeader->Height );
        delete [] ProcVector;
    }
    else
    {
        cout << "Unknow vector selection" << endl;
        return false;
    }

    return true;
}

void BMP::DuplicateField(bool isTopField)
{

    for( unsigned int y=0; y<Header->InfoHeader->Height; y=y+2 )
    {
        for( unsigned int x=0; x<Header->InfoHeader->Width; x++ )
        {
            if( isTopField )
            {
                RGBVector[x][y+1].R = RGBVector[x][y].R;
                RGBVector[x][y+1].G = RGBVector[x][y].G;
                RGBVector[x][y+1].B = RGBVector[x][y].B;
            }
            else
            {
                RGBVector[x][y].R = RGBVector[x][y+1].R;
                RGBVector[x][y].G = RGBVector[x][y+1].G;
                RGBVector[x][y].B = RGBVector[x][y+1].B;
            }
        }
    }

}

void BMP::UpsideDown()
{
    PIXEL** RGBBuffer = NEW2D(Header->InfoHeader->Width, Header->InfoHeader->Height, PIXEL);
    for( unsigned int y=0; y<Header->InfoHeader->Height; y++ )
    {
        for( unsigned int x=0; x<Header->InfoHeader->Width; x++ )
        {
            RGBBuffer[x][y] = RGBVector[x][Header->InfoHeader->Height-y-1];
        }
    }
    for( unsigned int y=0; y<Header->InfoHeader->Height; y++ )
    {
        for( unsigned int x=0; x<Header->InfoHeader->Width; x++ )
        {
            RGBVector[x][y] = RGBBuffer[x][y];
        }
    }
    delete [] RGBBuffer;
}

void BMP::Mirror()
{
    PIXEL** RGBBuffer = NEW2D(Header->InfoHeader->Width, Header->InfoHeader->Height, PIXEL);
    for( unsigned int y=0; y<Header->InfoHeader->Height; y++ )
    {
        for( unsigned int x=0; x<Header->InfoHeader->Width; x++ )
        {
            RGBBuffer[x][y] = RGBVector[Header->InfoHeader->Width-x-1][y];
        }
    }
    for( unsigned int y=0; y<Header->InfoHeader->Height; y++ )
    {
        for( unsigned int x=0; x<Header->InfoHeader->Width; x++ )
        {
            RGBVector[x][y] = RGBBuffer[x][y];
        }
    }
    delete [] RGBBuffer;
}

bool BMP::SetHeight(const unsigned int InputHeight)
{
    if( InputHeight > Header->InfoHeader->Height )
    {
        PIXEL** NewRGBVector = NEW2D(Header->InfoHeader->Width, InputHeight, PIXEL);
        for( unsigned int y=0; y<Header->InfoHeader->Height; y++ )
        {
            for( unsigned int x=0; x<Header->InfoHeader->Width; x++ )
            {
                NewRGBVector[x][y] = RGBVector[x][y];
            }
        }
        delete [] RGBVector;
        RGBVector = NewRGBVector;

        YUV_PIXEL** NewYUVVector = NEW2D(Header->InfoHeader->Width, InputHeight, YUV_PIXEL);
        for( unsigned int y=0; y<Header->InfoHeader->Height; y++ )
        {
            for( unsigned int x=0; x<Header->InfoHeader->Width; x++ )
            {
                NewYUVVector[x][y] = YUVVector[x][y];
            }
        }
        delete [] YUVVector;
        YUVVector = NewYUVVector;
    }

    Header->InfoHeader->Height = InputHeight;
}

bool BMP::SetWidth(const unsigned int InputWidth)
{
    if( InputWidth > Header->InfoHeader->Width )
    {
        PIXEL** NewRGBVector = NEW2D(InputWidth, Header->InfoHeader->Height, PIXEL);
        for( unsigned int y=0; y<Header->InfoHeader->Height; y++ )
        {
            for( unsigned int x=0; x<Header->InfoHeader->Width; x++ )
            {
                NewRGBVector[x][y] = RGBVector[x][y];
            }
        }
        delete [] RGBVector;
        RGBVector = NewRGBVector;

        YUV_PIXEL** NewYUVVector = NEW2D(InputWidth, Header->InfoHeader->Height, YUV_PIXEL);
        for( unsigned int y=0; y<Header->InfoHeader->Height; y++ )
        {
            for( unsigned int x=0; x<Header->InfoHeader->Width; x++ )
            {
                NewYUVVector[x][y] = YUVVector[x][y];
            }
        }
        delete [] YUVVector;
        YUVVector = NewYUVVector;
    }

    Header->InfoHeader->Width = InputWidth;
}

void BMP::CutByRegion(const unsigned int x_sta, const unsigned int width, const unsigned int y_sta, const unsigned int height)
{
    if( x_sta+width > Header->InfoHeader->Width )
    {
        cout << "Error, Cut region too wide" << endl;
        return;
    }
    if( y_sta+height > Header->InfoHeader->Height )
    {
        cout << "Error, Cut region too high" << endl;
        return;
    }
    printf("Cutting Image from [%d][%d] to [%d][%d]\n", x_sta, y_sta, x_sta+width-1, y_sta+height-1);
    for(unsigned int y=y_sta; y<y_sta+height; y++)
    {
        for(unsigned int x=x_sta; x<x_sta+width; x++)
        {
            RGBVector[x-x_sta][y-y_sta] = RGBVector[x][y];
        }
    }
    SetHeight(height);
    SetWidth(width);
}

void BMP::CutByIdx(const unsigned int x_sta, const unsigned int x_end, const unsigned int y_sta, const unsigned int y_end)
{
    if( x_sta > Header->InfoHeader->Width  || x_end > Header->InfoHeader->Width  ||
        y_sta > Header->InfoHeader->Height || y_end > Header->InfoHeader->Height ||
        x_sta > x_end || y_sta > y_end )
    {
        cout << "Error, Cut index setting error" << endl;
        return;
    }
    printf("Cutting Image from [%d][%d] to [%d][%d]\n", x_sta, y_sta, x_end, y_end);
    for(unsigned int y=y_sta; y<=y_end; y++)
    {
        for(unsigned int x=x_sta; x<=x_end; x++)
        {
            RGBVector[x-x_sta][y-y_sta] = RGBVector[x][y];
        }
    }
    SetHeight(y_end-y_sta+1);
    SetWidth(x_end-x_sta+1);
}

void BMP::ScaleUp_Duplicate(unsigned int Ratio)
{
    unsigned int OriWidth = Header->InfoHeader->Width;
    unsigned int OriHeight = Header->InfoHeader->Height;

    SetWidth(Header->InfoHeader->Width*Ratio);
    SetHeight(Header->InfoHeader->Height*Ratio);

    for(int y=Header->InfoHeader->Height-1; y>=0; y--)
    {
        for(int x=Header->InfoHeader->Width-1; x>=0; x--)
        {
            RGBVector[x][y] = RGBVector[x/Ratio][y/Ratio];
        }
    }

}

void BMP::PQ_HPF(){}
UINT32 BMP::GetHeight(){ return Header->InfoHeader->Height; }
UINT32 BMP::GetWidth(){ return Header->InfoHeader->Width; }
BMP_HEADER BMP::GetHeader(){ return (*Header); }
void BMP::PrintInfo2Stream(std::ostream& os){ os << (*Header); }

// PRIVATE FUNCTION
bool BMP::ReadBMPData(string StrInputFile)
{
    ResetUserData();
    if(DEBUG_INFO) cout << "Reading file " << StrInputFile << endl << endl;

    if( Header!=NULL )
    {
        delete Header->FileHeader;
        delete Header->InfoHeader;
        delete Header;
    }
    Header = new BMP_HEADER;
    Header->FileHeader = new FILE_HEADER;
    Header->InfoHeader = new INFO_HEADER;

    char InputFile[FILE_NAME_MAX_SIZE];
    strcpy(InputFile, StrInputFile.c_str());
    ifstream InputStream(InputFile, fstream::binary);
    if( !InputStream.good() )
    {
        cout << "Open file [" << StrInputFile << "] fail" << endl;
        return false;
    }

    UINT32 InputBuffer;
    //=========================================================================
    // READ FILE HEADER
    InputBuffer = 0;
    for( int i=0; i<2; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->FileHeader->Identifier = InputBuffer >> 16;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->FileHeader->FileSize = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->FileHeader->Reserved = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->FileHeader->BitmapDataOffset = InputBuffer;
    //=========================================================================

    //=========================================================================
    // READ INFO HEADER
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->BitmapHeaderSize = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->Width = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->Height = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<2; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->Planes = InputBuffer >> 16;
    InputBuffer = 0;
    for( int i=0; i<2; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->BitPerPixel = InputBuffer >> 16;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->Compression = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->BitmapDataSize = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->HorizontalResolution = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->VerticalResolution = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->UsedColors = InputBuffer;
    InputBuffer = 0;
    for( int i=0; i<4; i++ )  InputBuffer = (InputBuffer >> 8) + (InputStream.get() << 24);
    Header->InfoHeader->ImportanColors = InputBuffer;
    //=========================================================================


    //=========================================================================
    // ALLOCATE AND READ THE RGB DATA
    InputStream.seekg(Header->FileHeader->BitmapDataOffset, InputStream.beg);
    if( Header->InfoHeader->BitPerPixel == 24 )
    {
        RGBVector = NEW2D(Header->InfoHeader->Width, Header->InfoHeader->Height, PIXEL);

        // LET RGBVector[0][0] BE THE LEFT TOP PIXEL TO FIT THE HUMAN NATURE
        for( int y=Header->InfoHeader->Height-1; y>=0; y-- )
        {
            for( int x=0; x<Header->InfoHeader->Width; x++ )
            {
                RGBVector[x][y].B = InputStream.get();
                RGBVector[x][y].G = InputStream.get();
                RGBVector[x][y].R = InputStream.get();
                RGBVector[x][y].Res = 0;
            }
            for( int k=0; k<(4-(Header->InfoHeader->Width*3)%4)%4; k++ ) InputStream.get();
        }
    }
    //=========================================================================

    InputStream.close();
    return true;

}

bool BMP::WriteBMPData(string StrOutputFile)
{
    if(DEBUG_INFO) cout << "Writing file " << StrOutputFile << endl << endl;

    char OutputFile[FILE_NAME_MAX_SIZE];
    strcpy(OutputFile, StrOutputFile.c_str());
    ofstream OutputStream(OutputFile, fstream::binary);
    if( !OutputStream.good() )
    {
        cout << "Write file [" << StrOutputFile << "] fail" << endl;
        return false;
    }

    //=========================================================================
    // WRITE INFO HEADER
    Put2Byte(OutputStream, Header->FileHeader->Identifier);
    Put4Byte(OutputStream, Header->FileHeader->FileSize);
    Put4Byte(OutputStream, Header->FileHeader->Reserved);
    Put4Byte(OutputStream, Header->FileHeader->BitmapDataOffset);
    //=========================================================================

    //=========================================================================
    // WRITE FILE HEADER
    Put4Byte(OutputStream, Header->InfoHeader->BitmapHeaderSize);
    Put4Byte(OutputStream, Header->InfoHeader->Width);
    Put4Byte(OutputStream, Header->InfoHeader->Height);
    Put2Byte(OutputStream, Header->InfoHeader->Planes);
    Put2Byte(OutputStream, Header->InfoHeader->BitPerPixel);
    Put4Byte(OutputStream, Header->InfoHeader->Compression);
    Put4Byte(OutputStream, Header->InfoHeader->BitmapDataSize);
    Put4Byte(OutputStream, Header->InfoHeader->HorizontalResolution);
    Put4Byte(OutputStream, Header->InfoHeader->VerticalResolution);
    Put4Byte(OutputStream, Header->InfoHeader->UsedColors);
    Put4Byte(OutputStream, Header->InfoHeader->ImportanColors);
    //=========================================================================

    if( Header->InfoHeader->BitPerPixel == 24 )
    {
        for( int y=Header->InfoHeader->Height-1; y>=0; y-- )
        {
            for( int x=0; x<Header->InfoHeader->Width; x++ )
            {
                Put3Byte(OutputStream, RGBVector[x][y].ByteValue);
            }
            for( int k=0; k<(4-(Header->InfoHeader->Width*3)%4)%4; k++ ) Put1Byte(OutputStream, 0x00);
        }
    }

    OutputStream.close();

}

void BMP::DeleteUserData()
{
    if( RGBVector != NULL )
    {
        if(DEBUG_INFO) cout << "Delete RGB Vector" << endl;
        delete [] RGBVector;
    }

    if( YUVVector != NULL )
    {
        if(DEBUG_INFO) cout << "Delete YUV Vector" << endl;
        delete [] YUVVector;
    }

    if( HSIVector != NULL )
    {
        if(DEBUG_INFO) cout << "Delete HSI Vector why" << endl;
        delete [] HSIVector;
    }

    if( Histogram != NULL )
    {
        if(DEBUG_INFO) cout << "Delete Histogram" << endl;
        delete Histogram;
    }

}

void BMP::ResetUserData()
{
    if( RGBVector != NULL )
    {
        if(DEBUG_INFO) cout << "Reset RGB Vector" << endl;
        delete [] RGBVector;
        RGBVector = NULL;
    }

    if( YUVVector != NULL )
    {
        if(DEBUG_INFO) cout << "Reset YUV Vector" << endl;
        delete [] YUVVector;
        YUVVector = NULL;
    }

    if( HSIVector != NULL )
    {
        if(DEBUG_INFO) cout << "Reset HSI Vector" << endl;
        delete [] HSIVector;
        HSIVector = NULL;
    }

    if( Histogram != NULL )
    {
        if(DEBUG_INFO) cout << "Reset Histogram" << endl;
        delete Histogram;
        Histogram = NULL;
    }
}

void BMP::Put1Byte(ofstream& OutputStream, UINT8 Data)
{
    OutputStream.put(Data);
}

void BMP::Put2Byte(ofstream& OutputStream, UINT16 Data)
{
    for( int i=0; i<2; i++ )
    {
        OutputStream.put(Data & 0x00FF);
        Data = Data >> 8;
    }
}

void BMP::Put3Byte(ofstream& OutputStream, UINT32 Data)
{
    for( int i=0; i<3; i++ )
    {
        OutputStream.put(Data & 0x000000FF);
        Data = Data >> 8;
    }
}

void BMP::Put4Byte(ofstream& OutputStream, UINT32 Data)
{
    for( int i=0; i<4; i++ )
    {
        OutputStream.put(Data & 0x000000FF);
        Data = Data >> 8;
    }
}

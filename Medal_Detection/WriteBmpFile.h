// WriteBmpFile.h: The interface of the CWriteBmpFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WRITEBMPFILE_H__FEE6FD88_4B85_424F_A8DE_CDB7A4726BD9__INCLUDED_)
#define AFX_WRITEBMPFILE_H__FEE6FD88_4B85_424F_A8DE_CDB7A4726BD9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CWriteBmpFile  
{
public:
	CWriteBmpFile();
	virtual ~CWriteBmpFile();

	int	ImageWrite2BmpFile(const char * p_FileName, int p_X, int p_Y, unsigned char * p_buff, int p_inf);
protected:
	int SetBMPHeder(int p_X, int p_Y, int p_inf, BITMAPFILEHEADER * p_BMP_FILE, BITMAPINFOHEADER * p_BMP_INFO);
	int BMPSetPlet(FILE * p_fp, int p_inf);
	int ImgToBmp(int p_X, int p_Y, int p_inf, char * p_bmp, char * p_img);
};

#endif // !defined(AFX_WRITEBMPFILE_H__FEE6FD88_4B85_424F_A8DE_CDB7A4726BD9__INCLUDED_)

#include "stdafx.h"
#include "Bound.h"
#include "WriteBmpFile.h"
#include "jpeg.H"

void *	alloc_mem( int p_nSize )
{
	void*	w_pret = malloc(p_nSize);	
	return  w_pret;
}

void	free_mem( void *  p_lpUserData )
{
	free(p_lpUserData);
}

int	ImageWrite2BmpFile(											//:CAL:Save bmp-data as file
							const char*			p_FileName		//:IN :File name.
					   ,	int					p_X				//:IN :width
					   ,	int					p_Y				//:IN :height
					   ,	UCHAR*				p_buff			//:IN :Bitmap data.
					   ,	int					p_inf			//:IN :8=grayÅA24=color
					   );


#define DIB_HEADER_MARKER	((WORD) ('M' << 8) | 'B') 

typedef struct tagBITMAPFILEHEADER1
{
	DWORD bfSize; 
	WORD bfReserved1; 
	WORD bfReserved2; 
	DWORD bfOffBits; 
} BITMAPFILEHEADER1;//12

typedef struct tagBITMAPINFOHEADER1
{
	DWORD biSize; 
	long biWidth; 
	long biHeight; 
	WORD biPlanes; 
	WORD biBitCount;
	DWORD biCompression; 
	DWORD biSizeImage; 
	long biXPelsPerMeter; 
	long biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER1;//40



int LoadJpeg(PByteImage image, char *file_name)
{
	CJPEG jpeg;
	jpeg.LoadJPG(file_name);

	
	image->width = jpeg.GetWidth();
	image->height = jpeg.GetHeight();

	image->img = new BYTE[image->width * image->height];

	memcpy(image->img, jpeg.m_pGrayData, image->width * image->height);
	return SFE_OK;
}

int LoadBmp(PByteImage image,LPTSTR strFileName)
{
	int		vRet = SFE_OK;
	BYTE*	buffer = NULL;
	BITMAPFILEHEADER1 	bmfHdr;
	BITMAPINFOHEADER1 	bmiHeader;
	FILE*	fpFile;
	int		dwSize = 0;
	int 	i,j,m,stride;
	WORD	bfType;
	double	clr;
	
	//open bitmap file
	//if (!(fpFile = fopen(strFileName,"rb+"))) {

	_tfopen_s( &fpFile, strFileName, _T("rb+"));
	if (!fpFile) {
        vRet = SFE_FILEIO_ERROR;
		goto RET;
	}

	if (fread(&bfType,1,2,fpFile) != 2) {
		//printf("read BITMAPFILEHEADER error!\n");
		vRet = SFE_FILEIO_ERROR;
		goto RET;
	}
	// is DIB file?
	if (bfType != DIB_HEADER_MARKER) {
		//printf("Please give a right bitmap file!\n");
		vRet = SFE_INVALID_ARGUMENT;
		goto RET;
	}

	if (fread(&bmfHdr,1,sizeof(BITMAPFILEHEADER1),fpFile) != sizeof(BITMAPFILEHEADER1)) {
		//printf("read BITMAPFILEHEADER error!\n");
		vRet = SFE_FILEIO_ERROR;
		goto RET;
	}

	if (fread(&bmiHeader,1,sizeof(BITMAPINFOHEADER1),fpFile) != sizeof(BITMAPINFOHEADER1)) {
		//printf("read BITMAPINFOHEADER error!\n");
		vRet = SFE_FILEIO_ERROR;
		goto RET;
	}
	
	if ((bmiHeader.biBitCount!=8) && (bmiHeader.biBitCount!=16) && (bmiHeader.biBitCount!=24)) {
		//printf("Please give a 16BPP or 24BPP bitmap file!\n");
		vRet = SFE_INVALID_ARGUMENT;
		goto RET;
	}

	image->height = (short)bmiHeader.biHeight;
	image->width = (short)bmiHeader.biWidth;
	// 2011-08-22 Kim Song Gun
	stride = 4 * ((bmiHeader.biWidth - 1) / 4 + 1);
	fseek(fpFile, bmfHdr.bfOffBits, SEEK_SET);
	// 2011-08-22
	//dwSize = bmiHeader.biWidth * bmiHeader.biHeight * ( bmiHeader.biBitCount/8 );		
	dwSize = stride * bmiHeader.biHeight * ( bmiHeader.biBitCount / 8 );		

	if (dwSize<=0) {
		//printf("bitmap size=%d error!\n",dwSize);
		vRet = SFE_INVALID_ARGUMENT;
		goto RET;
	}
	//buffer = new BYTE[dwSize];
	// 2011-11-26 Kim Song Gun
	buffer = (BYTE*)alloc_mem(dwSize * sizeof(BYTE));
	if (buffer == NULL) {
		vRet = SFE_INVALID_ARGUMENT;
		goto RET;
	}
	
	fread(buffer, dwSize, 1,fpFile);
	//delete [] image->img;
	image->img = new BYTE[bmiHeader.biWidth * bmiHeader.biHeight];
	if (bmiHeader.biBitCount == 8) {
		//8BPP bitmap
	 	for( j = 0; j < bmiHeader.biHeight; j++) {
			for( i = 0; i < bmiHeader.biWidth; i++) {
				clr = buffer[(bmiHeader.biHeight - 1 - j) * stride + i];
				image->img[j * bmiHeader.biWidth + i] = (BYTE)clr;
			}
		}
	}
	else if (bmiHeader.biBitCount == 16) {
		//16BPP bitmap
	 	for( j = 0; j < bmiHeader.biHeight; j++) {
			for( i = 0; i < bmiHeader.biWidth; i++) {
				clr = buffer[((bmiHeader.biHeight - 1 - j) * bmiHeader.biWidth + i) * 2];
				image->img[j * bmiHeader.biWidth+i] = (BYTE)clr;
			}
		}
	}
	else {
		//24BPP bitmap
	 	for( j = 0; j < bmiHeader.biHeight; j++) {
			for( i = 0; i < bmiHeader.biWidth; i++) {
				m = ((bmiHeader.biHeight - 1 - j) * bmiHeader.biWidth + i) * 3;
				clr = buffer[m] * B_FACTOR + buffer[m + 1] * G_FACTOR + buffer[m + 2] * R_FACTOR;
				image->img[j * bmiHeader.biWidth + i] = (BYTE)clr;
			}
		}
	}

RET:
	if (buffer != NULL) {
		free_mem(buffer);
		buffer = NULL;
	}
	if (fpFile) {
		fclose(fpFile);
	}
	return vRet;
}

void SaveAsJpeg(PByteImage image, char *file_name)
{
	CJPEG jpeg;
	jpeg.SaveJPG(file_name, image->width, image->height, image->img);
}

int SaveAsBmp(PByteImage image, char *file_name)
{
	return ImageWrite2BmpFile(file_name, image->width, image->height, image->img, 8);
}




// 2011-09-08 Kim Song Gun
int SaveMAT(BYTE *image, int width , int height , char *file_name)
{
	return ImageWrite2BmpFile(file_name, width, height, image, 8);
}

int LoadColorImageFile(PByteColorImage in_img, CString file_name)
{
	CImage image;
	int i,j,w,h;
	int nPitch,nBPP,nstep;
	BYTE *pData, *pb_r, *pb_g, *pb_b;

	HRESULT ret = image.Load(file_name);
	if (FAILED(ret))
		return 1;
	nBPP = image.GetBPP();
	nPitch = image.GetPitch();
	w = image.GetWidth();
	h = image.GetHeight();
	pData  = (BYTE*)image.GetBits();

	in_img->height = h;
	in_img->width = w;
	in_img->R_img = new BYTE[w * h];
	in_img->G_img = new BYTE[w * h];
	in_img->B_img = new BYTE[w * h];

	pb_r = in_img->R_img;
	pb_g = in_img->G_img;
	pb_b = in_img->B_img;
	if (nBPP == 8){
		nstep = nBPP>>3;
		for (i=0; i<h; i++, pData += nPitch)
		{
			for (j=0; j<w; j++)
			{
				(*pb_b++) = pData[j];
				(*pb_g++) = pData[j];
				(*pb_r++) = pData[j];
			}
		}		
	}
	else
	{
		nstep = nBPP>>3;
		for (i=0; i<h; i++, pData += nPitch)
		{
			for (j=0; j<w; j++)
			{
				(*pb_b++) = pData[j * nstep];
				(*pb_g++) = pData[j * nstep + 1];
				(*pb_r++) = pData[j * nstep + 2];
			}
		}		
	}

	return 0;
}

void ImageFree(PByteImage image)
{
	if(image->img)
	{
		free(image->img);
		image->img = 0;
	}
}

void ColorImageFree(PByteColorImage image)
{
	if(image->R_img)
	{
		free(image->R_img);
		image->R_img = 0;
	}
	if(image->G_img)
	{
		free(image->G_img);
		image->G_img = 0;
	}
	if(image->B_img)
	{
		free(image->B_img);
		image->B_img = 0;
	}
}

int		SFLoadFile(char *file_name, PByteImage image)
{
	char *p;
	p = file_name + strlen(file_name) - 4;
	if((_stricmp(p, ".jpg") == 0) || (_stricmp(p, ".JPG") == 0)){
		return	LoadJpeg(image, file_name);
	}
	else if((_stricmp(p, ".bmp") == 0) || (_stricmp(p, ".BMP") == 0)){
		CString fn(file_name);
		return	LoadBmp(image, (LPTSTR)(LPCTSTR)fn);
	}
	return SFE_INVALID_ARGUMENT;
}

int SaveAsColorBmp(PByteImage image, char *file_name)
{
	return ImageWrite2BmpFile(file_name, image->width, image->height, image->img, 24);
}

int SaveColorImgAsBmp(PByteColorImage pImage, char *file_name)
{
	ByteImage gImage;
	BYTE *bImg = new BYTE[pImage->width * pImage->height * 3];

	memcpy(bImg, pImage->R_img, pImage->width * pImage->height);
	memcpy(bImg + pImage->width * pImage->height, pImage->G_img, pImage->width * pImage->height);
	memcpy(bImg + pImage->width * pImage->height * 2, pImage->B_img, pImage->width * pImage->height);

	gImage.width = pImage->width;
	gImage.height = pImage->height;
	gImage.img = bImg;

	SaveAsColorBmp(&gImage, file_name);

	delete[] bImg;

	return 0;
}
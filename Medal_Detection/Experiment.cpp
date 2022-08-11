#include "stdafx.h"
#include "Bound.h"

extern char Debug_Name[];

void save_Gray_Img(PByteImage in_Img, char* str)
{
	char dis_name[256];
	memset(dis_name, 0, 256);
	memcpy(dis_name, str, strlen(str));
	SaveAsBmp(in_Img, dis_name);
}

void Display_Gray_Img(PByteImage in_Img, char* str)
{
	char dis_name[256];
	memset(dis_name, 0, 256);
	memcpy(dis_name, Debug_Name, strlen(Debug_Name));
	strcat_s(dis_name, str);
	SaveAsBmp(in_Img, dis_name);
}

void Display_Bound_Img(uchar* bound, char* str)
{
	char dis_name[256];
	memset(dis_name, 0, 256);
	memcpy(dis_name, Debug_Name, strlen(Debug_Name));
	strcat_s(dis_name, str);

	int w0, h0;
	w0 = res16_w;	h0 = res16_h;

	uchar *gray_Mem = new uchar[w0 * h0];
	memset(gray_Mem, 0, w0 * h0);
	uchar* pu_in, *pu_out;
	pu_in = bound;	pu_out = gray_Mem;
	for (int i = 0; i < w0 * h0; i++, pu_in++, pu_out++)
	{
		if (*pu_in == 0)
			continue;

		*pu_out = 255;
	}

	ByteImage disImg;
	disImg.width = res16_w;	disImg.height = res16_h;
	disImg.img = gray_Mem;

	save_Gray_Img(&disImg, dis_name);
	delete[]	gray_Mem;

}

void Save_Bound_Img(PByteColorImage in_Img, uchar* bound, int w, int h, char* path_str, char* name_str)
{
	char dis_name[256];
	memset(dis_name, 0, 256);
	memcpy(dis_name, path_str, strlen(path_str));
	strcat_s(dis_name, name_str);

	int w0, h0;
	w0 = w;	h0 = h;

	uchar *r_Mem = new uchar[w0 * h0];
	uchar *g_Mem = new uchar[w0 * h0];
	uchar *b_Mem = new uchar[w0 * h0];
	memcpy(r_Mem, in_Img->R_img, w0 * h0);
	memcpy(g_Mem, in_Img->G_img, w0 * h0);
	memcpy(b_Mem, in_Img->B_img, w0 * h0);

	ByteColorImage disImg;
	disImg.width = w0;	disImg.height = h0;
	disImg.R_img = r_Mem;	disImg.G_img = g_Mem;	disImg.B_img = b_Mem;

	int i, j, i_st, i_ed, j_st, j_ed, ii, jj;
	for (i = 0; i < h0; i++)
	{
		for (j = 0; j < w0; j++)
		{
			if (bound[i*w0 + j] == 0)
				continue;

			r_Mem[i * w0 + j] = 255;
			g_Mem[i * w0 + j] = 0;
			b_Mem[i * w0 + j] = 0;

		}
	}


	SaveColorImgAsBmp(&disImg, dis_name);
	delete[]	r_Mem;
	delete[]	g_Mem;
	delete[]	b_Mem;
}
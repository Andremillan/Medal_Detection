#include "stdafx.h"
#include "Bound.h"


uchar		res32_Gray[res32_w * res32_h];

uchar		res16_Gray[res16_w * res16_h], res16_Bound[res16_w * res16_h], res16_Dir[ori_w * ori_h];


uchar		res4_Gray[res4_w * res4_h], res4_Bound[res4_w * res4_h], res4_Dir[res4_w * res4_h], res4_vhist[res4_h], res4_hhist[res4_w];

uchar		ori_tmp_1[ori_w * ori_h], ori_tmp_2[ori_w * ori_h], ori_Bound[ori_w * ori_h], ori_Dir[ori_w * ori_h];
uchar		ori_B_Reg[ori_w * ori_h], segOri[ori_w * ori_h];
float		Grad_float[ori_w * ori_h];
int			con_Mat[ori_w * ori_h], i_list[(ori_w + 1) * (ori_h + 1)], j_list[(ori_w + 1) * (ori_h + 1)];

// #define		DISPLAY
// #define		RESULT_32

void Get_Gray(PByteColorImage in_Img, PByteImage out_Img)
{
	int i, j, w, h, pixel_Num;
	w = out_Img->width;	h = in_Img->height;
	pixel_Num = w * h;

	uchar *pu_R, *pu_G, *pu_B, *pu_Gray;
	pu_R = in_Img->R_img;	pu_G = in_Img->G_img;	pu_B = in_Img->B_img;
	pu_Gray = out_Img->img;

	int tmp;
	for (i = 0; i < pixel_Num; i++, pu_R++, pu_G++, pu_B++, pu_Gray++)
	{
		tmp = floor(0.299 * pu_R[0] + 0.587 * pu_G[0] + 0.114 * pu_B[0]);
		*pu_Gray = (uchar)tmp;
	}
}

void Resize2(PByteImage in_Img, PByteImage out_Img)
{
	int w_in, h_in, w_out, h_out, scale;
	int i, j;
	uchar* pu_in, *pu_out;
	w_in = in_Img->width;	h_in = in_Img->height;
	w_out = out_Img->width;	h_out = out_Img->height;


	int tmp;
	uchar* pu, *pu_tmp;

	pu_out = out_Img->img;
	for (i = 0; i < h_out; i++)
	{
		pu_in = in_Img->img + (i * 2) * w_in;

		for (j = 0; j < w_out; j++, pu_out++, pu_in += 2)
		{

			tmp = pu_in[0] + pu_in[1] + pu_in[w_in] + pu_in[w_in + 1];

			*pu_out = tmp >> 2;
		}
	}

}

void Resize4(PByteImage in_Img, PByteImage out_Img)
{
	int w_in, h_in, w_out, h_out, scale;
	int i, j;
	uchar* pu_in, *pu_out;
	w_in = in_Img->width;	h_in = in_Img->height;
	w_out = out_Img->width;	h_out = out_Img->height;


	int tmp;
	uchar* pu, *pu_tmp;

	pu_out = out_Img->img;
	for (i = 0; i < h_out; i++)
	{
		pu_in = in_Img->img + (i * 4) * w_in;

		for (j = 0; j < w_out; j++, pu_out++, pu_in += 4)
		{
			/////////////////////////////////////////////////////////////////
			pu_tmp = pu_in;
			tmp = pu_tmp[0] + pu_tmp[1] + pu_tmp[2] + pu_tmp[3];

			pu_tmp += w_in;
			tmp += pu_tmp[0] + pu_tmp[1] + pu_tmp[2] + pu_tmp[3];

			pu_tmp += w_in;
			tmp += pu_tmp[0] + pu_tmp[1] + pu_tmp[2] + pu_tmp[3];

			pu_tmp += w_in;
			tmp += pu_tmp[0] + pu_tmp[1] + pu_tmp[2] + pu_tmp[3];
			
			*pu_out = tmp >> 4;
		}
	}

}

int	Detect_Object_Range(IN PByteImage in_Img, OUT uchar* B_Range)
{
	ByteImage res4_Img;
	res4_Img.width = res4_w;	res4_Img.height = res4_h;
	res4_Img.img = res4_Gray;
	Resize4(in_Img, &res4_Img);
#ifdef _DEBUG
	Display_Gray_Img(&res4_Img, "_resize4.bmp");
#endif

	ByteImage res16_Img;
	res16_Img.width = res16_w;	res16_Img.height = res16_h;
	res16_Img.img = res16_Gray;
	Resize4(&res4_Img, &res16_Img);
#ifdef _DEBUG
	Display_Gray_Img(&res16_Img, "_resize16.bmp");
#endif

	ByteImage res32_Img;
	res32_Img.width = res32_w;	res32_Img.height = res32_h;
	res32_Img.img = res32_Gray;
	Resize2(&res16_Img, &res32_Img);
#ifdef _DEBUG
	Display_Gray_Img(&res32_Img, "_resize32.bmp");
#endif

	Segment_32(&res32_Img, B_Range);	
#ifdef RESULT_32
	res32_Img.img = B_Range;
	Display_Gray_Img(&res32_Img, "_seg32.bmp");
#endif
	return 0;
}

void Get_Res16_Bound(PByteImage in_Img, uchar* in_Bound, uchar* out_Bound, uchar* out_Dir)
{
	int i, j, w, h, w1, h1;
	w = in_Img->width;	h = in_Img->height;
	w1 = w >> 1;	h1 = h >> 1;

	memset(ori_tmp_1, 0, w * h);
	uchar *pu_in, *pu_out;
	pu_in = in_Bound;
	for (i = 0; i < h1; i++)
	{
		pu_out = ori_tmp_1 + (i * 2) * w;
		for (j = 0; j < w1; j++, pu_in++, pu_out += 2)
		{
			if (*pu_in == 0)
				continue;

			pu_out[0] = 255;	pu_out[1] = 255;
			pu_out[w] = 255;	pu_out[w + 1] = 255;
		}
	}
#ifdef DISPLAY
	ByteImage disImg;
	disImg.width = w;	disImg.height = h;
	disImg.img = ori_tmp_1;

	Display_Gray_Img(&disImg, "_0st_bound.bmp");
#endif
	int g_step = 3, g_x, g_y, g_xy, g_yx, grad_thr = 5;
	float grad;

	float *pf_Bound;
	memset(Grad_float, 0, sizeof(float) * w * h);

	pu_in = in_Img->img + g_step * w;
	pu_out = ori_tmp_1 + g_step * w;
	pf_Bound = Grad_float + g_step * w;

	for (i = g_step; i < h - g_step; i++)
	{
		pu_in += g_step;	pu_out += g_step;	pf_Bound += g_step;
		for (j = g_step; j < w - g_step; j++, pu_in++, pu_out++, pf_Bound++)
		{
			if (*pu_out == 0)
				continue;

			grad = 0;
			g_x = abs(pu_in[g_step] - pu_in[-g_step]);
			if (grad < g_x)		grad = g_x;
			g_y = abs(pu_in[g_step*w] - pu_in[-g_step * w]);
			if (grad < g_y)		grad = g_y;
			g_xy = abs(pu_in[-g_step * w - g_step] - pu_in[g_step*w + g_step]);
			if (grad < g_xy)	grad = g_xy;
			g_yx = abs(pu_in[-g_step * w + g_step] - pu_in[g_step*w - g_step]);
			if (grad < g_yx)	grad = g_yx;
			if (grad < grad_thr)
			{
				*pu_out = 0;
				continue;
			}



			grad = 0;
			g_x = (pu_in[g_step] + pu_in[-g_step]) - 2 * pu_in[0];
			if (grad < g_x)		grad = g_x;
			g_y = (pu_in[g_step*w] + pu_in[-g_step * w]) - 2 * pu_in[0];
			if (grad < g_y)		grad = g_y;
			g_xy = pu_in[-g_step * w - g_step] + pu_in[g_step*w + g_step] - 2 * pu_in[0];
			if (grad < g_xy)	grad = g_xy;
			g_yx = pu_in[-g_step * w + g_step] + pu_in[g_step*w - g_step] - 2 * pu_in[0];
			if (grad < g_yx)	grad = g_yx;
			if (grad < grad_thr)
			{
				*pu_out = 0;
				continue;
			}


			*pf_Bound = grad_thr;
			*pu_out = 255;
		}
		pu_in += g_step;	pu_out += g_step;	pf_Bound += g_step;
	}

#ifdef DISPLAY

	disImg.img = ori_tmp_1;

	Display_Gray_Img(&disImg, "_1st_bound.bmp");
#endif

	///////////////////////////////////////////////////////////

	memset(Grad_float, 0, sizeof(float) * w * h);
	memset(out_Bound, 0, w * h);
	memset(out_Dir, 0, w * h);
	g_step = 6;		grad_thr = 15;
	int grad_thr1 = 5;
	uchar *pu_reg, *pu_dir;
	int *ps1, *ps2, *ps3;
	int k, s1, s2, s3, s1_t, s2_t, s3_t, dd = 2, hh = 5;
	int cur_val, max_val, maxk;

	pf_Bound = Grad_float + g_step * w;
	pu_in = in_Img->img + g_step * w;
	pu_reg = ori_tmp_1 + g_step * w;
	pu_out = out_Bound + g_step * w;
	pu_dir = out_Dir + g_step * w;

	for (i = g_step; i < h - g_step; i++)
	{
		pu_in += g_step;	pu_reg += g_step;	pu_out += g_step;	pu_dir += g_step;	pf_Bound += g_step;
		for (j = g_step; j < w - g_step; j++, pu_in++, pu_reg++, pu_out++, pu_dir++, pf_Bound++)
		{
			if (*pu_reg == 0)
				continue;


			max_val = 0;
			for (k = 0; k < 16; k++)
			{
				ps1 = Dir_TbXY_Res16 + k * (2 * dd + 1)*(2 * hh + 1);
				ps2 = ps1 + dd * (2 * hh + 1);
				ps3 = ps2 + dd * (2 * hh + 1);

				s1 = 0;	s2 = 0;	s3 = 0;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;

				s1 /= 11;	s2 /= 11;	s3 /= 11;
				//if (s1 + s3 - 2 * s2 < grad_thr1)
				//	continue;

				//cur_val = abs(s1 - s3);
				if (abs(s1 - s3) < grad_thr)
					continue;

				cur_val = s1 + s3 - 2 * s2;
				if (cur_val < 0)
					continue;

				if (cur_val > max_val)
				{
					max_val = cur_val;
					maxk = k;
				}
			}

			if (max_val < grad_thr1)
				continue;

			*pu_out = 255;
			*pu_dir = maxk;
			*pf_Bound = max_val;
		}
		pu_in += g_step;	pu_reg += g_step;	pu_out += g_step;	pu_dir += g_step;	pf_Bound += g_step;
	}

#ifdef DISPLAY	
	disImg.img = out_Bound;
	Display_Gray_Img(&disImg, "_2nd_bound.bmp");
#endif
	memcpy(ori_tmp_1, out_Dir, w * h);


	int di[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	int dj[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	int cDir, i0;
	float ssf, ccf, alpha, mss;

	pf_Bound = Grad_float + g_step * w;
	pu_in = ori_tmp_1 + g_step * w;
	pu_out = out_Dir + g_step * w;
	for (i = g_step; i < h - g_step; i++)
	{
		pf_Bound += g_step;	pu_in += g_step;	pu_out += g_step;
		for (j = g_step; j < w - g_step; j++, pf_Bound++, pu_in++, pu_out++)
		{
			if (*pf_Bound == 0)
				continue;

			ssf = pf_Bound[0] * SS2_T[*pu_in];
			ccf = pf_Bound[0] * CC2_T[*pu_in];
			for (k = 0; k < 8; k++)
			{
				i0 = res16_dij[k];
				mss = pf_Bound[i0];
				if (mss == 0)
					continue;

				cDir = pu_in[i0];
				ssf += mss * SS2_T[cDir];	ccf += mss * CC2_T[cDir];
			}
			alpha = atan2(ssf, ccf);
			if (alpha < 0)
				alpha += 2 * PI;
			alpha /= 2;
			cDir = floor(alpha * 16 / PI + 0.5);
			if (cDir > 15)
				cDir -= 16;

			*pu_out = cDir;
		}
		pf_Bound += g_step;	pu_in += g_step;	pu_out += g_step;
	}

}

void Erase_Point_Bound(PByteImage in_Img, uchar* Bound)
{
	int w, h;
	w = in_Img->width;	h = in_Img->height;

	memcpy(ori_tmp_1, Bound, w * h);
	memset(Bound, 0, w * h);

	int i, j, step = 1, nn;
	uchar *pu_in, *pu_out;
	pu_in = ori_tmp_1 + step * w;
	pu_out = Bound + step * w;
	for (i = step; i < h - step; i++)
	{
		pu_in += step;	pu_out += step;
		for (j = step; j < w - step; j++, pu_in++, pu_out++)
		{
			if (*pu_in == 0)
				continue;

			nn = pu_in[-w - 1] + pu_in[-w] + pu_in[-w + 1];
			nn += pu_in[-1] + pu_in[1];
			nn += pu_in[w - 1] + pu_in[w] + pu_in[w + 1];

			nn /= 255;
			if (nn < 2)
				continue;

			*pu_out = 255;
		}
		pu_in += step;	pu_out += step;
	}
}

void NMS_Res16_Bound(uchar* in_Bound, uchar* in_Dir, uchar* out_Bound)
{
	int w, h;
	w = res16_w;	h = res16_h;

	memset(out_Bound, 0, w * h);

	int i, j, k, k1, dd, hh, g_step, dir, maxi, maxj, s1, s2, s3, max_val, cur_val, cur_dir, nn, max_dir, delta_dir;
	int *ps, flag;

	dd = 0;	hh = 10;
	g_step = 10;


	uchar *pu_reg, *pu_dir, *pu_out;
	float *pf_Bound;

	pf_Bound = Grad_float + g_step * w;
	pu_reg = in_Bound + g_step * w;
	pu_dir = in_Dir + g_step * w;
	pu_out = out_Bound + g_step * w;
	for (i = g_step; i < h - g_step; i++)
	{
		pu_reg += g_step;	pu_dir += g_step;	pu_out += g_step;	pf_Bound += g_step;
		for (j = g_step; j < w - g_step; j++, pu_reg++, pu_dir++, pu_out++, pf_Bound++)
		{
			if (*pu_reg == 0)
				continue;

			nn = pu_reg[-w - 1] + pu_reg[-w] + pu_reg[-w + 1];
			nn += pu_reg[-1] + pu_reg[0] + pu_reg[1];
			nn += pu_reg[w - 1] + pu_reg[w] + pu_reg[w + 1];

			nn /= 255;
			if (nn == 3)
			{
				*pu_out = 255;
				continue;
			}

			if (nn < 3)
				continue;

			max_dir = *pu_dir;


			flag = 0;
			for (dir = -2; dir <= 2; dir++)
			{
				cur_dir = max_dir;
				if (cur_dir < 0)	cur_dir += 16;
				if (cur_dir > 15)	cur_dir -= 16;

				ps = Ver_TbXY_Res16 + cur_dir * (2 * dd + 1) * (2 * hh + 1) + hh;

				max_val = pf_Bound[ps[0]];

				cur_val = pf_Bound[ps[1]];
				delta_dir = abs(pu_dir[ps[1]] - cur_dir);
				if (delta_dir > 8)	delta_dir = 16 - delta_dir;
				if (delta_dir <= 1 && max_val < cur_val)
				{
					flag = 1;
					break;
				}

				cur_val = pf_Bound[ps[-1]];
				delta_dir = abs(pu_dir[ps[-1]] - cur_dir);
				if (delta_dir > 8)	delta_dir = 16 - delta_dir;
				if (delta_dir <= 1 && max_val < cur_val)
				{
					flag = 1;
					break;
				}
			}

			if (flag == 1)
				continue;

			*pu_out = 255;

		}

		pu_reg += g_step;	pu_dir += g_step;	pu_out += g_step;	pf_Bound += g_step;
	}
}

void Extend_Bound(uchar *Bound, int w, int h)
{
	memcpy(ori_tmp_1, Bound, w * h);
	int i, j, step = 1;
	uchar *pu_in, *pu_out;

	pu_in = ori_tmp_1 + step * w;
	pu_out = Bound + step * w;
	for (i = step; i < h - step; i++)
	{
		pu_in += step;	pu_out += step;	
		for (j = step; j < w - step; j++, pu_in++, pu_out++)
		{
			if (*pu_in == 255)
				continue;

			if (pu_in[-w - 1] == 255)
			{
				*pu_out = 255; continue;
			}

			if (pu_in[-w] == 255)
			{
				*pu_out = 255; continue;
			}

			if (pu_in[-w + 1] == 255)
			{
				*pu_out = 255; continue;
			}

			if (pu_in[-1] == 255)
			{
				*pu_out = 255; continue;
			}

			if (pu_in[1] == 255)
			{
				*pu_out = 255; continue;
			}

			if (pu_in[w - 1] == 255)
			{
				*pu_out = 255; continue;
			}

			if (pu_in[w] == 255)
			{
				*pu_out = 255; continue;
			}

			if (pu_in[w + 1] == 255)
			{
				*pu_out = 255; continue;
			}
		}
		pu_in += step;	pu_out += step;
	}
}

void Smooth_Bound(uchar *Bound, uchar* Dir, int w, int h)
{
	memcpy(ori_tmp_1, Bound, w * h);
	memcpy(ori_tmp_2, Dir, w * h);


	int i, j, step = 1;
	uchar *pu_in, *pu_out, *pu_in_dir, *pu_out_dir;

	pu_in = ori_tmp_1 + step * w;
	pu_out = Bound + step * w;
	pu_in_dir = ori_tmp_2 + step * w;
	pu_out_dir = Dir + step * w;
	for (i = step; i < h - step; i++)
	{
		pu_in += step;	pu_out += step;	pu_in_dir += step;	pu_out_dir += step;
		for (j = step; j < w - step; j++, pu_in++, pu_out++, pu_in_dir++, pu_out_dir++)
		{
			if (*pu_in == 255)
				continue;

			if (pu_in[-w - 1] == 255)
			{
				*pu_out = 255; *pu_out_dir = pu_in_dir[-w-1];	 continue;
			}

			if (pu_in[-w] == 255)
			{
				*pu_out = 255; *pu_out_dir = pu_in_dir[-w];	 continue;
			}

			if (pu_in[-w + 1] == 255)
			{
				*pu_out = 255; *pu_out_dir = pu_in_dir[-w+1];	 continue;
			}

			if (pu_in[-1] == 255)
			{
				*pu_out = 255; *pu_out_dir = pu_in_dir[-1];	 continue;
			}

			if (pu_in[1] == 255)
			{
				*pu_out = 255; *pu_out_dir = pu_in_dir[1];	 continue;
			}

			if (pu_in[w - 1] == 255)
			{
				*pu_out = 255; *pu_out_dir = pu_in_dir[w-1];	 continue;
			}

			if (pu_in[w] == 255)
			{
				*pu_out = 255; *pu_out_dir = pu_in_dir[w];	 continue;
			}

			if (pu_in[w + 1] == 255)
			{
				*pu_out = 255; *pu_out_dir = pu_in_dir[w+1];	 continue;
			}
		}
		pu_in += step;	pu_out += step;	pu_in_dir += step;	pu_out_dir += step;
	}
}

void Get_Res4_Bound(uchar *res_Bound, PByteImage in_Img, uchar *out_Bound, uchar *out_Dir)
{
	int w, h, w_res, h_res;
	w = in_Img->width;	h = in_Img->height;
	w_res = w >> 2;	h_res = h >> 2;

	memset(ori_tmp_1, 0, w * h);

	int i, j, dir0;
	
	uchar *pu_reg, *pu_in, *pu_out, *pu_dir;
	pu_reg = res_Bound;		pu_out = ori_tmp_1;
	
	for (i = 0; i < h_res; i++)
	{		
		pu_out = ori_tmp_1 + (4 * i) * w;
		for (j = 0; j < w_res; j++, pu_reg++, pu_out += 4)
		{

			if (*pu_reg == 0)
				continue;

			pu_out[0] = 255;	pu_out[1] = 255;	pu_out[2] = 255;	pu_out[3] = 255;
			pu_out[w] = 255;	pu_out[w + 1] = 255;	pu_out[w + 2] = 255;	pu_out[w + 3] = 255;
			pu_out[2 * w] = 255;	pu_out[2 * w + 1] = 255;	pu_out[2 * w + 2] = 255;	pu_out[2 * w + 3] = 255;	
			pu_out[3 * w] = 255;	pu_out[3 * w + 1] = 255;	pu_out[3 * w + 2] = 255;	pu_out[3 * w + 3] = 255;

		}
		
	}


#ifdef DISPLAY
	ByteImage disImg;
	disImg.width = w;	disImg.height = h;
	disImg.img = ori_tmp_1;

	Display_Gray_Img(&disImg, "_mid_1st_bound.bmp");
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int g_x, g_y, g_xy, g_yx, g_step = 4, grad_thr = 20;
	float grad;
	

	memset(out_Bound, 0, w * h);

	pu_in = in_Img->img + g_step * w;
	pu_reg = ori_tmp_1 + g_step * w;
	pu_out = out_Bound + g_step * w;
	for (i = g_step; i < h - g_step; i++)
	{
		pu_in += g_step;	pu_out += g_step;	pu_reg += g_step;	
		for (j = g_step; j < w - g_step; j++, pu_in++, pu_out++, pu_reg++)
		{
			if (*pu_reg == 0)
				continue;

			grad = 0;
			g_x = abs(pu_in[g_step] - pu_in[-g_step]);				
			if (grad < g_x)		grad = g_x;
			g_y = abs(pu_in[g_step*w] - pu_in[-g_step * w]);
			if (grad < g_y)		grad = g_y;
			g_xy = abs(pu_in[-g_step * w - g_step] - pu_in[g_step*w + g_step]);
			if (grad < g_xy)	grad = g_xy;
			g_yx = abs(pu_in[-g_step * w + g_step] - pu_in[g_step*w - g_step]);
			if (grad < g_yx)	grad = g_yx;
			
			if (grad < grad_thr)
				continue;

			grad = 0;
			g_x = (pu_in[g_step] + pu_in[-g_step]) - 2 * pu_in[0];
			if (grad < g_x)		grad = g_x;
			g_y = (pu_in[g_step*w] + pu_in[-g_step * w]) - 2 * pu_in[0];
			if (grad < g_y)		grad = g_y;
			g_xy = pu_in[-g_step * w - g_step] + pu_in[g_step*w + g_step] - 2 * pu_in[0];
			if (grad < g_xy)	grad = g_xy;
			g_yx = pu_in[-g_step * w + g_step] + pu_in[g_step*w - g_step] - 2 * pu_in[0];
			if (grad < g_yx)	grad = g_yx;

			if (grad < grad_thr)
				continue;

			*pu_out = 255;
		}
		pu_in += g_step;	pu_out += g_step;	pu_reg += g_step;	
	}
#ifdef DISPLAY
	disImg.img = out_Bound;
	Display_Gray_Img(&disImg, "_mid_2nd_bound.bmp");
#endif


	//////////////////////////////////////////////////////////////////////////////////

	memcpy(ori_tmp_1, out_Bound, w * h);
	memset(out_Bound, 0, w * h);
	memset(out_Dir, 0, w * h);
	
	float div_const_11 = 1.0 / 11;
	int cur_dir, grad_thr1 = 10;
	grad_thr = 20;
	int *ps1, *ps2, *ps3;
	int k, s1, s2, s3, s1_t, s2_t, s3_t, dd = 2, hh = 5;
	int cur_val, max_val, maxk;
	g_step = floor(sqrt(1.0*dd*dd + hh * hh) + 0.5);

	pu_in = in_Img->img + g_step * w;	
	pu_reg = ori_tmp_1 + g_step * w;
	pu_out = out_Bound + g_step * w;
	pu_dir = out_Dir + g_step * w;

	for (i = g_step; i < h - g_step; i++)
	{
		pu_in += g_step;	pu_reg += g_step;	pu_out += g_step;	pu_dir += g_step;	
		for (j = g_step; j < w - g_step; j++, pu_in++, pu_reg++, pu_out++, pu_dir++)
		{
			if (*pu_reg == 0)
				continue;

			max_val = 0;
			for (k = 0; k < 16; k++)
			{
				ps1 = Dir_TbXY_Res4 + k * (2 * dd + 1)*(2 * hh + 1);
				ps2 = ps1 + dd * (2 * hh + 1);
				ps3 = ps2 + dd * (2 * hh + 1);

				s1 = 0;	s2 = 0;	s3 = 0;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;
				s1 += pu_in[*ps1]; ps1++; s2 += pu_in[*ps2]; ps2++; s3 += pu_in[*ps3]; ps3++;

				s1 *= div_const_11;	s2 *= div_const_11;	s3 *= div_const_11;

				if ((s1 + s3 - 2 * s2) < grad_thr1)
					continue;

				cur_val = abs(s1 - s3);


				if (cur_val > max_val)
				{
					max_val = cur_val;
					maxk = k;
				}
			}

			if (max_val < grad_thr)
				continue;
				
			*pu_out = 255;
			*pu_dir = maxk;			
		}
		pu_in += g_step;	pu_reg += g_step;	pu_out += g_step;	pu_dir += g_step;	
	}
#ifdef DISPLAY	
	disImg.img = out_Bound;
	Display_Gray_Img(&disImg, "_mid_3rd_bound.bmp");
#endif

	memcpy(ori_tmp_1, out_Bound, w * h);
	memset(out_Bound, 0, w * h);
	dd = 7;	hh = 3;
	g_step = dd + hh;

	pu_in = in_Img->img + g_step * w;
	pu_reg = ori_tmp_1 + g_step * w;
	pu_dir = out_Dir + g_step * w;
	pu_out = out_Bound + g_step * w;
	
	for (i = g_step; i < h - g_step; i++)
	{
		pu_in += g_step;	 pu_reg += g_step;	pu_dir += g_step;	pu_out += g_step;
		for (j = g_step; j < w - g_step; j++, pu_in++, pu_reg++, pu_dir++, pu_out++)
		{
			if (*pu_reg == 0)
				continue;

			cur_dir = *pu_dir;
			ps2 = Ver_TbXY_Res4 + cur_dir * (2 * dd + 1) * (2 * hh + 1) + dd * (2* hh + 1) + hh;
			ps1 = ps2 - 4 * (2 * hh + 1);
			ps3 = ps2 + 4 * (2 * hh + 1);
			s1 = pu_in[ps1[-2]] + pu_in[ps1[-1]] + pu_in[ps1[0]] + pu_in[ps1[1]] + pu_in[ps1[2]];	s1 /= 5;
			s3 = pu_in[ps3[-2]] + pu_in[ps3[-1]] + pu_in[ps3[0]] + pu_in[ps3[1]] + pu_in[ps3[2]];	s3 /= 5;
			if (abs(s1 - s3) < 15)
				continue;

			*pu_out = 255;
		}
		pu_in += g_step;	 pu_reg += g_step;	pu_dir += g_step;	pu_out += g_step;
	}

#ifdef DISPLAY	
	disImg.img = out_Bound;
	Display_Gray_Img(&disImg, "_mid_4th_bound.bmp");
#endif
	i = 1;
}

void Connect_8_Ori(uchar* Bound, int* Con_Mat, int i_pos, int j_pos, int label, 
				int w_st, int h_st, int w_ed, int h_ed)
{
	int w, h;
	w = ori_w;	h = ori_h;

	int leng = 0;
	uchar *pu;
	int *pi;
	i_list[leng] = i_pos;	j_list[leng] = j_pos;
	leng++;

	int di_list[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	int dj_list[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	int k, cur_i, cur_j, next_i, next_j;
	while (leng > 0)
	{
		cur_i = i_list[leng - 1];	cur_j = j_list[leng - 1];
		leng--;

		pu = Bound + cur_i * w + cur_j;
		pi = Con_Mat + cur_i * w + cur_j;

		*pi = label;

		for (k = 0; k < 8; k++)
		{
			next_i = cur_i + di_list[k];	next_j = cur_j + dj_list[k];

			if (next_i < h_st || next_i > h_ed)
				continue;

			if (next_j < w_st || next_j > w_ed)
				continue;

			if (pu[ori_dij[k]] == 0)
				continue;

			if (pi[ori_dij[k]] != -1)
				continue;

			pi[ori_dij[k]] = label;

			i_list[leng] = next_i;	j_list[leng] = next_j;
			leng++;

		}
	}
}

void Erase_Isolated_Points_Ori(uchar* Bound, int w_st, int h_st, int w_ed, int h_ed)
{
	int i, j, w, h, cur_label;
	w = ori_w;	h = ori_h;

	memset(con_Mat, -1, sizeof(int) * w * h);
	uchar	*pu;
	int		*pi;

	cur_label = 0;
	pu = Bound + h_st * w;	
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pi++)
		{
			if (*pu == 0)
				continue;

			if (*pi == -1)
			{
				Connect_8_Ori(Bound, con_Mat, i, j, cur_label, w_st, h_st, w_ed, h_ed);
				cur_label++;
			}
		}
		pu += (w - 1 - w_ed);	pi += (w - 1 - w_ed);
	}

	int* leng_vec = new int[cur_label];
	memset(leng_vec, 0, sizeof(int) * cur_label);

	pu = Bound + h_st * w;
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pi++)
		{
			if (*pu == 0)
				continue;

			leng_vec[pi[0]]++;
		}
		pu += (w - 1 - w_ed);	pi += (w - 1 - w_ed);
	}


	pu = Bound + h_st * w;
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pi++)
		{
			if (*pu == 0)
				continue;

			if (leng_vec[pi[0]] < 150)
				*pu = 0;
		}
		pu += (w - 1 - w_ed);	pi += (w - 1 - w_ed);
	}
	delete[]	leng_vec;
}

void Get_Res4_Range(uchar *Bound, int* w_st, int* h_st, int* w_ed, int* h_ed)
{
	int w, h;
	w = res4_w;	h = res4_h;

	memset(res4_vhist, 0, h);
	memset(res4_hhist, 0, w);

	int i, j, flag;
	uchar *pu;
	for (i = 0; i < h; i++)
	{
		pu = Bound + i * w;
		flag = 0;
		for (j = 0; j < w; j++, pu++)
		{
			if (*pu != 0)
			{
				flag = 1;
				break;
			}
		}
		res4_vhist[i] = flag;
	}

	flag = 0;
	for (i = 0; i < h - 1; i++)
	{
		if (res4_vhist[i] == 0 && res4_vhist[i + 1] != 0)
		{
			flag = 1;
			break;
		}

	}
	if (flag == 1)
		*h_st = i;

	flag = 0;
	for (i = h - 1; i >= 1; i--)
	{
		if (res4_vhist[i] == 0 && res4_vhist[i - 1] != 0)
		{
			flag = 1;
			break;
		}

	}
	if (flag == 1)
		*h_ed = i;

	for (j = 0; j < w; j++)
	{
		flag = 0;
		pu = Bound + j;
		for (i = 0; i < h; i++, pu += w)
		{
			if (*pu != 0)
			{
				flag = 1;
				break;
			}
		}
		res4_hhist[j] = flag;
	}

	flag = 0;
	for (j = 0; j < w - 1; j++)
	{
		if (res4_hhist[j] == 0 && res4_hhist[j + 1] != 0)
		{
			flag = 1;
			break;
		}

	}
	if (flag == 1)
		*w_st = j;

	flag = 0;
	for (j = w - 1; j >= 0; j--)
	{
		if (res4_hhist[j] == 0 && res4_hhist[j - 1] != 0)
		{
			flag = 1;
			break;
		}

	}
	if (flag == 1)
		*w_ed = j;
}

void Get_ori_Bound_Region(uchar* res4_Bound, uchar* res4_Dir, 
						uchar* ori_Bound, uchar* out_Reg, 
						int w_st, int h_st, int w_ed, int h_ed)
{
	int w, h, w1, h1;
	w = ori_w;	h = ori_h;
	w1 = w >> 2;	h1 = h >> 2;

	int i, j, k, dir;
	uchar *pu_in, *pu_out, *pu_in_dir, *pu_out_dir;

	memset(ori_Bound, 0, w * h);
	//memset(ori_Dir, 0, w * h);

	pu_in = res4_Bound + h_st * w1;
	pu_in_dir = res4_Dir + h_st * w1;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_in += w_st;	pu_in_dir += w_st;
		pu_out = ori_Bound + (i * 4) * w + (w_st * 4);
		pu_out_dir = ori_Dir + (i * 4) * w + (w_st * 4);
		for (j = w_st; j <= w_ed; j++, pu_in++, pu_in_dir++, pu_out += 4, pu_out_dir += 4)
		{
			if (*pu_in == 0)
				continue;

			pu_out[0] = 255;	pu_out[1] = 255;	pu_out[2] = 255;	pu_out[3] = 255;
			pu_out[w] = 255;	pu_out[w + 1] = 255;	pu_out[w + 2] = 255;	pu_out[w + 3] = 255;
			pu_out[2 * w] = 255;	pu_out[2 * w + 1] = 255;	pu_out[2 * w + 2] = 255;	pu_out[2 * w + 3] = 255;
			pu_out[3 * w] = 255;	pu_out[3 * w + 1] = 255;	pu_out[3 * w + 2] = 255;	pu_out[3 * w + 3] = 255;

			dir = *pu_in_dir;
			pu_out_dir[0] = dir;	pu_out_dir[1] = dir;	pu_out_dir[2] = dir;	pu_out_dir[3] = dir;
			pu_out_dir[w] = dir;	pu_out_dir[w + 1] = dir;	pu_out_dir[w + 2] = dir;	pu_out_dir[w + 3] = dir;
			pu_out_dir[2 * w] = dir;	pu_out_dir[2 * w + 1] = dir;	pu_out_dir[2 * w + 2] = dir;	pu_out_dir[2 * w + 3] = dir;
			pu_out_dir[3 * w] = dir;	pu_out_dir[3 * w + 1] = dir;	pu_out_dir[3 * w + 2] = dir;	pu_out_dir[3 * w + 3] = dir;
		}
		pu_in += (w1 - 1 - w_ed);	pu_in_dir += (w1 - 1 - w_ed);
	}
#ifdef DISPLAY	
	ByteImage disImg;
	disImg.width = ori_w;	disImg.height = ori_h;
	disImg.img = ori_Bound;
	Display_Gray_Img(&disImg, "_ori_bound_1st.bmp");
#endif
	w_st *= 4;		w_ed = (w_ed + 1) * 4 - 1;
	h_st *= 4;		h_ed = (h_ed + 1) * 4 - 1;

	int dd, hh, g_step, *ps;
	dd = 10;	hh = 2;
	g_step = dd + hh;
	w_st = max(w_st, g_step);	
	w_ed = min(w_ed, w - 1 - g_step);
	h_st = max(h_st, g_step);
	h_ed = min(h_ed, h - 1 - g_step);
	
	memset(out_Reg, 0, w * h);
	

	pu_in = ori_Bound + h_st * w;
	pu_in_dir = ori_Dir + h_st * w;
	pu_out = out_Reg + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_in += w_st;	pu_in_dir += w_st;	pu_out += w_st;	
		for (j = w_st; j <= w_ed; j++, pu_in++, pu_in_dir++, pu_out++)
		{
			if (*pu_in == 0)
				continue;

			dir = *pu_in_dir;
			ps = Dir_TbXY_Ori_1 + dir * (2 * dd + 1) * (2 * hh + 1);

			for (k = 0; k < (2 * dd + 1) * (2 * hh+1); k++, ps++)
			{
				pu_out[*ps] = 255;				
			}
		}
		pu_in += (w - 1 - w_ed);	pu_in_dir += (w - 1 - w_ed);	pu_out += (w - 1 - w_ed);	
	}
}

void Get_Ori_Bound_using_Seg(uchar* Bound_Reg, uchar* seg_Gray, uchar* out_Bound, int w_st, int h_st, int w_ed, int h_ed)
{
	int w, h;
	w = ori_w;	h = ori_h;

	memset(out_Bound, 0, w * h);

	int i, j, g_step = 1, g_x, g_y, g_x1, g_y1, grad;
	uchar *pu_reg, *pu_in, *pu_out;
	h_st = max(h_st, g_step);	h_ed = min(h_ed, h - 1 - g_step);
	w_st = max(w_st, g_step);	w_ed = min(w_ed, w - 1 - g_step);

	pu_reg = Bound_Reg + h_st * w;
	pu_in = seg_Gray + h_st * w;
	pu_out = out_Bound + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_reg += w_st;	pu_in += w_st;	pu_out += w_st;
		for (j = w_st; j <= w_ed; j++, pu_reg++, pu_in++, pu_out++)
		{
			if (*pu_reg == 0)
				continue;

			if (*pu_in == 0)
				continue;

			if (pu_reg[g_step] == 0 || pu_reg[-g_step] == 0)
				continue;

			if (pu_reg[g_step * w] == 0 || pu_reg[-g_step*w] == 0)
				continue;

			g_x = abs(pu_in[g_step] - pu_in[0]);
			g_y = abs(pu_in[g_step * w] - pu_in[0]);
			g_x1 = abs(pu_in[-g_step] - pu_in[0]);
			g_y1 = abs(pu_in[-g_step * w] - pu_in[0]);
			grad = g_x + g_y + g_x1 + g_y1;

			if (grad < 100)
				continue;

			*pu_out = 255;
		}
		pu_reg += (w - 1 - w_ed);	pu_in += (w - 1 - w_ed);	pu_out += (w - 1 - w_ed);
	}
}

void Get_Final_Bound(PByteImage in_Img, uchar* Bound, int w_st, int h_st, int w_ed, int h_ed){
	int i, j, w, h;
	w = ori_w;	h = ori_h;

	memset(con_Mat, -1, w * h);
	uchar *pu;
	int *pi;

	pu = Bound + h_st * w;
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pi++)
		{
			if (*pu == 0)
				continue;

			if (*pi == -1)
			{

			}
		}
		pu += (w - 1 - w_ed);	pi += (w - 1 - w_ed);
	}
	
}

void Get_Object_Bound(IN PByteImage in_Img, IN uchar* B_Info_Res, OUT uchar* B_Info)
{

#ifdef DISPLAY
	Display_Gray_Img(in_Img, "_ori.bmp");
#endif
	ByteImage res4_Img;
	res4_Img.width = res4_w;	res4_Img.height = res4_h;
	res4_Img.img = res4_Gray;

	ByteImage res16_Img;
	res16_Img.width = res16_w;	res16_Img.height = res16_h;
	res16_Img.img = res16_Gray;

	Extend_Bound(B_Info_Res, res32_w, res32_h);

	Get_Res16_Bound(&res16_Img, B_Info_Res, res16_Bound, res16_Dir);
#ifdef DISPLAY
	Display_Bound_Img(res16_Bound, (char *)"_3rd_Bound.bmp");
#endif

	Erase_Point_Bound(&res16_Img, res16_Bound);
#ifdef DISPLAY
	Display_Bound_Img(B_Info, (char *)"_4th_Bound.bmp");
#endif
	Smooth_Bound(res16_Bound, res16_Dir, res16_w, res16_h);
	
	///////////////////////////////////////////////////////////////////	
	
	Get_Res4_Bound(res16_Bound, &res4_Img, res4_Bound, res4_Dir);

	Erase_Point_Bound(&res4_Img, res4_Bound);

	Smooth_Bound(res4_Bound, res4_Dir, res4_w, res4_h);
#ifdef DISPLAY	
	ByteImage disImg;
	disImg.width = res4_w;	disImg.height = res4_h;
	disImg.img = res4_Bound;
	Display_Gray_Img(&disImg, "_mid_5th_bound.bmp");
#endif
	//////////////////////////////////////////////////////////////////////////////
	int h_st = 0, h_ed = res4_h - 1, w_st = 0, w_ed = res4_w - 1;
	Get_Res4_Range(res4_Bound, &w_st, &h_st, &w_ed, &h_ed);	

	Get_ori_Bound_Region(res4_Bound, res4_Dir, ori_Bound, ori_B_Reg, w_st, h_st, w_ed, h_ed);
#ifdef _DEBUG	
	ByteImage disImg;
	disImg.width = ori_w;	disImg.height = ori_h;
	disImg.img = ori_B_Reg;
	Display_Gray_Img(&disImg, "_ori_range.bmp");
#endif
	
	w_st *= 4;	h_st *= 4;	w_ed = (w_ed + 1) * 4 - 1;	h_ed = (h_ed + 1) * 4 - 1;
	w_st -= 10;		w_ed += 10;
	if (w_st < 0)			w_st = 0;
	if (w_ed > ori_w - 1)	w_ed = ori_w - 1;
	h_st -= 10;		h_ed += 10;
	if (h_st < 0)			h_st = 0;
	if (h_ed > ori_h - 1)	h_ed = ori_h - 1;
	

	Segment_Ori(in_Img, ori_B_Reg, segOri, w_st, h_st, w_ed, h_ed);
#ifdef DISPLAY	
	disImg.img = segOri;
	Display_Gray_Img(&disImg, "_ori_seg.bmp");
#endif
	
	Get_Ori_Bound_using_Seg(ori_B_Reg, segOri, B_Info, w_st, h_st, w_ed, h_ed);
	Erase_Isolated_Points_Ori(B_Info, w_st, h_st, w_ed, h_ed);
#ifdef _DEBUG	
	disImg.img = B_Info;
	Display_Gray_Img(&disImg, "_ori_Bound.bmp");
#endif
}
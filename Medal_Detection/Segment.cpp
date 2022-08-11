#include "stdafx.h"
#include "Bound.h"


extern uchar	ori_tmp_1[], ori_tmp_2[], ori_Bound[];
extern float	Grad_float[];
extern int		con_Mat[], i_list[], j_list[];

int				hist[256], hist1[256], hist2[256];

//int				i_list[(ori_w + 1) * (ori_h + 1)], j_list[(ori_w + 1) * (ori_h + 1)];


int floodfill_4_Neighboor_Res32(int i_pos, int j_pos, uchar *in_Img, int new_color, uchar* out_Img)
{
	int w, h;
	w = res32_w;	h = res32_h;

	int old_color = in_Img[i_pos * w + j_pos];
	if (old_color == new_color)
		return -1;

	int k, leng, cur_j, cur_i, tmp_i, tmp_j;
	int tmp_i_list[4] = { 0, 1, 0, -1 };
	int tmp_j_list[4] = { 1, 0, -1, 0 };
	leng = 0;

	i_list[0] = i_pos;	j_list[0] = j_pos;
	leng++;

	out_Img[i_pos * w + j_pos] = new_color;

	while (leng > 0)
	{
		cur_i = i_list[leng - 1];
		cur_j = j_list[leng - 1];

		leng--;
		out_Img[cur_i * w + cur_j] = new_color;

		for (k = 0; k < 4; k++)
		{
			tmp_i = cur_i + tmp_i_list[k];
			tmp_j = cur_j + tmp_j_list[k];

			if (tmp_i < 0 || tmp_i > h - 1 || tmp_j < 0 || tmp_j > w - 1)
				continue;

			if (in_Img[tmp_i * w + tmp_j] == old_color && out_Img[tmp_i * w + tmp_j] != new_color)
			{
				out_Img[tmp_i * w + tmp_j] = new_color;
				i_list[leng] = tmp_i;
				j_list[leng] = tmp_j;
				leng++;

			}
		}
	}

	return 0;
}

void Double_Threshold_Res32(PByteImage in_Img, uchar* out_Seg)
{
	int i, j, w, h;
	w = in_Img->width;	h = in_Img->height;

	/////////////////////////////////////////////////
	memset(hist, 0, sizeof(int) * 256);
	uchar *pu;

	pu = in_Img->img;
	for (i = 0; i < w * h; i++, pu++)
	{
		hist[pu[0]]++;
	}

	memcpy(hist1, hist, sizeof(int) * 256);
	for (i = 0; i < 255; i++)
	{
		hist1[i + 1] = hist1[i] + hist1[i + 1];
	}

	memcpy(hist2, hist, sizeof(int) * 256);
	for (i = 0; i < 256; i++)
	{
		hist2[i] *= i;
	}

	for (i = 0; i < 255; i++)
	{
		hist2[i + 1] = hist2[i] + hist2[i + 1];
	}

	//////////////// intermeans algorithm for selecting threshold  /////////////////////
	int thr = w * h / 2;
	for (i = 0; i < 256; i++)
	{
		if (hist1[i] >= thr)
			break;
	}

	int flag = 0, t = i;
	float del_t, miu_back, miu_fore, t_n;
	while (flag == 0)
	{
		if (hist1[t] != 0)
		{
			miu_back = hist2[t] / hist1[t];
		}
		else
		{
			miu_back = hist2[t];
		}

		if (hist1[255] - hist1[t] != 0)
		{
			miu_fore = (hist2[255] - hist2[t]) / (hist1[255] - hist1[t]);
		}
		else
		{
			miu_fore = (hist2[255] - hist2[t]);
		}

		t_n = (miu_back + miu_fore) / 2;
		del_t = abs(t - t_n) / t;
		t = floor(t_n + 0.5);

		if (del_t < 1)
			flag = 1;
	}

	int t_high, t_low;
	t_high = (t + 200) / 2;
	t_low = (t + 200) / 4;
	////////////////////////////////////////////////////

	memcpy(ori_tmp_1, in_Img->img, w * h);
	memcpy(ori_tmp_2, in_Img->img, w * h);

	uchar *pu1, *pu2;
	pu1 = ori_tmp_1;	pu2 = ori_tmp_2;
	for (i = 0; i < w * h; i++, pu1++, pu2++)
	{
		if (pu1[0] > t_high)
			pu1[0] = 255;
		else
			pu1[0] = 0;

		if (pu2[0] > t_low)
			pu2[0] = 255;
		else
			pu2[0] = 0;
	}

	memset(out_Seg, 0, w * h);
	int new_color = 254;
	pu = ori_tmp_1;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++, pu++)
		{
			if (*pu == 0)
				continue;

			floodfill_4_Neighboor_Res32(i, j, ori_tmp_2, new_color, out_Seg);
		}
	}
}

void Erase_Small_Block_Res32(uchar* in_Seg)
{
	int w, h;
	w = res32_w;	h = res32_h;

	memset(con_Mat, 0, sizeof(int) * w * h);

	int *pi, i, j, i1, j1, i2, j2, i_st, i_ed, j_st, j_ed, k, flag, nn, min_val, cur_val;
	uchar *pu;

	pu = in_Seg;
	pi = con_Mat;
	nn = 1;
	for (i = 0; i < h; i++)
	{
		i_st = max(i - 1, 0);	i_ed = min(i + 1, h - 1);
		for (j = 0; j < w; j++, pu++, pi++)
		{
			if (*pu == 0)
				continue;

			j_st = max(j - 1, 0);	j_ed = min(j + 1, w - 1);

			flag = 0;	min_val = nn;
			for (i1 = i_st; i1 <= i_ed; i1++)
			{
				i2 = i1 - i;
				for (j1 = j_st; j1 <= j_ed; j1++)
				{
					j2 = j1 - j;
					cur_val = pi[i2 * w + j2];
					if (cur_val == 0)
						continue;

					flag = 1;
					if (min_val > cur_val)
						min_val = cur_val;
				}
			}

			if (flag == 0)
			{
				*pi = nn;
				nn++;
			}
			else
			{
				*pi = min_val;
			}
		}
	}
	////////////////////////////////////////////////////////////////////////
	pi = con_Mat + w * h - 1;
	for (i = h - 1; i >= 0; i--)
	{
		i_st = max(i - 1, 0);	i_ed = min(i + 1, h - 1);
		for (j = w - 1; j >= 0; j--, pi--)
		{
			if (*pi == 0)
				continue;

			j_st = max(j - 1, 0);	j_ed = min(j + 1, w - 1);
			min_val = *pi;
			for (i1 = i_st; i1 <= i_ed; i1++)
			{
				i2 = i1 - i;
				for (j1 = j_st; j1 <= j_ed; j1++)
				{
					j2 = j1 - j;
					cur_val = pi[i2 * w + j2];
					if (cur_val == 0)
						continue;

					if (min_val > cur_val)
						min_val = cur_val;
				}
			}

			*pi = min_val;
		}
	}
	///////////////////////////////////////////////////////////////////////
	pi = con_Mat;
	for (i = 0; i < h; i++)
	{
		i_st = max(i - 1, 0);	i_ed = min(i + 1, h - 1);
		for (j = 0; j < w; j++, pi++)
		{
			if (*pi == 0)
				continue;

			j_st = max(j - 1, 0);	j_ed = min(j + 1, res32_w - 1);
			min_val = *pi;
			for (i1 = i_st; i1 <= i_ed; i1++)
			{
				i2 = i1 - i;
				for (j1 = j_st; j1 <= j_ed; j1++)
				{
					j2 = j1 - j;
					cur_val = pi[i2 * w + j2];
					if (cur_val == 0)
						continue;

					if (min_val > cur_val)
						min_val = cur_val;
				}
			}

			*pi = min_val;
		}
	}

	///////////////////////////////////////////////////////////////////////
	int *leng_vec = new int[nn];
	memset(leng_vec, 0, sizeof(int) * nn);
	pi = con_Mat;
	for (i = 0; i < w * h; i++, pi++)
	{
		if (*pi == 0)
			continue;

		k = *pi;
		leng_vec[k - 1]++;
	}

	int thr = w * h / 10;
	pi = con_Mat;
	pu = in_Seg;
	for (i = 0; i < w * h; i++, pi++, pu++)
	{
		if (*pi == 0)
			continue;

		k = *pi;
		if (leng_vec[k - 1] < thr)
			*pu = 0;
	}
	delete[]	leng_vec;
}

void Segment_32(PByteImage in_Img, uchar* out_Seg)
{
	int w, h;
	w = res32_w;	h = res32_h;

	Double_Threshold_Res32(in_Img, out_Seg);
#ifdef _DEBUG
	ByteImage dis_Img;
	dis_Img.width = w;	dis_Img.height = h;
	dis_Img.img = out_Seg;
	Display_Gray_Img(&dis_Img, "_seg32_1.bmp");
#endif

	int i, j;
	uchar *pu1, *pu2, *pu3;
	pu1 = out_Seg;
	for (i = 0; i < w * h; i++, pu1++)
	{
		if (*pu1 == 0)
			*pu1 = 255;
		else
			*pu1 = 0;
	}

	Erase_Small_Block_Res32(out_Seg);
#ifdef _DEBUG
	Display_Gray_Img(&dis_Img, "_seg32_2.bmp");
#endif

}

int floodfill_4_Neighboor_Ori(int i, int j, uchar *seg_low, uchar* in_reg, int new_color, uchar* out_Seg)
{
	int w, h;
	w = ori_w;	h = ori_h;

	int old_color = seg_low[i * w + j];
	if (old_color == new_color)
		return -1;

	int k, x_leng, y_leng, q_x, q_y;
	x_leng = 0;	y_leng = 0;

	i_list[0] = j;	j_list[0] = i;
	x_leng = 1;	y_leng = 1;

	out_Seg[i * w + j] = new_color;

	while (x_leng > 0 && y_leng > 0)
	{
		q_x = i_list[x_leng - 1];
		q_y = j_list[y_leng - 1];
		x_leng = x_leng - 1;	y_leng = y_leng - 1;
		out_Seg[q_y * w + q_x] = new_color;

		for (k = 0; k < 4; k++)
		{
			if (k == 0 && q_x < w - 1 && in_reg[q_y * w + (q_x + 1)] == 255)
			{
				if (seg_low[q_y * w + (q_x + 1)] == old_color && out_Seg[q_y * w + (q_x + 1)] != new_color)
				{
					out_Seg[q_y * w + (q_x + 1)] = new_color;
					x_leng++;	y_leng++;
					i_list[x_leng - 1] = q_x + 1;
					j_list[y_leng - 1] = q_y;
				}
			}

			if (k == 1 && q_y < h - 1 && in_reg[(q_y + 1)*w + q_x] == 255)
			{
				if (seg_low[(q_y + 1)*w + q_x] == old_color && out_Seg[(q_y + 1)*w + q_x] != new_color)
				{
					out_Seg[(q_y + 1)*w + q_x] = new_color;
					x_leng++;	y_leng++;
					i_list[x_leng - 1] = q_x;
					j_list[y_leng - 1] = q_y + 1;
				}
			}

			if (k == 2 && q_x > 0 && in_reg[q_y * w + (q_x - 1)] == 255)
			{
				if (seg_low[q_y * w + (q_x - 1)] == old_color && out_Seg[q_y * w + (q_x - 1)] != new_color)
				{
					out_Seg[q_y * w + (q_x - 1)] = new_color;
					x_leng++;	y_leng++;
					i_list[x_leng - 1] = q_x - 1;
					j_list[y_leng - 1] = q_y;
				}
			}

			if (k == 3 && q_y > 0 && in_reg[(q_y - 1) * w + q_x] == 255)
			{
				if (seg_low[(q_y - 1) * w + q_x] == old_color && out_Seg[(q_y - 1) * w + q_x] != new_color)
				{
					out_Seg[(q_y - 1) * w + q_x] = new_color;
					x_leng++;	y_leng++;
					i_list[x_leng - 1] = q_x;
					j_list[y_leng - 1] = q_y - 1;
				}
			}
		}
	}

	return 0;
}

void Double_Threshold_Ori(PByteImage in_Img, uchar* in_reg, uchar* out_Seg, int w_st, int h_st, int w_ed, int h_ed)
{

	int i, j, k, w, h, nn, max_val, tmp_val;
	w = in_Img->width;	h = in_Img->height;
#ifdef _DEBUG
	ByteImage dis_Img;
	dis_Img.width = w;	dis_Img.height = h;

	dis_Img.img = in_Img->img;
	Display_Gray_Img(&dis_Img, "_seg_ori_0_0.bmp"); 
	
	dis_Img.img = in_reg;
	Display_Gray_Img(&dis_Img, "_seg_ori_0_1.bmp");
#endif

	uchar *pu_in, *pu_reg, *pu_out;
	int *pi, *pi_reg, ex_w = ori_w + 1, ex_w1 = ex_w + 1;

	///////////////////////////////////////////////////////

	memset(i_list, 0, sizeof(int) * (ori_w + 1) * (ori_h + 1));
	memset(j_list, 0, sizeof(int) * (ori_w + 1) * (ori_h + 1));
	pu_in = in_Img->img + h_st * w;
	pu_reg = in_reg + h_st * w;
	pi = i_list + (h_st + 1) * ex_w;
	pi_reg = j_list + (h_st + 1) * ex_w;
	for (i = h_st; i <= h_ed; i++)
	{
		pi++;	pi_reg++;
		pu_reg += w_st;	pu_in += w_st;	pi += w_st;	pi_reg += w_st;
		for (j = w_st; j <= w_ed; j++, pu_reg++, pu_in++, pi++, pi_reg++)
		{
			if (*pu_reg == 0)
			{
				*pi_reg = pi_reg[-1] + pi_reg[-ex_w] - pi_reg[-ex_w1];
				*pi = pi[-1] + pi[-ex_w] - pi[-ex_w1];
			}
			else
			{
				*pi_reg = pi_reg[-1] + pi_reg[-ex_w] - pi_reg[-ex_w1] + 1;
				*pi = pi[-1] + pi[-ex_w] - pi[-ex_w1] + *pu_in;
			}

		}
		pu_reg += (w - 1 - w_ed);	pu_in += (w - 1 - w_ed);	pi += (w - 1 - w_ed);	pi_reg += (w - 1 - w_ed);
	}

	/////////////////////////////////////////////////
	
	int i1, j1, g_step = 120;
	h_st = max(h_st, g_step);	h_ed = min(h_ed, h - 1 - g_step);
	w_st = max(w_st, g_step);	w_ed = min(w_ed, w - 1 - g_step);

	int pos1, pos2, pos3, pos4;	
	pos1 = g_step * ex_w + g_step;
	pos2 = g_step * ex_w - (g_step + 1);
	pos3 = -(g_step + 1) * ex_w + g_step;
	pos4 = -(g_step + 1) * ex_w - (g_step + 1);

	memset(ori_tmp_1, 0, w * h);
	pu_in = in_Img->img + h_st * w;
	pu_reg = in_reg + h_st * w;
	pu_out = ori_tmp_1 + h_st * w;
	pi = i_list + h_st * ex_w;
	pi_reg = j_list + h_st * ex_w;

	max_val = 0;
	for (i = h_st; i <= h_ed; i++)
	{
		pi++;	pi_reg++;
		pu_in += w_st;	pu_reg += w_st;	pu_out += w_st;	pi += w_st;	pi_reg += w_st;
		for (j = w_st; j <= w_ed; j++, pu_in++, pu_reg++, pu_out++, pi++, pi_reg++)
		{
			if (*pu_reg == 0)
				continue;

			nn = pi_reg[pos1] - pi_reg[pos2] - pi_reg[pos3] + pi_reg[pos4];
			tmp_val = pi[pos1] - pi[pos2] - pi[pos3] + pi[pos4];

			if (nn == 0)
				continue;

			tmp_val /= nn;
			tmp_val = pu_in[0] - tmp_val + 128;
			if (tmp_val < 0)
				tmp_val = 0;
			if (tmp_val > 255)
				tmp_val = 255;
			if (max_val < tmp_val)
				max_val = tmp_val;

			*pu_out = tmp_val;
		}
		pu_in += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);	pu_out += (w - 1 - w_ed);
		pi += (w - 1 - w_ed);	pi_reg += (w - 1 - w_ed);
	}
	

#ifdef _DEBUG
	dis_Img.img = ori_tmp_1;
	Display_Gray_Img(&dis_Img, "_seg_ori_0_2.bmp");
#endif
	//////////////////////////////////////////////////
	memset(Grad_float, 0, sizeof(float) * w * h);
	
	float *pf, max_grad;
	int gx, gy, grad;

	g_step = 15;	
	h_st = max(h_st, g_step);	h_ed = min(h_ed, h - 1 - g_step);
	w_st = max(w_st, g_step);	w_ed = min(w_ed, w - 1 - g_step);

	max_grad = 0;
	pu_in = ori_tmp_1 + h_st * w;
	pu_reg = in_reg + h_st * w;
	pf = Grad_float + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_in += w_st;	pu_reg += w_st;	pf += w_st;
		for (j = w_st; j <= w_ed; j++, pu_in++, pu_reg++, pf++)
		{
			if (*pu_reg == 0)
				continue;

			if (pu_reg[-g_step] == 0 || pu_reg[g_step] == 0)
				continue;

			if (pu_reg[-g_step*w] == 0 || pu_reg[g_step*w] == 0)
				continue;
						
			gx = pu_in[g_step] + pu_in[-g_step] - 2 * pu_in[0];
			if (gx < 0)
				gx = 0;
			gy = pu_in[g_step*w] + pu_in[-g_step * w] - 2 * pu_in[0];
			if (gy < 0)
				gy = 0;

			grad = sqrt(1.0 * gx * gx + gy * gy);

			if (max_grad < grad)
				max_grad = grad;

			*pf = grad;
		}
		pu_in += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);	pf += (w - 1 - w_ed);
	}

	if (max_grad == 0)
		max_grad = 1;


	memset(ori_Bound, 0, w * h);
	pu_in = ori_tmp_1 + h_st * w;
	pf = Grad_float + h_st * w;
	pu_reg = in_reg + h_st * w;
	pu_out = ori_Bound + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_in += w_st;	pf += w_st;	pu_reg += w_st;	pu_out += w_st;
		for (j = w_st; j <= w_ed; j++, pu_in++, pf++, pu_reg++, pu_out++)
		{
			if (*pu_reg == 0)
				continue;

			tmp_val = pu_in[0] - floor(pf[0] * max_val / (2 * max_grad) + 0.5);
			if (tmp_val < 0)
				tmp_val = 0;

			//tmp_val = pu_in[0];

			*pu_out = tmp_val;
		}
		pu_in += (w - 1 - w_ed);	pf += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);	pu_out += (w - 1 - w_ed);
	}
#ifdef _DEBUG
	dis_Img.img = ori_Bound;
	Display_Gray_Img(&dis_Img, "_seg_ori_0_3.bmp");
#endif


	/////////////////////////////////////////////////
	memset(hist, 0, sizeof(int) * 256);
	nn = 0;	max_val = 0;
	pu_in = ori_Bound + h_st * w;
	pu_reg = in_reg + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_in += w_st;	pu_reg += w_st;
		for (j = w_st; j <= w_ed; j++, pu_in++, pu_reg++)
		{
			if (*pu_reg == 0)
				continue;

			if (max_val < pu_in[0])
				max_val = pu_in[0];

			hist[pu_in[0]]++;
			nn++;
		}
		pu_in += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);
	}

	memcpy(hist1, hist, sizeof(int) * 256);
	for (i = 0; i < 255; i++)
	{
		hist1[i + 1] = hist1[i] + hist1[i + 1];
	}

	memcpy(hist2, hist, sizeof(int) * 256);
	for (i = 0; i < 256; i++)
	{
		hist2[i] *= i;
	}

	for (i = 0; i < 255; i++)
	{
		hist2[i + 1] = hist2[i] + hist2[i + 1];
	}


	/////////////////////   intermeans algorithm for selecting threshold	///////
	int thr = nn / 2;
	for (i = 0; i < 256; i++)
	{
		if (hist1[i] >= thr)
			break;
	}
	int flag = 0, t = i;
	float del_t, miu_back, miu_fore, t_n;
	while (flag == 0)
	{
		if (hist1[t] != 0)
		{
			miu_back = hist2[t] / hist1[t];
		}
		else
		{
			miu_back = hist2[t];
		}

		if (hist1[255] - hist1[t] != 0)
		{
			miu_fore = (hist2[255] - hist2[t]) / (hist1[255] - hist1[t]);
		}
		else
		{
			miu_fore = (hist2[255] - hist2[t]);
		}

		t_n = (miu_back + miu_fore) / 2;
		del_t = abs(t - t_n) / t;
		t = floor(t_n + 0.5);

		if (del_t < 1)
			flag = 1;
	}

	int t_high, t_low;

	t_high = (t + max_val) / 2;
	t_low = t_high * 0.7;

	///////////////////////////////////////////

	memcpy(ori_tmp_1, ori_Bound, w * h);
	memcpy(ori_tmp_2, ori_Bound, w * h);

	uchar *pu1, *pu2;
	pu1 = ori_tmp_1 + h_st * w;
	pu2 = ori_tmp_2 + h_st * w;
	pu_reg = in_reg + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_reg += w_st;	pu1 += w_st;	pu2 += w_st;
		for (j = w_st; j <= w_ed; j++, pu1++, pu2++, pu_reg++)
		{
			if (*pu_reg == 0)
				continue;

			if (pu1[0] > t_high)
				pu1[0] = 255;
			else
				pu1[0] = 0;

			if (pu2[0] > t_low)
				pu2[0] = 255;
			else
				pu2[0] = 0;
		}
		pu_reg += (w - 1 - w_ed);	pu1 += (w - 1 - w_ed);	pu2 += (w - 1 - w_ed);
	}

	memset(out_Seg, 0, w * h);
	int new_color = 254;
	pu_in = ori_tmp_1 + h_st * w;
	pu_reg = in_reg + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_in += w_st;	pu_reg += w_st;
		for (j = w_st; j <= w_ed; j++, pu_reg++, pu_in++)
		{
			if (*pu_reg == 0)
				continue;

			if (*pu_in == 0)
				continue;

			floodfill_4_Neighboor_Ori(i, j, ori_tmp_2, in_reg, new_color, out_Seg);
		}
		pu_in += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);
	}
}

void Connect_8_Ori_using_Reg(uchar* Bound, uchar* in_Reg, int* Con_Mat, int i_pos, int j_pos, int label,
	int w_st, int h_st, int w_ed, int h_ed)
{
	int w, h;
	w = ori_w;	h = ori_h;

	int leng = 0;
	uchar *pu, *pu_reg;
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
		pu_reg = in_Reg + cur_i * w + cur_j;

		*pi = label;

		for (k = 0; k < 8; k++)
		{
			next_i = cur_i + di_list[k];	next_j = cur_j + dj_list[k];

			if (next_i < h_st || next_i > h_ed)
				continue;

			if (next_j < w_st || next_j > w_ed)
				continue;

			if (pu_reg[ori_dij[k]] == 0)
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

void Erase_Small_Block_Ori(uchar* in_Seg, uchar* in_Reg, int w_st, int h_st, int w_ed, int h_ed)
{
	int i, j, w, h, cur_label, nn_total;
	w = ori_w;	h = ori_h;

	memset(con_Mat, -1, sizeof(int) * w * h);
	uchar	*pu, *pu_reg;
	int		*pi;

	cur_label = 0;	nn_total = 0;
	pu = in_Seg + h_st * w;
	pu_reg = in_Reg + h_st * w;
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pu_reg += w_st;	 pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pu_reg++, pi++)
		{
			if (*pu_reg == 0)
				continue;

			if (*pu == 0)
				continue;
			
			nn_total++;

			if (*pi == -1)
			{
				Connect_8_Ori_using_Reg(in_Seg, in_Reg, con_Mat, i, j, cur_label, w_st, h_st, w_ed, h_ed);
				cur_label++;
			}
		}
		pu += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);	 pi += (w - 1 - w_ed);
	}

	int* leng_vec = new int[cur_label];
	memset(leng_vec, 0, sizeof(int) * cur_label);

	pu = in_Seg + h_st * w;
	pu_reg = in_Reg + h_st * w;
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pu_reg += w_st;	 pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pu_reg++, pi++)
		{
			if (*pu_reg == 0)
				continue;

			if (*pu == 0)
				continue;

			leng_vec[pi[0]]++;
		}
		pu += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);	 pi += (w - 1 - w_ed);
	}
		
	int thr = nn_total / 600;
	pu = in_Seg + h_st * w;
	pu_reg = in_Reg + h_st * w;
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pu_reg += w_st;	pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pi++, pu_reg++)
		{
			if (*pu_reg == 0)
				continue;

			if (*pu == 0)
				continue;

			if (leng_vec[pi[0]] < thr)
				*pu = 0;
		}
		pu += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);	pi += (w - 1 - w_ed);
	}
	delete[]	leng_vec;
}

void Connect_8_Ori_using_Reg_2nd(uchar* Bound, uchar* in_Reg, int* Con_Mat, int i_pos, int j_pos, int label,
	int w_st, int h_st, int w_ed, int h_ed, int step)
{
	int w, h;
	w = ori_w;	h = ori_h;

	int leng = 0;
	uchar *pu, *pu_reg;
	int *pi;
	i_list[leng] = i_pos;	j_list[leng] = j_pos;
	leng++;

	int di_list[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };
	int dj_list[8] = { 1, 1, 0, -1, -1, -1, 0, 1 };

	int k, cur_i, cur_j, next_i, next_j, step1 = step * w;
	while (leng > 0)
	{
		cur_i = i_list[leng - 1];	cur_j = j_list[leng - 1];
		leng--;

		pu = Bound + cur_i * w + cur_j;
		pi = Con_Mat + cur_i * w + cur_j;
		pu_reg = in_Reg + cur_i * w + cur_j;

		*pi = label;

		for (k = 0; k < 8; k++)
		{
			next_i = cur_i + di_list[k];	next_j = cur_j + dj_list[k];

			if (next_i < h_st || next_i > h_ed)
				continue;

			if (next_j < w_st || next_j > w_ed)
				continue;

			if (pu_reg[ori_dij[k]] == 0)
				continue;

			if (pu_reg[ori_dij[k] - step] == 0 || pu_reg[ori_dij[k] + step] == 0)
				continue;

			if (pu_reg[ori_dij[k] - step1] == 0 || pu_reg[ori_dij[k] + step1] == 0)
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

void Erase_Small_Block_Ori_2nd(uchar* in_Seg, uchar* in_Reg, int w_st, int h_st, int w_ed, int h_ed, int step)
{
	int i, j, w, h, cur_label, nn_total;
	w = ori_w;	h = ori_h;

	memset(con_Mat, -1, sizeof(int) * w * h);
	uchar	*pu, *pu_reg;
	int		*pi, step1 = step * w;

	cur_label = 0;	nn_total = 0;
	h_st = max(h_st, step);	h_ed = min(h_ed, h - 1 - step);
	w_st = max(w_st, step);	w_ed = min(w_ed, w - 1 - step);

	pu = in_Seg + h_st * w;
	pu_reg = in_Reg + h_st * w;
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pu_reg += w_st;	 pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pu_reg++, pi++)
		{
			if (*pu_reg == 0)
				continue;

			if (pu_reg[-step] == 0 || pu_reg[step] == 0)
				continue;

			if (pu_reg[-step1] == 0 || pu_reg[step1] == 0)
				continue;

			if (*pu == 0)
				continue;

			nn_total++;

			if (*pi == -1)
			{
				Connect_8_Ori_using_Reg_2nd(in_Seg, in_Reg, con_Mat, i, j, cur_label, w_st, h_st, w_ed, h_ed, step);
				cur_label++;
			}
		}
		pu += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);	 pi += (w - 1 - w_ed);
	}

	int* leng_vec = new int[cur_label];
	memset(leng_vec, 0, sizeof(int) * cur_label);

	pu = in_Seg + h_st * w;
	pu_reg = in_Reg + h_st * w;
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pu_reg += w_st;	 pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pu_reg++, pi++)
		{
			if (*pu_reg == 0)
				continue;

			if (pu_reg[-step] == 0 || pu_reg[step] == 0)
				continue;

			if (pu_reg[-step1] == 0 || pu_reg[step1] == 0)
				continue;

			if (*pu == 0)
				continue;

			leng_vec[pi[0]]++;
		}
		pu += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);	 pi += (w - 1 - w_ed);
	}

	int thr = nn_total / 600;
	pu = in_Seg + h_st * w;
	pu_reg = in_Reg + h_st * w;
	pi = con_Mat + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu += w_st;	pu_reg += w_st;	pi += w_st;
		for (j = w_st; j <= w_ed; j++, pu++, pi++, pu_reg++)
		{
			if (*pu_reg == 0)
				continue;

			if (pu_reg[-step] == 0 || pu_reg[step] == 0)
				continue;

			if (pu_reg[-step1] == 0 || pu_reg[step1] == 0)
				continue;

			if (*pu == 0)
				continue;

			if (leng_vec[pi[0]] < thr)
				*pu = 0;
		}
		pu += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);	pi += (w - 1 - w_ed);
	}
	delete[]	leng_vec;
}

void Fill_Seg_Blank(uchar* out_Seg, uchar* in_Reg, int w_st, int h_st, int w_ed, int h_ed, int step)
{
	int i, j, k, w, h;
	w = ori_w;	h = ori_h;

	
	uchar *pu_reg, *pu_in, *pu_out;
	int *ps_u, *ps_d, *ps_m;
	int dir, dd, hh, g_step, nn_u, nn_d, sum_u, sum_d, h_step, wid, flag;

	dd = 20;	hh = 0;	h_step = 5;	wid = dd - h_step;
	g_step = dd + hh;
	g_step = max(g_step, step);
	h_st = max(g_step, h_st);	h_ed = min(h_ed, h - 1 - g_step);
	w_st = max(g_step, w_st);	w_ed = min(w_ed, w - 1 - g_step);

	memcpy(ori_tmp_1, out_Seg, w * h);
	memcpy(ori_tmp_2, out_Seg, w * h);
	pu_reg = in_Reg + h_st * w;
	pu_in = ori_tmp_1 + h_st * w;
	pu_out = ori_tmp_2 + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_reg += w_st;	pu_in += w_st;	pu_out += w_st;
		for (j = w_st; j <= w_ed; j++, pu_reg++, pu_in++, pu_out++)
		{
			if (*pu_reg == 0)
				continue;

			if (*pu_in == 0)
				continue;

			flag = 0;
			for (k = 0; k < 8; k++)
			{
				if (pu_in[ori_dij[k]] == 0)
				{
					flag = 1;
					break;
				}
			}

			if (flag == 1)
				*pu_out = 128;
		}
		pu_reg += (w - 1 - w_ed);	pu_in += (w - 1 - w_ed);	pu_out += (w - 1 - w_ed);
	}
	/////////////////////////////////////////////////////////
	int thr = wid * (2 * hh + 1) / 1.9, step1 = step * w;
	pu_reg = in_Reg + h_st * w;
	pu_in = ori_tmp_2 + h_st * w;
	pu_out = out_Seg + h_st * w;
	for (i = h_st; i <= h_ed; i++)
	{
		pu_reg += w_st;	pu_in += w_st;	pu_out += w_st;
		for (j = w_st; j <= w_ed; j++, pu_reg++, pu_in++, pu_out++)
		{
			if (i == 1701 && j == 2511)
				i = i;
			if (*pu_reg == 0)
				continue;

			if (pu_reg[-step] == 0 || pu_reg[step] == 0)
				continue;

			if (pu_reg[-step1] == 0 || pu_reg[step1] == 0)
				continue;

			if (*pu_in != 0)
				continue;

			for (dir = 0; dir < 16; dir++)
			{
				/////////////////////////////////////////////////////////////
				ps_u = Dir_TbXY_Ori_2 + dir * (2 * dd + 1) * (2 * hh + 1);
				nn_u = 0;	sum_u = 0;	
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;
				nn_u += pu_reg[*ps_u];	sum_u += pu_in[*ps_u];	ps_u++;

				if (nn_u < thr * 255)
					continue;

				sum_u /= 255;
				if (sum_u < thr)
					continue;

				ps_d = Dir_TbXY_Ori_2 + dir * (2 * dd + 1) * (2 * hh + 1) + (dd + h_step) * (2 * hh + 1);				
				nn_d = 0;	sum_d = 0;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;
				nn_d += pu_reg[*ps_d];	sum_d += pu_in[*ps_d];	ps_d++;

				if (nn_d < thr * 255)
					continue;

				sum_d /= 255;
				if (sum_d < thr)
					continue;

				*pu_out = 255;
				break;
			}
		}
		pu_reg += (w - 1 - w_ed);	pu_in += (w - 1 - w_ed);	pu_out += (w - 1 - w_ed);
	}
}

void Segment_Ori(PByteImage in_Img, uchar* in_Reg, uchar* out_Seg, int w_st, int h_st, int w_ed, int h_ed)
{
	int w, h;
	w = ori_w;	h = ori_h;

	Double_Threshold_Ori(in_Img, in_Reg, out_Seg, w_st, h_st, w_ed, h_ed);
#ifdef _DEBUG
	ByteImage dis_Img;
	dis_Img.width = w;	dis_Img.height = h;
	dis_Img.img = out_Seg;
	Display_Gray_Img(&dis_Img, "_seg_ori_1.bmp");
#endif

	Erase_Small_Block_Ori(out_Seg, in_Reg, w_st, h_st, w_ed, h_ed);
#ifdef _DEBUG
	dis_Img.img = out_Seg;
	Display_Gray_Img(&dis_Img, "_seg_ori_2.bmp");
#endif
	
	int step = 10;
	Fill_Seg_Blank(out_Seg, in_Reg, w_st, h_st, w_ed, h_ed, step);
#ifdef _DEBUG
	dis_Img.img = out_Seg;
	Display_Gray_Img(&dis_Img, "_seg_ori_3.bmp");
#endif

	int i, j;
	uchar *pu_seg, *pu_reg;
	pu_seg = out_Seg + h_st * w;
	pu_reg = in_Reg + h_st * w;

	for (i = h_st; i <= h_ed; i++)
	{
		pu_seg += w_st;	pu_reg += w_st;
		for (j = w_st; j <= w_ed; j++, pu_reg++, pu_seg++)
		{
			if (*pu_reg == 0)
				continue;

			*pu_seg = 255 - pu_seg[0];
			if (*pu_seg < 255)
				*pu_seg = 0;
		}
		pu_seg += (w - 1 - w_ed);	pu_reg += (w - 1 - w_ed);
	}

	step = 10;
	Erase_Small_Block_Ori_2nd(out_Seg, in_Reg, w_st, h_st, w_ed, h_ed, step);
#ifdef _DEBUG
	dis_Img.img = out_Seg;
	Display_Gray_Img(&dis_Img, "_seg_ori_4.bmp");
#endif
}
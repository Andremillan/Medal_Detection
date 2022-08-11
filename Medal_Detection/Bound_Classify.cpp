
#include "stdafx.h"
#include "Bound.h"


int				tranx_list[2 * Feat_dim], trany_list[2 * Feat_dim];
int				tmpx_list[2 * Feat_dim], tmpy_list[2 * Feat_dim];
int				inx_list[2 * Feat_dim], iny_list[2 * Feat_dim], inflag_list[2 * Feat_dim];
int				outx_list[2 * Feat_dim], outy_list[2 * Feat_dim], outflag_list[2 * Feat_dim];
	
extern int			con_Mat[], i_list[], j_list[];
extern uchar		ori_tmp_1[];


int Get_Object_Center(uchar* in_Bound, float* c_x, float* c_y)
{
	int i, j, w, h, nn;
	w = res32_w;	h = res32_h;

	float sum_x, sum_y;
	uchar* pu;

	pu = in_Bound;
	nn = 0;	sum_x = 0;	sum_y = 0;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++, pu++)
		{
			if (*pu == 0)
				continue;

			sum_y += i;
			sum_x += j;
			nn++;
		}
	}

	if (nn < 100)
		return -1;

	float x_pos, y_pos;
	x_pos = sum_x / nn;
	y_pos = sum_y / nn;

	if (x_pos < 0)		x_pos = 0;
	if (x_pos > w - 1)	x_pos = w - 1;
	if (y_pos < 0)		y_pos = 0;
	if (y_pos > h - 1)	y_pos = h - 1;

	*c_x = x_pos;
	*c_y = y_pos;

	return 0;
}

void Get_Min_Max_Radius(uchar* in_Bound, int c_x, int c_y, int* d_list)
{
	memset(d_list, 0, sizeof(int) * 2);

	int i, j, w, h, cur_x, cur_y;
	w = res32_w;	h = res32_h;

	float min_dist, max_dist, cur_dist;
	min_dist = w + h;	max_dist = 0;

	uchar* pu = in_Bound;
	for (i = 0; i < h; i++)
	{
		cur_y = i;
		for (j = 0; j < w; j++, pu++)
		{
			if (*pu == 0)
				continue;

			cur_x = j;
			cur_dist = sqrt(1.0*(c_x - cur_x)*(c_x - cur_x) + (c_y - cur_y)*(c_y - cur_y));

			if (min_dist > cur_dist)
				min_dist = cur_dist;

			if (max_dist < cur_dist)
				max_dist = cur_dist;
		}
	}

	min_dist -= 2;
	if (min_dist < 0)
		min_dist = 0;

	max_dist += 2;
	d_list[0] = floor(min_dist + 0.5);
	d_list[1] = floor(max_dist + 0.5);
}


void Get_Res32_Bound(uchar* in_Gray, uchar* out_Bound)
{
	int w, h;
	w = res32_w;	h = res32_h;

	int i, j, k, g_x1, g_y1, g_x2, g_y2, grad, nn, g_step = 1, grad_thr = 100;
	

	uchar *pu_in, *pu_out;
	memcpy(ori_tmp_1, in_Gray, w * h);
	pu_in = in_Gray + g_step * w;
	pu_out = ori_tmp_1 + g_step * w;
	for (i = g_step; i < h - g_step; i++)
	{
		pu_in += g_step;	pu_out += g_step;
		for (j = g_step; j < w - g_step; j++, pu_in++, pu_out++)
		{
			if (*pu_in == 0)
				continue;

			nn = 0;
			for (k = 0; k < 8; k++)
			{
				if (pu_in[res32_dij[k]] == 0)
					continue;

				nn++;
			}

			if (nn < 4)
				*pu_out = 0;
		}
		pu_in += g_step;	pu_out += g_step;
	}


	memset(out_Bound, 0, w * h);
	pu_in = ori_tmp_1 + g_step * w;
	pu_out = out_Bound + g_step * w;
	for (i = g_step; i < h - g_step; i++)
	{
		pu_in += g_step;	pu_out += g_step;
		for (j = g_step; j < w - g_step; j++, pu_in++, pu_out++)
		{
			if (*pu_in == 0)
				continue;

			g_x1 = abs(pu_in[g_step] - pu_in[0]);
			g_y1 = abs(pu_in[g_step * w] - pu_in[0]);
			g_x2 = abs(pu_in[-g_step] - pu_in[0]);
			g_y2 = abs(pu_in[-g_step * w] - pu_in[0]);
			grad = g_x1 + g_y1 + g_x2 + g_y2;
			if (grad < grad_thr)
				continue;

			*pu_out = 255;
		}
		pu_in += g_step;	pu_out += g_step;
	}
}

void Connect_8_Res32(uchar* Bound, int* Con_Mat, int i_pos, int j_pos, int label)
{
	int w, h;
	w = res32_w;	h = res32_h;

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

			if (next_i < 0 || next_i > h - 1)
				continue;

			if (next_j < 0 || next_j > w - 1)
				continue;

			if (pu[res32_dij[k]] == 0)
				continue;

			if (pi[res32_dij[k]] != -1)
				continue;

			pi[res32_dij[k]] = label;

			i_list[leng] = next_i;	j_list[leng] = next_j;
			leng++;
		}
	}
}

void Erase_Isolated_Points_Res32(uchar* Bound)
{
	int i, j, w, h, cur_label;
	w = res32_w;	h = res32_h;

	memset(con_Mat, -1, sizeof(int) * w * h);

	uchar *pu;
	int *pi;
	
	cur_label = 0;
	pu = Bound;	pi = con_Mat;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++, pu++, pi++)
		{
			if (*pu == 0)
				continue;

			if (*pi == -1)
			{
				Connect_8_Res32(Bound, con_Mat, i, j, cur_label);
				cur_label++;
			}
		}
	}


	int *leng_vec = new int[cur_label];
	memset(leng_vec, 0, sizeof(int) * cur_label);

	pu = Bound;	pi = con_Mat;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++, pu++, pi++)
		{
			if (*pu == 0)
				continue;

			leng_vec[pi[0]]++;
		}
	}

	pu = Bound;	pi = con_Mat;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++, pu++, pi++)
		{
			if (*pu == 0)
				continue;

			if (leng_vec[pi[0]] < 50)
				*pu = 0;
		}
	}
}

int Get_Object_Area(uchar* Range)
{
	int i, w, h, area;
	uchar *pu;
	w = res32_w;	h = res32_h;

	area = 0;
	pu = Range;
	for (i = 0; i < w * h; i++, pu++)
	{
		if (*pu == 0)
			continue;

		area++;
	}

	return area;
}

int Get_Object_Length(uchar* B_Info)
{
	int i, w, h, leng;
	w = res32_w;	h = res32_h;

	uchar *pu_in;
	
	leng = 0;
	pu_in = B_Info;
	for (i = 0; i < w * h; i++, pu_in++)
	{
		if (*pu_in == 0)
			continue;

		leng++;
	}

	return leng;
}

void Get_Circle_Feature(uchar* in_Bound, int c_x, int c_y, int* d_list, int* x_list, int* y_list, int* flag_list)
{
	int w, h;
	w = res32_w;	h = res32_h;

	memset(x_list, 0, sizeof(int) * (2 * Feat_dim));
	memset(y_list, 0, sizeof(int) * (2 * Feat_dim));
	memset(flag_list, 0, sizeof(int) * (2 * Feat_dim));

	float min_r, max_r, rr, step, alp, cc, ss;
	min_r = d_list[0];	max_r = d_list[1];
	min_r = (max_r + min_r) / 2 - 2;
	if (min_r < 1)
		min_r = 1;

	step = PI / Feat_dim;

	int i, j, k, flag, cur_x, cur_y, x_val, y_val, final_x, final_y;
	for (k = 0; k < Feat_dim; k++)
	{
		alp = (k - 1) * step;
		cc = cos(alp);	ss = sin(alp);
		flag = 0;
		for (rr = max_r; rr >= min_r; rr--)
		{
			if (flag == 1)
				break;

			cur_x = floor(c_x + rr * cc + 0.5);
			cur_y = floor(c_y + rr * ss + 0.5);

			if (cur_x < 0 || cur_x > w - 1 || cur_y < 0 || cur_y > h - 1)
				continue;

			if (in_Bound[cur_y * w + cur_x] != 0)
			{
				flag = 1;
				final_x = cur_x;
				final_y = cur_y;
				break;
			}
			else
			{
				for (i = -1; i <= 1; i++)
				{
					if (flag == 1)
						break;

					for (j = -1; j <= 1; j++)
					{
						x_val = cur_x + j;
						y_val = cur_y + i;

						if (x_val < 0 || x_val > w - 1 || y_val < 0 || y_val > w - 1)
							continue;

						if (in_Bound[y_val * w + x_val] != 0)
						{
							flag = 1;
							final_x = x_val;
							final_y = y_val;
							break;
						}
					}
				}
			}
		}

		if (flag == 1)
		{
			x_list[k] = final_x;
			y_list[k] = final_y;
			flag_list[k] = 1;
		}

		alp += PI;
		cc = cos(alp);	ss = sin(alp);
		flag = 0;
		for (rr = max_r; rr >= min_r; rr--)
		{
			if (flag == 1)
				break;

			cur_x = floor(c_x + rr * cc + 0.5);
			cur_y = floor(c_y + rr * ss + 0.5);

			if (cur_x < 0 || cur_x > w - 1 || cur_y < 0 || cur_y > h - 1)
				continue;

			if (in_Bound[cur_y * w + cur_x] != 0)
			{
				flag = 1;
				final_x = cur_x;
				final_y = cur_y;
				break;
			}
			else
			{
				for (i = -1; i <= 1; i++)
				{
					if (flag == 1)
						break;

					for (j = -1; j <= 1; j++)
					{
						x_val = cur_x + j;
						y_val = cur_y + i;

						if (x_val < 0 || x_val > w - 1 || y_val < 0 || y_val > h - 1)
							continue;

						if (in_Bound[y_val * w + x_val] != 0)
						{
							flag = 1;
							final_x = x_val;
							final_y = y_val;
							break;
						}
					}
				}
			}
		}

		if (flag == 1)
		{
			x_list[k + Feat_dim] = final_x;
			y_list[k + Feat_dim] = final_y;
			flag_list[k + Feat_dim] = 1;
		}
	}
}

void Make_Feature(BoundFeat* out_feat, int cx, int cy, int leng, int area, int* r_list, int* x_list, int* y_list, int* flag_list)
{
	memset(out_feat, 0, sizeof(BoundFeat));

	out_feat->Enroll_Num = 1;
	out_feat->cx = cx;
	out_feat->cy = cy;
	out_feat->Length = leng;
	out_feat->Area = area;
	out_feat->min_r = r_list[0];
	out_feat->max_r = r_list[1];

	memcpy(out_feat->x_list, x_list, sizeof(int) * (2 * Feat_dim));
	memcpy(out_feat->y_list, y_list, sizeof(int) * (2 * Feat_dim));
	memcpy(out_feat->flag_list, flag_list, sizeof(int) * (2 * Feat_dim));
}

int Get_Feature(IN uchar* B_Range, OUT uchar* B_Info, OUT BoundFeat* out_feat)
{
#ifdef _DEBUG
	ByteImage disImg;
	disImg.width = res32_w;	disImg.height = res32_h;
	disImg.img = B_Range;

	Display_Gray_Img(&disImg, "_32_seg.bmp");
#endif
	int area;
	area = Get_Object_Area(B_Range);

	Get_Res32_Bound(B_Range, B_Info);
	Erase_Isolated_Points_Res32(B_Info);
#ifdef _DEBUG
	disImg.img = B_Info;
	Display_Gray_Img(&disImg, "_32_bound.bmp");
#endif

	float c_x, c_y;
	c_x = 0;	c_y = 0;
	int cen_flag = Get_Object_Center(B_Range, &c_x, &c_y);
	if (cen_flag == -1)
		return cen_flag;

	int leng;
	leng = Get_Object_Length(B_Info);

	int r_list[2];
	Get_Min_Max_Radius(B_Info, c_x, c_y, r_list);

	int x_list[2 * Feat_dim], y_list[2 * Feat_dim], flag_list[2 * Feat_dim];
	Get_Circle_Feature(B_Info, c_x, c_y, r_list, x_list, y_list, flag_list);


	Make_Feature(out_feat, c_x, c_y, leng, area, r_list, x_list, y_list, flag_list);

	return 0;
}

int Compare_Length(BoundFeat* mat_feat, BoundFeat* reg_feat)
{
	int mat_leng, reg_leng, thr;

	mat_leng = mat_feat->Length;
	reg_leng = reg_feat->Length;

	thr = min(mat_leng, reg_leng) / 4;
	if (abs(mat_leng - reg_leng) > thr)
		return -1;

	return 0;
}

int Compare_Area(BoundFeat* mat_feat, BoundFeat* reg_feat)
{
	int mat_area, reg_area, thr;
	
	mat_area = mat_feat->Area;
	reg_area = reg_feat->Area;

	thr = min(mat_area, reg_area) / 20;
	if (abs(mat_area - reg_area) > thr)
		return -1;

	return 0;
}

int Compare_Radius(BoundFeat* mat_feat, BoundFeat* reg_feat)
{
	int mat_min, mat_max, reg_min, reg_max;
	mat_min = mat_feat->min_r;
	mat_max = mat_feat->max_r;
	reg_min = reg_feat->min_r;
	reg_max = reg_feat->max_r;

	int min_thr, max_thr;
	min_thr = max(min(mat_min, reg_min) / 4, 5);
	max_thr = max(min(mat_max, reg_max) / 4, 5);

	if (abs(mat_min - reg_min) > min_thr || abs(mat_max - reg_max) > max_thr)
		return -1;

	return 0;
}

float Estimate_Rotate(BoundFeat* mat_feat, BoundFeat* reg_feat, int* tranx_list, int* trany_list)
{
	memset(tranx_list, 0, sizeof(int) * (2 * Feat_dim));
	memset(trany_list, 0, sizeof(int) * (2 * Feat_dim));

	float mat_cx, mat_cy, reg_cx, reg_cy;
	mat_cx = mat_feat->cx;	mat_cy = mat_feat->cy;
	reg_cx = reg_feat->cx;	reg_cy = reg_feat->cy;

	memcpy(outx_list, reg_feat->x_list, sizeof(int) * (2 * Feat_dim));
	memcpy(outy_list, reg_feat->y_list, sizeof(int) * (2 * Feat_dim));
	memcpy(outflag_list, reg_feat->flag_list, sizeof(int) * (2 * Feat_dim));

	int k, kk;
	for (k = 0; k < 2 * Feat_dim; k++)
	{		
		inx_list[k] = mat_feat->x_list[k] - mat_cx + reg_cx;
		iny_list[k] = mat_feat->y_list[k] - mat_cy + reg_cy;
	}
	memcpy(inflag_list, mat_feat->flag_list, sizeof(int) * (2 * Feat_dim));

	float min_val = 1000000, min_alp = 0, cur_val, cur_dist;
	int min_index = -1, k_in, k_out, in_x, in_y, out_x, out_y, tran_x, tran_y, nn;
	float val1, val2, alp, cc, ss;
	int nn_thr;
	nn_thr = (2 * Feat_dim) * 4 / 5;

	for (kk = 0; kk < 2 * Feat_dim - 1; kk++)
	{
		val1 = 0;	val2 = 0;	nn = 0;
		for (k = 0; k < 2 * Feat_dim; k++)
		{
			k_in = k + kk;
			if (k_in >= 2 * Feat_dim)
				k_in -= 2 * Feat_dim;
			k_out = k;

			if (inflag_list[k_in] == 0 || outflag_list[k_out] == 0)
				continue;

			nn++;

			in_x = inx_list[k_in];		in_y = iny_list[k_in];
			out_x = outx_list[k_out];	out_y = outy_list[k_out];

			val1 += ((in_x - reg_cx)*(out_x - reg_cx) + (in_y - reg_cy)*(out_y - reg_cy));
			val2 += ((out_y - reg_cy)*(in_x - reg_cx) - (out_x - reg_cx)*(in_y - reg_cy));
		}

		if (nn < nn_thr)
			continue;

		alp = atan2(val2, val1);
		if (alp < 0)
			alp += 2 * PI;

		cc = cos(alp);	ss = sin(alp);
		cur_val = 0;	
		for (k = 0; k < 2 * Feat_dim; k++)
		{
			k_in = k + kk;
			if (k_in >= 2 * Feat_dim)
				k_in -= 2 * Feat_dim;
			k_out = k;

			if (inflag_list[k_in] == 0 || outflag_list[k_out] == 0)
				continue;			

			in_x = inx_list[k_in];		in_y = iny_list[k_in];
			out_x = outx_list[k_out];	out_y = outy_list[k_out];

			tran_x = floor((in_x - reg_cx) * cc - (in_y - reg_cy) * ss + reg_cx + 0.5);
			tran_y = floor((in_x - reg_cx) * ss + (in_y - reg_cy) * cc + reg_cy + 0.5);

			cur_dist = sqrt(1.0 * (tran_x - out_x)*(tran_x - out_x) + (tran_y - out_y)*(tran_y - out_y));
			cur_val += cur_dist;
		}
				
		cur_val /= nn;

		if (min_val > cur_val)
		{
			min_val = cur_val;
			min_index = kk;
			min_alp = alp;
		}
	}

	//cc = cos(min_alp);	ss = sin(min_alp);
	//for (k = 0; k < 2 * Feat_dim; k++)
	//{
	//	k_in = k + min_index;
	//	if (k_in >= 2 * Feat_dim)
	//		k_in -= 2 * Feat_dim;

	//	in_x = inx_list[k_in];		in_y = iny_list[k_in];

	//	tran_x = floor((in_x - reg_cx) * cc - (in_y - reg_cy) * ss + reg_cx + 0.5);
	//	tran_y = floor((in_x - reg_cx) * ss + (in_y - reg_cy) * cc + reg_cy + 0.5);
	//	if (tran_x < 0 || tran_x > res32_w - 1 || tran_y < 0 || tran_y > res32_h - 1)
	//		continue;

	//	tranx_list[k] = tran_x;
	//	trany_list[k] = tran_y;
	//}

	return min_val;
}

int Classify_Bound(BoundFeat* mat_feat, std::vector<BoundFeat>& reg_Feats, float* outx_list, float* outy_list)
{
	int reg_Num = reg_Feats.size();
	if (reg_Num == 0)
		return -1;
	
	int k, leng_flag, area_flag, rad_flag;
	float min_val = 10000, cur_val;
	int	min_index = -1;
	for (k = 0; k < reg_Num; k++)
	{
		BoundFeat cur_reg = reg_Feats.at(k);
		leng_flag = Compare_Length(mat_feat, &cur_reg);
		if (leng_flag == -1)
			continue;

		area_flag = Compare_Area(mat_feat, &cur_reg);
		if (area_flag == -1)
			continue;

		rad_flag = Compare_Radius(mat_feat, &cur_reg);
		if (rad_flag == -1)
			continue;

		cur_val = Estimate_Rotate(mat_feat, &cur_reg, tranx_list, trany_list);

		if (min_val > cur_val)
		{
			min_val = cur_val;
			min_index = k;
			memcpy(outx_list, tranx_list, sizeof(float) * (2 * Feat_dim));
			memcpy(outy_list, trany_list, sizeof(float) * (2 * Feat_dim));
		}
	}

	if (min_val < Thr_Val)
		return min_index + 1;
	else
		return -1;
}

int Enroll(BoundFeat* in_feat, uchar* B_Info, std::vector<BoundFeat>& reg_Feats, char* out_path)
{
	int w, h;
	w = res32_w;	h = res32_h;

	BoundFeat cur_feat;
	memcpy(&cur_feat, in_feat, sizeof(BoundFeat));
	reg_Feats.push_back(cur_feat);
	int count_Num = reg_Feats.size();

	///////////////////////////////////////////////////////////
	char fullfeatname[256];
	char featname[100];


	memset(fullfeatname, 0, 256);
	memset(featname, 0, 100);

	_itoa_s(count_Num, featname, 10);
	strcat_s(featname, ".dat");
	memcpy(fullfeatname, out_path, strlen(out_path));
	memcpy(fullfeatname + strlen(out_path), featname, strlen(featname));

	FILE *fp;
	if (fopen_s(&fp, fullfeatname, "wb") == 0)
	{
		fwrite(in_feat, sizeof(BoundFeat), 1, fp);
		fclose(fp);
	}

	///////////////////////////////////////////////////////////

	char fullimgname[256];
	char imgname[100];

	memset(fullimgname, 0, 256);
	memset(imgname, 0, 100);

	_itoa_s(count_Num, imgname, 10);
	strcat_s(imgname, ".bmp");
	memcpy(fullimgname, out_path, strlen(out_path));
	memcpy(fullimgname + strlen(out_path), imgname, strlen(imgname));


	memset(ori_tmp_1, 0, w * h);
	uchar *pu_in, *pu_out;
	pu_in = B_Info;	pu_out = ori_tmp_1;
	for (int i = 0; i < w * h; i++, pu_in++, pu_out++)
	{
		if (*pu_in == 0)
			continue;

		*pu_out = 255;
	}

	ByteImage FeatImg;
	FeatImg.width = w;	FeatImg.height = h;
	FeatImg.img = ori_tmp_1;

	save_Gray_Img(&FeatImg, fullimgname);

	return	count_Num;
}

int Enroll_Update(std::vector<BoundFeat>& reg_Feats, int c_id, int* inx_list, int* iny_list)
{
	int w, h;
	w = res32_w;	h = res32_h;

	BoundFeat cur_feat;
	cur_feat = reg_Feats.at(c_id);

	int reg_Num = cur_feat.Enroll_Num;
	memcpy(tmpx_list, cur_feat.x_list, sizeof(int) * (2 * Feat_dim));
	memcpy(tmpy_list, cur_feat.y_list, sizeof(int) * (2 * Feat_dim));

	int k, reg_x, reg_y, in_x, in_y;
	for (k = 0; k < 2 * Feat_dim; k++)
	{
		in_x = inx_list[k];	in_y = iny_list[k];
		reg_x = tmpx_list[k];	reg_y = tmpy_list[k];

		reg_x = floor(1.0 * (reg_x*reg_Num + in_x) / (reg_Num + 1) + 0.5);
		if (reg_x < 0 || reg_x > w - 1)
			reg_x = 0;

		reg_y = floor(1.0 * (reg_y*reg_Num + in_y) / (reg_Num + 1) + 0.5);
		if (reg_y < 0 || reg_y > h - 1)
			reg_y = 0;

		tmpx_list[k] = reg_x;
		tmpy_list[k] = reg_y;
	}

	memcpy(cur_feat.x_list, tmpx_list, sizeof(int) * (2 * Feat_dim));
	memcpy(cur_feat.y_list, tmpy_list, sizeof(int) * (2 * Feat_dim));
	
	reg_Num += 1;
	if (reg_Num > 100)
		reg_Num = 100;
	cur_feat.Enroll_Num = reg_Num;

	reg_Feats.at(c_id) = cur_feat;
	//memcpy(&reg_Feats.at(c_id), &cur_feat, sizeof(BoundFeat));
	return c_id + 1;
}

void save_update_Templates(std::vector<BoundFeat>& reg_Feats, char* out_path)
{
	char fullfeatname[256];
	char featname[100];

	for (int i = 0; i < reg_Feats.size(); i++)
	{
		memset(fullfeatname, 0, 256);
		memset(featname, 0, 100);

		BoundFeat cur_feat = reg_Feats.at(i);

		_itoa_s(i + 1, featname, 10);
		strcat_s(featname, ".dat");
		memcpy(fullfeatname, out_path, strlen(out_path));
		memcpy(fullfeatname + strlen(out_path), featname, strlen(featname));

		FILE *fp;
		if (fopen_s(&fp, fullfeatname, "wb") == 0)
		{
			fwrite(&cur_feat, sizeof(BoundFeat), 1, fp);
			fclose(fp);
		}
	}

}
#pragma once
#include <windows.h>
#include <atlimage.h>
#include <vector>

#include "TimeMeasure.h"

typedef	unsigned char uchar;


#define		SFE_OK								0
#define		SFE_FAILED							-1
#define		SFE_NOT_INITIALIZED					-2
#define		SFE_INVALID_ARGUMENT				-3
#define		SFE_OUT_OF_MEMORY					-5
#define		SFE_FILEIO_ERROR					-6
#define		SFE_LOCK_ERROR						-7
#define		SFE_DEV_ERR							-11
#define		SFE_ALGVERSION_ERR					-12
#define		SFE_UKEDLL_NOTFOUND					-20
#define		SFE_UKE_ERROR					    -100
#define		ErrorDebug							1

#define		R_FACTOR							0.29893602129378
#define		G_FACTOR							0.58704307445112
#define		B_FACTOR							0.11402090425510
#define		PI									3.141592


#define		ori_w								4024
#define		ori_h								3036

#define		res4_w								1006
#define		res4_h								756

#define		res16_w								251
#define		res16_h								189

#define		res32_w								125
#define		res32_h								94

#define		Feat_dim							30
#define		Thr_Val								2.5		//3



typedef struct _byteImage
{
	short						width;
	short						height;
	BYTE						*img;
}ByteImage, *PByteImage;

typedef struct _byteColorImage
{
	short						width;
	short						height;
	BYTE						*R_img;
	BYTE						*G_img;
	BYTE						*B_img;
}ByteColorImage, *PByteColorImage;

typedef struct _boundFeat
{
	int			Enroll_Num;
	int			Area;
	int			Length;
	int			cx;
	int			cy;
	int			min_r;
	int			max_r;
	int			x_list[2 * Feat_dim];
	int			y_list[2 * Feat_dim];
	int			flag_list[2 * Feat_dim];
}BoundFeat;

extern	int		res32_dij[];

extern	int		Dir_TbXY_Res16[];
extern	int		Ver_TbXY_Res16[];
extern	int		res16_dij[];

extern	int		Dir_TbXY_Res4[];
extern	int		Ver_TbXY_Res4[];

extern	int		Dir_TbXY_Ori_1[];
extern	int		Dir_TbXY_Ori_2[];
extern	int		Dir_TbXY_Ori_3[];

extern  int     ori_dij[];

extern int		CC2_T[];
extern int		SS2_T[];


void *			alloc_mem(int p_nSize);
void			free_mem(void *  p_lpUserData);
void *			alloc_mem(int p_nSize);
void			free_mem(void *  p_lpUserData);

int				LoadJpeg(PByteImage image, char *file_name);
int				LoadBmp(PByteImage image, LPTSTR strFileName);
int				SaveAsBmp(PByteImage image, char *file_name);
int				SaveColorImgAsBmp(PByteColorImage pImage, char *file_name);
int				LoadColorImageFile(PByteColorImage in_img, CString file_name);
void			ColorImageFree(PByteColorImage image);
void			save_Gray_Img(PByteImage in_Img, char* str);

BOOL			CreateMuliteDirectory(CString P);


void			Get_Gray(PByteColorImage in_Img, PByteImage out_Img);
int				Detect_Object_Range(IN PByteImage in_Img, OUT uchar* B_Range);
int				Get_Feature(IN uchar* B_Range, OUT uchar* B_Info, OUT BoundFeat* feat);
int				Classify_Bound(BoundFeat* mat_feat, std::vector<BoundFeat>& reg_Feats, float* outx_list, float* outy_list);
int				Enroll(BoundFeat* feat, uchar* B_Info, std::vector<BoundFeat>& reg_Feats, char* out_path);
int				Enroll_Update(std::vector<BoundFeat>& reg_Feats, int c_id, int* inx_list, int* iny_list);
void			Get_Object_Bound(IN PByteImage in_Img, IN uchar* B_Info_Res, OUT uchar* B_Info);
void			save_update_Templates(std::vector<BoundFeat>& reg_Feats, char* out_path);
void			Segment_32(PByteImage in_Img, uchar* out_Seg);
void			Segment_Ori(PByteImage in_Img, uchar* in_Reg, uchar* out_Seg, int w_st, int h_st, int w_ed, int h_ed);

void			save_Gray_Img(PByteImage in_Img, char* str);
void			Display_Gray_Img(PByteImage in_Img, char* str);
void			Display_Bound_Img(uchar* bound, char* str);
void			Save_Bound_Img(PByteColorImage in_Img, uchar* bound, int w, int h, char* path_str, char* name_str);
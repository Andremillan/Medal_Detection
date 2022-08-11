
#include "stdafx.h"
#include <io.h>

#include "Bound.h"



#define		SAVE_RESULT

char			Debug_Name[256];
uchar			Object_Range[res32_w * res32_h];
uchar			Bound_Res[res32_w * res32_h];
uchar			Bound_Info[ori_w * ori_h];
uchar			ori_Gray[ori_w * ori_h];
float			matx_list[2 * Feat_dim], maty_list[2 * Feat_dim];

int Detect_Bound(char *filepath)
{
	FILE* fp;
	int i;
	ByteColorImage	in_Img;
	struct _finddata_t fileinfo;
	intptr_t handle;

	char FilePath[250];
	char FilePath2[250];
	char DebugPath[250];
	char Detect_Path[250];
	char Result_Path[250];
	char Enroll_Path[250];
	char Enroll_Path2[250];

	char fullimgname[256];
	char imgname[100];

	char fullfeatname[256];
	char featname[100];

	strcpy_s(FilePath, filepath);
	strcat_s(FilePath, "\\in\\*.*");

	strcpy_s(FilePath2, filepath);
	strcat_s(FilePath2, "\\in\\");

	strcpy_s(DebugPath, filepath);
	strcat_s(DebugPath, "\\Debug\\");

	strcpy_s(Detect_Path, filepath);
	strcat_s(Detect_Path, "\\Detect\\");

	strcpy_s(Enroll_Path, filepath);
	strcat_s(Enroll_Path, "\\Template\\*.*");

	strcpy_s(Enroll_Path2, filepath);
	strcat_s(Enroll_Path2, "\\Template\\");

	/////////////////Read Registered Templates//////////////////////////
	CreateMuliteDirectory(Enroll_Path2);

	std::vector<BoundFeat> w_rFeatureStack;
	BoundFeat reg_feat;
	handle = _findfirst(Enroll_Path, &fileinfo);
	if (-1 == handle)
		return -1;

	while (!_findnext(handle, &fileinfo))
	{
		if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
		{
			continue;
		}

		if (strstr(fileinfo.name, ".dat"))
		{
			memset(fullfeatname, 0, 256);
			memset(featname, 0, 100);

			strcpy_s(featname, fileinfo.name);
			memcpy(fullfeatname, Enroll_Path2, strlen(Enroll_Path2));
			memcpy(fullfeatname + strlen(Enroll_Path2), featname, strlen(featname));

			memset(&reg_feat, 0, sizeof(BoundFeat));
			if (fopen_s(&fp, fullfeatname, "rb") == 0)
			{
				fread(&reg_feat, sizeof(BoundFeat), 1, fp);
				fclose(fp);
				
			}

			w_rFeatureStack.push_back(reg_feat);
		}
	}

	///////////////////////Read Image and Classify//////////////////////
	handle = _findfirst(FilePath, &fileinfo);
	if (-1 == handle)
		return -1;

	int class_idx;
	while (!_findnext(handle, &fileinfo))
	{
		if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
		{
			continue;
		}

		if (strstr(fileinfo.name, ".bmp"))
		{
			memset(fullimgname, 0, 256);
			memset(imgname, 0, 100);

			strcpy_s(imgname, fileinfo.name);
			//strcpy_s(imgname, "2021-06-23_10_28_17_662.jpg");
			memcpy(fullimgname, FilePath2, strlen(FilePath2));
			memcpy(fullimgname + strlen(FilePath2), imgname, strlen(imgname));

			memset(Debug_Name, 0, 256);
			memcpy(Debug_Name, DebugPath, strlen(DebugPath));
			memcpy(Debug_Name + strlen(DebugPath), imgname, strlen(imgname) - 4);
			
			LoadColorImageFile(&in_Img, fullimgname);
			
			{
				__COUNT_MILLI_SECONDS_IN_BLOCK__
				int w0, h0;
				w0 = in_Img.width;	h0 = in_Img.height;

				ByteImage gray_Img;
				gray_Img.width = w0;	gray_Img.height = h0;
				gray_Img.img = ori_Gray;

				Get_Gray(&in_Img, &gray_Img);				
				Detect_Object_Range(&gray_Img, Object_Range);

				BoundFeat cur_feat;
				int feat_flag;
				feat_flag = Get_Feature(Object_Range, Bound_Res, &cur_feat);
				if (feat_flag == -1)
					continue;

				class_idx = -1;
				class_idx = Classify_Bound(&cur_feat, w_rFeatureStack, matx_list, maty_list);

				if (class_idx == -1)
				{
					class_idx = Enroll(&cur_feat, Bound_Res, w_rFeatureStack, Enroll_Path2);
				}
				else
				{
					//class_idx = Enroll_Update(w_rFeatureStack, class_idx, matx_list, maty_list);
				}

				Get_Object_Bound(&gray_Img, Bound_Res, Bound_Info);
			}

			

#ifdef SAVE_RESULT
			char tmp_str[10];
			_itoa_s(class_idx, tmp_str, 10);
			strcpy_s(Result_Path, Detect_Path);
			strcat_s(Result_Path, tmp_str);
			strcat_s(Result_Path, "\\");
			CreateMuliteDirectory(Result_Path);
			Save_Bound_Img(&in_Img, Bound_Info, ori_w, ori_h, Result_Path, imgname);
#endif

			ColorImageFree(&in_Img);
		}
	}

	////////////////////////////////Save Updated Templates///////////////////////////////////
	save_update_Templates(w_rFeatureStack, Enroll_Path2);

	return 0;
}

int main(){
	char path_str[250];
	//strcpy_s(path_str, "E:\\Bata\\Medal_Detection\\image\\Test_Img");
	strcpy_s(path_str, "E:\\Bata\\Medal_Detection\\image\\1200-1");

	Detect_Bound(path_str);
    return 0;
}


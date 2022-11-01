//		font
// 
//

#include <stdio.h>
#include <string.h>

#define MAX_FONTDATA 6879

static struct _font_data {
	int jiscode;					// JIS CODE
	unsigned short bitmap[14];		// �r�b�g�}�b�v�f�[�^
} fontdata[ MAX_FONTDATA];



// ****************************************************************************
//	�t�H���g�t�@�C���ǂݍ���
// Out:1 ����   0:���s
// ****************************************************************************
int readFont(void)
{
	int jiscode;
	char path[] = "D:\\Users\\windy\\Documents\\test\\bdf\\shnmk14.bdf";	// TO DO �Ȃ�Ƃ�����
	char buff[20];
	int  idx=0;	



	FILE* fp = fopen(path, "r"); if (fp == NULL) { printf("file not found %s\n", path); return(0);}

	while (!feof(fp)) {
		fgets(buff, sizeof(buff), fp);
		if (strncmp(buff, "STARTCHAR", 9) == 0) {
			char name[10];
			if (sscanf(buff, "%s %X", name, &jiscode) == EOF) {  // JIS CODE �ǂݍ���
				printf("failed sscanf \n");
				return(0);
			}
			fontdata[idx].jiscode = jiscode;
		}
		else if (strncmp(buff, "BITMAP", 6) == 0) {
			int data;
			for (int j = 0; j < 14; j++) {
				fgets(buff, sizeof(buff), fp); 
				buff[4] = '\0';
				if (sscanf(buff, "%X", &data) == EOF) {
					printf("failed sscanf\n");
					return(0);
				}
				fontdata[idx].bitmap[j ] = data;
			}
		idx++;
		}
	}
	fclose(fp);
	return(1);
}

// bitmap �f�[�^�ւ̃|�C���^�[��Ԃ�
// NULL:���s
short *getFontData(int jiscode)
{
	int i=0;
	for (i = 0; i < MAX_FONTDATA; i++) {
		if (fontdata[i].jiscode == jiscode) {
			return fontdata[i].bitmap;
		}
	}
 return(NULL);
}
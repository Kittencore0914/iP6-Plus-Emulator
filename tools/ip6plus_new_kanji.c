// *******************************************
// Convert EXTKANJI.ROM to EXKANJI.ROM
//     name is ip6plus_new_kanji.c
//     by Windy
//     Date 2019/9/16
// *******************************************

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ROM_SIZE 0x20000
#define MAX_PATH 256

enum {FALSE ,TRUE};
char inPath[MAX_PATH]  = "EXTKANJI.ROM";
char outPath[MAX_PATH] = "EXKANJI.ROM";

// *******************************************
// Convert EXTKANJI.ROM to EXKANJI.ROM
// *******************************************
int convKanji(void)
{
 unsigned char *buffer1;
 unsigned char *buffer2;

 // ************* input  **************** 
 FILE *fp = fopen(inPath,"rb");
 if( fp ==NULL) { printf("*** ERROR *** �t�@�C�� '%s'�̓I�[�v���G���[�ł��B ���݂��邩�m�F���Ă��������B\n", inPath); return(0);}
 
 buffer1 = (unsigned char*)malloc( ROM_SIZE*sizeof(char) );
 if( buffer1 == NULL) {printf("memory allocation error"); fclose(fp); return(0);}
 buffer2 = (unsigned char*)malloc( ROM_SIZE*sizeof(char) );
 if( buffer2 == NULL) {printf("memory allocation error"); fclose(fp); return(0);}

 memset(buffer1, 0xff ,ROM_SIZE*sizeof(char));
 memset(buffer2, 0xff ,ROM_SIZE*sizeof(char));

 if( fread( buffer1 , ROM_SIZE, 1, fp) !=1) { printf("*** ERROR *** �t�@�C��'%s'�̓ǂݍ��݃G���[�ł��B\n",inPath); fclose(fp); return(0); }

 
 // ************* convert ***************
 for(int i=0; i< ROM_SIZE/2; i++)
	{
	 buffer2[i*2] = buffer1[i];
	 buffer2[i*2+1] = buffer1[i+ROM_SIZE/2];
	}
 fclose(fp);
 printf("�f�[�^��ǂݍ��݂܂����B\n");


 // ************* output **************** 
 fp =fopen(outPath,"r"); if(fp!=NULL) { printf("*** ERROR *** �t�@�C��'%s'�͂��łɑ��݂��܂��B�������݂𒆎~���܂����B\n",outPath); fclose(fp); return(0);}

 fp =fopen(outPath, "wb"); if(fp==NULL){printf("*** ERROR *** �t�@�C��'%s'�̃I�[�v���G���[�ł��B",outPath); return(0); }
 printf("�f�[�^���������ݒ�...");
 fwrite( buffer2, ROM_SIZE, 1,fp);
 fclose(fp);
 printf("�������܂���\n");

 return 1;
}

int main(int argc, char *argv[])
{
	int ch;
	char str[1024];

	printf("\n\n");
	printf("iP6 Plus �̊g������ROM���A�V�t�H�[�}�b�g�i���E�A���E...�j�ɕϊ����܂��B\n");
	printf("�t�@�C�� '%s'����ǂݍ���� �t�@�C�� '%s'�ɏ������݂܂��B\n\n",inPath ,outPath);
	printf("�����͂����ł��� (y/n)?");
	ch= fgetc(stdin);
 	if( ch=='y' || ch=='Y') 
		{
		convKanji();
		}
	else   {
		printf("�L�����Z�����܂����B\n");
		}

	system("pause");
}

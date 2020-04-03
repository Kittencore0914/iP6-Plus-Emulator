/*
Copyright (c) 2020 Windy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           WinKanji.c                    **/
/**                    make kanji rom for WIN32             **/
/** by Windy 2003                                           **/
/*************************************************************/
#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <assert.h>
#include "../types.h"
#include "Win32.h"
#include "../cgrom.h"

extern HWND hwndMain;

#define WIDTH  (16*2)
#define HEIGHT (16*2)

static char  *left_rom;		// left  rom
static char *right_rom;		// right rom


static unsigned char table_extkanji[]={		// Ext Kanji rom
#include "../extkanji.h"
0x00
};



// ****************************************************************************
//          make_extkanjirom: �U�̊g������ROM���������[��ɍ쐬
// ****************************************************************************
/* In: mem  EXTKANJIROM
  Out: ret  1:successfull  0: failed  */
int make_extkanjirom(byte *mem)
{
#if 1
 OSD_Surface *surface;

 // HDC   hdc;
 int   ret=0;
 char  *str;
 char  *p;
 //int   w;
 int   maxlen;
 int   i;
 int   idx;
 int   dy;
 
 char  tmpstr[3];



 surface = OSD_CreateSurface(WIDTH ,HEIGHT,1 ,SURFACE_BITMAP);	// make surface 1bpp 
 if( surface != NULL)
 	{
	 idx= 0x1210;			// start address   (jis 1 suijun)
	 str   = table_extkanji;
	 maxlen= strlen( str);

	 // �t�H���g��ݒ肷��悤�ɂ��� 2017/7/24
	 HFONT* pFont;
	 HFONT font;

	 // �t�H���g���쐬����
	 font = CreateFont(
		 15,                   // �t�H���g�̍���(�傫��)
		 0,                    // �t�H���g�̕�
		 0,                    // �p�x
		 0,                    // �p�x
		 FW_DONTCARE,          // �����̑���
		 FALSE,                // �C�^���b�N�Ȃ�TRUE
		 FALSE,                // �����Ȃ�TRUE
		 FALSE,                // ���������Ȃ�TRUE
		 SHIFTJIS_CHARSET,     // �t�H���g�̕����Z�b�g�B
		 OUT_DEFAULT_PRECIS,   // �o�͐��x�̐ݒ�B
		 CLIP_DEFAULT_PRECIS,  // �N���b�s���O���x�B
		 DRAFT_QUALITY,        // �t�H���g�̏o�͕i���B
		 DEFAULT_PITCH,        // �t�H���g�̃s�b�`�ƃt�@�~��
		 "MS UI Gothic" // �t�H���g�̃^�C�v�t�F�C�X��
	 );
	 pFont = SelectObject(surface->hdcBmp ,&font);        // �t�H���g��ݒ�B
	 SetTextColor(surface->hdcBmp ,RGB(255, 255, 0));     // �F
	 SetBkColor(surface->hdcBmp ,RGB(255, 00, 00));       // �w�i�F


	 //w   = 2;
	 for(i=0; i< maxlen; i+=2)
		{
#ifdef WIN32
		 SelectObject(surface->hdcBmp, GetStockObject( BLACK_BRUSH));
		 Rectangle(   surface->hdcBmp, 0,0 ,16  ,16);
#endif

		 tmpstr[0] = *(str+i);	 tmpstr[1] = *(str+i+1); tmpstr[2] = 0;
		OSD_textout( surface ,0,0, tmpstr ,16);

		// ------- Convert FONT image ---------
		 p = surface->pixels;
		 for(dy=0 ; dy< 16; dy++)
			{
			 mem[idx * 2]   = *(p + 0);
			 mem[idx * 2+1] = *(p + 1);

			p+= WIDTH/8;
			idx++;
			assert(idx < 0x10000);
		   }


#if 0
	 hdc = GetDC( hwndMain);
	 BitBlt( hdc ,0,0,WIDTH,HEIGHT, surface->hdcBmp,0,0,SRCCOPY);
	 Sleep(10);
	 ReleaseDC( hwndMain ,hdc);
#endif
		}

	 mem[0x2c9 * 2] =  mem[0x2c9 *2+1] = 0x41;		// bios checks character

	SelectObject(surface->hdcBmp ,pFont);                // �t�H���g�����ɖ߂��B
	OSD_ReleaseSurface( surface);
	ret=1;
	}
 else
    {
	 printf("createIBMP failed\n");
	}
 return( ret);
#endif
}
// SelectObject(hb.hdcBmp, hBitmapbak);
//DeleteDC(hb.hdcBmp);



#if 0
// ****************************************************************************
//          save_extkanjirom: �U�̊g������ROM�̕ۑ��@�@�i���͖��g�p�j
// ****************************************************************************
int save_extkanjirom(char *file_name)
{
 FILE *fp;

 printf("\nwriting extend kanji rom....");
 fp=fopen( file_name,"wb"); 
 if( fp==NULL) {printf("\n %s open failed \n",file_name);return(0);}
 
 do
    {
	 if(fwrite( left_rom,sizeof( left_rom),1,fp)!=1)
	 	break;
	 if(fwrite(right_rom,sizeof(right_rom),1,fp)!=1)
		break;
	 fclose(fp);
	 printf("done.\n");
	 return(1);
	}
 while(0);
 fclose(fp);
 printf("FAILED.\n");
 return(0);
}
#endif




#endif

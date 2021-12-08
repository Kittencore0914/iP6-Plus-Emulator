/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                       pngRW.c                           **/
/** by Windy 2004                                           **/
/*************************************************************/
/*
   �R���p�C���I�v�V�����ŁA-DHAVE_LIBPNG_=0 �Ƃ���ƁALIBPNG���g�p���Ȃ��悤�ɂł��܂�
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#if HAVE_LIBPNG_
#include <png.h>		/* libpng 's header */
#include <pngstruct.h>
#include <zlib.h>

#include <stdio.h>
#include <setjmp.h>
#include <memory.h>
#include <stdlib.h>

#include "types.h"
#include "Refresh.h"	/* getpixel() */
#include "Video.h"
#include "pngrw.h"


/*
�ӎ�:

���̃v���O�����́A���L�̃T�C�g���Q�l�ɂ����Ă��������܂����B
���肪�Ƃ��������܂����B                            Windy

�@PNG ���p�p  ����
  http://gmoon.jp/png/

�@�\

�@�C���[�W����APNG�t�@�C���Ƃ��ďo�͂��܂��B
�@8bpp�J���[�A24bpp�J���[�ɑΉ����Ă��܂��B

�o�O: 

  16bpp�J���[�A32bpp�J���[�ɂ͑Ή����Ă��܂���B

*/


// ****************************************************************************
//          �����ϐ��A�֐��Ȃ�
// ****************************************************************************
static png_color  png_color_table[256];	/* PNG �̃p���b�g */
static int        max_color;			/* �p���b�g�̐� */
static void set_png_text( png_structp png_ptr ,png_infop info_ptr);
static void write_row_callback(png_structp png_ptr, png_uint_32 row, int pass);

#define CONVERT_ARGB_RGBA( dat)  (dat  >>24) | (dat  <<8 ) 


// ****************************************************************************
//          write_png: �C���[�W�f�[�^���APNG �t�@�C���ɏ�������
//     Out: 1: success  0: failed
// ****************************************************************************
//int write_png(char *file_name, unsigned char *image, int Width, int Height,int depth)
int write_png( char * file_name , OSD_Surface *surface_in)
{
	int         x,y,i;
	png_bytepp  tmp;
	FILE		*fp;
	png_structp	png_ptr;
	png_infop	info_ptr;
	
	byte    *Xtab = OSD_GetXtab();
	//void write_row_callback(png_structp png_ptr, png_uint_32 row, int pass);

    unsigned char *image = surface_in->pixels;
    int Width  = surface_in->w;
    int Height = surface_in->h;
    if( Height ==204) Height=200;
    int pitch  = surface_in->pitch;
    int s_depth= surface_in->format->BitsPerPixel;  // source depth
    int d_depth= s_depth !=32 ? s_depth : 24;       // dest depth




	printf("Openning %s...",file_name);
	if ((fp = fopen(file_name, "wb")) == NULL) {printf("FAILED\n");return 0;}

    printf("OK\n png_create_write_struct...");

    // *** png_struct �𐶐� ****
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
        printf("FAILED\n");
		fclose(fp);
		return 0;
	}
    printf("OK\n png_create_info_struct...");

    // *** png_info �𐶐� ****
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
        printf("FAILED\n");
		png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
		fclose(fp);
		return 0;
	}

    printf("OK\n setjmp...");
    
    // *** setjmp ��ݒu (�ȍ~�Apng �̃��C�u�����Ŏ��s�����炱���ɂ���) ****

//	if (setjmp(png_ptr->jmpbuf)) {
	if( setjmp( png_jmpbuf(png_ptr))) {
    	printf("FAILED\n");
		png_destroy_write_struct(&png_ptr,  &info_ptr);
		fclose(fp);
		return 0;
	}

    printf("OK\n png_init_io...");
    // *** fp ��m�点�� ***
	png_init_io(png_ptr, fp);

    printf("OK\n png_set_write_status_fn...");
    // *** callback �֐���m�点�� ***
	png_set_write_status_fn(png_ptr, write_row_callback);

    printf("OK\n png_set_filter...");
	// *** filter �̐ݒ� ***
	png_set_filter(png_ptr, 0, PNG_ALL_FILTERS);

    printf("OK\n png_set_compression_level...");
	// *** ���k���x�� �̐ݒ� ***
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);


#if 0
    printf("png_ptr  =%p\n",png_ptr);
    printf("info_ptr =%p\n",info_ptr);
    printf("Width    =%d\n",Width);
    printf("Height   =%d\n",Height);
    printf("depth    =%d\n",depth);
#endif

	{
    int color_type;
    printf("OK\n png_set_IHDR...");
	// *** IHDR �`�����N���i�T�C�Y�A�J���[�A���k�j�Ȃǂ̐ݒ� ***
	switch( s_depth)
    	{
        case 8: 
        	color_type =  PNG_COLOR_TYPE_PALETTE;
            break;
        case 32:
        case 24:
 			color_type =  PNG_COLOR_TYPE_RGB;
            break;
        default:
        	printf("write_png(): Unknown depth = %d",s_depth);
        	return(0);
	    }
    png_set_IHDR(png_ptr, info_ptr, Width, Height, 8 /* depth */ ,color_type,
    PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	}
    
    printf("OK\n png_set_gAMA...");
	// *** �K���} �̐ݒ� ***
	png_set_gAMA(png_ptr, info_ptr, 1/ 2.2 /*1.0 */); /* �K���}�l�ɂ���Č��������ς��*/

	// *** �e�L�X�g�f�[�^�̐ݒ� ***
	set_png_text(png_ptr, info_ptr);
    
   
    printf("OK\n convert image data...");
	// �C���[�W�f�[�^���Q�����z��ɔz�u����
	tmp = ( png_bytepp )malloc( Height * sizeof( png_bytep ) );
    if( tmp==NULL)
    	{
         printf("FAILED\n");
         return(0);
        }

	for( y=0; y< Height; y++ )
    	{
        int bytes;
        int w;
        ColTyp c;
        byte  *P;

		bytes = d_depth/8;
        w     = Width * bytes * sizeof( png_byte);

		tmp[y] = ( png_bytep )malloc( w );
        if( tmp[y] ==NULL) { printf("FAILED\n"); return(0); }

		// �C���[�W��g�ݗ��Ă�BRGB �̂Ȃ�тɂ��邱��
        for(x=0; x< Width ; x++)
	     	{
		     P= tmp[y]+x *bytes;
             //c = getpixel( image, x, y , depth);
             c = getpixel( surface_in, x, y );
             
             if( s_depth ==32)
                {
                if( surface_in -> format->byteOrder == BPP32_ARGB)   // ARGB ?
                    {
                    *(P+0) = (c.ct_xid >> 16) & 0xff;   // R
                    *(P+1) = (c.ct_xid >>  8) & 0xff;   // G
                    *(P+2) = (c.ct_xid      ) & 0xff;   // B
                    }
                else    // RGBA ?
                    {
                    *(P+0) = (c.ct_xid >> 24) & 0xff;   // R
                    *(P+1) = (c.ct_xid >> 16) & 0xff;   // G
                    *(P+2) = (c.ct_xid >>  8) & 0xff;   // B
                    }
	            }
	         else
	            {     
	     	     for(i=0; i< bytes; i++)
	         	     {
                      *(P+i) = c.ct_byte[ i ];
                     }
	            }
	       	}
        }

    // *** PNG�t�@�C���̃p���b�g�̏������� ***
	if( s_depth ==8)
    	{
         printf("OK\n png_set_PLTE...");
		 png_set_PLTE( png_ptr, info_ptr, png_color_table, max_color);
		}
    
    printf("OK\n png_write_info...");
    // *** PNG�t�@�C��  �w�b�_�̏������� ***
	png_write_info(png_ptr, info_ptr);

    printf("OK\n png_write_image...");
    // *** image �̏������� ***
	png_write_image(png_ptr, tmp);

    printf("OK\n png_write_end...");
    // *** PNG�t�@�C��  �c��̏��̏������� ***
	png_write_end(png_ptr, info_ptr);

    printf("OK\n png_destroy_write_struct...");
    // *** �������[�̉�� ***
	png_destroy_write_struct(&png_ptr, &info_ptr);

    for(y=0; y< Height ; y++)
    	{
         if( tmp[y]) free( tmp[y]);
        }
    free( tmp);

    printf("OK\n file closing...");
    // *** �t�@�C���̃N���[�Y ***
	fclose(fp);
    printf("Done\n");
	return 1;	// ����!
}


// ****************************************************************************
//          set_png_palette: PNG �p�p���b�g�̐ݒ�
//     �@�@ �K�v�Ȑ������J��Ԃ��B
// ****************************************************************************
void set_png_palette(int i,unsigned char r, unsigned char g, unsigned char b)
{
 png_color_table[i].red   = r;
 png_color_table[i].green = g;
 png_color_table[i].blue  = b;
}

// ****************************************************************************
//          set_png_maxpalette: PNG �p�p���b�g�̐��̐ݒ�
// ****************************************************************************
void set_png_maxpalette(int max)
{
 max_color                = max;
}




// ****************************************************************************
//          set_png_text: PNG �̃e�L�X�g�f�[�^�̐ݒ�@�@(�����֐�)
// ****************************************************************************
static void set_png_text( png_structp png_ptr ,png_infop info_ptr)
    {
		time_t		gmt;		// G.M.T.
		png_time	mod_time;
		png_text	text_ptr[5];

		time(&gmt);
	    printf("OK\n png_convert_from_time_t...");
		png_convert_from_time_t(&mod_time, gmt);

	    printf("OK\n png_set_tIME...");
		png_set_tIME(png_ptr, info_ptr, &mod_time);
		
		text_ptr[0].key = "Title";
		text_ptr[0].text = "iP6 Plus  SNAPSHOT";
		text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[1].key = "Author";
		text_ptr[1].text = "";
		text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[2].key = "Description";
		text_ptr[2].text = "SNAPSHOT";
		text_ptr[2].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[3].key = "Creation Time";
		text_ptr[3].text = png_convert_to_rfc1123(png_ptr, &mod_time);
		text_ptr[3].compression = PNG_TEXT_COMPRESSION_NONE;
		text_ptr[4].key = "Software";
		text_ptr[4].text = "";
		text_ptr[4].compression = PNG_TEXT_COMPRESSION_NONE;

	    printf("OK\n png_set_text...");
		png_set_text(png_ptr, info_ptr, text_ptr, 5);
	}

// ****************************************************************************
//          write_row_callback: �������݂̓r���o�߂̕\���Ȃǁ@(�����֐�)
// ****************************************************************************
static void write_row_callback(png_structp png_ptr, png_uint_32 row, int pass)
{
//	printf("\r%3d%% saved\n", (row * 100) / png_ptr->height);
}


#else 		// libpng , zlib �������ꍇ

//int write_png(char *file_name, unsigned char *image, int Width, int Height,int depth)
#include "Refresh.h"
#include <stdio.h>

int write_png( char * file_name , OSD_Surface *surface_in)
    {printf("not installed LIBPNG. \n");return(0);}

void set_png_palette(int i,unsigned char r, unsigned char g, unsigned char b){}
void set_png_maxpalette(int max){}


#endif



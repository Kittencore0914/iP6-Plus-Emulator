/*
Copyright (c) 2020 windy

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
/**                           message.c                     **/
/** by Windy 2003                                           **/
/*************************************************************/
/*
What? 

    �P�ԏ�̃��b�Z�[�W��n���ƁA���{�ꃍ�P�[���̂Ƃ��́A�R�ڂ̃��b�Z�[�W��ԋp���܂��B
	���{��ȊO�̃��P�[���̂Ƃ��́A�Q�ڂ̃��b�Z�[�W��ԋp���܂��B
    �P�Ԗڂ̃��b�Z�[�W�ƁA�ǂ����v���Ȃ���΁A���������̂܂ܕԋp���܂��B

	�悤����ɁA���{�ꃍ�P�[���ȊO���ƁA���E�̋��ʌ�̉p��ɕϊ����܂��B
*/

#define MSG_EN 1
#define MSG_JP 2


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"
#include "os.h"



static unsigned char *translate[][3]={
{
	"MSG_ENABLE_CONFIG_REBOOT",
	"To enable the new change after reboot this app.",
	"(*)�}�[�N�̐ݒ�́AiP6 Plus���ċN����ɗL���ɂȂ�܂��B",
},
{
	"MSG_1D_FDD",
	"[WARNING] The PC-6601SR/ PC-6001mkIISR cannot use 1D format disk, \n You must convert 1DD format disk.",
	"�x���I�@SR �ŁA1D�^�C�v�̃f�B�X�N���g���Ƃ��́A�t�@�C���ϊ����Ȃ��Ǝg�p�ł��܂���B",
},
{
	"MSG_NO_1DD",
	"[ERROR]    PC-6001/6001mkII/6601 cannot use 1DD format disk. This disk ejected.",
	"���̋@��� 1DD�^�C�v�̃f�B�X�N�͎g���Ȃ��̂ŁA�C�W�F�N�g���܂��B",
},
{
	"MSG_RESET_CPU",
	"RESET CPU Ok?",
	"���Z�b�g���Ă������ł����H",
},
{
	"MSG_EXIT",
	"Exit Ok?",
	"�I�����Ă������ł����H",
},
{
	"MSG_FAILED_RESET",
	"RESET failed!  check ROM files.",
	"���Z�b�g�Ɏ��s���܂����B�@ROM�t�@�C�����`�F�b�N���Ă��������B",
},
{
	"MSG_REBOOT_ENABLE_CONFIG",
	"Reboot to enable new setting?",
	"�ݒ��L���ɂ��邽�߂ɁA�������ċN�����܂����H",
},
{
	"MSG_ROM_FOUND_BOOT",
	"ROM file found.  Launching... ",
	"ROM �t�@�C�����݂���܂����B�N�����ł��B",
},
{
	"MSG_REWIND_LOAD_TAPE",
	"May the LOAD TAPE rewind?",
	"LOAD�p �e�[�v�������߂��Ă������ł����H",
},
{
	"MSG_REWIND_SAVE_TAPE",
	"May the SAVE TAPE rewind?",
	"SAVE�p �e�[�v�������߂��Ă������ł����H",
},
{
	"MSG_SEARCH_ROM",
	"ROM file not found.  Do you try search ROM file ? ",
	"ROM �t�@�C����������܂���B�@ROM�t�@�C�����������Ă݂܂���? ",
},
{
	"MSG_SAME_DISK_IMAGE",
	"Do not put the same disk to drive 1 and drive 2",
	"�h���C�u1�ƁA�h���C�u2 �ɓ����f�B�X�N�����Ȃ��ŉ�����",
},
{
	"MSG_ROM_NOT_FOUND",
	"ROM file is not found.",
	"ROM �t�@�C����������܂���B",
},
{
	"MSG_NOT_FOUND_COMPATIBLE_ROM",
	"ROM file is not found. Launch with compatible rom ?",
	"ROM�t�@�C����������܂���B�݊�ROM�ŋN�����܂����H",
},
{
	"MSG_COMPATIBLE_ROM",
	"Launch with compatible rom ?",
	"�݊�ROM�ŋN�����܂����H",
},
{
	"MSG_SET_ROM_PATH",
	"Set ROM file path in Control-> Configure menu or...",
	"Control->Configure ���j���[��ROM �t�@�C���̃p�X��ݒ肷�邩�A",
},
{
	"MSG_COPY_ROM_FILE",
	"copy ROM file to the following path",
	"���L�̃p�X��ROM �t�@�C�����R�s�[���Ă�������"
},
{NULL,NULL,NULL}
};


//char *mgettext(char *key)
unsigned char *mgettext(unsigned char *key)
{
    int  i;
    int  destIdx;
    unsigned char *p;
	int  lang;

	lang = OSD_getlocale();
	destIdx = MSG_EN;
	if (lang == OSDL_JP)
		{
		destIdx = MSG_JP;
		}

    p= key;
    for(i=0; translate[i][0]!=NULL ; i++)
        {
         if( strncmp( key, translate[i][0],strlen(translate[i][0])) ==0)
            {
             p=  translate[i][destIdx];
             break;
            }
        }
    return( p);
}




// ******************************************************
// 			  buffer.h  
//            by Windy
// *******************************************************
#ifndef _BUFFER_H
#define _BUFFER_H

#include "types.h"

//#ifdef WIN32
//#include <windows.h>
//#else
//#define DWORD int
//#endif


// ****************************************************************************
//          ringbuffer: �����p�@�����O�o�b�t�@
// ****************************************************************************
//  short  buffer[SOUND_BUFSIZE*101];
typedef struct _ringbuff
{
  int   read_idx;		// �ǂݍ��݃C���f�b�N�X
  int   write_idx;		// �������݃C���f�b�N�X
  int   length;			// �O���猩���o�b�t�@�̒���
  int   datas;			// �i�[����Ă���f�[�^��
  int   in_length;		// �����I�ȃo�b�t�@�̒���
  short *buffer;		// �o�b�t�@
} RINGBUFFER;


RINGBUFFER *ringbuffer_Open(int size);
int  ringbuffer_Read( RINGBUFFER *buf , short *buffer,int size);
int  ringbuffer_Write(RINGBUFFER *buf , short *buffer,int size);
void ringbuffer_Close(RINGBUFFER *buf);

int ringbuffer_Get( RINGBUFFER *buff ,short *dat);
int ringbuffer_Put(RINGBUFFER *buff ,short dat);
int ringbuffer_DataNum( RINGBUFFER *buff );
int ringbuffer_FreeNum( RINGBUFFER *buff );


// ****************************************************************************
//          ringbuffer: �L�[���͗p�@�����O�o�b�t�@
// ****************************************************************************

typedef struct keybuffer_tag {
  int   read_idx;
  int   write_idx;
  int   length;

  char  chr   [1024];

  int   keydown[1024];		// 1: keydown  0: keyup
  int   scancode[1024];		// scan code
  int   osdkeycode[1024];	// osdkeycode
} KEYBUFFER;

KEYBUFFER*  init_keybuffer(void);
void clear_keybuffer(KEYBUFFER* _keybuffer);
void write_keybuffer( KEYBUFFER* keybuffer,char chr ,int keydown , int scancode  ,int osdkeycode );
int  read_keybuffer ( KEYBUFFER* keybuffer,char *chr ,int *keydown ,int *scancode ,int *osdkeycode );
int sense_keybuffer( KEYBUFFER *keybuffer ,char *chr ,int *keydown ,int *scancode ,int *osdkeycode);

void close_keybuffer( KEYBUFFER *_keybuffer);


#endif

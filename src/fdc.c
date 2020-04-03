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
/**                           fdc.c                         **/
/**                                                         **/
/**               Internal Floppy disk Controller           **/
/** by Windy                                                **/
/*************************************************************/
/*
FDC (Floppy Disk Controller) -  uPD765A

�G�~�����[�^�����ɕt����

   �e�R�}���h�͋K��̒����܂ŏ������܂ꂽ���_�ŁA���s���܂��B
   CPU����̃f�[�^�̎�n���́A1024�o�C�g�� FD BUFFER����čs���܂��B

���ƁA���@�ł́AFDC�ƁAFDD BUFFER�Ԃ̃f�[�^�]���� DMA�ōs���Ă���悤�ł��B
�������A�G�~�����[�^�ł́A�蔲�������Ă��܂��B
�f�B�X�N����ǂݎ�����f�[�^�́A������ FDD BUFFER�ɏ�������ł��܂��B
������ADMA���]������\���t���O���A�����邱�Ƃ͗L��܂���E�E�B(��)

---------------------------------------------------------------------------
�֘A I/O PORT �ꗗ

B1H:

    b3 b2     B1   B0
    0  TYPE  DIR   DMA
	    |     |     ---- DMA SW 0: ON              1:OFF
	    |     ----DMA DIRECTION 0: CPU->FD BUFFER  1:FD BUFFER ->CPU
	    --------- FDD TYPE      0: ����            1:�O�t��

B2H: bit0=0�� DMA�]�����ł��B
   (���������A���̃G�~�����[�^�R�[�h�ł́A��ɂP�ɂ��Ă����Ȃ��Ɠ����Ȃ��B�j


B3H: FDC-IF����ŁAB2H��ǂޑO��bit0=0�ŏo�͂���ƃX�e�C�^�X�̎������\�ɂȂ�


D0H: FDD BUFFER ��0000H - 00FFH
D1H: FDD BUFFER ��0100H - 01FFH
D2H: FDD BUFFER ��0200H - 02FFH
D3H: FDD BUFFER ��0300H - 03FFH

CPU���A4�ǂݎ��ꍇ�́AD3H,D2h,D1h,D0h �̏��Ԃɓǂݎ��B
�ȉ��A
3�ǂݎ��ꍇ�́A    D2h,D1h,D0h�Ɠǂݎ��B
2�ǂݎ��ꍇ�́A        D1h,D0h�Ɠǂݎ��B
1�ǂݎ��ꍇ�́A            D0h�Ɠǂݎ��B

�������AFDD Buffer�ւ̃A�N�Z�X�́ASTACK�̂悤�ɁA�����悾���ōs����B
FDC����AFDD Buffer�֓]������Ƃ��́ASTACK��push����悤�ɁA�i�[���Ă����A
FDD Buffer����@CPU�֓]������Ƃ��́ASTACK��pop ����悤�ɁA�o���Ă����B


D4H: FDD��Ready�M��?	bit1���h���C�u2 bit0���h���C�u1	 0�̂Ƃ�ready�݂���

D6H: FDD��MortorOn�M��?	bit1���h���C�u2 bit0���h���C�u1	 0�̂Ƃ�motor on�݂���

D8H: �������ݕ⏞��H��on/off ? 	bit0��0�̂Ƃ�on�ɂȂ�݂���

DAH: FD BUFFER �̃f�[�^�]���ʎw��H  100h�P�ʂŎw�肷��B

DEH: ��


----------------------------------------------------------------------------
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <memory.h>
#include "types.h"
#include "P6.h"
#include "mem.h"
#include "d88.h"
#include "error.h"
#include "Timer.h"
#include "schedule.h"

#include "os.h"
#include "dokodemo.h"


//************ FDC Status *******************
#define FDC_BUSY              0x10
#define FDC_READY             0x00
#define FDC_NON_DMA           0x20
#define FDC_FD2PC             0x40
#define FDC_PC2FD             0x00
#define FDC_DATA_READY        0x80

//************* Result Status 0 *************
#define ST0_NOT_READY         0x08
#define ST0_SEEK_END          0x20
#define ST0_FAILED_CMD        0x40
#define ST0_INVALID_CMD       0x80

//************* Result Status 1 *************
#define ST1_NOT_WRITABLE      0x2

//************* Result Status 2 *************

//************* Result Status 3 *************
#define ST3_TRACK0             0x10
#define ST3_READY              0x20
#define ST3_WRITE_PROTECT      0x40
#define ST3_FAULT              0x80

static int cur_drive=0;		// current drive no.

struct _fdc {
    int cylinder;	// current cylinder (seek command)
    int length;		// transfer length
} fdc[2];


char fdc_buff[4][256+1];     // FD data Buffer
int    idx[4];               // FD data Buffer Index

char fdc_buff_in[10+1];		// FD cmd Buffer
int    idx_in;
char fdc_buff_out[10+1];	// FD cmd Buffer
int    idx_out;

static int   cmd_length[]={0,0,0,3,2,9,9,2,1,0,0,0,0,6,0,3};


static int  busy_cnt=0;


#define DEFAULT_BUSY_CNT 100

void dokodemo_save_fdc(void)
{
	dokodemo_putsection("[FDC]");
	DOKODEMO_PUTENTRY_INT( fdc[0].cylinder);
	DOKODEMO_PUTENTRY_INT( fdc[0].length);
	DOKODEMO_PUTENTRY_INT( fdc[1].cylinder);
	DOKODEMO_PUTENTRY_INT( fdc[1].length);
	dokodemo_putentry_buffer("fdc_buff[0]" , fdc_buff[0] , 256);
	dokodemo_putentry_buffer("fdc_buff[1]" , fdc_buff[1] , 256);
	dokodemo_putentry_buffer("fdc_buff[2]" , fdc_buff[2] , 256);
	dokodemo_putentry_buffer("fdc_buff[3]" , fdc_buff[3] , 256);
	DOKODEMO_PUTENTRY_INT( idx[0] );
	DOKODEMO_PUTENTRY_INT( idx[1] );
	DOKODEMO_PUTENTRY_INT( idx[2] );
	DOKODEMO_PUTENTRY_INT( idx[3] );
	
	
	dokodemo_putentry_buffer("fdc_buff_in" , fdc_buff_in , 10);
	DOKODEMO_PUTENTRY_INT( idx_in);
	dokodemo_putentry_buffer("fdc_buff_out", fdc_buff_out, 10);
	DOKODEMO_PUTENTRY_INT( idx_out );
	
}


void dokodemo_load_fdc(void)
{
	DOKODEMO_GETENTRY_INT( fdc[0].cylinder);
	DOKODEMO_GETENTRY_INT( fdc[0].length);
	DOKODEMO_GETENTRY_INT( fdc[1].cylinder);
	DOKODEMO_GETENTRY_INT( fdc[1].length);
	dokodemo_getentry_buffer("fdc_buff[0]" , fdc_buff[0] , 256);
	dokodemo_getentry_buffer("fdc_buff[1]" , fdc_buff[1] , 256);
	dokodemo_getentry_buffer("fdc_buff[2]" , fdc_buff[2] , 256);
	dokodemo_getentry_buffer("fdc_buff[3]" , fdc_buff[3] , 256);
	DOKODEMO_GETENTRY_INT( idx[0] );
	DOKODEMO_GETENTRY_INT( idx[1] );
	DOKODEMO_GETENTRY_INT( idx[2] );
	DOKODEMO_GETENTRY_INT( idx[3] );
	
	
	dokodemo_getentry_buffer("fdc_buff_in" , fdc_buff_in , 10);
	DOKODEMO_GETENTRY_INT( idx_in);
	dokodemo_getentry_buffer("fdc_buff_out", fdc_buff_out, 10);
	DOKODEMO_GETENTRY_INT( idx_out );
	
}




//************* internal function *************
void fdc_command(void);
void fdc_read_data(char *fdc_buff_in);
void fdc_write_data(char *fdc_buff_in);


extern void PutDiskAccessLamp(int sw);


// ************************************
//      Initialize
// ************************************
void fdc_init(void)
{
 int i;
 portDC= FDC_DATA_READY | FDC_READY | FDC_PC2FD;
 for(i=0; i<4;i++)
      {
       idx[ i]=0;
       memset( fdc_buff[i], 0 ,256);
      }
 memset( fdc_buff_in, 0,10);
 idx_in=0;
}


// ************************************
//      push data to FD Buffer      OUT (0xD0...0xD3)
// ************************************
void fdc_push_buffer( int port ,byte Value)
{
 port=port & 0xf;
 if(idx[ port]>255) {PRINTDEBUG2(DSK_LOG ,"[fdc.c][fdc_push_buffer] out of range: push_buffer port=%X PC=%X\n",port,getPC()); return;}
 fdc_buff[ port ] [ idx[ port]]= Value;
 idx[ port]++;
}

// ************************************
//      pop data from FD Buffer      IN (0xD0...0xD3)
// ************************************
byte fdc_pop_buffer( int port)
{
 byte Value;
 Value= NORAM;
 port=port & 0xf;
  if(idx[ port]<1) {PRINTDEBUG2(DSK_LOG ,"[fdc.c][fdc_pop_buffer] out of range: pop_buffer  port=%X PC=%X\n",port,getPC()); return(Value);}
  //if(idx[ port]==1) {printf("pop_buffer  recieve done port=%X PC=%X\n",port,getPC());}
 idx[ port]--;

  Value = fdc_buff[ port] [ idx[ port]];
  return ( Value);
}


// ************************************
//      push status to FD Buffer Out
// ************************************
void fdc_push_status( int Value)
{
 fdc_buff_out[ idx_out++]= Value;
}

// ************************************
//      pop status from FD Buffer Out
// ************************************
byte fdc_pop_status(void)
{
 return(fdc_buff_out[ --idx_out]);
}

// ************************************
//	FDC transfer sectors  fdd buffer�̓]����
// ************************************
void fdc_outDA(byte Value)
{
 fdc[cur_drive].length = ~(Value-0x10);
 //printf("fdc.length= %x  \n",fdc.length );
}

// ************************************
//	FDC status
// ************************************
byte fdc_inpDC(void)
{
 byte ret;

 ret = portDC;
// if( busy_cnt >0)		// busy_cnt ���P�ȏゾ�ƁAFDC�͎d�����Ƃ���B
//	{
//	 ret |= FDC_BUSY;
//	 busy_cnt--;
//	}
 return ret;
}

// ************************************
//	FDC write
// ************************************
void fdc_outDD(byte Value)
{
 fdc_buff_in[ idx_in++]= Value;
 //printf( "cmd_len=%d %d\n",cmd_length[ fdc_buff_in[ 0] & 0xf],idx_in);
 if( cmd_length[ fdc_buff_in[ 0] & 0xf] ==idx_in)	// �R�}���h���̃R�}���h�������� �R�}���h���s
     {
      fdc_command();
     }
}

// ************************************
//	FDC  read
// ************************************
byte fdc_inpDD(void)
{
// printf("r=%X\n", fdc_buff_out[ idx_out]);
//  printf("idx_out=%d ", idx_out);
// ********************* PC�����A�S�Ď�M������APC2FD �ɂ���B*********
  if( idx_out==1)  { portDC= FDC_DATA_READY | FDC_PC2FD; PRINTDEBUG(DSK_LOG ,"[fdc.c][fdc_inpDD] Receive done..\n");}
  if( idx_out<=0)  { printf("FDC read : out of range %d",idx_out);}

//  return( fdc_buff_out[ idx_out--]);
  return(fdc_pop_status());
}



// ********  FDC COMMAND defs *********
enum {
C_DUMMY0               , C_DUMMY1 , C_DUMMY2 , C_SPECIFY , C_DUMMY4 , C_WRITEDATA , C_READDATA , C_RECALIBRATE,
C_SENSEINTERRUPTSTATUS , C_DUMMY9 , C_DUMMYA , C_DUMMYB  , C_DUMMYC , C_WRITEID   , C_DUMMYE   , C_SEEK,
};

// ************************************
//	FDC  Command Execute
// ************************************
void fdc_command(void)
{
	int cmdno;
	static int pre_cmdno=-1;
	static int recalibrate_flag = 0;
 
	cmdno= fdc_buff_in[0] & 0xf;
	PRINTDEBUG(DSK_LOG ,"[fdc.c][fdc_command] ");
	switch( cmdno )
		{
		case C_SPECIFY /*0x03*/: 
                  PRINTDEBUG(DSK_LOG ,"Specify  ");
                  break;
       case C_WRITEDATA /*0x05*/: 
                  PRINTDEBUG(DSK_LOG ,"Write Data ");
                  fdc_write_data(fdc_buff_in);
                  break;
       case C_READDATA /*0x06*/: 
                  PRINTDEBUG(DSK_LOG ,"Read Data  ");
                  fdc_read_data( fdc_buff_in);
                  break;
       case C_SENSEINTERRUPTSTATUS /*0x08*/: 
				  PRINTDEBUG(DSK_LOG ,"Sense Interrupt Status ");
						if( pre_cmdno != cmdno)
							{
							if( (recalibrate_flag ==0 && DskStream[ cur_drive])|| (recalibrate_flag==1 && cur_drive+1 <=disk_num ))     // disk ok?
								fdc_push_status( fdc[cur_drive].cylinder); // disk ok
							else
								fdc_push_status( 255);	// disk non
                      
							//if( pre_cmdno != cmdno)
								{
								int drive=0;
								//if( DskStream[ cur_drive] )	// mount disk ? 
									drive = cur_drive;
								fdc_push_status( ST0_SEEK_END | drive);	// normal
								}
							}
						else
							{
							fdc_push_status( ST0_SEEK_END |ST0_INVALID_CMD );
							}
			 	  recalibrate_flag =0;
				  portDC=FDC_DATA_READY | FDC_FD2PC;  // for RESULT
                  break;

       case C_WRITEID /*0x0D*/: 
                  PRINTDEBUG(DSK_LOG ,"Write ID    ");
                  break;	 // Format is Not Implimented

       case C_RECALIBRATE /*0x07*/:
                  PRINTDEBUG(DSK_LOG ,"Recalibrate "); 
                  fdc_buff_in[2]=0x00;		// Seek to TRACK0

				  PutDiskAccessLamp(1);		// �N�����̃A�N�Z�X
				  OSD_Delay(200);
				  PutDiskAccessLamp(0);
				  recalibrate_flag = 1;
       case C_SEEK /*0x0F*/: 
				  cur_drive= fdc_buff_in[1];
                  PRINTDEBUG1(DSK_LOG ,"Seek track=%d",fdc_buff_in[2]);
                  fdc[cur_drive].cylinder= fdc_buff_in[2];
				  break;
       default:   printf("Not Impliment cmd=%X",fdc_buff_in[0]);
                  break;
		}
	pre_cmdno = cmdno;
	PRINTDEBUG(DSK_LOG ,"\n");
 
//  dump3( fdc_buff_in ,cmd_length[ fdc_buff_in[0]&0xf] );
	idx_in=0;

}

diskLampOff_func()
{
	PutDiskAccessLamp(0);
}

// ************************************
//	read data
// ************************************
void fdc_read_data(char *fdc_buff_in)
{
	byte *p;
	int drive,c,r;
	int i;

	drive= fdc_buff_in[1];
	if( DskStream[drive])
		{
		PutDiskAccessLamp(1);					// �A�N�Z�X�����v������
		if( !Event_isActive( EVENT_DISK))		// �w�莞�Ԍ�ɁA�A�N�Z�X�����v������
			if( !Event_Add( EVENT_DISK, 120, EV_ONETIME |EV_MS , diskLampOff_func) ) 
				return;


		//OSD_Delay( 16); 		// �E�G�C�g����   2003/10/5
     

		cur_drive = drive;
		c= fdc[cur_drive].cylinder;
		c= (!is1DD( cur_drive) && isSYS( cur_drive) && P6Version== PC66SR ) ? c / 2: c;   // if 1D and sys-disk and PC-6601SR then c= c/2
		r= fdc_buff_in[4]-1;

		p= read_d88sector( fdc[ cur_drive].length, drive,c,r);

//		for(i= fdc.length-1; i>=0; i--)
		for(i= 0; i< fdc[cur_drive].length; i++)
			{
#if 0					// hudson  test code ------------------------
			if( c>=3)
				{
				for(j=255; j>=0; j--)
					{
					fdc_buff[i][j]=*p;
					p++;
					}
				idx[i]=256;
				}
			else	
#endif
				{
				memcpy( fdc_buff[ i], p,256);
				idx[i]=256;
				p+=256;
				}

//	     for(j=0; j<256; j++) { fdc_push_buffer(i,*p); p++;}
//	      memcpy( fdc_buff[ i], read_d88sector(1 ,d ,c ,r+i), 256);
//	      idx[i]=256;
			}
		}
	else
		{	// not ready �̏ꍇ�A�K���ȃf�[�^��ǂݍ��񂾂��Ƃɂ���  files �� ??AT Error�ɂȂ�
		for(i=0; i<4; i++)
			{
			int j;
			for(j=0; j<256; j++)
				{
				fdc_buff[i][j]= j;
				}
			idx[i]= 256;
			}
		}

	// ---------- Result Status -------------
	fdc_push_status( 0);  // n
	fdc_push_status( 0);  // r
	fdc_push_status( 0);  // h
	fdc_push_status( fdc[cur_drive].cylinder );  // c
	fdc_push_status( 0);  // st2
	fdc_push_status( 0);  // st1
	fdc_push_status( 0 | cur_drive);  // st0
	

	//PutDiskAccessLamp(0);
	busy_cnt= DEFAULT_BUSY_CNT;
	portDC=FDC_DATA_READY| FDC_FD2PC;	// for RESULT
}

// ************************************
//	write data
// ************************************
void  fdc_write_data(char *fdc_buff_in)
{
	int drive,c,r;
	int i,j;
	char buff[256*4];
	char *p;

	drive = fdc_buff_in[1];

	if( DskStream[ cur_drive] && !getDiskProtect(cur_drive))	// check write protect
		{
		PutDiskAccessLamp(1);					// �A�N�Z�X�����v������
		if( !Event_isActive( EVENT_DISK))		// �w�莞�Ԍ�ɁA�A�N�Z�X�����v������
			if( !Event_Add( EVENT_DISK, 90, EV_ONETIME |EV_MS , diskLampOff_func) ) 
				return;

		cur_drive = drive;	
		p= buff;
		c= /*fdc_buff_in[2]; */ fdc[cur_drive].cylinder;
		r= fdc_buff_in[4]-1;
//      for(i= fdc.length-1; i>=0; i--) 
		for(i= 0; i< fdc[cur_drive].length; i++)
			{
			for(j=0; j<256;j++) { *p=fdc_pop_buffer(i); p++; }	// write data
			}
		write_d88sector( fdc[cur_drive].length, cur_drive ,c ,r,  (byte*)buff);
		}

 // ---------- Result Status -------------
	fdc_push_status( 0);  		// n
	fdc_push_status( 0);  		// r
	fdc_push_status( 0);  		// h
	fdc_push_status( fdc[cur_drive].cylinder );  // c
	fdc_push_status( 0);  		// st2

	if( !getDiskProtect( cur_drive))
		fdc_push_status( 0);  	// st1
	else
		fdc_push_status( ST1_NOT_WRITABLE ); // st1   not writeable 2003/1/21

	if( DskStream[0])
		fdc_push_status( 0 | cur_drive);  // st0
	else
		fdc_push_status( ST0_FAILED_CMD);  // st0	media not ready

	PutDiskAccessLamp(0);
	busy_cnt= DEFAULT_BUSY_CNT;
	portDC=FDC_DATA_READY| FDC_FD2PC;	// for RESULT
	}



// **********************************************
//     dump    for debug
// **********************************************
int dump3(char *buff,int length)
 {
  int i;
  printf("\t ");
  for(i=0; i< length; i++)
      {
       printf("%02x ",*(buff+i) & 0xff);
      }
  printf("\n");
  return 0;
 }


#if 0
     for(i= fdc.length-1; i>=0; i--) 
       {
         for(j=0; j<256;j++) { buff[ j ]=fdc_pop_buffer(i); }	// write data
         write_d88sector(1, d ,c ,r+i,  buff);
       }
#endif

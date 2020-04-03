/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**            �C���e���W�F���g�f�B�X�N���j�b�g             **/
/**                                                         **/
/**                     name is disk.c                      **/
/**                                                         **/
/** by windy                                                **/
/*************************************************************/
/*
�C���e���W�F���g�^�C�v�ł��B

�G�~�����[�^�����ɕt����

   �R�}���h�^�f�[�^��ǂݍ���ł����A�K��̒����ɒB������A�e�R�}���h�̎��s�����܂��B
   �f�[�^�̎�M�v������������A���ɓǂ�ł���f�[�^�� �n���܂��B

�e�R�}���h�̐���

  write sector �́A�R�}���h�̌�ɁA�������݂����f�[�^����ׂ܂��B
  read  sector �́A�R�}���h�𔭍s���Ă���ASEND DATA �� �f�[�^��ǂ݂܂��B
  result status�́A�R�}���h�̎��s���ʂ�Ԃ��܂��B
  result drive status �́A�h���C�u�̏�Ԃ�Ԃ��܂��B
  XMIT         �́A�������[�h���ڂ̗L���� �`�F�b�N����̂Ɏg�p���Ă���悤�ł��B
  SetSurfaceMode�́A�C���e��������ƃ^�C�v�ŁA1DD���A1D���̃`�F�b�N�����Ă���݂����ł�

�������[�h�ɂ���

  �ʏ탂�[�h�́A���̃n���h�V�F�C�N�ɁA�P�o�C�g������n���ł��Ȃ��̂ɑ΂��āA
  �������[�h�́A���̃n���h���������ŁA�Q�o�C�g����n���ł��܂��B
    �G�~�����[�^�I�ɂ́A�������[�h�ł��ʏ탂�[�h�ł��A�قƂ�Ǔ��������ł��B
 
�Q�l����
 http://www2.odn.ne.jp/~haf09260/Pc80/Pc80s31.htm
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __APPLE__
#include <machine/types.h>
#endif

#include <string.h>
#if HAVE_DIRENT_H
#include <dirent.h>
#endif

#include "types.h"
#include "P6.h"
#include "mem.h"
#include "disk.h"
#include "d88.h"
#include "error.h"
#include "Timer.h"

#include "os.h"

#include "dokodemo.h"

//extern int disk_num;

// ------------ command length -----------------------------------------
static int cmd_length[]={1 ,0 ,5 ,1 ,8 ,2 ,1 ,1 ,5 ,1 ,1 ,1 ,1,1,1,1,  1 ,0,1};

#define PC_SEND_DONE       0x8
#define PC_RECV_READY      0xA
#define PC_RECV_DONE       0xC
#define PC_SEND_REQUEST    0xE

#define FD_SEND_DONE         1
#define FD_RECV_READY        2
#define FD_RECV_DONE         4

#define SET_8255          0x91

static int cur_drive=0;   // current drive

int fd_send;			// 1: PC -> FD Senging..
int fd_recv;            // 1: PC <- FD Recieving..
int fdindex_in=0;       // index of fdbuff input
int fdindex_out=0;      // index of fdbuff output
byte fdbuff_in[256*16];	// fd intenal buffer input
byte fdbuff_out[256*16];// fd intenal buffer output
						// 256*16 : length of 1 Track

void dokodemo_save_disk(void)
{
	dokodemo_putsection("[DISK]");
	DOKODEMO_PUTENTRY_INT( fd_send);
	DOKODEMO_PUTENTRY_INT( fd_recv);
	DOKODEMO_PUTENTRY_INT( fdindex_in);
	DOKODEMO_PUTENTRY_INT( fdindex_out);
	dokodemo_putentry_buffer("fdbuff_in" , fdbuff_in  , 256*16);
	dokodemo_putentry_buffer("fdbuff_out", fdbuff_out , 256*16);
	
}

void dokodemo_load_disk(void)
{
	DOKODEMO_GETENTRY_INT( fd_send);
	DOKODEMO_GETENTRY_INT( fd_recv);
	DOKODEMO_GETENTRY_INT( fdindex_in);
	DOKODEMO_GETENTRY_INT( fdindex_out);
	dokodemo_getentry_buffer("fdbuff_in" , fdbuff_in  , 256*16);
	dokodemo_getentry_buffer("fdbuff_out", fdbuff_out , 256*16);
	
}

void dump(char *buff,int length);

void set_fd_send(int val)
{
 fd_send = val;
}

// ************************************
//      controll (PC -> FD)
// ************************************
//  ATN DAC RFD DAV X  DAC RFD DAV
//
//
void outD3( byte Value)
{
	// portD3=Value;
	if( Value ==SET_8255)	                                // 8255 mode set
		{
		 fd_send=fd_recv= fdindex_in= fdindex_out=0;

		//PRINTDEBUG(DSK_LOG,"disk: set 8255 mode \n");
		}

	if( Value ==PC_SEND_REQUEST+1)   // F     ATN�M��ON�@�R�}���h�E�f�[�^���M�v��   // PC karano SEND_REQUEST
		{
		fd_send = 1;
		portD2 |= FD_RECV_READY;     // FD RECV ready on   2
		fdindex_in=0;
		//PRINTDEBUG(DSK_LOG," F) PC_SEND_REQUEST on  fd_send=1 fdindex_in=0\n");
		}
 	if( Value ==PC_SEND_REQUEST)     // E     ATN�M��OFF �R�}���h�E�f�[�^���M�v��	// PC gawa SEND DONE
		{
		fd_send = 1;
		//PRINTDEBUG(DSK_LOG," E) PC_SEND_REQUEST off fd_send=1\n");
		}
	if( Value ==PC_SEND_DONE+1)      // 9     DAV�M��ON�@�R�}���h�E�f�[�^���M����
		{
		fd_send= 1;
		portD2 |= FD_RECV_DONE;		 // FD RECV done on   4
		//PRINTDEBUG(DSK_LOG," 9) PC_SEND_DONE on    fd_send=1\n");
		}
	if( Value ==PC_SEND_DONE)        // 8     DAV�M��OFF�@�R�}���h�E�f�[�^���M����
		{
		fd_send= 0;
		fd_send= 1;		// fix me !!  �P�łȂ��ƁA�R���� �������Ȃ��B�B���疺�΍�B�B 
		
		// ----------- command execute ---------------
		portD2 &= ~FD_RECV_DONE;	// FD RECV done off  4
		if( ((fdbuff_in[0]==1)||(fdbuff_in[0]==0x11)) && fdindex_in ==2 ) 	// Write data
			{
			cmd_length[ fdbuff_in[0]]= 5+256*fdbuff_in[1];	// set length
			}
		//PRINTDEBUG(DSK_LOG," 8) PC_SEND_DONE off  fd_send=0 \n");

		if( fdindex_in == cmd_length[ fdbuff_in[0]])
			{
			exec_command();		// execute command
			PRINTDEBUG2(DSK_LOG,"[disk][outD3] command execute cmd no.= %d  fdindex_in=%d\n",fdbuff_in[0] , fdindex_in);
			}
		}


                                                                         // PC gawa Recive READY
	if( Value ==PC_RECV_READY+1)     // B      RFD�M��ON�@�@�R�}���h�E�f�[�^��M��������
		{
		fd_recv = 1;
		portD2 |= FD_SEND_DONE;		// FD Send done on  1
		//PRINTDEBUG(DSK_LOG," B) PC_RECV_READY on fd_recv=1\n");
		}
	if( Value ==PC_RECV_READY)       // A	�@�@RFD�M��OFF�@�@�R�}���h�E�f�[�^��M��������// PC gawa Recieve Ready off
		{
		fd_recv = 1;
		//PRINTDEBUG(DSK_LOG," A) PC_RECV_READY off  fd_recv=1\n");
		}
	if( Value ==PC_RECV_DONE+1)      // D      DAC�M��ON�@�@�R�}���h�E�f�[�^��M����
		{
		fd_recv = 1;
		portD2 &= ~FD_SEND_DONE;		// FD Send done off  1
		//PRINTDEBUG(DSK_LOG," D) PC_RECV_DONE on    fd_recv=1\n");
		}
	if( Value ==PC_RECV_DONE)        // C      DAC�M��OFF�@�@�R�}���h�E�f�[�^��M����
		{
		fd_recv = 0;
		//PRINTDEBUG(DSK_LOG," C) PC_RECV_DONE off   fd_recv=0\n");
		}
 }

// ************************************
//      controll (PC <- FD)
// ************************************
byte inpD2(void)
{
	int ret;
	if(DskStream)
		ret=portD2;
	else
		ret=NORAM;
	return(ret);
}

// ************************************
//       data (PC <- FD)
// ************************************
byte inpD0(void)
{
	byte Value = 0x2e;
	if( fdindex_out > 256*16) { return NORAM;} // out of range ...max Transfer is 16 Sector
	if(fd_recv==1)
		{
		Value=fdbuff_out[ fdindex_out++];
		//   printf("r=%02X",Value);
		}
	return (Value);
}

// ************************************
//      cmd/data (PC -> FD)
// ************************************
void outD1(byte Value)
{
	if( fdindex_in > 256*16) { printf("cmd/data: out of range %d\n",fdindex_in);return;}
	if(fd_send==1 || fdbuff_in[0]==0x11)	// when write sector (fast) command ,too
		{
		fdbuff_in[ fdindex_in++]= Value;
		PRINTDEBUG(DSK_LOG,"fdbuff_in =");
	   	dump( fdbuff_in, fdindex_in); // debug
    	}
	else
		{
	    portD1=Value;	// support for PC-6001Mk2 DISK -  �������܂ꂽ�l�����̂܂ܕԂ��B
	   }
}


// ************************************
//      controll output
// ************************************
void disk_out( byte Port, byte Value)
{
 	switch( Port)
    	{
    	case 0xD1: outD1( Value); break;
    	case 0xD3: outD3( Value); break;
    	}
}

// ************************************
//      controll input
// ************************************
byte disk_inp( byte Port)
{
	byte Value;
	switch( Port)
		{
		case 0xD0: Value = inpD0( ); break;
		case 0xD2: Value = inpD2(); break;
		case 0xD1: Value = portD1; break;	// suport for pc-6001MK2 disk
		}
	return( Value);
}


// ************************************
//      exec command
// ************************************
//    00:INZ
//    01:WRITE              N,DD,TT,SS,Data *N
//    02:READ               N,DD,TT,SS
//    03:SEND DATA          (Data *N)
//    04:COPY               N,(src)DD,TT,SS  (dest),DD,TT,SS
//    05:FORMAT DD
//    06:SEND RESULT STATUS  
//    07:SEND DRIVE  STATUS
// 0b 11:XMIT               ADDR, BYTECOUNT  (09H�Ɠ����@FDD->PC�Ƀ������[�]���j
// 0c 12:RCV                ADDR, BYTECOUNT
// 0e 14:LOAD               N,DD,TT,SS, ADDR
// 0f 15:SAVE               N,DD,TT,SS, ADDR
//
// DRIVE�� 07H �ŁA�A���Ă���
// READ  02H �œǂݍ��񂾌�A03H �ŁAPC���ɓ]������
// WRITE 01H �ŁA�������ރf�[�^���ꏏ�ɓn��
enum {C_INZ ,C_WRITE ,C_READ , C_SENDDATA ,C_COPY ,C_FORMAT ,C_SENDRESULTSTATUS ,C_SENDDRIVESTATUS ,
C_DUMMY8  ,C_DUMMY9 ,C_DUMMYA , C_XMIT    , C_RCV , DUMMYD ,  C_LOAD            , C_SAVE , 
C_DUMMY10 , C_FASTWRITE , C_FASTREAD};

void exec_command(void)
 {
	int nn,dd,tt,ss;
	static int Result;	// ���O�̃R�}���h�̐���
	// byte *p=NULL;

	PutDiskAccessLamp(1);

	PRINTDEBUG(DSK_LOG, "[DISK]:");
	switch ( fdbuff_in[0])
		{
		case C_INZ: 
					PRINTDEBUG(DSK_LOG, "Initialize    \n");
					fdbuff_out[0]=0x10;break;

		case C_FASTWRITE/*0x11*/:  
					PRINTDEBUG(DSK_LOG, "(fast mode)");	// fast mode Write Sector
		case C_WRITE /*1*/:
					PRINTDEBUG(DSK_LOG, "Write Sector  \n");
					nn=fdbuff_in[1]; dd=fdbuff_in[2]; tt=fdbuff_in[3]; ss=fdbuff_in[4];
					if( !getDiskProtect( 0))		// FIX ME
						{
						write_d88sector(nn,dd,tt,ss-1,fdbuff_in+5);
						Result=0x40;
						}
					else
						{
						Result=0x01;		// not writeable
						}
                	break;

		case C_READ /*2*/:
			        PRINTDEBUG(DSK_LOG, "Read  Sector \n");
//            		fdbuff_out[0]=0x40;
					Result=0x40;
					break;

		case C_FASTREAD /*0x12*/:  PRINTDEBUG(DSK_LOG, "(fast mode)");	// fast mode  Send Data
		case C_SENDDATA /*3*/:
					nn=fdbuff_in[1]; dd=fdbuff_in[2]; tt=fdbuff_in[3]; ss=fdbuff_in[4];
					PRINTDEBUG4(DSK_LOG, "Send  Data   sectors=%d drive=%d track=%d sector=%d \n",nn,dd,tt,ss);
					memcpy( fdbuff_out, read_d88sector( nn,dd ,tt ,ss-1), 256*nn);
					Result=0x40;
					break;

		case C_SENDRESULTSTATUS /*6*/:
				    PRINTDEBUG(DSK_LOG, "Result Status \n");
//                  fdbuff_out[0]=0x40;
					fdbuff_out[0]= Result;
                	break;

		case C_SENDDRIVESTATUS /*7*/:
				    PRINTDEBUG(DSK_LOG, "Drive  Status \n");
					switch (disk_num) 		// drive numbers   0: 00h  1: 10h  2: 30h
						{
						case 0: fdbuff_out[0]= 0x00; break;
						case 1: fdbuff_out[0]= 0x10; break;
						case 2: fdbuff_out[0]= 0x30; break;
						}
				    break;

				// if 1DD disk then set 00h  ( �������[�h   fast mode)
				// if 1D  disk then set FFh  ( �񍂑����[�h normal mode)
				// �{���́A�������[�h�� �f�B�X�N�^�C�v�͊֌W�Ȃ��̂ł����A�����Ɏ������܂����B
			case C_XMIT /*0xB*/:
					PRINTDEBUG(DSK_LOG, "XMit   \n");
					fdbuff_out[0]=(is1DD( cur_drive))?  0 : 0xff ;break;
				// ����/�Жʐݒ�
				// b1  �h���C�u�P  1:���ʁ@0:�Ж�
				// b0  �h���C�u�O  1:���  0:�Ж�
		case 0x17:  PRINTDEBUG(DSK_LOG, "SetSurfaceMode"); break;
		default:    printf("Not implimented %d",fdbuff_in[0]);break;
		}
	fdindex_out=0;

	//OSD_Delay(10);
	PutDiskAccessLamp(0);
   //  dump( fdbuff_in , cmd_length[ fdbuff_in[0]]);
 }




// **********************************************
//     dump    for debug
// **********************************************
void dump(char *buff,int length)
 {
#if DSK_LOG
  int i;
  printf("%d\t ",length);
  for(i=0; i< length; i++)
      {
       printf("%02x ",*(buff+i) & 0xff);
      }
  printf("\n");
#endif
   }


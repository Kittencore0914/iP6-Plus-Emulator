/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                        sound.c                          **/
/**                                                         **/
/** by Windy 2002-2004                                      **/
/** This code is based on PC6001V written by Mr. Yumitaro.  **/
/*************************************************************/
#include <stdio.h>
#include "P6.h"
#include "buffer.h"
#include "fm.h"
#include "voice.h"
#include "Sound.h"
#include "schedule.h"
#include "SThread.h"
#include "types.h"

#include "sysdep/sysdep_dsp.h"
#include "sysdep/sysdep_dsp_priv.h"
#include "sysdep/plugin_manager.h"

#if HAVE_AVAILABILITY_H
#include <Availability.h>
#endif


#ifdef WIN32
#include "win/waveout.h"
#else
#include "OpenAL.h"
#endif




const struct plugin_struct *plugins;		// plugin
struct sysdep_dsp_struct *dsp;		// sound device



//int   keyclick=0;				// use key click sound 1: use

int UseSound =1;
int newUseSound;
int sound_rate = SOUND_RATE;
int sound_samples;




		// RING buffer
RINGBUFFER *streambuffer[ MAX_STREAMBUFFER ];

SMutex  *soundMutex;
short soundbuf[MAX_SOUND_BUFSIZE];	// sound buffer

static byte Regs[ 0xff];

void fmsound_update(int samples);

int testPutData(char *src,int length);

// ****************************************************************************
//          �T�E���h�̏����� 
// ****************************************************************************
int InitSound(void)
{
#ifdef SOUND
	int i;
	int ret=1;

	
	plugins = NULL;
#if defined(WIN32) && defined(USEWAVEOUT)
	plugins =  &sysdep_dsp_waveout;
#else
	plugins =  &sysdep_dsp_openal;
#endif



	if(!UseSound) {printf(" Sound is disabled !! \n"); return(0);}

	UseSound=0;


	if( plugins ==NULL)
		{
		printf("We're Sorry, The sound driver was not found.\n");
		return 0;
		}

	soundMutex = OSD_CreateMutex();		// create mutex
	if( !soundMutex) printf("mutex create ...FAILED \n");


	// ***** sound buffer size ******
	sound_samples = sound_rate   /60.0;			/* samples */
	sound_samples *= 2;
    //printf("*** sound_samples=%d ***",sound_samples);

	for(i=0; i< MAX_STREAMBUFFER ; i++)		/* �T�E���h�o�b�t�@�̒����̃����O�o�b�t�@���쐬 */
		streambuffer[i]= ringbuffer_Open( sound_samples);
		
	if(Verbose) printf("YM2203 emulation init....");
	if( !ym2203_Open())	{ PRINTFAILED; return 0; }

	if(Verbose) printf("OK\n voice emulation init....");
	if( !InitVoice())	{ PRINTFAILED; return 0; }
	if(Verbose) printf("OK\n");
	
	if( !Event_Add( EVENT_PSG, 1000, EV_LOOP|EV_MS ,NULL) )  return 0;

	/* �T�E���h�h���C�o�������� */

	if(Verbose) printf("Audio device Openning...");

	{
	struct sysdep_dsp_create_params params = {0.0, NULL, sound_rate, SYSDEP_DSP_16BIT , 0};
	dsp = (struct sysdep_dsp_struct *)plugins-> create(&params);
	if( dsp ==NULL) { PRINTFAILED; return 0;}
	printf("OK\n");
	}
	
    if( plugins )
        if( plugins->init )
            plugins->init();

#ifdef UNIX
//	ret = unix_soundOpen();			// unix �����̏��������� �@dsp->init()�ɓ����H
//	if( ret ==0)
//		return 0;
#endif




#if SND_LOG
	testPutHeader(1);      // �e�X�g�p�@�ŏ��Ƀw�b�_����������
#endif

	UseSound=1;
	return(ret);
#endif
}


// ****************************************************************************
//          �T�E���h�̌㏈��
// ****************************************************************************
void TrashSound(void)
{
#ifdef SOUND
	int i;

	if(!UseSound) return;

	if( dsp !=(struct sysdep_dsp_struct *)NULL)
		dsp->destroy( dsp );


#ifdef UNIX
//	unix_soundClose();	// unix �����̏I������
#endif

	UseSound=0;

	// *********** ���ʂ̏��� ****************************
	for(i=0; i< MAX_STREAMBUFFER ;i++)
		ringbuffer_Close( streambuffer[i] );
	//voice_Close();

	TrashVoice();

	
#if SND_LOG
	testPutHeader(2);      // �e�X�g�p�@�Ō�ɒ�������������
#endif

	OSD_CloseMutex( soundMutex);		// close mutex
	if( !soundMutex) printf("mutex FAILED \n");

	return;
#endif
}


// ****************************************************************************
//          FM�����̃��W�X�^�[�ɏ�������
// ****************************************************************************
void SoundOut(byte R,byte V)
{

	/* ���W�X�^��ύX����O�ɁA�X�g���[�����X�V���Ă���*/	

    if( Regs[ (int)R ] !=V)
		{
		fmsound_update(0);
	  	PRINTDEBUG2(SND_LOG,"[Sound][SoundOut] register %02X  value %02X \n",R,V);
		}

	ym2203_setreg(R,V);		/* ���W�X�^��ύX���� */
	Regs[ (int)R] = V;

}



// ****************************************************************************
// �X�V�T���v�����擾
// ****************************************************************************
int get_update_samples( void )
{
#ifdef SOUND
	int samples = (int)( (double)sound_rate * Event_Scale( EVENT_PSG ) + 0.5 );
	Event_Reset( EVENT_PSG );
	return samples;
#endif
}


// ****************************************************************************
// FM����  �X�g���[���X�V
//
// ����:	samples	�X�V�T���v����(-1:�c��o�b�t�@�S�� 0:�����N���b�N��)
// �Ԓl:	int		�X�V�T���v����
// ****************************************************************************
void fmsound_update(int samples)
{
#ifdef SOUND
	//short soundbuf[MAX_SOUND_BUFSIZE];
	int   i;
	int   length;

	if( !UseSound) return;
	
	OSD_MutexLock( soundMutex);

	length = get_update_samples();		/* �X�V����ׂ��T���v�������擾 */
	if( samples == 0)					/* ���������N���b�N�� */
		{
		int tmp;
		length = min( length , tmp= ringbuffer_FreeNum( streambuffer[ AUDIOBUFFER]) );
		PRINTDEBUG3(SND_LOG,"[sound][fmsound_update(0)] do clock    \t update samples=%4d  rate=%3.1f  ringbuffer_FreeNum=%d\n", length ,(float)length/(float)sound_samples*100.0 ,tmp);
		}
	else if( samples >1)				/* �w�肵���N���b�N�� */
		{
		length = min( samples ,ringbuffer_FreeNum( streambuffer[ AUDIOBUFFER]) );
		PRINTDEBUG3(SND_LOG,"[sound][fmsound_update(%4d)] shitei clock \t update samples=%4d  rate=%3.1f\n",samples ,  length ,(float)length/(float)sound_samples*100.0);
		}
	else								/* �c��o�b�t�@�S�� */
		{
		length = ringbuffer_FreeNum( streambuffer[ AUDIOBUFFER]);	/* get samples to update */
		PRINTDEBUG2(SND_LOG,"[sound][fmsound_update(-1)] full remain buf \t update samples=%4d  rate=%3.1f\n", length ,(float)length/(float)sound_samples*100.0);
		}
						// channel=1 , left sound only 2003/1/2

	ym2203_makewave( soundbuf , length *sizeof(short));
			
	//ringbuffer_Write( streambuffer[ AUDIOBUFFER] , soundbuf, length);
	// �g�`���A�����O�o�b�t�@�ɏ�������
	//ym2203_Count( 1000);


	for(i=0; i< length; i++)
			{
			ringbuffer_Put( streambuffer[ AUDIOBUFFER] ,soundbuf[i]);
			}
	OSD_MutexUnlock( soundMutex);
	//ringbuffer_Write(  streambuffer[ AUDIOBUFFER] ,soundbuf ,length);

    //printf("*** sound_samples=%d ***",sound_samples);

#endif
}

// ****************************************************************************
//			�X�g���[�����~�L�V���O�����āA�����ɏ�������
// ****************************************************************************
void SoundUpdate(void)
{
#ifdef SOUND
	//static dword TClock=0;
	//char buf[10];
	//DWORD rc;
	//int R,V;
	//int tmp;
    //int ret=0;
	int i;

	{
	//int s = OSD_GetTicks() - TClock;
	//TClock = OSD_GetTicks();
	//PRINTDEBUG1(SND_LOG,"WON_DONE s= %d \n",s);
	}

	fmsound_update(-1);		// �c��o�b�t�@����S���X�V����

	//int exsam =0;
	//int i;
	//for(i=0; i< MAX_STREAMBUFFER ; i++)
	//	exsam = min ( max( exsam ,ringbuffer_DataNum( streambuffer[i])), sound_samples);
	//printf(" voicebuffer datas = %d --> ",tmp);

	PRINTDEBUG2( SND_LOG ,"[Sound][SoundUpdate] mixing & write device   sound_samples=%d  voicedata = %d  \n",sound_samples ,ringbuffer_DataNum( streambuffer[VOICEBUFFER]));
    PRINTDEBUG(SND_LOG,"--------------------------------------------------------------------- \n");

					
	// �����O�o�b�t�@����ǂݍ��݂A�~�L�V���O����
 	//memset(soundbuf ,0,sound_samples*sizeof(short));

	OSD_MutexLock( soundMutex);
	for(i=0; i< sound_samples ; i++)
			{
			 short dat;
					 

			 if( ringbuffer_Get( streambuffer[ AUDIOBUFFER ] ,&dat))
				soundbuf[i] =  dat;
			 if( ringbuffer_Get( streambuffer[ VOICEBUFFER ] ,&dat))
				soundbuf[i]+=  dat;

			 //soundbuf[i] = LIMITS( soundbuf[i], -32768 , 32767);
			}
	OSD_MutexUnlock( soundMutex);

	dsp->write( dsp , (unsigned char*)soundbuf, sound_samples);

    //printf("*** sound_samples=%d ***",sound_samples);


#if SND_LOG
/*	 {
	 int i;
	  printf("----------------------------");
	  for(i=0 ; i< sound_samples ;i++) 
		printf("%02X ",soundbuf[i] &0xff);
	  printf("\n----------------------------");
	 }
*/
	testPutData((char*)soundbuf, sound_samples* sizeof(short));

#endif
#endif
}


// ****************************************************************************
//			�����̒�~
// ****************************************************************************
void StopSound(void)       
{
#ifdef SOUND
	if( !UseSound) return;

	if( dsp != (struct sysdep_dsp_struct *)NULL)
		{
		if( dsp->stop != (void (*)()) NULL)
			dsp->stop();
		}

	/*if(UseSound)  SuspendThread( hThread1); */ 
#endif
}

// ****************************************************************************
//			�����̍ĊJ
// ****************************************************************************
void ResumeSound(void)     
{
#ifdef SOUND
	if( !UseSound) return;

	if( dsp != (struct sysdep_dsp_struct *)NULL ) 
		{
		if( dsp->resume !=  (void (*)())NULL)
			dsp->resume();
		}

	/*if(UseSound)  ResumeThread( hThread1); */
#endif
}




void FlushSound(void)      
{ 
	/* SoundOut(0xFF,0); */
}


void PSGOut(byte R,byte V) 
{
#ifdef SOUND
	if(R<0xc0||R==0xFF) SoundOut(R,V); 
#endif
}

void SCCOut(byte R,byte V) 
{
	/*if(UseSCC&&R<0x90) SoundOut(R+0x10,V);  */
}

#if SND_LOG
static int   testlength=0;


// ********************** put data of wav file *****************************
int testPutData(char *src,int length)
    {
      FILE *fp;
      char path[256];
      strcpy( path,"output.wav");

      fp=fopen( path,"a+b");
      if(fp==NULL) {printf(" %s open error\n",path);  exit(0);}
      fwrite( src,length,1,fp);
	  
      fclose(fp);
	  
	  testlength +=length;
      return(0);
     }

// ********************** put header of wav file *****************************
// flag: 1�ŁA��̃w�b�_����������
//       2�ŁA�����Ƃ����w�b�_����������
int testPutHeader(int flag)
    {
     int maxLen;

	 char mode[20];

#ifndef WIN32			// at not windows enviroment
	typedef struct { 
  		word  wFormatTag; 
  		word  nChannels; 
  		dword nSamplesPerSec; 
  		dword nAvgBytesPerSec; 
  		word  nBlockAlign; 
  		word  wBitsPerSample; 
  		word  cbSize; 
		} WAVEFORMATEX;
	#define WAVE_FORMAT_PCM 1 
#endif

      WAVEFORMATEX wfx;	// format setting
      FILE *fp;
      char buf[5];
      int tmp;
      char path[256];

     strcpy( path,"output.wav");
   // --------------- user setting -------------------------
     maxLen = testlength+46;		// length of max
     wfx.nChannels     = CHANNEL;	// channel             2ch / 1ch
     wfx.nSamplesPerSec= sound_rate;	// sampling rate freq. 44100/ 22050 ...
     wfx.wBitsPerSample= SAMPLING_BITS;// sampling bit        16bit/ 8bit

   // --------------- don't touch  -------------------------
     wfx.wFormatTag    = WAVE_FORMAT_PCM;
     wfx.nBlockAlign   = wfx.nChannels * wfx.wBitsPerSample / 8;
     wfx.nAvgBytesPerSec=wfx.nSamplesPerSec * wfx.nBlockAlign;
     wfx.cbSize = 0;

	 switch( flag)
		{
	 	case 1: strcpy(mode,"w+b");	break;	// �ŏ�
	 	case 2: strcpy(mode,"r+b");	break;	// �Ō�
		}

      fp=fopen(path , mode);
      if(fp==NULL) {printf(" %s open error\n",path); return(0);}
	  fseek(fp,0,SEEK_SET);		// �擪�ɖ߂�

      strcpy( buf,"RIFF");  fwrite( buf,4,1,fp);
      tmp= maxLen-8;
      memcpy( &buf,&tmp,4); fwrite( buf,4,1,fp);
      strcpy( buf,"WAVE");  fwrite( buf,4,1,fp);

      strcpy( buf,"fmt ");  fwrite( buf,4,1,fp);
      tmp= sizeof(WAVEFORMATEX);
      memcpy( &buf,&tmp,4); fwrite( buf,4,1,fp);
      fwrite( &wfx, tmp,1,fp);

      strcpy( buf,"data");  fwrite( buf,4,1,fp);
      tmp= testlength;
      memcpy( &buf,&tmp ,4);fwrite( buf,4,1,fp);
      fclose(fp);

      return(0);
     }
#endif

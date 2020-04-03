/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         voice.c                         **/
/**                                                         **/
/** translate from PC6001V's C++ to C  by windy             **/
/**                                                         **/
/** Original program:   PC6001V                             **/
/** Original author:    Yumitaro.                           **/
/** Original URL:       http://www.papicom.net/p6v/         **/
/*************************************************************/


#define VOICE_DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "voice.h"
#include "Pd7752.h"
#ifdef X11
#include "OpenAL.h"
#endif

#ifdef UNIX
#include <unistd.h>
#endif

#include "Sound.h"
#include "SThread.h"
#include "Timer.h"
#include "buffer.h"
#include "P6.h"
#include "types.h"
#include "schedule.h"
#include "wav.h"

#include "Z80.h"



// ****************************************************************************
//		thread Variables
// ****************************************************************************

//pthread_t voiceThread;
//static pthread_mutex_t voiceMutex;
static SMutex    *voiceMutex;		// voice mutex
static SThread   *voiceThread;		// voice thread object
//static SEvent    *voiceEvent;

extern SMutex    *soundMutex;		// sound mutex


static int ThreadLoopStop = 1;
static int VoiceCancel = 0;
int VoiceFlag = 0;					// Voice������1�ɂȂ�t���O

int voiceHz;

// ****************************************************************************
//		Variables
// ****************************************************************************

// I/O�|�[�g
static byte portE0;
static byte portE2;
static byte portE3;
	
static int VStat;					// �X�e�[�^�X
	
// ������p...
int IVLen;					// �T���v����
int *IVBuf=NULL;			// �f�[�^�o�b�t�@
int IVPos;					// �Ǎ��݃|�C���^

// �p�����[�^�o�b�t�@(1�t���[���̃p�����[�^����7��)
static byte ParaBuf[7];             // �p�����[�^�o�b�t�@
static byte Pnum;					// �p�����[�^��
static int Fnum;					// �J��Ԃ��t���[����
static int PReady;					// �p�����[�^�Z�b�g����

static D7752_SAMPLE *Fbuf = NULL;	// �t���[���o�b�t�@�|�C���^(10kHz 1�t���[��)

void FreeVoice( void );

	/*unsigned char voicebuf[samples];
	for(i=0; i<samples; i++) {
		int dat = (double)Fbuf[i*GetFrameSize()/samples]/10.0;
		if (dat > 127) dat = 127;
		if (dat < -128) dat = -128;
		voicebuf[i] = dat+128;
	}*/
	//coreaudio_dsp_write(dsp, voicebuf, samples) ;


// ****************************************************************************
//	�T���v�����O���[�g�ƃr�b�g����ϊ����� �X�g���[���ɏ�������
// ****************************************************************************
static void UpConvert(void)
{
	int i;
	//short *voicebuf;
	int framesize = GetFrameSize();

	// 10kHz -> ���ۂɍĐ�����T���v�����O���[�g�ɕϊ��������̃T���v����
	int samples = GetFrameSize() * sound_rate / 10000;

	PRINTDEBUG2(VOI_LOG ,"[voice][UpConvert] WRITTING VOICE DATA    framesize=%d -> samples=%d \n",GetFrameSize(), samples);

	if( UseSound)
		{
		OSD_MutexLock( soundMutex);
		for(i=0; i< samples ; i++)
			{
			int dat = Fbuf[ i * framesize/samples]*4;		// * 4 �� 16bit<-14bit �̂���
			ringbuffer_Put( streambuffer[VOICEBUFFER] ,  dat*2);	// �o�̓��x�����Ⴂ�̂łƂ肠����2�{
			}
		OSD_MutexUnlock( soundMutex);
		PRINTDEBUG1(VOI_LOG,"[voice][Upconvert] VOICE DATA=%d \n",ringbuffer_DataNum( streambuffer[VOICEBUFFER]));
		}


	//testPutData(voicebuf ,samples  * sizeof(short));
	//printf("samples == %d \n",samples);
	//OSD_MutexLock( voiceMutex);
	//ringbuffer_Write(audiobuffer ,voicebuf ,samples * sizeof(short));
	//OSD_MutexUnlock( voiceMutex);
	
	//free( voicebuf);
}

// ****************************************************************************
//	�@�������~����
// ****************************************************************************
static void AbortVoice(void)
{
	Event_Del( EVENT_VOICE);

	// �X���b�h�̃��[�v��~
	ThreadLoopStop = 1;
	// �c��̃p�����[�^�̓L�����Z��
	Pnum = Fnum = 0;
	PReady = FALSE;
	// �t���[���o�b�t�@�J��
	if (Fbuf) {
		free(Fbuf);
		Fbuf = NULL;
	}
	VStat &= ~D7752E_BSY;
}





int VoiceMain(void* arg)
{
 return 0;	
}


int updateVoice(int num)
{
	int ret=0;
	while( num-- )
		{
		//SndDev::Buf->Put( ( IVBuf[IVPos++] * SndDev::Volume ) / 100 );
		ringbuffer_Put( streambuffer[VOICEBUFFER] , ( IVBuf[IVPos++] * 100 ) / 100);
		}

	if( IVPos >= IVLen ){	// �Ō�܂Ŕ��������甭���I��
		AbortVoice();
	}
}


extern reg R;

// ****************************************************************************
//			���������@�ʃX���b�h�ŁA�p�����[�^���󂯎��A�g�`�𐶐�����B
// ****************************************************************************
int VoiceMainLoop(void* arg)
{
	int cnt =0;
	//UPeriod_bak = UPeriod =30;

	PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] THREAD START \n");
	//OSD_InitTimer();
	while (1) {
		//int handle, stat;
		//OSD_OpenEvent( voiceEvent, "voiceEvent");
		//stat = WaitForSingleObject( voiceEvent->handle ,30);

		if (VoiceCancel) {  // �������Ƀ��Z�b�g���ꂽ��B�B�B
			PRINTDEBUG(VOI_LOG , "[voice][VoiceMainLoop] VOICE RESET \n");
			VoiceCancel = 0;
			AbortVoice();
			VoiceFlag = 0;
			//pthread_exit(NULL);	
			PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] THREAD END \n");
			break;
		} else if (!ThreadLoopStop) {
			//pthread_mutex_lock(&voiceMutex);
			OSD_MutexLock( voiceMutex);

			if(VStat & D7752E_EXT) {	// �O���唭������
				if(PReady) {	// �p�����[�^�Z�b�g�������Ă�?
					PRINTDEBUG( VOI_LOG ,"[voice][VoiceMainLoop] EXT VOICE. PARAMETER is done.\n");
					// �t���[����=0�Ȃ�ΏI������
					if(!(ParaBuf[0]>>3)) {
						PRINTDEBUG( VOI_LOG, "[voice][VoiceMainLoop] if frame ==0 then end\n");
						
						AbortVoice();
						//pthread_mutex_unlock(&voiceMutex);
						OSD_MutexUnlock( voiceMutex);
						VoiceFlag = 0;
						//pthread_exit(NULL);
						break;		// exit thread
					} else {
						// 1�t���[�����̃T���v������
						Synth(ParaBuf, Fbuf);
						// �T���v�����O���[�g��ϊ����ăo�b�t�@�ɏ�����
						UpConvert();
						// ���t���[���̃p�����[�^���󂯕t����
						PReady = FALSE;
						ThreadLoopStop = 1;
						VStat |= D7752E_REQ;


					}
				} else {
					PRINTDEBUG(VOI_LOG , "[voice][VoiceMainLoop] VOICE STOP\n");
					AbortVoice();		// ������~
					VStat = D7752E_ERR;
				}
			}else{						// �����唭������
				// 1�t���[�����̃T���v�����o�b�t�@�ɏ�����
				int num = min( IVLen - IVPos, GetFrameSize() * sound_rate / 10000 );
				PRINTDEBUG( VOI_LOG, "[voice][VoiceMainLoop] INTERNAL VOICE " );
				PRINTDEBUG3( VOI_LOG, "num=%d %d/%d\n", num, IVPos ,IVLen);
				OSD_MutexLock( soundMutex);
				while( num-- )
					{
					//SndDev::Buf->Put( ( IVBuf[IVPos++] * SndDev::Volume ) / 100 );
					ringbuffer_Put( streambuffer[VOICEBUFFER] , ( IVBuf[IVPos++] * 100 ) / 100);
					}
				OSD_MutexUnlock( soundMutex);

				if( IVPos >= IVLen ){	// �Ō�܂Ŕ��������甭���I��
					AbortVoice();
				}
			}
			//pthread_mutex_unlock(&voiceMutex);			
			OSD_MutexUnlock( voiceMutex);
		}
		usleep(300);	// ���X�AUnix�����̊֐��B�}�C�N���b�X���[�v����BWindows�ł́A��ւ̊֐����쐬�B win32.c 
	}
	PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] THREAD END \n");
	//OSD_TrashTimer();

	//UPeriod_bak = UPeriod =2;
	return 0;
}



// ****************************************************************************
//		Voice �X���b�h���L�����Z������
// ****************************************************************************
void CancelVoiceThread(void)
{
	if (VoiceFlag) {
		VoiceCancel = 1;
	}
}


// ****************************************************************************
//		������ wave �t�@�C���ǂݍ���
// ****************************************************************************
int  LoadVoice( char *filename )
{
	int i;
	struct _spec ws;
	int len;		// ���f�[�^�T�C�Y
	char *buf;	// ���f�[�^�o�b�t�@
	char filepath[PATH_MAX];
	char path[PATH_MAX];

	buf=NULL;

	path[0]=0;
#ifdef WIN32
	if( !IsDebuggerPresent())
		 strcpy( path, "..\\..\\wav\\");	// on normal
	else
		 strcpy( path ,"..\\wav\\");		// on debugger
#endif

	PRINTDEBUG1( VOI_LOG, "[VOICE][LoadVoice]  [%s] -> ", filename );

	// WAV�t�@�C����Ǎ���
	sprintf( filepath, "%s%s", "wav\\", filename );
	PRINTDEBUG1( VOI_LOG, "%s ->", filepath );
	
	//if( !SDL_LoadWAV( filepath, &ws, &buf, &len ) ){
	if( !loadWav( filepath , &ws, &buf , &len) ) {			// �܂��Awav�f�B���N�g������ǂ� 
		sprintf( filepath, "%s%s", path, filename );
		PRINTDEBUG1( VOI_LOG, "%s ->", filepath );
	
		if( !loadWav( filepath , &ws, &buf , &len) ) {		// �w��f�B���N�g������ǂ�
			PRINTDEBUG( VOI_LOG, "Error!\n" );
			return FALSE;
		}
	}
	
	// 22050Hz�ȏ�,16bit,1ch �łȂ�������_��
	if( ws.freq < 22050 || ws.samplebit != 16 || ws.channels != 1 ){
		PRINTDEBUG3( VOI_LOG, "F:%d S:%d C:%d -> Error!\n", ws.freq, ws.samplebit, ws.channels );
		//SDL_FreeWAV( buf );
		freeWav( buf );
		return FALSE;
	}
	PRINTDEBUG( VOI_LOG, "OK\n" );
	
	FreeVoice();

	// �ϊ���̃T�C�Y���v�Z���ăo�b�t�@���m��
	// �������x4�̎�,1�t���[���̃T���v������160
	IVLen = (int)( (double)sound_rate * (double)(len/2) / (double)ws.freq
					* (double)GetFrameSize() / (double)160 );
	
	//PRINTD2( VOI_LOG, "Len:%d/%d ->", IVLen, len );
	

	//IVBuf = new int[IVLen];
	IVBuf = malloc( IVLen *sizeof(int));
	if( !IVBuf ){
		//SDL_FreeWAV( buf );
		freeWav( buf );
		IVLen = 0;
		return FALSE;
	}
	
	// �ϊ�
	{
	short *sbuf = (short *)buf;
	for( i=0; i<IVLen; i++ ){
		IVBuf[i] = sbuf[(int)(( (double)i * (double)(len/2) ) / (double)IVLen)];
		}
	
	// WAV�J��
	//SDL_FreeWAV( buf );
	freeWav( buf );

	// �Ǎ��݃|�C���^������
	IVPos = 0;
	}

	return TRUE;
}


// ****************************************************************************
//		������ wave �t�@�C�����
// ****************************************************************************
void FreeVoice( void )
{
	if( IVBuf ){
		//delete [] IVBuf;
		free( IVBuf);
		IVBuf = NULL;
		IVLen = 0;
	}
}


// ****************************************************************************
//		���������̏�����
// ****************************************************************************
int InitVoice(void)
{
	portE0 = portE2 = portE3 = 0;
	Pnum    = 0;
	Fnum    = 0;
	PReady 	= FALSE;
	Fbuf    = NULL;
	VStat   = D7752E_IDL;

	// ���[�v���~���Ă���
	ThreadLoopStop = 1;

	// mutex����
	//if (pthread_mutex_init(&voiceMutex, NULL) != 0) return 0;
	voiceMutex = OSD_CreateMutex();
	if( voiceMutex ==0) return(0);

	//voiceEvent = OSD_CreateEvent("voiceEvent");


	return 1;
}

// ****************************************************************************
//		���������̌㏈��
// ****************************************************************************
void TrashVoice(void)
{
	//pthread_mutex_destroy(&voiceMutex);
	OSD_CloseMutex( voiceMutex);
	//OSD_CloseEvent( voiceEvent);
	if(Fbuf) free(Fbuf);
	
	//Event_Del( EVENT_VOICE);		// ���������C�x���g���폜


}

// ****************************************************************************
//		���[�h�Z�b�g
// ****************************************************************************
static void VSetMode(byte mode)
{
	PRINTDEBUG1(VOI_LOG, "[VOICE][VSetMode]      VOICE START , CLEAR STATUS  mode=%X\n",mode);
	//pthread_mutex_lock(&voiceMutex);
	OSD_MutexLock( voiceMutex);

	// ���������J�n
	PD7752_Start(mode);
	// �X�e�[�^�X�N���A
	VStat = D7752E_IDL;

    //pthread_mutex_unlock(&voiceMutex);
	OSD_MutexUnlock( voiceMutex);
}


// ****************************************************************************
//		�R�}���h�Z�b�g
// ****************************************************************************
static void VSetCommand(byte comm)
{
	char filename[ PATH_MAX];

	//pthread_mutex_lock(&voiceMutex);
	OSD_MutexLock( voiceMutex);
	// �������Ȃ�~�߂�
	AbortVoice();
	switch (comm) {
	case 0x00:		// ������I���R�}���h ----------
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
		// ������Đ����[�h
		PRINTDEBUG1(VOI_LOG,"[voice][VSetCommand] %02X ",comm);
		PRINTDEBUG( VOI_LOG,"INT VOICE CMD \n");
		// �X���b�h���� �i�������̂��߁A���������X���b�h�𐶐�����j
		//pthread_create(&voiceThread, NULL, VoiceMainLoop, NULL);

		PRINTDEBUG( VOI_LOG,"[voice][VSetCommand] Create Thread \n");

		voiceThread = (SThread*) OSD_CreateThread( VoiceMainLoop ,NULL); 
		if( !voiceThread ) {printf("voice thread  create ....FAILED \n"); break;}
		
		OSD_Delay(3);

		
		VoiceFlag = 1;
		// �t���[���o�b�t�@�m��
		Fbuf = malloc(sizeof(D7752_SAMPLE)*GetFrameSize());
		if(!Fbuf) break;

		//������@WAVE�f�[�^�ǂݍ���

		sprintf( filename ,"f4%d.wav", comm);
		LoadVoice( filename );

		// �X�e�[�^�X�������Đ����[�h�ɃZ�b�g
		VStat = D7752E_BSY;
		PRINTDEBUG(VOI_LOG, "[voice][VSetCommand] start to recive parameter  \n");
		
		//Event_Add( EVENT_VOICE , (double)10000.0/(double)GetFrameSize() ,EV_LOOP|EV_HZ , VoiceCallback );

		// �X���b�h�ĊJ
		ThreadLoopStop=0;
		break;
	case 0xfe:		// �O����I���R�}���h ----------
		if( sr_mode) VoiceIntFlag = INTFLAG_REQ;
		PRINTDEBUG1(VOI_LOG,"[voice][VSetCommand] %02X ",comm);
		PRINTDEBUG( VOI_LOG,"EXT VOICE CMD \n");
		// �X���b�h���� �i�������̂��߁A���������X���b�h�𐶐�����j
		//pthread_create(&voiceThread, NULL, VoiceMainLoop, NULL);

		PRINTDEBUG( VOI_LOG,"[voice][VSetCommand] Create Thread \n");

		voiceThread = (SThread*) OSD_CreateThread( VoiceMainLoop ,NULL); 
		if( !voiceThread ) {printf("voice thread  create ....FAILED \n"); break;}
		
		OSD_Delay(3);

		
		VoiceFlag = 1;
		// �t���[���o�b�t�@�m��
		Fbuf = malloc(sizeof(D7752_SAMPLE)*GetFrameSize());
		if(!Fbuf) break;
		// �X�e�[�^�X���O����Đ����[�h�ɃZ�b�g�C�p�����[�^��t�J�n
		VStat = D7752E_BSY | D7752E_EXT | D7752E_REQ;
		PRINTDEBUG(VOI_LOG, "[voice][VSetCommand] start to recive parameter  \n");

		//Event_Add( EVENT_VOICE , (double)10000.0/(double)GetFrameSize() ,EV_LOOP|EV_HZ , VoiceCallback );

		break;
	case 0xff:		// �X�g�b�v�R�}���h ----------
		break;
	default:		// �����R�}���h  ----------
		VStat = D7752E_ERR;	// �z���g?
		break;
	}
	//pthread_mutex_unlock(&voiceMutex);	
	OSD_MutexUnlock(voiceMutex);
}


// ****************************************************************************
//		�����p�����[�^�̓]��
// ****************************************************************************
static void VSetData(byte data)
{
	//pthread_mutex_lock(&voiceMutex);
	OSD_MutexLock(voiceMutex);


	PRINTDEBUG1(VOI_LOG,"[voice][VSetData Data]     [%02X] \n",data);
	
	// �Đ����̂݃f�[�^���󂯕t����
	if ((VStat & D7752E_BSY)&&(VStat & D7752E_REQ)) {
		if (Fnum == 0 || Pnum) {	// �ŏ��̃t���[��?
			// �ŏ��̃p�����[�^��������J��Ԃ�����ݒ�
			if(Pnum == 0) {
				Fnum = data>>3;
				if (Fnum == 0) { PReady = TRUE; ThreadLoopStop = 0; }
				PRINTDEBUG(VOI_LOG, "[voice][VSetData] FIRST PARAMETER, SET REPEAT TIME\n");
			}
			// �p�����[�^�o�b�t�@�ɕۑ�
			ParaBuf[Pnum++] = data;
			// ����1�t���[�����̃p�����[�^�����܂����甭����������
			if(Pnum == 7) {
				VStat &= ~D7752E_REQ;
				Pnum = 0;
				if(Fnum > 0) Fnum--;
				ThreadLoopStop = 0;
				PReady = TRUE;
				PRINTDEBUG(VOI_LOG , "[voice][VSetData] If the parameter for one frame collects, a vocal preparation is completed. *** \n");
			}
		} else {						// �J��Ԃ��t���[��?
			// �p�����[�^�o�b�t�@�ɕۑ�
			// PD7752�̎d�l�ɍ��킹��
			int i;
			PRINTDEBUG(VOI_LOG,"[voice][VSetData Data]     �J��Ԃ��t���[��? \n");

			for(i=1; i<6; i++) ParaBuf[i] = 0;
			ParaBuf[6] = data;
			VStat &= ~D7752E_REQ;
			Pnum = 0;
			Fnum--;
			ThreadLoopStop = 0;			
			PReady = TRUE;
		}
	}
	//pthread_mutex_unlock(&voiceMutex);
	OSD_MutexUnlock( voiceMutex);


	if( sr_mode) VoiceIntFlag = INTFLAG_REQ;
}

// ****************************************************************************
//		�X�e�[�^�X���W�X�^���擾
// ****************************************************************************
static int VGetStatus(void)
{
	int ret;
	//PRINTDEBUG( VOI_LOG,"[voice][VGetStatus]\n");
	//pthread_mutex_lock(&voiceMutex);	
	OSD_MutexLock( voiceMutex);
	
	ret = VStat;


	//pthread_mutex_unlock(&voiceMutex);	
	OSD_MutexUnlock( voiceMutex);



	return ret;
}


// ****************************************************************************
//		I/O �A�N�Z�X�֐�
// ****************************************************************************

void OutE0H(byte data)
{
	VSetData(data);
}
void OutE2H(byte data)
{
	VSetMode(data);
}
void OutE3H(byte data)
{
	VSetCommand(data);
}
byte InE0H(void)
{
	int stat = VGetStatus();
#if 0
	static int cnt =0;

	if( (stat & 0x80) ==0x80 && (stat & 0x40)==0)
		{
		cnt++;
		printf(" %3d busy ",cnt);
		}
	else
		cnt=0;
	PRINTDEBUG(VOI_LOG, "[voice][VGetStatus]        Status=�@ " );
	if( stat & 0x80) PRINTDEBUG(VOI_LOG ,"BSY ") else PRINTDEBUG(VOI_LOG ,"--- ");
	if( stat & 0x40) PRINTDEBUG(VOI_LOG ,"REQ ") else PRINTDEBUG(VOI_LOG ,"--- ");
	if( stat & 0x20) PRINTDEBUG(VOI_LOG ,"EXT ") else PRINTDEBUG(VOI_LOG ,"INT ");
	if( stat & 0x10) PRINTDEBUG(VOI_LOG ,"ERR \n") else PRINTDEBUG(VOI_LOG ,"--- \n");
#endif
#if 0
	if( stat & 0x10) PRINTDEBUG(VOI_LOG ,"ERR \n") else PRINTDEBUG(VOI_LOG ,"--- \n");
#endif


	return stat;
}
byte InE2H(void)
{
	return portE2;
}
byte InE3H(void) {
	return portE3;
}



// Voice �X���b�h
/*
int VoiceMainLoop(void* arg)
{
	int cnt =0;
		UPeriod_bak = UPeriod =60;

	PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] �X���b�h�J�n \n");
	OSD_InitTimer();
	while (1) {
		//int handle, stat;
		//OSD_OpenEvent( voiceEvent, "voiceEvent");
		//stat = WaitForSingleObject( voiceEvent->handle ,30);

		if (VoiceCancel) {  // �������Ƀ��Z�b�g���ꂽ��B�B�B
			PRINTDEBUG(VOI_LOG , "[voice][VoiceMainLoop] �������Ƀ��Z�b�g���ꂽ�B\n");
			VoiceCancel = 0;
			AbortVoice();
			VoiceFlag = 0;
			//pthread_exit(NULL);	
			PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] �X���b�h�I�� \n");
			break;
		} else if (!ThreadLoopStop) {
			//pthread_mutex_lock(&voiceMutex);
			//OSD_MutexLock( voiceMutex);

			if(VStat & D7752E_EXT) {	// �O���唭������
				if(PReady) {	// �p�����[�^�Z�b�g�������Ă�?
					PRINTDEBUG( VOI_LOG ,"[voice][VoiceMainLoop] �O���唭������,�p�����[�^�Z�b�g�����B\n");
					// �t���[����=0�Ȃ�ΏI������
					if(!(ParaBuf[0]>>3)) {
						PRINTDEBUG( VOI_LOG, "[voice][VoiceMainLoop] �t���[�����O�A�I������\n");
						
						AbortVoice();
						//pthread_mutex_unlock(&voiceMutex);
						//OSD_MutexUnlock( voiceMutex);
						VoiceFlag = 0;
						//pthread_exit(NULL);
						break;
					} else {
						// 1�t���[�����̃T���v������
						Synth(ParaBuf, Fbuf);
						// �T���v�����O���[�g��ϊ����ăo�b�t�@�ɏ�����
						UpConvert();
						// ���t���[���̃p�����[�^���󂯕t����
						PReady = FALSE;
						ThreadLoopStop = 1;
						VStat |= D7752E_REQ;
					}
				} else {
					PRINTDEBUG(VOI_LOG , "[voice][VoiceMainLoop] ������~\n");
					AbortVoice();		// ������~
					VStat = D7752E_ERR;
				}
			}else{						// �����唭������
				// 1�t���[�����̃T���v�����o�b�t�@�ɏ�����
				int num = min( IVLen - IVPos, GetFrameSize() * sound_rate / 10000 );
				PRINTDEBUG( VOI_LOG, "[voice][VoiceMainLoop] �����唭������ " );
				//PRINTD2( VOI_LOG, "%d/%d\n", num, IVPos );
				while( num-- )
					{
					//SndDev::Buf->Put( ( IVBuf[IVPos++] * SndDev::Volume ) / 100 );
					ringbuffer_Put( streambuffer[VOICEBUFFER] , ( IVBuf[IVPos++] * 100 ) / 100);
					}

				if( IVPos >= IVLen ){	// �Ō�܂Ŕ��������甭���I��
					AbortVoice();
				}
			}
			//pthread_mutex_unlock(&voiceMutex);			
			//OSD_MutexUnlock( voiceMutex);
		}
		//if( ++cnt >10) {OSD_Delay(1); cnt=0;}
		OSD_Delay(1);
		//usleep(10);
		//Sleep(5);
	}
	PRINTDEBUG(VOI_LOG, "[voice][VoiceMainLoop] �X���b�h�I�� \n");
	OSD_TrashTimer();

	UPeriod_bak = UPeriod =2;
	return 0;
}
*/
// **************************************************
//	�@���̔g�`�f�[�^��p�ӂ��āA�X�g���[���ɏ�������
// **************************************************
/*
int SCALL VoiceCallback(void *arg)
{
	int cnt =0;
	PRINTDEBUG( VOI_LOG,"[VOICE][VoiceCallback]\n");

	if( !PReady ) {PRINTDEBUG2(VOI_LOG,"failed Pnum=%d PC=%04X \n" , Pnum ,R.PC.W); return 0;}

	if(VStat & D7752E_EXT) 
		{	// �O���唭������
		if(PReady) 
			{	// �p�����[�^�Z�b�g�������Ă�?
			PRINTDEBUG( VOI_LOG ,"[voice][VoiceCallback] �O���唭������,�p�����[�^�Z�b�g�����B\n");
			// �t���[����=0�Ȃ�ΏI������
			if(!(ParaBuf[0]>>3)) 
				{
				PRINTDEBUG( VOI_LOG, "[voice][VoiceCallback] �t���[�����O�A�I������\n");
						
				AbortVoice();
				VoiceFlag = 0;
				}
			else {
				// 1�t���[�����̃T���v������
				Synth(ParaBuf, Fbuf);
				// �T���v�����O���[�g��ϊ����ăo�b�t�@�ɏ�����
				UpConvert();
				// ���t���[���̃p�����[�^���󂯕t����
				PReady = FALSE;
				ThreadLoopStop = 1;
				VStat |= D7752E_REQ;
				}
			} else 
			{
			PRINTDEBUG(VOI_LOG , "[voice][VoiceCallback] ������~\n");
			AbortVoice();		// ������~
			VStat = D7752E_ERR;
			}
	}else{						// �����唭������
		// 1�t���[�����̃T���v�����o�b�t�@�ɏ�����
		int num = min( IVLen - IVPos, GetFrameSize() * sound_rate / 10000 );
		PRINTDEBUG( VOI_LOG, "[voice][VoiceCallback] �����唭������ " );
		//PRINTD2( VOI_LOG, "%d/%d\n", num, IVPos );
		while( num-- )
			{
			//SndDev::Buf->Put( ( IVBuf[IVPos++] * SndDev::Volume ) / 100 );
			ringbuffer_Put( streambuffer[VOICEBUFFER] , ( IVBuf[IVPos++] * 100 ) / 100);
			}

		if( IVPos >= IVLen ){	// �Ō�܂Ŕ��������甭���I��
					AbortVoice();
			}
		}


	return 0;
}
#if 0 // VOI_LOG
	PRINTDEBUG(VOI_LOG, "[voice][VGetStatus]        Status=�@ " );
	if( ret & 0x80) PRINTDEBUG(VOI_LOG ,"BSY ") else PRINTDEBUG(VOI_LOG ,"--- ");
	if( ret & 0x40) PRINTDEBUG(VOI_LOG ,"REQ ") else PRINTDEBUG(VOI_LOG ,"--- ");
	if( ret & 0x20) PRINTDEBUG(VOI_LOG ,"EXT ") else PRINTDEBUG(VOI_LOG ,"INT ");
	if( ret & 0x10) PRINTDEBUG(VOI_LOG ,"ERR \n") else PRINTDEBUG(VOI_LOG ,"--- \n");
#endif
*/

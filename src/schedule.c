/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         schedule.c                      **/
/**                                                         **/
/** translate from PC6001V's C++ to C  by windy             **/
/**                                                         **/
/** Original program:   PC6001V                             **/
/** Original author:    Yumitaro.                           **/
/** Original URL:       http://www.papicom.net/p6v/         **/
/*************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "schedule.h"
#include "types.h"
#include "P6.h"



evinfo   ev[ MAX_EVENT];
int      SaveCloclk;	// ���̃C�x���g�����p
int      WRClockTmp;	// ���s���x�v�Z�p
int		 NextEvent;

void Event_init(void)
{
	int i;
	// �S�ẴC�x���g���폜���Ă���
	for(i=0; i< MAX_EVENT; i++)
		Event_Del( i);
	SaveCloclk =0;	// ���̃C�x���g�����p
	WRClockTmp =0;	// ���s���x�v�Z�p
	NextEvent  =0;
}

// ****************************************************************************
// �C�x���g�ǉ�
//
// ����:	dev		�C�x���g�f�o�C�X�I�u�W�F�N�g�|�C���^
//			id		�C�x���gID
//			hz		�C�x���g�������g��
//			flag	�C�x���g�X�^�C���w��
//				bit0  : �J��Ԃ��w�� 0:�����^�C�� 1:���[�v
//				bit2,3,4: �����w��P�� 000:Hz 001:us 010:ms 100:CPU�X�e�[�g��
// �Ԓl:	BOOL	TRUE:���� FALSE:���s
// ****************************************************************************
int Event_Add( int id, double hz ,int flag, int (SCALL *CallbackProc)( int id, void*) )
{
	double master_clock= (( (double)(2000000.0*0.58) *86)/100);
//	if(sr_mode)
//		master_clock *= 1.18 ; // 0.9; // 1.0; // 1.27; // 1.15; // 1.2; // 1.3;		// sr_mode ?
//	else
//	    master_clock *= 1.0; // 1.05;// 0.85; // 0.65;  // 0.68; // 0.77;  // 0.85;  


	switch( flag&(EV_US|EV_MS|EV_STATE) )
		{
		case EV_MS:		// ms�Ŏw��
			ev[id].Active = TRUE;
			ev[id].nps    = (double)1000 / hz;
			ev[id].Clock  = (int)((double)master_clock / ev[id].nps);
			break;
		default:	// Hz �Ŏw��
			ev[id].Active = TRUE;
			ev[id].nps   = hz;
			ev[id].Clock = (int)((double)(master_clock / hz));
			break;
		}

				// CALLBACK �֐��̐ݒ�
	ev[id].CallbackProc = CallbackProc;

				// �����̐ݒ�
	if( flag & EV_LOOP )	// ���[�v�C�x���g
		{
		ev[id].Period = ev[id].Clock;
		if( ev[id].Period < 1 ) ev[id].Period = 1;
	
		}
	else				// �����^�C���C�x���g
		ev[id].Period = 0;

	// ���̃C�x���g�܂ł̃N���b�N���X�V
	if( NextEvent < 0 ) 
		NextEvent = ev[id].Clock;
	else
		NextEvent = min( NextEvent, ev[id].Clock );
			
	return 1;
}

// ****************************************************************************
// �w��C�x���g�����Z�b�g����
//
// ����:	dev		�C�x���g�f�o�C�X�I�u�W�F�N�g�|�C���^
//			id		�C�x���gID
// �Ԓl:	�Ȃ�
// ****************************************************************************
void Event_Reset( int id )
{
	if( ev[id].Active ){
		// ���ߍ��񂾃N���b�N�����l��
		ev[id].Clock = ev[id].Period + SaveCloclk;
	}
}

// ****************************************************************************
// �C�x���g�폜
//
// ����:	dev		�C�x���g�f�o�C�X�I�u�W�F�N�g�|�C���^
//			id		�C�x���gID
// �Ԓl:	BOOL	TRUE:���� FALSE:���s
// ****************************************************************************
void Event_Del(int id)
{
	ev[id].Active = 0;
	ev[id].CallbackProc = NULL;
}

// ****************************************************************************
// �C�x���g�X�V
//
// ����:	clk		�i�߂�N���b�N��
// �Ԓl:	�Ȃ�
// ****************************************************************************
int Event_Update(int clk)
{
	int i;
	int cnt;

	// �N���b�N�𗭂ߍ���
	SaveCloclk += clk;	// ���̃C�x���g�����p
	WRClockTmp += clk;	// ���s���x�v�Z�p
	
	// ���̃C�x���g�����N���b�N�ɒB���Ă��Ȃ�������߂�
	// ������ clk=0 �̏ꍇ�͍X�V���s��
	if( NextEvent > SaveCloclk && clk )
		return 0;
	NextEvent = -1;
	

	do{
		cnt = 0;
		for( i=0; i<MAX_EVENT; i++ )
			{
			// �L���ȃC�x���g?
			if( ev[i].Active )
				{
				ev[i].Clock -= SaveCloclk;
				// �X�V�Ԋu�������ꍇ�͕����񔭐�����\������
				// �Ƃ肠�����S�Ă��Ȃ��܂ŌJ��Ԃ����Ă��Ƃł����̂�?
				if( ev[i].Clock <= 0 ){
					// �C�x���g�R�[���o�b�N�����s
					if( ev[i].CallbackProc !=NULL)
						ev[i].CallbackProc(ev[i].id,0);
					
					if( ev[i].Period > 0 ){	// ���[�v�C�x���g
						ev[i].Clock += ev[i].Period;
						if( ev[i].Clock <= 0 ) 
							cnt++;	// ���̃C�x���g���������Ă�����J�E���g
					}else{					// �����^�C���C�x���g
						Event_Del( i );
						break;
					}
				}
				// ���̃C�x���g�܂ł̃N���b�N���X�V
				if( NextEvent < 0 ) 
					NextEvent = ev[i].Clock;
				else
					NextEvent = min( NextEvent, ev[i].Clock );
			}
		}
		SaveCloclk = 0;
	}while( cnt > 0 );
	return 0;
}


// ****************************************************************************
// �C�x���g�̐i�s�������߂�
//   �S������10�{���ĕԂ�(100%��1000)
//   0�ȉ�,100�ȏ�ɂȂ�ꍇ��0-100�Ɋۂ߂�
//
// ����:	dev		�C�x���g�f�o�C�X�I�u�W�F�N�g�|�C���^
//			id		�C�x���gID
// �Ԓl:	double	�C�x���g�i�s��(1.0��100%)
// ****************************************************************************
double Event_Scale( int id )
{
	// �C�x���g�����݂�1�����̃N���b�N�����ݒ肳��Ă���?
	if( ev[id].Active == TRUE && ev[id].Period > 0 )
		{
		// ���ߍ��񂾃N���b�N���l��
		double sc = (double)( (double)( ev[id].Period - ev[id].Clock + SaveCloclk ) / (double)ev[id].Period );
		return min( max( 0.0, sc ), 1.0 );
		}
	else
		return 0;
}

// ****************************************************************************
// �C�x���g���o�^����Ă��邩�H
// ����:	id		�C�x���gID
// �Ԓl:	1: �o�^����Ă���@�@0:���o�^
// ****************************************************************************

int Event_isActive(int id)
{
	return( ev[id].Active );
}

/*Event_Update(int clk)
{
	int i;
	for(i=0; i< MAX_EVENT ;i++)
		{
		if( ev[id].Enable)
			{
			ev[id].Clock -= clk;
			if( ev[id].Clock <0)
				{
				ev[id].CallbackProc();
				ev[id].Clock += ev.Period;
				}
			}
		}
}
*/



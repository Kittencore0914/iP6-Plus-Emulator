/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         schedule.h                      **/
/**                                                         **/
/** translate from PC6001V's C++ to C  by windy             **/
/**                                                         **/
/** Original program:   PC6001V                             **/
/** Original author:    Yumitaro.                           **/
/** Original URL:       http://www.papicom.net/p6v/         **/
/*************************************************************/

#ifdef WIN32
#define SCALL __cdecl
#else
#define SCALL
#endif

#define MAX_EVENT   5
#define EVENT_PSG   0
#define EVENT_VOICE 1
#define EVENT_KEYIN 2
#define EVENT_DISK  3

// �C�x���g�X�^�C���t���O
// bit0: �J��Ԃ��w��
#define	EV_ONETIME	(0x00)	/* �����^�C�� */
#define	EV_LOOP		(0x01)	/* ���[�v */
// bit1,2: �����w��P��
#define	EV_HZ		(0x00)	/* Hz */
#define	EV_US		(0x02)	/* us */
#define	EV_MS		(0x04)	/* ms */
#define	EV_STATE	(0x08)	/* CPU�X�e�[�g�� */

typedef struct {
		int Active;			// �L���E����
		int Period;			// 1�����̃N���b�N��
		int Clock;			// �c��N���b�N��
		double nps;			// �C�x���g�������g��(Hz)
		
		int ncount;			// �C�x���g������
		int ratio;			// �C�x���g������
		int (SCALL *CallbackProc)( void*);	// �R�[���o�b�N�֐�
	} evinfo;


void Event_init(void);
int Event_Add( int id, double hz ,int flag, int (SCALL *CallbackProc)( void*) );
void Event_Del(int id);
int Event_Update(int clk);
double Event_Scale( int id );
void Event_Reset( int id );
int Event_isActive(int id);


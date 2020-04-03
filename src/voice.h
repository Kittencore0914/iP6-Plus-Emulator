/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                         voice.h                         **/
/**                                                         **/
/** translate from PC6001V's C++ to C  by windy             **/
/**                                                         **/
/** Original program:   PC6001V                             **/
/** Original author:    Yumitaro.                           **/
/** Original URL:       http://www.papicom.net/p6v/         **/
/*************************************************************/
#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED

#include "Z80.h"

// [OUT]
//  E0H : �����p�����[�^�]��
//  E1H : ?
//  E2H : ���[�h�Z�b�g : 0 0 0 0 F S1 S0
//			F     : ���̓t���[������ 0: 10ms/�t���[��
//									 1: 20ms/�t���[��
//			S1,S0 : �������xBIT     00: NORMAL SPEED
//									01: SLOW SPEED
//									10: FAST SPEED
//									11: �֎~

//  E3H : �R�}���h�Z�b�g
//		�E������I���R�}���h : 0  0 P5 P4 P3 P2 P1 P0
//			P5-P0 : ��I���r�b�g(0-63)
//		�E�X�g�b�v�R�}���h   : 1  1  1  1  1  1  1  1
//		�E�O����I���R�}���h : 1  1  1  1  1  1  1  0
//

// [IN]
//  E0H : �X�e�[�^�X���W�X�^ : BSY REQ ~INT/EXT ERR 0 0 0 0
//			BSY      : �������� 1:������ 0:��~��
//			REQ      : �����p�����[�^ 1:���͗v�� 0:�֎~
//			~INT/EXT : ���b�Z�[�W�f�[�^ 1:�O�� 0:����
//			ERR      : �G���[�t���O 1:�G���[ 0:�G���[�Ȃ�
//  E1H : ?
//  E2H : PortE2�ɏ����񂾒l?
//  E3H : PortE3�ɏ����񂾒l?


// �����^�C�~���O�Ɋւ��鋓��
// [�{���̃X�y�b�N]
// �E�R�}���h���s����1�t���[���ڂ̃p�����[�^���͊����܂�2ms�ȓ�(NORMAL SPEED)
//   �y�^��z�R�}���h���s����REQ�܂ł̎��Ԃ�?
// �E�t���[�����ōŏ���REQ����S�p�����[�^���͂܂�3/4�t���[�����Ԉȓ�
//   �y�l�@�z���͊������甭���܂ł�1/4�t���[�����Ԉȓ��Ŋm���ɏ��������?
//   �y�^��z����REQ�܂ł̎��Ԃ�?
//
// [�����X�y�b�N]
// �E�R�}���h���s����1�t���[���ڂ̃p�����[�^���͊����܂�1�t���[�����Ԉȓ�
// �E1�t���[�����Ԍo�ߎ��_�Ŕ���(1�t���[�����̃T���v������)�C������REQ=1
// �E�������玟�t���[���̃p�����[�^���͊����܂�1�t���[�����Ԉȓ�

// translated into C for Cocoa iP6 by Koichi Nishida 2004
// ������I���R�}���h�ɂ͖��Ή�

// Voice������1�ɂȂ�t���O
#ifdef __cplusplus
extern "C" {
#endif

	extern int VoiceFlag;

	// ������
	int InitVoice(void);
	// �㏈��
	void TrashVoice(void);
	// Voice �X���b�h���L�����Z������
	void CancelVoiceThread(void);

	// I/O�A�N�Z�X�֐�
	void OutE0H(byte data);
	void OutE2H(byte data);
	void OutE3H(byte data);
	byte InE0H(void);
	byte InE2H(void);
	byte InE3H(void);

#ifdef __cplusplus
}
#endif

#endif	// VOICE_H_INCLUDED

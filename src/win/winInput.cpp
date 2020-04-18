/** iP6: PC-6000/6600 series emualtor ************************/
/**          Windows Direct Input                           **/
/**          name is winInput.cpp                           **/
/**                                                         **/
/**          by Windy                                       **/
/*************************************************************/


/*************************************************************/
/*		Direct Input Variable                                */
/*************************************************************/
#include <stdio.h>
#define DIRECTINPUT_VERSION     0x0800          // DirectInput�̃o�[�W�����w��
#include <dinput.h>
#include "WinInput.h"

// ���C�u���������N
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")


// �O���[�o���ϐ���`
LPDIRECTINPUT8			lpDI = NULL;			// IDirectInput8
LPDIRECTINPUTDEVICE8	lpKeyboard = NULL;		// �L�[�{�[�h�f�o�C�X



/************************************************************/
/*    init Direct Input                                     */
/************************************************************/
int init_directinput(HWND hWnd ,HINSTANCE hInstance)
{
	// IDirectInput8�̍쐬
	HRESULT ret = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&lpDI, NULL);
	if (FAILED(ret)) {
		// �쐬�Ɏ��s
		printf("DirectInput8�̍쐬�Ɏ��s\n");
		return -1;
	}

	// IDirectInputDevice8�̎擾
	ret = lpDI->CreateDevice(GUID_SysKeyboard, &lpKeyboard, NULL);
	if (FAILED(ret)) {
		printf("�L�[�{�[�h�f�o�C�X�̍쐬�Ɏ��s\n");
		lpDI->Release();
		return -1;
	}

	// ���̓f�[�^�`���̃Z�b�g
	ret = lpKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(ret)) {
		printf("���̓f�[�^�`���̃Z�b�g���s\n");
		lpKeyboard->Release();
		lpDI->Release();
		return -1;
	}

	// �r������̃Z�b�g
	ret = lpKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	if (FAILED(ret)) {
		printf("�r������̃Z�b�g���s\n");
		lpKeyboard->Release();
		lpDI->Release();
		return -1;
	}

	// ����J�n
	lpKeyboard->Acquire();
	return 0;
}


/************************************************************/
/*    get stick   (Direct Input)                            */
/*                                                          */
/************************************************************/
BYTE OSD_GetStickKeyboard(void)
{
	byte ret=0;

	// �L�[�̓���
	BYTE key[256];
	ZeroMemory(key, sizeof(key));
	ret = lpKeyboard->GetDeviceState(sizeof(key), key);
	if (FAILED(ret)) {
		// ���s�Ȃ�ĊJ�����Ă�����x�擾
		lpKeyboard->Acquire();
		lpKeyboard->GetDeviceState(sizeof(key), key);
	}

	if (key[DIK_SPACE] & 0x80) {
		ret |= 1 << 7;
	}
	if (key[DIK_LEFT]  & 0x80) {
		ret |= 1 << 5;
	}
	if (key[DIK_RIGHT] & 0x80) {
		ret |= 1 << 4;
	}
	if (key[DIK_DOWN] & 0x80) {
		ret |= 1 << 3;
	}
	if (key[DIK_UP]    & 0x80) {
		ret |= 1 << 2;
	}
	if (key[DIK_END] & 0x80) {
		ret |= 1 << 1;
	}
	if (key[DIK_LSHIFT] & 0x80 || key[DIK_RSHIFT] & 0x80) {
		ret |= 1;
	}
	return ret;
}

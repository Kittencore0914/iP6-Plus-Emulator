/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                           Win32stick.h                  **/
/** by Windy 2002-2004                                      **/
/*************************************************************/
#ifndef _WIN32STICK_H
#define _WIN32STICK_H

#include "../types.h"

int JoysticOpen(void);				//          JoysticOpen: JOYSTICK ��������
byte JoystickGetState(int joy_no);	//          JoysticGetState: JOYSTICK �̏�Ԃ��擾
int NumJoysticks(void);             //          NumJoysticks: JOYSTICK �̐����擾
int JoysticClose(void);				//          JoystickClose: JOYSTICK �̌�n��

#endif

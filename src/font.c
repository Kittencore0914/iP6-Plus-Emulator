//		font
// 
//

#include <stdio.h>
#include <string.h>

#include "font/shnmk14.h"




// bitmap �f�[�^�ւ̃|�C���^�[��Ԃ�
// NULL:���s
short *getFontData(int jiscode)
{
	int i=0;
	for (i = 0; i < length; i++) {
		if (fontdata[i].jiscode == jiscode) {
			return fontdata[i].bitmap;
		}
	}
 return(NULL);
}
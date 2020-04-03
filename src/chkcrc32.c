/** iP6: PC-6000/6600 series emualtor ************************/
/**                                                         **/
/**                        chkcrc32.c                       **/
/**                                                         **/
/*************************************************************/
// �Q�l: http://www.tnksoft.com/reading/zipfile/nonarc.php

static unsigned int table[256];

// �v�Z�̌��ƂȂ�e�[�u�������炩���ߍ쐬
void InitCRC32(void)
{
	unsigned int poly = 0xEDB88320, u, i, j;

	for(i = 0; i < 256; i++){
		u = i;

		for(j = 0; j < 8; j++){
			if(u & 0x1){
				u = (u >> 1) ^ poly;
			}else{
				u >>= 1;
			}
		}

		table[i] = u;
	}
}

// crc32_start�ł́A�O�̃o�b�t�@�u���b�N��CRC32�l�������Ɏw�肷��B
// �o�b�t�@���P��ł���̂Ȃ�A�X�Ɉ����͎w�肵�Ȃ��Ă悢(0xFFFFFFFF�������l)�B
unsigned int GetCRC32(unsigned char *buffer, unsigned int bufferlen, unsigned int crc32_start)
{
	unsigned int i;
	unsigned int result = crc32_start;
	for( i = 0; i < bufferlen; i++){
		result = (result >> 8) ^ table[buffer[i] ^ (result & 0xFF)];
	}
	return ~result;
}


#ifndef	___APP_H___
#define	___APP_H___

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "pd_api.h"


typedef int		bool;
#ifndef	true
#define	true	1
#endif
#ifndef	false
#define	false	0
#endif

#ifndef	M_PI
#define	M_PI	3.14159265358979f
#endif


extern const PlaydateAPI*	pd;
extern const struct playdate_graphics*		gfx;

extern int	common_counter;			// ���ʃJ�E���^


/****************
    �{�^������
 ****************/
typedef struct
{
	PDButtons	push;
	PDButtons	trigger;
	PDButtons	repeat;
	PDButtons	release;
} Button;

extern Button	button;				// �{�^������


void	init_input(void);					// �{�^�����͏�����
void	update_input(void);					// �{�^�����͉ғ�

LCDBitmap*	load_bitmap(const char*);							// �r�b�g�}�b�v�ǂݍ���
LCDBitmap*	cut_bitmap(LCDBitmap*, int, int, int, int);			// �r�b�g�}�b�v�؂蔲��

extern int	fade_cnt;				// �t�F�[�h�p�J�E���^

void	fade_in(void);						// �t�F�[�h�C��
void	fade_out(void);						// �t�F�[�h�A�E�g
void	init_fade(void);					// �t�F�[�h����������
void	update_fade(void);					// �t�F�[�h�����ғ�
void	draw_fade(void);					// �t�F�[�h�`��

#endif

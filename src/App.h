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

extern int	common_counter;			// 共通カウンタ


/****************
    ボタン入力
 ****************/
typedef struct
{
	PDButtons	push;
	PDButtons	trigger;
	PDButtons	repeat;
	PDButtons	release;
} Button;

extern Button	button;				// ボタン入力


void	init_input(void);					// ボタン入力初期化
void	update_input(void);					// ボタン入力稼働

LCDBitmap*	load_bitmap(const char*);							// ビットマップ読み込み
LCDBitmap*	cut_bitmap(LCDBitmap*, int, int, int, int);			// ビットマップ切り抜き

extern int	fade_cnt;				// フェード用カウンタ

void	fade_in(void);						// フェードイン
void	fade_out(void);						// フェードアウト
void	init_fade(void);					// フェード処理初期化
void	update_fade(void);					// フェード処理稼働
void	draw_fade(void);					// フェード描画

#endif

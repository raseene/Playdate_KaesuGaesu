#ifndef	___PANEL_H___
#define	___PANEL_H___

#include "App.h"


#define	PANEL_W		56					// パネルの大きさ
#define	PANEL_H		PANEL_W

/************
    パネル
 ************/
typedef struct
{
	LCDBitmap*	bmp[2];			// ビットマップ
	int			x, y;			// 位置
	int			side;			// 表裏
	int			cnt;			// 返しカウンタ
}  Panel;


void	init_panel(Panel*, int, int, LCDBitmap*, LCDBitmap*);		// 初期化
void	quit_panel(Panel*);				// 終了
bool	set(Panel*, bool);				// 表裏設定
bool	update_panel(Panel*);			// 稼働
void	reverse_h(Panel*);				// 反転
void	reverse_v(Panel*);
void	draw_panel(Panel*);				// 描画

#endif


#include "Panel.h"


/*******************************************
    初期化
		引数	_x, _y = 位置
				_back  = 絵ビットマップ
				_base  = 下地ビットマップ
 *******************************************/
void	init_panel(Panel* this, int _x, int _y, LCDBitmap* _back, LCDBitmap* _base)
{
	this->x = _x;
	this->y = _y;

	LCDBitmap*	p = gfx->newBitmap(PANEL_W, PANEL_H, kColorBlack);

	gfx->pushContext(p);
	gfx->drawBitmap(_back, -_x, -_y, kBitmapUnflipped);
	gfx->drawRect(0, 0, PANEL_W - 1, PANEL_H - 1, kColorBlack);
	gfx->drawRect(0, 0, PANEL_W, PANEL_H, kColorWhite);
	gfx->popContext();

	this->bmp[0] = p;					// 絵
	this->bmp[1] = _base;				// 下地
}

/**********
    終了
 **********/
void	quit_panel(Panel* this)
{
	if ( this->bmp[0] ) {
		gfx->freeBitmap(this->bmp[0]);
		this->bmp[0] = NULL;
	}
}

/***********************************
    表裏設定
		引数	_side = TRUE ：表
				      = FALSE：裏
		戻り値	表裏
 ***********************************/
bool	set(Panel* this, bool _side)
{
	this->side = _side ? 0 : 2;
	this->cnt  = 0;
	return	_side;
}


/**************************
    稼働
		戻り値	稼働中か
 **************************/
bool	update_panel(Panel* this)
{
	if ( this->cnt > 0 ) {
		this->cnt--;
		return	true;
	}
	return	false;
}

/**********
    反転
 **********/
void	reverse_h(Panel* this)
{
	this->side = (this->side < 2) ? 2 : 0;
	this->cnt  = 12;
}

void	reverse_v(Panel* this)
{
	this->side = (this->side < 2) ? 3 : 1;
	this->cnt  = 12;
}


/**********
    描画
 **********/
void	draw_panel(Panel* this)
{
	float	_sx = 1.0f, _sy = 1.0f;

	if ( (this->cnt > 0) || (this->side >= 2) ) {				// 下地
		gfx->drawBitmap(this->bmp[1], this->x, this->y, kBitmapUnflipped);
	}
	switch ( this->side ) {
	  case 0 :					// 表
		if ( this->cnt > 0 ) {
			_sx = cosf(this->cnt*M_PI/24);
		}
		break;

	  case 1 :					// 表
		if ( this->cnt > 0 ) {
			_sy = cosf(this->cnt*M_PI/24);
		}
		break;

	  case 2 :					// 裏
		if ( this->cnt > 0 ) {
			_sx = sinf(this->cnt*M_PI/24);
		}
		else {
			return;
		}
		break;

	  case 3 :					// 裏
		if ( this->cnt > 0 ) {
			_sy = sinf(this->cnt*M_PI/48);
		}
		else {
			return;
		}
		break;
	}

	if ( (_sx == 1.0f) && (_sy == 1.0f) ) {						// 絵
		gfx->drawBitmap(this->bmp[0], this->x, this->y, kBitmapUnflipped);
	}
	else {														// 回転中
		gfx->drawScaledBitmap(this->bmp[0], this->x + (int)((1.0f - _sx)*PANEL_W/2), this->y + (int)((1.0f - _sy)*PANEL_H/2), _sx, _sy);
	}
}

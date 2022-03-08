
#include "App.h"


Button	button;					// ボタン入力
static int	repeat_cnt;			// リピート用カウンタ

/**********************
    ボタン入力初期化
 **********************/
void	init_input(void)
{
	button.push		= 0;
	button.trigger	= 0;
	button.repeat	= 0;
	button.release	= 0;
	repeat_cnt		= 0;
}

/********************
    ボタン入力稼働
 ********************/
void	update_input(void)
{
	PDButtons	_button;

	pd->system->getButtonState(&_button, NULL, NULL);			// ボタン状態取得

	button.trigger	= _button & ~button.push;
	button.release	= ~_button & button.push;
	button.repeat	= button.trigger;
	if ( _button ) {
		if ( button.push == 0 ) {
			repeat_cnt = 15;
		}
		else if ( --repeat_cnt == 0 ) {
			button.repeat = _button;
			repeat_cnt = 4;
		}
	}
	button.push = _button;
}


/**************************************
    ビットマップ読み込み
			引数	_path = ファイル
			戻り値	ビットマップ
 **************************************/
LCDBitmap*	load_bitmap(const char* _path)
{
	const char*	_err = NULL;
	LCDBitmap*	_bmp = gfx->loadBitmap(_path, &_err);
	if ( _err ) {
		pd->system->logToConsole("Error loading image at path '%s': %s", _path, _err);
	}
	return	_bmp;
}

/*********************************************
    ビットマップ切り抜き
			引数	_base  = 元ビットマップ
					_u, _v = 位置
					_w, _h = 大きさ
			戻り値	ビットマップ
 *********************************************/
LCDBitmap*	cut_bitmap(LCDBitmap* _base, int _u, int _v, int _w, int _h)
{
	LCDBitmap*	_bmp = gfx->newBitmap(_w, _h, gfx->getBitmapMask(_base) ? kColorClear : kColorBlack);
	gfx->pushContext(_bmp);
	gfx->drawBitmap(_base, -_u, -_v, kBitmapUnflipped);
	gfx->popContext();
	return	_bmp;
}


int		fade_cnt;			// フェード用カウンタ

/******************
    フェードイン
 ******************/
void	fade_in(void)
{
	fade_cnt = -7;
}

/********************
    フェードアウト
 ********************/
void	fade_out(void)
{
	fade_cnt = 1;
}

/************************
    フェード処理初期化
 ************************/
void	init_fade(void)
{
	fade_cnt = 0;
}

/**********************
    フェード処理稼働
 **********************/
void	update_fade(void)
{
	if ( fade_cnt != 0 ) {
		fade_cnt++;
	}
}

/******************
    フェード描画
 ******************/
void	draw_fade(void)
{
	if ( fade_cnt > 0 ) {					// フェードアウト
		int		w = (fade_cnt < 8) ? fade_cnt : 8;
		for (int y = 0; y < 240; y += 8) {
			for (int x = 0; x < 400; x += 8) {
				gfx->fillRect(x, y, w, w, kColorWhite);
			}
		}
	}
	else if ( fade_cnt < 0 ) {				// フェードイン
		for (int y = 0; y < 240; y += 8) {
			for (int x = 0; x < 400; x += 8) {
				gfx->fillRect(x + 8 + fade_cnt, y + 8 + fade_cnt, -fade_cnt, -fade_cnt, kColorWhite);
			}
		}
	}
}

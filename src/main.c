
#include "App.h"
#include "Game/Game.h"


const PlaydateAPI*	pd;
const struct playdate_graphics*		gfx;

int		common_counter;			// 共通カウンタ


/************
    初期化
 ************/
static
void	init(void)
{
	pd->display->setRefreshRate(30.0f);						// フレームレート設定

	srand(pd->system->getSecondsSinceEpoch(NULL));			// 乱数初期化
	common_counter = 0;				// 共通カウンタ
	init_fade();					// フェード処理初期化
	init_input();					// ボタン入力初期化

	init_game();					// ゲーム初期化
}

/**********************
    フレーム毎の処理
 **********************/
static
int		update(void* _pd)
{
	pd  = _pd;
	gfx = ((PlaydateAPI*)_pd)->graphics;

	common_counter++;				// 共通カウンタ
	rand();
	update_fade();					// フェード処理稼働
	update_input();					// ボタン入力稼働

	update_game();					// ゲーム稼働
	draw_game();					// ゲーム描画

	draw_fade();					// フェード描画
	return	1;
}


#ifdef _WINDLL
__declspec(dllexport)
#endif
int		eventHandler(PlaydateAPI* _pd, PDSystemEvent _event, uint32_t _arg)
{
	(void)_arg;

	pd  = _pd;
	gfx = _pd->graphics;
	if ( _event == kEventInit ) {
		init();						// 初期化
		_pd->system->setUpdateCallback(update, _pd);		// フレーム処理登録
	}
	return	0;
}

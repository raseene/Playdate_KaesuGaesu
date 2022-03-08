
#include "Game.h"
#include "Panel.h"


#define	BACK_MAX	25				// 背景画像数

#define	FIELD_W		4				// パネルの数
#define	FIELD_H		FIELD_W


/*** 状態 *******/
enum
{
	PHASE_TITLE,						// タイトル
	PHASE_LEVEL,						// レベル選択
	PHASE_START	= PHASE_LEVEL + 3,		// ゲーム開始
	PHASE_GAME,							// ゲーム中
	PHASE_CLEAR,						// クリア
};


/*** BGM番号 *******/
enum
{
	BGM_MENU	= 0,			// メニュー
	BGM_GAME,					// ゲーム
};

/*** SE番号 *******/
enum
{
	SE_CLICK,
	SE_FORWARD,
	SE_BACK,
	SE_STOP,
	SE_CLEAR,
	SE_MAX,
};


/****************
    ライン情報
 ****************/
typedef struct
{
	int		state;				// 状態
	int		correct;			// 正解ルート
} Line;


static Panel		panel[FIELD_H][FIELD_W];			// パネル
static Line			line_h[FIELD_H + 1][FIELD_W];		// 横ライン
static Line			line_v[FIELD_H][FIELD_W + 1];		// 縦ライン
static int			field_w, field_h;					// フィールドの大きさ
static int			field_x, field_y;					// フィールドの位置

static int			cursor_x, cursor_y;					// カーソル位置
static int			cursor_dx, cursor_dy;				// 移動方向
static int			move_cnt;							// 移動カウンタ
static Line*		current_line;						// 移動中のライン
static PDButtons	undo[FIELD_W*FIELD_H*2];			// やり直しバッファ
static int			undo_cnt;							// やり直しカウンタ
static bool			flag_answer;						// 解答表示フラグ

static int			phase;								// 状態
static int			cnt;								// 汎用カウンタ
static int			level;								// 選択レベル
static bool			free_mode;							// フリーモードか

static int				back_num;						// 背景番号
static LCDBitmap*		bmp_back;						// 背景
static LCDBitmap*		bmp_base;						// パネル下地
static LCDBitmap*		bmp_cursor[4];					// カーソル
static LCDBitmap*		bmp_clear[176/8];				// クリア
static LCDBitmap*		bmp_logo[4];					// タイトルロゴ
static LCDBitmap*		bmp_level[4];					// レベル選択
static LCDBitmap*		bmp_select;						// 選択中
static LCDBitmap*		bmp_board;						// 選択背景

static LCDBitmap*		bmp_game;						// ゲーム画面バッファ
static bool				flag_draw;						// 描画フラグ

static AudioSample*		se_data[SE_MAX];				// SE
static SamplePlayer*	se_player[4];					// SEプレイヤー
static FilePlayer*		bgm_player;						// BGMプレイヤー



static void		load_back(void);		// 背景読み込み
static void		play_bgm(int);			// BGM再生

/************
    初期化
 ************/
void	init_game(void)
{
	back_num = -1;
	load_back();											// 背景
	bmp_base = load_bitmap("images/base");					// パネル下地

	LCDBitmap*	_tmp = load_bitmap("images/cursor");		// カーソル
	for (int i = 0; i < 4; i++) {
		bmp_cursor[i] = cut_bitmap(_tmp, i*32, 0, 32, 32);
	}
	gfx->freeBitmap(_tmp);

	_tmp = load_bitmap("images/clear");						// クリア
	for (int i = 0; i < 176/8; i++) {
		bmp_clear[i] = cut_bitmap(_tmp, i*8, 0, 8, 32);
	}
	gfx->freeBitmap(_tmp);

	_tmp = load_bitmap("images/logo");						// タイトルロゴ
	for (int i = 0; i < 4; i++) {
		bmp_logo[i] = cut_bitmap(_tmp, i*56, 0, 56, 50);
	}
	gfx->freeBitmap(_tmp);

	_tmp = load_bitmap("images/level");						// レベル
	for (int i = 0; i < 4; i++) {
		bmp_level[i] = cut_bitmap(_tmp, 0, 24*i, 80, 24);
	}
	gfx->freeBitmap(_tmp);

	bmp_select = load_bitmap("images/select");				// 選択中
	bmp_board = load_bitmap("images/board");				// 選択背景

	bmp_game = gfx->copyBitmap(bmp_back);					// ゲーム画面バッファ


	static const
	char*	se_file[] =
	{
		"sounds/se_click_adpcm",
		"sounds/se_forward_adpcm",
		"sounds/se_back_adpcm",
		"sounds/se_stop_adpcm",
		"sounds/se_clear_adpcm",
	};

	bgm_player = pd->sound->fileplayer->newPlayer();		// BGMプレイヤー
	for (int i = 0; i < 4; i++) {							// SEプレイヤー
		se_player[i] = pd->sound->sampleplayer->newPlayer();
	}
	for (int i = 0; i < SE_MAX; i++) {						// SEデータ
		se_data[i] = pd->sound->sample->load(se_file[i]);
	}


	memset(&panel[0][0], 0, sizeof(panel));					// パネル

	phase	= PHASE_TITLE;
	cnt		= 150;
	level	= 0;

	play_bgm(BGM_MENU);
}

/**********
    終了
 **********/
void	quit_game(void)
{
	gfx->freeBitmap(bmp_back);								// 背景
	gfx->freeBitmap(bmp_base);								// パネル下地
	for (int i = 0; i < 4; i++) {							// カーソル
		gfx->freeBitmap(bmp_cursor[i]);
	}
	for (int i = 0; i < 176/8; i++) {						// クリア
		gfx->freeBitmap(bmp_clear[i]);
	}
	for (int i = 0; i < 4; i++) {							// タイトルロゴ
		gfx->freeBitmap(bmp_logo[i]);
	}
	for (int i = 0; i < 4; i++) {							// レベル
		gfx->freeBitmap(bmp_level[i]);
	}
	gfx->freeBitmap(bmp_select);							// 選択中
	gfx->freeBitmap(bmp_board);								// 選択背景
	gfx->freeBitmap(bmp_game);								// ゲーム画面バッファ

	pd->sound->fileplayer->stop(bgm_player);
	pd->sound->fileplayer->freePlayer(bgm_player);			// BGMプレイヤー
	for (int i = 0; i < 4; i++) {							// SEプレイヤー
		pd->sound->sampleplayer->stop(se_player[i]);
		pd->sound->sampleplayer->freePlayer(se_player[i]);
	}
	for (int i = 0; i < SE_MAX; i++) {						// SEデータ
		pd->sound->sample->freeSample(se_data[i]);
	}

	for (int i = 0; i < FIELD_H; i++) {						// パネル
		for (int j = 0; j < FIELD_W; j++) {
			quit_panel(&panel[i][j]);
		}
	}
}


/******************
    背景読み込み
 ******************/
static
void	load_back(void)
{
	if ( back_num >= 0 ) {
		gfx->freeBitmap(bmp_back);
	}

	int		_t;
	do {
		_t = rand() % BACK_MAX;
	} while ( _t == back_num );
	back_num = _t;

	char*	_file;

	pd->system->formatString(&_file, "images/back%02d", back_num);
	bmp_back = load_bitmap(_file);
	pd->system->realloc(_file, 0);
}

/********************************
    BGM再生
		引数	_bgm = BGM番号
 ********************************/
static
void	play_bgm(int _bgm)
{
	static const
	char*	bgm_file[] =
	{
		"sounds/bgm_menu_adpcm",
		"sounds/bgm_game_adpcm",
	};

	pd->sound->fileplayer->stop(bgm_player);
	pd->sound->fileplayer->loadIntoPlayer(bgm_player, bgm_file[_bgm]);
	pd->sound->fileplayer->setVolume(bgm_player, 1.0f, 1.0f);
	pd->sound->fileplayer->play(bgm_player, 0);
}

/*******************************
    SE再生
		引数	_se = SEM番号
 *******************************/
static
void	play_se(int _se)
{
	static int	track = 0;

	pd->sound->sampleplayer->setSample(se_player[track], se_data[_se]);
	pd->sound->sampleplayer->play(se_player[track], 1, 1.0f);
	track = ++track % 4;
}


static bool		check_clear(void);		// クリアチェック

/*********************************
    問題作成
		引数	_level = 難易度
 *********************************/
static
void	init_field(int _level)
{
	int		i, j, _sx, _sy, _ex, _ey, _len, _x, _y;
	bool	_clear = true;

	do {
		memset(&line_h[0][0], 0, sizeof(line_h));		// ライン情報クリア
		memset(&line_v[0][0], 0, sizeof(line_v));

		_sx = rand() % (field_w + 1);					// 出発点
		_sy = rand() % (field_h + 1);
		_len = (_level == 0) ? 2 : ((_level == 1) ? (field_w + field_h)*4/5 : (field_w + field_h)*3/2);			// 最低限の長さ
		if ( _level == 0 ) {
			i = 1;
			j = 4;
		}
		else {
			i = 2;
			j = FIELD_W + FIELD_H;
		}
		do {
			_ex = rand() % (field_w + 1);				// 到着点
			_ey = rand() % (field_h + 1);
			_x = abs(_sx - _ex) + abs(_sy - _ey);
		} while ( (_x <= i) || (_x >= j) );

		if ( _sx < _ex ) {								// 最短経路設定
			i = _sx;
			j = _ex;
		}
		else {
			i = _ex;
			j = _sx;
		}
		for (; i < j; i++) {
			line_h[_sy][i].correct = 1;
			_len--;
		}
		if ( _sy < _ey ) {
			i = _sy;
			j = _ey;
		}
		else {
			i = _ey;
			j = _sy;
		}
		for (; i < j; i++) {
			line_v[i][_ex].correct = 1;
			_len--;
		}

		int		_x1 = -1, _y1 = -1;

		for (i = (_level == 0) ? 1 : ((_level == 1) ? 5 : 30); (i > 0) || (_len > 0); i--) {			// 経路シャッフル
			int		_t;
			do {
				_x = rand() % field_w;
				_y = rand() % field_h;
				if ( (line_h[_y][_x].correct == line_h[_y + 1][_x].correct) && (line_v[_y][_x].correct == line_v[_y][_x + 1].correct) ) {
					_t = 3;
				}
				else {
					_t = 0;
					if ( _x > 0 ) {
						_t += line_h[_y][_x - 1].correct + line_h[_y + 1][_x - 1].correct;
					}
					if ( _x < field_w - 1 ) {
						_t += line_h[_y][_x + 1].correct + line_h[_y + 1][_x + 1].correct;
					}
					if ( _y > 0 ) {
						_t += line_v[_y - 1][_x].correct + line_v[_y - 1][_x + 1].correct;
					}
					if ( _y < field_h - 1 ) {
						_t += line_v[_y + 1][_x].correct + line_v[_y + 1][_x + 1].correct;
					}
					if ( ((_x == _sx) || (_x + 1 == _sx)) && ((_y == _sy) || (_y + 1 == _sy)) && ((_x == _ex) || (_x + 1 == _ex)) || ((_y == _ey) && (_y + 1 == _ey)) ) {
						_t++;
					}
				}
			} while ( _t > 2 );
			if ( (_x == _x1) && (_y == _y1) ) {
				i += 2;
			}
			_x1 = _x;
			_y1 = _y;

			_len += line_h[_y][_x].correct + line_h[_y + 1][_x].correct + line_v[_y][_x].correct + line_v[_y][_x + 1].correct;
			line_h[_y    ][_x    ].correct ^= 1;
			line_h[_y + 1][_x    ].correct ^= 1;
			line_v[_y    ][_x    ].correct ^= 1;
			line_v[_y    ][_x + 1].correct ^= 1;
			_len -= line_h[_y][_x].correct + line_h[_y + 1][_x].correct + line_v[_y][_x].correct + line_v[_y][_x + 1].correct;
		}

		if ( _ex + _ey == 1 ) {							// 左上
			line_h[0][0].correct = 0;
			line_v[0][0].correct = 0;
			if ( _len > -2 ) {
				continue;
			}
		}
		if ( field_w - _ex + _ey == 1 ) {				// 右上
			line_h[0][field_w - 1].correct = 0;
			line_v[0][field_w].correct = 0;
			if ( _len > -2 ) {
				continue;
			}
		}
		if ( _ex + field_h - _ey == 1 ) {				// 左下
			line_h[field_h][0].correct = 0;
			line_v[field_h - 1][0].correct = 0;
			if ( _len > -2 ) {
				continue;
			}
		}
		if ( field_w - _ex + field_h - _ey == 1 ) {		// 右下
			line_h[field_h][field_w - 1].correct = 0;
			line_v[field_h - 1][field_w].correct = 0;
			if ( _len > -2 ) {
				continue;
			}
		}

		for (i = 0; i < field_h; i++) {					// パネル初期化
			for (j = 0; j < field_w; j++) {
				if ( !set(&panel[i][j], (bool)!(line_h[i][j].correct ^ line_h[i + 1][j].correct ^ line_v[i][j].correct ^ line_v[i][j + 1].correct)) ) {
					_clear = false;
				}
			}
		}
	} while ( _clear );

	cursor_x = _sx;										// カーソル位置
	cursor_y = _sy;
}

static
void	init_field_free(void)
{
	int		_x, _y;

	_x = rand() % (field_w + 1);						// 初期位置
	_y = rand() % (field_w + 1);
	do {
		int		_m = -1, _n;

		for (int i = 0; i < field_h; i++) {				// パネル初期化
			for (int j = 0; j < field_w; j++) {
				panel[i][j].side = 0;
			}
		}
		for (int i = 0; i < 50; i++) {
			do {
				_n = rand() % 4;
			} while ( _n == _m );
			switch ( _n ) {
			  case 0 :					// →
				if ( _x < field_w ) {
					_x++;
					if ( _y > 0 ) {						// パネル反転
						panel[_y - 1][_x - 1].side ^= 0x02;
					}
					if ( _y < field_h ) {
						panel[_y][_x - 1].side ^= 0x02;
					}
					_m = 1;
				}
				else {
					i++;
				}
				break;

			  case 1 :					// ←
				if ( _x > 0 ) {
					_x--;
					if ( _y > 0 ) {						// パネル反転
						panel[_y - 1][_x].side ^= 0x02;
					}
					if ( _y < field_h ) {
						panel[_y][_x].side ^= 0x02;
					}
					_m = 0;
				}
				else {
					i++;
				}
				break;

			  case 2 :					// ↓
				if ( _y < field_h ) {
					_y++;
					if ( _x > 0 ) {						// パネル反転
						panel[_y - 1][_x - 1].side ^= 0x02;
					}
					if ( _x < field_w ) {
						panel[_y - 1][_x].side ^= 0x02;
					}
					_m = 3;
				}
				else {
					i++;
				}
				break;

			  case 3 :					// ↑
				if ( _y > 0 ) {
					_y--;
					if ( _x > 0 ) {						// パネル反転
						panel[_y][_x - 1].side ^= 0x02;
					}
					if ( _x < field_w ) {
						panel[_y][_x].side ^= 0x02;
					}
					_m = 2;
				}
				else {
					i++;
				}
				break;
			}
		}
	} while ( check_clear() );

	cursor_x = _x;										// カーソル位置
	cursor_y = _y;
}

static void		set_menu(void);			// メニュー設定

/****************
    ゲーム開始
 ****************/
static
void	start_game(void)
{
	load_back();										// 背景切り替え

	if ( level == 0 ) {
		field_w = field_h = 3;							// フィールドの大きさ
		field_x = 112;									// フィールドの位置
		field_y = 38;
	}
	else {
		field_w = field_h = 4;							// フィールドの大きさ
		field_x = 88;									// フィールドの位置
		field_y = 10;
	}
	for (int i = 0; i < field_h; i++) {					// パネル初期化
		for (int j = 0; j < field_w; j++) {
			init_panel(&panel[i][j], field_x + PANEL_W*j, field_y + PANEL_H*i, bmp_back, bmp_base);
		}
	}

	if ( level < 3 ) {
		free_mode = false;
		init_field(level);								// 問題作成
	}
	else {
		free_mode = true;
		init_field_free();								// 問題作成
	}
	move_cnt		= 0;								// 移動カウンタ
	current_line	= NULL;								// 移動中のライン
	undo_cnt		= 0;								// やり直しカウンタ
	flag_answer		= false;							// 解答表示フラグ
	flag_draw		= true;								// 描画フラグ

	set_menu();											// メニュー設定
}


static PDMenuItem*	item_answer;

/***********************
    解答例表示/非表示
 ***********************/
static
void	show_answer(void* _data)
{
	flag_answer = (bool)pd->system->getMenuItemValue(item_answer);
	flag_draw = true;
}

/****************
    ゲーム中止
 ****************/
static
void	give_up(void* _data)
{
	phase = PHASE_LEVEL + 2;
	pd->system->removeAllMenuItems();					// メニュー削除
}

/******************
    メニュー設定
 ******************/
static
void	set_menu(void)
{
	if ( !free_mode ) {
		item_answer = pd->system->addCheckmarkMenuItem("answer", 0, show_answer, NULL);		// 解答例表示
	}
	pd->system->addMenuItem("give up", give_up, NULL);										// ゲーム中止
}

/**********************************
    クリアチェック
			戻り値	クリア状態か
 **********************************/
static
bool	check_clear(void)
{
	int		i, j;

	for (i = 0; i < field_h; i++) {
		for (j = 0; j < field_w; j++) {
			if ( panel[i][j].side >= 2 ) {
				return	false;
			}
		}
	}
	return	true;
}


static Line*	move_cursor(void);		// カーソル移動

/**********
    稼働
 **********/
void	update_game(void)
{
	Line*	_line = NULL;

	switch ( phase ) {
	  case PHASE_START :				// ゲーム開始
		phase = PHASE_GAME;
		start_game();
		fade_in();
		play_bgm(BGM_GAME);
	  case PHASE_GAME :					// ゲーム中
		_line = move_cursor();							// カーソル移動
		break;
	}
	if ( move_cnt != 0 ) {
		flag_draw = true;
		if ( move_cnt > 0 ) {							// 移動中
			move_cnt--;
		}
		else if ( move_cnt < 0 ) {
			move_cnt++;
		}
	}

	if ( current_line && (_line || (move_cnt == 0)) ) {
		current_line->state &= 0x01;
		current_line = NULL;
	}
	if ( _line ) {
		current_line = _line;
	}

	for (int i = 0; i < field_h; i++) {					// パネル
		for (int j = 0; j < field_w; j++) {
			if ( update_panel(&panel[i][j]) ) {
				flag_draw = true;
			}
		}
	}

	switch ( phase ) {
	  case PHASE_GAME :					// ゲーム中
		if ( check_clear() ) {
			phase = PHASE_CLEAR;
			cnt = 0;
			pd->system->removeAllMenuItems();			// メニュー削除
			pd->sound->fileplayer->fadeVolume(bgm_player, 0.0f, 0.0f, 44100, NULL);			// BGMフェードアウト
		}
		break;

	  case PHASE_CLEAR :				// クリア
		if ( ++cnt == 15 ) {
			play_se(SE_CLEAR);
		}
		if ( (cnt > 30) && (button.trigger & (kButtonA | kButtonB)) ) {
			phase = PHASE_LEVEL + 1;
			play_se(SE_CLICK);
			play_bgm(BGM_MENU);
		}
		break;

	  case PHASE_LEVEL + 2 :			// レベル選択（ゲーム中）
		if ( button.trigger & kButtonB ) {				// ゲームに戻る
			phase = PHASE_GAME;
			play_se(SE_BACK);
			set_menu();									// メニュー設定
			break;
		}
	  case PHASE_LEVEL + 0 :			// レベル選択（タイトル後）
	  case PHASE_LEVEL + 1 :			// レベル選択（クリア後）
		if ( (button.repeat & kButtonUp) && (level > 0) ) {
			level--;
			play_se(SE_FORWARD);
		}
		else if ( (button.repeat & kButtonDown) && (level < 4 - 1) ) {
			level++;
			play_se(SE_FORWARD);
		}
		if ( button.trigger & kButtonA ) {
			play_se(SE_CLICK);
			pd->sound->fileplayer->fadeVolume(bgm_player, 0.0f, 0.0f, 44100*7/30, NULL);		// BGMフェードアウト
			fade_out();									// 画面フェードアウト
		}
		if ( fade_cnt >= 8 ) {
			if ( phase > PHASE_LEVEL + 0 ) {
				for (int i = 0; i < field_h; i++) {		// パネル解放
					for (int j = 0; j < field_w; j++) {
						quit_panel(&panel[i][j]);
					}
				}
			}
			phase = PHASE_START;
		}
		break;

	  case PHASE_TITLE :				// タイトル
		cnt++;
		if ( button.trigger & (kButtonA | kButtonB) ) {
			phase = PHASE_LEVEL + 0;
			play_se(SE_CLICK);
		}
		break;
	}
}

static bool		check_point(int, int);	// 移動チェック

/********************************
    カーソル移動
		戻り値	移動中のライン
 ********************************/
static
Line*	move_cursor(void)
{
	Line*		_line = NULL;
	PDButtons	_btn = 0;

	if ( (button.repeat & kButtonRight) && (cursor_x < field_w) ) {					// →
		_btn = kButtonRight;
	}
	else if ( (button.repeat & kButtonLeft) && (cursor_x > 0) ) {					// ←
		_btn = kButtonLeft;
	}
	else if ( (button.repeat & kButtonDown) && (cursor_y < field_h) ) {				// ↓
		_btn = kButtonDown;
	}
	else if ( (button.repeat & kButtonUp) && (cursor_y > 0) ) {						// ↑
		_btn = kButtonUp;
	}
	else if ( (button.repeat & kButtonB) && (undo_cnt > 0) && !free_mode ) {		// やり直し
		switch ( undo[undo_cnt - 1] ) {
		  case kButtonRight :
			_btn = kButtonLeft;
			break;
		  case kButtonLeft :
			_btn = kButtonRight;
			break;
		  case kButtonDown :
			_btn = kButtonUp;
			break;
		  case kButtonUp :
			_btn = kButtonDown;
			break;
		}
	}

	switch ( _btn ) {
	  case kButtonRight :				// →
		_line = &line_h[cursor_y][cursor_x];
		cursor_dx = 1;
		cursor_dy = 0;
		if ( free_mode ) {
			play_se(SE_FORWARD);
		}
		else if ( check_point(cursor_x + 1, cursor_y) ) {
			_line->state = 0x11;
			undo[undo_cnt++] = kButtonRight;
			play_se(SE_FORWARD);
		}
		else if ( (undo_cnt > 0) && (undo[undo_cnt - 1] == kButtonLeft) ) {		// やり直し
			_line->state = 0x10;
			undo_cnt--;
			play_se(SE_BACK);
		}
		else if ( button.trigger & kButtonRight ) {
			move_cnt = -6;
			play_se(SE_STOP);
			return	_line;
		}
		else {
			return	NULL;
		}
		cursor_x++;
		move_cnt = 8;
		if ( cursor_y > 0 ) {							// パネル反転
			reverse_h(&panel[cursor_y - 1][cursor_x - 1]);
		}
		if ( cursor_y < field_h ) {
			reverse_h(&panel[cursor_y][cursor_x - 1]);
		}
		break;

	  case kButtonLeft :				// ←
		_line = &line_h[cursor_y][cursor_x - 1];
		cursor_dx = -1;
		cursor_dy = 0;
		if ( free_mode ) {
			play_se(SE_FORWARD);
		}
		else if ( check_point(cursor_x - 1, cursor_y) ) {
			_line->state = 0x21;
			undo[undo_cnt++] = kButtonLeft;
			play_se(SE_FORWARD);
		}
		else if ( (undo_cnt > 0) && (undo[undo_cnt - 1] == kButtonRight) ) {	// やり直し
			_line->state = 0x20;
			undo_cnt--;
			play_se(SE_BACK);
		}
		else if ( button.trigger & kButtonLeft ) {
			move_cnt = -6;
			play_se(SE_STOP);
			return	_line;
		}
		else {
			return	NULL;
		}
		cursor_x--;
		move_cnt = 8;
		if ( cursor_y > 0 ) {							// パネル反転
			reverse_h(&panel[cursor_y - 1][cursor_x]);
		}
		if ( cursor_y < field_h ) {
			reverse_h(&panel[cursor_y][cursor_x]);
		}
		break;

	  case kButtonDown :				// ↓
		_line = &line_v[cursor_y][cursor_x];
		cursor_dx = 0;
		cursor_dy = 1;
		if ( free_mode ) {
			play_se(SE_FORWARD);
		}
		else if ( check_point(cursor_x, cursor_y + 1) ) {
			_line->state = 0x11;
			undo[undo_cnt++] = kButtonDown;
			play_se(SE_FORWARD);
		}
		else if ( (undo_cnt > 0) && (undo[undo_cnt - 1] == kButtonUp) ) {		// やり直し
			_line->state = 0x10;
			undo_cnt--;
			play_se(SE_BACK);
		}
		else if ( button.trigger & kButtonDown ) {
			move_cnt = -6;
			play_se(SE_STOP);
			return	_line;
		}
		else {
			return	NULL;
		}
		cursor_y++;
		move_cnt = 8;
		if ( cursor_x > 0 ) {							// パネル反転
			reverse_v(&panel[cursor_y - 1][cursor_x - 1]);
		}
		if ( cursor_x < field_w ) {
			reverse_v(&panel[cursor_y - 1][cursor_x]);
		}
		break;

	  case kButtonUp :					// ↑
		_line = &line_v[cursor_y - 1][cursor_x];
		cursor_dx = 0;
		cursor_dy = -1;
		if ( free_mode ) {
			play_se(SE_FORWARD);
		}
		else if ( check_point(cursor_x, cursor_y - 1) ) {
			_line->state = 0x21;
			undo[undo_cnt++] = kButtonUp;
			play_se(SE_FORWARD);
		}
		else if ( (undo_cnt > 0) && (undo[undo_cnt - 1] == kButtonDown) ) {		// やり直し
			_line->state = 0x20;
			undo_cnt--;
			play_se(SE_BACK);
		}
		else if ( button.trigger & kButtonUp ) {
			move_cnt = -6;
			play_se(SE_STOP);
			return	_line;
		}
		else {
			return	NULL;
		}
		cursor_y--;
		move_cnt = 8;
		if ( cursor_x > 0 ) {							// パネル反転
			reverse_v(&panel[cursor_y][cursor_x - 1]);
		}
		if ( cursor_x < field_w ) {
			reverse_v(&panel[cursor_y][cursor_x]);
		}
		break;
	}
	return	_line;
}

/******************
	移動チェック
 ******************/
static
bool	check_point(int _x, int _y)
{
	return	(((_x == 0) || !(line_h[_y][_x - 1].state & 0x01)) && ((_y == 0) || !(line_v[_y - 1][_x].state & 0x01))
				&& ((_x == field_w) || !(line_h[_y][_x].state & 0x01)) && ((_y == field_h) || !(line_v[_y][_x].state & 0x01)));
}


static void		draw_panels(void);		// パネル描画
static void		draw_lines(void);		// ライン描画
static void		draw_answer(void);		// 解答描画
static void		draw_cursor(void);		// カーソル描画
static void		draw_clear(void);		// クリア描画
static void		draw_level(void);		// レベル選択画面描画
static void		draw_title(void);		// タイトル描画

/**********
    描画
 **********/
void	draw_game(void)
{
	if ( flag_draw ) {
		gfx->pushContext(bmp_game);								// ゲーム画面バッファ
		gfx->drawBitmap(bmp_back, 0, 0, kBitmapUnflipped);		// 背景
		draw_panels();											// パネル
		if ( flag_answer ) {
			draw_answer();										// 解答例
		}
		if ( !free_mode ) {
			draw_lines();										// ライン
		}
		gfx->popContext();
		flag_draw = false;
	}
	gfx->drawBitmap(bmp_game, 0, 0, kBitmapUnflipped);

	switch ( phase ) {
	  case PHASE_LEVEL + 0 :
	  case PHASE_LEVEL + 1 :
	  case PHASE_LEVEL + 2 :
		draw_level();									// レベル選択
		break;

	  case PHASE_TITLE :
		draw_title();									// タイトル
		break;

	  case PHASE_CLEAR :
		draw_clear();									// "CLEAR!"
		if ( cnt > 30 ) {
			break;
		}
	  case PHASE_GAME :
		draw_cursor();									// カーソル
		break;
	}

//	pd->system->drawFPS(0,0);
}

/****************
    パネル描画
 ****************/
static
void	draw_panels(void)
{
	int		i, j;

	for (i = 0; i < field_h; i++) {
		for (j = 0; j < field_w; j++) {
			draw_panel(&panel[i][j]);
		}
	}
}

/****************
    ライン描画
 ****************/
static
void	draw_lines(void)
{
	int		i, j, _t;

	_t = move_cnt*move_cnt*PANEL_W/(8*8);
	for (i = 0; i < FIELD_H + 1; i++) {					// 横ライン
		for (j = 0; j < FIELD_W; j++) {
			switch ( line_h[i][j].state ) {
			  case 0x01 :
				gfx->fillRect(field_x + j*PANEL_W - 3, field_y + i*PANEL_H - 3, PANEL_W + 6, 6, kColorWhite);
				break;

			  case 0x20 :
				_t = PANEL_W - _t;
			  case 0x11 :
				gfx->fillRect(field_x + j*PANEL_W - 3, field_y + i*PANEL_H - 3, PANEL_W + 6 - _t, 6, kColorWhite);
				break;

			  case 0x10 :
				_t = PANEL_W - _t;
			  case 0x21 :
				gfx->fillRect(field_x + j*PANEL_W - 3 + _t, field_y + i*PANEL_H - 3, PANEL_W + 6 - _t, 6, kColorWhite);
				break;
			}
		}
	}
	_t = move_cnt*move_cnt*PANEL_H/(8*8);
	for (i = 0; i < FIELD_H; i++) {						// 縦ライン
		for (j = 0; j < FIELD_W + 1; j++) {
			switch ( line_v[i][j].state ) {
			  case 0x01 :
				gfx->fillRect(field_x + j*PANEL_W - 3, field_y + i*PANEL_H - 3, 6, PANEL_H + 6, kColorWhite);
				break;

			  case 0x20 :
				_t = PANEL_H - _t;
			  case 0x11 :
				gfx->fillRect(field_x + j*PANEL_W - 3, field_y + i*PANEL_H - 3, 6, PANEL_H + 6 - _t, kColorWhite);
				break;

			  case 0x10 :
				_t = PANEL_H - _t;
			  case 0x21 :
				gfx->fillRect(field_x + j*PANEL_W - 3, field_y + i*PANEL_H - 3 + _t, 6, PANEL_H + 6 - _t, kColorWhite);
				break;
			}
		}
	}

	_t = move_cnt*move_cnt*PANEL_W/(8*8);
	for (i = 0; i < FIELD_H + 1; i++) {					// 横ライン
		for (j = 0; j < FIELD_W; j++) {
			switch ( line_h[i][j].state ) {
			  case 0x01 :
				gfx->fillRect(field_x + j*PANEL_W - 2, field_y + i*PANEL_H - 2, PANEL_W + 4, 4, kColorBlack);
				break;

			  case 0x20 :
				_t = PANEL_W - _t;
			  case 0x11 :
				gfx->fillRect(field_x + j*PANEL_W - 2, field_y + i*PANEL_H - 2, PANEL_W + 4 - _t, 4, kColorBlack);
				break;

			  case 0x10 :
				_t = PANEL_W - _t;
			  case 0x21 :
				gfx->fillRect(field_x + j*PANEL_W - 2 + _t, field_y + i*PANEL_H - 2, PANEL_W + 4 - _t, 4, kColorBlack);
				break;
			}
		}
	}
	_t = move_cnt*move_cnt*PANEL_H/(8*8);
	for (i = 0; i < FIELD_H; i++) {						// 縦ライン
		for (j = 0; j < FIELD_W + 1; j++) {
			switch ( line_v[i][j].state ) {
			  case 0x01 :
				gfx->fillRect(field_x + j*PANEL_W - 2, field_y + i*PANEL_H - 2, 4, PANEL_H + 4, kColorBlack);
				break;

			  case 0x20 :
				_t = PANEL_H - _t;
			  case 0x11 :
				gfx->fillRect(field_x + j*PANEL_W - 2, field_y + i*PANEL_H - 2, 4, PANEL_H + 4 - _t, kColorBlack);
				break;

			  case 0x10 :
				_t = PANEL_H - _t;
			  case 0x21 :
				gfx->fillRect(field_x + j*PANEL_W - 2, field_y + i*PANEL_H - 2 + _t, 4, PANEL_H + 4 - _t, kColorBlack);
				break;
			}
		}
	}
}

/**************
    解答描画
 **************/
static
void	draw_answer(void)
{
	LCDSolidColor	_color = kColorBlack;
	int		i, j;

	for (i = 0; i < FIELD_H + 1; i++) {					// 横ライン
		for (j = 0; j < FIELD_W; j++) {
			if ( line_h[i][j].correct && (line_h[i][j].state != 0x01) ) {
				gfx->fillRect(field_x + j*PANEL_W - 1, field_y + i*PANEL_H - 1, PANEL_W + 2, 2, _color);
			}
		}
	}
	for (i = 0; i < FIELD_H; i++) {						// 縦ライン
		for (j = 0; j < FIELD_W + 1; j++) {
			if ( line_v[i][j].correct && (line_v[i][j].state != 0x01) ) {
				gfx->fillRect(field_x + j*PANEL_W - 1, field_y + i*PANEL_H - 1, 2, PANEL_H + 2, _color);
			}
		}
	}
}

/******************
    カーソル描画
 ******************/
static
void	draw_cursor(void)
{
	int		_x = field_x + cursor_x*PANEL_W - 16,
			_y = field_y + cursor_y*PANEL_H - 16;

	if ( move_cnt > 0 ) {								// 移動中
		_x -= cursor_dx*move_cnt*move_cnt*PANEL_W/(8*8);
		_y -= cursor_dy*move_cnt*move_cnt*PANEL_H/(8*8);
	}
	else if ( move_cnt < 0 ) {							// 移動不可
		_x += cursor_dx*(3*3 - (3 + move_cnt)*(3 + move_cnt))*PANEL_W/(3*3*3);
		_y += cursor_dy*(3*3 - (3 + move_cnt)*(3 + move_cnt))*PANEL_H/(3*3*3);
	}
	gfx->drawBitmap(bmp_cursor[(common_counter % 8)/2], _x, _y, kBitmapUnflipped);
}

/****************
    クリア描画
 ****************/
static
void	draw_clear(void)
{
	int		_t;

	for (int i = 0, _x = 112; i < 176/8; i++, _x += 8) {
		_t = cnt - i;
		gfx->drawBitmap(bmp_clear[i], _x, (_t < 40) ? (100 - (_t - 40)*(_t - 40)/3) : (96 + (int)(cosf(((_t - 40) % 48)*(M_PI*2/48))*4.0f)), kBitmapUnflipped);
	}
}

/************************
    レベル選択画面描画
 ************************/
static
void	draw_level(void)
{
	static const
	int		item_y[] = {72, 108, 144, 184};

	gfx->drawBitmap(bmp_board, 120, 40, kBitmapUnflipped);							// 背景
	for (int i = 0; i < 4; i++) {
		if ( i == level ) {								// 選択中
			gfx->setDrawMode(kDrawModeWhiteTransparent);
			gfx->drawBitmap(bmp_select, 136, item_y[i] - 16, kBitmapUnflipped);		// 下地
			gfx->setDrawMode(kDrawModeInverted);
		}
		else {
			gfx->setDrawMode(kDrawModeWhiteTransparent);
		}
		gfx->drawBitmap(bmp_level[i], 160, item_y[i] - 12, kBitmapUnflipped);		// レベル
	}
	gfx->setDrawMode(kDrawModeCopy);
}

/******************
    タイトル描画
 ******************/
static
void	draw_title(void)
{
	static const
	int		param[][3] =
	{
		0,	 52, 84,			// か
		2,	146, 80,			// す
		1,	101, 90,			// え
		1,	246, 84,			// え
		3,	196, 88,			// が
		2,	292, 92,			// す
	};

	const int*	p = &param[0][0];
	for (int i = 0; i < 6; i++) {
		int		_t = (cnt + 200 - i*12) % 200;

		if ( _t >= 20 ) {
			gfx->drawBitmap(bmp_logo[p[0]], p[1], p[2], kBitmapUnflipped);
		}
		else {
			float	_scl = cosf(_t*M_PI/10);
			if ( i % 2 == 0 ) {
				gfx->drawScaledBitmap(bmp_logo[p[0]], p[1] + 28 - (int)(_scl*((_scl > 0.0f) ? 28 : -28)), p[2], _scl, 1.0f);
			}
			else {
				gfx->drawScaledBitmap(bmp_logo[p[0]], p[1], p[2] + 25 - (int)(_scl*((_scl > 0.0f) ? 25 : -25)), 1.0f, _scl);
			}
		}
		p += 3;
	}
}

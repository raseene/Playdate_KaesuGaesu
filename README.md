# ひっくり返しパズル　かえすがえす

Playdate(https://play.date/jp/)用の、お手軽パズルゲームです。
パネルをひっくり返して、画像を完成させましょう。

![Screenshot](http://raseene.asablo.jp/blog/img/2022/03/08/6252d1.png)

## ゲームの目的

パネルを全て表にひっくり返して、画像を完成させることです。

## 遊び方

十字ボタンでカーソルを動かします。
ただしキャンセル（Bボタン：来た方向に戻る）を除いて、一度通った場所には行けません。
カーソルが移動するとその両脇のパネルがひっくり返り、消えていたものは出現、出現していたものは消えます。
全てのパネルを出現させて、画像を完成させればクリアです。

### フリーモード

３つの難易度の他に、「一度通った場所には行けない」という制限を外した「フリー」を追加しました。
手詰まり状態の無い、通常より気軽に遊ぶことができるモードです。

### メニュー

ゲーム中に "MENU"ボタンを押すと、メニューを開きます。

[answer]
解答例の表示/非表示を切り替えます。
"解答例"なので、別解が存在する場合があります。

[give up]
現在の問題を終了して、難易度選択に戻ります。


## 実行（Windows）

Playdate SDK(https://play.date/jp/dev/)をインストール後、
PlaydateSimulatorで KaesuGaesu.pdxフォルダをオープンしてください。

Windows以外の環境では、プロジェクトをビルドして実行バイナリを作成する必要があります。

## ビルド

Inside Playdate with C(https://sdk.play.date/1.9.0/Inside%20Playdate%20with%20C.html#_ide_support)を参照してください。


## ライセンス

このプロジェクトのソースコードは、以下のライセンスの下に公開されています。

* MIT License(http://opensource.org/licenses/MIT)

## 謝辞

背景画像・BGMに、以下のサイトより素材を使用させていただきました。

・背景画像
フリー素材ぱくたそ	https://www.pakutaso.com/

・BGM
田中サムシングさん	https://dova-s.jp/_contents/author/profile030.html

どうもありがとうございます。


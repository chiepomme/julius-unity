julius-unity
=================

大語彙連続音声認識エンジン Julius を Unity のネイティブプラグインとして使用するためのつなぎを提供するプロジェクトです。

以下のライブラリや辞書を使用しています。

- [大語彙連続音声認識エンジン Julius](https://github.com/julius-speech/julius)
- [Juliusディクテーションキット](https://github.com/julius-speech/dictation-kit)
- [GRGSIBERIA/utf8-sjis-encoder](https://github.com/GRGSIBERIA/utf8-sjis-encoder)


対応プラットフォーム
-----------------

- Windows Editor x64
- iOS


使用方法
-----------------

リポジトリをクローンしたあと `<Root>/unity` を Unity で開き、 `Assets/Sample.unity` シーンを開いて再生すると音声認識が開始します。
iOS の場合には Unity でプロジェクトを書き出したあと Signing の設定だけ行ってビルドをしてください。

辞書ファイルは `<Root>/unity/Assets/StreamingAssets/grammar-kit/` 内にありますので、任意の辞書を使いたい場合そちらを更新してください。

現状インターフェースも決め兼ねていて互換性を気にせず更新していくため、このプロジェクトを参考にしてご自分でライブラリを作ることをおすすめします。



Julius Subtree Commands
-----------------

[大語彙連続音声認識エンジン Julius](https://github.com/julius-speech/julius) リポジトリを subtree コマンドでマージして使用しています。
更新のためには以下のコマンド群を使用します。

```
git remote add julius https://github.com/julius-speech/julius.git
git subtree add --prefix=julius --squash julius master
git subtree pull --prefix=julius --squash julius master
```


Author
-----------------

chiepomme  
http://chiepom.me/  
http://twitter.com/chiepomme  

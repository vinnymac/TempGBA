mokが弄ったDEVHOOK用の外部ファイル作成支援ツールです。
超手抜きプログラムなので、お見せするのも恥ずかしいですが参考まで。
VB6SP6で作っていますので、別途ランタイムが必要になります。

■bdf2fbm.exe
　ランチャーとXMB上に表示するフォントを作成するツールです。
　入力は、X11でおなじみのbdfファイルになります。
　手抜きのため、フォントセットは"ISO8859","JISX0201","JISX0208"のみに対応しています。
　他コードはマッピングさえ分かれば簡単に対応できるでしょう。
　※3バイト以上のコード体系は扱えません。

■mkmsg.exe
　ランチャー用のメッセージファイルを作成するツールです。
　できたファイルは、"ms0:/dh/mok/msg"配下にコピーしてください。
　上のbdf2fbmとこれでうまくすると他言語の表示もできるかと思います。
　私の環境が日本語環境のためか、欧文系の8ビット目立っている文字の入力/出力はできなさそうです。

使い方は、どちらも弄れば分かると思いますので、さっくり割愛します。



(おまけ)
【fbmファイルの仕様】
　ソースのユーザ定義体みてもらえば分かるとおもいますが、一応。
　※[]内はバイト数

　ヘッダ部
　　フォント数[2],
　　フォントマップ数[2],
　　デフォルトフォント[2],
　　フォント幅[1],	… プロポーショナルフォントの場合は、最大幅を設定
　　フォント高[1],
　　1フォントあたりのバイト数[1])

　マップ部　… 文字コードに対するフォント配列とのマッピングに使用
　　文字コード範囲(始め)[2],
　　文字コード範囲(終わり)[2],
　　文字コードとフォント配列との差分[2]

　フォント部
　　フォント幅[1],
　　フォントビットマップ(2値)[1バイトあたりのバイト数]

　fbmファイルには、ヘッダ部、マップ部、フォント部の順番に格納されています。
　マップ部は、ヘッダ部のフォントマップ数分繰り返します。
　フォント部は、ヘッダ部のフォント数分繰り返します。
　それぞれに隙間はありません。


【msgファイルの仕様】
　0x00でメッセージIDとメッセージを区切って隙間なく詰め込んでいます。
　改行コードは0x0aです。
　日本語はSJISで格納します。


This text written by mok.

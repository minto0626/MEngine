# 動作環境
- Visual Studio 2022
- Windows 11 x64

# 環境構築

## ➀ DirectXTexライブラリを入手
1. [DirectXTex](https://github.com/microsoft/DirectXTex) から `git clone`\
・任意の場所でよい

2. クローンしたディレクトリにある DirectXTex_Desktop_2022.sln を開く

3. DirectXTexをビルドする\
・プラットフォーム：x64\
・構成：Debug / Release

4. 環境変数を設定する\
・変数名：DXTEX_DIR\
・値：(クローンしたディレクトリのパス)/DirectXTex

## ➁ assimpライブラリを入手
1. [CMake](https://cmake.org/)が無い方はインストールする\
・筆者のバージョンは4.1.1

2. [assimp](https://github.com/assimp/assimp) から `git clone`
・任意の場所でよい

3. クローンしたディレクトリに移動してコマンドプロンプトを開き、assimpをビルドする\
`mkdir build`\
`cd build`\
`cmake ..`\
`cmake --build . --config Debug`\
`cmake --build . --config Release`

4. 環境変数を設定する\
・変数名：ASSIMP_ROOT_DIR\
・値：(クローンしたディレクトリのパス)

## ➂ 本プロジェクトをクローンし、ビルドできることを確認

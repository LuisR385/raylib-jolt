# raylib-jolt

raylib と Jolt Physics を使って、3D の描画と物理をつなぐプロジェクトです。
少しずつ rayjolt として育てていく予定です。

今は床に球を落として跳ねさせる、小さな Hello World が動きます。
描画と物理の処理は `src/main.cpp` にまとめてあり、Jolt の C++ API を直接使っています。

## 動かし方

CMake 3.20 以上と C++17 に対応したコンパイラが必要です。
Windows / Visual Studio 2022 では、次のコマンドで動かせます。

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --parallel
./build/Release/rayjolt.exe
```

R キーで球を最初の位置に戻し、Esc キーで終了します。
物理は 60 Hz の固定ステップで更新しています。

## 使用しているライブラリ

ソースは `external/` に置いています。ビルド時にダウンロードする必要はありません。

- [raylib 6.0](https://github.com/raysan5/raylib/releases/tag/6.0): `external/raylib`、zlib/libpng ライセンス。
- [Jolt Physics v5.6.0](https://github.com/jrouwe/JoltPhysics/releases/tag/v5.6.0): `external/JoltPhysics`、MIT ライセンス。

Jolt はリリースされたバージョンを使っていく予定です。
各ライブラリのライセンスは、それぞれのディレクトリに含まれています。

## 参考

床と球の構成は [Jolt raylib Hello World](https://github.com/rodneylab/jolt-raylib-hello-world) を、
Jolt の初期化と終了処理は公式の `HelloWorld/HelloWorld.cpp` を参考にしています。

## ライセンス

rayjolt は [MIT License](LICENSE) で公開しています。
Copyright (c) 2026 Kojima Ryoichi (@LuisR385)

`external/` 内のライブラリには、それぞれのライセンスが適用されます。

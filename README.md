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

## 三角メッシュの所有権

`CreateStaticMesh()`は入力の頂点・インデックスを内部の物理形状へ変換します。
生成後は入力の`TriangleMeshData`を変更・破棄しても、生成済みBodyは影響を受けません。
rfbxからコピーした入力配列も、Body生成後まで保持し続ける必要はありません。

BodyはWorldが管理します。`BodyHandle`は識別用なので、コピーや破棄ではBodyを削除しません。
個別に削除するときは`world.DestroyBody(handle)`を呼びます。
World終了時には残りのBodyを片付け、Joltの参照カウントで形状も解放します。

`context.End()`はWorldを破棄してからRuntimeを終了します。Contextがスコープを抜ける場合も同じ順序です。
終了後はWorldへの参照やBodyHandleを使わず、再初期化後は新しく取得してください。
`BodyHandle::IsValid()`はIDが空でないことだけを確認します。同じWorld内での存在確認には`ContainsBody()`を使います。

## rfbxから静的な当たり判定を作る

同梱の`include/rfbx.h`と`external/ufbx/ufbx.c`を使用します。rfbxは既定で有効です。

```powershell
cmake -S . -B build
cmake --build build --config Release
./build/Release/rayjolt.exe assets/box.fbx
```

FBXファイルは利用側で用意してください。rfbxを使わない場合は`-DRJ_ENABLE_RFBX=OFF`で無効にできます。
`RFBX_IMPLEMENTATION`はCMakeが生成する専用のCファイルだけで定義するため、利用側での追加定義は不要です。

`RJ::BuildRfbxCollisionMesh(model, mesh, transform)`に、描画と同じ行列を渡します。
成功したら `world.CreateStaticMesh(mesh)` で生成できます。ノードと描画の変換は頂点に反映済みなので、Body生成時に同じ変換を渡す必要はありません。
対象は読み込み時の表示対象ノードにある非スキニングメッシュです。アニメーションに合わせた当たり判定の更新は行いません。

描画用と衝突用を分ける場合は、第2引数に軽量な衝突用FBXを指定します。

```powershell
./build/Release/rayjolt.exe stage.fbx stage_collision.fbx
```

第2引数を省略すると、従来どおり描画モデルから当たり判定を作ります。
両ファイルは同じ原点・単位・軸に揃えて出力してください。同じ配置行列を使用し、自動の位置合わせは行いません。
衝突用モデルは変換後に解放し、描画には使用しません。指定した衝突用モデルの読み込みに失敗した場合は、描画用モデルへ自動で切り替えず、警告を出します。描画自体は継続します。

## 動く物体とアニメーションの当たり判定

静的なステージは`CreateStaticMesh()`、動く小物は`CreateConvexHull()`、キャラクターは`CreateCharacter()`のCapsuleを基本とします。

```cpp
RJ::ConvexHullDesc desc;
desc.position = {0, 3, 5};
desc.mass = 10.0f;
auto body = world.CreateConvexHull(mesh.vertices, desc);
```

凸包は頂点群を包む1つの凸形状です。穴や凹み、離れた部品の間も埋まります。平面や直線の頂点群は受け付けません。
`motionType`にはDynamic、Kinematic、Staticを指定できます。Kinematicは既存の`MoveKinematic()`で移動します。

rfbxのデモで動的な凸包を試す場合は、描画用・衝突用FBXの後に`--dynamic`を指定します。

```powershell
./build/Release/rayjolt.exe prop.fbx prop_collision.fbx --dynamic
```

描画はBodyの位置・回転に追従します。複雑な動く物体は、今後複数の凸形状を組み合わせる方針です。
ボーンの変形やノードのアニメーションを、生成済みの物理形状へ自動反映する処理はありません。
キャラクター全身の移動はCapsuleで扱い、将来の部位判定はボーンに追従するBoxやCapsuleとして分けます。

## 参考

床と球の構成は [Jolt raylib Hello World](https://github.com/rodneylab/jolt-raylib-hello-world) を、
Jolt の初期化と終了処理は公式の `HelloWorld/HelloWorld.cpp` を参考にしています。

## ライセンス

rayjolt は [MIT License](LICENSE) で公開しています。
Copyright (c) 2026 Kojima Ryoichi (@LuisR385)

同梱のrfbx（`include/rfbx.h`）も、同名義の[MIT License](include/rfbx.LICENSE)です。

`external/` 内のライブラリには、それぞれのライセンスが適用されます。

// Copyright (c) 2026 Kojima Ryoichi (@LuisR385)
// SPDX-License-Identifier: MIT

#ifndef RFBX_H
#define RFBX_H

#include "raylib.h"

#include <stdbool.h>
#include <stdint.h> //size_t, etc...

#ifdef __cplusplus
extern "C" {
#endif
    //有効でない数値のとき明示的にマクロで使用経由
#define RFBX_INVALID_INDEX (-1)

// raylib DrawMesh reads MAX_MATERIAL_MAPS (12), including one reserved slot
// beyond MATERIAL_MAP_BRDF. Keep that slot zeroed in temporary draw materials.
// raylibのDrawMeshを読むマテリアルマップ配列が12なので、一致させるために使用
#define RFBX_DRAW_MATERIAL_MAP_COUNT 12


#ifndef RFBX_MEMREALLOC
#define RFBX_MEMREALLOC(ptr,sz) MemRealloc(ptr,sz)
#endif

#ifndef RFBX_MEMALLOC
#define RFBX_MEMALLOC(sz) MemAlloc(sz)
#endif

#ifndef RFBX_MEMFREE
#define RFBX_MEMFREE(ptr) MemFree(ptr)
#endif

#ifndef RFBX_ALLOCARRAY
#define RFBX_ALLOCARRAY(count, elementSize) rfbxAllocArray(count, elementSize)
#endif
    //* rfbxAllocArray(size_t count, size_t elementSize)

// Maximum number of skin clusters uploaded to the built-in GPU shader.
// 組み込みGPU Skinning Shaderへ送る最大Skin Cluster数
#ifndef RFBX_MAX_GPU_BONES
#define RFBX_MAX_GPU_BONES 128
#endif

// Fixed names understood by RfbxSetModelShader().
#define RFBX_SHADER_ATTRIB_BONE_INDICES "vertexBoneIndices"
#define RFBX_SHADER_ATTRIB_BONE_WEIGHTS "vertexBoneWeights"
#define RFBX_SHADER_UNIFORM_BONE_MATRICES "boneMatrices"
#define RFBX_SHADER_UNIFORM_USE_SKINNING "rfbxUseSkinning"
#define RFBX_SHADER_UNIFORM_HAS_NORMAL_MAP "rfbxHasNormalMap"
#define RFBX_SHADER_UNIFORM_LIGHT_DIRECTION "rfbxLightDirection"
#define RFBX_SHADER_UNIFORM_LIGHT_COLOR "rfbxLightColor"
#define RFBX_SHADER_UNIFORM_LIGHT_INTENSITY "rfbxLightIntensity"
#define RFBX_SHADER_UNIFORM_AMBIENT_COLOR "rfbxAmbientColor"
#define RFBX_SHADER_UNIFORM_AMBIENT_INTENSITY "rfbxAmbientIntensity"
#define RFBX_SHADER_UNIFORM_VIEW_POSITION "rfbxViewPosition"
#define RFBX_SHADER_UNIFORM_HAS_METALLIC_MAP "rfbxHasMetallicMap"
#define RFBX_SHADER_UNIFORM_HAS_ROUGHNESS_MAP "rfbxHasRoughnessMap"
#define RFBX_SHADER_UNIFORM_HAS_EMISSION_MAP "rfbxHasEmissionMap"
#define RFBX_SHADER_UNIFORM_HAS_OPACITY_MAP "rfbxHasOpacityMap"
#define RFBX_SHADER_UNIFORM_METALLIC_FACTOR "rfbxMetallicFactor"
#define RFBX_SHADER_UNIFORM_ROUGHNESS_FACTOR "rfbxRoughnessFactor"
#define RFBX_SHADER_UNIFORM_ROUGHNESS_MAP_INVERTED "rfbxRoughnessMapInverted"
#define RFBX_SHADER_UNIFORM_EMISSION_COLOR "rfbxEmissionColor"
#define RFBX_SHADER_UNIFORM_ALPHA_MODE "rfbxAlphaMode"
#define RFBX_SHADER_UNIFORM_ALPHA_CUTOFF "rfbxAlphaCutoff"
#define RFBX_SHADER_UNIFORM_DOUBLE_SIDED "rfbxDoubleSided"

// How a material writes alpha during drawing.
// Material描画時のAlpha処理方式
    typedef enum RfbxAlphaMode {
        RFBX_ALPHA_OPAQUE = 0,
        RFBX_ALPHA_MASK,
        RFBX_ALPHA_BLEND
    } RfbxAlphaMode;

    // Texture channels understood by the built-in RFBX material shader.
    // 組み込みRFBX Material Shaderが扱うTexture Channel
    typedef enum RfbxMaterialMapType {
        RFBX_MATERIAL_MAP_BASE_COLOR = 0,
        RFBX_MATERIAL_MAP_NORMAL,
        RFBX_MATERIAL_MAP_METALLIC,
        RFBX_MATERIAL_MAP_ROUGHNESS,
        RFBX_MATERIAL_MAP_EMISSION,
        RFBX_MATERIAL_MAP_OPACITY
    } RfbxMaterialMapType;

    // A model-owned texture loaded from an external path or embedded FBX data.
    // 外部パスまたはFBX埋め込みデータから読み込んだModel所有Texture
    typedef struct RfbxTexture {
        char* name;
        char* fileName;
        uint32_t sourceTypedId;
        Texture2D texture;
        bool ready;
    } RfbxTexture;

    // A converted FBX surface material and its raylib representation.
    // FBX Surface Materialを変換したraylib Material表現
    typedef struct RfbxMaterial {
        char* name;
        uint32_t sourceTypedId;
        Color baseColor;
        Color emissiveColor;
        float metallic;
        float roughness;
        float opacity;
        float alphaCutoff;
        RfbxAlphaMode alphaMode;
        bool doubleSided;
        bool roughnessTextureIsGlossiness;
        int baseColorTextureIndex;
        int normalTextureIndex;
        int emissiveTextureIndex;
        int metallicTextureIndex;
        int roughnessTextureIndex;
        int opacityTextureIndex;
        Material material;
        bool materialReady;
    } RfbxMaterial;

    // A drawable, non-indexed section of an FBX mesh.
    //FBXメッシュのうち、描画可能でインデックス化されていない部分
    typedef struct RfbxPrimitive {
        Mesh mesh;
        int materialSlot;
        uint32_t* sourceIndices;
        unsigned char* boneIndices;
        float* boneWeights;
        unsigned int boneIndicesVboId;
        unsigned int boneWeightsVboId;
    } RfbxPrimitive;

    // A unique FBX mesh shared by one or more nodes.
    //1つ以上のノードによって共有される一意のFBXメッシュ
    typedef struct RfbxMesh {
        char* name;
        uint32_t sourceTypedId;
        bool hasSkinning;
        bool gpuSkinningReady;
        bool verticesAreLocal;
        bool boundsValid;
        BoundingBox localBounds;
        int gpuBoneCount;
        int materialCount;
        int* materialIndices;
        int primitiveCount;
        RfbxPrimitive* primitives;
    } RfbxMesh;

    // A node in the FBX transform hierarchy.
    //FBX変換階層内のノード
    typedef struct RfbxNode {
        char* name;
        uint32_t sourceTypedId;
        int parentIndex;
        int meshIndex;
        int materialCount;
        int* materialIndices;
        bool visible;
        Matrix nodeToParent;
        Matrix nodeToWorld;
        Matrix geometryToNode;
        Matrix geometryToWorld;
    } RfbxNode;

    // An animation stack exposed as a model-local clip.
    // モデル内のアニメーションスタックを表すクリップ
    typedef struct RfbxAnimation {
        char* name;
        double timeBegin;
        double timeEnd;
        double duration;
        uint32_t sourceStackIndex;
    } RfbxAnimation;

    // A game-defined marker on an animation clip. The name is model-owned.
    // ゲーム側からAnimation Clipへ登録する時刻付きEvent
    typedef struct RfbxAnimationEvent {
        const char* name;
        int animationIndex;
        double time;
        float normalizedTime;
    } RfbxAnimationEvent;

    // Model-space motion accumulated from a selected animated node.
    // 選択したAnimation Nodeから抽出したModel空間の移動・回転差分
    typedef struct RfbxRootMotion {
        Vector3 translation;
        Quaternion rotation;
    } RfbxRootMotion;

    // One directional light plus ambient light used by the built-in shader.
    // 組み込みShaderが使用する平行光源と環境光
    typedef struct RfbxLighting {
        Vector3 direction;      // Direction the light rays travel in draw/world space
        Color color;            // Directional light color
        float intensity;        // Directional light multiplier, zero or greater
        Color ambientColor;     // Ambient light color
        float ambientIntensity; // Ambient light multiplier, zero or greater
    } RfbxLighting;

    // A bone backed by an FBX node in the normalized RFBX coordinate space.
    // RFBX座標系へ正規化されたFBX Nodeを参照するBone
    typedef struct RfbxBone {
        char* name;
        uint32_t sourceNodeTypedId;
        int parentIndex;
        bool isSkinBone;
        float radius;
        Matrix bindToWorld;
    } RfbxBone;

    // A model-owned bone hierarchy shared by all instances.
    // 全Instanceから共有されるModel所有のBone階層
    typedef struct RfbxSkeleton {
        int boneCount;
        RfbxBone* bones;
    } RfbxSkeleton;

    // A named attachment point backed by an FBX node and a local offset.
    // FBX NodeとLocal Offsetから構成される名前付きAttachment Point
    typedef struct RfbxSocket {
        char* name;
        int nodeIndex;
        int boneIndex;
        Matrix localTransform;
    } RfbxSocket;

    typedef struct RfbxInternal RfbxInternal;
    typedef struct RfbxInstanceInternal RfbxInstanceInternal;

    // An FBX scene converted to raylib resources.
    //FBXシーンをraylibリソースに変換するもの
    typedef struct RfbxModel {
        bool valid;
        int nodeCount;
        RfbxNode* nodes;
        int meshCount;
        RfbxMesh* meshes;
        int textureCount;
        RfbxTexture* textures;
        int materialCount;
        RfbxMaterial* materials;
        int animationCount;
        RfbxAnimation* animations;
        RfbxSkeleton skeleton;
        int socketCount;
        RfbxSocket* sockets;
        int currentAnimation;
        double currentTime;
        Material defaultMaterial;
        bool defaultMaterialReady;
        bool boundsValid;
        BoundingBox bounds;
        RfbxInternal* internal;
    } RfbxModel;

    // A playback and pose instance that references a shared model.
    // 共有Modelを参照する独立した再生状態とPose
    typedef struct RfbxInstance {
        bool valid;
        const RfbxModel* model;
        int currentAnimation;
        double currentTime;
        int nextAnimation;
        double nextTime;
        double fadeDuration;
        double fadeTime;
        int queuedAnimation;
        double queuedFadeDuration;
        int layerAnimation;
        double layerTime;
        double layerFadeDuration;
        double layerFadeTime;
        float playbackSpeed;
        float layerPlaybackSpeed;
        float layerWeight;
        float layerFadeStartWeight;
        float layerTargetWeight;
        bool loop;
        bool nextLoop;
        bool queuedLoop;
        bool paused;
        bool playing;
        bool fading;
        bool layerLoop;
        bool layerPlaying;
        bool boundsValid;
        BoundingBox bounds;
        RfbxInstanceInternal* internal;
    } RfbxInstance;

    // Load an FBX model using the RFBX coordinate convention.
    //RFBX座標系を使用してFBXモデルを読み込む
    RfbxModel RfbxLoadModel(const char* fileName);

    // Load an FBX and automatically apply a matching optional .rfbxmat sidecar.
    // FBXと同名の任意.rfbxmatを自動適用してモデルを読み込む
    RfbxModel RfbxLoadModelWithMaterial(const char* fileName);

    // Return whether a model was loaded successfully.
    // モデルが正常に読み込まれたかどうかを返す
    bool RfbxIsModelValid(const RfbxModel* model);

    // Return how many meshes can use the built-in GPU skinning path.
    // 組み込みGPU Skinning経路を使用できるMesh数を返す
    int RfbxGetGpuSkinnedMeshCount(const RfbxModel* model);

    // Return whether a mesh uses GPU skinning, or false for an invalid index.
    // 指定MeshがGPU Skinning対応か返す(無効なIndexはfalse)
    bool RfbxIsMeshGpuSkinned(const RfbxModel* model, int meshIndex);

    // Replace the draw shader without transferring ownership to the model.
    // Modelへ所有権を渡さず描画Shaderを差し替える
    bool RfbxSetModelShader(RfbxModel* model, Shader shader);

    // Restore the model-owned lighting and skinning shader.
    // Model所有のLighting/Skinning Shaderへ戻す
    void RfbxResetModelShader(RfbxModel* model);

    // Return whether a caller-owned shader is currently selected.
    // 利用側所有Shaderが現在選択されているか返す
    bool RfbxIsUsingCustomShader(const RfbxModel* model);

    // Return the default lighting values used for newly loaded models.
    // 新規Modelで使用される標準Lighting設定を返す
    RfbxLighting RfbxGetDefaultLighting(void);

    // Change directional and ambient lighting for the active compatible shader.
    // 対応Shaderへ渡す平行光源と環境光を変更する
    bool RfbxSetModelLighting(RfbxModel* model, RfbxLighting lighting);

    // Return the current per-model lighting values.
    // Modelに設定された現在のLightingを返す
    RfbxLighting RfbxGetModelLighting(const RfbxModel* model);

    // Return the number of animation stacks in a model.
    // モデルに含まれるアニメーションクリップ数を返す
    int RfbxGetAnimationCount(const RfbxModel* model);

    // Return an animation clip, or NULL if the index is invalid.
    // 指定したアニメーションクリップを返す
    const RfbxAnimation* RfbxGetAnimation(
        const RfbxModel* model,
        int animationIndex);

    // Find an animation by its exact FBX stack name.
    // FBXのAnimation Stack名からクリップ番号を検索する
    int RfbxFindAnimation(const RfbxModel* model, const char* name);

    // Register an event at clip-local seconds.
    // Animation Clip内の秒数を指定してEventを登録する
    bool RfbxAddAnimationEvent(
        RfbxModel* model,
        const char* animationName,
        double timeSeconds,
        const char* eventName);

    // Register an event at normalized clip time in the range 0..1.
    // Animation Clip内の正規化時刻0～1を指定してEventを登録する
    bool RfbxAddAnimationEventNormalized(
        RfbxModel* model,
        const char* animationName,
        float normalizedTime,
        const char* eventName);

    // Return the number of registered events for a named clip.
    // 指定Clipに登録されているEvent数を返す
    int RfbxGetAnimationEventCount(
        const RfbxModel* model,
        const char* animationName);

    // Return one registered event in chronological order.
    // The pointer remains valid until another event is added or the model unloads.
    // 指定ClipのEventを時刻順で返す
    const RfbxAnimationEvent* RfbxGetAnimationEvent(
        const RfbxModel* model,
        const char* animationName,
        int eventIndex);

    // Return the number of textures loaded or referenced by the FBX scene.
    // FBX Sceneから読み込み・参照されたTexture数を返す
    int RfbxGetTextureCount(const RfbxModel* model);

    // Return a model-owned texture definition. The pointer may change after
    // RfbxSetMaterialTexture() appends another texture.
    // Model所有Textureを返す(追加Texture設定後は以前のPointerを再取得する)
    const RfbxTexture* RfbxGetTexture(const RfbxModel* model, int textureIndex);

    // Return the number of converted FBX materials.
    // 変換済みFBX Material数を返す
    int RfbxGetMaterialCount(const RfbxModel* model);

    // Return a model-owned material definition.
    // Model所有のMaterial定義を返す
    const RfbxMaterial* RfbxGetMaterial(const RfbxModel* model, int materialIndex);

    // Find the first material with an exact, case-sensitive FBX material name.
    // FBX Material名の完全一致で最初のMaterial番号を検索する
    int RfbxFindMaterial(const RfbxModel* model, const char* name);

    // Load and assign a model-owned texture file to one material channel.
    // 画像をModel所有Textureとして読み込み、指定Material Channelへ割り当てる
    bool RfbxSetMaterialTexture(
        RfbxModel* model,
        int materialIndex,
        RfbxMaterialMapType mapType,
        const char* fileName);

    // Remove a runtime texture override and restore the original FBX connection.
    // 実行時Texture上書きを外し、元のFBX Texture接続へ戻す
    bool RfbxClearMaterialTexture(
        RfbxModel* model,
        int materialIndex,
        RfbxMaterialMapType mapType);

    // Change the base color factor used with or without a color texture.
    // Color Textureの有無にかかわらずBase Color係数を変更する
    bool RfbxSetMaterialBaseColor(
        RfbxModel* model,
        int materialIndex,
        Color color);

    // Change the metallic factor in the normalized range 0..1.
    // Metallic係数を0～1の範囲で変更する
    bool RfbxSetMaterialMetallic(
        RfbxModel* model,
        int materialIndex,
        float metallic);

    // Change the roughness factor in the normalized range 0..1.
    // Roughness係数を0～1の範囲で変更する
    bool RfbxSetMaterialRoughness(
        RfbxModel* model,
        int materialIndex,
        float roughness);

    // Restore every supported value and texture to the loaded FBX material state.
    // 対応する全ParameterとTextureをFBX読み込み直後の状態へ戻す
    bool RfbxResetMaterial(RfbxModel* model, int materialIndex);

    // Apply named material overrides from a versioned .rfbxmat JSON file.
    // Version付き.rfbxmat JSONからMaterial名指定の上書きを適用する
    bool RfbxLoadMaterialFile(RfbxModel* model, const char* fileName);

    // Save the current material overrides as a version 1 .rfbxmat JSON file.
    // 現在のMaterial設定をVersion 1 .rfbxmat JSONとして保存する
    bool RfbxSaveMaterialFile(const RfbxModel* model, const char* fileName);

    // Override ambiguous FBX transparency as opaque, cutout, or blended.
    // FBXで曖昧な透過方式をOpaque、Cutout、Blendから明示指定する
    bool RfbxSetMaterialAlphaMode(
        RfbxModel* model,
        int materialIndex,
        RfbxAlphaMode alphaMode,
        float alphaCutoff);

    // Enable or disable two-sided drawing for one material.
    // 指定Materialの両面描画を切り替える
    bool RfbxSetMaterialDoubleSided(
        RfbxModel* model,
        int materialIndex,
        bool doubleSided);

    // Return the number of bones collected from nodes and skin clusters.
    // NodeとSkin Clusterから収集したBone数を返す
    int RfbxGetBoneCount(const RfbxModel* model);

    // Return a bone definition, or NULL if the index is invalid.
    // 指定したBone定義を返す
    const RfbxBone* RfbxGetBone(const RfbxModel* model, int boneIndex);

    // Find a bone by its exact FBX node name.
    // FBX Node名からBone番号を検索する
    int RfbxFindBone(const RfbxModel* model, const char* name);

    // Return the current instance-space bone-to-world matrix.
    // Instanceの現在のBone-to-World行列を返す
    Matrix RfbxGetBoneToWorld(const RfbxInstance* instance, int boneIndex);

    // Add or replace a named socket attached to a skeleton bone.
    // Skeleton Boneへ名前付きSocketを追加または更新する
    bool RfbxAddBoneSocket(
        RfbxModel* model,
        const char* socketName,
        const char* boneName,
        Matrix localTransform);

    // Add or replace a named socket attached to any FBX node.
    // 任意のFBX Nodeへ名前付きSocketを追加または更新する
    bool RfbxAddNodeSocket(
        RfbxModel* model,
        const char* socketName,
        const char* nodeName,
        Matrix localTransform);

    // Return the number of registered sockets.
    // Modelへ登録されたSocket数を返す
    int RfbxGetSocketCount(const RfbxModel* model);

    // Return a model-owned socket definition.
    // Model所有のSocket定義を返す
    const RfbxSocket* RfbxGetSocket(
        const RfbxModel* model,
        int socketIndex);

    // Find a socket by exact, case-sensitive name.
    // 大文字小文字を区別する名前からSocket番号を検索する
    int RfbxFindSocket(const RfbxModel* model, const char* socketName);

    // Return the current socket matrix in the same model space used for drawing.
    // 現在の描画用Model空間におけるSocket行列を返す
    Matrix RfbxGetSocketToModel(
        const RfbxInstance* instance,
        int socketIndex);

    // Apply the same placement matrix passed to RfbxDrawInstanceMatrix().
    // RfbxDrawInstanceMatrix()と同じ配置行列を適用したSocket行列を返す
    Matrix RfbxGetSocketToWorld(
        const RfbxInstance* instance,
        int socketIndex,
        Matrix transform);

    // Draw RGB axes for every socket using the current animated pose.
    // 現在Poseにある全SocketをRGB軸でデバッグ描画する
    void RfbxDrawSockets(
        const RfbxInstance* instance,
        Matrix transform,
        float axisLength);

    // Extract root motion from any named FBX node.
    // FBX Node名を指定してRoot Motion抽出を有効にする
    bool RfbxSetRootMotionNode(
        RfbxInstance* instance,
        const char* nodeName,
        bool inPlace);

    // Extract root motion from a named skeleton bone.
    // Skeleton Bone名を指定してRoot Motion抽出を有効にする
    bool RfbxSetRootMotionBone(
        RfbxInstance* instance,
        const char* boneName,
        bool inPlace);

    // Stop root motion extraction and remove in-place correction.
    // Root Motion抽出とIn-place補正を無効にする
    void RfbxDisableRootMotion(RfbxInstance* instance);

    // Return whether a root motion source is configured.
    // Root Motion抽出が有効か返す
    bool RfbxIsRootMotionEnabled(const RfbxInstance* instance);

    // Return the selected root motion node name, or NULL when disabled.
    // 選択中のRoot Motion Node名を返す
    const char* RfbxGetRootMotionNodeName(const RfbxInstance* instance);

    // Enable or disable visual in-place correction without changing playback.
    // 再生状態を変えずに描画用In-place補正を切り替える
    void RfbxSetRootMotionInPlace(RfbxInstance* instance, bool inPlace);

    // Return whether visual in-place correction is active.
    // 描画用In-place補正が有効か返す
    bool RfbxIsRootMotionInPlace(const RfbxInstance* instance);

    // Read accumulated root motion without clearing it.
    // 前回ResetまたはConsume以降のRoot Motionを保持したまま取得する
    RfbxRootMotion RfbxPeekRootMotion(const RfbxInstance* instance);

    // Read and clear accumulated root motion.
    // 蓄積したRoot Motionを取得して差分をクリアする
    RfbxRootMotion RfbxConsumeRootMotion(RfbxInstance* instance);

    // Clear pending and in-place accumulated motion at the current pose.
    // 現在Poseを基準にRoot Motion差分とIn-place累積をリセットする
    void RfbxResetRootMotion(RfbxInstance* instance);

    // Return the model-space correction used for in-place drawing.
    // In-place描画へ使用するModel空間補正行列を返す
    Matrix RfbxGetRootMotionCorrection(const RfbxInstance* instance);

    // Draw joints and parent links for debugging an animated skeleton.
    // Animation Skeletonの関節と親子線をデバッグ描画する
    void RfbxDrawSkeleton(
        const RfbxInstance* instance,
        Matrix transform,
        float jointRadius,
        Color color);

    // Create an independent pose and playback state. The model must outlive it.
    // 共有Modelから独立した再生Instanceを作る(ModelはInstanceより長く保持する)
    RfbxInstance RfbxCreateInstance(const RfbxModel* model);

    // Return whether an instance is ready for update and drawing.
    // Instanceが更新・描画可能か返す
    bool RfbxIsInstanceValid(const RfbxInstance* instance);

    // Start an animation from the beginning.
    // 指定クリップを先頭から再生する
    bool RfbxPlayAnimation(
        RfbxInstance* instance,
        int animationIndex,
        bool loop);

    // Blend from the current clip to another clip over fadeDuration seconds.
    // 現在のClipから別Clipへ指定秒数でCross Fadeする
    bool RfbxCrossFadeAnimation(
        RfbxInstance* instance,
        int animationIndex,
        double fadeDuration,
        bool loop);

    // Start an animation by its exact FBX stack name.
    // FBXのAnimation Stack名を指定して先頭から再生する
    bool RfbxPlay(
        RfbxInstance* instance,
        const char* animationName,
        bool loop);

    // Cross-fade to an animation selected by its exact FBX stack name.
    // FBXのAnimation Stack名を指定してCross Fadeする
    bool RfbxCrossFade(
        RfbxInstance* instance,
        const char* animationName,
        double fadeDuration,
        bool loop);

    // Play a named clip once, optionally cross-fading from the current clip.
    // 名前指定Clipを1回だけ再生する(現在ClipからのFade秒数を指定可能)
    bool RfbxPlayOnce(
        RfbxInstance* instance,
        const char* animationName,
        double fadeDuration);

    // Queue a named clip after the current non-looping clip.
    // 現在の非Loop Clip終了時に再生するClipを予約する
    bool RfbxQueueAnimation(
        RfbxInstance* instance,
        const char* animationName,
        double fadeDuration,
        bool loop);

    // Play one named clip once, then automatically transition to a looping clip.
    // One-shot再生後に指定Clipへ自動で戻してLoop再生する
    bool RfbxPlayOnceThen(
        RfbxInstance* instance,
        const char* oneShotName,
        const char* nextAnimationName,
        double fadeInDuration,
        double fadeOutDuration);

    // Play one clip over a bone subtree without replacing the base animation.
    // Base Animationを置き換えずBone階層へClipを重ねて再生する
    bool RfbxPlayAnimationLayer(
        RfbxInstance* instance,
        const char* animationName,
        const char* maskRootBoneName,
        float weight,
        double fadeInDuration,
        bool loop);

    // Fade the active layer out, or remove it immediately at zero duration.
    // 追加Animation LayerをFade Outまたは即時解除する
    void RfbxStopAnimationLayer(
        RfbxInstance* instance,
        double fadeOutDuration);

    // Change the layer target weight with an optional fade.
    // 追加Animation Layerの目標WeightをFade時間付きで変更する
    void RfbxSetAnimationLayerWeight(
        RfbxInstance* instance,
        float weight,
        double fadeDuration);

    // Change only the independent layer playback speed.
    // Baseとは独立したLayer再生速度を設定する
    void RfbxSetAnimationLayerSpeed(RfbxInstance* instance, float speed);

    // Return whether a layer is configured, including a layer fading out.
    // Fade Out中を含めAnimation Layerが設定済みか返す
    bool RfbxIsAnimationLayerActive(const RfbxInstance* instance);

    // Return whether the layer clip time is still advancing.
    // Layer Clipの時刻が現在進行中か返す
    bool RfbxIsAnimationLayerPlaying(const RfbxInstance* instance);

    // Return the active layer clip name, or NULL.
    // 現在のLayer Clip名を返す
    const char* RfbxGetAnimationLayerName(const RfbxInstance* instance);

    // Return the bone name at the root of the current layer mask.
    // 現在のLayer MaskルートBone名を返す
    const char* RfbxGetAnimationLayerRootBoneName(
        const RfbxInstance* instance);

    // Return the current effective layer weight in the range 0..1.
    // 現在適用中のLayer Weightを0～1で返す
    float RfbxGetAnimationLayerWeight(const RfbxInstance* instance);

    // Return current layer clip-local seconds.
    // 現在のLayer Clip内時刻を秒で返す
    double RfbxGetAnimationLayerTime(const RfbxInstance* instance);

    // Return whether an animation is waiting in the playback queue.
    // 次に再生するAnimationが予約されているか返す
    bool RfbxHasQueuedAnimation(const RfbxInstance* instance);

    // Return the queued clip name, or NULL if the queue is empty.
    // 予約中のClip名を返す(予約なしの場合はNULL)
    const char* RfbxGetQueuedAnimationName(const RfbxInstance* instance);

    // Cancel a queued clip without changing the current playback state.
    // 現在の再生状態を変えずに予約Clipを取り消す
    void RfbxClearAnimationQueue(RfbxInstance* instance);

    // Pop the oldest event fired by this instance.
    // Instanceで発火した最も古いAnimation Eventを取り出す
    bool RfbxPollAnimationEvent(
        RfbxInstance* instance,
        RfbxAnimationEvent* event);

    // Return the number of fired events waiting to be polled.
    // Poll待ちのAnimation Event数を返す
    int RfbxGetPendingAnimationEventCount(const RfbxInstance* instance);

    // Discard all fired events waiting on this instance.
    // Instanceに残っているPoll待ちEventをすべて破棄する
    void RfbxClearPendingAnimationEvents(RfbxInstance* instance);

    // Return the active clip name. During a fade this is the destination clip.
    // 現在のClip名を返す(Cross Fade中は遷移先、未選択時はNULL)
    const char* RfbxGetCurrentAnimationName(const RfbxInstance* instance);

    // Return active clip-local seconds. During a fade this is destination time.
    // 現在のClip内時刻を秒で返す(Cross Fade中は遷移先時刻)
    double RfbxGetAnimationTime(const RfbxInstance* instance);

    // Return active clip progress in the inclusive range 0..1.
    // 現在のClip再生進捗を0～1で返す
    float RfbxGetAnimationProgress(const RfbxInstance* instance);

    // Return whether animation time is currently advancing.
    // Animation時刻が現在進行中か返す(Pause中はfalse)
    bool RfbxIsAnimationPlaying(const RfbxInstance* instance);

    // Return whether animation playback is paused.
    // Animation再生がPause中か返す
    bool RfbxIsAnimationPaused(const RfbxInstance* instance);

    // Return whether a cross-fade transition is active.
    // Cross Fade遷移中か返す
    bool RfbxIsAnimationFading(const RfbxInstance* instance);

    // Return whether a non-looping animation has reached its end.
    // 非Loop Animationが終端へ到達したか返す
    bool RfbxIsAnimationFinished(const RfbxInstance* instance);

    // Stop playback and restore the loaded pose.
    // 再生を停止して読み込み直後のPoseへ戻す
    void RfbxStopAnimation(RfbxInstance* instance);

    // Pause or resume animation time advancement.
    // アニメーション時間の進行を一時停止・再開する
    void RfbxSetAnimationPaused(RfbxInstance* instance, bool paused);

    // Set the playback speed multiplier.
    // アニメーションの再生速度倍率を設定する
    void RfbxSetAnimationSpeed(RfbxInstance* instance, float speed);

    // Seek the current clip without changing play, pause, or loop state.
    // 再生状態を変えずに現在のクリップ時刻へ移動する
    bool RfbxSetAnimationTime(RfbxInstance* instance, double timeSeconds);

    // Advance and evaluate an instance by delta seconds.
    // 経過秒数だけInstanceを進めて評価する
    bool RfbxUpdateInstance(RfbxInstance* instance, double deltaSeconds);

    // Return the current model-space bounds, or a zero box if no mesh is visible.
    // 現在PoseのModel空間Boundsを返す(表示MeshがなければZero Box)
    BoundingBox RfbxGetModelBounds(const RfbxModel* model);

    // Return the current instance bounds at the identity placement.
    // 現在PoseのInstance Boundsを配置Transformなしで返す
    BoundingBox RfbxGetInstanceBounds(const RfbxInstance* instance);

    // Return current instance bounds transformed into caller world space.
    // 現在PoseのBoundsへ配置Transformを適用して返す
    BoundingBox RfbxGetInstanceBoundsMatrix(
        const RfbxInstance* instance,
        Matrix transform);

    // Draw current world bounds for animation and authoring checks.
    // Animation確認用に現在のWorld Boundsを描画する
    void RfbxDrawInstanceBounds(
        const RfbxInstance* instance,
        Matrix transform,
        Color color);

    // Draw an instance at a position with a uniform scale.
    // Instanceを位置と均一Scaleで描画する
    void RfbxDrawInstance(
        const RfbxInstance* instance,
        Vector3 position,
        float scale,
        Color tint);

    // Draw an instance at position with a uniform scale, and rotation(radian).
    // Instanceを位置,均一のScaleとrotationで描画する
    // NOTE : This api change scale Vector3 base.
    void RfbxDrawInstanceEx(
        const RfbxInstance* instance,
        Vector3 position,
        Vector3 rotationAxix,
        float rotationAngle,
        Vector3 scale,
        Color tint);

    // Draw an instance with an explicit root transform.
    // Instanceを明示的なRoot Transformで描画する
    void RfbxDrawInstanceMatrix(
        const RfbxInstance* instance,
        Matrix transform,
        Color tint);

    // Release instance-owned pose and dynamic mesh resources.
    // Instance専用のPoseと動的Meshを解放する
    void RfbxUnloadInstance(RfbxInstance* instance);

    // Evaluate a clip at model-local seconds and update nodes and deformation.
    // クリップ先頭からの秒数でNodeとSkinningを更新する
    bool RfbxUpdateAnimation(
        RfbxModel* model,
        int animationIndex,
        double timeSeconds);

    // Restore the bind/static scene loaded from the FBX file.
    // FBX読み込み直後の静的な状態へ戻す
    void RfbxResetAnimation(RfbxModel* model);

    // Draw a model at a position with a uniform scale.
    //Rfbx経由で読み込まれたモデルを描画する
    void RfbxDrawModel(
        const RfbxModel* model,
        Vector3 position,
        float scale,
        Color tint);

    // Draw a model with an explicit root transform.
    // Rfbx経由で読み込まれたモデルを描画する(Matrixを操作したい場合のみ使用)
    void RfbxDrawModelMatrix(
        const RfbxModel* model,
        Matrix transform,
        Color tint);

    // Release all CPU, GPU, and ufbx resources owned by a model.
    //すべてのCPU,GPUI,ufbxのリソースを持ったモデルの解放処理を行う
    void RfbxUnloadModel(RfbxModel* model);

    // all print model summary native information.
    // モデルにあるすべての結果を表示する
    void RfbxPrintModelSummary(const RfbxModel* model);

    void RfbxDrawNodes(
        const RfbxInstance* instance,
        Matrix transform,
        float axisLength);

    void RfbxDrawNodeHierarchy(
        const RfbxInstance* instance,
        Matrix transform
    );

    void RfbxDrawMeshBounds(
        const RfbxInstance* instance,
        Matrix transform,
        Color color);

    void RfbxDrawDebugInfo(
        const RfbxInstance* instance,
        int posX,
        int posY); //TODO : enum列挙型で複数、個数のみで表示可能にさせる(ビット演算で良い)

    void RfbxPrintDebugInfo(const RfbxInstance* instance);

    void RfbxSetAnimationLoop(
        RfbxInstance* instance,
        bool loop);

    bool RfbxIsAnimationLooping(RfbxInstance* instance);

    int RfbxGetCurrentAnimationIndex(
        const RfbxInstance* instance);

    bool RfbxSetNodeScaleMultiplier(
        RfbxInstance* instance,
        int nodeIndex,
        Vector3 multiplier);

    bool RfbxSetNodeOffset(
        RfbxInstance* instance,
        int nodeIndex,
        Vector3 offset //NOTE : a node of offset.
    );

#ifdef __cplusplus
}
#endif

#endif // RFBX_H
//IMPL
#if defined(RFBX_IMPLEMENTATION) && !defined(RFBX_IMPLEMENTATION_INCLUDED)
#define RFBX_IMPLEMENTATION_INCLUDED

#include "raymath.h" //NOTE : 依存関係をraylibのみ、またはユーザーによってraymathをoptionalとして考える
#include "rlgl.h"
#include "ufbx.h"

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct RfbxAnimationEventListInternal {
    int count;
    int capacity;
    RfbxAnimationEvent* events;
} RfbxAnimationEventListInternal;

typedef struct RfbxGpuMeshState {
    int boneCount;
    Matrix* boneMatrices;
    BoundingBox* boneLocalBounds;
    bool* boneBoundsValid;
} RfbxGpuMeshState;

typedef struct RfbxMaterialDefaultState {
    int textureIndices[6];
    Texture2D textures[6];
    Color baseColor;
    Color emissiveColor;
    float metallic;
    float roughness;
    float opacity;
    float alphaCutoff;
    RfbxAlphaMode alphaMode;
    bool doubleSided;
    bool roughnessTextureIsGlossiness;
} RfbxMaterialDefaultState;

struct RfbxInternal {
    ufbx_scene* scene;
    RfbxAnimationEventListInternal* animationEventLists;
    RfbxMaterialDefaultState* materialDefaults;
    int socketCapacity;
    Shader builtinShader;
    Shader activeShader;
    int gpuBoneMatricesLocation;
    int useSkinningLocation;
    int hasNormalMapLocation;
    int lightDirectionLocation;
    int lightColorLocation;
    int lightIntensityLocation;
    int ambientColorLocation;
    int ambientIntensityLocation;
    int viewPositionLocation;
    int hasMetallicMapLocation;
    int hasRoughnessMapLocation;
    int hasEmissionMapLocation;
    int hasOpacityMapLocation;
    int metallicFactorLocation;
    int roughnessFactorLocation;
    int roughnessMapInvertedLocation;
    int emissionColorLocation;
    int alphaModeLocation;
    int alphaCutoffLocation;
    int doubleSidedLocation;
    bool builtinShaderReady;
    bool activeShaderReady;
    bool usingCustomShader;
    RfbxLighting lighting;
    RfbxGpuMeshState* gpuMeshes;
};

typedef struct RfbxInstanceNodeState {
    bool visible;
    Matrix nodeToParent;
    Matrix nodeToWorld;
    Matrix geometryToNode;
    Matrix geometryToWorld;
    Vector3 scaleMultiplier;
    Vector3 offset; //NOTE : モデル座標での位置補正ためだけに使用する
} RfbxInstanceNodeState;

typedef struct RfbxInstanceMeshState {
    bool verticesAreLocal;
    bool gpuSkinning;
    int primitiveCount;
    Mesh* primitives;
    int boneCount;
    Matrix* boneMatrices;
} RfbxInstanceMeshState;

struct RfbxInstanceInternal {
    int nodeCount;
    int meshCount;
    RfbxInstanceNodeState* nodes;
    RfbxInstanceMeshState* meshes;
    Matrix* boneToWorld;
    ufbx_transform_override* blendOverrides;
    bool* layerNodeMask;
    int layerRootBoneIndex;
    bool layerEventStartPending;
    bool layerRemoveWhenWeightZero;
    RfbxAnimationEvent* pendingEvents;
    int pendingEventCount;
    int pendingEventReadIndex;
    int pendingEventCapacity;
    bool currentEventStartPending;
    bool nextEventStartPending;
    int rootMotionNodeIndex;
    bool rootMotionEnabled;
    bool rootMotionInPlace;
    bool rootMotionHasPrevious;
    Matrix rootMotionPreviousTransform;
    Matrix rootMotionVisualOriginTransform;
    Matrix* rootMotionStartTransforms;
    Matrix* rootMotionEndTransforms;
    RfbxRootMotion pendingRootMotion;
};

// Allocate and zero an array using raylib's allocator.
//内部で使用。Rfbxの動的配列確保をします
static void* rfbxAllocArray(size_t count, size_t elementSize)
{
    if (count == 0 || elementSize == 0) return NULL;
    if (count > (size_t)UINT_MAX / elementSize) return NULL;

    unsigned int size = (unsigned int)(count * elementSize);
    void* data = MemAlloc(size);
    if (data) memset(data, 0, size);
    return data;
}

// Copy a length-delimited ufbx string to an owned C string.
static char* rfbxCopyString(ufbx_string source)
{
    if (source.length >= UINT_MAX) return NULL;

    char* result = MemAlloc((unsigned int)source.length + 1u);
    if (!result) return NULL;

    if (source.length > 0) memcpy(result, source.data, source.length);
    result[source.length] = '\0';
    return result;
}

// Convert an affine ufbx 4x3 matrix to a raylib 4x4 matrix.
static Matrix rfbxMatrixFromUfbx(ufbx_matrix source)
{
    Matrix result = { 0 };

    result.m0 = (float)source.m00;
    result.m4 = (float)source.m01;
    result.m8 = (float)source.m02;
    result.m12 = (float)source.m03;

    result.m1 = (float)source.m10;
    result.m5 = (float)source.m11;
    result.m9 = (float)source.m12;
    result.m13 = (float)source.m13;

    result.m2 = (float)source.m20;
    result.m6 = (float)source.m21;
    result.m10 = (float)source.m22;
    result.m14 = (float)source.m23;

    result.m15 = 1.0f;
    return result;
}

// Extend an optional axis-aligned box by one finite point.
static void rfbxBoundsIncludePoint(
    BoundingBox* bounds,
    bool* valid,
    Vector3 point)
{
    if (!bounds || !valid ||
        !isfinite(point.x) || !isfinite(point.y) || !isfinite(point.z)) {
        return;
    }

    if (!*valid) {
        bounds->min = point;
        bounds->max = point;
        *valid = true;
        return;
    }

    if (point.x < bounds->min.x) bounds->min.x = point.x;
    if (point.y < bounds->min.y) bounds->min.y = point.y;
    if (point.z < bounds->min.z) bounds->min.z = point.z;
    if (point.x > bounds->max.x) bounds->max.x = point.x;
    if (point.y > bounds->max.y) bounds->max.y = point.y;
    if (point.z > bounds->max.z) bounds->max.z = point.z;
}

// Merge a valid source box into an optional destination box.
static void rfbxBoundsIncludeBox(
    BoundingBox* bounds,
    bool* valid,
    BoundingBox source)
{
    rfbxBoundsIncludePoint(bounds, valid, source.min);
    rfbxBoundsIncludePoint(bounds, valid, source.max);
}

// Transform all eight corners and return their enclosing axis-aligned box.
static BoundingBox rfbxTransformBounds(BoundingBox source, Matrix transform)
{
    BoundingBox result = { 0 };
    bool valid = false;
    for (int corner = 0; corner < 8; ++corner) {
        Vector3 point = {
            (corner & 1) ? source.max.x : source.min.x,
            (corner & 2) ? source.max.y : source.min.y,
            (corner & 4) ? source.max.z : source.min.z
        };
        rfbxBoundsIncludePoint(
            &result,
            &valid,
            Vector3Transform(point, transform));
    }
    return result;
}

// Calculate exact bounds from a raylib non-indexed vertex stream.
static bool rfbxCalculateMeshBounds(const Mesh* mesh, BoundingBox* bounds)
{
    if (!mesh || !bounds || !mesh->vertices || mesh->vertexCount <= 0) {
        return false;
    }

    bool valid = false;
    *bounds = (BoundingBox){ 0 };
    for (int vertex = 0; vertex < mesh->vertexCount; ++vertex) {
        Vector3 point = {
            mesh->vertices[(size_t)vertex * 3u + 0u],
            mesh->vertices[(size_t)vertex * 3u + 1u],
            mesh->vertices[(size_t)vertex * 3u + 2u]
        };
        rfbxBoundsIncludePoint(bounds, &valid, point);
    }
    return valid;
}

// Clamp a material scalar to the normalized display range.
static float rfbxClamp01(double value)
{
    if (value < 0.0) return 0.0f;
    if (value > 1.0) return 1.0f;
    return (float)value;
}

// Convert normalized floating-point RGBA values to a raylib color.
static Color rfbxColorFromValues(double r, double g, double b, double a)
{
    Color result = {
        (unsigned char)(rfbxClamp01(r) * 255.0f + 0.5f),
        (unsigned char)(rfbxClamp01(g) * 255.0f + 0.5f),
        (unsigned char)(rfbxClamp01(b) * 255.0f + 0.5f),
        (unsigned char)(rfbxClamp01(a) * 255.0f + 0.5f)
    };
    return result;
}

// Resolve layered or shader textures to the first underlying file texture.
static const ufbx_texture* rfbxResolveFileTexture(const ufbx_texture* texture)
{
    if (!texture) return NULL;
    if (texture->type == UFBX_TEXTURE_FILE) return texture;
    if (texture->file_textures.count > 0) return texture->file_textures.data[0];
    if (texture->shader && texture->shader->main_texture) {
        return rfbxResolveFileTexture(texture->shader->main_texture);
    }
    return NULL;
}

// Guess the image decoder hint for embedded texture bytes.
static const char* rfbxGuessImageExtension(
    const char* fileName,
    const unsigned char* data,
    size_t size)
{
    const char* extension = fileName ? GetFileExtension(fileName) : "";
    if (extension && extension[0] != '\0') return extension;

    if (size >= 8 &&
        data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G') {
        return ".png";
    }
    if (size >= 3 && data[0] == 0xff && data[1] == 0xd8 && data[2] == 0xff) {
        return ".jpg";
    }
    if (size >= 2 && data[0] == 'B' && data[1] == 'M') return ".bmp";
    if (size >= 6 && memcmp(data, "GIF8", 4) == 0) return ".gif";
    if (size >= 4 && memcmp(data, "DDS ", 4) == 0) return ".dds";
    return ".png";
}

// Return embedded bytes from the texture, video, or deduplicated texture file.
static ufbx_blob rfbxGetTextureContent(
    const ufbx_scene* scene,
    const ufbx_texture* texture)
{
    ufbx_blob empty = { 0 };
    if (!texture) return empty;
    if (texture->content.size > 0) return texture->content;
    if (texture->video && texture->video->content.size > 0) {
        return texture->video->content;
    }
    if (texture->has_file && texture->file_index < scene->texture_files.count) {
        return scene->texture_files.data[texture->file_index].content;
    }
    return empty;
}

// Try to load a texture path and remember the successful path.
static bool rfbxTryLoadTexturePath(RfbxTexture* destination, const char* path)
{
    if (!path || path[0] == '\0' || !FileExists(path)) return false;

    Texture2D texture = LoadTexture(path);
    if (!IsTextureValid(texture)) return false;

    destination->fileName = rfbxCopyString((ufbx_string) { path, strlen(path) });
    if (!destination->fileName) {
        UnloadTexture(texture);
        return false;
    }

    destination->texture = texture;
    destination->ready = true;
    return true;
}

// Load one ufbx texture from embedded bytes or common external path variants.
static bool rfbxBuildTexture(
    RfbxTexture* destination,
    const ufbx_scene* scene,
    const ufbx_texture* source,
    const char* fbxFileName)
{
    destination->name = rfbxCopyString(source->name);
    if (!destination->name) return false;
    destination->sourceTypedId = source->typed_id;

    const ufbx_texture* fileTexture = rfbxResolveFileTexture(source);
    if (!fileTexture) return true;

    const ufbx_texture_file* textureFile = NULL;
    if (fileTexture->has_file &&
        fileTexture->file_index < scene->texture_files.count) {
        textureFile = &scene->texture_files.data[fileTexture->file_index];
    }

    const char* displayName = fileTexture->filename.data;
    if ((!displayName || displayName[0] == '\0') && textureFile) {
        displayName = textureFile->filename.data;
    }

    ufbx_blob content = rfbxGetTextureContent(scene, fileTexture);
    if (content.data && content.size > 0 && content.size <= INT_MAX) {
        const char* extension = rfbxGuessImageExtension(
            displayName,
            (const unsigned char*)content.data,
            content.size);
        Image image = LoadImageFromMemory(
            extension,
            (const unsigned char*)content.data,
            (int)content.size);
        if (IsImageValid(image)) {
            Texture2D texture = LoadTextureFromImage(image);
            UnloadImage(image);
            if (IsTextureValid(texture)) {
                destination->texture = texture;
                destination->ready = true;
                if (displayName && displayName[0] != '\0') {
                    destination->fileName = rfbxCopyString(
                        (ufbx_string) {
                        displayName, strlen(displayName)
                    });
                    if (!destination->fileName) return false;
                }
            }
        }
    }

    const char* directory = GetDirectoryPath(fbxFileName);
    if (!destination->ready) {
        const char* candidates[6] = {
            fileTexture->filename.data,
            fileTexture->absolute_filename.data,
            fileTexture->relative_filename.data,
            textureFile ? textureFile->filename.data : NULL,
            textureFile ? textureFile->absolute_filename.data : NULL,
            textureFile ? textureFile->relative_filename.data : NULL
        };

        for (int candidateIndex = 0;
            candidateIndex < 6 && !destination->ready;
            ++candidateIndex) {
            const char* candidate = candidates[candidateIndex];
            if (!candidate || candidate[0] == '\0') continue;
            if (rfbxTryLoadTexturePath(destination, candidate)) break;

            const char* joined = TextFormat("%s/%s", directory, candidate);
            rfbxTryLoadTexturePath(destination, joined);
        }
    }

    if (!destination->fileName && displayName && displayName[0] != '\0') {
        destination->fileName = rfbxCopyString(
            (ufbx_string) {
            displayName, strlen(displayName)
        });
        if (!destination->fileName) return false;
    }

    if (destination->ready) {
        SetTextureFilter(destination->texture, TEXTURE_FILTER_BILINEAR);
        int wrapMode = fileTexture->wrap_u == UFBX_WRAP_CLAMP ||
            fileTexture->wrap_v == UFBX_WRAP_CLAMP
            ? TEXTURE_WRAP_CLAMP
            : TEXTURE_WRAP_REPEAT;
        SetTextureWrap(destination->texture, wrapMode);
    }
    else {
        TraceLog(LOG_WARNING,
            "RFBX: texture not found or unsupported: %s",
            displayName && displayName[0] != '\0' ? displayName : source->name.data);
    }

    return true;
}

// Find the model texture index connected to one of two material maps.
static int rfbxGetMapTextureIndex(
    const ufbx_scene* scene,
    const ufbx_material_map* primary,
    const ufbx_material_map* fallback)
{
    const ufbx_texture* texture = NULL;
    if (primary && primary->texture_enabled && primary->texture) {
        texture = primary->texture;
    }
    else if (fallback && fallback->texture_enabled && fallback->texture) {
        texture = fallback->texture;
    }
    if (!texture) return RFBX_INVALID_INDEX;

    if (texture->typed_id < scene->textures.count &&
        scene->textures.data[texture->typed_id] == texture) {
        return (int)texture->typed_id;
    }

    for (size_t textureIndex = 0;
        textureIndex < scene->textures.count;
        ++textureIndex) {
        if (scene->textures.data[textureIndex] == texture) {
            return (int)textureIndex;
        }
    }
    return RFBX_INVALID_INDEX;
}

// Build textures and raylib materials shared by all model instances.
static bool rfbxBuildMaterials(
    RfbxModel* model,
    const ufbx_scene* scene,
    const char* fbxFileName)
{
    if (scene->textures.count > INT_MAX || scene->materials.count > INT_MAX) {
        return false;
    }

    model->textureCount = (int)scene->textures.count;
    if (model->textureCount > 0) {
        model->textures = rfbxAllocArray(
            scene->textures.count,
            sizeof(RfbxTexture));
        if (!model->textures) return false;
    }

    for (int textureIndex = 0;
        textureIndex < model->textureCount;
        ++textureIndex) {
        if (!rfbxBuildTexture(
            &model->textures[textureIndex],
            scene,
            scene->textures.data[textureIndex],
            fbxFileName)) {
            return false;
        }
    }

    model->materialCount = (int)scene->materials.count;
    if (model->materialCount > 0) {
        model->materials = rfbxAllocArray(
            scene->materials.count,
            sizeof(RfbxMaterial));
        model->internal->materialDefaults = rfbxAllocArray(
            scene->materials.count,
            sizeof(RfbxMaterialDefaultState));
        if (!model->materials || !model->internal->materialDefaults) {
            return false;
        }
    }

    for (int materialIndex = 0;
        materialIndex < model->materialCount;
        ++materialIndex) {
        const ufbx_material* source = scene->materials.data[materialIndex];
        RfbxMaterial* destination = &model->materials[materialIndex];

        destination->baseColorTextureIndex = RFBX_INVALID_INDEX;
        destination->normalTextureIndex = RFBX_INVALID_INDEX;
        destination->emissiveTextureIndex = RFBX_INVALID_INDEX;
        destination->metallicTextureIndex = RFBX_INVALID_INDEX;
        destination->roughnessTextureIndex = RFBX_INVALID_INDEX;
        destination->opacityTextureIndex = RFBX_INVALID_INDEX;

        destination->name = rfbxCopyString(source->name);
        if (!destination->name) return false;
        destination->sourceTypedId = source->typed_id;

        ufbx_vec4 baseColor = { 1.0, 1.0, 1.0, 1.0 };
        if (source->pbr.base_color.has_value) {
            baseColor = source->pbr.base_color.value_vec4;
        }
        else if (source->fbx.diffuse_color.has_value) {
            ufbx_vec3 diffuse = source->fbx.diffuse_color.value_vec3;
            baseColor.x = diffuse.x;
            baseColor.y = diffuse.y;
            baseColor.z = diffuse.z;
        }

        double baseFactor = source->pbr.base_factor.has_value
            ? source->pbr.base_factor.value_real
            : 1.0;
        double opacity = source->pbr.opacity.has_value
            ? source->pbr.opacity.value_real
            : 1.0;
        if (source->pbr.base_color.value_components >= 4) {
            opacity *= baseColor.w;
        }

        destination->opacity = rfbxClamp01(opacity);
        destination->baseColor = rfbxColorFromValues(
            baseColor.x * baseFactor,
            baseColor.y * baseFactor,
            baseColor.z * baseFactor,
            destination->opacity);
        destination->metallic = source->pbr.metalness.has_value
            ? rfbxClamp01(source->pbr.metalness.value_real)
            : 0.0f;
        if (source->pbr.roughness.has_value) {
            destination->roughness =
                rfbxClamp01(source->pbr.roughness.value_real);
        }
        else if (source->pbr.glossiness.has_value) {
            destination->roughness =
                1.0f - rfbxClamp01(source->pbr.glossiness.value_real);
        }
        else {
            destination->roughness = 1.0f;
        }

        ufbx_vec3 emission = { 0.0, 0.0, 0.0 };
        if (source->pbr.emission_color.has_value) {
            emission = source->pbr.emission_color.value_vec3;
        }
        else if (source->fbx.emission_color.has_value) {
            emission = source->fbx.emission_color.value_vec3;
        }
        double emissionFactor = source->pbr.emission_factor.has_value
            ? source->pbr.emission_factor.value_real
            : (source->fbx.emission_factor.has_value
                ? source->fbx.emission_factor.value_real
                : 1.0);
        destination->emissiveColor = rfbxColorFromValues(
            emission.x * emissionFactor,
            emission.y * emissionFactor,
            emission.z * emissionFactor,
            1.0);

        destination->baseColorTextureIndex = rfbxGetMapTextureIndex(
            scene,
            &source->pbr.base_color,
            &source->fbx.diffuse_color);
        destination->normalTextureIndex = rfbxGetMapTextureIndex(
            scene,
            &source->pbr.normal_map,
            &source->fbx.normal_map);
        if (destination->normalTextureIndex == RFBX_INVALID_INDEX) {
            destination->normalTextureIndex = rfbxGetMapTextureIndex(
                scene,
                &source->fbx.bump,
                NULL);
        }
        destination->emissiveTextureIndex = rfbxGetMapTextureIndex(
            scene,
            &source->pbr.emission_color,
            &source->pbr.emission_factor);
        if (destination->emissiveTextureIndex == RFBX_INVALID_INDEX) {
            destination->emissiveTextureIndex = rfbxGetMapTextureIndex(
                scene,
                &source->fbx.emission_color,
                &source->fbx.emission_factor);
        }
        destination->metallicTextureIndex = rfbxGetMapTextureIndex(
            scene,
            &source->pbr.metalness,
            NULL);
        destination->roughnessTextureIndex = rfbxGetMapTextureIndex(
            scene,
            &source->pbr.roughness,
            NULL);
        destination->roughnessTextureIsGlossiness =
            destination->roughnessTextureIndex != RFBX_INVALID_INDEX &&
            source->features.roughness_as_glossiness.enabled;
        if (destination->roughnessTextureIndex == RFBX_INVALID_INDEX) {
            destination->roughnessTextureIndex = rfbxGetMapTextureIndex(
                scene,
                &source->pbr.glossiness,
                NULL);
            destination->roughnessTextureIsGlossiness =
                destination->roughnessTextureIndex != RFBX_INVALID_INDEX;
        }
        destination->opacityTextureIndex = rfbxGetMapTextureIndex(
            scene,
            &source->pbr.opacity,
            NULL);
        destination->alphaCutoff = 0.5f;
        destination->alphaMode =
            destination->opacity < 0.999f ||
            destination->opacityTextureIndex != RFBX_INVALID_INDEX
            ? RFBX_ALPHA_BLEND
            : RFBX_ALPHA_OPAQUE;
        destination->doubleSided = source->features.double_sided.enabled;

        destination->material = LoadMaterialDefault();
        destination->materialReady = true;
        destination->material.maps[MATERIAL_MAP_ALBEDO].color =
            destination->baseColor;
        destination->material.maps[MATERIAL_MAP_METALNESS].value =
            destination->metallic;
        destination->material.maps[MATERIAL_MAP_ROUGHNESS].value =
            destination->roughness;
        destination->material.maps[MATERIAL_MAP_EMISSION].color =
            destination->emissiveColor;

        int mapTextureIndices[6] = {
            destination->baseColorTextureIndex,
            destination->normalTextureIndex,
            destination->emissiveTextureIndex,
            destination->metallicTextureIndex,
            destination->roughnessTextureIndex,
            destination->opacityTextureIndex
        };
        int raylibMapIndices[6] = {
            MATERIAL_MAP_ALBEDO,
            MATERIAL_MAP_NORMAL,
            MATERIAL_MAP_EMISSION,
            MATERIAL_MAP_METALNESS,
            MATERIAL_MAP_ROUGHNESS,
            MATERIAL_MAP_HEIGHT
        };

        for (int mapIndex = 0; mapIndex < 6; ++mapIndex) {
            int textureIndex = mapTextureIndices[mapIndex];
            if (textureIndex >= 0 &&
                textureIndex < model->textureCount &&
                model->textures[textureIndex].ready) {
                destination->material.maps[raylibMapIndices[mapIndex]].texture =
                    model->textures[textureIndex].texture;
            }
        }

        RfbxMaterialDefaultState* defaults =
            &model->internal->materialDefaults[materialIndex];
        defaults->textureIndices[RFBX_MATERIAL_MAP_BASE_COLOR] =
            destination->baseColorTextureIndex;
        defaults->textureIndices[RFBX_MATERIAL_MAP_NORMAL] =
            destination->normalTextureIndex;
        defaults->textureIndices[RFBX_MATERIAL_MAP_METALLIC] =
            destination->metallicTextureIndex;
        defaults->textureIndices[RFBX_MATERIAL_MAP_ROUGHNESS] =
            destination->roughnessTextureIndex;
        defaults->textureIndices[RFBX_MATERIAL_MAP_EMISSION] =
            destination->emissiveTextureIndex;
        defaults->textureIndices[RFBX_MATERIAL_MAP_OPACITY] =
            destination->opacityTextureIndex;
        defaults->textures[RFBX_MATERIAL_MAP_BASE_COLOR] =
            destination->material.maps[MATERIAL_MAP_ALBEDO].texture;
        defaults->textures[RFBX_MATERIAL_MAP_NORMAL] =
            destination->material.maps[MATERIAL_MAP_NORMAL].texture;
        defaults->textures[RFBX_MATERIAL_MAP_METALLIC] =
            destination->material.maps[MATERIAL_MAP_METALNESS].texture;
        defaults->textures[RFBX_MATERIAL_MAP_ROUGHNESS] =
            destination->material.maps[MATERIAL_MAP_ROUGHNESS].texture;
        defaults->textures[RFBX_MATERIAL_MAP_EMISSION] =
            destination->material.maps[MATERIAL_MAP_EMISSION].texture;
        defaults->textures[RFBX_MATERIAL_MAP_OPACITY] =
            destination->material.maps[MATERIAL_MAP_HEIGHT].texture;
        defaults->baseColor = destination->baseColor;
        defaults->emissiveColor = destination->emissiveColor;
        defaults->metallic = destination->metallic;
        defaults->roughness = destination->roughness;
        defaults->opacity = destination->opacity;
        defaults->alphaCutoff = destination->alphaCutoff;
        defaults->alphaMode = destination->alphaMode;
        defaults->doubleSided = destination->doubleSided;
        defaults->roughnessTextureIsGlossiness =
            destination->roughnessTextureIsGlossiness;
    }

    return true;
}

// Find the model mesh index for a ufbx mesh pointer.
static int rfbxFindMeshIndex(const ufbx_scene* scene, const ufbx_mesh* mesh)
{
    if (!mesh) return RFBX_INVALID_INDEX;

    if (mesh->typed_id < scene->meshes.count &&
        scene->meshes.data[mesh->typed_id] == mesh) {
        return (int)mesh->typed_id;
    }

    for (size_t i = 0; i < scene->meshes.count; ++i) {
        if (scene->meshes.data[i] == mesh) return (int)i;
    }

    return RFBX_INVALID_INDEX;
}

// Find the model node index for a ufbx node pointer.
static int rfbxFindNodeIndex(const ufbx_scene* scene, const ufbx_node* node)
{
    if (!node) return RFBX_INVALID_INDEX;

    if (node->typed_id < scene->nodes.count &&
        scene->nodes.data[node->typed_id] == node) {
        return (int)node->typed_id;
    }

    for (size_t i = 0; i < scene->nodes.count; ++i) {
        if (scene->nodes.data[i] == node) return (int)i;
    }

    return RFBX_INVALID_INDEX;
}

// Release a partially or fully built skeleton.
static void rfbxUnloadSkeleton(RfbxSkeleton* skeleton)
{
    if (!skeleton) return;

    if (skeleton->bones) {
        for (int boneIndex = 0;
            boneIndex < skeleton->boneCount;
            ++boneIndex) {
            MemFree(skeleton->bones[boneIndex].name);
        }
        MemFree(skeleton->bones);
    }

    memset(skeleton, 0, sizeof(*skeleton));
}

// Collect bone nodes and skin-cluster nodes into a stable hierarchy.
static bool rfbxBuildSkeleton(
    RfbxSkeleton* destination,
    const ufbx_scene* scene)
{
    if (!destination || !scene) return false;
    if (scene->nodes.count == 0) return true;

    bool* isBoneNode = rfbxAllocArray(scene->nodes.count, sizeof(bool));
    bool* isSkinBone = rfbxAllocArray(scene->nodes.count, sizeof(bool));
    int* nodeToBone = rfbxAllocArray(scene->nodes.count, sizeof(int));
    if (!isBoneNode || !isSkinBone || !nodeToBone) {
        MemFree(isBoneNode);
        MemFree(isSkinBone);
        MemFree(nodeToBone);
        return false;
    }

    for (size_t nodeIndex = 0; nodeIndex < scene->nodes.count; ++nodeIndex) {
        nodeToBone[nodeIndex] = RFBX_INVALID_INDEX;
        if (scene->nodes.data[nodeIndex]->bone) isBoneNode[nodeIndex] = true;
    }

    for (size_t clusterIndex = 0;
        clusterIndex < scene->skin_clusters.count;
        ++clusterIndex) {
        const ufbx_skin_cluster* cluster = scene->skin_clusters.data[clusterIndex];
        int nodeIndex = rfbxFindNodeIndex(scene, cluster->bone_node);
        if (nodeIndex == RFBX_INVALID_INDEX) continue;
        isBoneNode[nodeIndex] = true;
        isSkinBone[nodeIndex] = true;
    }

    int boneCount = 0;
    for (size_t nodeIndex = 0; nodeIndex < scene->nodes.count; ++nodeIndex) {
        if (isBoneNode[nodeIndex]) ++boneCount;
    }

    destination->boneCount = boneCount;
    if (boneCount > 0) {
        destination->bones = rfbxAllocArray(
            (size_t)boneCount,
            sizeof(RfbxBone));
        if (!destination->bones) goto build_failed;
    }

    int destinationIndex = 0;
    for (size_t nodeIndex = 0; nodeIndex < scene->nodes.count; ++nodeIndex) {
        if (!isBoneNode[nodeIndex]) continue;

        const ufbx_node* source = scene->nodes.data[nodeIndex];
        RfbxBone* bone = &destination->bones[destinationIndex];

        bone->name = rfbxCopyString(source->name);
        if (!bone->name) goto build_failed;

        bone->sourceNodeTypedId = source->typed_id;
        bone->parentIndex = RFBX_INVALID_INDEX;
        bone->isSkinBone = isSkinBone[nodeIndex];
        bone->radius = source->bone ? (float)source->bone->radius : 0.0f;
        bone->bindToWorld = rfbxMatrixFromUfbx(source->node_to_world);

        for (size_t clusterIndex = 0;
            clusterIndex < scene->skin_clusters.count;
            ++clusterIndex) {
            const ufbx_skin_cluster* cluster =
                scene->skin_clusters.data[clusterIndex];
            if (cluster->bone_node == source) {
                bone->bindToWorld = rfbxMatrixFromUfbx(cluster->bind_to_world);
                break;
            }
        }

        nodeToBone[nodeIndex] = destinationIndex;
        ++destinationIndex;
    }

    for (int boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
        RfbxBone* bone = &destination->bones[boneIndex];
        const ufbx_node* node = scene->nodes.data[bone->sourceNodeTypedId];

        for (const ufbx_node* parent = node->parent;
            parent;
            parent = parent->parent) {
            int parentNodeIndex = rfbxFindNodeIndex(scene, parent);
            if (parentNodeIndex != RFBX_INVALID_INDEX &&
                nodeToBone[parentNodeIndex] != RFBX_INVALID_INDEX) {
                bone->parentIndex = nodeToBone[parentNodeIndex];
                break;
            }
        }
    }

    MemFree(isBoneNode);
    MemFree(isSkinBone);
    MemFree(nodeToBone);
    return true;

build_failed:
    MemFree(isBoneNode);
    MemFree(isSkinBone);
    MemFree(nodeToBone);
    rfbxUnloadSkeleton(destination);
    return false;
}

// Generate per-triangle tangent frames for a non-indexed raylib mesh.
static bool rfbxGenerateMeshTangents(Mesh* mesh)
{
    if (!mesh ||
        mesh->vertexCount <= 0 ||
        !mesh->vertices ||
        !mesh->normals ||
        !mesh->texcoords) {
        return false;
    }

    size_t vertexCount = (size_t)mesh->vertexCount;
    if (!mesh->tangents) {
        mesh->tangents = rfbxAllocArray(vertexCount * 4u, sizeof(float));
        if (!mesh->tangents) return false;
    }

    for (size_t triangle = 0; triangle + 2u < vertexCount; triangle += 3u) {
        size_t i0 = triangle;
        size_t i1 = triangle + 1u;
        size_t i2 = triangle + 2u;

        Vector3 p0 = {
            mesh->vertices[i0 * 3u + 0u],
            mesh->vertices[i0 * 3u + 1u],
            mesh->vertices[i0 * 3u + 2u]
        };
        Vector3 p1 = {
            mesh->vertices[i1 * 3u + 0u],
            mesh->vertices[i1 * 3u + 1u],
            mesh->vertices[i1 * 3u + 2u]
        };
        Vector3 p2 = {
            mesh->vertices[i2 * 3u + 0u],
            mesh->vertices[i2 * 3u + 1u],
            mesh->vertices[i2 * 3u + 2u]
        };
        Vector2 uv0 = {
            mesh->texcoords[i0 * 2u + 0u],
            mesh->texcoords[i0 * 2u + 1u]
        };
        Vector2 uv1 = {
            mesh->texcoords[i1 * 2u + 0u],
            mesh->texcoords[i1 * 2u + 1u]
        };
        Vector2 uv2 = {
            mesh->texcoords[i2 * 2u + 0u],
            mesh->texcoords[i2 * 2u + 1u]
        };

        Vector3 edge1 = Vector3Subtract(p1, p0);
        Vector3 edge2 = Vector3Subtract(p2, p0);
        float du1 = uv1.x - uv0.x;
        float dv1 = uv1.y - uv0.y;
        float du2 = uv2.x - uv0.x;
        float dv2 = uv2.y - uv0.y;
        float determinant = du1 * dv2 - du2 * dv1;

        Vector3 tangent = { 1.0f, 0.0f, 0.0f };
        Vector3 bitangent = { 0.0f, 0.0f, 1.0f };
        if (fabsf(determinant) > 0.0000001f) {
            float reciprocal = 1.0f / determinant;
            tangent = Vector3Scale(
                Vector3Subtract(
                    Vector3Scale(edge1, dv2),
                    Vector3Scale(edge2, dv1)),
                reciprocal);
            bitangent = Vector3Scale(
                Vector3Subtract(
                    Vector3Scale(edge2, du1),
                    Vector3Scale(edge1, du2)),
                reciprocal);
        }

        for (size_t localVertex = 0; localVertex < 3u; ++localVertex) {
            size_t vertexIndex = triangle + localVertex;
            Vector3 normal = {
                mesh->normals[vertexIndex * 3u + 0u],
                mesh->normals[vertexIndex * 3u + 1u],
                mesh->normals[vertexIndex * 3u + 2u]
            };
            normal = Vector3Normalize(normal);

            Vector3 orthogonalTangent = Vector3Subtract(
                tangent,
                Vector3Scale(normal, Vector3DotProduct(normal, tangent)));
            if (Vector3LengthSqr(orthogonalTangent) < 0.0000001f) {
                Vector3 axis = fabsf(normal.y) < 0.999f
                    ? (Vector3) { 0.0f, 1.0f, 0.0f }
                : (Vector3) { 1.0f, 0.0f, 0.0f };
                orthogonalTangent = Vector3CrossProduct(axis, normal);
            }
            orthogonalTangent = Vector3Normalize(orthogonalTangent);
            float handedness = Vector3DotProduct(
                Vector3CrossProduct(normal, orthogonalTangent),
                bitangent) < 0.0f ? -1.0f : 1.0f;

            mesh->tangents[vertexIndex * 4u + 0u] = orthogonalTangent.x;
            mesh->tangents[vertexIndex * 4u + 1u] = orthogonalTangent.y;
            mesh->tangents[vertexIndex * 4u + 2u] = orthogonalTangent.z;
            mesh->tangents[vertexIndex * 4u + 3u] = handedness;
        }
    }

    return true;
}

// Check whether a ufbx skin can be represented by the built-in linear shader.
static bool rfbxCanGpuSkinMesh(const ufbx_mesh* mesh)
{
#if defined(RFBX_DISABLE_GPU_SKINNING)
    (void)mesh;
    return false;
#else
    if (!mesh ||
        mesh->skin_deformers.count != 1 ||
        mesh->blend_deformers.count > 0 ||
        mesh->cache_deformers.count > 0) {
        return false;
    }

    const ufbx_skin_deformer* skin = mesh->skin_deformers.data[0];
    if (!skin ||
        (skin->skinning_method != UFBX_SKINNING_METHOD_LINEAR &&
            skin->skinning_method != UFBX_SKINNING_METHOD_RIGID) ||
        skin->clusters.count == 0 ||
        skin->clusters.count > RFBX_MAX_GPU_BONES ||
        skin->clusters.count > 256u ||
        skin->vertices.count < mesh->num_vertices) {
        return false;
    }

    // Every rendered vertex needs at least one valid positive influence.
    for (size_t index = 0; index < mesh->vertex_indices.count; ++index) {
        uint32_t vertexIndex = mesh->vertex_indices.data[index];
        if ((size_t)vertexIndex >= skin->vertices.count) return false;

        ufbx_skin_vertex vertex = skin->vertices.data[vertexIndex];
        size_t begin = vertex.weight_begin;
        size_t count = vertex.num_weights;
        if (begin > skin->weights.count ||
            count > skin->weights.count - begin) {
            return false;
        }

        bool hasWeight = false;
        for (size_t weightIndex = 0; weightIndex < count; ++weightIndex) {
            ufbx_skin_weight weight = skin->weights.data[begin + weightIndex];
            if (weight.cluster_index >= skin->clusters.count) return false;
            if (!isfinite((double)weight.weight)) return false;
            if (weight.weight > 0.0) hasWeight = true;
        }
        if (!hasWeight) return false;
    }
    return true;
#endif
}

// Upload four normalized bone influences without depending on raylib build flags.
static bool rfbxBuildGpuSkinAttributes(
    RfbxPrimitive* primitive,
    const ufbx_mesh* mesh,
    const ufbx_skin_deformer* skin)
{
    if (!primitive || !mesh || !skin || primitive->mesh.vertexCount <= 0) {
        return false;
    }

    size_t vertexCount = (size_t)primitive->mesh.vertexCount;
    if (vertexCount > (size_t)INT_MAX / (4u * sizeof(float))) return false;

    unsigned char* boneIndices = rfbxAllocArray(
        vertexCount * 4u,
        sizeof(unsigned char));
    float* boneWeights = rfbxAllocArray(vertexCount * 4u, sizeof(float));
    if (!boneIndices || !boneWeights) {
        MemFree(boneIndices);
        MemFree(boneWeights);
        return false;
    }

    for (size_t vertex = 0; vertex < vertexCount; ++vertex) {
        uint32_t sourceIndex = primitive->sourceIndices[vertex];
        if ((size_t)sourceIndex >= mesh->vertex_indices.count) goto failed;
        uint32_t logicalIndex = mesh->vertex_indices.data[sourceIndex];
        if ((size_t)logicalIndex >= skin->vertices.count) goto failed;

        ufbx_skin_vertex skinVertex = skin->vertices.data[logicalIndex];
        size_t begin = skinVertex.weight_begin;
        size_t count = skinVertex.num_weights;
        if (begin > skin->weights.count ||
            count > skin->weights.count - begin) {
            goto failed;
        }

        int stored = 0;
        float total = 0.0f;
        for (size_t influence = 0;
            influence < count && stored < 4;
            ++influence) {
            ufbx_skin_weight weight = skin->weights.data[begin + influence];
            if (weight.weight <= 0.0) continue;
            if (weight.cluster_index >= skin->clusters.count ||
                weight.cluster_index > 255u) {
                goto failed;
            }
            boneIndices[vertex * 4u + (size_t)stored] =
                (unsigned char)weight.cluster_index;
            boneWeights[vertex * 4u + (size_t)stored] = (float)weight.weight;
            total += (float)weight.weight;
            ++stored;
        }
        if (stored == 0 || !isfinite(total) || total <= 0.0f) goto failed;
        for (int influence = 0; influence < stored; ++influence) {
            boneWeights[vertex * 4u + (size_t)influence] /= total;
        }
    }

    if (!rlEnableVertexArray(primitive->mesh.vaoId)) goto failed;
    unsigned int indexVbo = rlLoadVertexBuffer(
        boneIndices,
        (int)(vertexCount * 4u * sizeof(unsigned char)),
        false);
    if (indexVbo == 0) {
        rlDisableVertexArray();
        goto failed;
    }
    rlSetVertexAttribute(
        RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES,
        4,
        RL_UNSIGNED_BYTE,
        false,
        0,
        0);
    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEINDICES);

    unsigned int weightVbo = rlLoadVertexBuffer(
        boneWeights,
        (int)(vertexCount * 4u * sizeof(float)),
        false);
    if (weightVbo == 0) {
        rlUnloadVertexBuffer(indexVbo);
        rlDisableVertexArray();
        goto failed;
    }
    rlSetVertexAttribute(
        RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS,
        4,
        RL_FLOAT,
        false,
        0,
        0);
    rlEnableVertexAttribute(RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS);
    rlDisableVertexBuffer();
    rlDisableVertexArray();

    primitive->boneIndices = boneIndices;
    primitive->boneWeights = boneWeights;
    primitive->boneIndicesVboId = indexVbo;
    primitive->boneWeightsVboId = weightVbo;
    return true;

failed:
    MemFree(boneIndices);
    MemFree(boneWeights);
    return false;
}

// Convert one ufbx mesh to a non-indexed raylib primitive.
static bool rfbxBuildPrimitive(
    RfbxPrimitive* destination,
    const ufbx_mesh* source,
    const ufbx_mesh_part* part,
    int materialSlot,
    bool dynamic,
    const ufbx_skin_deformer* gpuSkin)
{
    size_t triangleTotal = part ? part->num_triangles : source->num_triangles;
    if (triangleTotal == 0) return true;
    if (triangleTotal > (size_t)INT_MAX / 3u) return false;

    size_t vertexCount = triangleTotal * 3u;
    size_t maxFaceIndexCount = source->max_face_triangles * 3u;

    float* positions = rfbxAllocArray(vertexCount * 3u, sizeof(float));
    float* normals = rfbxAllocArray(vertexCount * 3u, sizeof(float));
    float* texcoords = rfbxAllocArray(vertexCount * 2u, sizeof(float));
    uint32_t* sourceIndices = rfbxAllocArray(vertexCount, sizeof(uint32_t));
    uint32_t* triangleIndices = rfbxAllocArray(maxFaceIndexCount, sizeof(uint32_t));

    if (!positions || !normals || !texcoords || !sourceIndices || !triangleIndices) {
        MemFree(positions);
        MemFree(normals);
        MemFree(texcoords);
        MemFree(sourceIndices);
        MemFree(triangleIndices);
        return false;
    }

    size_t destinationVertex = 0;

    size_t faceCount = part ? part->face_indices.count : source->faces.count;
    for (size_t partFaceIndex = 0; partFaceIndex < faceCount; ++partFaceIndex) {
        size_t faceIndex = part
            ? (size_t)part->face_indices.data[partFaceIndex]
            : partFaceIndex;
        if (faceIndex >= source->faces.count) {
            MemFree(positions);
            MemFree(normals);
            MemFree(texcoords);
            MemFree(sourceIndices);
            MemFree(triangleIndices);
            return false;
        }
        ufbx_face face = source->faces.data[faceIndex];
        size_t triangleCount = ufbx_triangulate_face(
            triangleIndices,
            maxFaceIndexCount,
            source,
            face);

        for (size_t triangleVertex = 0; triangleVertex < triangleCount * 3u; ++triangleVertex) {
            uint32_t sourceIndex = triangleIndices[triangleVertex];
            ufbx_vec3 position = ufbx_get_vertex_vec3(
                &source->vertex_position,
                sourceIndex);

            ufbx_vec3 normal = { 0.0, 1.0, 0.0 };
            if (source->vertex_normal.exists) {
                normal = ufbx_get_vertex_vec3(
                    &source->vertex_normal,
                    sourceIndex);
            }

            ufbx_vec2 uv = { 0.0, 0.0 };
            if (source->vertex_uv.exists) {
                uv = ufbx_get_vertex_vec2(
                    &source->vertex_uv,
                    sourceIndex);
            }

            positions[destinationVertex * 3u + 0u] = (float)position.x;
            positions[destinationVertex * 3u + 1u] = (float)position.y;
            positions[destinationVertex * 3u + 2u] = (float)position.z;

            normals[destinationVertex * 3u + 0u] = (float)normal.x;
            normals[destinationVertex * 3u + 1u] = (float)normal.y;
            normals[destinationVertex * 3u + 2u] = (float)normal.z;

            texcoords[destinationVertex * 2u + 0u] = (float)uv.x;
            texcoords[destinationVertex * 2u + 1u] = 1.0f - (float)uv.y;

            sourceIndices[destinationVertex] = sourceIndex;
            ++destinationVertex;
        }
    }

    MemFree(triangleIndices);

    if (destinationVertex != vertexCount) {
        MemFree(positions);
        MemFree(normals);
        MemFree(texcoords);
        MemFree(sourceIndices);
        return false;
    }

    destination->mesh.vertexCount = (int)vertexCount;
    destination->mesh.triangleCount = (int)triangleTotal;
    destination->mesh.vertices = positions;
    destination->mesh.normals = normals;
    destination->mesh.texcoords = texcoords;
    destination->materialSlot = materialSlot;
    destination->sourceIndices = sourceIndices;

    if (!rfbxGenerateMeshTangents(&destination->mesh)) return false;
    UploadMesh(&destination->mesh, dynamic);
    if (gpuSkin &&
        !rfbxBuildGpuSkinAttributes(destination, source, gpuSkin)) {
        return false;
    }
    return true;
}

// Cache bind/local bounds shared by every node that references this mesh.
static void rfbxUpdateMeshLocalBounds(RfbxMesh* mesh)
{
    if (!mesh) return;
    mesh->localBounds = (BoundingBox){ 0 };
    mesh->boundsValid = false;
    for (int primitiveIndex = 0;
        primitiveIndex < mesh->primitiveCount;
        ++primitiveIndex) {
        BoundingBox primitiveBounds = { 0 };
        if (rfbxCalculateMeshBounds(
            &mesh->primitives[primitiveIndex].mesh,
            &primitiveBounds)) {
            rfbxBoundsIncludeBox(
                &mesh->localBounds,
                &mesh->boundsValid,
                primitiveBounds);
        }
    }
}

// Convert a unique ufbx mesh resource.
static bool rfbxBuildMesh(RfbxMesh* destination, const ufbx_mesh* source)
{
    destination->name = rfbxCopyString(source->name);
    if (!destination->name) return false;

    destination->sourceTypedId = source->typed_id;
    destination->hasSkinning = source->skin_deformers.count > 0;
    destination->gpuSkinningReady = rfbxCanGpuSkinMesh(source);
    destination->gpuBoneCount = destination->gpuSkinningReady
        ? (int)source->skin_deformers.data[0]->clusters.count
        : 0;
    destination->verticesAreLocal = true;
    const ufbx_skin_deformer* gpuSkin = destination->gpuSkinningReady
        ? source->skin_deformers.data[0]
        : NULL;

    if (source->materials.count > INT_MAX) return false;
    destination->materialCount = (int)source->materials.count;
    if (destination->materialCount > 0) {
        destination->materialIndices = rfbxAllocArray(
            source->materials.count,
            sizeof(int));
        if (!destination->materialIndices) return false;

        for (int materialIndex = 0;
            materialIndex < destination->materialCount;
            ++materialIndex) {
            destination->materialIndices[materialIndex] =
                (int)source->materials.data[materialIndex]->typed_id;
        }
    }

    if (source->num_triangles == 0) return true;

    if (source->material_parts.count > INT_MAX) return false;

    int primitiveCount = 0;
    for (size_t partIndex = 0;
        partIndex < source->material_parts.count;
        ++partIndex) {
        if (source->material_parts.data[partIndex].num_triangles > 0) {
            ++primitiveCount;
        }
    }
    if (primitiveCount == 0) primitiveCount = 1;

    destination->primitives = rfbxAllocArray(
        (size_t)primitiveCount,
        sizeof(RfbxPrimitive));
    if (!destination->primitives) return false;

    destination->primitiveCount = primitiveCount;
    int destinationPrimitive = 0;

    for (size_t partIndex = 0;
        partIndex < source->material_parts.count;
        ++partIndex) {
        const ufbx_mesh_part* part = &source->material_parts.data[partIndex];
        if (part->num_triangles == 0) continue;
        if (part->index > INT_MAX) return false;

        if (!rfbxBuildPrimitive(
            &destination->primitives[destinationPrimitive],
            source,
            part,
            (int)part->index,
            destination->hasSkinning,
            gpuSkin)) {
            return false;
        }
        ++destinationPrimitive;
    }

    if (destinationPrimitive == 0) {
        bool success = rfbxBuildPrimitive(
            &destination->primitives[0],
            source,
            NULL,
            0,
            destination->hasSkinning,
            gpuSkin);
        if (success) rfbxUpdateMeshLocalBounds(destination);
        return success;
    }

    bool success = destinationPrimitive == destination->primitiveCount;
    if (success) rfbxUpdateMeshLocalBounds(destination);
    return success;
}

// Release a partially or fully built primitive.
static void rfbxUnloadPrimitive(RfbxPrimitive* primitive)
{
    if (!primitive) return;

    if (primitive->boneIndicesVboId != 0) {
        rlUnloadVertexBuffer(primitive->boneIndicesVboId);
    }
    if (primitive->boneWeightsVboId != 0) {
        rlUnloadVertexBuffer(primitive->boneWeightsVboId);
    }
    if (primitive->mesh.vertices || primitive->mesh.vboId) {
        UnloadMesh(primitive->mesh);
    }

    MemFree(primitive->boneIndices);
    MemFree(primitive->boneWeights);
    MemFree(primitive->sourceIndices);
    memset(primitive, 0, sizeof(*primitive));
}

#define RFBX_STRINGIFY_INNER(value) #value
#define RFBX_STRINGIFY(value) RFBX_STRINGIFY_INNER(value)

// Create the built-in direct-light PBR, material, and skinning shader.
static Shader rfbxLoadBuiltinShader(void)
{
    static const char* vertexShader =
        "#version 330\n"
        "in vec3 vertexPosition;\n"
        "in vec2 vertexTexCoord;\n"
        "in vec3 vertexNormal;\n"
        "in vec4 vertexTangent;\n"
        "in vec4 " RFBX_SHADER_ATTRIB_BONE_INDICES ";\n"
        "in vec4 " RFBX_SHADER_ATTRIB_BONE_WEIGHTS ";\n"
        "uniform mat4 mvp;\n"
        "uniform mat4 matModel;\n"
        "uniform mat4 matNormal;\n"
        "uniform mat4 " RFBX_SHADER_UNIFORM_BONE_MATRICES
        "[" RFBX_STRINGIFY(RFBX_MAX_GPU_BONES) "];\n"
        "uniform int " RFBX_SHADER_UNIFORM_USE_SKINNING ";\n"
        "out vec2 fragTexCoord;\n"
        "out vec3 fragPosition;\n"
        "out mat3 fragTbn;\n"
        "void main()\n"
        "{\n"
        "    vec4 localPosition = vec4(vertexPosition, 1.0);\n"
        "    vec3 localNormal = vertexNormal;\n"
        "    vec3 localTangent = vertexTangent.xyz;\n"
        "    if (" RFBX_SHADER_UNIFORM_USE_SKINNING " != 0)\n"
        "    {\n"
        "        mat4 skin = " RFBX_SHADER_UNIFORM_BONE_MATRICES
        "[int(" RFBX_SHADER_ATTRIB_BONE_INDICES ".x + 0.5)] * "
        RFBX_SHADER_ATTRIB_BONE_WEIGHTS ".x;\n"
        "        skin += " RFBX_SHADER_UNIFORM_BONE_MATRICES
        "[int(" RFBX_SHADER_ATTRIB_BONE_INDICES ".y + 0.5)] * "
        RFBX_SHADER_ATTRIB_BONE_WEIGHTS ".y;\n"
        "        skin += " RFBX_SHADER_UNIFORM_BONE_MATRICES
        "[int(" RFBX_SHADER_ATTRIB_BONE_INDICES ".z + 0.5)] * "
        RFBX_SHADER_ATTRIB_BONE_WEIGHTS ".z;\n"
        "        skin += " RFBX_SHADER_UNIFORM_BONE_MATRICES
        "[int(" RFBX_SHADER_ATTRIB_BONE_INDICES ".w + 0.5)] * "
        RFBX_SHADER_ATTRIB_BONE_WEIGHTS ".w;\n"
        "        mat3 skinLinear = mat3(skin);\n"
        "        localPosition = skin * localPosition;\n"
        "        localNormal = transpose(inverse(skinLinear)) * localNormal;\n"
        "        localTangent = skinLinear * localTangent;\n"
        "    }\n"
        "    vec3 normal = normalize(mat3(matNormal) * localNormal);\n"
        "    vec3 tangent = normalize(mat3(matModel) * localTangent);\n"
        "    tangent = normalize(tangent - normal * dot(normal, tangent));\n"
        "    vec3 bitangent = cross(normal, tangent) * vertexTangent.w;\n"
        "    fragTexCoord = vertexTexCoord;\n"
        "    fragPosition = vec3(matModel * localPosition);\n"
        "    fragTbn = mat3(tangent, bitangent, normal);\n"
        "    gl_Position = mvp * localPosition;\n"
        "}\n";
    static const char* fragmentShader =
        "#version 330\n"
        "in vec2 fragTexCoord;\n"
        "in vec3 fragPosition;\n"
        "in mat3 fragTbn;\n"
        "uniform sampler2D texture0;\n"
        "uniform sampler2D texture1;\n"
        "uniform sampler2D texture2;\n"
        "uniform sampler2D texture3;\n"
        "uniform sampler2D texture5;\n"
        "uniform sampler2D texture6;\n"
        "uniform vec4 colDiffuse;\n"
        "uniform int " RFBX_SHADER_UNIFORM_HAS_NORMAL_MAP ";\n"
        "uniform int " RFBX_SHADER_UNIFORM_HAS_METALLIC_MAP ";\n"
        "uniform int " RFBX_SHADER_UNIFORM_HAS_ROUGHNESS_MAP ";\n"
        "uniform int " RFBX_SHADER_UNIFORM_HAS_EMISSION_MAP ";\n"
        "uniform int " RFBX_SHADER_UNIFORM_HAS_OPACITY_MAP ";\n"
        "uniform float " RFBX_SHADER_UNIFORM_METALLIC_FACTOR ";\n"
        "uniform float " RFBX_SHADER_UNIFORM_ROUGHNESS_FACTOR ";\n"
        "uniform int " RFBX_SHADER_UNIFORM_ROUGHNESS_MAP_INVERTED ";\n"
        "uniform vec4 " RFBX_SHADER_UNIFORM_EMISSION_COLOR ";\n"
        "uniform int " RFBX_SHADER_UNIFORM_ALPHA_MODE ";\n"
        "uniform float " RFBX_SHADER_UNIFORM_ALPHA_CUTOFF ";\n"
        "uniform int " RFBX_SHADER_UNIFORM_DOUBLE_SIDED ";\n"
        "uniform vec3 " RFBX_SHADER_UNIFORM_LIGHT_DIRECTION ";\n"
        "uniform vec4 " RFBX_SHADER_UNIFORM_LIGHT_COLOR ";\n"
        "uniform float " RFBX_SHADER_UNIFORM_LIGHT_INTENSITY ";\n"
        "uniform vec4 " RFBX_SHADER_UNIFORM_AMBIENT_COLOR ";\n"
        "uniform float " RFBX_SHADER_UNIFORM_AMBIENT_INTENSITY ";\n"
        "uniform vec3 " RFBX_SHADER_UNIFORM_VIEW_POSITION ";\n"
        "out vec4 finalColor;\n"
        "const float PI = 3.14159265359;\n"
        "vec3 srgbToLinear(vec3 value)\n"
        "{\n"
        "    return pow(max(value, vec3(0.0)), vec3(2.2));\n"
        "}\n"
        "vec3 linearToSrgb(vec3 value)\n"
        "{\n"
        "    return pow(max(value, vec3(0.0)), vec3(1.0 / 2.2));\n"
        "}\n"
        "float distributionGgx(vec3 normal, vec3 halfway, float roughness)\n"
        "{\n"
        "    float a = roughness * roughness;\n"
        "    float a2 = a * a;\n"
        "    float nDotH = max(dot(normal, halfway), 0.0);\n"
        "    float denominator = nDotH * nDotH * (a2 - 1.0) + 1.0;\n"
        "    return a2 / max(PI * denominator * denominator, 0.0001);\n"
        "}\n"
        "float geometrySchlickGgx(float nDotDirection, float roughness)\n"
        "{\n"
        "    float r = roughness + 1.0;\n"
        "    float k = (r * r) / 8.0;\n"
        "    return nDotDirection / max(nDotDirection * (1.0 - k) + k, 0.0001);\n"
        "}\n"
        "float geometrySmith(vec3 normal, vec3 viewDirection, vec3 lightDirection, float roughness)\n"
        "{\n"
        "    return geometrySchlickGgx(max(dot(normal, viewDirection), 0.0), roughness) *\n"
        "        geometrySchlickGgx(max(dot(normal, lightDirection), 0.0), roughness);\n"
        "}\n"
        "vec3 fresnelSchlick(float cosine, vec3 reflectance)\n"
        "{\n"
        "    return reflectance + (1.0 - reflectance) * pow(1.0 - cosine, 5.0);\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    vec4 baseSample = texture(texture0, fragTexCoord);\n"
        "    vec3 albedo = srgbToLinear(baseSample.rgb) * colDiffuse.rgb;\n"
        "    float alpha = baseSample.a * colDiffuse.a;\n"
        "    if (" RFBX_SHADER_UNIFORM_HAS_OPACITY_MAP " != 0)\n"
        "        alpha *= texture(texture6, fragTexCoord).r;\n"
        "    if (" RFBX_SHADER_UNIFORM_ALPHA_MODE " == 0) alpha = 1.0;\n"
        "    if (" RFBX_SHADER_UNIFORM_ALPHA_MODE " == 1)\n"
        "    {\n"
        "        if (alpha < " RFBX_SHADER_UNIFORM_ALPHA_CUTOFF ") discard;\n"
        "        alpha = 1.0;\n"
        "    }\n"
        "    vec3 normal = normalize(fragTbn[2]);\n"
        "    if (" RFBX_SHADER_UNIFORM_HAS_NORMAL_MAP " != 0)\n"
        "    {\n"
        "        vec3 tangentNormal = texture(texture2, fragTexCoord).xyz * 2.0 - 1.0;\n"
        "        normal = normalize(fragTbn * tangentNormal);\n"
        "    }\n"
        "    if (" RFBX_SHADER_UNIFORM_DOUBLE_SIDED " != 0 && !gl_FrontFacing)\n"
        "        normal = -normal;\n"
        "    float metallic = " RFBX_SHADER_UNIFORM_METALLIC_FACTOR ";\n"
        "    if (" RFBX_SHADER_UNIFORM_HAS_METALLIC_MAP " != 0)\n"
        "        metallic *= texture(texture1, fragTexCoord).r;\n"
        "    float roughness = " RFBX_SHADER_UNIFORM_ROUGHNESS_FACTOR ";\n"
        "    if (" RFBX_SHADER_UNIFORM_HAS_ROUGHNESS_MAP " != 0)\n"
        "    {\n"
        "        float sampleRoughness = texture(texture3, fragTexCoord).r;\n"
        "        if (" RFBX_SHADER_UNIFORM_ROUGHNESS_MAP_INVERTED " != 0)\n"
        "            sampleRoughness = 1.0 - sampleRoughness;\n"
        "        roughness *= sampleRoughness;\n"
        "    }\n"
        "    metallic = clamp(metallic, 0.0, 1.0);\n"
        "    roughness = clamp(roughness, 0.04, 1.0);\n"
        "    vec3 emission = " RFBX_SHADER_UNIFORM_EMISSION_COLOR ".rgb;\n"
        "    if (" RFBX_SHADER_UNIFORM_HAS_EMISSION_MAP " != 0)\n"
        "        emission *= srgbToLinear(texture(texture5, fragTexCoord).rgb);\n"
        "    vec3 lightDirection = normalize(-"
        RFBX_SHADER_UNIFORM_LIGHT_DIRECTION ");\n"
        "    vec3 viewDirection = normalize("
        RFBX_SHADER_UNIFORM_VIEW_POSITION " - fragPosition);\n"
        "    vec3 halfway = normalize(viewDirection + lightDirection);\n"
        "    vec3 reflectance = mix(vec3(0.04), albedo, metallic);\n"
        "    vec3 fresnel = fresnelSchlick(max(dot(halfway, viewDirection), 0.0), reflectance);\n"
        "    float distribution = distributionGgx(normal, halfway, roughness);\n"
        "    float geometry = geometrySmith(normal, viewDirection, lightDirection, roughness);\n"
        "    float denominator = 4.0 * max(dot(normal, viewDirection), 0.0) *\n"
        "        max(dot(normal, lightDirection), 0.0) + 0.0001;\n"
        "    vec3 specular = distribution * geometry * fresnel / denominator;\n"
        "    vec3 diffuseWeight = (vec3(1.0) - fresnel) * (1.0 - metallic);\n"
        "    float nDotL = max(dot(normal, lightDirection), 0.0);\n"
        "    vec3 radiance = srgbToLinear("
        RFBX_SHADER_UNIFORM_LIGHT_COLOR ".rgb) * "
        RFBX_SHADER_UNIFORM_LIGHT_INTENSITY ";\n"
        "    vec3 direct = (diffuseWeight * albedo / PI + specular) * radiance * nDotL;\n"
        "    vec3 ambient = srgbToLinear("
        RFBX_SHADER_UNIFORM_AMBIENT_COLOR ".rgb) * "
        RFBX_SHADER_UNIFORM_AMBIENT_INTENSITY " * albedo;\n"
        "    vec3 color = direct + ambient + emission;\n"
        "    color = color / (color + vec3(1.0));\n"
        "    finalColor = vec4(linearToSrgb(color), alpha);\n"
        "}\n";

    return LoadShaderFromMemory(vertexShader, fragmentShader);
}

#undef RFBX_STRINGIFY
#undef RFBX_STRINGIFY_INNER

// Refresh a cluster palette from an original or evaluated ufbx mesh.
static bool rfbxUpdateGpuPalette(
    Matrix* matrices,
    int boneCount,
    const ufbx_mesh* mesh)
{
    if (!matrices || !mesh || mesh->skin_deformers.count != 1) return false;
    const ufbx_skin_deformer* skin = mesh->skin_deformers.data[0];
    if (!skin || skin->clusters.count != (size_t)boneCount) return false;

    for (int boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
        const ufbx_skin_cluster* cluster = skin->clusters.data[boneIndex];
        if (!cluster) return false;
        matrices[boneIndex] = rfbxMatrixFromUfbx(cluster->geometry_to_world);
    }
    return true;
}

// Group influenced bind vertices per cluster for conservative GPU pose bounds.
static bool rfbxBuildGpuBoneBounds(
    RfbxGpuMeshState* state,
    const RfbxMesh* mesh)
{
    if (!state || !mesh || state->boneCount <= 0) return false;
    state->boneLocalBounds = rfbxAllocArray(
        (size_t)state->boneCount,
        sizeof(BoundingBox));
    state->boneBoundsValid = rfbxAllocArray(
        (size_t)state->boneCount,
        sizeof(bool));
    if (!state->boneLocalBounds || !state->boneBoundsValid) return false;

    for (int primitiveIndex = 0;
        primitiveIndex < mesh->primitiveCount;
        ++primitiveIndex) {
        const RfbxPrimitive* primitive = &mesh->primitives[primitiveIndex];
        if (!primitive->mesh.vertices ||
            !primitive->boneIndices ||
            !primitive->boneWeights) {
            return false;
        }

        for (int vertex = 0; vertex < primitive->mesh.vertexCount; ++vertex) {
            Vector3 point = {
                primitive->mesh.vertices[(size_t)vertex * 3u + 0u],
                primitive->mesh.vertices[(size_t)vertex * 3u + 1u],
                primitive->mesh.vertices[(size_t)vertex * 3u + 2u]
            };
            for (int influence = 0; influence < 4; ++influence) {
                size_t offset = (size_t)vertex * 4u + (size_t)influence;
                if (primitive->boneWeights[offset] <= 0.0f) continue;
                int boneIndex = primitive->boneIndices[offset];
                if (boneIndex >= state->boneCount) return false;
                rfbxBoundsIncludePoint(
                    &state->boneLocalBounds[boneIndex],
                    &state->boneBoundsValid[boneIndex],
                    point);
            }
        }
    }
    return true;
}

// Cache the RFBX-specific uniform locations of the active draw shader.
static void rfbxCacheActiveShaderLocations(
    RfbxInternal* internal,
    Shader shader,
    bool custom)
{
    internal->activeShader = shader;
    internal->gpuBoneMatricesLocation =
        GetShaderLocation(
            shader,
            RFBX_SHADER_UNIFORM_BONE_MATRICES "[0]");
    internal->useSkinningLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_USE_SKINNING);
    internal->hasNormalMapLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_NORMAL_MAP);
    internal->lightDirectionLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_LIGHT_DIRECTION);
    internal->lightColorLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_LIGHT_COLOR);
    internal->lightIntensityLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_LIGHT_INTENSITY);
    internal->ambientColorLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_AMBIENT_COLOR);
    internal->ambientIntensityLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_AMBIENT_INTENSITY);
    internal->viewPositionLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_VIEW_POSITION);
    internal->hasMetallicMapLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_METALLIC_MAP);
    internal->hasRoughnessMapLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_ROUGHNESS_MAP);
    internal->hasEmissionMapLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_EMISSION_MAP);
    internal->hasOpacityMapLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_OPACITY_MAP);
    internal->metallicFactorLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_METALLIC_FACTOR);
    internal->roughnessFactorLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_ROUGHNESS_FACTOR);
    internal->roughnessMapInvertedLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_ROUGHNESS_MAP_INVERTED);
    internal->emissionColorLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_EMISSION_COLOR);
    internal->alphaModeLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_ALPHA_MODE);
    internal->alphaCutoffLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_ALPHA_CUTOFF);
    internal->doubleSidedLocation =
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_DOUBLE_SIDED);
    internal->activeShaderReady = IsShaderValid(shader);
    internal->usingCustomShader = custom;
}

// Initialize the built-in shader and bind-pose palettes with CPU fallback.
static bool rfbxInitializeRendering(RfbxModel* model, const ufbx_scene* scene)
{
    if (!model || !model->internal || !scene) return false;

    int gpuMeshCount = 0;
    int drawableMeshCount = 0;
    for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
        if (model->meshes[meshIndex].gpuSkinningReady) ++gpuMeshCount;
        if (model->meshes[meshIndex].primitiveCount > 0) ++drawableMeshCount;
    }
    if (drawableMeshCount == 0) return true;

    Shader shader = rfbxLoadBuiltinShader();
    int matricesLocation = IsShaderValid(shader)
        ? GetShaderLocation(
            shader,
            RFBX_SHADER_UNIFORM_BONE_MATRICES "[0]")
        : -1;
    int useSkinningLocation = IsShaderValid(shader)
        ? GetShaderLocation(shader, RFBX_SHADER_UNIFORM_USE_SKINNING)
        : -1;
    bool pbrContract = IsShaderValid(shader) &&
        GetShaderLocationAttrib(shader, "vertexPosition") >= 0 &&
        GetShaderLocationAttrib(shader, "vertexTexCoord") >= 0 &&
        GetShaderLocationAttrib(shader, "vertexNormal") >= 0 &&
        GetShaderLocationAttrib(shader, "vertexTangent") >= 0 &&
        GetShaderLocation(shader, "mvp") >= 0 &&
        GetShaderLocation(shader, "matModel") >= 0 &&
        GetShaderLocation(shader, "matNormal") >= 0 &&
        GetShaderLocation(shader, "texture0") >= 0 &&
        GetShaderLocation(shader, "texture1") >= 0 &&
        GetShaderLocation(shader, "texture2") >= 0 &&
        GetShaderLocation(shader, "texture3") >= 0 &&
        GetShaderLocation(shader, "texture5") >= 0 &&
        GetShaderLocation(shader, "texture6") >= 0 &&
        GetShaderLocation(shader, "colDiffuse") >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_NORMAL_MAP) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_METALLIC_MAP) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_ROUGHNESS_MAP) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_EMISSION_MAP) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_HAS_OPACITY_MAP) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_METALLIC_FACTOR) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_ROUGHNESS_FACTOR) >= 0 &&
        GetShaderLocation(
            shader,
            RFBX_SHADER_UNIFORM_ROUGHNESS_MAP_INVERTED) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_EMISSION_COLOR) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_ALPHA_MODE) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_ALPHA_CUTOFF) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_DOUBLE_SIDED) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_LIGHT_DIRECTION) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_LIGHT_COLOR) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_LIGHT_INTENSITY) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_AMBIENT_COLOR) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_AMBIENT_INTENSITY) >= 0 &&
        GetShaderLocation(shader, RFBX_SHADER_UNIFORM_VIEW_POSITION) >= 0;
    if (!IsShaderValid(shader) ||
        !pbrContract ||
        (gpuMeshCount > 0 &&
            (matricesLocation < 0 || useSkinningLocation < 0))) {
        unsigned int defaultShaderId = rlGetShaderIdDefault();
        if (IsShaderValid(shader) && shader.id != defaultShaderId) {
            UnloadShader(shader);
        }
        for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
            model->meshes[meshIndex].gpuSkinningReady = false;
            model->meshes[meshIndex].gpuBoneCount = 0;
        }
        TraceLog(LOG_WARNING,
            "RFBX: built-in shader unavailable, using raylib and CPU fallback");
        return true;
    }

    model->internal->builtinShader = shader;
    model->internal->builtinShaderReady = true;
    rfbxCacheActiveShaderLocations(model->internal, shader, false);
    if (shader.locs) {
        shader.locs[SHADER_LOC_MAP_ALBEDO] =
            GetShaderLocation(shader, "texture0");
        shader.locs[SHADER_LOC_MAP_NORMAL] =
            GetShaderLocation(shader, "texture2");
        shader.locs[SHADER_LOC_MAP_METALNESS] =
            GetShaderLocation(shader, "texture1");
        shader.locs[SHADER_LOC_MAP_ROUGHNESS] =
            GetShaderLocation(shader, "texture3");
        shader.locs[SHADER_LOC_MAP_EMISSION] =
            GetShaderLocation(shader, "texture5");
        shader.locs[SHADER_LOC_MAP_HEIGHT] =
            GetShaderLocation(shader, "texture6");
    }

    if (gpuMeshCount > 0) {
        model->internal->gpuMeshes = rfbxAllocArray(
            (size_t)model->meshCount,
            sizeof(RfbxGpuMeshState));
        if (!model->internal->gpuMeshes) return false;
    }

    for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
        RfbxMesh* mesh = &model->meshes[meshIndex];
        if (!mesh->gpuSkinningReady) continue;
        if (mesh->sourceTypedId >= scene->meshes.count) return false;

        RfbxGpuMeshState* state = &model->internal->gpuMeshes[meshIndex];
        state->boneCount = mesh->gpuBoneCount;
        state->boneMatrices = rfbxAllocArray(
            (size_t)state->boneCount,
            sizeof(Matrix));
        if (!state->boneMatrices ||
            !rfbxUpdateGpuPalette(
                state->boneMatrices,
                state->boneCount,
                scene->meshes.data[mesh->sourceTypedId]) ||
            !rfbxBuildGpuBoneBounds(state, mesh)) {
            return false;
        }
        mesh->verticesAreLocal = false;
    }

    if (gpuMeshCount > 0) {
        TraceLog(LOG_INFO,
            "RFBX: GPU skinning enabled for %d mesh(es), CPU fallback remains active",
            gpuMeshCount);
    }
    return true;
}

// Return whether a model texture index refers to a loaded GPU texture.
static bool rfbxIsTextureReady(const RfbxModel* model, int textureIndex)
{
    return model && textureIndex >= 0 && textureIndex < model->textureCount &&
        model->textures[textureIndex].ready;
}

// Upload PBR material state, lighting, and an optional skinning palette.
static bool rfbxPrepareShaderDraw(
    const RfbxModel* model,
    Material* material,
    const Matrix* boneMatrices,
    int boneCount,
    bool useSkinning,
    int materialIndex)
{
    if (!model || !model->internal || !material) return false;
    const RfbxInternal* internal = model->internal;
    if (!internal->activeShaderReady) return !useSkinning;
    if (useSkinning &&
        (!boneMatrices || boneCount <= 0 ||
            boneCount > RFBX_MAX_GPU_BONES ||
            internal->gpuBoneMatricesLocation < 0 ||
            internal->useSkinningLocation < 0)) {
        return false;
    }

    material->shader = internal->activeShader;
    rlEnableShader(material->shader.id);

    int skinningValue = useSkinning ? 1 : 0;
    if (internal->useSkinningLocation >= 0) {
        rlSetUniform(
            internal->useSkinningLocation,
            &skinningValue,
            RL_SHADER_UNIFORM_INT,
            1);
    }
    if (useSkinning) {
        rlSetUniformMatrices(
            internal->gpuBoneMatricesLocation,
            boneMatrices,
            boneCount);
    }

    const RfbxMaterial* sourceMaterial =
        materialIndex >= 0 && materialIndex < model->materialCount
        ? &model->materials[materialIndex]
        : NULL;
    int normalMapValue = sourceMaterial && rfbxIsTextureReady(
        model,
        sourceMaterial->normalTextureIndex);
    int metallicMapValue = sourceMaterial && rfbxIsTextureReady(
        model,
        sourceMaterial->metallicTextureIndex);
    int roughnessMapValue = sourceMaterial && rfbxIsTextureReady(
        model,
        sourceMaterial->roughnessTextureIndex);
    int emissionMapValue = sourceMaterial && rfbxIsTextureReady(
        model,
        sourceMaterial->emissiveTextureIndex);
    int opacityMapValue = sourceMaterial && rfbxIsTextureReady(
        model,
        sourceMaterial->opacityTextureIndex);
    int roughnessMapInverted = sourceMaterial &&
        sourceMaterial->roughnessTextureIsGlossiness;
    int alphaMode = sourceMaterial
        ? (int)sourceMaterial->alphaMode
        : (int)RFBX_ALPHA_OPAQUE;
    int doubleSided = sourceMaterial && sourceMaterial->doubleSided;
    float metallicFactor = sourceMaterial ? sourceMaterial->metallic : 0.0f;
    float roughnessFactor = sourceMaterial ? sourceMaterial->roughness : 1.0f;
    float alphaCutoff = sourceMaterial ? sourceMaterial->alphaCutoff : 0.5f;
    Color emissionSource = sourceMaterial
        ? sourceMaterial->emissiveColor
        : BLACK;
    float emissionColor[4] = {
        emissionSource.r / 255.0f,
        emissionSource.g / 255.0f,
        emissionSource.b / 255.0f,
        emissionSource.a / 255.0f
    };

    if (internal->hasNormalMapLocation >= 0) {
        rlSetUniform(
            internal->hasNormalMapLocation,
            &normalMapValue,
            RL_SHADER_UNIFORM_INT,
            1);
    }
    if (internal->hasMetallicMapLocation >= 0) {
        rlSetUniform(
            internal->hasMetallicMapLocation,
            &metallicMapValue,
            RL_SHADER_UNIFORM_INT,
            1);
    }
    if (internal->hasRoughnessMapLocation >= 0) {
        rlSetUniform(
            internal->hasRoughnessMapLocation,
            &roughnessMapValue,
            RL_SHADER_UNIFORM_INT,
            1);
    }
    if (internal->hasEmissionMapLocation >= 0) {
        rlSetUniform(
            internal->hasEmissionMapLocation,
            &emissionMapValue,
            RL_SHADER_UNIFORM_INT,
            1);
    }
    if (internal->hasOpacityMapLocation >= 0) {
        rlSetUniform(
            internal->hasOpacityMapLocation,
            &opacityMapValue,
            RL_SHADER_UNIFORM_INT,
            1);
    }
    if (internal->metallicFactorLocation >= 0) {
        rlSetUniform(
            internal->metallicFactorLocation,
            &metallicFactor,
            RL_SHADER_UNIFORM_FLOAT,
            1);
    }
    if (internal->roughnessFactorLocation >= 0) {
        rlSetUniform(
            internal->roughnessFactorLocation,
            &roughnessFactor,
            RL_SHADER_UNIFORM_FLOAT,
            1);
    }
    if (internal->roughnessMapInvertedLocation >= 0) {
        rlSetUniform(
            internal->roughnessMapInvertedLocation,
            &roughnessMapInverted,
            RL_SHADER_UNIFORM_INT,
            1);
    }
    if (internal->emissionColorLocation >= 0) {
        rlSetUniform(
            internal->emissionColorLocation,
            emissionColor,
            RL_SHADER_UNIFORM_VEC4,
            1);
    }
    if (internal->alphaModeLocation >= 0) {
        rlSetUniform(
            internal->alphaModeLocation,
            &alphaMode,
            RL_SHADER_UNIFORM_INT,
            1);
    }
    if (internal->alphaCutoffLocation >= 0) {
        rlSetUniform(
            internal->alphaCutoffLocation,
            &alphaCutoff,
            RL_SHADER_UNIFORM_FLOAT,
            1);
    }
    if (internal->doubleSidedLocation >= 0) {
        rlSetUniform(
            internal->doubleSidedLocation,
            &doubleSided,
            RL_SHADER_UNIFORM_INT,
            1);
    }

    const RfbxLighting* lighting = &internal->lighting;
    float direction[3] = {
        lighting->direction.x,
        lighting->direction.y,
        lighting->direction.z
    };
    float lightColor[4] = {
        lighting->color.r / 255.0f,
        lighting->color.g / 255.0f,
        lighting->color.b / 255.0f,
        lighting->color.a / 255.0f
    };
    float ambientColor[4] = {
        lighting->ambientColor.r / 255.0f,
        lighting->ambientColor.g / 255.0f,
        lighting->ambientColor.b / 255.0f,
        lighting->ambientColor.a / 255.0f
    };
    if (internal->lightDirectionLocation >= 0) {
        rlSetUniform(
            internal->lightDirectionLocation,
            direction,
            RL_SHADER_UNIFORM_VEC3,
            1);
    }
    if (internal->lightColorLocation >= 0) {
        rlSetUniform(
            internal->lightColorLocation,
            lightColor,
            RL_SHADER_UNIFORM_VEC4,
            1);
    }
    if (internal->lightIntensityLocation >= 0) {
        rlSetUniform(
            internal->lightIntensityLocation,
            &lighting->intensity,
            RL_SHADER_UNIFORM_FLOAT,
            1);
    }
    if (internal->ambientColorLocation >= 0) {
        rlSetUniform(
            internal->ambientColorLocation,
            ambientColor,
            RL_SHADER_UNIFORM_VEC4,
            1);
    }
    if (internal->ambientIntensityLocation >= 0) {
        rlSetUniform(
            internal->ambientIntensityLocation,
            &lighting->ambientIntensity,
            RL_SHADER_UNIFORM_FLOAT,
            1);
    }
    if (internal->viewPositionLocation >= 0) {
        Matrix camera = MatrixInvert(rlGetMatrixModelview());
        float viewPosition[3] = { camera.m12, camera.m13, camera.m14 };
        rlSetUniform(
            internal->viewPositionLocation,
            viewPosition,
            RL_SHADER_UNIFORM_VEC3,
            1);
    }
    rlDisableShader();
    return true;
}

// Clone the vertex streams needed by an instance-owned dynamic mesh.
static bool rfbxCloneDynamicMesh(Mesh* destination, const Mesh* source)
{
    if (!destination || !source || source->vertexCount <= 0) return false;

    size_t vertexCount = (size_t)source->vertexCount;
    destination->vertexCount = source->vertexCount;
    destination->triangleCount = source->triangleCount;

    destination->vertices = rfbxAllocArray(vertexCount * 3u, sizeof(float));
    destination->normals = rfbxAllocArray(vertexCount * 3u, sizeof(float));
    destination->texcoords = rfbxAllocArray(vertexCount * 2u, sizeof(float));
    destination->tangents = rfbxAllocArray(vertexCount * 4u, sizeof(float));

    if (!destination->vertices ||
        !destination->normals ||
        !destination->texcoords ||
        !destination->tangents) {
        MemFree(destination->vertices);
        MemFree(destination->normals);
        MemFree(destination->texcoords);
        MemFree(destination->tangents);
        memset(destination, 0, sizeof(*destination));
        return false;
    }

    memcpy(
        destination->vertices,
        source->vertices,
        vertexCount * 3u * sizeof(float));
    memcpy(
        destination->normals,
        source->normals,
        vertexCount * 3u * sizeof(float));
    memcpy(
        destination->texcoords,
        source->texcoords,
        vertexCount * 2u * sizeof(float));
    memcpy(
        destination->tangents,
        source->tangents,
        vertexCount * 4u * sizeof(float));

    UploadMesh(destination, true);
    return true;
}

// Update a dynamic raylib primitive from evaluated ufbx vertex attributes.
static bool rfbxUpdateMeshVertices(
    Mesh* mesh,
    const uint32_t* sourceIndices,
    const ufbx_vertex_vec3* positions,
    const ufbx_vertex_vec3* normals)
{
    if (!mesh || !positions || !positions->exists) return false;
    if (!mesh->vertices || !mesh->normals || !sourceIndices) return false;

    size_t vertexCount = (size_t)mesh->vertexCount;
    if (vertexCount > (size_t)INT_MAX / (4u * sizeof(float))) return false;

    for (size_t vertexIndex = 0; vertexIndex < vertexCount; ++vertexIndex) {
        uint32_t sourceIndex = sourceIndices[vertexIndex];
        if ((size_t)sourceIndex >= positions->indices.count) return false;

        ufbx_vec3 position = ufbx_get_vertex_vec3(positions, sourceIndex);
        mesh->vertices[vertexIndex * 3u + 0u] = (float)position.x;
        mesh->vertices[vertexIndex * 3u + 1u] = (float)position.y;
        mesh->vertices[vertexIndex * 3u + 2u] = (float)position.z;

        ufbx_vec3 normal = { 0.0, 1.0, 0.0 };
        if (normals && normals->exists) {
            if ((size_t)sourceIndex >= normals->indices.count) return false;
            normal = ufbx_get_vertex_vec3(normals, sourceIndex);
        }

        mesh->normals[vertexIndex * 3u + 0u] = (float)normal.x;
        mesh->normals[vertexIndex * 3u + 1u] = (float)normal.y;
        mesh->normals[vertexIndex * 3u + 2u] = (float)normal.z;
    }

    if (!rfbxGenerateMeshTangents(mesh)) return false;

    int vectorBufferSize = (int)(vertexCount * 3u * sizeof(float));
    int tangentBufferSize = (int)(vertexCount * 4u * sizeof(float));
    UpdateMeshBuffer(*mesh, 0, mesh->vertices, vectorBufferSize, 0);
    UpdateMeshBuffer(*mesh, 2, mesh->normals, vectorBufferSize, 0);
    UpdateMeshBuffer(*mesh, 4, mesh->tangents, tangentBufferSize, 0);
    return true;
}

// Calculate bounds across an array of raylib meshes.
static bool rfbxCalculateMeshArrayBounds(
    const Mesh* meshes,
    int meshCount,
    BoundingBox* bounds)
{
    if (!meshes || meshCount <= 0 || !bounds) return false;
    *bounds = (BoundingBox){ 0 };
    bool valid = false;
    for (int meshIndex = 0; meshIndex < meshCount; ++meshIndex) {
        BoundingBox meshBounds = { 0 };
        if (rfbxCalculateMeshBounds(&meshes[meshIndex], &meshBounds)) {
            rfbxBoundsIncludeBox(bounds, &valid, meshBounds);
        }
    }
    return valid;
}

// Calculate current CPU-side bounds across RFBX mesh primitives.
static bool rfbxCalculatePrimitiveBounds(
    const RfbxMesh* mesh,
    BoundingBox* bounds)
{
    if (!mesh || !bounds) return false;
    *bounds = (BoundingBox){ 0 };
    bool valid = false;
    for (int primitiveIndex = 0;
        primitiveIndex < mesh->primitiveCount;
        ++primitiveIndex) {
        BoundingBox primitiveBounds = { 0 };
        if (rfbxCalculateMeshBounds(
            &mesh->primitives[primitiveIndex].mesh,
            &primitiveBounds)) {
            rfbxBoundsIncludeBox(bounds, &valid, primitiveBounds);
        }
    }
    return valid;
}

// Transform per-cluster influence boxes and conservatively enclose GPU skinning.
static bool rfbxCalculateGpuPoseBounds(
    const RfbxGpuMeshState* state,
    const Matrix* boneMatrices,
    int boneCount,
    BoundingBox* bounds)
{
    if (!state || !boneMatrices || !bounds ||
        boneCount != state->boneCount ||
        !state->boneLocalBounds || !state->boneBoundsValid) {
        return false;
    }

    *bounds = (BoundingBox){ 0 };
    bool valid = false;
    for (int boneIndex = 0; boneIndex < boneCount; ++boneIndex) {
        if (!state->boneBoundsValid[boneIndex]) continue;
        BoundingBox transformed = rfbxTransformBounds(
            state->boneLocalBounds[boneIndex],
            boneMatrices[boneIndex]);
        rfbxBoundsIncludeBox(bounds, &valid, transformed);
    }
    return valid;
}

// Rebuild model-space bounds from the current mutable model pose.
static void rfbxUpdateModelBounds(RfbxModel* model)
{
    if (!model) return;
    model->bounds = (BoundingBox){ 0 };
    model->boundsValid = false;

    for (int nodeIndex = 0; nodeIndex < model->nodeCount; ++nodeIndex) {
        const RfbxNode* node = &model->nodes[nodeIndex];
        if (!node->visible ||
            node->meshIndex < 0 ||
            node->meshIndex >= model->meshCount) {
            continue;
        }

        const RfbxMesh* mesh = &model->meshes[node->meshIndex];
        BoundingBox geometryBounds = { 0 };
        bool valid = false;
        bool applyNodeTransform = !mesh->hasSkinning || mesh->verticesAreLocal;

        if (mesh->gpuSkinningReady &&
            model->internal && model->internal->gpuMeshes) {
            const RfbxGpuMeshState* state =
                &model->internal->gpuMeshes[node->meshIndex];
            valid = rfbxCalculateGpuPoseBounds(
                state,
                state->boneMatrices,
                state->boneCount,
                &geometryBounds);
            applyNodeTransform = false;
        }
        else if (mesh->hasSkinning) {
            valid = rfbxCalculatePrimitiveBounds(mesh, &geometryBounds);
        }
        else if (mesh->boundsValid) {
            geometryBounds = mesh->localBounds;
            valid = true;
        }

        if (!valid) continue;
        if (applyNodeTransform) {
            geometryBounds = rfbxTransformBounds(
                geometryBounds,
                node->geometryToWorld);
        }
        rfbxBoundsIncludeBox(
            &model->bounds,
            &model->boundsValid,
            geometryBounds);
    }
}

// Rebuild model-space bounds from an independent instance pose.
static void rfbxUpdateInstanceBounds(RfbxInstance* instance)
{
    if (!instance || !instance->model || !instance->internal) return;
    const RfbxModel* model = instance->model;
    instance->bounds = (BoundingBox){ 0 };
    instance->boundsValid = false;

    for (int nodeIndex = 0; nodeIndex < model->nodeCount; ++nodeIndex) {
        const RfbxNode* modelNode = &model->nodes[nodeIndex];
        const RfbxInstanceNodeState* node =
            &instance->internal->nodes[nodeIndex];
        if (!node->visible ||
            modelNode->meshIndex < 0 ||
            modelNode->meshIndex >= model->meshCount) {
            continue;
        }

        const RfbxMesh* mesh = &model->meshes[modelNode->meshIndex];
        const RfbxInstanceMeshState* meshState =
            &instance->internal->meshes[modelNode->meshIndex];
        BoundingBox geometryBounds = { 0 };
        bool valid = false;
        bool applyNodeTransform = !mesh->hasSkinning ||
            meshState->verticesAreLocal;

        if (meshState->gpuSkinning &&
            model->internal && model->internal->gpuMeshes) {
            const RfbxGpuMeshState* gpuState =
                &model->internal->gpuMeshes[modelNode->meshIndex];
            valid = rfbxCalculateGpuPoseBounds(
                gpuState,
                meshState->boneMatrices,
                meshState->boneCount,
                &geometryBounds);
            applyNodeTransform = false;
        }
        else if (mesh->hasSkinning) {
            valid = rfbxCalculateMeshArrayBounds(
                meshState->primitives,
                meshState->primitiveCount,
                &geometryBounds);
        }
        else if (mesh->boundsValid) {
            geometryBounds = mesh->localBounds;
            valid = true;
        }

        if (!valid) continue;
        if (applyNodeTransform) {

            //乗算用を合わせる
            const Matrix correction = MatrixScale(
                node->scaleMultiplier.x,
                node->scaleMultiplier.y,
                node->scaleMultiplier.z);
            //同上。ジオメトリも合わせる
            const Matrix correctedGeometry = MatrixMultiply(
                correction,
                node->geometryToWorld);

            // モデル座標の移動量。ノード自身の拡縮では移動量を拡大しない。
            const Matrix adjustedGeometry = MatrixMultiply(
                correctedGeometry,
                MatrixTranslate(node->offset.x, node->offset.y, node->offset.z));

            geometryBounds = rfbxTransformBounds(
                geometryBounds,
                adjustedGeometry);
        }
        rfbxBoundsIncludeBox(
            &instance->bounds,
            &instance->boundsValid,
            geometryBounds);
    }
}

// Copy evaluated transforms and deformed vertices into an existing model.
static bool rfbxApplySceneState(RfbxModel* model, const ufbx_scene* scene)
{
    if (!model || !scene) return false;

    for (int nodeIndex = 0; nodeIndex < model->nodeCount; ++nodeIndex) {
        RfbxNode* destination = &model->nodes[nodeIndex];
        if (destination->sourceTypedId >= scene->nodes.count) return false;

        const ufbx_node* source = scene->nodes.data[destination->sourceTypedId];
        destination->visible = source->visible;
        destination->nodeToParent = rfbxMatrixFromUfbx(source->node_to_parent);
        destination->nodeToWorld = rfbxMatrixFromUfbx(source->node_to_world);
        destination->geometryToNode = rfbxMatrixFromUfbx(source->geometry_to_node);
        destination->geometryToWorld = rfbxMatrixFromUfbx(source->geometry_to_world);
    }

    for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
        RfbxMesh* destination = &model->meshes[meshIndex];
        if (!destination->hasSkinning) continue;
        if (destination->sourceTypedId >= scene->meshes.count) return false;

        const ufbx_mesh* source = scene->meshes.data[destination->sourceTypedId];
        if (destination->gpuSkinningReady) {
            if (!model->internal || !model->internal->gpuMeshes) return false;
            RfbxGpuMeshState* state =
                &model->internal->gpuMeshes[meshIndex];
            if (!rfbxUpdateGpuPalette(
                state->boneMatrices,
                state->boneCount,
                source)) {
                return false;
            }
            destination->verticesAreLocal = false;
            continue;
        }

        const ufbx_vertex_vec3* positions = source->skinned_position.exists
            ? &source->skinned_position
            : &source->vertex_position;
        const ufbx_vertex_vec3* normals = source->skinned_normal.exists
            ? &source->skinned_normal
            : &source->vertex_normal;

        for (int primitiveIndex = 0;
            primitiveIndex < destination->primitiveCount;
            ++primitiveIndex) {
            RfbxPrimitive* primitive = &destination->primitives[primitiveIndex];
            if (!rfbxUpdateMeshVertices(
                &primitive->mesh,
                primitive->sourceIndices,
                positions,
                normals)) {
                return false;
            }
        }

        destination->verticesAreLocal = source->skinned_is_local;
    }

    rfbxUpdateModelBounds(model);
    return true;
}

// Copy evaluated transforms and deformation into an independent instance.
static bool rfbxApplySceneStateToInstance(
    RfbxInstance* instance,
    const ufbx_scene* scene)
{
    if (!instance || !instance->model || !instance->internal || !scene) {
        return false;
    }

    const RfbxModel* model = instance->model;

    for (int nodeIndex = 0; nodeIndex < model->nodeCount; ++nodeIndex) {
        const RfbxNode* modelNode = &model->nodes[nodeIndex];
        if (modelNode->sourceTypedId >= scene->nodes.count) return false;

        const ufbx_node* source = scene->nodes.data[modelNode->sourceTypedId];
        RfbxInstanceNodeState* destination =
            &instance->internal->nodes[nodeIndex];

        destination->visible = source->visible;
        destination->nodeToParent = rfbxMatrixFromUfbx(source->node_to_parent);
        destination->nodeToWorld = rfbxMatrixFromUfbx(source->node_to_world);
        destination->geometryToNode = rfbxMatrixFromUfbx(source->geometry_to_node);
        destination->geometryToWorld = rfbxMatrixFromUfbx(source->geometry_to_world);
    }

    for (int boneIndex = 0;
        boneIndex < model->skeleton.boneCount;
        ++boneIndex) {
        uint32_t nodeIndex = model->skeleton.bones[boneIndex].sourceNodeTypedId;
        if (nodeIndex >= scene->nodes.count) return false;
        instance->internal->boneToWorld[boneIndex] = rfbxMatrixFromUfbx(
            scene->nodes.data[nodeIndex]->node_to_world);
    }

    for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
        const RfbxMesh* modelMesh = &model->meshes[meshIndex];
        if (!modelMesh->hasSkinning) continue;
        if (modelMesh->sourceTypedId >= scene->meshes.count) return false;

        const ufbx_mesh* source = scene->meshes.data[modelMesh->sourceTypedId];
        RfbxInstanceMeshState* destination =
            &instance->internal->meshes[meshIndex];
        if (destination->gpuSkinning) {
            if (!rfbxUpdateGpuPalette(
                destination->boneMatrices,
                destination->boneCount,
                source)) {
                return false;
            }
            destination->verticesAreLocal = false;
            continue;
        }

        const ufbx_vertex_vec3* positions = source->skinned_position.exists
            ? &source->skinned_position
            : &source->vertex_position;
        const ufbx_vertex_vec3* normals = source->skinned_normal.exists
            ? &source->skinned_normal
            : &source->vertex_normal;
        if (destination->primitiveCount != modelMesh->primitiveCount) {
            return false;
        }

        for (int primitiveIndex = 0;
            primitiveIndex < modelMesh->primitiveCount;
            ++primitiveIndex) {
            const RfbxPrimitive* modelPrimitive =
                &modelMesh->primitives[primitiveIndex];
            if (!rfbxUpdateMeshVertices(
                &destination->primitives[primitiveIndex],
                modelPrimitive->sourceIndices,
                positions,
                normals)) {
                return false;
            }
        }

        destination->verticesAreLocal = source->skinned_is_local;
    }

    rfbxUpdateInstanceBounds(instance);
    return true;
}

// Load and convert an FBX scene to RFBX-owned resources.
RfbxModel RfbxLoadModel(const char* fileName)
{
    RfbxModel result = { 0 };
    ufbx_load_opts options = { 0 };

    result.currentAnimation = RFBX_INVALID_INDEX;

    if (!fileName || fileName[0] == '\0') {
        TraceLog(LOG_WARNING, "RFBX: file name is empty");
        return result;
    }

    options.target_axes = ufbx_axes_right_handed_y_up;
    options.target_unit_meters = 1.0;
    options.space_conversion = UFBX_SPACE_CONVERSION_TRANSFORM_ROOT;
    options.geometry_transform_handling = UFBX_GEOMETRY_TRANSFORM_HANDLING_PRESERVE;
    options.inherit_mode_handling = UFBX_INHERIT_MODE_HANDLING_PRESERVE;
    options.pivot_handling = UFBX_PIVOT_HANDLING_RETAIN;
    options.generate_missing_normals = true;
    options.use_blender_pbr_material = true;

    ufbx_error error = { 0 };
    ufbx_scene* scene = ufbx_load_file(fileName, &options, &error);
    if (!scene) {
        size_t length = error.description.length;
        const char* description = error.description.data;
        if (!description) {
            description = "Unknown error";
            length = strlen(description);
        }
        if (length > (size_t)INT_MAX) length = INT_MAX;
        TraceLog(LOG_WARNING,
            "RFBX: failed to load '%s': %.*s",
            fileName,
            (int)length,
            description);
        return result;
    }

    if (scene->nodes.count > INT_MAX ||
        scene->meshes.count > INT_MAX ||
        scene->anim_stacks.count > INT_MAX) {
        TraceLog(LOG_WARNING, "RFBX: scene contains too many elements");
        ufbx_free_scene(scene);
        return result;
    }

    result.internal = rfbxAllocArray(1, sizeof(RfbxInternal));
    if (!result.internal) {
        ufbx_free_scene(scene);
        return result;
    }
    result.internal->scene = scene;
    result.internal->lighting = RfbxGetDefaultLighting();

    if (!rfbxBuildMaterials(&result, scene, fileName)) goto load_failed;

    result.meshCount = (int)scene->meshes.count;
    if (result.meshCount > 0) {
        result.meshes = rfbxAllocArray(scene->meshes.count, sizeof(RfbxMesh));
        if (!result.meshes) goto load_failed;
    }

    int drawableMeshCount = 0;
    for (int i = 0; i < result.meshCount; ++i) {
        if (!rfbxBuildMesh(&result.meshes[i], scene->meshes.data[i])) {
            goto load_failed;
        }
        if (result.meshes[i].primitiveCount > 0) ++drawableMeshCount;
    }

    if (!rfbxInitializeRendering(&result, scene)) goto load_failed;

    result.nodeCount = (int)scene->nodes.count;
    if (result.nodeCount > 0) {
        result.nodes = rfbxAllocArray(scene->nodes.count, sizeof(RfbxNode));
        if (!result.nodes) goto load_failed;
    }

    for (int i = 0; i < result.nodeCount; ++i) {
        const ufbx_node* source = scene->nodes.data[i];
        RfbxNode* destination = &result.nodes[i];

        destination->name = rfbxCopyString(source->name);
        if (!destination->name) goto load_failed;

        destination->sourceTypedId = source->typed_id;
        destination->parentIndex = rfbxFindNodeIndex(scene, source->parent);
        destination->meshIndex = rfbxFindMeshIndex(scene, source->mesh);
        if (source->materials.count > INT_MAX) goto load_failed;
        destination->materialCount = (int)source->materials.count;
        if (destination->materialCount > 0) {
            destination->materialIndices = rfbxAllocArray(
                source->materials.count,
                sizeof(int));
            if (!destination->materialIndices) goto load_failed;

            for (int materialIndex = 0;
                materialIndex < destination->materialCount;
                ++materialIndex) {
                destination->materialIndices[materialIndex] =
                    (int)source->materials.data[materialIndex]->typed_id;
            }
        }
        destination->visible = source->visible;
        destination->nodeToParent = rfbxMatrixFromUfbx(source->node_to_parent);
        destination->nodeToWorld = rfbxMatrixFromUfbx(source->node_to_world);
        destination->geometryToNode = rfbxMatrixFromUfbx(source->geometry_to_node);
        destination->geometryToWorld = rfbxMatrixFromUfbx(source->geometry_to_world);
    }

    if (!rfbxBuildSkeleton(&result.skeleton, scene)) goto load_failed;

    result.animationCount = (int)scene->anim_stacks.count;
    if (result.animationCount > 0) {
        result.animations = rfbxAllocArray(
            scene->anim_stacks.count,
            sizeof(RfbxAnimation));
        if (!result.animations) goto load_failed;
    }

    for (int i = 0; i < result.animationCount; ++i) {
        const ufbx_anim_stack* source = scene->anim_stacks.data[i];
        RfbxAnimation* destination = &result.animations[i];

        destination->name = rfbxCopyString(source->name);
        if (!destination->name) goto load_failed;

        destination->timeBegin = source->time_begin;
        destination->timeEnd = source->time_end;
        destination->duration = source->time_end > source->time_begin
            ? source->time_end - source->time_begin
            : 0.0;
        destination->sourceStackIndex = (uint32_t)i;
    }

    if (drawableMeshCount > 0) {
        result.defaultMaterial = LoadMaterialDefault();
        result.defaultMaterialReady = true;
    }

    result.valid = true;
    rfbxUpdateModelBounds(&result);
    TraceLog(LOG_INFO,
        "RFBX: loaded '%s' (%d nodes, %d meshes, %d materials, %d textures, %d bones, %d animations)",
        fileName,
        result.nodeCount,
        result.meshCount,
        result.materialCount,
        result.textureCount,
        result.skeleton.boneCount,
        result.animationCount);
    return result;

load_failed:
    TraceLog(LOG_WARNING, "RFBX: failed to convert '%s'", fileName);
    RfbxUnloadModel(&result);
    return (RfbxModel) { 0 };
}

// Check whether loading and conversion completed successfully.
bool RfbxIsModelValid(const RfbxModel* model)
{
    return model && model->valid;
}

// Return a neutral outdoor-style light suitable for inspecting imported assets.
RfbxLighting RfbxGetDefaultLighting(void)
{
    RfbxLighting lighting = { 0 };
    lighting.direction = Vector3Normalize((Vector3) { -0.5f, -1.0f, -0.35f });
    lighting.color = WHITE;
    lighting.intensity = 0.85f;
    lighting.ambientColor = (Color){ 150, 165, 190, 255 };
    lighting.ambientIntensity = 0.45f;
    return lighting;
}

// Select a caller-owned raylib shader and validate the GPU skinning contract.
bool RfbxSetModelShader(RfbxModel* model, Shader shader)
{
    if (!RfbxIsModelValid(model) ||
        !model->internal ||
        !IsShaderValid(shader) ||
        !shader.locs ||
        shader.locs[SHADER_LOC_MATRIX_MVP] < 0) {
        return false;
    }

    if (RfbxGetGpuSkinnedMeshCount(model) > 0) {
        int boneIndices = GetShaderLocationAttrib(
            shader,
            RFBX_SHADER_ATTRIB_BONE_INDICES);
        int boneWeights = GetShaderLocationAttrib(
            shader,
            RFBX_SHADER_ATTRIB_BONE_WEIGHTS);
        int boneMatrices = GetShaderLocation(
            shader,
            RFBX_SHADER_UNIFORM_BONE_MATRICES "[0]");
        int useSkinning = GetShaderLocation(
            shader,
            RFBX_SHADER_UNIFORM_USE_SKINNING);
        if (boneIndices < 0 ||
            boneWeights < 0 ||
            boneMatrices < 0 ||
            useSkinning < 0) {
            TraceLog(LOG_WARNING,
                "RFBX: custom shader is missing the GPU skinning contract");
            return false;
        }
    }

    int albedoMap = GetShaderLocation(shader, "texture0");
    int metallicMap = GetShaderLocation(shader, "texture1");
    int normalMap = GetShaderLocation(shader, "texture2");
    int roughnessMap = GetShaderLocation(shader, "texture3");
    int emissionMap = GetShaderLocation(shader, "texture5");
    int opacityMap = GetShaderLocation(shader, "texture6");
    if (albedoMap >= 0) {
        shader.locs[SHADER_LOC_MAP_ALBEDO] = albedoMap;
    }
    if (normalMap >= 0) {
        shader.locs[SHADER_LOC_MAP_NORMAL] = normalMap;
    }
    if (metallicMap >= 0) {
        shader.locs[SHADER_LOC_MAP_METALNESS] = metallicMap;
    }
    if (roughnessMap >= 0) {
        shader.locs[SHADER_LOC_MAP_ROUGHNESS] = roughnessMap;
    }
    if (emissionMap >= 0) {
        shader.locs[SHADER_LOC_MAP_EMISSION] = emissionMap;
    }
    if (opacityMap >= 0) {
        shader.locs[SHADER_LOC_MAP_HEIGHT] = opacityMap;
    }
    rfbxCacheActiveShaderLocations(model->internal, shader, true);
    return true;
}

// Restore the model-owned built-in shader without touching a custom shader.
void RfbxResetModelShader(RfbxModel* model)
{
    if (!RfbxIsModelValid(model) || !model->internal) return;
    RfbxInternal* internal = model->internal;
    if (internal->builtinShaderReady) {
        rfbxCacheActiveShaderLocations(
            internal,
            internal->builtinShader,
            false);
    }
    else {
        internal->activeShader = (Shader){ 0 };
        internal->activeShaderReady = false;
        internal->usingCustomShader = false;
    }
}

// Report whether the current draw shader belongs to the caller.
bool RfbxIsUsingCustomShader(const RfbxModel* model)
{
    return RfbxIsModelValid(model) &&
        model->internal &&
        model->internal->usingCustomShader;
}

// Validate and store per-model lighting used by compatible shaders.
bool RfbxSetModelLighting(RfbxModel* model, RfbxLighting lighting)
{
    if (!RfbxIsModelValid(model) || !model->internal ||
        !isfinite(lighting.direction.x) ||
        !isfinite(lighting.direction.y) ||
        !isfinite(lighting.direction.z) ||
        Vector3LengthSqr(lighting.direction) < 0.0000001f ||
        !isfinite(lighting.intensity) ||
        !isfinite(lighting.ambientIntensity) ||
        lighting.intensity < 0.0f ||
        lighting.ambientIntensity < 0.0f) {
        return false;
    }

    lighting.direction = Vector3Normalize(lighting.direction);
    model->internal->lighting = lighting;
    return true;
}

// Return current lighting or the same defaults used during model loading.
RfbxLighting RfbxGetModelLighting(const RfbxModel* model)
{
    if (!RfbxIsModelValid(model) || !model->internal) {
        return RfbxGetDefaultLighting();
    }
    return model->internal->lighting;
}

// Count meshes that passed GPU skinning validation and shader setup.
int RfbxGetGpuSkinnedMeshCount(const RfbxModel* model)
{
    if (!RfbxIsModelValid(model)) return 0;

    int count = 0;
    for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
        if (model->meshes[meshIndex].gpuSkinningReady) ++count;
    }
    return count;
}

// Query the selected skinning path for one mesh.
bool RfbxIsMeshGpuSkinned(const RfbxModel* model, int meshIndex)
{
    return RfbxIsModelValid(model) &&
        meshIndex >= 0 &&
        meshIndex < model->meshCount &&
        model->meshes[meshIndex].gpuSkinningReady;
}

// Return the number of animation stacks in a valid model.
int RfbxGetAnimationCount(const RfbxModel* model)
{
    return RfbxIsModelValid(model) ? model->animationCount : 0;
}

// Return a read-only animation clip by index.
const RfbxAnimation* RfbxGetAnimation(
    const RfbxModel* model,
    int animationIndex)
{
    if (!RfbxIsModelValid(model) ||
        animationIndex < 0 ||
        animationIndex >= model->animationCount) {
        return NULL;
    }

    return &model->animations[animationIndex];
}

// Find an animation using an exact, case-sensitive stack name.
int RfbxFindAnimation(const RfbxModel* model, const char* name)
{
    if (!RfbxIsModelValid(model) || !name) return RFBX_INVALID_INDEX;

    for (int animationIndex = 0;
        animationIndex < model->animationCount;
        ++animationIndex) {
        if (strcmp(model->animations[animationIndex].name, name) == 0) {
            return animationIndex;
        }
    }

    return RFBX_INVALID_INDEX;
}

// Return an event list, allocating one list per animation on first use.
static RfbxAnimationEventListInternal* rfbxGetAnimationEventList(
    RfbxModel* model,
    int animationIndex,
    bool create)
{
    if (!RfbxIsModelValid(model) ||
        !model->internal ||
        animationIndex < 0 ||
        animationIndex >= model->animationCount) {
        return NULL;
    }

    if (!model->internal->animationEventLists && create) {
        model->internal->animationEventLists = rfbxAllocArray(
            (size_t)model->animationCount,
            sizeof(RfbxAnimationEventListInternal));
    }
    return model->internal->animationEventLists
        ? &model->internal->animationEventLists[animationIndex]
        : NULL;
}

// Grow one event list without changing any model-owned event name pointers.
static bool rfbxGrowAnimationEventList(
    RfbxAnimationEventListInternal* list)
{
    if (!list) return false;
    if (list->count < list->capacity) return true;

    if (list->capacity > INT_MAX / 2) return false;
    int newCapacity = list->capacity > 0 ? list->capacity * 2 : 4;
    if ((size_t)newCapacity > (size_t)UINT_MAX /
        sizeof(RfbxAnimationEvent)) {
        return false;
    }

    RfbxAnimationEvent* events = rfbxAllocArray(
        (size_t)newCapacity,
        sizeof(RfbxAnimationEvent));
    if (!events) return false;

    if (list->count > 0) {
        memcpy(events,
            list->events,
            (size_t)list->count * sizeof(RfbxAnimationEvent));
    }
    MemFree(list->events);
    list->events = events;
    list->capacity = newCapacity;
    return true;
}

// Register a game-defined marker and keep each clip list time-sorted.
bool RfbxAddAnimationEvent(
    RfbxModel* model,
    const char* animationName,
    double timeSeconds,
    const char* eventName)
{
    int animationIndex = RfbxFindAnimation(model, animationName);
    if (animationIndex == RFBX_INVALID_INDEX) {
        TraceLog(LOG_WARNING,
            "RFBX: animation '%s' was not found for event '%s'",
            animationName ? animationName : "<null>",
            eventName ? eventName : "<null>");
        return false;
    }
    if (!eventName || eventName[0] == '\0') {
        TraceLog(LOG_WARNING, "RFBX: animation event name is empty");
        return false;
    }

    const RfbxAnimation* animation = RfbxGetAnimation(model, animationIndex);
    if (!animation ||
        !(timeSeconds >= 0.0 && timeSeconds <= animation->duration)) {
        TraceLog(LOG_WARNING,
            "RFBX: event '%s' time %.3f is outside animation '%s'",
            eventName,
            timeSeconds,
            animationName);
        return false;
    }

    RfbxAnimationEventListInternal* list = rfbxGetAnimationEventList(
        model,
        animationIndex,
        true);
    if (!list || !rfbxGrowAnimationEventList(list)) {
        TraceLog(LOG_WARNING,
            "RFBX: failed to allocate animation event '%s'",
            eventName);
        return false;
    }

    char* nameCopy = rfbxCopyString((ufbx_string) {
        eventName,
            strlen(eventName),
    });
    if (!nameCopy) return false;

    int insertIndex = 0;
    while (insertIndex < list->count &&
        list->events[insertIndex].time <= timeSeconds) {
        ++insertIndex;
    }
    if (insertIndex < list->count) {
        memmove(
            &list->events[insertIndex + 1],
            &list->events[insertIndex],
            (size_t)(list->count - insertIndex) *
            sizeof(RfbxAnimationEvent));
    }

    RfbxAnimationEvent* event = &list->events[insertIndex];
    event->name = nameCopy;
    event->animationIndex = animationIndex;
    event->time = timeSeconds;
    event->normalizedTime = animation->duration > 0.0
        ? (float)(timeSeconds / animation->duration)
        : 0.0f;
    ++list->count;
    return true;
}

// Convert normalized clip time to seconds before using the common path.
bool RfbxAddAnimationEventNormalized(
    RfbxModel* model,
    const char* animationName,
    float normalizedTime,
    const char* eventName)
{
    if (!(normalizedTime >= 0.0f && normalizedTime <= 1.0f)) {
        TraceLog(LOG_WARNING,
            "RFBX: normalized event time must be between 0 and 1");
        return false;
    }

    int animationIndex = RfbxFindAnimation(model, animationName);
    const RfbxAnimation* animation = RfbxGetAnimation(
        model,
        animationIndex);
    if (!animation) {
        TraceLog(LOG_WARNING,
            "RFBX: animation '%s' was not found for event '%s'",
            animationName ? animationName : "<null>",
            eventName ? eventName : "<null>");
        return false;
    }

    return RfbxAddAnimationEvent(
        model,
        animationName,
        animation->duration * (double)normalizedTime,
        eventName);
}

// Return the number of markers registered on one named clip.
int RfbxGetAnimationEventCount(
    const RfbxModel* model,
    const char* animationName)
{
    int animationIndex = RfbxFindAnimation(model, animationName);
    if (animationIndex == RFBX_INVALID_INDEX ||
        !model->internal ||
        !model->internal->animationEventLists) {
        return 0;
    }
    return model->internal->animationEventLists[animationIndex].count;
}

// Return one registered marker in chronological order.
const RfbxAnimationEvent* RfbxGetAnimationEvent(
    const RfbxModel* model,
    const char* animationName,
    int eventIndex)
{
    int animationIndex = RfbxFindAnimation(model, animationName);
    if (animationIndex == RFBX_INVALID_INDEX ||
        !model->internal ||
        !model->internal->animationEventLists) {
        return NULL;
    }

    const RfbxAnimationEventListInternal* list =
        &model->internal->animationEventLists[animationIndex];
    return eventIndex >= 0 && eventIndex < list->count
        ? &list->events[eventIndex]
        : NULL;
}

// Return the number of texture definitions retained by a model.
int RfbxGetTextureCount(const RfbxModel* model)
{
    return RfbxIsModelValid(model) ? model->textureCount : 0;
}

// Return a read-only model texture by index.
const RfbxTexture* RfbxGetTexture(const RfbxModel* model, int textureIndex)
{
    if (!RfbxIsModelValid(model) ||
        textureIndex < 0 ||
        textureIndex >= model->textureCount) {
        return NULL;
    }
    return &model->textures[textureIndex];
}

// Return the number of converted material definitions.
int RfbxGetMaterialCount(const RfbxModel* model)
{
    return RfbxIsModelValid(model) ? model->materialCount : 0;
}

// Return a read-only model material by index.
const RfbxMaterial* RfbxGetMaterial(const RfbxModel* model, int materialIndex)
{
    if (!RfbxIsModelValid(model) ||
        materialIndex < 0 ||
        materialIndex >= model->materialCount) {
        return NULL;
    }
    return &model->materials[materialIndex];
}

// Find the first matching material in stable FBX scene order.
int RfbxFindMaterial(const RfbxModel* model, const char* name)
{
    if (!RfbxIsModelValid(model) || !name) return RFBX_INVALID_INDEX;

    for (int materialIndex = 0;
        materialIndex < model->materialCount;
        ++materialIndex) {
        if (strcmp(model->materials[materialIndex].name, name) == 0) {
            return materialIndex;
        }
    }

    return RFBX_INVALID_INDEX;
}

// Resolve an RFBX map to the material's texture index and raylib map slot.
static int* rfbxGetMaterialTextureSlot(
    RfbxMaterial* material,
    RfbxMaterialMapType mapType,
    int* raylibMapIndex)
{
    if (!material || !raylibMapIndex) return NULL;

    switch (mapType) {
    case RFBX_MATERIAL_MAP_BASE_COLOR:
        *raylibMapIndex = MATERIAL_MAP_ALBEDO;
        return &material->baseColorTextureIndex;
    case RFBX_MATERIAL_MAP_NORMAL:
        *raylibMapIndex = MATERIAL_MAP_NORMAL;
        return &material->normalTextureIndex;
    case RFBX_MATERIAL_MAP_METALLIC:
        *raylibMapIndex = MATERIAL_MAP_METALNESS;
        return &material->metallicTextureIndex;
    case RFBX_MATERIAL_MAP_ROUGHNESS:
        *raylibMapIndex = MATERIAL_MAP_ROUGHNESS;
        return &material->roughnessTextureIndex;
    case RFBX_MATERIAL_MAP_EMISSION:
        *raylibMapIndex = MATERIAL_MAP_EMISSION;
        return &material->emissiveTextureIndex;
    case RFBX_MATERIAL_MAP_OPACITY:
        *raylibMapIndex = MATERIAL_MAP_HEIGHT;
        return &material->opacityTextureIndex;
    default:
        return NULL;
    }
}

// Reuse a texture previously added through RfbxSetMaterialTexture().
static int rfbxFindRuntimeTexture(
    const RfbxModel* model,
    const char* fileName)
{
    if (!model || !fileName) return RFBX_INVALID_INDEX;
    for (int textureIndex = 0;
        textureIndex < model->textureCount;
        ++textureIndex) {
        const RfbxTexture* texture = &model->textures[textureIndex];
        if (texture->sourceTypedId == UINT32_MAX &&
            texture->ready &&
            texture->fileName &&
            strcmp(texture->fileName, fileName) == 0) {
            return textureIndex;
        }
    }
    return RFBX_INVALID_INDEX;
}

// Append one caller-selected texture without invalidating GPU resources.
static int rfbxAppendRuntimeTexture(RfbxModel* model, const char* fileName)
{
    if (!model || !fileName || fileName[0] == '\0' ||
        model->textureCount == INT_MAX) {
        return RFBX_INVALID_INDEX;
    }

    RfbxTexture texture = { 0 };
    if (!rfbxTryLoadTexturePath(&texture, fileName)) {
        TraceLog(LOG_WARNING,
            "RFBX: failed to load material texture: %s",
            fileName);
        return RFBX_INVALID_INDEX;
    }

    const char* displayName = GetFileName(fileName);
    if (!displayName || displayName[0] == '\0') displayName = fileName;
    texture.name = rfbxCopyString((ufbx_string) {
        displayName,
            strlen(displayName)
    });
    texture.sourceTypedId = UINT32_MAX;
    if (!texture.name) {
        UnloadTexture(texture.texture);
        MemFree(texture.fileName);
        return RFBX_INVALID_INDEX;
    }

    size_t newCount = (size_t)model->textureCount + 1u;
    RfbxTexture* textures = rfbxAllocArray(newCount, sizeof(RfbxTexture));
    if (!textures) {
        UnloadTexture(texture.texture);
        MemFree(texture.fileName);
        MemFree(texture.name);
        return RFBX_INVALID_INDEX;
    }

    if (model->textureCount > 0) {
        memcpy(
            textures,
            model->textures,
            (size_t)model->textureCount * sizeof(RfbxTexture));
    }
    textures[model->textureCount] = texture;
    MemFree(model->textures);
    model->textures = textures;
    return model->textureCount++;
}

// Load or reuse a texture and assign it to a converted material.
bool RfbxSetMaterialTexture(
    RfbxModel* model,
    int materialIndex,
    RfbxMaterialMapType mapType,
    const char* fileName)
{
    if (!RfbxIsModelValid(model) ||
        materialIndex < 0 ||
        materialIndex >= model->materialCount ||
        !fileName ||
        fileName[0] == '\0') {
        return false;
    }

    RfbxMaterial* material = &model->materials[materialIndex];
    int raylibMapIndex = 0;
    int* textureSlot = rfbxGetMaterialTextureSlot(
        material,
        mapType,
        &raylibMapIndex);
    if (!textureSlot || !material->materialReady) return false;

    int textureIndex = rfbxFindRuntimeTexture(model, fileName);
    if (textureIndex == RFBX_INVALID_INDEX) {
        textureIndex = rfbxAppendRuntimeTexture(model, fileName);
    }
    if (textureIndex == RFBX_INVALID_INDEX) return false;

    *textureSlot = textureIndex;
    material->material.maps[raylibMapIndex].texture =
        model->textures[textureIndex].texture;
    if (mapType == RFBX_MATERIAL_MAP_ROUGHNESS) {
        material->roughnessTextureIsGlossiness = false;
    }
    else if (mapType == RFBX_MATERIAL_MAP_OPACITY &&
        material->alphaMode == RFBX_ALPHA_OPAQUE) {
        material->alphaMode = RFBX_ALPHA_BLEND;
    }
    return true;
}

// Restore the texture state captured immediately after FBX conversion.
bool RfbxClearMaterialTexture(
    RfbxModel* model,
    int materialIndex,
    RfbxMaterialMapType mapType)
{
    if (!RfbxIsModelValid(model) ||
        !model->internal ||
        !model->internal->materialDefaults ||
        materialIndex < 0 ||
        materialIndex >= model->materialCount) {
        return false;
    }

    RfbxMaterial* material = &model->materials[materialIndex];
    int raylibMapIndex = 0;
    int* textureSlot = rfbxGetMaterialTextureSlot(
        material,
        mapType,
        &raylibMapIndex);
    if (!textureSlot || !material->materialReady) return false;

    const RfbxMaterialDefaultState* defaults =
        &model->internal->materialDefaults[materialIndex];
    *textureSlot = defaults->textureIndices[mapType];
    material->material.maps[raylibMapIndex].texture =
        defaults->textures[mapType];

    if (mapType == RFBX_MATERIAL_MAP_ROUGHNESS) {
        material->roughnessTextureIsGlossiness =
            defaults->roughnessTextureIsGlossiness;
    }
    else if (mapType == RFBX_MATERIAL_MAP_OPACITY) {
        material->alphaMode = defaults->alphaMode;
    }
    return true;
}

// Update both the RFBX factor and the raylib material color.
bool RfbxSetMaterialBaseColor(
    RfbxModel* model,
    int materialIndex,
    Color color)
{
    if (!RfbxIsModelValid(model) ||
        materialIndex < 0 ||
        materialIndex >= model->materialCount) {
        return false;
    }

    RfbxMaterial* material = &model->materials[materialIndex];
    if (!material->materialReady) return false;

    material->baseColor = color;
    material->opacity = color.a / 255.0f;
    material->material.maps[MATERIAL_MAP_ALBEDO].color = color;
    if (color.a < 255 && material->alphaMode == RFBX_ALPHA_OPAQUE) {
        material->alphaMode = RFBX_ALPHA_BLEND;
    }
    return true;
}

// Update the scalar consumed by the built-in PBR shader and raylib map.
bool RfbxSetMaterialMetallic(
    RfbxModel* model,
    int materialIndex,
    float metallic)
{
    if (!RfbxIsModelValid(model) ||
        materialIndex < 0 ||
        materialIndex >= model->materialCount ||
        !isfinite(metallic) ||
        metallic < 0.0f ||
        metallic > 1.0f) {
        return false;
    }

    RfbxMaterial* material = &model->materials[materialIndex];
    if (!material->materialReady) return false;
    material->metallic = metallic;
    material->material.maps[MATERIAL_MAP_METALNESS].value = metallic;
    return true;
}

// Update the scalar consumed by the built-in PBR shader and raylib map.
bool RfbxSetMaterialRoughness(
    RfbxModel* model,
    int materialIndex,
    float roughness)
{
    if (!RfbxIsModelValid(model) ||
        materialIndex < 0 ||
        materialIndex >= model->materialCount ||
        !isfinite(roughness) ||
        roughness < 0.0f ||
        roughness > 1.0f) {
        return false;
    }

    RfbxMaterial* material = &model->materials[materialIndex];
    if (!material->materialReady) return false;
    material->roughness = roughness;
    material->material.maps[MATERIAL_MAP_ROUGHNESS].value = roughness;
    return true;
}

// Restore all public material state captured directly after FBX conversion.
bool RfbxResetMaterial(RfbxModel* model, int materialIndex)
{
    if (!RfbxIsModelValid(model) ||
        !model->internal ||
        !model->internal->materialDefaults ||
        materialIndex < 0 ||
        materialIndex >= model->materialCount) {
        return false;
    }

    RfbxMaterial* material = &model->materials[materialIndex];
    if (!material->materialReady) return false;
    const RfbxMaterialDefaultState* defaults =
        &model->internal->materialDefaults[materialIndex];

    material->baseColorTextureIndex =
        defaults->textureIndices[RFBX_MATERIAL_MAP_BASE_COLOR];
    material->normalTextureIndex =
        defaults->textureIndices[RFBX_MATERIAL_MAP_NORMAL];
    material->metallicTextureIndex =
        defaults->textureIndices[RFBX_MATERIAL_MAP_METALLIC];
    material->roughnessTextureIndex =
        defaults->textureIndices[RFBX_MATERIAL_MAP_ROUGHNESS];
    material->emissiveTextureIndex =
        defaults->textureIndices[RFBX_MATERIAL_MAP_EMISSION];
    material->opacityTextureIndex =
        defaults->textureIndices[RFBX_MATERIAL_MAP_OPACITY];

    material->material.maps[MATERIAL_MAP_ALBEDO].texture =
        defaults->textures[RFBX_MATERIAL_MAP_BASE_COLOR];
    material->material.maps[MATERIAL_MAP_NORMAL].texture =
        defaults->textures[RFBX_MATERIAL_MAP_NORMAL];
    material->material.maps[MATERIAL_MAP_METALNESS].texture =
        defaults->textures[RFBX_MATERIAL_MAP_METALLIC];
    material->material.maps[MATERIAL_MAP_ROUGHNESS].texture =
        defaults->textures[RFBX_MATERIAL_MAP_ROUGHNESS];
    material->material.maps[MATERIAL_MAP_EMISSION].texture =
        defaults->textures[RFBX_MATERIAL_MAP_EMISSION];
    material->material.maps[MATERIAL_MAP_HEIGHT].texture =
        defaults->textures[RFBX_MATERIAL_MAP_OPACITY];

    material->baseColor = defaults->baseColor;
    material->emissiveColor = defaults->emissiveColor;
    material->metallic = defaults->metallic;
    material->roughness = defaults->roughness;
    material->opacity = defaults->opacity;
    material->alphaCutoff = defaults->alphaCutoff;
    material->alphaMode = defaults->alphaMode;
    material->doubleSided = defaults->doubleSided;
    material->roughnessTextureIsGlossiness =
        defaults->roughnessTextureIsGlossiness;

    material->material.maps[MATERIAL_MAP_ALBEDO].color =
        defaults->baseColor;
    material->material.maps[MATERIAL_MAP_METALNESS].value =
        defaults->metallic;
    material->material.maps[MATERIAL_MAP_ROUGHNESS].value =
        defaults->roughness;
    material->material.maps[MATERIAL_MAP_EMISSION].color =
        defaults->emissiveColor;
    return true;
}

typedef struct RfbxMaterialFileEntry {
    char* name;
    char* texturePaths[6];
    int materialIndex;
    Color baseColor;
    float metallic;
    float roughness;
    float alphaCutoff;
    RfbxAlphaMode alphaMode;
    bool doubleSided;
    bool hasBaseColor;
    bool hasMetallic;
    bool hasRoughness;
    bool hasAlphaCutoff;
    bool hasAlphaMode;
    bool hasDoubleSided;
} RfbxMaterialFileEntry;

typedef struct RfbxJsonParser {
    const char* begin;
    const char* cursor;
    const char* end;
    bool failed;
} RfbxJsonParser;

// Skip JSON whitespace without accepting comments or non-standard syntax.
static void rfbxJsonSkipWhitespace(RfbxJsonParser* parser)
{
    while (parser->cursor < parser->end) {
        char c = *parser->cursor;
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') break;
        ++parser->cursor;
    }
}

// Consume one structural JSON character after optional whitespace.
static bool rfbxJsonConsume(RfbxJsonParser* parser, char expected)
{
    rfbxJsonSkipWhitespace(parser);
    if (parser->cursor >= parser->end || *parser->cursor != expected) {
        parser->failed = true;
        return false;
    }
    ++parser->cursor;
    return true;
}

static int rfbxJsonHexDigit(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Decode four hexadecimal digits used by a JSON Unicode escape.
static bool rfbxJsonParseHex4(RfbxJsonParser* parser, uint32_t* value)
{
    if (!value || parser->end - parser->cursor < 4) {
        parser->failed = true;
        return false;
    }
    uint32_t result = 0;
    for (int i = 0; i < 4; ++i) {
        int digit = rfbxJsonHexDigit(parser->cursor[i]);
        if (digit < 0) {
            parser->failed = true;
            return false;
        }
        result = result * 16u + (uint32_t)digit;
    }
    parser->cursor += 4;
    *value = result;
    return true;
}

// Append one Unicode code point as UTF-8 into a buffer sized to the input.
static bool rfbxJsonAppendUtf8(char** output, uint32_t codepoint)
{
    char* destination = *output;
    if (codepoint <= 0x7fu) {
        *destination++ = (char)codepoint;
    }
    else if (codepoint <= 0x7ffu) {
        *destination++ = (char)(0xc0u | (codepoint >> 6));
        *destination++ = (char)(0x80u | (codepoint & 0x3fu));
    }
    else if (codepoint <= 0xffffu) {
        *destination++ = (char)(0xe0u | (codepoint >> 12));
        *destination++ = (char)(0x80u | ((codepoint >> 6) & 0x3fu));
        *destination++ = (char)(0x80u | (codepoint & 0x3fu));
    }
    else if (codepoint <= 0x10ffffu) {
        *destination++ = (char)(0xf0u | (codepoint >> 18));
        *destination++ = (char)(0x80u | ((codepoint >> 12) & 0x3fu));
        *destination++ = (char)(0x80u | ((codepoint >> 6) & 0x3fu));
        *destination++ = (char)(0x80u | (codepoint & 0x3fu));
    }
    else {
        return false;
    }
    *output = destination;
    return true;
}

// Parse and allocate a UTF-8 JSON string, including escape decoding.
static char* rfbxJsonParseString(RfbxJsonParser* parser)
{
    rfbxJsonSkipWhitespace(parser);
    if (parser->cursor >= parser->end || *parser->cursor != '"') {
        parser->failed = true;
        return NULL;
    }
    ++parser->cursor;

    size_t maximumLength = (size_t)(parser->end - parser->cursor) + 1u;
    if (maximumLength > UINT_MAX) {
        parser->failed = true;
        return NULL;
    }
    char* result = MemAlloc((unsigned int)maximumLength);
    if (!result) {
        parser->failed = true;
        return NULL;
    }
    char* output = result;

    while (parser->cursor < parser->end) {
        unsigned char c = (unsigned char)*parser->cursor++;
        if (c == '"') {
            *output = '\0';
            return result;
        }
        if (c < 0x20u) break;
        if (c != '\\') {
            *output++ = (char)c;
            continue;
        }
        if (parser->cursor >= parser->end) break;
        char escaped = *parser->cursor++;
        switch (escaped) {
        case '"': *output++ = '"'; break;
        case '\\': *output++ = '\\'; break;
        case '/': *output++ = '/'; break;
        case 'b': *output++ = '\b'; break;
        case 'f': *output++ = '\f'; break;
        case 'n': *output++ = '\n'; break;
        case 'r': *output++ = '\r'; break;
        case 't': *output++ = '\t'; break;
        case 'u': {
            uint32_t codepoint = 0;
            if (!rfbxJsonParseHex4(parser, &codepoint)) break;
            if (codepoint >= 0xd800u && codepoint <= 0xdbffu) {
                if (parser->end - parser->cursor < 6 ||
                    parser->cursor[0] != '\\' ||
                    parser->cursor[1] != 'u') {
                    parser->failed = true;
                    break;
                }
                parser->cursor += 2;
                uint32_t low = 0;
                if (!rfbxJsonParseHex4(parser, &low) ||
                    low < 0xdc00u || low > 0xdfffu) {
                    parser->failed = true;
                    break;
                }
                codepoint = 0x10000u +
                    ((codepoint - 0xd800u) << 10) +
                    (low - 0xdc00u);
            }
            else if (codepoint >= 0xdc00u && codepoint <= 0xdfffu) {
                parser->failed = true;
                break;
            }
            if (codepoint == 0u ||
                !rfbxJsonAppendUtf8(&output, codepoint)) {
                parser->failed = true;
            }
            break;
        }
        default:
            parser->failed = true;
            break;
        }
        if (parser->failed) break;
    }

    MemFree(result);
    parser->failed = true;
    return NULL;
}

// Parse one finite JSON number.
static bool rfbxJsonParseNumber(RfbxJsonParser* parser, double* value)
{
    rfbxJsonSkipWhitespace(parser);
    if (!value || parser->cursor >= parser->end) {
        parser->failed = true;
        return false;
    }
    char* numberEnd = NULL;
    double result = strtod(parser->cursor, &numberEnd);
    if (numberEnd == parser->cursor || numberEnd > parser->end ||
        !isfinite(result)) {
        parser->failed = true;
        return false;
    }
    parser->cursor = numberEnd;
    *value = result;
    return true;
}

// Parse a strict JSON boolean.
static bool rfbxJsonParseBool(RfbxJsonParser* parser, bool* value)
{
    rfbxJsonSkipWhitespace(parser);
    size_t remaining = (size_t)(parser->end - parser->cursor);
    if (remaining >= 4 && memcmp(parser->cursor, "true", 4) == 0) {
        parser->cursor += 4;
        *value = true;
        return true;
    }
    if (remaining >= 5 && memcmp(parser->cursor, "false", 5) == 0) {
        parser->cursor += 5;
        *value = false;
        return true;
    }
    parser->failed = true;
    return false;
}

// Parse four normalized color components and convert them to raylib Color.
static bool rfbxJsonParseColor(RfbxJsonParser* parser, Color* color)
{
    if (!color || !rfbxJsonConsume(parser, '[')) return false;
    double values[4] = { 0 };
    for (int i = 0; i < 4; ++i) {
        if (!rfbxJsonParseNumber(parser, &values[i]) ||
            values[i] < 0.0 || values[i] > 1.0) {
            parser->failed = true;
            return false;
        }
        if (i < 3 && !rfbxJsonConsume(parser, ',')) return false;
    }
    if (!rfbxJsonConsume(parser, ']')) return false;
    *color = (Color){
        (unsigned char)(values[0] * 255.0 + 0.5),
        (unsigned char)(values[1] * 255.0 + 0.5),
        (unsigned char)(values[2] * 255.0 + 0.5),
        (unsigned char)(values[3] * 255.0 + 0.5)
    };
    return true;
}

static int rfbxMaterialFileTextureKey(const char* key)
{
    if (strcmp(key, "baseColorTexture") == 0) {
        return RFBX_MATERIAL_MAP_BASE_COLOR;
    }
    if (strcmp(key, "normalTexture") == 0) {
        return RFBX_MATERIAL_MAP_NORMAL;
    }
    if (strcmp(key, "metallicTexture") == 0) {
        return RFBX_MATERIAL_MAP_METALLIC;
    }
    if (strcmp(key, "roughnessTexture") == 0) {
        return RFBX_MATERIAL_MAP_ROUGHNESS;
    }
    if (strcmp(key, "emissiveTexture") == 0) {
        return RFBX_MATERIAL_MAP_EMISSION;
    }
    if (strcmp(key, "opacityTexture") == 0) {
        return RFBX_MATERIAL_MAP_OPACITY;
    }
    return RFBX_INVALID_INDEX;
}

// Parse one named material object from the version 1 sidecar schema.
static bool rfbxJsonParseMaterialEntry(
    RfbxJsonParser* parser,
    RfbxMaterialFileEntry* entry)
{
    if (!entry || !rfbxJsonConsume(parser, '{')) return false;
    rfbxJsonSkipWhitespace(parser);
    if (parser->cursor < parser->end && *parser->cursor == '}') {
        ++parser->cursor;
        return true;
    }

    for (;;) {
        char* key = rfbxJsonParseString(parser);
        if (!key || !rfbxJsonConsume(parser, ':')) {
            MemFree(key);
            return false;
        }

        bool propertyValid = true;
        int textureMap = rfbxMaterialFileTextureKey(key);
        if (textureMap != RFBX_INVALID_INDEX) {
            if (entry->texturePaths[textureMap]) {
                propertyValid = false;
            }
            else {
                entry->texturePaths[textureMap] =
                    rfbxJsonParseString(parser);
                propertyValid = entry->texturePaths[textureMap] &&
                    entry->texturePaths[textureMap][0] != '\0';
            }
        }
        else if (strcmp(key, "shader") == 0) {
            char* shader = rfbxJsonParseString(parser);
            propertyValid = shader && strcmp(shader, "rfbx_pbr") == 0;
            MemFree(shader);
        }
        else if (strcmp(key, "baseColor") == 0 &&
            !entry->hasBaseColor) {
            propertyValid = rfbxJsonParseColor(parser, &entry->baseColor);
            entry->hasBaseColor = propertyValid;
        }
        else if (strcmp(key, "metallic") == 0 &&
            !entry->hasMetallic) {
            double value = 0.0;
            propertyValid = rfbxJsonParseNumber(parser, &value) &&
                value >= 0.0 && value <= 1.0;
            entry->metallic = (float)value;
            entry->hasMetallic = propertyValid;
        }
        else if (strcmp(key, "roughness") == 0 &&
            !entry->hasRoughness) {
            double value = 0.0;
            propertyValid = rfbxJsonParseNumber(parser, &value) &&
                value >= 0.0 && value <= 1.0;
            entry->roughness = (float)value;
            entry->hasRoughness = propertyValid;
        }
        else if (strcmp(key, "alphaCutoff") == 0 &&
            !entry->hasAlphaCutoff) {
            double value = 0.0;
            propertyValid = rfbxJsonParseNumber(parser, &value) &&
                value >= 0.0 && value <= 1.0;
            entry->alphaCutoff = (float)value;
            entry->hasAlphaCutoff = propertyValid;
        }
        else if (strcmp(key, "alphaMode") == 0 &&
            !entry->hasAlphaMode) {
            char* mode = rfbxJsonParseString(parser);
            if (mode && strcmp(mode, "opaque") == 0) {
                entry->alphaMode = RFBX_ALPHA_OPAQUE;
            }
            else if (mode && strcmp(mode, "mask") == 0) {
                entry->alphaMode = RFBX_ALPHA_MASK;
            }
            else if (mode && strcmp(mode, "blend") == 0) {
                entry->alphaMode = RFBX_ALPHA_BLEND;
            }
            else {
                propertyValid = false;
            }
            entry->hasAlphaMode = propertyValid;
            MemFree(mode);
        }
        else if (strcmp(key, "doubleSided") == 0 &&
            !entry->hasDoubleSided) {
            propertyValid = rfbxJsonParseBool(
                parser,
                &entry->doubleSided);
            entry->hasDoubleSided = propertyValid;
        }
        else {
            propertyValid = false;
        }
        MemFree(key);
        if (!propertyValid || parser->failed) {
            parser->failed = true;
            return false;
        }

        rfbxJsonSkipWhitespace(parser);
        if (parser->cursor < parser->end && *parser->cursor == '}') {
            ++parser->cursor;
            return true;
        }
        if (!rfbxJsonConsume(parser, ',')) return false;
    }
}

static void rfbxFreeMaterialFileEntries(
    RfbxMaterialFileEntry* entries,
    int entryCount)
{
    for (int entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
        MemFree(entries[entryIndex].name);
        for (int mapIndex = 0; mapIndex < 6; ++mapIndex) {
            MemFree(entries[entryIndex].texturePaths[mapIndex]);
        }
    }
    MemFree(entries);
}

// Grow the parsed entry list without retaining pointers into temporary text.
static bool rfbxGrowMaterialFileEntries(
    RfbxMaterialFileEntry** entries,
    int* capacity,
    int count)
{
    if (count < *capacity) return true;
    if (*capacity > INT_MAX / 2) return false;
    int newCapacity = *capacity > 0 ? *capacity * 2 : 4;
    RfbxMaterialFileEntry* newEntries = rfbxAllocArray(
        (size_t)newCapacity,
        sizeof(RfbxMaterialFileEntry));
    if (!newEntries) return false;
    if (count > 0) {
        memcpy(
            newEntries,
            *entries,
            (size_t)count * sizeof(RfbxMaterialFileEntry));
    }
    MemFree(*entries);
    *entries = newEntries;
    *capacity = newCapacity;
    return true;
}

// Parse the complete version 1 .rfbxmat root object.
static bool rfbxParseMaterialFile(
    const char* text,
    RfbxMaterialFileEntry** entries,
    int* entryCount,
    size_t* errorOffset)
{
    RfbxJsonParser parser = { text, text, text + strlen(text), false };
    int capacity = 0;
    bool hasVersion = false;
    bool hasMaterials = false;
    double version = 0.0;
    *entries = NULL;
    *entryCount = 0;

    if (!rfbxJsonConsume(&parser, '{')) goto parse_failed;
    rfbxJsonSkipWhitespace(&parser);
    if (parser.cursor < parser.end && *parser.cursor == '}') {
        parser.failed = true;
        goto parse_failed;
    }

    for (;;) {
        char* key = rfbxJsonParseString(&parser);
        if (!key || !rfbxJsonConsume(&parser, ':')) {
            MemFree(key);
            goto parse_failed;
        }

        if (strcmp(key, "version") == 0 && !hasVersion) {
            hasVersion = rfbxJsonParseNumber(&parser, &version);
        }
        else if (strcmp(key, "materials") == 0 && !hasMaterials) {
            hasMaterials = true;
            if (!rfbxJsonConsume(&parser, '{')) {
                MemFree(key);
                goto parse_failed;
            }
            rfbxJsonSkipWhitespace(&parser);
            if (parser.cursor < parser.end && *parser.cursor == '}') {
                ++parser.cursor;
            }
            else {
                for (;;) {
                    char* materialName = rfbxJsonParseString(&parser);
                    if (!materialName || !rfbxJsonConsume(&parser, ':')) {
                        MemFree(materialName);
                        MemFree(key);
                        goto parse_failed;
                    }
                    for (int i = 0; i < *entryCount; ++i) {
                        if (strcmp((*entries)[i].name, materialName) == 0) {
                            parser.failed = true;
                            break;
                        }
                    }
                    if (parser.failed || !rfbxGrowMaterialFileEntries(
                        entries,
                        &capacity,
                        *entryCount)) {
                        MemFree(materialName);
                        MemFree(key);
                        goto parse_failed;
                    }
                    RfbxMaterialFileEntry* entry =
                        &(*entries)[(*entryCount)++];
                    entry->name = materialName;
                    entry->materialIndex = RFBX_INVALID_INDEX;
                    if (!rfbxJsonParseMaterialEntry(&parser, entry)) {
                        MemFree(key);
                        goto parse_failed;
                    }

                    rfbxJsonSkipWhitespace(&parser);
                    if (parser.cursor < parser.end &&
                        *parser.cursor == '}') {
                        ++parser.cursor;
                        break;
                    }
                    if (!rfbxJsonConsume(&parser, ',')) {
                        MemFree(key);
                        goto parse_failed;
                    }
                }
            }
        }
        else {
            parser.failed = true;
        }
        MemFree(key);
        if (parser.failed) goto parse_failed;

        rfbxJsonSkipWhitespace(&parser);
        if (parser.cursor < parser.end && *parser.cursor == '}') {
            ++parser.cursor;
            break;
        }
        if (!rfbxJsonConsume(&parser, ',')) goto parse_failed;
    }

    rfbxJsonSkipWhitespace(&parser);
    if (parser.cursor != parser.end ||
        !hasVersion ||
        version != 1.0 ||
        !hasMaterials) {
        parser.failed = true;
        goto parse_failed;
    }
    return true;

parse_failed:
    if (errorOffset) *errorOffset = (size_t)(parser.cursor - parser.begin);
    rfbxFreeMaterialFileEntries(*entries, *entryCount);
    *entries = NULL;
    *entryCount = 0;
    return false;
}

static bool rfbxIsAbsolutePath(const char* path)
{
    if (!path || path[0] == '\0') return false;
    if (path[0] == '/' || path[0] == '\\') return true;
    return path[1] == ':' &&
        ((path[0] >= 'A' && path[0] <= 'Z') ||
            (path[0] >= 'a' && path[0] <= 'z'));
}

// Resolve texture references relative to the .rfbxmat location.
static char* rfbxResolveMaterialTexturePath(
    const char* materialFileName,
    const char* texturePath)
{
    if (rfbxIsAbsolutePath(texturePath)) {
        return rfbxCopyString((ufbx_string) {
            texturePath,
                strlen(texturePath)
        });
    }

    const char* directory = GetDirectoryPath(materialFileName);
    size_t directoryLength = directory ? strlen(directory) : 0u;
    size_t textureLength = strlen(texturePath);
    bool needsSeparator = directoryLength > 0 &&
        directory[directoryLength - 1u] != '/' &&
        directory[directoryLength - 1u] != '\\';
    size_t totalLength = directoryLength +
        (needsSeparator ? 1u : 0u) + textureLength;
    if (totalLength >= UINT_MAX) return NULL;

    char* result = MemAlloc((unsigned int)totalLength + 1u);
    if (!result) return NULL;
    size_t offset = 0;
    if (directoryLength > 0) {
        memcpy(result, directory, directoryLength);
        offset += directoryLength;
    }
    if (needsSeparator) result[offset++] = '/';
    memcpy(result + offset, texturePath, textureLength);
    result[totalLength] = '\0';
    return result;
}

// Load, validate, and apply a strict version 1 RFBX material sidecar.
bool RfbxLoadMaterialFile(RfbxModel* model, const char* fileName)
{
    if (!RfbxIsModelValid(model) || !fileName || fileName[0] == '\0') {
        return false;
    }

    char* text = LoadFileText(fileName);
    if (!text) {
        TraceLog(LOG_WARNING,
            "RFBX: material file could not be read: %s",
            fileName);
        return false;
    }

    RfbxMaterialFileEntry* entries = NULL;
    int entryCount = 0;
    size_t errorOffset = 0;
    bool success = rfbxParseMaterialFile(
        text,
        &entries,
        &entryCount,
        &errorOffset);
    UnloadFileText(text);
    if (!success) {
        TraceLog(LOG_WARNING,
            "RFBX: invalid material JSON at byte %u: %s",
            errorOffset > UINT_MAX ? UINT_MAX : (unsigned int)errorOffset,
            fileName);
        return false;
    }

    // Validate every name and path before changing any material state.
    bool applyStarted = false;
    for (int entryIndex = 0; entryIndex < entryCount && success; ++entryIndex) {
        RfbxMaterialFileEntry* entry = &entries[entryIndex];
        entry->materialIndex = RfbxFindMaterial(model, entry->name);
        if (entry->materialIndex == RFBX_INVALID_INDEX) {
            TraceLog(LOG_WARNING,
                "RFBX: material '%s' from '%s' was not found",
                entry->name,
                fileName);
            success = false;
            break;
        }
        for (int mapIndex = 0; mapIndex < 6; ++mapIndex) {
            if (!entry->texturePaths[mapIndex]) continue;
            char* resolved = rfbxResolveMaterialTexturePath(
                fileName,
                entry->texturePaths[mapIndex]);
            if (!resolved || !FileExists(resolved)) {
                TraceLog(LOG_WARNING,
                    "RFBX: material texture was not found: %s",
                    resolved ? resolved : entry->texturePaths[mapIndex]);
                MemFree(resolved);
                success = false;
                break;
            }
            MemFree(entry->texturePaths[mapIndex]);
            entry->texturePaths[mapIndex] = resolved;
        }
    }

    if (success) applyStarted = true;
    for (int entryIndex = 0; entryIndex < entryCount && success; ++entryIndex) {
        RfbxMaterialFileEntry* entry = &entries[entryIndex];
        success = RfbxResetMaterial(model, entry->materialIndex);
        if (success && entry->hasBaseColor) {
            success = RfbxSetMaterialBaseColor(
                model,
                entry->materialIndex,
                entry->baseColor);
        }
        if (success && entry->hasMetallic) {
            success = RfbxSetMaterialMetallic(
                model,
                entry->materialIndex,
                entry->metallic);
        }
        if (success && entry->hasRoughness) {
            success = RfbxSetMaterialRoughness(
                model,
                entry->materialIndex,
                entry->roughness);
        }
        for (int mapIndex = 0; mapIndex < 6 && success; ++mapIndex) {
            if (entry->texturePaths[mapIndex]) {
                success = RfbxSetMaterialTexture(
                    model,
                    entry->materialIndex,
                    (RfbxMaterialMapType)mapIndex,
                    entry->texturePaths[mapIndex]);
            }
        }
        if (success && (entry->hasAlphaMode || entry->hasAlphaCutoff)) {
            const RfbxMaterial* material = RfbxGetMaterial(
                model,
                entry->materialIndex);
            success = material && RfbxSetMaterialAlphaMode(
                model,
                entry->materialIndex,
                entry->hasAlphaMode
                ? entry->alphaMode
                : material->alphaMode,
                entry->hasAlphaCutoff
                ? entry->alphaCutoff
                : material->alphaCutoff);
        }
        if (success && entry->hasDoubleSided) {
            success = RfbxSetMaterialDoubleSided(
                model,
                entry->materialIndex,
                entry->doubleSided);
        }
    }

    if (!success) {
        if (applyStarted) {
            for (int entryIndex = 0; entryIndex < entryCount; ++entryIndex) {
                if (entries[entryIndex].materialIndex != RFBX_INVALID_INDEX) {
                    RfbxResetMaterial(model, entries[entryIndex].materialIndex);
                }
            }
        }
        TraceLog(LOG_WARNING,
            "RFBX: failed to apply material file: %s",
            fileName);
    }
    else {
        TraceLog(LOG_INFO,
            "RFBX: loaded material file '%s' (%d materials)",
            fileName,
            entryCount);
    }

    rfbxFreeMaterialFileEntries(entries, entryCount);
    return success;
}

// Replace the model extension, or append one when the model has no extension.
static char* rfbxGetAutomaticMaterialFileName(const char* modelFileName)
{
    static const char extension[] = ".rfbxmat";
    if (!modelFileName || modelFileName[0] == '\0') return NULL;

    const char* name = modelFileName;
    const char* separator = strrchr(modelFileName, '/');
    const char* backslash = strrchr(modelFileName, '\\');
    if (!separator || (backslash && backslash > separator)) {
        separator = backslash;
    }
    if (separator) name = separator + 1;

    const char* dot = strrchr(name, '.');
    if (dot == name) dot = NULL;
    size_t stemLength = dot
        ? (size_t)(dot - modelFileName)
        : strlen(modelFileName);
    size_t extensionLength = sizeof(extension) - 1u;
    if (stemLength > UINT_MAX - extensionLength - 1u) return NULL;

    size_t resultLength = stemLength + extensionLength;
    char* result = MemAlloc((unsigned int)resultLength + 1u);
    if (!result) return NULL;
    memcpy(result, modelFileName, stemLength);
    memcpy(result + stemLength, extension, extensionLength + 1u);
    return result;
}

// Load the FBX normally and treat an existing matching sidecar as authoritative.
RfbxModel RfbxLoadModelWithMaterial(const char* fileName)
{
    RfbxModel model = RfbxLoadModel(fileName);
    if (!RfbxIsModelValid(&model)) return model;

    char* materialFileName = rfbxGetAutomaticMaterialFileName(fileName);
    if (!materialFileName) {
        TraceLog(LOG_WARNING,
            "RFBX: failed to build the automatic material file name");
        RfbxUnloadModel(&model);
        return (RfbxModel) { 0 };
    }

    bool hasMaterialFile = FileExists(materialFileName);
    bool materialLoaded = !hasMaterialFile ||
        RfbxLoadMaterialFile(&model, materialFileName);
    if (hasMaterialFile && !materialLoaded) {
        TraceLog(LOG_WARNING,
            "RFBX: matching material file is invalid: %s",
            materialFileName);
        RfbxUnloadModel(&model);
        model = (RfbxModel){ 0 };
    }
    MemFree(materialFileName);
    return model;
}

typedef struct RfbxJsonWriter {
    char* data;
    size_t length;
    size_t capacity;
    bool failed;
} RfbxJsonWriter;

// Grow a JSON output buffer while respecting raylib's unsigned allocation API.
static bool rfbxJsonWriterReserve(
    RfbxJsonWriter* writer,
    size_t additionalLength)
{
    if (!writer || writer->failed ||
        additionalLength > SIZE_MAX - writer->length - 1u) {
        if (writer) writer->failed = true;
        return false;
    }

    size_t required = writer->length + additionalLength + 1u;
    if (required <= writer->capacity) return true;
    if (required > UINT_MAX) {
        writer->failed = true;
        return false;
    }

    size_t capacity = writer->capacity > 0 ? writer->capacity : 1024u;
    while (capacity < required) {
        if (capacity > (size_t)UINT_MAX / 2u) {
            capacity = required;
            break;
        }
        capacity *= 2u;
    }

    char* data = MemAlloc((unsigned int)capacity);
    if (!data) {
        writer->failed = true;
        return false;
    }
    if (writer->length > 0) {
        memcpy(data, writer->data, writer->length);
    }
    MemFree(writer->data);
    writer->data = data;
    writer->capacity = capacity;
    writer->data[writer->length] = '\0';
    return true;
}

static bool rfbxJsonWriterAppendBytes(
    RfbxJsonWriter* writer,
    const char* text,
    size_t length)
{
    if (!text || !rfbxJsonWriterReserve(writer, length)) return false;
    memcpy(writer->data + writer->length, text, length);
    writer->length += length;
    writer->data[writer->length] = '\0';
    return true;
}

static bool rfbxJsonWriterAppend(
    RfbxJsonWriter* writer,
    const char* text)
{
    return text && rfbxJsonWriterAppendBytes(writer, text, strlen(text));
}

// Write one UTF-8 string with the escapes required by strict JSON.
static bool rfbxJsonWriterAppendString(
    RfbxJsonWriter* writer,
    const char* text)
{
    static const char hex[] = "0123456789abcdef";
    if (!text || !rfbxJsonWriterAppend(writer, "\"")) return false;

    const unsigned char* cursor = (const unsigned char*)text;
    while (*cursor && !writer->failed) {
        unsigned char c = *cursor++;
        switch (c) {
        case '"': rfbxJsonWriterAppend(writer, "\\\""); break;
        case '\\': rfbxJsonWriterAppend(writer, "\\\\"); break;
        case '\b': rfbxJsonWriterAppend(writer, "\\b"); break;
        case '\f': rfbxJsonWriterAppend(writer, "\\f"); break;
        case '\n': rfbxJsonWriterAppend(writer, "\\n"); break;
        case '\r': rfbxJsonWriterAppend(writer, "\\r"); break;
        case '\t': rfbxJsonWriterAppend(writer, "\\t"); break;
        default:
            if (c < 0x20u) {
                char escaped[6] = {
                    '\\', 'u', '0', '0',
                    hex[(c >> 4) & 0x0fu],
                    hex[c & 0x0fu]
                };
                rfbxJsonWriterAppendBytes(writer, escaped, sizeof(escaped));
            }
            else {
                char byte = (char)c;
                rfbxJsonWriterAppendBytes(writer, &byte, 1u);
            }
            break;
        }
    }
    return rfbxJsonWriterAppend(writer, "\"");
}

// Format a finite normalized value without depending on the process locale.
static bool rfbxJsonWriterAppendNumber(
    RfbxJsonWriter* writer,
    double value)
{
    if (!isfinite(value)) {
        writer->failed = true;
        return false;
    }
    char number[64] = { 0 };
    int length = snprintf(number, sizeof(number), "%.9g", value);
    if (length <= 0 || (size_t)length >= sizeof(number)) {
        writer->failed = true;
        return false;
    }
    for (int i = 0; i < length; ++i) {
        if (number[i] == ',') number[i] = '.';
    }
    return rfbxJsonWriterAppendBytes(writer, number, (size_t)length);
}

// Return a normalized absolute path using forward slashes.
static char* rfbxNormalizeAbsolutePath(const char* path)
{
    if (!path || path[0] == '\0') return NULL;

    const char* workingDirectory = GetWorkingDirectory();
    size_t pathLength = strlen(path);
    size_t workingLength = workingDirectory
        ? strlen(workingDirectory)
        : 0u;
    bool absolute = rfbxIsAbsolutePath(path);
    size_t combinedLength = pathLength;
    if (!absolute) {
        if (workingLength == 0 ||
            workingLength > SIZE_MAX - pathLength - 1u) {
            return NULL;
        }
        combinedLength = workingLength + 1u + pathLength;
    }
    if (combinedLength > UINT_MAX - 1u) return NULL;

    char* scratch = MemAlloc((unsigned int)combinedLength + 1u);
    if (!scratch) return NULL;
    size_t offset = 0;
    if (!absolute) {
        memcpy(scratch, workingDirectory, workingLength);
        offset = workingLength;
        scratch[offset++] = '/';
    }
    memcpy(scratch + offset, path, pathLength);
    scratch[combinedLength] = '\0';
    for (size_t i = 0; i < combinedLength; ++i) {
        if (scratch[i] == '\\') scratch[i] = '/';
    }

    bool driveRoot = combinedLength >= 2u && scratch[1] == ':';
    bool uncRoot = combinedLength >= 2u &&
        scratch[0] == '/' && scratch[1] == '/';
    size_t cursor = driveRoot ? 2u : (uncRoot ? 2u : 0u);
    while (cursor < combinedLength && scratch[cursor] == '/') ++cursor;

    size_t componentCapacity = combinedLength / 2u + 2u;
    char** components = rfbxAllocArray(
        componentCapacity,
        sizeof(char*));
    if (!components) {
        MemFree(scratch);
        return NULL;
    }
    size_t componentCount = 0;
    size_t protectedComponents = uncRoot ? 2u : 0u;
    while (cursor < combinedLength) {
        size_t begin = cursor;
        while (cursor < combinedLength && scratch[cursor] != '/') ++cursor;
        if (cursor < combinedLength) scratch[cursor++] = '\0';
        while (cursor < combinedLength && scratch[cursor] == '/') ++cursor;

        char* component = scratch + begin;
        if (component[0] == '\0' || strcmp(component, ".") == 0) continue;
        if (strcmp(component, "..") == 0) {
            if (componentCount > protectedComponents) --componentCount;
            continue;
        }
        components[componentCount++] = component;
    }

    if (combinedLength > UINT_MAX - 4u) {
        MemFree(components);
        MemFree(scratch);
        return NULL;
    }
    char* result = MemAlloc((unsigned int)combinedLength + 4u);
    if (!result) {
        MemFree(components);
        MemFree(scratch);
        return NULL;
    }

    size_t resultLength = 0;
    if (driveRoot) {
        result[resultLength++] = scratch[0];
        result[resultLength++] = ':';
        result[resultLength++] = '/';
    }
    else if (uncRoot) {
        result[resultLength++] = '/';
        result[resultLength++] = '/';
    }
    else {
        result[resultLength++] = '/';
    }
    for (size_t i = 0; i < componentCount; ++i) {
        if (resultLength > 0 && result[resultLength - 1u] != '/') {
            result[resultLength++] = '/';
        }
        size_t componentLength = strlen(components[i]);
        memcpy(result + resultLength, components[i], componentLength);
        resultLength += componentLength;
    }
    result[resultLength] = '\0';

    MemFree(components);
    MemFree(scratch);
    return result;
}

static size_t rfbxRelativePathRootLength(
    const char* path,
    bool* caseInsensitive)
{
    *caseInsensitive = false;
    if (path[0] == '/' && path[1] == '/') return 0u;
    if (path[0] == '/') return 1u;
    if (path[0] != '\0' && path[1] == ':' && path[2] == '/') {
        *caseInsensitive = true;
        return 3u;
    }
    return 0u;
}

static bool rfbxPathSegmentEqual(
    const char* left,
    size_t leftLength,
    const char* right,
    size_t rightLength,
    bool caseInsensitive)
{
    if (leftLength != rightLength) return false;
    for (size_t i = 0; i < leftLength; ++i) {
        char a = left[i];
        char b = right[i];
        if (caseInsensitive) {
            if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
            if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        }
        if (a != b) return false;
    }
    return true;
}

// Make a portable texture path relative to the sidecar when roots match.
static char* rfbxMakeMaterialTexturePath(
    const char* materialFileName,
    const char* textureFileName)
{
    char* materialPath = rfbxNormalizeAbsolutePath(materialFileName);
    char* texturePath = rfbxNormalizeAbsolutePath(textureFileName);
    if (!materialPath || !texturePath) {
        MemFree(materialPath);
        MemFree(texturePath);
        return NULL;
    }

    char* lastSeparator = strrchr(materialPath, '/');
    if (!lastSeparator) {
        MemFree(materialPath);
        return texturePath;
    }
    if (lastSeparator == materialPath ||
        (lastSeparator == materialPath + 2 && materialPath[1] == ':')) {
        lastSeparator[1] = '\0';
    }
    else {
        *lastSeparator = '\0';
    }

    bool baseInsensitive = false;
    bool textureInsensitive = false;
    size_t baseRoot = rfbxRelativePathRootLength(
        materialPath,
        &baseInsensitive);
    size_t textureRoot = rfbxRelativePathRootLength(
        texturePath,
        &textureInsensitive);
    bool compatible = baseRoot > 0u && textureRoot > 0u &&
        baseInsensitive == textureInsensitive;
    if (compatible && baseInsensitive) {
        compatible = rfbxPathSegmentEqual(
            materialPath,
            baseRoot,
            texturePath,
            textureRoot,
            true);
    }
    if (!compatible) {
        MemFree(materialPath);
        return texturePath;
    }

    const char* baseCursor = materialPath + baseRoot;
    const char* textureCursor = texturePath + textureRoot;
    while (*baseCursor && *textureCursor) {
        const char* baseEnd = strchr(baseCursor, '/');
        const char* textureEnd = strchr(textureCursor, '/');
        if (!baseEnd) baseEnd = baseCursor + strlen(baseCursor);
        if (!textureEnd) textureEnd = textureCursor + strlen(textureCursor);
        if (!rfbxPathSegmentEqual(
            baseCursor,
            (size_t)(baseEnd - baseCursor),
            textureCursor,
            (size_t)(textureEnd - textureCursor),
            baseInsensitive)) {
            break;
        }
        baseCursor = *baseEnd ? baseEnd + 1 : baseEnd;
        textureCursor = *textureEnd ? textureEnd + 1 : textureEnd;
    }

    size_t parentCount = 0;
    const char* remainingBase = baseCursor;
    while (*remainingBase) {
        ++parentCount;
        const char* separator = strchr(remainingBase, '/');
        if (!separator) break;
        remainingBase = separator + 1;
    }

    size_t textureRemainderLength = strlen(textureCursor);
    if (parentCount > (SIZE_MAX - textureRemainderLength - 1u) / 3u) {
        MemFree(materialPath);
        MemFree(texturePath);
        return NULL;
    }
    size_t resultLength = parentCount * 3u + textureRemainderLength;
    if (resultLength == 0u || resultLength > UINT_MAX - 1u) {
        MemFree(materialPath);
        MemFree(texturePath);
        return NULL;
    }

    char* result = MemAlloc((unsigned int)resultLength + 1u);
    if (!result) {
        MemFree(materialPath);
        MemFree(texturePath);
        return NULL;
    }
    char* output = result;
    for (size_t i = 0; i < parentCount; ++i) {
        memcpy(output, "../", 3u);
        output += 3;
    }
    memcpy(output, textureCursor, textureRemainderLength);
    result[resultLength] = '\0';

    MemFree(materialPath);
    MemFree(texturePath);
    return result;
}

// Return current texture indices in RfbxMaterialMapType order.
static void rfbxGetMaterialTextureIndices(
    const RfbxMaterial* material,
    int indices[6])
{
    indices[RFBX_MATERIAL_MAP_BASE_COLOR] =
        material->baseColorTextureIndex;
    indices[RFBX_MATERIAL_MAP_NORMAL] =
        material->normalTextureIndex;
    indices[RFBX_MATERIAL_MAP_METALLIC] =
        material->metallicTextureIndex;
    indices[RFBX_MATERIAL_MAP_ROUGHNESS] =
        material->roughnessTextureIndex;
    indices[RFBX_MATERIAL_MAP_EMISSION] =
        material->emissiveTextureIndex;
    indices[RFBX_MATERIAL_MAP_OPACITY] =
        material->opacityTextureIndex;
}

// Save all supported values and runtime texture overrides to a sidecar.
bool RfbxSaveMaterialFile(const RfbxModel* model, const char* fileName)
{
    static const char* textureKeys[6] = {
        "baseColorTexture",
        "normalTexture",
        "metallicTexture",
        "roughnessTexture",
        "emissiveTexture",
        "opacityTexture"
    };
    static const char* alphaModes[3] = {
        "opaque", "mask", "blend"
    };

    if (!RfbxIsModelValid(model) || !fileName || fileName[0] == '\0' ||
        !model->internal ||
        (model->materialCount > 0 && !model->internal->materialDefaults)) {
        return false;
    }

    // Material names are JSON object keys, so duplicates cannot round-trip.
    for (int materialIndex = 0;
        materialIndex < model->materialCount;
        ++materialIndex) {
        const RfbxMaterial* material = &model->materials[materialIndex];
        if (!material->name || !material->materialReady ||
            !isfinite(material->metallic) ||
            material->metallic < 0.0f || material->metallic > 1.0f ||
            !isfinite(material->roughness) ||
            material->roughness < 0.0f || material->roughness > 1.0f ||
            !isfinite(material->alphaCutoff) ||
            material->alphaCutoff < 0.0f || material->alphaCutoff > 1.0f ||
            material->alphaMode < RFBX_ALPHA_OPAQUE ||
            material->alphaMode > RFBX_ALPHA_BLEND) {
            TraceLog(LOG_WARNING,
                "RFBX: material %d contains values that cannot be saved",
                materialIndex);
            return false;
        }
        for (int previous = 0; previous < materialIndex; ++previous) {
            if (strcmp(model->materials[previous].name, material->name) == 0) {
                TraceLog(LOG_WARNING,
                    "RFBX: duplicate material name cannot be saved: %s",
                    material->name);
                return false;
            }
        }

        int textureIndices[6] = { 0 };
        rfbxGetMaterialTextureIndices(material, textureIndices);
        const RfbxMaterialDefaultState* defaults =
            &model->internal->materialDefaults[materialIndex];
        for (int mapIndex = 0; mapIndex < 6; ++mapIndex) {
            int textureIndex = textureIndices[mapIndex];
            if (textureIndex == defaults->textureIndices[mapIndex]) continue;
            if (textureIndex < 0 || textureIndex >= model->textureCount) {
                TraceLog(LOG_WARNING,
                    "RFBX: material '%s' has an invalid texture override",
                    material->name);
                return false;
            }
            const RfbxTexture* texture = &model->textures[textureIndex];
            if (!texture->ready || !texture->fileName ||
                texture->fileName[0] == '\0' ||
                !FileExists(texture->fileName)) {
                TraceLog(LOG_WARNING,
                    "RFBX: material texture cannot be saved: %s",
                    texture->fileName ? texture->fileName : "<unnamed>");
                return false;
            }
        }
    }

    RfbxJsonWriter writer = { 0 };
    rfbxJsonWriterAppend(&writer,
        "{\n  \"version\": 1,\n  \"materials\": {");

    for (int materialIndex = 0;
        materialIndex < model->materialCount && !writer.failed;
        ++materialIndex) {
        const RfbxMaterial* material = &model->materials[materialIndex];
        if (materialIndex > 0) rfbxJsonWriterAppend(&writer, ",");
        rfbxJsonWriterAppend(&writer, "\n    ");
        rfbxJsonWriterAppendString(&writer, material->name);
        rfbxJsonWriterAppend(&writer,
            ": {\n      \"shader\": \"rfbx_pbr\",\n"
            "      \"baseColor\": [");
        rfbxJsonWriterAppendNumber(&writer, material->baseColor.r / 255.0);
        rfbxJsonWriterAppend(&writer, ", ");
        rfbxJsonWriterAppendNumber(&writer, material->baseColor.g / 255.0);
        rfbxJsonWriterAppend(&writer, ", ");
        rfbxJsonWriterAppendNumber(&writer, material->baseColor.b / 255.0);
        rfbxJsonWriterAppend(&writer, ", ");
        rfbxJsonWriterAppendNumber(&writer, material->baseColor.a / 255.0);
        rfbxJsonWriterAppend(&writer, "],\n      \"metallic\": ");
        rfbxJsonWriterAppendNumber(&writer, material->metallic);
        rfbxJsonWriterAppend(&writer, ",\n      \"roughness\": ");
        rfbxJsonWriterAppendNumber(&writer, material->roughness);
        rfbxJsonWriterAppend(&writer, ",\n      \"alphaMode\": ");
        rfbxJsonWriterAppendString(&writer, alphaModes[material->alphaMode]);
        rfbxJsonWriterAppend(&writer, ",\n      \"alphaCutoff\": ");
        rfbxJsonWriterAppendNumber(&writer, material->alphaCutoff);
        rfbxJsonWriterAppend(&writer, ",\n      \"doubleSided\": ");
        rfbxJsonWriterAppend(
            &writer,
            material->doubleSided ? "true" : "false");

        int textureIndices[6] = { 0 };
        rfbxGetMaterialTextureIndices(material, textureIndices);
        const RfbxMaterialDefaultState* defaults =
            &model->internal->materialDefaults[materialIndex];
        for (int mapIndex = 0; mapIndex < 6 && !writer.failed; ++mapIndex) {
            int textureIndex = textureIndices[mapIndex];
            if (textureIndex == defaults->textureIndices[mapIndex]) continue;

            char* texturePath = rfbxMakeMaterialTexturePath(
                fileName,
                model->textures[textureIndex].fileName);
            if (!texturePath) {
                writer.failed = true;
                break;
            }
            rfbxJsonWriterAppend(&writer, ",\n      \"");
            rfbxJsonWriterAppend(&writer, textureKeys[mapIndex]);
            rfbxJsonWriterAppend(&writer, "\": ");
            rfbxJsonWriterAppendString(&writer, texturePath);
            MemFree(texturePath);
        }
        rfbxJsonWriterAppend(&writer, "\n    }");
    }
    rfbxJsonWriterAppend(&writer, "\n  }\n}\n");

    bool success = !writer.failed && writer.data &&
        SaveFileText(fileName, writer.data);
    if (success) {
        TraceLog(LOG_INFO,
            "RFBX: saved material file '%s' (%d materials)",
            fileName,
            model->materialCount);
    }
    else {
        TraceLog(LOG_WARNING,
            "RFBX: material file could not be saved: %s",
            fileName);
    }
    MemFree(writer.data);
    return success;
}

// Override the inferred FBX transparency mode for gameplay rendering.
bool RfbxSetMaterialAlphaMode(
    RfbxModel* model,
    int materialIndex,
    RfbxAlphaMode alphaMode,
    float alphaCutoff)
{
    if (!RfbxIsModelValid(model) ||
        materialIndex < 0 ||
        materialIndex >= model->materialCount ||
        alphaMode < RFBX_ALPHA_OPAQUE ||
        alphaMode > RFBX_ALPHA_BLEND ||
        !isfinite(alphaCutoff) ||
        alphaCutoff < 0.0f ||
        alphaCutoff > 1.0f) {
        return false;
    }

    RfbxMaterial* material = &model->materials[materialIndex];
    material->alphaMode = alphaMode;
    material->alphaCutoff = alphaCutoff;
    return true;
}

// Override the ufbx double-sided material feature.
bool RfbxSetMaterialDoubleSided(
    RfbxModel* model,
    int materialIndex,
    bool doubleSided)
{
    if (!RfbxIsModelValid(model) ||
        materialIndex < 0 ||
        materialIndex >= model->materialCount) {
        return false;
    }

    model->materials[materialIndex].doubleSided = doubleSided;
    return true;
}

// Return the number of model-owned bones.
int RfbxGetBoneCount(const RfbxModel* model)
{
    return RfbxIsModelValid(model) ? model->skeleton.boneCount : 0;
}

// Return a read-only bone definition by index.
const RfbxBone* RfbxGetBone(const RfbxModel* model, int boneIndex)
{
    if (!RfbxIsModelValid(model) ||
        boneIndex < 0 ||
        boneIndex >= model->skeleton.boneCount) {
        return NULL;
    }

    return &model->skeleton.bones[boneIndex];
}

// Find a bone using an exact, case-sensitive FBX node name.
int RfbxFindBone(const RfbxModel* model, const char* name)
{
    if (!RfbxIsModelValid(model) || !name) return RFBX_INVALID_INDEX;

    for (int boneIndex = 0;
        boneIndex < model->skeleton.boneCount;
        ++boneIndex) {
        if (strcmp(model->skeleton.bones[boneIndex].name, name) == 0) {
            return boneIndex;
        }
    }

    return RFBX_INVALID_INDEX;
}

// Find a converted model node using its exact FBX name.
static int rfbxFindModelNode(
    const RfbxModel* model,
    const char* name)
{
    if (!RfbxIsModelValid(model) || !name) return RFBX_INVALID_INDEX;

    for (int nodeIndex = 0; nodeIndex < model->nodeCount; ++nodeIndex) {
        if (strcmp(model->nodes[nodeIndex].name, name) == 0) {
            return nodeIndex;
        }
    }
    return RFBX_INVALID_INDEX;
}

// Return the number of game-defined attachment points.
int RfbxGetSocketCount(const RfbxModel* model)
{
    return RfbxIsModelValid(model) ? model->socketCount : 0;
}

// Return one model-owned socket definition.
const RfbxSocket* RfbxGetSocket(
    const RfbxModel* model,
    int socketIndex)
{
    if (!RfbxIsModelValid(model) ||
        socketIndex < 0 ||
        socketIndex >= model->socketCount) {
        return NULL;
    }
    return &model->sockets[socketIndex];
}

// Find a game-defined socket using an exact name.
int RfbxFindSocket(const RfbxModel* model, const char* socketName)
{
    if (!RfbxIsModelValid(model) || !socketName) {
        return RFBX_INVALID_INDEX;
    }

    for (int socketIndex = 0;
        socketIndex < model->socketCount;
        ++socketIndex) {
        if (strcmp(model->sockets[socketIndex].name, socketName) == 0) {
            return socketIndex;
        }
    }
    return RFBX_INVALID_INDEX;
}

// Grow the model-owned socket array while preserving registered names.
static bool rfbxGrowSocketArray(RfbxModel* model)
{
    if (model->socketCount < model->internal->socketCapacity) return true;
    if (model->internal->socketCapacity > INT_MAX / 2) return false;

    int newCapacity = model->internal->socketCapacity > 0
        ? model->internal->socketCapacity * 2
        : 4;
    RfbxSocket* sockets = rfbxAllocArray(
        (size_t)newCapacity,
        sizeof(RfbxSocket));
    if (!sockets) return false;

    if (model->socketCount > 0) {
        memcpy(
            sockets,
            model->sockets,
            (size_t)model->socketCount * sizeof(RfbxSocket));
    }
    MemFree(model->sockets);
    model->sockets = sockets;
    model->internal->socketCapacity = newCapacity;
    return true;
}

// Add a resolved node target, replacing a socket that uses the same name.
static bool rfbxAddSocket(
    RfbxModel* model,
    const char* socketName,
    int nodeIndex,
    int boneIndex,
    Matrix localTransform)
{
    if (!RfbxIsModelValid(model) ||
        !socketName ||
        socketName[0] == '\0' ||
        nodeIndex < 0 ||
        nodeIndex >= model->nodeCount) {
        return false;
    }

    int socketIndex = RfbxFindSocket(model, socketName);
    if (socketIndex != RFBX_INVALID_INDEX) {
        RfbxSocket* socket = &model->sockets[socketIndex];
        socket->nodeIndex = nodeIndex;
        socket->boneIndex = boneIndex;
        socket->localTransform = localTransform;
        return true;
    }

    if (!rfbxGrowSocketArray(model)) {
        TraceLog(LOG_WARNING,
            "RFBX: failed to allocate socket '%s'",
            socketName);
        return false;
    }

    char* nameCopy = rfbxCopyString((ufbx_string) {
        socketName,
            strlen(socketName),
    });
    if (!nameCopy) return false;

    RfbxSocket* socket = &model->sockets[model->socketCount++];
    socket->name = nameCopy;
    socket->nodeIndex = nodeIndex;
    socket->boneIndex = boneIndex;
    socket->localTransform = localTransform;
    return true;
}

// Register an attachment point backed by an animated skeleton bone.
bool RfbxAddBoneSocket(
    RfbxModel* model,
    const char* socketName,
    const char* boneName,
    Matrix localTransform)
{
    int boneIndex = RfbxFindBone(model, boneName);
    const RfbxBone* bone = RfbxGetBone(model, boneIndex);
    if (!bone) {
        TraceLog(LOG_WARNING,
            "RFBX: bone '%s' was not found for socket '%s'",
            boneName ? boneName : "<null>",
            socketName ? socketName : "<null>");
        return false;
    }

    int nodeIndex = RFBX_INVALID_INDEX;
    for (int index = 0; index < model->nodeCount; ++index) {
        if (model->nodes[index].sourceTypedId == bone->sourceNodeTypedId) {
            nodeIndex = index;
            break;
        }
    }
    return rfbxAddSocket(
        model,
        socketName,
        nodeIndex,
        boneIndex,
        localTransform);
}

// Register an attachment point backed by any animated FBX node.
bool RfbxAddNodeSocket(
    RfbxModel* model,
    const char* socketName,
    const char* nodeName,
    Matrix localTransform)
{
    int nodeIndex = rfbxFindModelNode(model, nodeName);
    if (nodeIndex == RFBX_INVALID_INDEX) {
        TraceLog(LOG_WARNING,
            "RFBX: node '%s' was not found for socket '%s'",
            nodeName ? nodeName : "<null>",
            socketName ? socketName : "<null>");
        return false;
    }
    return rfbxAddSocket(
        model,
        socketName,
        nodeIndex,
        RFBX_INVALID_INDEX,
        localTransform);
}

// Return whether ufbx must generate deformed vertex streams for this model.
static bool rfbxModelNeedsCpuSkinning(const RfbxModel* model)
{
    if (!model) return false;
    for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
        const RfbxMesh* mesh = &model->meshes[meshIndex];
        if (mesh->hasSkinning && !mesh->gpuSkinningReady) return true;
    }
    return false;
}

// Return whether an instance contains any automatic CPU fallback mesh.
static bool rfbxInstanceNeedsCpuSkinning(const RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance)) return false;
    for (int meshIndex = 0;
        meshIndex < instance->internal->meshCount;
        ++meshIndex) {
        const RfbxMesh* mesh = &instance->model->meshes[meshIndex];
        const RfbxInstanceMeshState* state =
            &instance->internal->meshes[meshIndex];
        if (mesh->hasSkinning && !state->gpuSkinning) return true;
    }
    return false;
}

// Evaluate a clip and optionally ask ufbx to generate CPU-skinned vertices.
static ufbx_scene* rfbxEvaluateAnimationScene(
    const RfbxModel* model,
    int animationIndex,
    double timeSeconds,
    double* evaluatedLocalTime,
    bool evaluateSkinning)
{
    const RfbxAnimation* animation = RfbxGetAnimation(model, animationIndex);
    if (!animation || !model->internal || !model->internal->scene) return NULL;

    ufbx_scene* sourceScene = model->internal->scene;
    if (animation->sourceStackIndex >= sourceScene->anim_stacks.count) return NULL;

    double localTime = timeSeconds;
    if (localTime < 0.0) localTime = 0.0;
    if (localTime > animation->duration) localTime = animation->duration;

    const ufbx_anim_stack* stack =
        sourceScene->anim_stacks.data[animation->sourceStackIndex];
    ufbx_evaluate_opts options = { 0 };
    options.evaluate_skinning = evaluateSkinning;

    ufbx_error error = { 0 };
    ufbx_scene* evaluated = ufbx_evaluate_scene(
        sourceScene,
        stack->anim,
        animation->timeBegin + localTime,
        &options,
        &error);
    if (!evaluated) {
        size_t length = error.description.length;
        const char* description = error.description.data;
        if (!description) {
            description = "Unknown error";
            length = strlen(description);
        }
        if (length > (size_t)INT_MAX) length = INT_MAX;
        TraceLog(LOG_WARNING,
            "RFBX: failed to evaluate animation '%s': %.*s",
            animation->name,
            (int)length,
            description);
        return NULL;
    }

    if (evaluatedLocalTime) *evaluatedLocalTime = localTime;
    return evaluated;
}

// Create an independent pose with dynamic meshes only where skinning needs them.
RfbxInstance RfbxCreateInstance(const RfbxModel* model)
{
    RfbxInstance result = { 0 };
    result.model = model;
    result.currentAnimation = RFBX_INVALID_INDEX;
    result.nextAnimation = RFBX_INVALID_INDEX;
    result.queuedAnimation = RFBX_INVALID_INDEX;
    result.layerAnimation = RFBX_INVALID_INDEX;
    result.playbackSpeed = 1.0f;
    result.layerPlaybackSpeed = 1.0f;

    if (!RfbxIsModelValid(model)) return result;

    result.internal = rfbxAllocArray(1, sizeof(RfbxInstanceInternal));
    if (!result.internal) goto create_failed;

    result.internal->rootMotionNodeIndex = RFBX_INVALID_INDEX;
    result.internal->layerRootBoneIndex = RFBX_INVALID_INDEX;
    result.internal->pendingRootMotion.rotation.w = 1.0f;

    result.internal->nodeCount = model->nodeCount;
    result.internal->meshCount = model->meshCount;

    if (model->nodeCount > 0) {
        result.internal->nodes = rfbxAllocArray(
            (size_t)model->nodeCount,
            sizeof(RfbxInstanceNodeState));
        if (!result.internal->nodes) goto create_failed;

        result.internal->blendOverrides = rfbxAllocArray(
            (size_t)model->nodeCount,
            sizeof(ufbx_transform_override));
        if (!result.internal->blendOverrides) goto create_failed;

        result.internal->layerNodeMask = rfbxAllocArray(
            (size_t)model->nodeCount,
            sizeof(bool));
        if (!result.internal->layerNodeMask) goto create_failed;
    }

    if (model->meshCount > 0) {
        result.internal->meshes = rfbxAllocArray(
            (size_t)model->meshCount,
            sizeof(RfbxInstanceMeshState));
        if (!result.internal->meshes) goto create_failed;
    }

    if (model->skeleton.boneCount > 0) {
        result.internal->boneToWorld = rfbxAllocArray(
            (size_t)model->skeleton.boneCount,
            sizeof(Matrix));
        if (!result.internal->boneToWorld) goto create_failed;
    }

    for (int nodeIndex = 0; nodeIndex < model->nodeCount; ++nodeIndex) {
        const RfbxNode* source = &model->nodes[nodeIndex];
        RfbxInstanceNodeState* destination = &result.internal->nodes[nodeIndex];

        destination->visible = source->visible;
        destination->nodeToParent = source->nodeToParent;
        destination->nodeToWorld = source->nodeToWorld;
        destination->geometryToNode = source->geometryToNode;
        destination->geometryToWorld = source->geometryToWorld;
        destination->scaleMultiplier = (Vector3){ 1.0f, 1.0f, 1.0f }; //デフォルト指定等倍
        destination->offset = (Vector3){ 0.0f,0.0f,0.0f };
    }

    for (int boneIndex = 0;
        boneIndex < model->skeleton.boneCount;
        ++boneIndex) {
        uint32_t nodeIndex = model->skeleton.bones[boneIndex].sourceNodeTypedId;
        if (nodeIndex >= (uint32_t)model->nodeCount) goto create_failed;
        result.internal->boneToWorld[boneIndex] =
            model->nodes[nodeIndex].nodeToWorld;
    }

    for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
        const RfbxMesh* source = &model->meshes[meshIndex];
        RfbxInstanceMeshState* destination =
            &result.internal->meshes[meshIndex];

        destination->verticesAreLocal = source->verticesAreLocal;
        if (!source->hasSkinning || source->primitiveCount == 0) continue;

        if (source->gpuSkinningReady) {
            if (!model->internal || !model->internal->gpuMeshes) {
                goto create_failed;
            }
            const RfbxGpuMeshState* modelState =
                &model->internal->gpuMeshes[meshIndex];
            destination->boneCount = modelState->boneCount;
            destination->boneMatrices = rfbxAllocArray(
                (size_t)destination->boneCount,
                sizeof(Matrix));
            if (!destination->boneMatrices) goto create_failed;
            memcpy(
                destination->boneMatrices,
                modelState->boneMatrices,
                (size_t)destination->boneCount * sizeof(Matrix));
            destination->gpuSkinning = true;
            destination->verticesAreLocal = false;
            continue;
        }

        destination->primitiveCount = source->primitiveCount;
        destination->primitives = rfbxAllocArray(
            (size_t)source->primitiveCount,
            sizeof(Mesh));
        if (!destination->primitives) goto create_failed;

        for (int primitiveIndex = 0;
            primitiveIndex < source->primitiveCount;
            ++primitiveIndex) {
            if (!rfbxCloneDynamicMesh(
                &destination->primitives[primitiveIndex],
                &source->primitives[primitiveIndex].mesh)) {
                goto create_failed;
            }
        }
    }

    result.valid = true;
    rfbxUpdateInstanceBounds(&result);
    return result;

create_failed:
    TraceLog(LOG_WARNING, "RFBX: failed to create model instance");
    RfbxUnloadInstance(&result);
    return (RfbxInstance) { 0 };
}

// Check that the instance and its referenced model are both alive.
bool RfbxIsInstanceValid(const RfbxInstance* instance)
{
    return instance &&
        instance->valid &&
        RfbxIsModelValid(instance->model) &&
        instance->internal;
}

// Return a neutral motion delta with an identity rotation.
static RfbxRootMotion rfbxIdentityRootMotion(void)
{
    RfbxRootMotion motion = { 0 };
    motion.rotation.w = 1.0f;
    return motion;
}

// Evaluate one node at a clip-local time for loop-boundary compensation.
static bool rfbxEvaluateRootMotionNode(
    const RfbxModel* model,
    int animationIndex,
    int nodeIndex,
    double timeSeconds,
    Matrix* transform)
{
    if (!transform ||
        nodeIndex < 0 ||
        nodeIndex >= model->nodeCount) {
        return false;
    }

    ufbx_scene* evaluated = rfbxEvaluateAnimationScene(
        model,
        animationIndex,
        timeSeconds,
        NULL,
        false);
    if (!evaluated) return false;

    uint32_t sourceNodeIndex = model->nodes[nodeIndex].sourceTypedId;
    bool success = sourceNodeIndex < evaluated->nodes.count;
    if (success) {
        *transform = rfbxMatrixFromUfbx(
            evaluated->nodes.data[sourceNodeIndex]->node_to_world);
    }
    ufbx_free_scene(evaluated);
    return success;
}

// Cache each clip's root transform at both loop boundaries.
static bool rfbxBuildRootMotionEndpoints(
    const RfbxInstance* instance,
    int nodeIndex,
    Matrix** startTransforms,
    Matrix** endTransforms)
{
    const RfbxModel* model = instance->model;
    *startTransforms = NULL;
    *endTransforms = NULL;
    if (model->animationCount <= 0) return true;

    Matrix* starts = rfbxAllocArray(
        (size_t)model->animationCount,
        sizeof(Matrix));
    Matrix* ends = rfbxAllocArray(
        (size_t)model->animationCount,
        sizeof(Matrix));
    if (!starts || !ends) {
        MemFree(starts);
        MemFree(ends);
        return false;
    }

    for (int animationIndex = 0;
        animationIndex < model->animationCount;
        ++animationIndex) {
        const RfbxAnimation* animation = RfbxGetAnimation(
            model,
            animationIndex);
        if (!animation ||
            !rfbxEvaluateRootMotionNode(
                model,
                animationIndex,
                nodeIndex,
                0.0,
                &starts[animationIndex]) ||
            !rfbxEvaluateRootMotionNode(
                model,
                animationIndex,
                nodeIndex,
                animation->duration,
                &ends[animationIndex])) {
            MemFree(starts);
            MemFree(ends);
            return false;
        }
    }

    *startTransforms = starts;
    *endTransforms = ends;
    return true;
}

// Configure one already-resolved model node as the motion source.
static bool rfbxSetRootMotionNodeIndex(
    RfbxInstance* instance,
    int nodeIndex,
    bool inPlace)
{
    if (!RfbxIsInstanceValid(instance) ||
        nodeIndex < 0 ||
        nodeIndex >= instance->model->nodeCount) {
        return false;
    }

    Matrix* starts = NULL;
    Matrix* ends = NULL;
    if (!rfbxBuildRootMotionEndpoints(
        instance,
        nodeIndex,
        &starts,
        &ends)) {
        TraceLog(LOG_WARNING,
            "RFBX: failed to cache root motion loop boundaries");
        return false;
    }

    MemFree(instance->internal->rootMotionStartTransforms);
    MemFree(instance->internal->rootMotionEndTransforms);
    instance->internal->rootMotionStartTransforms = starts;
    instance->internal->rootMotionEndTransforms = ends;
    instance->internal->rootMotionNodeIndex = nodeIndex;
    instance->internal->rootMotionEnabled = true;
    instance->internal->rootMotionInPlace = inPlace;
    RfbxResetRootMotion(instance);
    return true;
}

// Select any exact FBX node name as a root motion source.
bool RfbxSetRootMotionNode(
    RfbxInstance* instance,
    const char* nodeName,
    bool inPlace)
{
    if (!RfbxIsInstanceValid(instance) || !nodeName) return false;

    for (int nodeIndex = 0;
        nodeIndex < instance->model->nodeCount;
        ++nodeIndex) {
        if (strcmp(instance->model->nodes[nodeIndex].name, nodeName) == 0) {
            return rfbxSetRootMotionNodeIndex(
                instance,
                nodeIndex,
                inPlace);
        }
    }

    TraceLog(LOG_WARNING,
        "RFBX: root motion node '%s' was not found",
        nodeName);
    return false;
}

// Resolve a skeleton bone back to its source node.
bool RfbxSetRootMotionBone(
    RfbxInstance* instance,
    const char* boneName,
    bool inPlace)
{
    if (!RfbxIsInstanceValid(instance) || !boneName) return false;

    int boneIndex = RfbxFindBone(instance->model, boneName);
    const RfbxBone* bone = RfbxGetBone(instance->model, boneIndex);
    if (!bone) {
        TraceLog(LOG_WARNING,
            "RFBX: root motion bone '%s' was not found",
            boneName);
        return false;
    }

    for (int nodeIndex = 0;
        nodeIndex < instance->model->nodeCount;
        ++nodeIndex) {
        if (instance->model->nodes[nodeIndex].sourceTypedId ==
            bone->sourceNodeTypedId) {
            return rfbxSetRootMotionNodeIndex(
                instance,
                nodeIndex,
                inPlace);
        }
    }
    return false;
}

// Reset motion deltas and establish the current pose as the new origin.
void RfbxResetRootMotion(RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance)) return;

    RfbxInstanceInternal* internal = instance->internal;
    internal->pendingRootMotion = rfbxIdentityRootMotion();
    internal->rootMotionHasPrevious = false;

    int nodeIndex = internal->rootMotionNodeIndex;
    if (internal->rootMotionEnabled &&
        nodeIndex >= 0 &&
        nodeIndex < internal->nodeCount) {
        internal->rootMotionPreviousTransform =
            internal->nodes[nodeIndex].nodeToWorld;
        internal->rootMotionVisualOriginTransform =
            internal->nodes[nodeIndex].nodeToWorld;
        internal->rootMotionHasPrevious = true;
    }
}

// Disable extraction and release per-clip loop-boundary caches.
void RfbxDisableRootMotion(RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance)) return;

    MemFree(instance->internal->rootMotionStartTransforms);
    MemFree(instance->internal->rootMotionEndTransforms);
    instance->internal->rootMotionStartTransforms = NULL;
    instance->internal->rootMotionEndTransforms = NULL;
    instance->internal->rootMotionNodeIndex = RFBX_INVALID_INDEX;
    instance->internal->rootMotionEnabled = false;
    instance->internal->rootMotionInPlace = false;
    instance->internal->rootMotionHasPrevious = false;
    instance->internal->pendingRootMotion = rfbxIdentityRootMotion();
}

// Report whether a valid node is selected for extraction.
bool RfbxIsRootMotionEnabled(const RfbxInstance* instance)
{
    return RfbxIsInstanceValid(instance) &&
        instance->internal->rootMotionEnabled &&
        instance->internal->rootMotionNodeIndex >= 0 &&
        instance->internal->rootMotionNodeIndex < instance->model->nodeCount;
}

// Return the model-owned selected node name.
const char* RfbxGetRootMotionNodeName(const RfbxInstance* instance)
{
    return RfbxIsRootMotionEnabled(instance)
        ? instance->model->nodes[
            instance->internal->rootMotionNodeIndex].name
        : NULL;
}

// Toggle visual correction and make the current pose its new visual origin.
void RfbxSetRootMotionInPlace(RfbxInstance* instance, bool inPlace)
{
    if (!RfbxIsRootMotionEnabled(instance) ||
        instance->internal->rootMotionInPlace == inPlace) {
        return;
    }

    instance->internal->rootMotionInPlace = inPlace;
    instance->internal->rootMotionPreviousTransform =
        instance->internal->nodes[
            instance->internal->rootMotionNodeIndex].nodeToWorld;
    instance->internal->rootMotionVisualOriginTransform =
        instance->internal->rootMotionPreviousTransform;
    instance->internal->rootMotionHasPrevious = true;
}

// Report whether extraction also removes motion from drawing.
bool RfbxIsRootMotionInPlace(const RfbxInstance* instance)
{
    return RfbxIsRootMotionEnabled(instance) &&
        instance->internal->rootMotionInPlace;
}

// Read the unconsumed model-space motion delta.
RfbxRootMotion RfbxPeekRootMotion(const RfbxInstance* instance)
{
    return RfbxIsRootMotionEnabled(instance)
        ? instance->internal->pendingRootMotion
        : rfbxIdentityRootMotion();
}

// Read and reset only the gameplay-facing delta accumulator.
RfbxRootMotion RfbxConsumeRootMotion(RfbxInstance* instance)
{
    if (!RfbxIsRootMotionEnabled(instance)) {
        return rfbxIdentityRootMotion();
    }

    RfbxRootMotion result = instance->internal->pendingRootMotion;
    instance->internal->pendingRootMotion = rfbxIdentityRootMotion();
    return result;
}

// Build the exact current-root to visual-origin correction used by drawing.
Matrix RfbxGetRootMotionCorrection(const RfbxInstance* instance)
{
    if (!RfbxIsRootMotionInPlace(instance)) return MatrixIdentity();

    int nodeIndex = instance->internal->rootMotionNodeIndex;
    Matrix current = instance->internal->nodes[nodeIndex].nodeToWorld;
    Matrix origin = instance->internal->rootMotionVisualOriginTransform;
    Vector3 currentTranslation = { 0 };
    Vector3 originTranslation = { 0 };
    Quaternion currentRotation = { 0 };
    Quaternion originRotation = { 0 };
    Vector3 ignoredScale = { 0 };
    MatrixDecompose(
        current,
        &currentTranslation,
        &currentRotation,
        &ignoredScale);
    MatrixDecompose(
        origin,
        &originTranslation,
        &originRotation,
        &ignoredScale);

    current = QuaternionToMatrix(QuaternionNormalize(currentRotation));
    current.m12 = currentTranslation.x;
    current.m13 = currentTranslation.y;
    current.m14 = currentTranslation.z;
    origin = QuaternionToMatrix(QuaternionNormalize(originRotation));
    origin.m12 = originTranslation.x;
    origin.m13 = originTranslation.y;
    origin.m14 = originTranslation.z;
    return MatrixMultiply(MatrixInvert(current), origin);
}

// Return cached bounds for the model's current bind or animated pose.
BoundingBox RfbxGetModelBounds(const RfbxModel* model)
{
    return RfbxIsModelValid(model) && model->boundsValid
        ? model->bounds
        : (BoundingBox) { 0 };
}

// Return current instance bounds using identity placement.
BoundingBox RfbxGetInstanceBounds(const RfbxInstance* instance)
{
    return RfbxGetInstanceBoundsMatrix(instance, MatrixIdentity());
}

// Transform cached pose bounds exactly like RfbxDrawInstanceMatrix().
BoundingBox RfbxGetInstanceBoundsMatrix(
    const RfbxInstance* instance,
    Matrix transform)
{
    if (!RfbxIsInstanceValid(instance) || !instance->boundsValid) {
        return (BoundingBox) { 0 };
    }

    Matrix drawTransform = transform;
    if (RfbxIsRootMotionInPlace(instance)) {
        drawTransform = MatrixMultiply(
            RfbxGetRootMotionCorrection(instance),
            transform);
    }
    return rfbxTransformBounds(instance->bounds, drawTransform);
}

// Draw current bounds without affecting the model or animation state.
void RfbxDrawInstanceBounds(
    const RfbxInstance* instance,
    Matrix transform,
    Color color)
{
    if (!RfbxIsInstanceValid(instance) || !instance->boundsValid) return;
    DrawBoundingBox(RfbxGetInstanceBoundsMatrix(instance, transform), color);
}

// Append one fired marker to an instance-owned FIFO queue.
static bool rfbxPushPendingAnimationEvent(
    RfbxInstance* instance,
    const RfbxAnimationEvent* event)
{
    if (!RfbxIsInstanceValid(instance) || !event) return false;

    RfbxInstanceInternal* internal = instance->internal;
    if (internal->pendingEventReadIndex >= internal->pendingEventCount) {
        internal->pendingEventReadIndex = 0;
        internal->pendingEventCount = 0;
    }

    if (internal->pendingEventCount >= internal->pendingEventCapacity &&
        internal->pendingEventReadIndex > 0) {
        int unreadCount = internal->pendingEventCount -
            internal->pendingEventReadIndex;
        memmove(
            internal->pendingEvents,
            &internal->pendingEvents[internal->pendingEventReadIndex],
            (size_t)unreadCount * sizeof(RfbxAnimationEvent));
        internal->pendingEventCount = unreadCount;
        internal->pendingEventReadIndex = 0;
    }

    if (internal->pendingEventCount >= internal->pendingEventCapacity) {
        if (internal->pendingEventCapacity > INT_MAX / 2) return false;
        int newCapacity = internal->pendingEventCapacity > 0
            ? internal->pendingEventCapacity * 2
            : 8;
        RfbxAnimationEvent* events = rfbxAllocArray(
            (size_t)newCapacity,
            sizeof(RfbxAnimationEvent));
        if (!events) return false;

        if (internal->pendingEventCount > 0) {
            memcpy(
                events,
                internal->pendingEvents,
                (size_t)internal->pendingEventCount *
                sizeof(RfbxAnimationEvent));
        }
        MemFree(internal->pendingEvents);
        internal->pendingEvents = events;
        internal->pendingEventCapacity = newCapacity;
    }

    internal->pendingEvents[internal->pendingEventCount++] = *event;
    return true;
}

// Pop one fired marker in chronological firing order.
bool RfbxPollAnimationEvent(
    RfbxInstance* instance,
    RfbxAnimationEvent* event)
{
    if (!RfbxIsInstanceValid(instance) ||
        !event ||
        instance->internal->pendingEventReadIndex >=
        instance->internal->pendingEventCount) {
        return false;
    }

    *event = instance->internal->pendingEvents[
        instance->internal->pendingEventReadIndex++];
    if (instance->internal->pendingEventReadIndex >=
        instance->internal->pendingEventCount) {
        instance->internal->pendingEventReadIndex = 0;
        instance->internal->pendingEventCount = 0;
    }
    return true;
}

// Return the unread FIFO length without consuming an event.
int RfbxGetPendingAnimationEventCount(const RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance)) return 0;
    return instance->internal->pendingEventCount -
        instance->internal->pendingEventReadIndex;
}

// Discard unread fired markers while retaining queue capacity.
void RfbxClearPendingAnimationEvents(RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance)) return;
    instance->internal->pendingEventCount = 0;
    instance->internal->pendingEventReadIndex = 0;
}

// Return an evaluated bone matrix without exposing ufbx internals.
Matrix RfbxGetBoneToWorld(const RfbxInstance* instance, int boneIndex)
{
    if (!RfbxIsInstanceValid(instance) ||
        boneIndex < 0 ||
        boneIndex >= instance->model->skeleton.boneCount) {
        return MatrixIdentity();
    }

    return instance->internal->boneToWorld[boneIndex];
}

// Resolve a socket against the current pose and the active in-place correction.
Matrix RfbxGetSocketToModel(
    const RfbxInstance* instance,
    int socketIndex)
{
    if (!RfbxIsInstanceValid(instance)) return MatrixIdentity();

    const RfbxSocket* socket = RfbxGetSocket(
        instance->model,
        socketIndex);
    if (!socket ||
        socket->nodeIndex < 0 ||
        socket->nodeIndex >= instance->internal->nodeCount) {
        return MatrixIdentity();
    }

    Matrix result = MatrixMultiply(
        socket->localTransform,
        instance->internal->nodes[socket->nodeIndex].nodeToWorld);
    if (RfbxIsRootMotionInPlace(instance)) {
        result = MatrixMultiply(
            result,
            RfbxGetRootMotionCorrection(instance));
    }
    return result;
}

// Transform a pose socket exactly like the owning animated instance.
Matrix RfbxGetSocketToWorld(
    const RfbxInstance* instance,
    int socketIndex,
    Matrix transform)
{
    if (!RfbxIsInstanceValid(instance) ||
        !RfbxGetSocket(instance->model, socketIndex)) {
        return MatrixIdentity();
    }
    return MatrixMultiply(
        RfbxGetSocketToModel(instance, socketIndex),
        transform);
}

// Draw one RGB basis at every registered socket for authoring checks.
void RfbxDrawSockets(
    const RfbxInstance* instance,
    Matrix transform,
    float axisLength)
{
    if (!RfbxIsInstanceValid(instance)) return;
    if (axisLength <= 0.0f) axisLength = 0.1f;

    for (int socketIndex = 0;
        socketIndex < instance->model->socketCount;
        ++socketIndex) {
        Matrix socket = RfbxGetSocketToWorld(
            instance,
            socketIndex,
            transform);
        Vector3 origin = { socket.m12, socket.m13, socket.m14 };
        Vector3 x = Vector3Transform(
            (Vector3) {
            axisLength, 0.0f, 0.0f
        },
            socket);
        Vector3 y = Vector3Transform(
            (Vector3) {
            0.0f, axisLength, 0.0f
        },
            socket);
        Vector3 z = Vector3Transform(
            (Vector3) {
            0.0f, 0.0f, axisLength
        },
            socket);
        DrawSphere(origin, axisLength * 0.08f, GOLD);
        DrawLine3D(origin, x, RED);
        DrawLine3D(origin, y, GREEN);
        DrawLine3D(origin, z, BLUE);
    }
}

// Draw the current bone hierarchy as points and parent-child lines.
void RfbxDrawSkeleton(
    const RfbxInstance* instance,
    Matrix transform,
    float jointRadius,
    Color color)
{
    if (!RfbxIsInstanceValid(instance)) return;
    if (jointRadius <= 0.0f) jointRadius = 0.015f;

    Matrix drawTransform = transform;
    if (RfbxIsRootMotionInPlace(instance)) {
        drawTransform = MatrixMultiply(
            RfbxGetRootMotionCorrection(instance),
            transform);
    }

    const RfbxSkeleton* skeleton = &instance->model->skeleton;
    for (int boneIndex = 0; boneIndex < skeleton->boneCount; ++boneIndex) {
        const RfbxBone* bone = &skeleton->bones[boneIndex];
        Matrix boneMatrix = instance->internal->boneToWorld[boneIndex];
        Vector3 position = {
            boneMatrix.m12,
            boneMatrix.m13,
            boneMatrix.m14
        };
        position = Vector3Transform(position, drawTransform);

        if (bone->parentIndex >= 0 &&
            bone->parentIndex < skeleton->boneCount) {
            Matrix parentMatrix =
                instance->internal->boneToWorld[bone->parentIndex];
            Vector3 parentPosition = {
                parentMatrix.m12,
                parentMatrix.m13,
                parentMatrix.m14
            };
            parentPosition = Vector3Transform(parentPosition, drawTransform);
            DrawLine3D(parentPosition, position, color);
        }

        DrawSphere(position, jointRadius, color);
    }
}

// Evaluate one animation stack and update its GPU palette or CPU vertices.
bool RfbxUpdateAnimation(
    RfbxModel* model,
    int animationIndex,
    double timeSeconds)
{
    const RfbxAnimation* animation = RfbxGetAnimation(model, animationIndex);
    if (!animation) return false;

    double localTime = 0.0;
    ufbx_scene* evaluated = rfbxEvaluateAnimationScene(
        model,
        animationIndex,
        timeSeconds,
        &localTime,
        rfbxModelNeedsCpuSkinning(model));
    if (!evaluated) return false;

    bool success = rfbxApplySceneState(model, evaluated);
    ufbx_free_scene(evaluated);

    if (!success) {
        TraceLog(LOG_WARNING,
            "RFBX: failed to apply animation '%s'",
            animation->name);
        return false;
    }

    model->currentAnimation = animationIndex;
    model->currentTime = localTime;
    return true;
}

// Restore transforms and skinned buffers from the original loaded scene.
void RfbxResetAnimation(RfbxModel* model)
{
    if (!RfbxIsModelValid(model) || !model->internal || !model->internal->scene) {
        return;
    }

    if (!rfbxApplySceneState(model, model->internal->scene)) {
        TraceLog(LOG_WARNING, "RFBX: failed to reset animation state");
        return;
    }

    model->currentAnimation = RFBX_INVALID_INDEX;
    model->currentTime = 0.0;
}

static bool rfbxSetInstanceBlendedAnimationTime(
    RfbxInstance* instance,
    int fromAnimationIndex,
    double fromTime,
    int toAnimationIndex,
    double toTime,
    double blendWeight);

// Report whether an overlay clip is configured, even at zero fade weight.
static bool rfbxHasAnimationLayer(const RfbxInstance* instance)
{
    return RfbxIsInstanceValid(instance) &&
        RfbxGetAnimation(
            instance->model,
            instance->layerAnimation) != NULL &&
        instance->internal->layerRootBoneIndex != RFBX_INVALID_INDEX;
}

// Evaluate and apply an animation pose to one independent instance.
static bool rfbxSetInstanceAnimationTime(
    RfbxInstance* instance,
    int animationIndex,
    double timeSeconds)
{
    if (!RfbxIsInstanceValid(instance)) return false;

    double localTime = 0.0;
    const RfbxAnimation* animation = RfbxGetAnimation(
        instance->model,
        animationIndex);
    if (!animation) return false;
    localTime = timeSeconds;
    if (localTime < 0.0) localTime = 0.0;
    if (localTime > animation->duration) localTime = animation->duration;

    bool success = false;
    if (rfbxHasAnimationLayer(instance)) {
        success = rfbxSetInstanceBlendedAnimationTime(
            instance,
            animationIndex,
            localTime,
            RFBX_INVALID_INDEX,
            0.0,
            0.0);
    }
    else {
        ufbx_scene* evaluated = rfbxEvaluateAnimationScene(
            instance->model,
            animationIndex,
            localTime,
            NULL,
            rfbxInstanceNeedsCpuSkinning(instance));
        if (!evaluated) return false;
        success = rfbxApplySceneStateToInstance(instance, evaluated);
        ufbx_free_scene(evaluated);
    }

    if (!success) {
        TraceLog(LOG_WARNING,
            "RFBX: failed to apply animation '%s' to instance",
            animation ? animation->name : "");
        return false;
    }

    instance->currentAnimation = animationIndex;
    instance->currentTime = localTime;
    return true;
}

// Evaluate two clips at independent times and let ufbx skin the blended pose.
static bool rfbxSetInstanceBlendedAnimationTime(
    RfbxInstance* instance,
    int fromAnimationIndex,
    double fromTime,
    int toAnimationIndex,
    double toTime,
    double blendWeight)
{
    if (!RfbxIsInstanceValid(instance)) return false;

    const RfbxModel* model = instance->model;
    const RfbxAnimation* fromAnimation =
        RfbxGetAnimation(model, fromAnimationIndex);
    const RfbxAnimation* toAnimation =
        RfbxGetAnimation(model, toAnimationIndex);
    bool hasBaseBlend = toAnimation != NULL;
    if (!fromAnimation ||
        !model->internal || !model->internal->scene) {
        return false;
    }

    ufbx_scene* sourceScene = model->internal->scene;
    if (fromAnimation->sourceStackIndex >= sourceScene->anim_stacks.count ||
        (hasBaseBlend &&
            toAnimation->sourceStackIndex >= sourceScene->anim_stacks.count) ||
        sourceScene->nodes.count != (size_t)model->nodeCount) {
        return false;
    }

    if (fromTime < 0.0) fromTime = 0.0;
    if (fromTime > fromAnimation->duration) fromTime = fromAnimation->duration;
    if (hasBaseBlend) {
        if (toTime < 0.0) toTime = 0.0;
        if (toTime > toAnimation->duration) toTime = toAnimation->duration;
    }
    if (blendWeight < 0.0) blendWeight = 0.0;
    if (blendWeight > 1.0) blendWeight = 1.0;

    const ufbx_anim* fromAnim = sourceScene->anim_stacks
        .data[fromAnimation->sourceStackIndex]->anim;
    const ufbx_anim* toAnim = hasBaseBlend
        ? sourceScene->anim_stacks
        .data[toAnimation->sourceStackIndex]->anim
        : NULL;
    ufbx_real weight = (ufbx_real)blendWeight;
    const RfbxAnimation* layerAnimation = rfbxHasAnimationLayer(instance)
        ? RfbxGetAnimation(model, instance->layerAnimation)
        : NULL;
    const ufbx_anim* layerAnim = NULL;
    double layerTime = instance->layerTime;
    ufbx_real layerWeight = (ufbx_real)instance->layerWeight;
    if (layerWeight < 0.0) layerWeight = 0.0;
    if (layerWeight > 1.0) layerWeight = 1.0;
    if (layerAnimation) {
        if (layerAnimation->sourceStackIndex >=
            sourceScene->anim_stacks.count) {
            return false;
        }
        if (layerTime < 0.0) layerTime = 0.0;
        if (layerTime > layerAnimation->duration) {
            layerTime = layerAnimation->duration;
        }
        layerAnim = sourceScene->anim_stacks
            .data[layerAnimation->sourceStackIndex]->anim;
    }

    for (size_t nodeIndex = 0;
        nodeIndex < sourceScene->nodes.count;
        ++nodeIndex) {
        const ufbx_node* node = sourceScene->nodes.data[nodeIndex];
        ufbx_transform fromTransform = ufbx_evaluate_transform(
            fromAnim,
            node,
            fromAnimation->timeBegin + fromTime);

        ufbx_transform_override* override =
            &instance->internal->blendOverrides[nodeIndex];
        override->node_id = node->typed_id;
        override->transform = fromTransform;

        if (hasBaseBlend) {
            ufbx_transform toTransform = ufbx_evaluate_transform(
                toAnim,
                node,
                toAnimation->timeBegin + toTime);
            override->transform.translation.x =
                fromTransform.translation.x +
                (toTransform.translation.x - fromTransform.translation.x) *
                weight;
            override->transform.translation.y =
                fromTransform.translation.y +
                (toTransform.translation.y - fromTransform.translation.y) *
                weight;
            override->transform.translation.z =
                fromTransform.translation.z +
                (toTransform.translation.z - fromTransform.translation.z) *
                weight;
            override->transform.scale.x = fromTransform.scale.x +
                (toTransform.scale.x - fromTransform.scale.x) * weight;
            override->transform.scale.y = fromTransform.scale.y +
                (toTransform.scale.y - fromTransform.scale.y) * weight;
            override->transform.scale.z = fromTransform.scale.z +
                (toTransform.scale.z - fromTransform.scale.z) * weight;
            override->transform.rotation = ufbx_quat_slerp(
                fromTransform.rotation,
                toTransform.rotation,
                weight);
        }

        if (layerAnim &&
            layerWeight > 0.0 &&
            instance->internal->layerNodeMask[nodeIndex]) {
            ufbx_transform layerTransform = ufbx_evaluate_transform(
                layerAnim,
                node,
                layerAnimation->timeBegin + layerTime);
            ufbx_transform baseTransform = override->transform;
            override->transform.translation.x =
                baseTransform.translation.x +
                (layerTransform.translation.x -
                    baseTransform.translation.x) * layerWeight;
            override->transform.translation.y =
                baseTransform.translation.y +
                (layerTransform.translation.y -
                    baseTransform.translation.y) * layerWeight;
            override->transform.translation.z =
                baseTransform.translation.z +
                (layerTransform.translation.z -
                    baseTransform.translation.z) * layerWeight;
            override->transform.scale.x = baseTransform.scale.x +
                (layerTransform.scale.x - baseTransform.scale.x) *
                layerWeight;
            override->transform.scale.y = baseTransform.scale.y +
                (layerTransform.scale.y - baseTransform.scale.y) *
                layerWeight;
            override->transform.scale.z = baseTransform.scale.z +
                (layerTransform.scale.z - baseTransform.scale.z) *
                layerWeight;
            override->transform.rotation = ufbx_quat_slerp(
                baseTransform.rotation,
                layerTransform.rotation,
                layerWeight);
        }
    }

    ufbx_anim_opts animOptions = { 0 };
    animOptions.transform_overrides.data = instance->internal->blendOverrides;
    animOptions.transform_overrides.count = sourceScene->nodes.count;

    ufbx_error error = { 0 };
    ufbx_anim* blendedAnim = ufbx_create_anim(sourceScene, &animOptions, &error);
    if (!blendedAnim) {
        size_t length = error.description.length;
        const char* description = error.description.data
            ? error.description.data
            : "Unknown error";
        if (!error.description.data) length = strlen(description);
        if (length > (size_t)INT_MAX) length = (size_t)INT_MAX;
        TraceLog(LOG_WARNING,
            "RFBX: failed to create blended animation: %.*s",
            (int)length,
            description);
        return false;
    }

    ufbx_evaluate_opts evaluateOptions = { 0 };
    evaluateOptions.evaluate_skinning =
        rfbxInstanceNeedsCpuSkinning(instance);
    ufbx_scene* evaluated = ufbx_evaluate_scene(
        sourceScene,
        blendedAnim,
        0.0,
        &evaluateOptions,
        &error);
    if (!evaluated) {
        size_t length = error.description.length;
        const char* description = error.description.data
            ? error.description.data
            : "Unknown error";
        if (!error.description.data) length = strlen(description);
        if (length > (size_t)INT_MAX) length = (size_t)INT_MAX;
        TraceLog(LOG_WARNING,
            "RFBX: failed to evaluate blended animation: %.*s",
            (int)length,
            description);
        ufbx_free_anim(blendedAnim);
        return false;
    }

    bool success = rfbxApplySceneStateToInstance(instance, evaluated);
    ufbx_free_scene(evaluated);
    ufbx_free_anim(blendedAnim);

    if (!success) {
        TraceLog(LOG_WARNING, "RFBX: failed to apply blended animation");
    }
    return success;
}

// Advance one clip time and report a non-looping endpoint.
static double rfbxAdvanceAnimationTime(
    const RfbxAnimation* animation,
    double currentTime,
    double deltaSeconds,
    bool loop,
    bool* finished)
{
    if (finished) *finished = false;
    if (!animation || animation->duration <= 0.0) {
        if (finished) *finished = true;
        return 0.0;
    }

    double result = currentTime + deltaSeconds;
    if (loop) {
        result = fmod(result, animation->duration);
        if (result < 0.0) result += animation->duration;
    }
    else if (result >= animation->duration) {
        result = animation->duration;
        if (finished) *finished = true;
    }
    else if (result <= 0.0) {
        result = 0.0;
        if (finished && deltaSeconds < 0.0) *finished = true;
    }
    return result;
}

// Clear pending transition state without changing the current pose.
static void rfbxClearCrossFade(RfbxInstance* instance)
{
    instance->nextAnimation = RFBX_INVALID_INDEX;
    instance->nextTime = 0.0;
    instance->fadeDuration = 0.0;
    instance->fadeTime = 0.0;
    instance->nextLoop = false;
    instance->fading = false;
    if (instance->internal) {
        instance->internal->nextEventStartPending = false;
    }
}

static void rfbxSyncRootMotionBaseline(RfbxInstance* instance);
static void rfbxResetRootMotionVisualOrigin(RfbxInstance* instance);

// Cancel pending playback without changing the currently evaluated pose.
void RfbxClearAnimationQueue(RfbxInstance* instance)
{
    if (!instance) return;
    instance->queuedAnimation = RFBX_INVALID_INDEX;
    instance->queuedFadeDuration = 0.0;
    instance->queuedLoop = false;
}

// Start a clip from zero while retaining per-instance playback controls.
bool RfbxPlayAnimation(
    RfbxInstance* instance,
    int animationIndex,
    bool loop)
{
    if (!RfbxIsInstanceValid(instance) ||
        !RfbxGetAnimation(instance->model, animationIndex)) {
        return false;
    }

    RfbxClearAnimationQueue(instance);
    rfbxClearCrossFade(instance);
    instance->loop = loop;
    instance->paused = false;
    instance->playing = true;

    if (!rfbxSetInstanceAnimationTime(instance, animationIndex, 0.0)) {
        instance->playing = false;
        return false;
    }

    instance->internal->currentEventStartPending = true;
    rfbxSyncRootMotionBaseline(instance);
    rfbxResetRootMotionVisualOrigin(instance);
    return true;
}

// Start a weighted transition while both source and target clip times advance.
bool RfbxCrossFadeAnimation(
    RfbxInstance* instance,
    int animationIndex,
    double fadeDuration,
    bool loop)
{
    if (!RfbxIsInstanceValid(instance) ||
        !RfbxGetAnimation(instance->model, animationIndex)) {
        return false;
    }

    RfbxClearAnimationQueue(instance);

    if (fadeDuration <= 0.0 ||
        instance->currentAnimation == RFBX_INVALID_INDEX) {
        return RfbxPlayAnimation(instance, animationIndex, loop);
    }

    if (instance->fading) {
        if (instance->nextAnimation == animationIndex) {
            instance->nextLoop = loop;
            return true;
        }

        bool eventStartPending =
            instance->internal->nextEventStartPending;
        instance->currentAnimation = instance->nextAnimation;
        instance->currentTime = instance->nextTime;
        instance->loop = instance->nextLoop;
        rfbxClearCrossFade(instance);
        instance->internal->currentEventStartPending = eventStartPending;
    }

    if (instance->currentAnimation == animationIndex) {
        instance->loop = loop;
        return true;
    }

    instance->nextAnimation = animationIndex;
    instance->nextTime = 0.0;
    instance->fadeDuration = fadeDuration;
    instance->fadeTime = 0.0;
    instance->nextLoop = loop;
    instance->fading = true;
    instance->playing = true;

    if (!rfbxSetInstanceBlendedAnimationTime(
        instance,
        instance->currentAnimation,
        instance->currentTime,
        instance->nextAnimation,
        instance->nextTime,
        0.0)) {
        rfbxClearCrossFade(instance);
        return false;
    }

    instance->internal->nextEventStartPending = true;
    rfbxSyncRootMotionBaseline(instance);
    return true;
}

// Select the clip exposed by the high-level playback query API.
static int rfbxGetActiveAnimationIndex(const RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance)) return RFBX_INVALID_INDEX;
    return instance->fading
        ? instance->nextAnimation
        : instance->currentAnimation;
}

// Resolve a gameplay-facing clip name and report invalid names consistently.
static int rfbxFindNamedAnimation(
    const RfbxInstance* instance,
    const char* animationName)
{
    if (!RfbxIsInstanceValid(instance)) return RFBX_INVALID_INDEX;

    int animationIndex = RfbxFindAnimation(instance->model, animationName);
    if (animationIndex == RFBX_INVALID_INDEX) {
        TraceLog(LOG_WARNING,
            "RFBX: animation '%s' was not found",
            animationName ? animationName : "<null>");
    }
    return animationIndex;
}

// Resolve a named clip and start it through the index-based core API.
bool RfbxPlay(
    RfbxInstance* instance,
    const char* animationName,
    bool loop)
{
    int animationIndex = rfbxFindNamedAnimation(instance, animationName);
    return animationIndex != RFBX_INVALID_INDEX &&
        RfbxPlayAnimation(instance, animationIndex, loop);
}

// Resolve a named clip and transition through the index-based core API.
bool RfbxCrossFade(
    RfbxInstance* instance,
    const char* animationName,
    double fadeDuration,
    bool loop)
{
    int animationIndex = rfbxFindNamedAnimation(instance, animationName);
    return animationIndex != RFBX_INVALID_INDEX &&
        RfbxCrossFadeAnimation(
            instance,
            animationIndex,
            fadeDuration,
            loop);
}

// Start one non-looping clip, restarting it when it is already active.
static bool rfbxPlayOnceByIndex(
    RfbxInstance* instance,
    int animationIndex,
    double fadeDuration)
{
    if (!RfbxIsInstanceValid(instance) ||
        !RfbxGetAnimation(instance->model, animationIndex)) {
        return false;
    }

    if (rfbxGetActiveAnimationIndex(instance) == animationIndex) {
        return RfbxPlayAnimation(instance, animationIndex, false);
    }
    return RfbxCrossFadeAnimation(
        instance,
        animationIndex,
        fadeDuration,
        false);
}

// Start a named non-looping clip and discard any older queued playback.
bool RfbxPlayOnce(
    RfbxInstance* instance,
    const char* animationName,
    double fadeDuration)
{
    int animationIndex = rfbxFindNamedAnimation(instance, animationName);
    return animationIndex != RFBX_INVALID_INDEX &&
        rfbxPlayOnceByIndex(instance, animationIndex, fadeDuration);
}

// Store one pending clip after validating that the active clip can finish.
static bool rfbxQueueAnimationByIndex(
    RfbxInstance* instance,
    int animationIndex,
    double fadeDuration,
    bool loop)
{
    int activeAnimation = rfbxGetActiveAnimationIndex(instance);
    if (activeAnimation == RFBX_INVALID_INDEX) {
        TraceLog(LOG_WARNING,
            "RFBX: cannot queue animation without an active clip");
        return false;
    }

    bool activeLoop = instance->fading
        ? instance->nextLoop
        : instance->loop;
    if (activeLoop) {
        TraceLog(LOG_WARNING,
            "RFBX: cannot queue after a looping clip; play it once first");
        return false;
    }

    if (!instance->playing && !instance->fading) {
        if (animationIndex == instance->currentAnimation) {
            return RfbxPlayAnimation(instance, animationIndex, loop);
        }
        return RfbxCrossFadeAnimation(
            instance,
            animationIndex,
            fadeDuration,
            loop);
    }

    instance->queuedAnimation = animationIndex;
    instance->queuedFadeDuration = fadeDuration > 0.0
        ? fadeDuration
        : 0.0;
    instance->queuedLoop = loop;
    return true;
}

// Queue a named clip after the active non-looping clip.
bool RfbxQueueAnimation(
    RfbxInstance* instance,
    const char* animationName,
    double fadeDuration,
    bool loop)
{
    int animationIndex = rfbxFindNamedAnimation(instance, animationName);
    return animationIndex != RFBX_INVALID_INDEX &&
        rfbxQueueAnimationByIndex(
            instance,
            animationIndex,
            fadeDuration,
            loop);
}

// Atomically validate both names, then play once and queue a looping return.
bool RfbxPlayOnceThen(
    RfbxInstance* instance,
    const char* oneShotName,
    const char* nextAnimationName,
    double fadeInDuration,
    double fadeOutDuration)
{
    int oneShotIndex = rfbxFindNamedAnimation(instance, oneShotName);
    int nextAnimationIndex = rfbxFindNamedAnimation(
        instance,
        nextAnimationName);
    if (oneShotIndex == RFBX_INVALID_INDEX ||
        nextAnimationIndex == RFBX_INVALID_INDEX) {
        return false;
    }

    if (!rfbxPlayOnceByIndex(
        instance,
        oneShotIndex,
        fadeInDuration)) {
        return false;
    }

    instance->queuedAnimation = nextAnimationIndex;
    instance->queuedFadeDuration = fadeOutDuration > 0.0
        ? fadeOutDuration
        : 0.0;
    instance->queuedLoop = true;
    return true;
}

// Re-evaluate the current base transition with the configured overlay layer.
static bool rfbxApplyCurrentInstancePose(RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance) ||
        !RfbxGetAnimation(instance->model, instance->currentAnimation)) {
        return false;
    }

    if (instance->fading) {
        double weight = instance->fadeDuration > 0.0
            ? instance->fadeTime / instance->fadeDuration
            : 1.0;
        if (weight < 0.0) weight = 0.0;
        if (weight > 1.0) weight = 1.0;
        return rfbxSetInstanceBlendedAnimationTime(
            instance,
            instance->currentAnimation,
            instance->currentTime,
            instance->nextAnimation,
            instance->nextTime,
            weight);
    }
    return rfbxSetInstanceAnimationTime(
        instance,
        instance->currentAnimation,
        instance->currentTime);
}

// Clear only layer playback and mask state, retaining the base animation.
static void rfbxClearAnimationLayer(RfbxInstance* instance)
{
    if (!instance || !instance->internal) return;

    instance->layerAnimation = RFBX_INVALID_INDEX;
    instance->layerTime = 0.0;
    instance->layerFadeDuration = 0.0;
    instance->layerFadeTime = 0.0;
    instance->layerPlaybackSpeed = 1.0f;
    instance->layerWeight = 0.0f;
    instance->layerFadeStartWeight = 0.0f;
    instance->layerTargetWeight = 0.0f;
    instance->layerLoop = false;
    instance->layerPlaying = false;
    instance->internal->layerRootBoneIndex = RFBX_INVALID_INDEX;
    instance->internal->layerEventStartPending = false;
    instance->internal->layerRemoveWhenWeightZero = false;
    if (instance->internal->layerNodeMask) {
        memset(
            instance->internal->layerNodeMask,
            0,
            (size_t)instance->internal->nodeCount * sizeof(bool));
    }
}

// Mark the selected bone node and every descendant model node.
static bool rfbxBuildAnimationLayerMask(
    RfbxInstance* instance,
    int rootBoneIndex)
{
    const RfbxBone* bone = RfbxGetBone(instance->model, rootBoneIndex);
    if (!bone || !instance->internal->layerNodeMask) return false;

    int rootNodeIndex = RFBX_INVALID_INDEX;
    for (int nodeIndex = 0;
        nodeIndex < instance->model->nodeCount;
        ++nodeIndex) {
        if (instance->model->nodes[nodeIndex].sourceTypedId ==
            bone->sourceNodeTypedId) {
            rootNodeIndex = nodeIndex;
            break;
        }
    }
    if (rootNodeIndex == RFBX_INVALID_INDEX) return false;

    memset(
        instance->internal->layerNodeMask,
        0,
        (size_t)instance->internal->nodeCount * sizeof(bool));
    for (int nodeIndex = 0;
        nodeIndex < instance->model->nodeCount;
        ++nodeIndex) {
        for (int ancestor = nodeIndex;
            ancestor != RFBX_INVALID_INDEX;
            ancestor = instance->model->nodes[ancestor].parentIndex) {
            if (ancestor == rootNodeIndex) {
                instance->internal->layerNodeMask[nodeIndex] = true;
                break;
            }
        }
    }
    instance->internal->layerRootBoneIndex = rootBoneIndex;
    return true;
}

// Start one override layer on a named bone subtree.
bool RfbxPlayAnimationLayer(
    RfbxInstance* instance,
    const char* animationName,
    const char* maskRootBoneName,
    float weight,
    double fadeInDuration,
    bool loop)
{
    if (!RfbxIsInstanceValid(instance) ||
        instance->currentAnimation == RFBX_INVALID_INDEX) {
        TraceLog(LOG_WARNING,
            "RFBX: animation layer requires an active base clip");
        return false;
    }
    if (!isfinite(weight) || !isfinite(fadeInDuration)) return false;

    int animationIndex = rfbxFindNamedAnimation(instance, animationName);
    int boneIndex = RfbxFindBone(instance->model, maskRootBoneName);
    if (animationIndex == RFBX_INVALID_INDEX ||
        boneIndex == RFBX_INVALID_INDEX) {
        if (boneIndex == RFBX_INVALID_INDEX) {
            TraceLog(LOG_WARNING,
                "RFBX: layer mask bone '%s' was not found",
                maskRootBoneName ? maskRootBoneName : "<null>");
        }
        return false;
    }

    if (weight < 0.0f) weight = 0.0f;
    if (weight > 1.0f) weight = 1.0f;
    rfbxClearAnimationLayer(instance);
    if (!rfbxBuildAnimationLayerMask(instance, boneIndex)) return false;

    instance->layerAnimation = animationIndex;
    instance->layerTime = 0.0;
    instance->layerFadeDuration = fadeInDuration > 0.0
        ? fadeInDuration
        : 0.0;
    instance->layerFadeTime = 0.0;
    instance->layerPlaybackSpeed = 1.0f;
    instance->layerWeight = fadeInDuration > 0.0 ? 0.0f : weight;
    instance->layerFadeStartWeight = instance->layerWeight;
    instance->layerTargetWeight = weight;
    instance->layerLoop = loop;
    instance->layerPlaying = true;
    instance->internal->layerEventStartPending = true;

    if (!rfbxApplyCurrentInstancePose(instance)) {
        rfbxClearAnimationLayer(instance);
        return false;
    }
    rfbxSyncRootMotionBaseline(instance);
    return true;
}

// Change the target weight while retaining clip time and mask selection.
void RfbxSetAnimationLayerWeight(
    RfbxInstance* instance,
    float weight,
    double fadeDuration)
{
    if (!rfbxHasAnimationLayer(instance)) return;
    if (!isfinite(weight) || !isfinite(fadeDuration)) return;
    if (weight < 0.0f) weight = 0.0f;
    if (weight > 1.0f) weight = 1.0f;

    instance->layerFadeStartWeight = instance->layerWeight;
    instance->layerTargetWeight = weight;
    instance->layerFadeDuration = fadeDuration > 0.0
        ? fadeDuration
        : 0.0;
    instance->layerFadeTime = 0.0;
    instance->internal->layerRemoveWhenWeightZero = false;
    if (fadeDuration <= 0.0) {
        instance->layerWeight = weight;
        if (!rfbxApplyCurrentInstancePose(instance)) {
            TraceLog(LOG_WARNING,
                "RFBX: failed to apply animation layer weight");
        }
        rfbxSyncRootMotionBaseline(instance);
    }
}

// Fade to zero and release the layer state when the fade completes.
void RfbxStopAnimationLayer(
    RfbxInstance* instance,
    double fadeOutDuration)
{
    if (!rfbxHasAnimationLayer(instance)) return;
    if (!isfinite(fadeOutDuration)) return;
    if (fadeOutDuration > 0.0 && instance->layerWeight > 0.0f) {
        instance->layerFadeStartWeight = instance->layerWeight;
        instance->layerTargetWeight = 0.0f;
        instance->layerFadeDuration = fadeOutDuration;
        instance->layerFadeTime = 0.0;
        instance->internal->layerRemoveWhenWeightZero = true;
        return;
    }

    rfbxClearAnimationLayer(instance);
    if (!rfbxApplyCurrentInstancePose(instance)) {
        TraceLog(LOG_WARNING,
            "RFBX: failed to restore base pose after layer stop");
    }
    rfbxSyncRootMotionBaseline(instance);
}

// Set a finite independent time scale for the overlay clip.
void RfbxSetAnimationLayerSpeed(RfbxInstance* instance, float speed)
{
    if (!RfbxIsInstanceValid(instance) || !isfinite(speed)) return;
    instance->layerPlaybackSpeed = speed;
}

// Report whether an overlay clip and bone mask are configured.
bool RfbxIsAnimationLayerActive(const RfbxInstance* instance)
{
    return rfbxHasAnimationLayer(instance);
}

// Report whether the layer time advances with the next update.
bool RfbxIsAnimationLayerPlaying(const RfbxInstance* instance)
{
    return rfbxHasAnimationLayer(instance) &&
        instance->layerPlaying &&
        !instance->paused;
}

// Return the model-owned layer clip name.
const char* RfbxGetAnimationLayerName(const RfbxInstance* instance)
{
    const RfbxAnimation* animation = rfbxHasAnimationLayer(instance)
        ? RfbxGetAnimation(instance->model, instance->layerAnimation)
        : NULL;
    return animation ? animation->name : NULL;
}

// Return the model-owned bone name used to build the subtree mask.
const char* RfbxGetAnimationLayerRootBoneName(
    const RfbxInstance* instance)
{
    const RfbxBone* bone = rfbxHasAnimationLayer(instance)
        ? RfbxGetBone(
            instance->model,
            instance->internal->layerRootBoneIndex)
        : NULL;
    return bone ? bone->name : NULL;
}

// Return the effective overlay amount after fade evaluation.
float RfbxGetAnimationLayerWeight(const RfbxInstance* instance)
{
    return rfbxHasAnimationLayer(instance) ? instance->layerWeight : 0.0f;
}

// Return clip-local layer time without exposing internal evaluation state.
double RfbxGetAnimationLayerTime(const RfbxInstance* instance)
{
    return rfbxHasAnimationLayer(instance) ? instance->layerTime : 0.0;
}

// Report whether a valid clip is waiting to be played.
bool RfbxHasQueuedAnimation(const RfbxInstance* instance)
{
    return RfbxIsInstanceValid(instance) &&
        RfbxGetAnimation(
            instance->model,
            instance->queuedAnimation) != NULL;
}

// Return the model-owned queued clip name without allocating memory.
const char* RfbxGetQueuedAnimationName(const RfbxInstance* instance)
{
    const RfbxAnimation* animation = RfbxHasQueuedAnimation(instance)
        ? RfbxGetAnimation(instance->model, instance->queuedAnimation)
        : NULL;
    return animation ? animation->name : NULL;
}

// Return a model-owned active clip name without allocating memory.
const char* RfbxGetCurrentAnimationName(const RfbxInstance* instance)
{
    int animationIndex = rfbxGetActiveAnimationIndex(instance);
    const RfbxAnimation* animation = RfbxIsInstanceValid(instance)
        ? RfbxGetAnimation(instance->model, animationIndex)
        : NULL;
    return animation ? animation->name : NULL;
}

// Return the active clip-local time used by gameplay-facing queries.
double RfbxGetAnimationTime(const RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance)) return 0.0;
    return instance->fading ? instance->nextTime : instance->currentTime;
}

// Convert active clip-local time to a clamped normalized value.
float RfbxGetAnimationProgress(const RfbxInstance* instance)
{
    int animationIndex = rfbxGetActiveAnimationIndex(instance);
    const RfbxAnimation* animation = RfbxIsInstanceValid(instance)
        ? RfbxGetAnimation(instance->model, animationIndex)
        : NULL;
    if (!animation || animation->duration <= 0.0) return 0.0f;

    double progress = RfbxGetAnimationTime(instance) / animation->duration;
    if (progress < 0.0) progress = 0.0;
    if (progress > 1.0) progress = 1.0;
    return (float)progress;
}

// Report whether a selected clip is actively advancing rather than paused.
bool RfbxIsAnimationPlaying(const RfbxInstance* instance)
{
    return RfbxIsInstanceValid(instance) &&
        instance->playing &&
        !instance->paused;
}

// Report the paused state only while a clip is selected.
bool RfbxIsAnimationPaused(const RfbxInstance* instance)
{
    return RfbxIsInstanceValid(instance) &&
        instance->currentAnimation != RFBX_INVALID_INDEX &&
        instance->paused;
}

// Hide the internal transition fields behind one stable query.
bool RfbxIsAnimationFading(const RfbxInstance* instance)
{
    return RfbxIsInstanceValid(instance) && instance->fading;
}

// Report completion only after any active transition has also completed.
bool RfbxIsAnimationFinished(const RfbxInstance* instance)
{
    return RfbxIsInstanceValid(instance) &&
        instance->currentAnimation != RFBX_INVALID_INDEX &&
        !instance->playing &&
        !instance->fading &&
        instance->queuedAnimation == RFBX_INVALID_INDEX;
}

// Restore the model's loaded pose without touching shared model resources.
void RfbxStopAnimation(RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance)) return;

    instance->loop = false;
    instance->paused = false;
    instance->playing = false;
    RfbxClearAnimationQueue(instance);
    rfbxClearCrossFade(instance);
    rfbxClearAnimationLayer(instance);
    instance->internal->currentEventStartPending = false;
    RfbxClearPendingAnimationEvents(instance);

    const RfbxModel* model = instance->model;
    if (!model->internal ||
        !model->internal->scene ||
        !rfbxApplySceneStateToInstance(instance, model->internal->scene)) {
        TraceLog(LOG_WARNING, "RFBX: failed to reset instance pose");
        return;
    }

    instance->currentAnimation = RFBX_INVALID_INDEX;
    instance->currentTime = 0.0;
    RfbxResetRootMotion(instance);
}

// Set the paused state without changing the current pose.
void RfbxSetAnimationPaused(RfbxInstance* instance, bool paused)
{
    if (!RfbxIsInstanceValid(instance)) return;
    instance->paused = paused;
}

// Set a playback multiplier, including zero or reverse playback.
void RfbxSetAnimationSpeed(RfbxInstance* instance, float speed)
{
    if (!RfbxIsInstanceValid(instance)) return;
    instance->playbackSpeed = speed;
}

// Seek the active clip, promoting the destination if a cross-fade is active.
bool RfbxSetAnimationTime(RfbxInstance* instance, double timeSeconds)
{
    if (!RfbxIsInstanceValid(instance) ||
        instance->currentAnimation == RFBX_INVALID_INDEX) {
        return false;
    }

    if (instance->fading) {
        bool eventStartPending =
            instance->internal->nextEventStartPending;
        instance->currentAnimation = instance->nextAnimation;
        instance->currentTime = instance->nextTime;
        instance->loop = instance->nextLoop;
        instance->internal->currentEventStartPending = eventStartPending;
    }
    rfbxClearCrossFade(instance);
    bool success = rfbxSetInstanceAnimationTime(
        instance,
        instance->currentAnimation,
        timeSeconds);
    if (success) {
        instance->internal->currentEventStartPending = false;
        RfbxResetRootMotion(instance);
    }
    return success;
}

// Consume the queued clip and start it without letting core APIs erase it first.
static bool rfbxStartQueuedAnimation(RfbxInstance* instance)
{
    if (!RfbxHasQueuedAnimation(instance)) return false;

    int animationIndex = instance->queuedAnimation;
    double fadeDuration = instance->queuedFadeDuration;
    bool loop = instance->queuedLoop;
    RfbxClearAnimationQueue(instance);

    if (!instance->fading &&
        animationIndex == instance->currentAnimation) {
        return RfbxPlayAnimation(instance, animationIndex, loop);
    }
    return RfbxCrossFadeAnimation(
        instance,
        animationIndex,
        fadeDuration,
        loop);
}

// Decide when a queued fade must begin for the active non-looping clip.
static bool rfbxShouldStartQueuedAnimation(
    const RfbxInstance* instance,
    const RfbxAnimation* animation,
    double localTime,
    bool finished)
{
    if (!RfbxHasQueuedAnimation(instance) ||
        !animation ||
        instance->loop) {
        return false;
    }

    double fadeLead = instance->queuedFadeDuration *
        fabs((double)instance->playbackSpeed);
    double remainingTime = animation->duration - localTime;
    if (remainingTime < 0.0) remainingTime = 0.0;
    return finished || remainingTime <= fadeLead;
}

// Append markers crossed by one monotonic clip-time segment.
static bool rfbxEmitAnimationEventRange(
    RfbxInstance* instance,
    const RfbxAnimationEventListInternal* list,
    double fromTime,
    double toTime,
    bool forward,
    bool includeFrom)
{
    if (forward) {
        for (int eventIndex = 0;
            eventIndex < list->count;
            ++eventIndex) {
            const RfbxAnimationEvent* event = &list->events[eventIndex];
            bool afterStart = event->time > fromTime ||
                (includeFrom && event->time == fromTime);
            if (afterStart && event->time <= toTime &&
                !rfbxPushPendingAnimationEvent(instance, event)) {
                TraceLog(LOG_WARNING,
                    "RFBX: failed to queue animation event '%s'",
                    event->name);
                return false;
            }
        }
    }
    else {
        for (int eventIndex = list->count; eventIndex-- > 0;) {
            const RfbxAnimationEvent* event = &list->events[eventIndex];
            bool beforeStart = event->time < fromTime ||
                (includeFrom && event->time == fromTime);
            if (beforeStart && event->time >= toTime &&
                !rfbxPushPendingAnimationEvent(instance, event)) {
                TraceLog(LOG_WARNING,
                    "RFBX: failed to queue animation event '%s'",
                    event->name);
                return false;
            }
        }
    }
    return true;
}

// Emit all markers crossed by one update, including loop wrap-around.
static bool rfbxEmitAnimationEvents(
    RfbxInstance* instance,
    int animationIndex,
    double oldTime,
    double deltaSeconds,
    bool loop,
    bool* startPending)
{
    if (!RfbxIsInstanceValid(instance) || !startPending) return false;

    const RfbxModel* model = instance->model;
    const RfbxAnimation* animation = RfbxGetAnimation(
        model,
        animationIndex);
    bool includeStart = *startPending;
    *startPending = false;

    if (!animation ||
        !model->internal ||
        !model->internal->animationEventLists) {
        return animation != NULL;
    }

    const RfbxAnimationEventListInternal* list =
        &model->internal->animationEventLists[animationIndex];
    if (list->count == 0) return true;

    double duration = animation->duration;
    if (duration <= 0.0) {
        return includeStart && oldTime == 0.0
            ? rfbxEmitAnimationEventRange(
                instance,
                list,
                0.0,
                0.0,
                true,
                true)
            : true;
    }

    if (oldTime < 0.0) oldTime = 0.0;
    if (oldTime > duration) oldTime = duration;

    if (!loop) {
        double newTime = oldTime + deltaSeconds;
        if (newTime < 0.0) newTime = 0.0;
        if (newTime > duration) newTime = duration;
        bool forward = newTime >= oldTime;
        return rfbxEmitAnimationEventRange(
            instance,
            list,
            oldTime,
            newTime,
            forward,
            includeStart);
    }

    if (deltaSeconds == 0.0) {
        return includeStart
            ? rfbxEmitAnimationEventRange(
                instance,
                list,
                oldTime,
                oldTime,
                true,
                true)
            : true;
    }

    const int maxWrapCount = 1024;
    int wrapCount = 0;
    double remaining = fabs(deltaSeconds);
    double cursor = oldTime;
    bool forward = deltaSeconds > 0.0;

    for (;;) {
        double boundaryDistance = forward
            ? duration - cursor
            : cursor;
        if (remaining < boundaryDistance) {
            double endTime = forward
                ? cursor + remaining
                : cursor - remaining;
            return rfbxEmitAnimationEventRange(
                instance,
                list,
                cursor,
                endTime,
                forward,
                includeStart);
        }

        double boundary = forward ? duration : 0.0;
        if (!rfbxEmitAnimationEventRange(
            instance,
            list,
            cursor,
            boundary,
            forward,
            includeStart)) {
            return false;
        }

        remaining -= boundaryDistance;
        cursor = forward ? 0.0 : duration;
        includeStart = true;
        if (++wrapCount > maxWrapCount) {
            TraceLog(LOG_WARNING,
                "RFBX: animation event loop limit exceeded for '%s'",
                animation->name);
            return true;
        }
        if (remaining == 0.0) {
            return rfbxEmitAnimationEventRange(
                instance,
                list,
                cursor,
                cursor,
                forward,
                true);
        }
    }
}

// Advance independent overlay time, fade weight, and layer event markers.
static bool rfbxAdvanceAnimationLayer(
    RfbxInstance* instance,
    double deltaSeconds)
{
    if (!rfbxHasAnimationLayer(instance)) return true;

    const RfbxAnimation* animation = RfbxGetAnimation(
        instance->model,
        instance->layerAnimation);
    if (!animation) return false;

    if (instance->layerPlaying) {
        double oldTime = instance->layerTime;
        double layerDelta = deltaSeconds *
            (double)instance->layerPlaybackSpeed;
        bool finished = false;
        instance->layerTime = rfbxAdvanceAnimationTime(
            animation,
            instance->layerTime,
            layerDelta,
            instance->layerLoop,
            &finished);
        if (!rfbxEmitAnimationEvents(
            instance,
            instance->layerAnimation,
            oldTime,
            layerDelta,
            instance->layerLoop,
            &instance->internal->layerEventStartPending)) {
            return false;
        }
        if (!instance->layerLoop && finished) {
            instance->layerPlaying = false;
        }
    }

    if (instance->layerFadeDuration > 0.0) {
        instance->layerFadeTime += fabs(deltaSeconds);
        double amount = instance->layerFadeTime /
            instance->layerFadeDuration;
        if (amount > 1.0) amount = 1.0;
        instance->layerWeight = instance->layerFadeStartWeight +
            (instance->layerTargetWeight -
                instance->layerFadeStartWeight) * (float)amount;

        if (amount >= 1.0) {
            bool remove =
                instance->internal->layerRemoveWhenWeightZero &&
                instance->layerTargetWeight <= 0.0f;
            instance->layerFadeDuration = 0.0;
            instance->layerFadeTime = 0.0;
            instance->layerFadeStartWeight = instance->layerWeight;
            instance->internal->layerRemoveWhenWeightZero = false;
            if (remove) rfbxClearAnimationLayer(instance);
        }
    }
    return true;
}

// Extract position and orientation while intentionally ignoring node scale.
static void rfbxDecomposeRootTransform(
    Matrix transform,
    Vector3* translation,
    Quaternion* rotation)
{
    Vector3 scale = { 0 };
    MatrixDecompose(transform, translation, rotation, &scale);
    *rotation = QuaternionNormalize(*rotation);
}

// Calculate model-space motion from one evaluated root transform to another.
static RfbxRootMotion rfbxRootMotionBetween(Matrix from, Matrix to)
{
    Vector3 fromTranslation = { 0 };
    Vector3 toTranslation = { 0 };
    Quaternion fromRotation = { 0 };
    Quaternion toRotation = { 0 };
    rfbxDecomposeRootTransform(
        from,
        &fromTranslation,
        &fromRotation);
    rfbxDecomposeRootTransform(
        to,
        &toTranslation,
        &toRotation);

    RfbxRootMotion result = { 0 };
    result.translation = Vector3Subtract(toTranslation, fromTranslation);
    result.rotation = QuaternionNormalize(QuaternionMultiply(
        toRotation,
        QuaternionInvert(fromRotation)));
    return result;
}

// Compose a later motion segment after the current accumulated result.
static void rfbxAppendRootMotion(
    RfbxRootMotion* result,
    RfbxRootMotion segment)
{
    result->translation = Vector3Add(
        result->translation,
        segment.translation);
    result->rotation = QuaternionNormalize(QuaternionMultiply(
        segment.rotation,
        result->rotation));
}

// Count signed loop boundary crossings for one clip-time update.
static int rfbxGetRootMotionWrapCount(
    double oldTime,
    double deltaSeconds,
    double duration)
{
    if (duration <= 0.0 || deltaSeconds == 0.0) return 0;

    double rawTime = oldTime + deltaSeconds;
    double wrapCount = 0.0;
    if (deltaSeconds > 0.0 && rawTime >= duration) {
        wrapCount = floor(rawTime / duration);
    }
    else if (deltaSeconds < 0.0 && rawTime < 0.0) {
        wrapCount = -ceil(-rawTime / duration);
    }

    const double maxWrapCount = 1024.0;
    if (wrapCount > maxWrapCount) return (int)maxWrapCount;
    if (wrapCount < -maxWrapCount) return -(int)maxWrapCount;
    return (int)wrapCount;
}

// Make the currently evaluated root pose the next delta baseline.
static void rfbxSyncRootMotionBaseline(RfbxInstance* instance)
{
    if (!RfbxIsRootMotionEnabled(instance)) return;

    int nodeIndex = instance->internal->rootMotionNodeIndex;
    instance->internal->rootMotionPreviousTransform =
        instance->internal->nodes[nodeIndex].nodeToWorld;
    instance->internal->rootMotionHasPrevious = true;
}

// Begin a fresh in-place visual reference without discarding pending motion.
static void rfbxResetRootMotionVisualOrigin(RfbxInstance* instance)
{
    if (!RfbxIsRootMotionEnabled(instance)) return;

    int nodeIndex = instance->internal->rootMotionNodeIndex;
    instance->internal->rootMotionVisualOriginTransform =
        instance->internal->nodes[nodeIndex].nodeToWorld;
}

// Extract one update of motion, compensating pure clip loop boundaries.
static void rfbxUpdateRootMotion(
    RfbxInstance* instance,
    int animationIndex,
    double oldTime,
    double deltaSeconds,
    bool loop,
    bool allowLoopCompensation)
{
    if (!RfbxIsRootMotionEnabled(instance)) return;

    RfbxInstanceInternal* internal = instance->internal;
    int nodeIndex = internal->rootMotionNodeIndex;
    Matrix currentTransform = internal->nodes[nodeIndex].nodeToWorld;
    if (!internal->rootMotionHasPrevious) {
        internal->rootMotionPreviousTransform = currentTransform;
        internal->rootMotionHasPrevious = true;
        return;
    }

    const RfbxAnimation* animation = RfbxGetAnimation(
        instance->model,
        animationIndex);
    int wrapCount = animation && loop
        ? rfbxGetRootMotionWrapCount(
            oldTime,
            deltaSeconds,
            animation->duration)
        : 0;

    RfbxRootMotion motion = rfbxIdentityRootMotion();
    if (wrapCount == 0) {
        motion = rfbxRootMotionBetween(
            internal->rootMotionPreviousTransform,
            currentTransform);
    }
    else if (!allowLoopCompensation ||
        !internal->rootMotionStartTransforms ||
        !internal->rootMotionEndTransforms) {
        // A blended wrap cannot be reconstructed reliably: suppress the snap.
    }
    else if (wrapCount > 0) {
        Matrix start = internal->rootMotionStartTransforms[animationIndex];
        Matrix end = internal->rootMotionEndTransforms[animationIndex];
        rfbxAppendRootMotion(
            &motion,
            rfbxRootMotionBetween(
                internal->rootMotionPreviousTransform,
                end));
        RfbxRootMotion cycle = rfbxRootMotionBetween(start, end);
        for (int wrapIndex = 1;
            wrapIndex < wrapCount;
            ++wrapIndex) {
            rfbxAppendRootMotion(&motion, cycle);
        }
        rfbxAppendRootMotion(
            &motion,
            rfbxRootMotionBetween(start, currentTransform));
    }
    else {
        Matrix start = internal->rootMotionStartTransforms[animationIndex];
        Matrix end = internal->rootMotionEndTransforms[animationIndex];
        rfbxAppendRootMotion(
            &motion,
            rfbxRootMotionBetween(
                internal->rootMotionPreviousTransform,
                start));
        RfbxRootMotion reverseCycle = rfbxRootMotionBetween(end, start);
        for (int wrapIndex = -1;
            wrapIndex > wrapCount;
            --wrapIndex) {
            rfbxAppendRootMotion(&motion, reverseCycle);
        }
        rfbxAppendRootMotion(
            &motion,
            rfbxRootMotionBetween(end, currentTransform));
    }

    internal->rootMotionPreviousTransform = currentTransform;
    rfbxAppendRootMotion(&internal->pendingRootMotion, motion);
}

// Advance local clip time, handle looping, and evaluate the resulting pose.
bool RfbxUpdateInstance(RfbxInstance* instance, double deltaSeconds)
{
    if (!RfbxIsInstanceValid(instance)) return false;
    if (instance->paused) return true;

    bool updateBase = instance->playing;
    bool updateLayer = rfbxHasAnimationLayer(instance) &&
        (instance->layerPlaying || instance->layerFadeDuration > 0.0);
    if (!updateBase && !updateLayer) return true;

    double scaledDelta = updateBase
        ? deltaSeconds * (double)instance->playbackSpeed
        : 0.0;
    const RfbxAnimation* animation = RfbxGetAnimation(
        instance->model,
        instance->currentAnimation);
    if (!animation) return false;

    if (instance->fading) {
        const RfbxAnimation* nextAnimation = RfbxGetAnimation(
            instance->model,
            instance->nextAnimation);
        if (!nextAnimation) return false;

        double oldNextTime = instance->nextTime;
        bool nextFinished = false;
        instance->currentTime = rfbxAdvanceAnimationTime(
            animation,
            instance->currentTime,
            scaledDelta,
            instance->loop,
            NULL);
        instance->nextTime = rfbxAdvanceAnimationTime(
            nextAnimation,
            instance->nextTime,
            scaledDelta,
            instance->nextLoop,
            &nextFinished);
        instance->fadeTime += fabs(deltaSeconds);

        double blendWeight = instance->fadeDuration > 0.0
            ? instance->fadeTime / instance->fadeDuration
            : 1.0;
        if (blendWeight > 1.0) blendWeight = 1.0;

        if (!rfbxAdvanceAnimationLayer(instance, deltaSeconds)) {
            return false;
        }

        if (!rfbxSetInstanceBlendedAnimationTime(
            instance,
            instance->currentAnimation,
            instance->currentTime,
            instance->nextAnimation,
            instance->nextTime,
            blendWeight)) {
            return false;
        }

        if (!rfbxEmitAnimationEvents(
            instance,
            instance->nextAnimation,
            oldNextTime,
            scaledDelta,
            instance->nextLoop,
            &instance->internal->nextEventStartPending)) {
            return false;
        }
        rfbxUpdateRootMotion(
            instance,
            instance->nextAnimation,
            oldNextTime,
            scaledDelta,
            instance->nextLoop,
            false);

        if (blendWeight >= 1.0) {
            bool eventStartPending =
                instance->internal->nextEventStartPending;
            instance->currentAnimation = instance->nextAnimation;
            instance->currentTime = instance->nextTime;
            instance->loop = instance->nextLoop;
            rfbxClearCrossFade(instance);
            instance->internal->currentEventStartPending = eventStartPending;
            instance->playing = instance->loop || !nextFinished;
            if (rfbxShouldStartQueuedAnimation(
                instance,
                nextAnimation,
                instance->currentTime,
                nextFinished)) {
                return rfbxStartQueuedAnimation(instance);
            }
        }
        return true;
    }

    double oldTime = instance->currentTime;
    bool finished = false;
    double localTime = updateBase
        ? rfbxAdvanceAnimationTime(
            animation,
            instance->currentTime,
            scaledDelta,
            instance->loop,
            &finished)
        : instance->currentTime;
    if (!rfbxAdvanceAnimationLayer(instance, deltaSeconds)) {
        return false;
    }
    if (!rfbxSetInstanceAnimationTime(
        instance,
        instance->currentAnimation,
        localTime)) {
        return false;
    }

    if (!updateBase) {
        rfbxSyncRootMotionBaseline(instance);
        return true;
    }

    if (!rfbxEmitAnimationEvents(
        instance,
        instance->currentAnimation,
        oldTime,
        scaledDelta,
        instance->loop,
        &instance->internal->currentEventStartPending)) {
        return false;
    }
    rfbxUpdateRootMotion(
        instance,
        instance->currentAnimation,
        oldTime,
        scaledDelta,
        instance->loop,
        true);

    if (rfbxShouldStartQueuedAnimation(
        instance,
        animation,
        localTime,
        finished)) {
        return rfbxStartQueuedAnimation(instance);
    }

    instance->playing = instance->loop || !finished;
    return true;
}

static int rfbxResolveMaterialIndex(
    const RfbxModel* model,
    const RfbxNode* node,
    const RfbxMesh* mesh,
    const RfbxPrimitive* primitive);

static Material rfbxPrepareDrawMaterial(
    const RfbxModel* model,
    int materialIndex,
    Color tint,
    MaterialMap* maps);

static bool rfbxIsMaterialBlended(
    const RfbxModel* model,
    int materialIndex);

static void rfbxDrawMeshWithMaterialState(
    const RfbxModel* model,
    int materialIndex,
    Mesh mesh,
    Material material,
    Matrix transform);



// Draw an independently evaluated pose using shared static meshes where possible.
void RfbxDrawInstanceMatrix(
    const RfbxInstance* instance,
    Matrix transform,
    Color tint)
{
    if (!RfbxIsInstanceValid(instance)) return;

    const RfbxModel* model = instance->model;
    if (!model->defaultMaterialReady) return;

    Matrix drawTransform = transform;
    if (RfbxIsRootMotionInPlace(instance)) {
        drawTransform = MatrixMultiply(
            RfbxGetRootMotionCorrection(instance),
            transform);
    }

    // Opaque/cutout primitives write depth before blended primitives.
    for (int renderPass = 0; renderPass < 2; ++renderPass) {
        for (int nodeIndex = 0; nodeIndex < model->nodeCount; ++nodeIndex) {
            const RfbxNode* modelNode = &model->nodes[nodeIndex];
            const RfbxInstanceNodeState* node =
                &instance->internal->nodes[nodeIndex];
            if (!node->visible ||
                modelNode->meshIndex < 0 ||
                modelNode->meshIndex >= model->meshCount) {
                continue;
            }

            const RfbxMesh* modelMesh = &model->meshes[modelNode->meshIndex];
            const RfbxInstanceMeshState* instanceMesh =
                &instance->internal->meshes[modelNode->meshIndex];

            bool verticesAreLocal = !modelMesh->hasSkinning ||
                instanceMesh->verticesAreLocal;
            Matrix finalTransform = drawTransform;
            if (verticesAreLocal) {

                const Vector3 multiplier = node->scaleMultiplier;

                const Matrix correction = MatrixScale(
                    multiplier.x, multiplier.y, multiplier.z);

                // メッシュのローカル原点を中心に拡縮してから、
                // 元のノード変換とゲーム内の配置を適用する

                //NOTE : 最終的なジオメトリ
                const Matrix correctedGeometry = MatrixMultiply(
                    correction,
                    node->geometryToWorld);

                const Matrix adjustedGeometry = MatrixMultiply(
                    correctedGeometry,
                    MatrixTranslate(node->offset.x, node->offset.y, node->offset.z));

                finalTransform = MatrixMultiply(
                    adjustedGeometry,
                    drawTransform);
            }

            for (int primitiveIndex = 0;
                primitiveIndex < modelMesh->primitiveCount;
                ++primitiveIndex) {
                const RfbxPrimitive* primitive =
                    &modelMesh->primitives[primitiveIndex];
                int materialIndex = rfbxResolveMaterialIndex(
                    model,
                    modelNode,
                    modelMesh,
                    primitive);
                bool blended = rfbxIsMaterialBlended(model, materialIndex);
                if (blended != (renderPass == 1)) continue;

                Mesh mesh = primitive->mesh;
                if (modelMesh->hasSkinning && !instanceMesh->gpuSkinning) {
                    mesh = instanceMesh->primitives[primitiveIndex];
                }

                MaterialMap maps[RFBX_DRAW_MATERIAL_MAP_COUNT] = { 0 };
                Material material = rfbxPrepareDrawMaterial(
                    model,
                    materialIndex,
                    tint,
                    maps);
                if (!rfbxPrepareShaderDraw(
                    model,
                    &material,
                    instanceMesh->boneMatrices,
                    instanceMesh->boneCount,
                    instanceMesh->gpuSkinning,
                    materialIndex)) {
                    continue;
                }
                rfbxDrawMeshWithMaterialState(
                    model,
                    materialIndex,
                    mesh,
                    material,
                    finalTransform);
            }
        }
    }
}

// Build a raylib-style root transform and draw an instance.
void RfbxDrawInstance(
    const RfbxInstance* instance,
    Vector3 position,
    float scale,
    Color tint)
{
    Matrix transform = MatrixScale(scale, scale, scale);
    transform.m12 = position.x;
    transform.m13 = position.y;
    transform.m14 = position.z;
    RfbxDrawInstanceMatrix(instance, transform, tint);
}

void RfbxDrawInstanceEx(const RfbxInstance* instance, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    Matrix transform = MatrixMultiply(
        MatrixScale(scale.x, scale.y, scale.z),
        MatrixRotate(rotationAxis, rotationAngle * DEG2RAD));

    transform.m12 = position.x;
    transform.m13 = position.y;
    transform.m14 = position.z;

    RfbxDrawInstanceMatrix(instance, transform, tint);
}

// Unload only instance-owned dynamic meshes and pose arrays.
void RfbxUnloadInstance(RfbxInstance* instance)
{
    if (!instance) return;

    if (instance->internal) {
        if (instance->internal->meshes) {
            for (int meshIndex = 0;
                meshIndex < instance->internal->meshCount;
                ++meshIndex) {
                RfbxInstanceMeshState* mesh =
                    &instance->internal->meshes[meshIndex];
                if (mesh->primitives) {
                    for (int primitiveIndex = 0;
                        primitiveIndex < mesh->primitiveCount;
                        ++primitiveIndex) {
                        Mesh* primitive = &mesh->primitives[primitiveIndex];
                        if (primitive->vertices || primitive->vboId) {
                            UnloadMesh(*primitive);
                        }
                    }
                    MemFree(mesh->primitives);
                }
                MemFree(mesh->boneMatrices);
            }
            MemFree(instance->internal->meshes);
        }

        MemFree(instance->internal->boneToWorld);
        MemFree(instance->internal->blendOverrides);
        MemFree(instance->internal->layerNodeMask);
        MemFree(instance->internal->pendingEvents);
        MemFree(instance->internal->rootMotionStartTransforms);
        MemFree(instance->internal->rootMotionEndTransforms);
        MemFree(instance->internal->nodes);
        MemFree(instance->internal);
    }

    memset(instance, 0, sizeof(*instance));
}

// Resolve per-node material overrides, falling back to mesh material slots.
static int rfbxResolveMaterialIndex(
    const RfbxModel* model,
    const RfbxNode* node,
    const RfbxMesh* mesh,
    const RfbxPrimitive* primitive)
{
    int slot = primitive->materialSlot;
    int materialIndex = RFBX_INVALID_INDEX;

    if (slot >= 0 && slot < node->materialCount) {
        materialIndex = node->materialIndices[slot];
    }
    else if (slot >= 0 && slot < mesh->materialCount) {
        materialIndex = mesh->materialIndices[slot];
    }

    if (materialIndex < 0 || materialIndex >= model->materialCount) {
        return RFBX_INVALID_INDEX;
    }
    return materialIndex;
}

// Copy and tint a material without mutating shared model state.
static Material rfbxPrepareDrawMaterial(
    const RfbxModel* model,
    int materialIndex,
    Color tint,
    MaterialMap* maps)
{
    Material result = model->defaultMaterial;
    if (materialIndex >= 0 && materialIndex < model->materialCount) {
        const RfbxMaterial* material = &model->materials[materialIndex];
        if (material->materialReady) result = material->material;
    }

    memcpy(maps, result.maps, sizeof(MaterialMap) * (MATERIAL_MAP_BRDF + 1));
    Color baseColor = maps[MATERIAL_MAP_ALBEDO].color;
    maps[MATERIAL_MAP_ALBEDO].color = (Color){
        (unsigned char)(((unsigned int)baseColor.r * tint.r + 127u) / 255u),
        (unsigned char)(((unsigned int)baseColor.g * tint.g + 127u) / 255u),
        (unsigned char)(((unsigned int)baseColor.b * tint.b + 127u) / 255u),
        (unsigned char)(((unsigned int)baseColor.a * tint.a + 127u) / 255u)
    };
    result.maps = maps;
    return result;
}

// Return whether a primitive belongs in the transparent render pass.
static bool rfbxIsMaterialBlended(
    const RfbxModel* model,
    int materialIndex)
{
    return model && materialIndex >= 0 && materialIndex < model->materialCount &&
        model->materials[materialIndex].alphaMode == RFBX_ALPHA_BLEND;
}

// Apply per-material raster state and restore raylib's default state afterwards.
static void rfbxDrawMeshWithMaterialState(
    const RfbxModel* model,
    int materialIndex,
    Mesh mesh,
    Material material,
    Matrix transform)
{
    const RfbxMaterial* sourceMaterial =
        model && materialIndex >= 0 && materialIndex < model->materialCount
        ? &model->materials[materialIndex]
        : NULL;
    bool blended = sourceMaterial &&
        sourceMaterial->alphaMode == RFBX_ALPHA_BLEND;
    bool doubleSided = sourceMaterial && sourceMaterial->doubleSided;

    if (doubleSided) rlDisableBackfaceCulling();
    if (blended) {
        rlDisableDepthMask();
        BeginBlendMode(BLEND_ALPHA);
    }

    DrawMesh(mesh, material, transform);

    if (blended) {
        EndBlendMode();
        rlEnableDepthMask();
    }
    if (doubleSided) rlEnableBackfaceCulling();
}

// Draw every visible mesh node using its geometry-to-world transform.
void RfbxDrawModelMatrix(const RfbxModel* model, Matrix transform, Color tint)
{
    if (!RfbxIsModelValid(model) || !model->defaultMaterialReady) return;

    // Opaque/cutout primitives write depth before blended primitives.
    for (int renderPass = 0; renderPass < 2; ++renderPass) {
        for (int nodeIndex = 0; nodeIndex < model->nodeCount; ++nodeIndex) {
            const RfbxNode* node = &model->nodes[nodeIndex];
            if (!node->visible ||
                node->meshIndex < 0 ||
                node->meshIndex >= model->meshCount) {
                continue;
            }

            const RfbxMesh* mesh = &model->meshes[node->meshIndex];

            // World-space skinning already contains the node transform.
            Matrix finalTransform = transform;
            if (!mesh->hasSkinning || mesh->verticesAreLocal) {

                //TODO : スキニングなどの場合はまだ考慮していないので乗算用Vector3の各スカラーの使用には気を付ける

                // raymath applies the node transform first, then the caller transform.
                finalTransform = MatrixMultiply(node->geometryToWorld, transform);
            }

            for (int primitiveIndex = 0;
                primitiveIndex < mesh->primitiveCount;
                ++primitiveIndex) {
                const RfbxPrimitive* primitive = &mesh->primitives[primitiveIndex];
                int materialIndex = rfbxResolveMaterialIndex(
                    model,
                    node,
                    mesh,
                    primitive);
                bool blended = rfbxIsMaterialBlended(model, materialIndex);
                if (blended != (renderPass == 1)) continue;

                MaterialMap maps[RFBX_DRAW_MATERIAL_MAP_COUNT] = { 0 };
                Material material = rfbxPrepareDrawMaterial(
                    model,
                    materialIndex,
                    tint,
                    maps);
                if (mesh->gpuSkinningReady) {
                    if (!model->internal || !model->internal->gpuMeshes) continue;
                    const RfbxGpuMeshState* state =
                        &model->internal->gpuMeshes[node->meshIndex];
                    if (!rfbxPrepareShaderDraw(
                        model,
                        &material,
                        state->boneMatrices,
                        state->boneCount,
                        true,
                        materialIndex)) {
                        continue;
                    }
                }
                else if (!rfbxPrepareShaderDraw(
                    model,
                    &material,
                    NULL,
                    0,
                    false,
                    materialIndex)) {
                    continue;
                }
                rfbxDrawMeshWithMaterialState(
                    model,
                    materialIndex,
                    primitive->mesh,
                    material,
                    finalTransform);
            }
        }
    }
}

// Build a raylib-style root transform and draw the model.
void RfbxDrawModel(
    const RfbxModel* model,
    Vector3 position,
    float scale,
    Color tint)
{
    Matrix transform = MatrixScale(scale, scale, scale);
    transform.m12 = position.x;
    transform.m13 = position.y;
    transform.m14 = position.z;
    RfbxDrawModelMatrix(model, transform, tint);
}

// Unload unique meshes once, then release scene and hierarchy data.
void RfbxUnloadModel(RfbxModel* model)
{
    if (!model) return;

    if (model->meshes) {
        for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
            RfbxMesh* mesh = &model->meshes[meshIndex];

            if (mesh->primitives) {
                for (int primitiveIndex = 0;
                    primitiveIndex < mesh->primitiveCount;
                    ++primitiveIndex) {
                    rfbxUnloadPrimitive(&mesh->primitives[primitiveIndex]);
                }
                MemFree(mesh->primitives);
            }

            MemFree(mesh->materialIndices);
            MemFree(mesh->name);
        }
        MemFree(model->meshes);
    }

    if (model->nodes) {
        for (int nodeIndex = 0; nodeIndex < model->nodeCount; ++nodeIndex) {
            MemFree(model->nodes[nodeIndex].materialIndices);
            MemFree(model->nodes[nodeIndex].name);
        }
        MemFree(model->nodes);
    }

    if (model->materials) {
        for (int materialIndex = 0;
            materialIndex < model->materialCount;
            ++materialIndex) {
            RfbxMaterial* material = &model->materials[materialIndex];
            if (material->materialReady) UnloadMaterial(material->material);
            MemFree(material->name);
        }
        MemFree(model->materials);
    }

    if (model->textures) {
        for (int textureIndex = 0;
            textureIndex < model->textureCount;
            ++textureIndex) {
            RfbxTexture* texture = &model->textures[textureIndex];
            if (texture->ready) UnloadTexture(texture->texture);
            MemFree(texture->fileName);
            MemFree(texture->name);
        }
        MemFree(model->textures);
    }

    if (model->animations) {
        for (int animationIndex = 0;
            animationIndex < model->animationCount;
            ++animationIndex) {
            MemFree(model->animations[animationIndex].name);
        }
        MemFree(model->animations);
    }

    if (model->sockets) {
        for (int socketIndex = 0;
            socketIndex < model->socketCount;
            ++socketIndex) {
            MemFree(model->sockets[socketIndex].name);
        }
        MemFree(model->sockets);
    }

    rfbxUnloadSkeleton(&model->skeleton);

    if (model->defaultMaterialReady) {
        UnloadMaterial(model->defaultMaterial);
    }

    if (model->internal) {
        if (model->internal->gpuMeshes) {
            for (int meshIndex = 0; meshIndex < model->meshCount; ++meshIndex) {
                MemFree(model->internal->gpuMeshes[meshIndex].boneMatrices);
                MemFree(model->internal->gpuMeshes[meshIndex].boneLocalBounds);
                MemFree(model->internal->gpuMeshes[meshIndex].boneBoundsValid);
            }
            MemFree(model->internal->gpuMeshes);
        }
        if (model->internal->builtinShaderReady) {
            UnloadShader(model->internal->builtinShader);
        }
        if (model->internal->animationEventLists) {
            for (int animationIndex = 0;
                animationIndex < model->animationCount;
                ++animationIndex) {
                RfbxAnimationEventListInternal* list =
                    &model->internal->animationEventLists[animationIndex];
                for (int eventIndex = 0;
                    eventIndex < list->count;
                    ++eventIndex) {
                    MemFree((void*)list->events[eventIndex].name);
                }
                MemFree(list->events);
            }
            MemFree(model->internal->animationEventLists);
        }
        MemFree(model->internal->materialDefaults);
        if (model->internal->scene) ufbx_free_scene(model->internal->scene);
        MemFree(model->internal);
    }

    memset(model, 0, sizeof(*model));
}

void RfbxPrintModelSummary(const RfbxModel* model)
{
    if (model == NULL) {
        printf("RFBX: model is NULL\n");
        return;
    }

    printf("===== RFBX Model Summary =====\n");
    printf("------------------------------\n");
    printf("Nodes      : %d\n", model->nodeCount);
    printf("Meshes     : %d\n", model->meshCount);
    printf("Materials  : %d\n", model->materialCount);
    printf("Textures   : %d\n", model->textureCount);
    printf("Animations : %d\n", model->animationCount);
    printf("Bones      : %d\n", model->skeleton.boneCount);
    printf("Sockets    : %d\n", model->socketCount);
    printf("GPU Skinned: %d\n", RfbxGetGpuSkinnedMeshCount(model));

}

void RfbxDrawNodes(const RfbxInstance* instance, Matrix transform, float axisLength)
{
    if (!RfbxIsInstanceValid(instance))
        return;

    if (axisLength <= 0.0f)
        axisLength = 0.05f;

    for (int i = 0; i < instance->internal->nodeCount; i++)
    {
        Matrix nodeMatrix = instance->internal->nodes[i].nodeToWorld;

        // RfbxDrawInstanceMatrix() と同じ外部Transformを適用
        Matrix drawMatrix = MatrixMultiply(nodeMatrix, transform);

        Vector3 origin = {
            drawMatrix.m12,
            drawMatrix.m13,
            drawMatrix.m14
        };

        Vector3 x = Vector3Transform(
            (Vector3) {
            axisLength, 0.0f, 0.0f
        },
            drawMatrix);

        Vector3 y = Vector3Transform(
            (Vector3) {
            0.0f, axisLength, 0.0f
        },
            drawMatrix);

        Vector3 z = Vector3Transform(
            (Vector3) {
            0.0f, 0.0f, axisLength
        },
            drawMatrix);
        //X,Y,Z
        DrawLine3D(origin, x, RED);
        DrawLine3D(origin, y, GREEN);
        DrawLine3D(origin, z, BLUE);
    }

}

void RfbxDrawNodeHierarchy(const RfbxInstance* instance, Matrix transform)
{
    if (!RfbxIsInstanceValid(instance))
        return;


    for (int childIndex = 0; childIndex < instance->internal->nodeCount; childIndex++)
    {
        int parent = instance->model->nodes[childIndex].parentIndex;

        if (parent >= 0)
        {
            // 子の位置
            Matrix childMatrix = instance->internal->nodes[childIndex].nodeToWorld;
            // 親の位置
            Matrix parentMatrix = instance->internal->nodes[parent].nodeToWorld;

            Vector3 childPos = {
                childMatrix.m12,
                childMatrix.m13,
                childMatrix.m14
            };

            Vector3 parentPos = {
                parentMatrix.m12,
                parentMatrix.m13,
                parentMatrix.m14
            };

            childPos = Vector3Transform(childPos, transform);
            parentPos = Vector3Transform(parentPos, transform);

            //TODO : 引数の追加。colorの受け渡しをする
            DrawLine3D(parentPos, childPos, RED);
        }
    }
}

void RfbxDrawMeshBounds(const RfbxInstance* instance, Matrix transform, Color color)
{
    if (!RfbxIsInstanceValid(instance))
        return;

    for (int nodeIndex = 0; nodeIndex < instance->internal->nodeCount; nodeIndex++)
    {
        int meshIndex = instance->model->nodes[nodeIndex].meshIndex;

        if (meshIndex < 0)
            continue;

        // このNodeにはMeshが付いている
        const RfbxMesh* mesh = &instance->model->meshes[meshIndex];
        if (!mesh->boundsValid)
            continue;

        // Mesh Local -> Model space
        BoundingBox modelBounds =
            rfbxTransformBounds(
                mesh->localBounds,
                instance->internal->nodes[nodeIndex].geometryToWorld);

        // Model space -> Caller World space
        BoundingBox worldBounds =
            rfbxTransformBounds(
                modelBounds,
                transform);

        DrawBoundingBox(worldBounds, color);
    }

}

//TODO : 表示するものが多いから列挙型で対応予定
void RfbxDrawDebugInfo(const RfbxInstance* instance, int posX, int posY)
{
    if (!RfbxIsInstanceValid(instance)) return;
    if (!instance->model) return;

    const RfbxModel* model = instance->model;

    const int fontSize = 20;
    const int lineHeight = 22;

    int y = posY;

#define RFBX_DEBUG_TEXT(...)                                      \
    do                                                            \
    {                                                             \
        DrawText(TextFormat(__VA_ARGS__), posX, y, fontSize, WHITE); \
        y += lineHeight;                                          \
    } while (0)

    // ============================================================
    // Model
    // ============================================================

    RFBX_DEBUG_TEXT("=== RFBX MODEL ===");

    RFBX_DEBUG_TEXT(
        "Valid: %s",
        model->valid ? "true" : "false"
    );

    RFBX_DEBUG_TEXT("Nodes: %d", model->nodeCount);
    RFBX_DEBUG_TEXT("Meshes: %d", model->meshCount);
    RFBX_DEBUG_TEXT("Textures: %d", model->textureCount);
    RFBX_DEBUG_TEXT("Materials: %d", model->materialCount);

    RFBX_DEBUG_TEXT(
        "Bones: %d",
        RfbxGetBoneCount(model)
    );

    RFBX_DEBUG_TEXT(
        "Sockets: %d",
        RfbxGetSocketCount(model)
    );

    RFBX_DEBUG_TEXT(
        "GPU Skinned Meshes: %d",
        RfbxGetGpuSkinnedMeshCount(model)
    );

    y += lineHeight;


    // ============================================================
    // Animations
    // ============================================================

    RFBX_DEBUG_TEXT("=== ANIMATIONS ===");

    const int animationCount = RfbxGetAnimationCount(model);

    RFBX_DEBUG_TEXT(
        "Animation Count: %d",
        animationCount
    );

    for (int i = 0; i < animationCount; ++i)
    {
        const RfbxAnimation* animation =
            RfbxGetAnimation(model, i);

        if (!animation)
        {
            RFBX_DEBUG_TEXT(
                "[%d] <invalid animation>",
                i
            );

            continue;
        }

        RFBX_DEBUG_TEXT(
            "[%d] %s",
            i,
            animation->name
            ? animation->name
            : "(unnamed)"
        );

        RFBX_DEBUG_TEXT(
            "    Begin: %.3f",
            animation->timeBegin
        );

        RFBX_DEBUG_TEXT(
            "    End: %.3f",
            animation->timeEnd
        );

        RFBX_DEBUG_TEXT(
            "    Duration: %.3f sec",
            animation->duration
        );

        RFBX_DEBUG_TEXT(
            "    Source Stack: %u",
            (unsigned int)animation->sourceStackIndex
        );
    }

    y += lineHeight;


    // ============================================================
    // Current Animation
    // ============================================================

    RFBX_DEBUG_TEXT("=== CURRENT ANIMATION ===");

    const char* currentAnimationName =
        RfbxGetCurrentAnimationName(instance);

    RFBX_DEBUG_TEXT(
        "Name: %s",
        currentAnimationName
        ? currentAnimationName
        : "(none)"
    );

    RFBX_DEBUG_TEXT(
        "Index: %d",
        instance->currentAnimation
    );

    RFBX_DEBUG_TEXT(
        "Time: %.3f sec",
        RfbxGetAnimationTime(instance)
    );

    RFBX_DEBUG_TEXT(
        "Progress: %.1f%%",
        RfbxGetAnimationProgress(instance) * 100.0f
    );

    RFBX_DEBUG_TEXT(
        "Speed: %.2f",
        instance->playbackSpeed
    );

    RFBX_DEBUG_TEXT(
        "Loop: %s",
        instance->loop ? "true" : "false"
    );

    RFBX_DEBUG_TEXT(
        "Playing: %s",
        RfbxIsAnimationPlaying(instance)
        ? "true"
        : "false"
    );

    RFBX_DEBUG_TEXT(
        "Paused: %s",
        RfbxIsAnimationPaused(instance)
        ? "true"
        : "false"
    );

    RFBX_DEBUG_TEXT(
        "Fading: %s",
        RfbxIsAnimationFading(instance)
        ? "true"
        : "false"
    );

    RFBX_DEBUG_TEXT(
        "Finished: %s",
        RfbxIsAnimationFinished(instance)
        ? "true"
        : "false"
    );

    y += lineHeight;


    // ============================================================
    // Cross Fade
    // ============================================================

    RFBX_DEBUG_TEXT("=== CROSS FADE ===");

    RFBX_DEBUG_TEXT(
        "Next Animation Index: %d",
        instance->nextAnimation
    );

    RFBX_DEBUG_TEXT(
        "Next Time: %.3f",
        instance->nextTime
    );

    RFBX_DEBUG_TEXT(
        "Fade Duration: %.3f",
        instance->fadeDuration
    );

    RFBX_DEBUG_TEXT(
        "Fade Time: %.3f",
        instance->fadeTime
    );

    if (instance->fadeDuration > 0.0)
    {
        float fadeProgress =
            (float)(instance->fadeTime /
                instance->fadeDuration);

        if (fadeProgress < 0.0f)
            fadeProgress = 0.0f;

        if (fadeProgress > 1.0f)
            fadeProgress = 1.0f;

        RFBX_DEBUG_TEXT(
            "Fade Progress: %.1f%%",
            fadeProgress * 100.0f
        );
    }

    y += lineHeight;


    // ============================================================
    // Queue
    // ============================================================

    RFBX_DEBUG_TEXT("=== QUEUE ===");

    const bool hasQueuedAnimation =
        RfbxHasQueuedAnimation(instance);

    RFBX_DEBUG_TEXT(
        "Queued: %s",
        hasQueuedAnimation
        ? "true"
        : "false"
    );

    if (hasQueuedAnimation)
    {
        const char* queuedName =
            RfbxGetQueuedAnimationName(instance);

        RFBX_DEBUG_TEXT(
            "Name: %s",
            queuedName
            ? queuedName
            : "(unnamed)"
        );

        RFBX_DEBUG_TEXT(
            "Index: %d",
            instance->queuedAnimation
        );

        RFBX_DEBUG_TEXT(
            "Fade Duration: %.3f",
            instance->queuedFadeDuration
        );

        RFBX_DEBUG_TEXT(
            "Loop: %s",
            instance->queuedLoop
            ? "true"
            : "false"
        );
    }

    y += lineHeight;


    // ============================================================
    // Animation Layer
    // ============================================================

    RFBX_DEBUG_TEXT("=== ANIMATION LAYER ===");

    const bool layerActive =
        RfbxIsAnimationLayerActive(instance);

    RFBX_DEBUG_TEXT(
        "Active: %s",
        layerActive
        ? "true"
        : "false"
    );

    if (layerActive)
    {
        const char* layerName =
            RfbxGetAnimationLayerName(instance);

        const char* rootBoneName =
            RfbxGetAnimationLayerRootBoneName(instance);

        RFBX_DEBUG_TEXT(
            "Name: %s",
            layerName
            ? layerName
            : "(unnamed)"
        );

        RFBX_DEBUG_TEXT(
            "Index: %d",
            instance->layerAnimation
        );

        RFBX_DEBUG_TEXT(
            "Root Bone: %s",
            rootBoneName
            ? rootBoneName
            : "(none)"
        );

        RFBX_DEBUG_TEXT(
            "Time: %.3f",
            RfbxGetAnimationLayerTime(instance)
        );

        RFBX_DEBUG_TEXT(
            "Weight: %.3f",
            RfbxGetAnimationLayerWeight(instance)
        );

        RFBX_DEBUG_TEXT(
            "Speed: %.3f",
            instance->layerPlaybackSpeed
        );

        RFBX_DEBUG_TEXT(
            "Playing: %s",
            RfbxIsAnimationLayerPlaying(instance)
            ? "true"
            : "false"
        );

        RFBX_DEBUG_TEXT(
            "Loop: %s",
            instance->layerLoop
            ? "true"
            : "false"
        );
    }

    y += lineHeight;


    // ============================================================
    // Animation Events
    // ============================================================

    RFBX_DEBUG_TEXT("=== EVENTS ===");

    RFBX_DEBUG_TEXT(
        "Pending Events: %d",
        RfbxGetPendingAnimationEventCount(instance)
    );


    // ============================================================
    // Root Motion
    // ============================================================

    y += lineHeight;

    RFBX_DEBUG_TEXT("=== ROOT MOTION ===");

    const bool rootMotionEnabled =
        RfbxIsRootMotionEnabled(instance);

    RFBX_DEBUG_TEXT(
        "Enabled: %s",
        rootMotionEnabled
        ? "true"
        : "false"
    );

    if (rootMotionEnabled)
    {
        const char* rootMotionNode =
            RfbxGetRootMotionNodeName(instance);

        RFBX_DEBUG_TEXT(
            "Node: %s",
            rootMotionNode
            ? rootMotionNode
            : "(none)"
        );

        RFBX_DEBUG_TEXT(
            "In Place: %s",
            RfbxIsRootMotionInPlace(instance)
            ? "true"
            : "false"
        );

        RfbxRootMotion motion =
            RfbxPeekRootMotion(instance);

        RFBX_DEBUG_TEXT(
            "Translation: %.3f %.3f %.3f",
            motion.translation.x,
            motion.translation.y,
            motion.translation.z
        );

        RFBX_DEBUG_TEXT(
            "Rotation: %.3f %.3f %.3f %.3f",
            motion.rotation.x,
            motion.rotation.y,
            motion.rotation.z,
            motion.rotation.w
        );
    }

#undef RFBX_DEBUG_TEXT
}

//TODO : 列挙型のbit flagで分岐別出力予定
void RfbxPrintDebugInfo(const RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance))
    {
        printf("RFBX Debug Info: invalid instance\n");
        return;
    }

    if (!instance->model)
    {
        printf("RFBX Debug Info: model is NULL\n");
        return;
    }

    const RfbxModel* model = instance->model;

    // ============================================================
    // Model
    // ============================================================

    printf("========================================\n");
    printf("             RFBX DEBUG INFO            \n");
    printf("========================================\n\n");

    printf("=== MODEL ===\n");

    printf("Valid: %s\n",
        model->valid ? "true" : "false");

    printf("Nodes: %d\n",
        model->nodeCount);

    printf("Meshes: %d\n",
        model->meshCount);

    printf("Textures: %d\n",
        model->textureCount);

    printf("Materials: %d\n",
        model->materialCount);

    printf("Animations: %d\n",
        model->animationCount);

    printf("Bones: %d\n",
        RfbxGetBoneCount(model));

    printf("Sockets: %d\n",
        RfbxGetSocketCount(model));

    printf("GPU Skinned Meshes: %d\n",
        RfbxGetGpuSkinnedMeshCount(model));

    printf("\n");


    // ============================================================
    // Animations
    // ============================================================

    printf("=== ANIMATIONS ===\n");

    const int animationCount =
        RfbxGetAnimationCount(model);

    printf("Animation Count: %d\n",
        animationCount);

    for (int i = 0; i < animationCount; ++i)
    {
        const RfbxAnimation* animation =
            RfbxGetAnimation(model, i);

        if (!animation)
        {
            printf("[%d] <invalid animation>\n", i);
            continue;
        }

        printf("[%d] %s\n",
            i,
            animation->name
            ? animation->name
            : "(unnamed)");

        printf("    Begin: %.3f\n",
            animation->timeBegin);

        printf("    End: %.3f\n",
            animation->timeEnd);

        printf("    Duration: %.3f sec\n",
            animation->duration);

        printf("    Source Stack: %u\n",
            (unsigned int)animation->sourceStackIndex);
    }

    printf("\n");


    // ============================================================
    // Current Animation
    // ============================================================

    printf("=== CURRENT ANIMATION ===\n");

    const char* currentAnimationName =
        RfbxGetCurrentAnimationName(instance);

    printf("Name: %s\n",
        currentAnimationName
        ? currentAnimationName
        : "(none)");

    printf("Index: %d\n",
        instance->currentAnimation);

    printf("Time: %.3f sec\n",
        RfbxGetAnimationTime(instance));

    printf("Progress: %.1f%%\n",
        RfbxGetAnimationProgress(instance) * 100.0f);

    printf("Speed: %.3f\n",
        instance->playbackSpeed);

    printf("Loop: %s\n",
        instance->loop ? "true" : "false");

    printf("Playing: %s\n",
        RfbxIsAnimationPlaying(instance)
        ? "true"
        : "false");

    printf("Paused: %s\n",
        RfbxIsAnimationPaused(instance)
        ? "true"
        : "false");

    printf("Finished: %s\n",
        RfbxIsAnimationFinished(instance)
        ? "true"
        : "false");

    printf("\n");


    // ============================================================
    // Cross Fade
    // ============================================================

    printf("=== CROSS FADE ===\n");

    printf("Fading: %s\n",
        RfbxIsAnimationFading(instance)
        ? "true"
        : "false");

    printf("Next Animation Index: %d\n",
        instance->nextAnimation);

    if (instance->nextAnimation != RFBX_INVALID_INDEX)
    {
        const RfbxAnimation* nextAnimation =
            RfbxGetAnimation(
                model,
                instance->nextAnimation);

        printf("Next Animation Name: %s\n",
            nextAnimation && nextAnimation->name
            ? nextAnimation->name
            : "(none)");
    }

    printf("Next Time: %.3f sec\n",
        instance->nextTime);

    printf("Fade Duration: %.3f sec\n",
        instance->fadeDuration);

    printf("Fade Time: %.3f sec\n",
        instance->fadeTime);

    if (instance->fadeDuration > 0.0)
    {
        double progress =
            instance->fadeTime /
            instance->fadeDuration;

        if (progress < 0.0)
            progress = 0.0;

        if (progress > 1.0)
            progress = 1.0;

        printf(
            "Fade Progress: %.1f%%\n",
            progress * 100.0);
    }

    printf("\n");


    // ============================================================
    // Queue
    // ============================================================

    printf("=== QUEUE ===\n");

    const bool hasQueuedAnimation =
        RfbxHasQueuedAnimation(instance);

    printf("Queued: %s\n",
        hasQueuedAnimation
        ? "true"
        : "false");

    if (hasQueuedAnimation)
    {
        const char* queuedName =
            RfbxGetQueuedAnimationName(instance);

        printf("Name: %s\n",
            queuedName
            ? queuedName
            : "(unnamed)");

        printf("Index: %d\n",
            instance->queuedAnimation);

        printf("Fade Duration: %.3f sec\n",
            instance->queuedFadeDuration);

        printf("Loop: %s\n",
            instance->queuedLoop
            ? "true"
            : "false");
    }

    printf("\n");


    // ============================================================
    // Animation Layer
    // ============================================================

    printf("=== ANIMATION LAYER ===\n");

    const bool layerActive =
        RfbxIsAnimationLayerActive(instance);

    printf("Active: %s\n",
        layerActive
        ? "true"
        : "false");

    if (layerActive)
    {
        const char* layerName =
            RfbxGetAnimationLayerName(instance);

        const char* rootBoneName =
            RfbxGetAnimationLayerRootBoneName(instance);

        printf("Name: %s\n",
            layerName
            ? layerName
            : "(unnamed)");

        printf("Index: %d\n",
            instance->layerAnimation);

        printf("Root Bone: %s\n",
            rootBoneName
            ? rootBoneName
            : "(none)");

        printf("Time: %.3f sec\n",
            RfbxGetAnimationLayerTime(instance));

        printf("Weight: %.3f\n",
            RfbxGetAnimationLayerWeight(instance));

        printf("Speed: %.3f\n",
            instance->layerPlaybackSpeed);

        printf("Playing: %s\n",
            RfbxIsAnimationLayerPlaying(instance)
            ? "true"
            : "false");

        printf("Loop: %s\n",
            instance->layerLoop
            ? "true"
            : "false");
    }

    printf("\n");


    // ============================================================
    // Events
    // ============================================================

    printf("=== EVENTS ===\n");

    printf("Pending Events: %d\n",
        RfbxGetPendingAnimationEventCount(instance));

    printf("\n");


    // ============================================================
    // Root Motion
    // ============================================================

    printf("=== ROOT MOTION ===\n");

    const bool rootMotionEnabled =
        RfbxIsRootMotionEnabled(instance);

    printf("Enabled: %s\n",
        rootMotionEnabled
        ? "true"
        : "false");

    if (rootMotionEnabled)
    {
        const char* nodeName =
            RfbxGetRootMotionNodeName(instance);

        printf("Node: %s\n",
            nodeName
            ? nodeName
            : "(none)");

        printf("In Place: %s\n",
            RfbxIsRootMotionInPlace(instance)
            ? "true"
            : "false");

        RfbxRootMotion motion =
            RfbxPeekRootMotion(instance);

        printf(
            "Translation: %.3f %.3f %.3f\n",
            motion.translation.x,
            motion.translation.y,
            motion.translation.z);

        printf(
            "Rotation: %.3f %.3f %.3f %.3f\n",
            motion.rotation.x,
            motion.rotation.y,
            motion.rotation.z,
            motion.rotation.w);
    }

    printf("\n");


    // ============================================================
    // Skeleton
    // ============================================================

    printf("=== SKELETON ===\n");

    const int boneCount =
        RfbxGetBoneCount(model);

    printf("Bone Count: %d\n",
        boneCount);

    for (int i = 0; i < boneCount; ++i)
    {
        const RfbxBone* bone =
            RfbxGetBone(model, i);

        if (!bone)
            continue;

        printf("[%d] %s\n",
            i,
            bone->name
            ? bone->name
            : "(unnamed)");

        printf("    Parent: %d\n",
            bone->parentIndex);

        printf("    Skin Bone: %s\n",
            bone->isSkinBone
            ? "true"
            : "false");

        printf("    Radius: %.3f\n",
            bone->radius);

        printf("    Source Node ID: %u\n",
            (unsigned int)bone->sourceNodeTypedId);
    }

    printf("\n");


    // ============================================================
    // Sockets
    // ============================================================

    printf("=== SOCKETS ===\n");

    const int socketCount =
        RfbxGetSocketCount(model);

    printf("Socket Count: %d\n",
        socketCount);

    for (int i = 0; i < socketCount; ++i)
    {
        const RfbxSocket* socket =
            RfbxGetSocket(model, i);

        if (!socket)
            continue;

        printf("[%d] %s\n",
            i,
            socket->name
            ? socket->name
            : "(unnamed)");

        printf("    Node Index: %d\n",
            socket->nodeIndex);

        printf("    Bone Index: %d\n",
            socket->boneIndex);
    }

    printf("\n");


    // ============================================================
    // Meshes
    // ============================================================

    printf("=== MESHES ===\n");

    printf("Mesh Count: %d\n",
        model->meshCount);

    for (int i = 0; i < model->meshCount; ++i)
    {
        const RfbxMesh* mesh =
            &model->meshes[i];

        printf("[%d] %s\n",
            i,
            mesh->name
            ? mesh->name
            : "(unnamed)");

        printf("    Source ID: %u\n",
            (unsigned int)mesh->sourceTypedId);

        printf("    Primitives: %d\n",
            mesh->primitiveCount);

        printf("    Materials: %d\n",
            mesh->materialCount);

        printf("    Has Skinning: %s\n",
            mesh->hasSkinning
            ? "true"
            : "false");

        printf("    GPU Skinning Ready: %s\n",
            mesh->gpuSkinningReady
            ? "true"
            : "false");

        printf("    GPU Bone Count: %d\n",
            mesh->gpuBoneCount);

        printf("    Vertices Local: %s\n",
            mesh->verticesAreLocal
            ? "true"
            : "false");

        printf("    Bounds Valid: %s\n",
            mesh->boundsValid
            ? "true"
            : "false");
    }

    printf("\n");


    // ============================================================
    // Materials
    // ============================================================

    printf("=== MATERIALS ===\n");

    printf("Material Count: %d\n",
        model->materialCount);

    for (int i = 0; i < model->materialCount; ++i)
    {
        const RfbxMaterial* material =
            RfbxGetMaterial(model, i);

        if (!material)
            continue;

        printf("[%d] %s\n",
            i,
            material->name
            ? material->name
            : "(unnamed)");

        printf("    Source ID: %u\n",
            (unsigned int)material->sourceTypedId);

        printf("    Metallic: %.3f\n",
            material->metallic);

        printf("    Roughness: %.3f\n",
            material->roughness);

        printf("    Opacity: %.3f\n",
            material->opacity);

        printf("    Alpha Cutoff: %.3f\n",
            material->alphaCutoff);

        printf("    Double Sided: %s\n",
            material->doubleSided
            ? "true"
            : "false");

        printf("    Material Ready: %s\n",
            material->materialReady
            ? "true"
            : "false");

        printf("    Base Color Texture: %d\n",
            material->baseColorTextureIndex);

        printf("    Normal Texture: %d\n",
            material->normalTextureIndex);

        printf("    Metallic Texture: %d\n",
            material->metallicTextureIndex);

        printf("    Roughness Texture: %d\n",
            material->roughnessTextureIndex);

        printf("    Emission Texture: %d\n",
            material->emissiveTextureIndex);

        printf("    Opacity Texture: %d\n",
            material->opacityTextureIndex);
    }

    printf("\n");


    // ============================================================
    // Textures
    // ============================================================

    printf("=== TEXTURES ===\n");

    printf("Texture Count: %d\n",
        model->textureCount);

    for (int i = 0; i < model->textureCount; ++i)
    {
        const RfbxTexture* texture =
            RfbxGetTexture(model, i);

        if (!texture)
            continue;

        printf("[%d] %s\n",
            i,
            texture->name
            ? texture->name
            : "(unnamed)");

        printf("    File: %s\n",
            texture->fileName
            ? texture->fileName
            : "(embedded / none)");

        printf("    Source ID: %u\n",
            (unsigned int)texture->sourceTypedId);

        printf("    Ready: %s\n",
            texture->ready
            ? "true"
            : "false");

        if (texture->ready)
        {
            printf("    Size: %d x %d\n",
                texture->texture.width,
                texture->texture.height);

            printf("    Texture ID: %u\n",
                texture->texture.id);
        }
    }

    printf("\n");


    // ============================================================
    // Nodes
    // ============================================================

    printf("=== NODES ===\n");

    printf("Node Count: %d\n",
        model->nodeCount);

    for (int i = 0; i < model->nodeCount; ++i)
    {
        const RfbxNode* node =
            &model->nodes[i];

        printf("[%d] %s\n",
            i,
            node->name
            ? node->name
            : "(unnamed)");

        printf("    Source ID: %u\n",
            (unsigned int)node->sourceTypedId);

        printf("    Parent Index: %d\n",
            node->parentIndex);

        printf("    Mesh Index: %d\n",
            node->meshIndex);

        printf("    Material Count: %d\n",
            node->materialCount);

        printf("    Visible: %s\n",
            node->visible
            ? "true"
            : "false");
    }

    printf("\n");

    printf("========================================\n");
    printf("          END RFBX DEBUG INFO           \n");
    printf("========================================\n");
}

void RfbxSetAnimationLoop(RfbxInstance* instance, bool loop)
{
    if (!RfbxIsInstanceValid(instance))
    {
        TraceLog(LOG_WARNING, "instance is not valid!");
        return;
    }
    //CrossFadeを考慮
    if (instance->fading)
    {
        // CrossFade中は遷移先AnimationのLoop設定を変更する
        instance->nextLoop = loop;
    }
    else
    {
        // 通常再生中は現在AnimationのLoop設定を変更する
        instance->loop = loop;
    }
}

bool RfbxIsAnimationLooping(RfbxInstance* instance)
{
    if (!RfbxIsInstanceValid(instance))
    {
        return false;
    }

    //CrossFadingを考慮する場合に必要
    if (instance->fading) {
        return instance->nextLoop;
    }

    return instance->loop;
}

int RfbxGetCurrentAnimationIndex(const RfbxInstance* instance)
{
    return rfbxGetActiveAnimationIndex(instance);
}

bool RfbxSetNodeScaleMultiplier(RfbxInstance* instance, int nodeIndex, Vector3 multiplier)
{
    if (!RfbxIsInstanceValid(instance)) {
        return false;
    }

    //制限/既定値よりも大きい場合はなにもしない
    if (nodeIndex < 0 || nodeIndex >= instance->model->nodeCount)
        return false;

    //各NodeState内のmultiplier(乗算数値)が有限値/または0.0以下なら処理を停止する
    if (!isfinite(multiplier.x) ||
        !isfinite(multiplier.y) ||
        !isfinite(multiplier.z) ||
        multiplier.x <= 0.0f ||
        multiplier.y <= 0.0f ||
        multiplier.z <= 0.0f)
        return false;

    instance->internal->nodes[nodeIndex].scaleMultiplier = multiplier;

    //設定した分の各成分スカラーを再計算をするために置いている
    rfbxUpdateInstanceBounds(instance);

    return true;

}

bool RfbxSetNodeOffset(RfbxInstance* instance, int nodeIndex, Vector3 offset) //NOTE : a node of offset.
{
    if (!RfbxIsInstanceValid(instance)) {
        return false;
    }

    //制限/既定値よりも大きい場合はなにもしない
    if (nodeIndex < 0 || nodeIndex >= instance->model->nodeCount)
        return false;

    // オフセットは0や負数も許可する。非有限値だけを拒否する。
    if (!isfinite(offset.x) ||
        !isfinite(offset.y) ||
        !isfinite(offset.z))
        return false;

    instance->internal->nodes[nodeIndex].offset = offset;

    //設定した分の各成分スカラーを再計算をするために置いている
    rfbxUpdateInstanceBounds(instance);

    return true;

}



#endif // RFBX_IMPLEMENTATION

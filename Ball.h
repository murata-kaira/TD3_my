#pragma once
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

/**
 * @brief ボールクラス
 *
 * プレイヤーがバットでスイングして打つことができるボール。
 * 打たれると指定した方向に飛び、摩擦で徐々に止まる。
 */
class Ball {
public:
	/// @brief 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief 当たり判定（AABB）を取得
	AABB GetAABB();

	/// @brief ワールド座標を取得
	Vector3 GetWorldPosition();

	/// @brief ボールを打つ（velocityを与えて飛ばす）
	void Hit(const Vector3& velocity);

	/// @brief ボールが移動中かどうか
	bool IsMoving() const { return isMoving_; }

private:
	// --- コンポーネント ---
	WorldTransform worldTransform_; // ワールド座標データ
	Model* model_ = nullptr;       // ボールのモデル
	Camera* camera_ = nullptr;     // カメラへの参照

	// --- 物理 ---
	Vector3 velocity_ = {};  // 現在の速度
	bool isMoving_ = false;  // 移動中フラグ

	// --- 定数 ---
	static inline const float kRadius = 0.25f;     // ボールの半径（判定・表示用）
	static inline const float kFriction = 0.03f;   // 1フレームあたりの速度減衰量
	static inline const float kStopSpeed = 0.005f; // 停止とみなす速度の閾値
};

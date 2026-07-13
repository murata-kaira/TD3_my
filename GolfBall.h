#pragma once
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

/**
 * @brief ゴルフボールクラス
 * ボールの物理挙動（移動・摩擦）を管理する
 */
class GolfBall {
public:
	/// @brief 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	/// @brief 更新（摩擦・位置計算）
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief ボールを打つ
	/// @param direction 正規化済み方向ベクトル
	/// @param speed     初速
	void Hit(const Vector3& direction, float speed);

	/// @brief ボールが動いているか
	bool IsMoving() const;

	// --- アクセッサ ---
	const Vector3& GetPosition() const { return position_; }
	Vector3&       GetVelocity()       { return velocity_; }

	/// @brief 位置を直接セット（衝突補正用）
	void SetPosition(const Vector3& pos);

	/// @brief ボールの半径（衝突判定に使用）
	static inline const float kRadius = 0.4f;

private:
	WorldTransform worldTransform_;
	Camera*        camera_ = nullptr;
	Model*         model_  = nullptr;

	Vector3 position_{};
	Vector3 velocity_{};

	/// @brief 1フレームあたりの摩擦係数
	static inline const float kFriction  = 0.985f;
	/// @brief この速度以下なら停止とみなす
	static inline const float kStopSpeed = 0.005f;
};

#include "GolfBall.h"
#include <cmath>

/**
 * @brief 初期化
 */
void GolfBall::Initialize(Model* model, Camera* camera, const Vector3& position) {
	model_    = model;
	camera_   = camera;
	position_ = position;
	velocity_ = {0.0f, 0.0f, 0.0f};

	worldTransform_.Initialize();
	worldTransform_.scale_       = {kRadius, kRadius, kRadius};
	worldTransform_.translation_ = position_;
	WorldTransformUpdate(worldTransform_);
}

/**
 * @brief 更新（摩擦・位置計算）
 */
void GolfBall::Update() {
	// 摩擦を適用して速度を減衰
	velocity_.x *= kFriction;
	velocity_.z *= kFriction;

	// 速度が閾値以下なら停止
	if (std::abs(velocity_.x) < kStopSpeed) velocity_.x = 0.0f;
	if (std::abs(velocity_.z) < kStopSpeed) velocity_.z = 0.0f;

	// 位置更新
	position_.x += velocity_.x;
	position_.z += velocity_.z;

	worldTransform_.translation_ = position_;
	WorldTransformUpdate(worldTransform_);
}

/**
 * @brief 描画
 */
void GolfBall::Draw() {
	model_->Draw(worldTransform_, *camera_);
}

/**
 * @brief ボールを打つ
 */
void GolfBall::Hit(const Vector3& direction, float speed) {
	velocity_.x = direction.x * speed;
	velocity_.z = direction.z * speed;
}

/**
 * @brief ボールが動いているか
 */
bool GolfBall::IsMoving() const {
	return (std::abs(velocity_.x) >= kStopSpeed || std::abs(velocity_.z) >= kStopSpeed);
}

/**
 * @brief 位置を直接セット（衝突補正用）
 */
void GolfBall::SetPosition(const Vector3& pos) {
	position_                    = pos;
	worldTransform_.translation_ = pos;
	WorldTransformUpdate(worldTransform_);
}

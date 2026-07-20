#include "Ball.h"
#include <cassert>
#include <cmath>

/**
 * @brief 初期化
 */
void Ball::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	// ボールを小さく表示する
	worldTransform_.scale_ = {kRadius, kRadius, kRadius};

	velocity_ = {};
	isMoving_ = false;

	WorldTransformUpdate(worldTransform_);
}

/**
 * @brief 更新
 *
 * 速度を座標に加算し、摩擦で速度を減衰させる。
 * 速度が閾値以下になったら停止する。
 */
void Ball::Update() {
	if (!isMoving_) return;

	// 座標を速度分だけ移動
	worldTransform_.translation_ += velocity_;

	// 摩擦で速度を減衰
	float speed = std::sqrtf(velocity_.x * velocity_.x + velocity_.z * velocity_.z);
	if (speed <= kStopSpeed) {
		velocity_ = {};
		isMoving_ = false;
	} else {
		float ratio = std::max(0.0f, speed - kFriction) / speed;
		velocity_.x *= ratio;
		velocity_.z *= ratio;
	}

	WorldTransformUpdate(worldTransform_);
}

/**
 * @brief 描画
 */
void Ball::Draw() {
	model_->Draw(worldTransform_, *camera_);
}

/**
 * @brief 当たり判定（AABB）を取得
 */
AABB Ball::GetAABB() {
	Vector3 pos = GetWorldPosition();
	return {
		{pos.x - kRadius, pos.y - kRadius, pos.z - kRadius},
		{pos.x + kRadius, pos.y + kRadius, pos.z + kRadius}
	};
}

/**
 * @brief ワールド座標を取得
 */
Vector3 Ball::GetWorldPosition() {
	return {
		worldTransform_.matWorld_.m[3][0],
		worldTransform_.matWorld_.m[3][1],
		worldTransform_.matWorld_.m[3][2]
	};
}

/**
 * @brief ボールを打つ
 * @param velocity 打ち出し速度ベクトル
 */
void Ball::Hit(const Vector3& velocity) {
	velocity_ = velocity;
	isMoving_ = true;
}

#include <algorithm>
#include <cmath>
#include "CameraController.h"
#include "Math.h"
#include "Player.h"


static Vector3 Lerp(const Vector3& start, const Vector3& end, float t) {
	return {
		start.x + (end.x - start.x) * t,
		start.y + (end.y - start.y) * t,
		start.z + (end.z - start.z) * t,
	};
}

void CameraController::Initialize(Camera* camera) { 
	camera_ = camera;
}

void CameraController::Update() {
	const WorldTransform& targetWT = target_->GetWorldTransform();
	const Vector3& playerPos = targetWT.translation_;
	float yaw = targetWT.rotation_.y;

	float cosY = std::cos(yaw);
	float sinY = std::sin(yaw);

	// ローカルオフセットをプレイヤーのY回転に合わせてワールド空間へ変換
	Vector3 worldOffset = {
		targetOffset_.x * cosY + targetOffset_.z * sinY,
		targetOffset_.y,
		-targetOffset_.x * sinY + targetOffset_.z * cosY,
	};

	Vector3 dest = {
		playerPos.x + worldOffset.x,
		playerPos.y + worldOffset.y,
		playerPos.z + worldOffset.z,
	};

	camera_->translation_ = Lerp(camera_->translation_, dest, kInterpolationRate);

	// カメラの向きをプレイヤー方向へ
	camera_->rotation_.y = yaw;
	camera_->rotation_.x = std::atan2(targetOffset_.y, -targetOffset_.z);

	camera_->UpdateMatrix();
}


void CameraController::Reset() { 
	const WorldTransform& targetWT = target_->GetWorldTransform();
	float yaw = targetWT.rotation_.y;
	float cosY = std::cos(yaw);
	float sinY = std::sin(yaw);

	Vector3 worldOffset = {
		targetOffset_.x * cosY + targetOffset_.z * sinY,
		targetOffset_.y,
		-targetOffset_.x * sinY + targetOffset_.z * cosY,
	};

	camera_->translation_ = {
		targetWT.translation_.x + worldOffset.x,
		targetWT.translation_.y + worldOffset.y,
		targetWT.translation_.z + worldOffset.z,
	};
	camera_->rotation_.y = yaw;
	camera_->rotation_.x = std::atan2(targetOffset_.y, -targetOffset_.z);
}

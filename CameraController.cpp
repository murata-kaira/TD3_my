#include <algorithm>
#include "CameraController.h"
#include "Math.h"
#include "Player.h"


void CameraController::Initialize(Camera* camera) { 
	camera_ = camera;
}

void CameraController::Update() {
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 一人称視点：プレイヤーの目の位置にカメラを配置
	camera_->translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;
	camera_->translation_.y = targetWorldTransform.translation_.y + targetOffset_.y;
	camera_->translation_.z = targetWorldTransform.translation_.z + targetOffset_.z;

	// プレイヤーの向いている方向にカメラを合わせる
	camera_->rotation_.y = targetWorldTransform.rotation_.y;

	camera_->UpdateMatrix();
}


void CameraController::Reset() { 
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	camera_->translation_ = {
	    targetWorldTransform.translation_.x + targetOffset_.x,
	    targetWorldTransform.translation_.y + targetOffset_.y,
	    targetWorldTransform.translation_.z + targetOffset_.z};
	camera_->rotation_.y = targetWorldTransform.rotation_.y;
}

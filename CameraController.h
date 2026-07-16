#pragma once
#include "KamataEngine.h"

// 前方宣言
class Player;
using namespace KamataEngine;

class CameraController {
public:
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	void Initialize(Camera* camera);
	void Update();
	void SetTarget(Player* target) { target_ = target; }
	void Reset();
	void SetMovableArea(Rect area) { movableArea_ = area; }

private:
	Camera* camera_ = nullptr;
	Player* target_ = nullptr;
	Vector3 targetOffset_ = {0, 5.0f, -10.0f}; // プレイヤーローカル空間でのオフセット（上5・後方10）

	Rect movableArea_ = {0, 100, 0, 100}; // 使用しないが互換のため保持

	static inline const float kInterpolationRate = 0.1f;
};

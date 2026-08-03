#pragma once
#include "KamataEngine.h"

// 前方宣言
class Player;
using namespace KamataEngine;

class CameraController {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Camera* camera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	void SetTarget(Player* target) { target_ = target; }

	void Reset();

private:
	Camera* camera_ = nullptr;
	Player* target_ = nullptr;
	// プレイヤー原点から目線位置へのオフセット（一人称視点用）
	Vector3 targetOffset_ = {0, 0.4f, 0};
};

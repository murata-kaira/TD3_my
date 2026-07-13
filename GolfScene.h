#pragma once
#include "KamataEngine.h"
#include "GolfBall.h"
#include "Skydome.h"
#include "Fade.h"
#include "Math.h"
#include <vector>

using namespace KamataEngine;

/**
 * @brief ゴルフゲームシーン
 *
 * 操作方法:
 *   左右矢印キー … 打つ方向を変える
 *   SPACEキー長押し … パワーをチャージ
 *   SPACEキー離す … ショット
 */
class GolfScene {
public:
	~GolfScene();

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

	/// @brief 描画
	void Draw();

	/// @brief シーン終了フラグを取得
	bool IsFinished() const { return finished_; }

private:
	// --- フェーズ ---
	enum class Phase {
		kFadeIn,     // フェードイン
		kAiming,     // 照準・パワーチャージ中
		kBallMoving, // ボール転がり中
		kGoal,       // ホールイン！
		kFadeOut,    // フェードアウト
	};
	Phase phase_ = Phase::kFadeIn;

	// --- カメラ（真上からの俯瞰視点）---
	Camera camera_;

	// --- モデル ---
	Model* ballModel_    = nullptr; // ボール・照準インジケーター用
	Model* blockModel_   = nullptr; // 壁・床・障害物用
	Model* skydomeModel_ = nullptr; // スカイドーム

	// --- ゲームオブジェクト ---
	GolfBall* ball_    = nullptr;
	Skydome*  skydome_ = nullptr;

	// --- コース要素のワールドトランスフォーム ---
	WorldTransform worldTransformFloor_;
	WorldTransform worldTransformWallN_;
	WorldTransform worldTransformWallS_;
	WorldTransform worldTransformWallE_;
	WorldTransform worldTransformWallW_;
	WorldTransform worldTransformHole_;

	// --- 障害物 ---
	struct Obstacle {
		WorldTransform* wt = nullptr;
		float cx = 0.0f, cz = 0.0f; // 中心座標
		float hw = 0.0f, hd = 0.0f; // 衝突判定の半サイズ（X / Z）
	};
	std::vector<Obstacle> obstacles_;

	// --- 照準インジケーター ---
	WorldTransform worldTransformArrow_;

	// --- エイム・パワー ---
	float aimAngle_   = 0.0f;
	float power_      = 0.0f;
	bool  isCharging_ = false;

	// --- コース定数 ---
	static inline const float kCourseMinX  = 0.5f;
	static inline const float kCourseMaxX  = 17.5f;
	static inline const float kCourseMinZ  = 0.5f;
	static inline const float kCourseMaxZ  = 17.5f;
	static inline const float kHoleRadius  = 0.7f;  // ホールに入る判定半径
	static inline const float kBounceDecay = 0.65f; // 壁・障害物バウンス時の速度減衰率
	static inline const float kHolePosX    = 14.0f; // ホールの X 座標
	static inline const float kHolePosZ    = 14.0f; // ホールの Z 座標

	// --- スコア ---
	int   strokeCount_ = 0;
	float goalTimer_   = 0.0f; // ゴール後の待機タイマー

	// --- システム ---
	Fade* fade_      = nullptr;
	bool  finished_  = false;

	// --- 内部処理 ---

	/// @brief コース各要素の初期化
	void InitializeCourseElements();

	/// @brief 照準フェーズの入力・更新
	void UpdateAiming();

	/// @brief ボールとコース境界・障害物との衝突処理
	void CheckBallCollision();
};

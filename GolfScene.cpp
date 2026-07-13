#define NOMINMAX

#include "GolfScene.h"
#include "Math.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using namespace KamataEngine;

/**
 * @brief デストラクタ
 */
GolfScene::~GolfScene() {
	delete ball_;
	delete skydome_;
	delete fade_;
	delete ballModel_;
	delete blockModel_;
	delete skydomeModel_;

	for (auto& obs : obstacles_) {
		delete obs.wt;
	}
	obstacles_.clear();
}

/**
 * @brief 初期化
 */
void GolfScene::Initialize() {
	// --- カメラ（俯瞰視点）---
	camera_.Initialize();
	camera_.translation_ = {9.0f, 28.0f, -6.0f};
	camera_.rotation_    = {1.3f, 0.0f, 0.0f};

	// --- フェード ---
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// --- モデルロード ---
	ballModel_    = Model::Create();
	blockModel_   = Model::CreateFromOBJ("block");
	skydomeModel_ = Model::CreateFromOBJ("skydome", true);

	// --- スカイドーム ---
	skydome_ = new Skydome();
	skydome_->Initialize(skydomeModel_, &camera_);

	// --- コース要素 ---
	InitializeCourseElements();

	// --- ゴルフボール（スタート位置）---
	ball_ = new GolfBall();
	ball_->Initialize(ballModel_, &camera_, {2.0f, 0.2f, 2.0f});

	// --- 照準インジケーター ---
	worldTransformArrow_.Initialize();
	worldTransformArrow_.scale_       = {0.25f, 0.25f, 0.35f};
	worldTransformArrow_.translation_ = ball_->GetPosition();
	WorldTransformUpdate(worldTransformArrow_);

	phase_ = Phase::kFadeIn;
}

/**
 * @brief コース各要素（床・壁・障害物・ホール）の初期化
 */
void GolfScene::InitializeCourseElements() {
	// コース全体の床（緑のフェアウェイを1つの平たいブロックで表現）
	worldTransformFloor_.Initialize();
	worldTransformFloor_.scale_       = {18.0f, 0.1f, 18.0f};
	worldTransformFloor_.translation_ = {9.0f, -0.05f, 9.0f};
	WorldTransformUpdate(worldTransformFloor_);

	// 北壁 (z+ 側)
	worldTransformWallN_.Initialize();
	worldTransformWallN_.scale_       = {22.0f, 2.0f, 2.0f};
	worldTransformWallN_.translation_ = {9.0f, 1.0f, 18.5f};
	WorldTransformUpdate(worldTransformWallN_);

	// 南壁 (z- 側)
	worldTransformWallS_.Initialize();
	worldTransformWallS_.scale_       = {22.0f, 2.0f, 2.0f};
	worldTransformWallS_.translation_ = {9.0f, 1.0f, -0.5f};
	WorldTransformUpdate(worldTransformWallS_);

	// 東壁 (x+ 側)
	worldTransformWallE_.Initialize();
	worldTransformWallE_.scale_       = {2.0f, 2.0f, 18.0f};
	worldTransformWallE_.translation_ = {18.5f, 1.0f, 9.0f};
	WorldTransformUpdate(worldTransformWallE_);

	// 西壁 (x- 側)
	worldTransformWallW_.Initialize();
	worldTransformWallW_.scale_       = {2.0f, 2.0f, 18.0f};
	worldTransformWallW_.translation_ = {-0.5f, 1.0f, 9.0f};
	WorldTransformUpdate(worldTransformWallW_);

	// ホール（目標地点）
	worldTransformHole_.Initialize();
	worldTransformHole_.scale_       = {0.9f, 0.15f, 0.9f};
	worldTransformHole_.translation_ = {kHolePosX, 0.08f, kHolePosZ};
	WorldTransformUpdate(worldTransformHole_);

	// --- 障害物の追加 ---
	// ヘルパーラムダ: 障害物をリストに追加
	auto addObs = [this](float cx, float cz, float hw, float hd, float height) {
		Obstacle obs;
		obs.wt = new WorldTransform();
		obs.wt->Initialize();
		obs.wt->scale_       = {hw * 2.0f, height, hd * 2.0f};
		obs.wt->translation_ = {cx, height * 0.5f, cz};
		WorldTransformUpdate(*obs.wt);
		obs.cx = cx;
		obs.cz = cz;
		obs.hw = hw;
		obs.hd = hd;
		obstacles_.push_back(obs);
	};

	// 障害物1: 横長の壁（コース中央左寄り）
	addObs(7.0f, 5.0f, 2.5f, 0.5f, 1.2f);
	// 障害物2: 縦長の壁（コース左中央）
	addObs(4.0f, 11.5f, 0.5f, 2.5f, 1.2f);
	// 障害物3: 正方形ブロック（コース中央）
	addObs(11.0f, 8.0f, 1.2f, 1.2f, 1.2f);
	// 障害物4: 横長の壁（ホール手前）
	addObs(10.0f, 13.5f, 3.0f, 0.5f, 1.2f);
}

/**
 * @brief 更新
 */
void GolfScene::Update() {
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kAiming;
		}
		break;

	case Phase::kAiming:
		UpdateAiming();
		break;

	case Phase::kBallMoving:
		ball_->Update();
		CheckBallCollision();

		// ホールインの判定
		{
			const Vector3& pos = ball_->GetPosition();
			float dx = pos.x - kHolePosX;
			float dz = pos.z - kHolePosZ;
			if (dx * dx + dz * dz < kHoleRadius * kHoleRadius) {
				// ホールイン！
				phase_     = Phase::kGoal;
				goalTimer_ = 0.0f;
			}
		}

		// ボールが止まったら再エイムへ
		if (phase_ == Phase::kBallMoving && !ball_->IsMoving()) {
			phase_ = Phase::kAiming;
		}
		break;

	case Phase::kGoal:
		goalTimer_ += 1.0f / 60.0f;
		if (goalTimer_ >= 2.5f) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	skydome_->Update();
	camera_.UpdateMatrix();
}

/**
 * @brief 照準フェーズの入力・更新
 */
void GolfScene::UpdateAiming() {
	const float kAimSpeed   = 0.04f;  // 1フレームあたりの照準回転速度 [rad]
	const float kPowerSpeed = 0.018f; // 1フレームあたりのパワー増加量
	const float kMaxPower   = 1.0f;
	const float kMaxSpeed   = 0.38f;  // ショット最大初速

	// 左右矢印キーで照準を回転
	if (Input::GetInstance()->PushKey(DIK_LEFT)) {
		aimAngle_ -= kAimSpeed;
	}
	if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
		aimAngle_ += kAimSpeed;
	}

	// SPACEキー長押しでパワーチャージ、離したらショット
	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
		isCharging_ = true;
		power_ = std::min(power_ + kPowerSpeed, kMaxPower);
	} else if (isCharging_) {
		// キーを離した → ショット実行
		Vector3 direction = {std::sin(aimAngle_), 0.0f, std::cos(aimAngle_)};
		ball_->Hit(direction, power_ * kMaxSpeed);
		strokeCount_++;
		power_      = 0.0f;
		isCharging_ = false;
		phase_      = Phase::kBallMoving;
	}

	// 照準インジケーターをボールの前方に配置
	const Vector3& ballPos = ball_->GetPosition();
	float arrowDist  = 1.3f + power_ * 4.5f;
	float arrowScale = 0.22f + power_ * 0.18f;

	worldTransformArrow_.translation_ = {
	    ballPos.x + std::sin(aimAngle_) * arrowDist,
	    0.3f,
	    ballPos.z + std::cos(aimAngle_) * arrowDist,
	};
	worldTransformArrow_.scale_      = {arrowScale, arrowScale, arrowScale};
	worldTransformArrow_.rotation_.y = aimAngle_;
	WorldTransformUpdate(worldTransformArrow_);
}

/**
 * @brief ボールとコース境界・障害物との衝突処理
 */
void GolfScene::CheckBallCollision() {
	const float r = GolfBall::kRadius;
	Vector3     pos = ball_->GetPosition();
	Vector3&    vel = ball_->GetVelocity();

	// --- コース境界との反射 ---
	if (pos.x - r < kCourseMinX) {
		pos.x = kCourseMinX + r;
		vel.x = std::abs(vel.x) * kBounceDecay;
		ball_->SetPosition(pos);
	}
	if (pos.x + r > kCourseMaxX) {
		pos.x = kCourseMaxX - r;
		vel.x = -std::abs(vel.x) * kBounceDecay;
		ball_->SetPosition(pos);
	}
	if (pos.z - r < kCourseMinZ) {
		pos.z = kCourseMinZ + r;
		vel.z = std::abs(vel.z) * kBounceDecay;
		ball_->SetPosition(pos);
	}
	if (pos.z + r > kCourseMaxZ) {
		pos.z = kCourseMaxZ - r;
		vel.z = -std::abs(vel.z) * kBounceDecay;
		ball_->SetPosition(pos);
	}

	// --- 障害物との衝突 ---
	for (const auto& obs : obstacles_) {
		pos = ball_->GetPosition();

		float obsMinX = obs.cx - obs.hw;
		float obsMaxX = obs.cx + obs.hw;
		float obsMinZ = obs.cz - obs.hd;
		float obsMaxZ = obs.cz + obs.hd;

		float ballMinX = pos.x - r;
		float ballMaxX = pos.x + r;
		float ballMinZ = pos.z - r;
		float ballMaxZ = pos.z + r;

		// AABB のオーバーラップチェック
		if (ballMaxX > obsMinX && ballMinX < obsMaxX &&
		    ballMaxZ > obsMinZ && ballMinZ < obsMaxZ) {

			// オーバーラップ量が小さい軸で押し出し
			float overlapX = std::min(ballMaxX - obsMinX, obsMaxX - ballMinX);
			float overlapZ = std::min(ballMaxZ - obsMinZ, obsMaxZ - ballMinZ);

			if (overlapX < overlapZ) {
				// X 軸方向に押し出して X 速度を反転
				if (vel.x > 0.0f) {
					pos.x = obsMinX - r - 0.01f;
					vel.x = -std::abs(vel.x) * kBounceDecay;
				} else {
					pos.x = obsMaxX + r + 0.01f;
					vel.x = std::abs(vel.x) * kBounceDecay;
				}
			} else {
				// Z 軸方向に押し出して Z 速度を反転
				if (vel.z > 0.0f) {
					pos.z = obsMinZ - r - 0.01f;
					vel.z = -std::abs(vel.z) * kBounceDecay;
				} else {
					pos.z = obsMaxZ + r + 0.01f;
					vel.z = std::abs(vel.z) * kBounceDecay;
				}
			}
			ball_->SetPosition(pos);
		}
	}
}

/**
 * @brief 描画
 */
void GolfScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	// スカイドーム
	skydome_->Draw();

	// コースの床
	blockModel_->Draw(worldTransformFloor_, camera_);

	// コースの壁（4面）
	blockModel_->Draw(worldTransformWallN_, camera_);
	blockModel_->Draw(worldTransformWallS_, camera_);
	blockModel_->Draw(worldTransformWallE_, camera_);
	blockModel_->Draw(worldTransformWallW_, camera_);

	// 障害物
	for (const auto& obs : obstacles_) {
		blockModel_->Draw(*obs.wt, camera_);
	}

	// ホール（目標マーカー）
	ballModel_->Draw(worldTransformHole_, camera_);

	// ゴルフボール
	ball_->Draw();

	// 照準インジケーター（エイム中のみ）
	if (phase_ == Phase::kAiming) {
		ballModel_->Draw(worldTransformArrow_, camera_);
	}

	Model::PostDraw();

	// フェード描画（Sprite バッチは Fade 内で管理）
	fade_->Draw();
}

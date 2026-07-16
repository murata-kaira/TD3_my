#include "GameScene.h"
#include "Math.h"

using namespace KamataEngine;

/**
 * @brief デストラクタ
 */
GameScene::~GameScene() { 
	delete model_;
	delete player_;
	delete debugCamera_;
	delete skydome_;
	delete mapChipField_;
	delete cameraController_;
	delete fade_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();
	
	delete deathParticles_;
	
	delete playerModel_;
	delete blockModel_;
	delete skydomeModel_;
	delete deathParticleModel_;

	delete golfScene_;
}

/**
 * @brief 初期化
 */
void GameScene::Initialize() {
	camera_.Initialize();
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);

	// アクションパートをスキップしてゴルフパートへ直行
	golfScene_ = new GolfScene();
	golfScene_->Initialize();
	phase_ = Phase::kGolf;
}

/**
 * @brief フェーズの変更判定
 */
void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kPlay:
		if (player_->IsDead()) {
			phase_ = Phase::kDeath;
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();
			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(deathParticleModel_, &camera_, deathParticlesPosition);
		}
		break;
	case Phase::kDeath:
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
		break;
	case Phase::kFadeOut:
		if (fade_->IsFinished()) {
			// アクションパート終了 → ゴルフゲームパートへ
			golfScene_ = new GolfScene();
			golfScene_->Initialize();
			phase_ = Phase::kGolf;
		}
		break;
	case Phase::kGolf:
		if (golfScene_ && golfScene_->IsFinished()) {
			finished_ = true;
		}
		break;
	}
}

/**
 * @brief マップデータに基づいてブロックを生成
 */
void GameScene::GenerateBlocks() {
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal, nullptr);
	}

	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
				worldTransformBlocks_[i][j] = worldTransform;
			}
		}
	}
}

/**
 * @brief 更新
 */
void GameScene::Update() { 
	ChangePhase();

	// ゴルフフェーズ中はゴルフシーンに全処理を委譲
	if (phase_ == Phase::kGolf) {
		if (golfScene_) golfScene_->Update();
		return;
	}

	skydome_->Update();
	cameraController_->Update();
	
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			phase_ = Phase::kPlay;
		}
		player_->Update();
		break;

	case Phase::kPlay:
		player_->Update();
		CheckAllCollisions();
		break;

	case Phase::kDeath:
		if (deathParticles_) {
			deathParticles_->Update();
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();
		break;

	case Phase::kGolf:
		// 上のアーリーリターンで処理済み
		break;
	}

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}

	for (auto& line : worldTransformBlocks_) {
		for (auto& block : line) {
			if (block) WorldTransformUpdate(*block);
		}
	}
}

/**
 * @brief 描画
 */
void GameScene::Draw() { 
	// ゴルフフェーズ中はゴルフシーンに全描画を委譲
	if (phase_ == Phase::kGolf) {
		if (golfScene_) golfScene_->Draw();
		return;
	}

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	skydome_->Draw(); 

	if (!player_->IsDead()) {
		player_->Draw(); 
	}

	for (auto& line : worldTransformBlocks_) {
		for (auto& block : line) {
			if (block) blockModel_->Draw(*block, camera_);
		}
	}

	if (player_->IsDead() && deathParticles_) {
		deathParticles_->Draw();
	}

	Model::PostDraw();

	Sprite::PreDraw(dxCommon->GetCommandList());
	Sprite::PostDraw();
}

/**
 * @brief 当たり判定のチェック
 */
void GameScene::CheckAllCollisions() {
	// エネミー削除に伴い、現在はプレイヤーとマップの判定のみ（Playerクラス内で処理済み）
	// 将来的にアイテム等の判定が必要になればここに追加する
}

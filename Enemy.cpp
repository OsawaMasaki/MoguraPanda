#include "Enemy.h"
#include "time.h"
#include "Stage.h"
#include "Player.h"

namespace
{
	const int ENEMY_SIZE = 48; //敵のサイズ 32*32
	const Point ENEMY_START_POS = { 20 * ENEMY_SIZE, 10 * ENEMY_SIZE }; //敵の初期位置
	const DIR INIT_ENEMY_DIR = { RIGHT };
	const int ENEMY_DRAW_SIZE = 32; //敵の描画サイズ
	const int animFrame[4]{ 0, 1, 2, 1 };
	const float ANIM_INTERVAL = 0.2f;
}

enum State {
	Patrol = 0,
	Chase,
	Attack,
	Search
};

Enemy::Enemy()
	: GameObject()
	, state_(Patrol)
{
	hImage_ = LoadGraph("Assets/panda_R.png");
	pos_ = ENEMY_START_POS; //32はブロックの位置pos_
	dir_ = INIT_ENEMY_DIR;
}

Enemy::~Enemy()
{
}

void Enemy::Update()
{
	//GetRand(数値)
	//数秒に1回向きをランダムに変える
	static float dir_timer = 2.0f;
	static float prog_timer = 0.5f;
	float dt = Time::DeltaTime();
	dir_timer = dir_timer - dt;
	prog_timer = prog_timer - dt;

	//Playerを探す
	auto player = FindGameObject<Player>();
	Point playerPos = player->GetPlayerPos();

	//if (dir_timer < 0.0f)
	//{
	//	//dir_ = (DIR)(GetRand(3));
	//	dir_timer = 3.0f + dir_timer;
	//}

	if (prog_timer < 0.0f)
	{
		// 1. まず、現在の位置から「一歩進んだ仮の座標」を計算する
		Point newPos = pos_;
		switch (dir_)
		{
		case UP:    newPos.y -= ENEMY_DRAW_SIZE; break;
		case DOWN:  newPos.y += ENEMY_DRAW_SIZE; break;
		case LEFT:  newPos.x -= ENEMY_DRAW_SIZE; break;
		case RIGHT: newPos.x += ENEMY_DRAW_SIZE; break;
		default: break;
		}

		// 2.　一歩進めた「newPos」の場所が壁かどうかをここで調べる
		int mapValue = FindGameObject<Stage>()->GetMap(newPos.x / CHA_SIZE, newPos.y / CHA_SIZE);

		float distX = abs(playerPos.x - pos_.x);
		float distY = abs(playerPos.y - pos_.y);
		float totalDist = distX + distY;
	 
		//ここからプレイヤーが近くにいるかの処理
		//範囲内に入ったら追いかけるように
		switch (state_)
		{
		case Patrol:
			// プレイヤーが近く（5マス未満）にいたら追跡（Chase）に切り替え
			if (totalDist < 5 * ENEMY_DRAW_SIZE)
			{
				state_ = Chase;
				break;
			}

			// 壁（1）だったら、移動せずにその場で向きだけ変える
			if (mapValue == 1)
			{
				switch (dir_)
				{
				case UP:    dir_ = RIGHT; break;
				case RIGHT: dir_ = DOWN;  break;
				case DOWN:  dir_ = LEFT;  break;
				case LEFT:  dir_ = UP;    break;
				default: break;
				}
			}
			else
			{
				pos_ = newPos;
			}
			break;
			
		case Chase:
			// プレイヤーが離れたら（7マス以上とかにすると自然だよ）見失って探す（Search）へ
			if (totalDist > 7 * ENEMY_DRAW_SIZE)
			{
				state_ = Search;
				break;
			}
			// もし攻撃が届く距離（例: 1マス以内）なら攻撃（Attack）へ
			else if (totalDist <= ENEMY_DRAW_SIZE)
			{
				state_ = Attack;
				break;
			}

			// 追跡移動の処理
			pos_ = newPos;
			if (distX > distY)
			{
				dir_ = (playerPos.x > pos_.x) ? RIGHT : LEFT;
			}
			else
			{
				dir_ = (playerPos.y > pos_.y) ? DOWN : UP;
			}
			break;
		case Attack:
			if (totalDist > ENEMY_DRAW_SIZE)
			{
				state_ = Chase;
			}
			break;
		case Search:
			// その場でキョロキョロ探す処理などをここに書くよ
			// 今は暫定で、プレイヤーを見つけたらChaseに戻り、見つからなければPatrolに戻るようにしておくね
			if (totalDist < 5 * ENEMY_DRAW_SIZE)
			{
				state_ = Chase;
			}
			else
			{
				state_ = Patrol; // 見つからなければ通常の巡回に戻る
			}
			break;
		}
		//if (distX + distY < 5 * ENEMY_DRAW_SIZE)
		//{
		//	pos_ = newPos;
		//	if (distX > distY)
		//	{
		//		// X方向の差の方が大きいから、左右のどちらかに向きを変える
		//		dir_ = (playerPos.x > pos_.x) ? RIGHT : LEFT;
		//	}
		//	else
		//	{
		//		// Y方向の差の方が大きいから、上下のどちらかに向きを変える
		//		dir_ = (playerPos.y > pos_.y) ? DOWN : UP;
		//	}
		//}
		//else
		//{
		//	// 3. もし移動先が「壁（1）」だったら、移動せずにその場で向きだけ変える
		//	if (mapValue == 1)
		//	{
		//		switch (dir_)
		//		{
		//		case UP:    dir_ = RIGHT; break;
		//		case RIGHT: dir_ = DOWN;  break;
		//		case DOWN:  dir_ = LEFT;  break;
		//		case LEFT:  dir_ = UP;    break;
		//		default: break;
		//		}
		//	}
		//	// 4. 壁じゃない（床）なら移動する
		//	else
		//	{
		//		pos_ = newPos;
		//	}
		//}

		prog_timer = 0.5f + prog_timer;
	}
}

void Enemy::Draw()
{
	static float animTimer = ANIM_INTERVAL;
	static int frame = 0;
	int nowFrame = animFrame[frame];

	Rect iRect[4] = {
		{  nowFrame * ENEMY_SIZE, 3 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE},
		{  nowFrame * ENEMY_SIZE, 0 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE},
		{  nowFrame * ENEMY_SIZE, 1 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE},
		{  nowFrame * ENEMY_SIZE, 2 * ENEMY_SIZE, ENEMY_SIZE, ENEMY_SIZE}
	};

	// ==========================================================
	// 【追加】敵の正面1マスにデバッグ用の箱を描画する
	// ==========================================================
	// 1. まず、今の位置から「正面一歩先」の座標を計算する
	Point frontBoxPos = pos_; // 敵の今の位置をベースにする
	switch (dir_)
	{
	case UP:
		frontBoxPos.y -= ENEMY_DRAW_SIZE; // 上にずらす
		break;
	case DOWN:
		frontBoxPos.y += ENEMY_DRAW_SIZE; // 下にずらす
		break;
	case LEFT:
		frontBoxPos.x -= ENEMY_DRAW_SIZE; // 左にずらす
		break;
	case RIGHT:
		frontBoxPos.x += ENEMY_DRAW_SIZE; // 右にずらす
		break;
	}

	// 2. 計算した座標（frontBoxPos）に、デバッグ用の箱を描画する
	//    敵本体の黄色（255, 255, 0）と違う色にすると分かりやすいよ（例：水色）
	for (int y = 0; y < 3;y++)
	{
		for (int x = 0; x < 3;x++)
		{
			DrawBox(frontBoxPos.x - x * ENEMY_DRAW_SIZE, frontBoxPos.y - y * ENEMY_DRAW_SIZE,
				frontBoxPos.x + x * ENEMY_DRAW_SIZE + ENEMY_DRAW_SIZE,
				frontBoxPos.y + y * ENEMY_DRAW_SIZE + ENEMY_DRAW_SIZE,
				GetColor(0, 200, 200), FALSE); // 水色の箱を5*5で描画
		}
	}
	// ==========================================================


	DrawBox(pos_.x, pos_.y, pos_.x + ENEMY_DRAW_SIZE, pos_.y + ENEMY_DRAW_SIZE,
		GetColor(255, 255, 0), FALSE,2);
	DrawRectExtendGraph(pos_.x, pos_.y,pos_.x + ENEMY_DRAW_SIZE, pos_.y + ENEMY_DRAW_SIZE,
		               iRect[dir_].x, iRect[dir_].y, iRect[dir_].w, iRect[dir_].h, hImage_, TRUE);
	if (animTimer < 0) {
		frame = (++frame) % 4;
		animTimer = ANIM_INTERVAL + animTimer;
	}
	animTimer = animTimer - Time::DeltaTime();



}

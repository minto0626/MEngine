#include "Random.h"

// 非決定的シードで静的エンジンを初期化
std::mt19937 Random::_engine(std::random_device{}());

int Random::Range(int min, int max)
{
	// [min, max-1] の範囲で整数を生成
	std::uniform_int_distribution<int> dist(min, max - 1);
	return dist(_engine);
}

float Random::Range(float min, float max)
{
	// [min, max] の範囲で浮動小数点を生成
	std::uniform_real_distribution<float> dist(min, max);
	return dist(_engine);
}

void Random::InitState(unsigned int seed)
{
	// 提供されたシードでエンジンを再シード
	_engine = std::mt19937(seed);
}
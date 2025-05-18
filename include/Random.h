#pragma once
#include <random>

class Random
{
public:
	/// <summary>
	/// 整数を[min, max-1]の範囲で生成
	/// </summary>
	static int Range(int min, int max);

	/// <summary>
	/// 浮動小数点を[min, max]の範囲で生成
	/// </summary>
	static float Range(float min, float max);

	/// <summary>
	/// 乱数生成器を特定のシードで初期化
	/// </summary>
	static void InitState(unsigned int seed);

private:
	// 乱数生成用の静的エンジン
	static std::mt19937 _engine;

};
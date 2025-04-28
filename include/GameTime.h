#pragma once
#include <chrono>
#include <cstdint>

namespace Engine
{
    class Time
    {
    private:
        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;

        TimePoint _startTime;            // ゲーム開始時刻
        TimePoint _lastFrameTime;        // 前フレームの時刻
        float _deltaTime = 0.0f;         // フレーム間隔（秒）
        float _totalTime = 0.0f;         // ゲーム開始からの経過時間（秒）
        uint64_t _frameCount = 0;        // フレーム数
        float _fixedDeltaTime = 1.0f / 60.0f; // 固定デルタタイム（デフォルト60FPS）
        float _timeScale = 1.0f;         // タイムスケール（1.0が通常速度）

    public:
        void Init();
        void Update();

        float GetDeltaTime() const { return _deltaTime; }
        float GetTime() const { return _totalTime; }
        uint64_t GetFrameCount() const { return _frameCount; }

        // 固定デルタタイム（物理更新用、秒単位）を設定/取得
        void SetFixedDeltaTime(float fixedDelta) { _fixedDeltaTime = fixedDelta; }
        float GetFixedDeltaTime() const { return _fixedDeltaTime; }

        // タイムスケールの設定/取得（ゲーム速度の調整用）
        void SetTimeScale(float scale) { _timeScale = scale; }
        float GetTimeScale() const { return _timeScale; }

        // デルタタイムにタイムスケールを適用した値を取得
        float GetScaledDeltaTime() const { return _deltaTime * _timeScale; }
    };
}
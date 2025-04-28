#include "GameTime.h"

namespace Engine
{
    void Time::Init()
    {
        _startTime = Clock::now();
        _lastFrameTime = _startTime;
        _deltaTime = 0.0f;
        _totalTime = 0.0f;
        _frameCount = 0;
        _fixedDeltaTime = 1.0f / 60.0f;
        _timeScale = 1.0f;

        _deltaTimeHistory.clear();
        _accumulatedTime = 0.0f;
    }

    void Time::Update()
    {
        // 現在の時刻を取得
        auto currentTime = Clock::now();

        // デルタタイムを計算（秒単位）
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - _lastFrameTime);
        _deltaTime = duration.count() / 1'000'000.0f;

        // 経過時間を計算
        auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - _startTime);
        _totalTime = totalDuration.count() / 1'000'000.0f;

        // フレームカウントをインクリメント
        _frameCount++;

        // 平均FPS計算用のデルタタイム履歴を更新
        _deltaTimeHistory.push_back(_deltaTime);
        _accumulatedTime += _deltaTime;

        // 時間窓（１秒）を超えた古いデータを削除
        while (_accumulatedTime > _fpsWindow && !_deltaTimeHistory.empty())
        {
            _accumulatedTime -= _deltaTimeHistory.front();
            _deltaTimeHistory.erase(_deltaTimeHistory.begin());
        }

        // 前フレームの時刻を更新
        _lastFrameTime = currentTime;
    }

    float Time::GetInstantFPS() const
    {
        return (_deltaTime > 0.0f) ? 1.0f / _deltaTime : 0.0f;
    }

    float Time::GetAverageFPS() const
    {
        if (_deltaTimeHistory.empty())
        {
            return 0.0f;
        }

        float averageDeltaTime = _accumulatedTime / static_cast<float>(_deltaTimeHistory.size());
        return (averageDeltaTime > 0.0f) ? 1.0f / averageDeltaTime : 0.0f;
    }
}
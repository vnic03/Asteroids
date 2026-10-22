#pragma once

#include <string>
#include <vector>

enum class ScoreMode {
    Solo,
    Coop
};

struct HighScore {
    std::string name;
    int score = 0;
    int legacyScore = 0;
};

bool validPlayerName(const std::string& name);

std::vector<HighScore> readHighScores(
    ScoreMode mode = ScoreMode::Solo
);

bool writeHighScore(
    const std::string& name,
    int score,
    ScoreMode mode = ScoreMode::Solo
);

bool removePlayer(const std::string& name);

std::string highScoreStatus();

#pragma once
#include <string>
#include <vector>

enum class TitleCategory { Allowed, Blocked, Neutral, Ignored };
enum class FocusState { OnTask, Neutral, Distracted, Idle };

struct FocusRulesConfig {
    std::vector<std::string> allowKeywords;
    std::vector<std::string> blockKeywords;
    std::vector<std::string> ignoreKeywords;
    double graceSeconds = 10.0;
    double idleLimitSeconds = 300.0;
};

TitleCategory classifyTitle(const std::string& title, const FocusRulesConfig& config);
const char* toString(FocusState state);

class FocusRules {
public:
    explicit FocusRules(FocusRulesConfig config);

    FocusState update(const std::string& title, double idleSeconds, double secondsSinceLastUpdate);
    FocusState state() const { return state_; }

private:
    FocusRulesConfig config_;
    TitleCategory    lastCategory_ = TitleCategory::Neutral;
    double           blockedFor_ = 0.0;
    FocusState       state_ = FocusState::Neutral;
};
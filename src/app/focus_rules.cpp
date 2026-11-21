#include "focus_rules.h"

#include <algorithm>
#include <cctype>
#include <utility>

namespace {

    std::string toLower(std::string text) {
        std::transform(text.begin(), text.end(), text.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return text;
    }

    bool containsAny(const std::string& lowerTitle, const std::vector<std::string>& keywords) {
        for (const std::string& keyword : keywords) {
            if (lowerTitle.find(toLower(keyword)) != std::string::npos) return true;
        }
        return false;
    }

}

TitleCategory classifyTitle(const std::string& title, const FocusRulesConfig& config) {
    if (title.empty()) return TitleCategory::Ignored;
    const std::string lower = toLower(title);
    if (containsAny(lower, config.ignoreKeywords)) return TitleCategory::Ignored;
    if (containsAny(lower, config.allowKeywords))  return TitleCategory::Allowed;
    if (containsAny(lower, config.blockKeywords))  return TitleCategory::Blocked;
    return TitleCategory::Neutral;
}

const char* toString(FocusState state) {
    switch (state) {
    case FocusState::OnTask:     return "OnTask";
    case FocusState::Neutral:    return "Neutral";
    case FocusState::Distracted: return "Distracted";
    case FocusState::Idle:       return "Idle";
    }
    return "Unknown";
}

FocusRules::FocusRules(FocusRulesConfig config) : config_(std::move(config)) {}

FocusState FocusRules::update(const std::string& title, double idleSeconds, double secondsSinceLastUpdate) {
    TitleCategory category = classifyTitle(title, config_);
    if (category == TitleCategory::Ignored) {
        category = lastCategory_;
    }
    lastCategory_ = category;

    if (idleSeconds >= config_.idleLimitSeconds) {
        state_ = FocusState::Idle;
        return state_;
    }

    switch (category) {
    case TitleCategory::Blocked:
        blockedFor_ += secondsSinceLastUpdate;
        if (blockedFor_ >= config_.graceSeconds) {
            state_ = FocusState::Distracted;
        }
        else if (state_ == FocusState::Idle) {
            state_ = FocusState::Neutral;
        }
        break;
    case TitleCategory::Allowed:
        blockedFor_ = 0.0;
        state_ = FocusState::OnTask;
        break;
    default:
        blockedFor_ = 0.0;
        state_ = FocusState::Neutral;
        break;
    }
    return state_;
}
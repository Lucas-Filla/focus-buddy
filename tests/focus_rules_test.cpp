#include "app/focus_rules.h"

#include <gtest/gtest.h>

namespace {

	FocusRulesConfig testConfig() {
		return FocusRulesConfig{
			.allowKeywords = { "Visual Studio", "CS 4040", "Lecture" },
			.blockKeywords = { "Youtube", "Reddit" },
			.ignoreKeywords = {},
			.graceSeconds = 10.0,
			.idleLimitSeconds = 300.0,
		};
	}

}

TEST(ClassifyTitle, BlockedKeywordIsBlocked) {
    EXPECT_EQ(classifyTitle("Cat videos - YouTube - Google Chrome", testConfig()), TitleCategory::Blocked);
}

TEST(ClassifyTitle, AllowedKeywordIsAllowed) {
    EXPECT_EQ(classifyTitle("Assignments: CS 4040 100 Fall 26 - Google Chrome", testConfig()), TitleCategory::Allowed);
}

TEST(ClassifyTitle, AllowBeatsBlock) {
    EXPECT_EQ(classifyTitle("Lecture 12 - YouTube - Google Chrome", testConfig()), TitleCategory::Allowed);
}

TEST(ClassifyTitle, MatchingIgnoresCase) {
    EXPECT_EQ(classifyTitle("REDDIT - the front page", testConfig()), TitleCategory::Blocked);
}

TEST(ClassifyTitle, UnknownTitleIsNeutral) {
    EXPECT_EQ(classifyTitle("Downloads - File Explorer", testConfig()), TitleCategory::Neutral);
}

TEST(ClassifyTitle, EmptyTitleIsIgnored) {
    EXPECT_EQ(classifyTitle("", testConfig()), TitleCategory::Ignored);
}

TEST(FocusRules, StartsNeutral) {
    FocusRules rules(testConfig());
    EXPECT_EQ(rules.state(), FocusState::Neutral);
}

TEST(FocusRules, AllowedTitleIsOnTask) {
    FocusRules rules(testConfig());
    EXPECT_EQ(rules.update("main.cpp - Microsoft Visual Studio", 0.0, 1.0), FocusState::OnTask);
}

TEST(FocusRules, BlockedTitleWaitsForGracePeriod) {
    FocusRules rules(testConfig());
    rules.update("main.cpp - Microsoft Visual Studio", 0.0, 1.0);

    for (int second = 1; second < 10; ++second) {
        EXPECT_EQ(rules.update("Cats - YouTube", 0.0, 1.0), FocusState::OnTask) << "after " << second << " s";
    }
    EXPECT_EQ(rules.update("Cats - YouTube", 0.0, 1.0), FocusState::Distracted);
}

TEST(FocusRules, LeavingBlockedTitleResetsTheTimer) {
    FocusRules rules(testConfig());
    rules.update("Cats - YouTube", 0.0, 9.0);
    rules.update("main.cpp - Microsoft Visual Studio", 0.0, 1.0);
    EXPECT_EQ(rules.update("Cats - YouTube", 0.0, 9.0), FocusState::OnTask);
}

TEST(FocusRules, IgnoredWindowKeepsCurrentState) {
    FocusRules rules(testConfig());
    rules.update("Cats - YouTube", 0.0, 10.0);
    ASSERT_EQ(rules.state(), FocusState::Distracted);
    EXPECT_EQ(rules.update("", 0.0, 1.0), FocusState::Distracted);
}

TEST(FocusRules, IdleBeatsEverything) {
    FocusRules rules(testConfig());
    EXPECT_EQ(rules.update("main.cpp - Microsoft Visual Studio", 300.0, 1.0), FocusState::Idle);
}

TEST(FocusRules, ComingBackFromIdleToNeutralTitle) {
    FocusRules rules(testConfig());
    rules.update("main.cpp - Microsoft Visual Studio", 400.0, 1.0);
    EXPECT_EQ(rules.update("Downloads - File Explorer", 0.0, 1.0), FocusState::Neutral);
}
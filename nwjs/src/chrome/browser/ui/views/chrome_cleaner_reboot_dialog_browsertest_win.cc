// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/chrome_cleaner_reboot_dialog_win.h"

#include "base/test/scoped_feature_list.h"
#include "chrome/browser/safe_browsing/chrome_cleaner/chrome_cleaner_controller_win.h"
#include "chrome/browser/safe_browsing/chrome_cleaner/chrome_cleaner_reboot_dialog_controller_impl_win.h"
#include "chrome/browser/safe_browsing/chrome_cleaner/mock_chrome_cleaner_controller_win.h"
#include "chrome/browser/safe_browsing/chrome_cleaner/srt_field_trial_win.h"
#include "chrome/browser/ui/test/test_browser_dialog.h"
#include "content/public/test/browser_test.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace safe_browsing {
namespace {

// Provides tests which allows explicit invocation of the Chrome Cleaner Reboot
// Prompt useful for checking dialog layout or any other interactive
// functionality tests. See docs/testing/test_browser_dialog.md for description
// of the testing framework.
class ChromeCleanerRebootDialog : public DialogBrowserTest {
 public:
  void SetUpInProcessBrowserTestFixture() override {
    scoped_feature_list_.InitAndEnableFeature(kRebootPromptDialogFeature);
  }

  // DialogBrowserTest overrides.
  void ShowDialog(const std::string& name) override {
    ON_CALL(mock_cleaner_controller_, state())
        .WillByDefault(::testing::Return(
            safe_browsing::ChromeCleanerController::State::kRebootRequired));

    safe_browsing::ChromeCleanerRebootDialogControllerImpl::Create(
        &mock_cleaner_controller_);
  }

 private:
  // Since the DialogBrowserTest can be run interactively, we use NiceMock here
  // to suppress warnings about uninteresting calls.
  ::testing::NiceMock<MockChromeCleanerController> mock_cleaner_controller_;

  base::test::ScopedFeatureList scoped_feature_list_;
};

IN_PROC_BROWSER_TEST_F(ChromeCleanerRebootDialog, InvokeDialog_default) {
  RunDialog();
}

}  // namespace
}  // namespace safe_browsing
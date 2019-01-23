// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "content/browser/accessibility/browser_accessibility.h"
#include "content/browser/accessibility/browser_accessibility_manager.h"
#include "content/browser/web_contents/web_contents_impl.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/content_browser_test.h"
#include "content/public/test/content_browser_test_utils.h"
#include "content/shell/browser/shell.h"
#include "content/test/accessibility_browser_test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace content {

class AccessibilityFullscreenBrowserTest : public ContentBrowserTest {
 public:
  AccessibilityFullscreenBrowserTest() = default;
  ~AccessibilityFullscreenBrowserTest() override = default;

 protected:
  BrowserAccessibility* FindButton(BrowserAccessibility* node) {
    if (node->GetRole() == ui::AX_ROLE_BUTTON)
      return node;
    for (unsigned i = 0; i < node->PlatformChildCount(); i++) {
      if (BrowserAccessibility* button = FindButton(node->PlatformGetChild(i)))
        return button;
    }
    return nullptr;
  }

  int CountLinks(BrowserAccessibility* node) {
    if (node->GetRole() == ui::AX_ROLE_LINK)
      return 1;
    int links_in_children = 0;
    for (unsigned i = 0; i < node->PlatformChildCount(); i++) {
      links_in_children += CountLinks(node->PlatformGetChild(i));
    }
    return links_in_children;
  }
};

namespace {

// FakeFullscreenDelegate simply stores the latest requested mod and reports it
// back, which is all that is required for the renderer to enter fullscreen.
class FakeFullscreenDelegate : public WebContentsDelegate {
 public:
  FakeFullscreenDelegate() = default;
  ~FakeFullscreenDelegate() override = default;

  void EnterFullscreenModeForTab(WebContents*, const GURL&) override {
    is_fullscreen_ = true;
  }

  void ExitFullscreenModeForTab(WebContents*) override {
    is_fullscreen_ = false;
  }

  bool IsFullscreenForTabOrPending(const WebContents*) const override {
    return is_fullscreen_;
  }

 private:
  bool is_fullscreen_ = false;
  DISALLOW_COPY_AND_ASSIGN(FakeFullscreenDelegate);
};

}  // namespace

IN_PROC_BROWSER_TEST_F(AccessibilityFullscreenBrowserTest,
                       IgnoreElementsOutsideFullscreenElement) {
  ASSERT_TRUE(embedded_test_server()->Start());

  FakeFullscreenDelegate delegate;
  shell()->web_contents()->SetDelegate(&delegate);

  AccessibilityNotificationWaiter waiter(
      shell()->web_contents(), ui::kAXModeComplete, ui::AX_EVENT_LOAD_COMPLETE);
  GURL url(
      embedded_test_server()->GetURL("/accessibility/fullscreen/links.html"));
  NavigateToURL(shell(), url);
  waiter.WaitForNotification();

  WebContentsImpl* web_contents =
      static_cast<WebContentsImpl*>(shell()->web_contents());
  BrowserAccessibilityManager* manager =
      web_contents->GetRootBrowserAccessibilityManager();

  // Initially there are 3 links in the accessiblity tree.
  EXPECT_EQ(3, CountLinks(manager->GetRoot()));

  // Enter fullscreen by finding the button and performing the default action,
  // which is to click it.
  BrowserAccessibility* button = FindButton(manager->GetRoot());
  ASSERT_NE(nullptr, button);
  manager->DoDefaultAction(*button);

  // Upon entering fullscreen, the page will change the button text to "Done".
  WaitForAccessibilityTreeToContainNodeWithName(web_contents, "Done");

  // Now, the two links outside of the fullscreen element are gone.
  EXPECT_EQ(1, CountLinks(manager->GetRoot()));
}

}  // namespace content
// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_INTERNET_HANDLER_H_
#define CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_INTERNET_HANDLER_H_

#include <memory>

#include "base/macros.h"
#include "chrome/browser/ui/app_list/arc/arc_vpn_provider_manager.h"
#include "chrome/browser/ui/webui/settings/settings_page_ui_handler.h"
#include "ui/gfx/native_widget_types.h"

class Profile;

namespace chromeos {
namespace settings {

// Chrome OS Internet settings page UI handler.
// TODO(lgcheng/stevenjb): Rename this ThirdPartyVpnHandler once configuration
// is handled in the Settings UI (crbug.com/380937
class InternetHandler : public app_list::ArcVpnProviderManager::Observer,
                        public ::settings::SettingsPageUIHandler {
 public:
  explicit InternetHandler(Profile* profile);
  ~InternetHandler() override;

  // SettingsPageUIHandler implementation.
  void RegisterMessages() override;
  void OnJavascriptAllowed() override;
  void OnJavascriptDisallowed() override;

  // app_list::ArcVpnProviderManager::Observer:
  void OnArcVpnProvidersRefreshed(
      const std::vector<
          std::unique_ptr<app_list::ArcVpnProviderManager::ArcVpnProvider>>&
          arc_vpn_providers) override;
  void OnArcVpnProviderRemoved(const std::string& package_name) override;
  void OnArcVpnProviderUpdated(app_list::ArcVpnProviderManager::ArcVpnProvider*
                                   arc_vpn_provider) override;

 private:
  // Settings JS handlers.
  void AddNetwork(const base::ListValue* args);
  void ConfigureNetwork(const base::ListValue* args);
  void RequestArcVpnProviders(const base::ListValue* args);

  // Sets list of Arc Vpn providers.
  void SetArcVpnProviders(
      const std::vector<
          std::unique_ptr<app_list::ArcVpnProviderManager::ArcVpnProvider>>&
          arc_vpn_providers);

  // Sends list of Arc Vpn providers to Chrome://settings.
  void SendArcVpnProviders();

  gfx::NativeWindow GetNativeWindow() const;

  std::map<std::string, std::unique_ptr<base::DictionaryValue>>
      arc_vpn_providers_;

  Profile* const profile_;

  app_list::ArcVpnProviderManager* arc_vpn_provider_manager_;

  DISALLOW_COPY_AND_ASSIGN(InternetHandler);
};

}  // namespace settings
}  // namespace chromeos

#endif  // CHROME_BROWSER_UI_WEBUI_SETTINGS_CHROMEOS_INTERNET_HANDLER_H_
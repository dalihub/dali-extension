#ifndef DALI_TIZEN_WEB_ENGINE_CHROMIUM_H
#define DALI_TIZEN_WEB_ENGINE_CHROMIUM_H

/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <functional>
#include <dali-toolkit/dali-toolkit.h>
#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>
#include <dali/public-api/images/native-image-interface.h>

#include <tbm_surface.h>

namespace Dali
{
namespace Plugin
{

class WebViewContainerForDali;

class WebViewContainerClient
{
public:
  virtual void UpdateImage(tbm_surface_h buffer) = 0;
  virtual void LoadStarted() = 0;
  virtual void LoadFinished() = 0;
};

class TizenWebEngineChromium
    : public Dali::WebEnginePlugin,
      public WebViewContainerClient
{
public:
  TizenWebEngineChromium();
  ~TizenWebEngineChromium() override;

  // WebEnginePlugin Interface
  void Create( int width, int height,
               const std::string& locale,
               const std::string& timezoneID ) override;
  void Destroy() override;

  void LoadUrl( const std::string& url ) override;
  NativeImageInterfacePtr GetNativeImageSource() override;
  const std::string& GetUrl() override;

  void LoadHTMLString( const std::string& string ) override;
  void Reload() override;
  void StopLoading() override;

  bool CanGoForward() override;
  void GoForward() override;
  bool CanGoBack() override;
  void GoBack() override;

  void EvaluateJavaScript( const std::string& script ) override;
  void AddJavaScriptInterface(
      const std::string& exposedObjectName,
      const std::string& jsFunctionName,
      std::function<std::string( const std::string& )> cb ) override;
  void RemoveJavascriptInterface( const std::string& exposedObjectName,
                                  const std::string& jsFunctionName ) override;

  void ClearHistory() override;
  void ClearCache() override;

  void SetSize( int width, int height ) override;

  bool SendTouchEvent( const Dali::TouchData& touch ) override;
  bool SendKeyEvent( const Dali::KeyEvent& event ) override;

  Dali::WebEnginePlugin::WebEngineSignalType& PageLoadStartedSignal() override;
  Dali::WebEnginePlugin::WebEngineSignalType& PageLoadFinishedSignal() override;

  // WebViewContainerClient Interface
  void UpdateImage( tbm_surface_h buffer ) override;
  void LoadStarted() override;
  void LoadFinished() override;

private:
  WebViewContainerForDali* mWebViewContainer;
  Dali::NativeImageSourcePtr mDaliImageSrc;

  Dali::WebEnginePlugin::WebEngineSignalType mLoadStartedSignal;
  Dali::WebEnginePlugin::WebEngineSignalType mLoadFinishedSignal;

  std::string mUrl;
};
} // namespace Plugin
} // namespace Dali

#endif

#ifndef DALI_TIZEN_WEB_ENGINE_CHROMIUM_H
#define DALI_TIZEN_WEB_ENGINE_CHROMIUM_H

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

// EXTERNAL INCLUDES
#include <Evas.h>

#include <dali-toolkit/dali-toolkit.h>
#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>
#include <dali/public-api/images/native-image-interface.h>

#include <ewk_view_internal.h>
#include <memory>
#include <tbm_surface.h>
#include <unordered_map>

namespace Dali
{

class PixelData;

namespace Plugin
{

/**
 * @brief A class implements Dali::WebEnginePlugin for tizen chromium.
 */
class TizenWebEngineChromium : public Dali::WebEnginePlugin
{

public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineChromium();

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineChromium() override;


  // WebEnginePlugin Interface

  /**
   * @copydoc Dali::WebEnginePlugin::Create()
   */
  void Create(int width, int height, const std::string& locale, const std::string& timezoneID) override;

  /**
   * @copydoc Dali::WebEnginePlugin::Create()
   */
  void Create(int width, int height, int argc, char** argv) override;

  /**
   * @copydoc Dali::WebEnginePlugin::Destroy()
   */
  void Destroy() override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetSettings()
   */
  Dali::WebEngineSettings& GetSettings() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetContext()
   */
  Dali::WebEngineContext& GetContext() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetCookieManager()
   */
  Dali::WebEngineCookieManager& GetCookieManager() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetBackForwardList()
   */
  Dali::WebEngineBackForwardList& GetBackForwardList() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::LoadUrl()
   */
  void LoadUrl(const std::string& url) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetNativeImageSource()
   */
  NativeImageInterfacePtr GetNativeImageSource() override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetTitle()
   */
  std::string GetTitle() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetFavicon()
   */
  Dali::PixelData GetFavicon() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetUrl()
   */
  std::string GetUrl() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::LoadHtmlString()
   */
  void LoadHtmlString( const std::string& string ) override;

  /**
   * @copydoc Dali::WebEnginePlugin::Reload()
   */
  void Reload() override;

  /**
   * @copydoc Dali::WebEnginePlugin::StopLoading()
   */
  void StopLoading() override;

  /**
   * @copydoc Dali::WebEnginePlugin::Suspend()
   */
  void Suspend() override;

  /**
   * @copydoc Dali::WebEnginePlugin::Resume()
   */
  void Resume() override;

  /**
   * @copydoc Dali::WebEnginePlugin::ScrollBy()
   */
  void ScrollBy(int deltaX, int deltaY) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetScrollPosition()
   */
  void SetScrollPosition(int x, int y) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetScrollPosition()
   */
  void GetScrollPosition(int& x, int& y) const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetScrollSize()
   */
  void GetScrollSize(int& width, int& height) const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetContentSize()
   */
  void GetContentSize(int& width, int& height) const override;

  /**
   * @copydoc Dali::WebEnginePlugin::CanGoForward()
   */
  bool CanGoForward() override;

  /**
   * @copydoc Dali::WebEnginePlugin::GoForward()
   */
  void GoForward() override;

  /**
   * @copydoc Dali::WebEnginePlugin::CanGoBack()
   */
  bool CanGoBack() override;

  /**
   * @copydoc Dali::WebEnginePlugin::GoBack()
   */
  void GoBack() override;

  /**
   * @copydoc Dali::WebEnginePlugin::EvaluateJavaScript()
   */
  void EvaluateJavaScript(const std::string& script, JavaScriptMessageHandlerCallback resultHandler) override;

  /**
   * @copydoc Dali::WebEnginePlugin::AddJavaScriptMessageHandler()
   */
  void AddJavaScriptMessageHandler(const std::string& exposedObjectName, JavaScriptMessageHandlerCallback handler) override;

  /**
   * @copydoc Dali::WebEnginePlugin::ClearAllTilesResources()
   */
  void ClearAllTilesResources() override;

  /**
   * @copydoc Dali::WebEnginePlugin::ClearHistory()
   */
  void ClearHistory() override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetUserAgent()
   */
  std::string GetUserAgent() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetUserAgent()
   */
  void SetUserAgent(const std::string& userAgent) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetSize()
   */
  void SetSize(int width, int height) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SendTouchEvent()
   */
  bool SendTouchEvent(const Dali::TouchEvent& touch) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SendKeyEvent()
   */
  bool SendKeyEvent(const Dali::KeyEvent& event) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetFocus()
   * @param[in] focused True if web view is focused, false otherwise.
   */
  void SetFocus(bool focused) override;

  /**
   * @copydoc Dali::WebEnginePlugin::UpdateDisplayArea()
   * @brief Update display area.
   * @param[in] displayArea A display area to be updated.
   */
  void UpdateDisplayArea(Dali::Rect< int > displayArea) override;

  /**
   * @copydoc Dali::WebEnginePlugin::EnableVideoHole()
   * @param[in] enabled True if video hole is enabled, false otherwise.
   */
  void EnableVideoHole(bool enabled) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterPageLoadStartedCallback()
   */
  void RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterPageLoadFinishedCallback()
   */
  void RegisterPageLoadFinishedCallback(WebEnginePageLoadCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterPageLoadErrorCallback()
   */
  void RegisterPageLoadErrorCallback(WebEnginePageLoadErrorCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterScrollEdgeReachedCallback()
   */
  void RegisterScrollEdgeReachedCallback(WebEngineScrollEdgeReachedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterUrlChangedCallback()
   */
  void RegisterUrlChangedCallback(WebEngineUrlChangedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterNavigationPolicyDecidedCallback()
   */
  void RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterNewWindowCreatedCallback()
   */
  void RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterFrameRenderedCallback()
   */
  void RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetPlainTextAsynchronously()
   */
  void GetPlainTextAsynchronously(PlainTextReceivedCallback callback) override;

private:
  void UpdateImage(tbm_surface_h buffer);
  void InitWebView(int argc, char** argv);

  static void OnFrameRendered(void* data, Evas_Object*, void* buffer);
  static void OnLoadStarted(void* data, Evas_Object*, void*);
  static void OnLoadFinished(void* data, Evas_Object*, void*);
  static void OnLoadError(void* data, Evas_Object*, void* rawError);
  static void OnConsoleMessage(void*, Evas_Object*, void* eventInfo);
  static void OnEdgeLeft(void* data, Evas_Object*, void*);
  static void OnEdgeRight(void* data, Evas_Object*, void*);
  static void OnEdgeTop(void* data, Evas_Object*, void*);
  static void OnEdgeBottom(void* data, Evas_Object*, void*);
  static void OnUrlChanged(void* data, Evas_Object*, void* newUrl);
  static void OnNavigationPolicyDecided(void* data, Evas_Object*, void* policy);
  static void OnNewWindowCreated(void* data, Evas_Object*, void* out_view);
  static void OnJavaScriptEvaluated(Evas_Object*, const char* result, void* data);
  static void OnJavaScriptInjected(Evas_Object* o, Ewk_Script_Message message);
  static void OnPlainTextReceived(Evas_Object* o, const char* plainText, void* data);

private:
  Dali::NativeImageSourcePtr mDaliImageSrc;
  Evas_Object*               mWebView;
  int                        mWidth;
  int                        mHeight;

  std::unique_ptr<WebEngineSettings>        mWebEngineSettings;
  std::unique_ptr<WebEngineContext>         mWebEngineContext;
  std::unique_ptr<WebEngineCookieManager>   mWebEngineCookieManager;
  std::unique_ptr<WebEngineBackForwardList> mWebEngineBackForwardList;

  WebEnginePageLoadCallback                mLoadStartedCallback;
  WebEnginePageLoadCallback                mLoadFinishedCallback;
  WebEnginePageLoadErrorCallback           mLoadErrorCallback;
  WebEngineScrollEdgeReachedCallback       mScrollEdgeReachedCallback;
  WebEngineUrlChangedCallback              mUrlChangedCallback;
  WebEngineNavigationPolicyDecidedCallback mNavigationPolicyDecidedCallback;
  WebEngineNewWindowCreatedCallback        mNewWindowCreatedCallback;
  WebEngineFrameRenderedCallback           mFrameRenderedCallback;
  PlainTextReceivedCallback                mPlainTextReceivedCallback;
  JavaScriptMessageHandlerCallback         mJavaScriptEvaluatedCallback;

  std::unordered_map<std::string, JavaScriptMessageHandlerCallback> mJavaScriptMessageHandlers;
};
} // namespace Plugin
} // namespace Dali

#endif

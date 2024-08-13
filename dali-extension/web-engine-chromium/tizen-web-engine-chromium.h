#ifndef DALI_TIZEN_WEB_ENGINE_CHROMIUM_H
#define DALI_TIZEN_WEB_ENGINE_CHROMIUM_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/public-api/images/native-image-interface.h>

#include <ewk_view_internal.h>

#include <functional>
#include <memory>
#include <unordered_map>

#include <tbm_surface.h>

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
  void Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneID) override;

  /**
   * @copydoc Dali::WebEnginePlugin::Create()
   */
  void Create(uint32_t width, uint32_t height, uint32_t argc, char** argv) override;

  /**
   * @copydoc Dali::WebEnginePlugin::Destroy()
   */
  void Destroy() override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetSettings()
   */
  Dali::WebEngineSettings& GetSettings() const override;

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
  NativeImageSourcePtr GetNativeImageSource() override;

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
  void LoadHtmlString(const std::string& string) override;

  /**
   * @copydoc Dali::WebEnginePlugin::LoadHtmlStringOverrideCurrentEntry()
   */
  bool LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl) override;

  /**
   * @copydoc Dali::WebEnginePlugin::LoadContents()
   */
  bool LoadContents(const int8_t* contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri) override;

  /**
   * @copydoc Dali::WebEnginePlugin::Reload()
   */
  void Reload() override;

  /**
   * @copydoc Dali::WebEnginePlugin::ReloadWithoutCache()
   */
  bool ReloadWithoutCache() override;

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
   * @copydoc Dali::WebEnginePlugin::SuspendNetworkLoading()
   */
  void SuspendNetworkLoading() override;

  /**
   * @copydoc Dali::WebEnginePlugin::ResumeNetworkLoading()
   */
  void ResumeNetworkLoading() override;

  /**
   * @copydoc Dali::WebEnginePlugin::AddCustomHeader()
   */
  bool AddCustomHeader(const std::string& name, const std::string& value) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RemoveCustomHeader()
   */
  bool RemoveCustomHeader(const std::string& name) override;

  /**
   * @copydoc Dali::WebEnginePlugin::StartInspectorServer()
   */
  uint32_t StartInspectorServer(uint32_t port) override;

  /**
   * @copydoc Dali::WebEnginePlugin::StopInspectorServer()
   */
  bool StopInspectorServer() override;

  /**
   * @copydoc Dali::WebEnginePlugin::ScrollBy()
   */
  void ScrollBy(int32_t deltaX, int32_t deltaY) override;

  /**
   * @copydoc Dali::WebEnginePlugin::ScrollEdgeBy()
   */
  bool ScrollEdgeBy(int32_t deltaX, int32_t deltaY) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetScrollPosition()
   */
  void SetScrollPosition(int32_t x, int32_t y) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetScrollPosition()
   */
  Dali::Vector2 GetScrollPosition() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetScrollSize()
   */
  Dali::Vector2 GetScrollSize() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetContentSize()
   */
  Dali::Vector2 GetContentSize() const override;

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
   * @copydoc Dali::WebEnginePlugin::RegisterJavaScriptAlertCallback()
   */
  void RegisterJavaScriptAlertCallback(JavaScriptAlertCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::JavaScriptAlertReply()
   */
  void JavaScriptAlertReply() override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterJavaScriptConfirmCallback()
   */
  void RegisterJavaScriptConfirmCallback(JavaScriptConfirmCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::JavaScriptConfirmReply()
   */
  void JavaScriptConfirmReply(bool confirmed) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterJavaScriptPromptCallback()
   */
  void RegisterJavaScriptPromptCallback(JavaScriptPromptCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::JavaScriptPromptReply()
   */
  void JavaScriptPromptReply(const std::string& result) override;

  /**
   * @copydoc Dali::WebEnginePlugin::CreateHitTest()
   */
  std::unique_ptr<Dali::WebEngineHitTest> CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode) override;

  /**
   * @copydoc Dali::WebEnginePlugin::CreateHitTestAsynchronously()
   */
  bool CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, WebEngineHitTestCreatedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::ClearHistory()
   */
  void ClearHistory() override;

  /**
   * @copydoc Dali::WebEnginePlugin::ClearAllTilesResources()
   */
  void ClearAllTilesResources() override;

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
  void SetSize(uint32_t width, uint32_t height) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetDocumentBackgroundColor()
   */
  void SetDocumentBackgroundColor(Dali::Vector4 color) override;

  /**
   * @copydoc Dali::WebEnginePlugin::ClearTilesWhenHidden()
   */
  void ClearTilesWhenHidden(bool cleared) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetTileCoverAreaMultiplier()
   */
  void SetTileCoverAreaMultiplier(float multiplier) override;

  /**
   * @copydoc Dali::WebEnginePlugin::EnableCursorByClient()
   */
  void EnableCursorByClient(bool enabled) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetSelectedText()
   */
  std::string GetSelectedText() const override;

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
   */
  void SetFocus(bool focused) override;

  /**
   * @copydoc Dali::WebEnginePlugin::EnableMouseEvents()
   */
  void EnableMouseEvents(bool enabled) override;

  /**
   * @copydoc Dali::WebEnginePlugin::EnableKeyEvents()
   */
  void EnableKeyEvents(bool enabled) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetPageZoomFactor()
   */
  void SetPageZoomFactor(float zoomFactor) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetPageZoomFactor()
   */
  float GetPageZoomFactor() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetTextZoomFactor()
   */
  void SetTextZoomFactor(float zoomFactor) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetTextZoomFactor()
   */
  float GetTextZoomFactor() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetLoadProgressPercentage()
   */
  float GetLoadProgressPercentage() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetScaleFactor()
   */
  void SetScaleFactor(float scaleFactor, Dali::Vector2 point) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetScaleFactor()
   */
  float GetScaleFactor() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::ActivateAccessibility()
   */
  void ActivateAccessibility(bool activated) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetAccessibilityAddress()
   */
  Accessibility::Address GetAccessibilityAddress() override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetVisibility()
   */
  bool SetVisibility(bool visible) override;

  /**
   * @copydoc Dali::WebEnginePlugin::HighlightText()
   */
  bool HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount) override;

  /**
   * @copydoc Dali::WebEnginePlugin::AddDynamicCertificatePath()
   */
  void AddDynamicCertificatePath(const std::string& host, const std::string& certPath) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetScreenshot()
   */
  Dali::PixelData GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetScreenshotAsynchronously()
   */
  bool GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor, ScreenshotCapturedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::CheckVideoPlayingAsynchronously()
   */
  bool CheckVideoPlayingAsynchronously(VideoPlayingCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::UpdateDisplayArea()
   */
  void UpdateDisplayArea(Dali::Rect<int32_t> displayArea) override;

  /**
   * @copydoc Dali::WebEnginePlugin::EnableVideoHole()
   */
  void EnableVideoHole(bool enabled) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SendHoverEvent()
   */
  bool SendHoverEvent(const Dali::HoverEvent& event) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SendWheelEvent()
   */
  bool SendWheelEvent(const Dali::WheelEvent& event) override;

  /**
   * @copydoc Dali::WebEnginePlugin::ExitFullscreen()
   */
  void ExitFullscreen() override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterFrameRenderedCallback()
   */
  void RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterPageLoadStartedCallback()
   */
  void RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterPageLoadInProgressCallback()
   */
  void RegisterPageLoadInProgressCallback(WebEnginePageLoadCallback callback) override;

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
   * @copydoc Dali::WebEnginePlugin::RegisterFormRepostDecidedCallback()
   */
  void RegisterFormRepostDecidedCallback(WebEngineFormRepostDecidedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterConsoleMessageReceivedCallback()
   */
  void RegisterConsoleMessageReceivedCallback(WebEngineConsoleMessageReceivedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterResponsePolicyDecidedCallback()
   */
  void RegisterResponsePolicyDecidedCallback(WebEngineResponsePolicyDecidedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterNavigationPolicyDecidedCallback()
   */
  void RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterNewWindowPolicyDecidedCallback()
   */
  void RegisterNewWindowPolicyDecidedCallback(WebEngineNewWindowPolicyDecidedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterNewWindowCreatedCallback()
   */
  void RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterGeolocationPermissionCallback()
   */
  void RegisterGeolocationPermissionCallback(GeolocationPermissionCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterCertificateConfirmedCallback()
   */
  void RegisterCertificateConfirmedCallback(WebEngineCertificateCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterSslCertificateChangedCallback()
   */
  void RegisterSslCertificateChangedCallback(WebEngineCertificateCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterHttpAuthHandlerCallback()
   */
  void RegisterHttpAuthHandlerCallback(WebEngineHttpAuthHandlerCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterContextMenuShownCallback()
   */
  void RegisterContextMenuShownCallback(WebEngineContextMenuShownCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterContextMenuHiddenCallback()
   */
  void RegisterContextMenuHiddenCallback(WebEngineContextMenuHiddenCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterFullscreenEnteredCallback()
   */
  void RegisterFullscreenEnteredCallback(WebEngineFullscreenEnteredCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterFullscreenExitedCallback()
   */
  void RegisterFullscreenExitedCallback(WebEngineFullscreenExitedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterTextFoundCallback()
   */
  void RegisterTextFoundCallback(WebEngineTextFoundCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetPlainTextAsynchronously()
   */
  void GetPlainTextAsynchronously(PlainTextReceivedCallback callback) override;

private:
  static Dali::PixelData ConvertImageColorSpace(Evas_Object* image);

  static void OnFrameRendered(void* data, Evas_Object*, void* buffer);
  static void OnLoadStarted(void* data, Evas_Object*, void*);
  static void OnLoadInProgress(void* data, Evas_Object*, void*);
  static void OnLoadFinished(void* data, Evas_Object*, void*);
  static void OnLoadError(void* data, Evas_Object*, void* rawError);
  static void OnUrlChanged(void* data, Evas_Object*, void* newUrl);
  static void OnConsoleMessageReceived(void* data, Evas_Object*, void* eventInfo);
  static void OnEdgeLeft(void* data, Evas_Object*, void*);
  static void OnEdgeRight(void* data, Evas_Object*, void*);
  static void OnEdgeTop(void* data, Evas_Object*, void*);
  static void OnEdgeBottom(void* data, Evas_Object*, void*);
  static void OnFormRepostDecided(void* data, Evas_Object*, void* eventInfo);
  static void OnResponsePolicyDecided(void* data, Evas_Object*, void* policy);
  static void OnNavigationPolicyDecided(void* data, Evas_Object*, void* policy);
  static void OnNewWindowPolicyDecided(void* data, Evas_Object*, void* policy);
  static void OnNewWindowCreated(void* data, Evas_Object*, void* out_view);
  static void OnCertificateConfirmed(void* data, Evas_Object*, void* eventInfo);
  static void OnSslCertificateChanged(void* data, Evas_Object*, void* eventInfo);
  static void OnContextMenuShown(void* data, Evas_Object*, void* eventInfo);
  static void OnContextMenuHidden(void* data, Evas_Object*, void* eventInfo);
  static void OnFullscreenEntered(void* data, Evas_Object*, void* eventInfo);
  static void OnFullscreenExited(void* data, Evas_Object*, void* eventInfo);
  static void OnTextFound(void* data, Evas_Object*, void* eventInfo);
  static void OnAuthenticationChallenged(Evas_Object*, Ewk_Auth_Challenge* authChallenge, void* data);
  static void OnJavaScriptEvaluated(Evas_Object* o, const char* result, void* data);
  static void OnJavaScriptInjected(Evas_Object* o, Ewk_Script_Message message);
  static Eina_Bool OnJavaScriptAlert(Evas_Object* o, const char* alert_text, void*);
  static Eina_Bool OnJavaScriptConfirm(Evas_Object* o, const char* message, void*);
  static Eina_Bool OnJavaScriptPrompt(Evas_Object* o, const char* message, const char* default_value, void*);
  static void      OnHitTestCreated(Evas_Object*, int x, int y, int hitTestMode, Ewk_Hit_Test* hitTest, void* data);
  static void      OnScreenshotCaptured(Evas_Object* image, void* data);
  static void      OnVideoPlaying(Evas_Object*, Eina_Bool isPlaying, void* data);
  static void      OnPlainTextReceived(Evas_Object* o, const char* plainText, void* data);
  static Eina_Bool OnGeolocationPermission(Evas_Object*, Ewk_Geolocation_Permission_Request* request, void* data);

  void UpdateImage(tbm_surface_h buffer);
  void InitWebView();
  bool FeedMouseEvent(const TouchEvent& touch);
  bool FeedTouchEvent(const TouchEvent& touch);

  /**
   * @brief Destroy and re-create native image source.
   */
  void ResetDaliImageSource();

  // ewk web view.
  Dali::NativeImageSourcePtr                mDaliImageSrc;
  Evas_Object*                              mWebView;
  uint32_t                                  mWidth;
  uint32_t                                  mHeight;
  std::unique_ptr<WebEngineBackForwardList> mWebEngineBackForwardList;
  std::unique_ptr<WebEngineSettings>        mWebEngineSettings;
  std::unordered_map<std::string, JavaScriptMessageHandlerCallback> mJavaScriptInjectedCallbacks;

  // callback.
  WebEnginePageLoadCallback                mLoadStartedCallback;
  WebEnginePageLoadCallback                mLoadInProgressCallback;
  WebEnginePageLoadCallback                mLoadFinishedCallback;
  WebEnginePageLoadErrorCallback           mLoadErrorCallback;
  WebEngineUrlChangedCallback              mUrlChangedCallback;
  WebEngineScrollEdgeReachedCallback       mScrollEdgeReachedCallback;
  WebEngineFormRepostDecidedCallback       mFormRepostDecidedCallback;
  WebEngineConsoleMessageReceivedCallback  mConsoleMessageReceivedCallback;
  WebEngineResponsePolicyDecidedCallback   mResponsePolicyDecidedCallback;
  WebEngineNavigationPolicyDecidedCallback mNavigationPolicyDecidedCallback;
  WebEngineNewWindowPolicyDecidedCallback  mNewWindowPolicyDecidedCallback;
  WebEngineNewWindowCreatedCallback        mNewWindowCreatedCallback;
  WebEngineFrameRenderedCallback           mFrameRenderedCallback;
  WebEngineCertificateCallback             mCertificateConfirmedCallback;
  WebEngineCertificateCallback             mSslCertificateChangedCallback;
  WebEngineHttpAuthHandlerCallback         mHttpAuthHandlerCallback;
  WebEngineContextMenuShownCallback        mContextMenuShownCallback;
  WebEngineContextMenuHiddenCallback       mContextMenuHiddenCallback;
  WebEngineFullscreenEnteredCallback       mFullscreenEnteredCallback;
  WebEngineFullscreenExitedCallback        mFullscreenExitedCallback;
  WebEngineTextFoundCallback               mTextFoundCallback;
  WebEngineHitTestCreatedCallback          mHitTestCreatedCallback;
  JavaScriptAlertCallback                  mJavaScriptAlertCallback;
  JavaScriptConfirmCallback                mJavaScriptConfirmCallback;
  JavaScriptPromptCallback                 mJavaScriptPromptCallback;
  ScreenshotCapturedCallback               mScreenshotCapturedCallback;
  VideoPlayingCallback                     mVideoPlayingCallback;
  GeolocationPermissionCallback            mGeolocationPermissionCallback;
  PlainTextReceivedCallback                mPlainTextReceivedCallback;
  JavaScriptMessageHandlerCallback         mJavaScriptEvaluatedCallback;
};
} // namespace Plugin
} // namespace Dali

#endif

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

#ifndef DALI_EXTENSION_WEB_ENGINE_LWE_H
#define DALI_EXTENSION_WEB_ENGINE_LWE_H

#include "web-engine-lwe-backend.h"

#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>

#include <memory>
#include <mutex>
#include <set>
#include <string>

namespace Dali
{
namespace Plugin
{
class WebEngineLweSettings;
class WebEngineLweBackForwardList;

/**
 * Platform-independent DALi WebEngine implementation for LWE.
 */
class WebEngineLwe : public Dali::WebEnginePlugin
{
public:
  explicit WebEngineLwe(std::unique_ptr<WebEngineLweBackend> backend);
  ~WebEngineLwe() override;

  WebEngineLwe(const WebEngineLwe&)            = delete;
  WebEngineLwe(WebEngineLwe&&)                 = delete;
  WebEngineLwe& operator=(const WebEngineLwe&) = delete;
  WebEngineLwe& operator=(WebEngineLwe&&)      = delete;

  void Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId) override;
  void Create(uint32_t width, uint32_t height, uint32_t argc, char** argv) override;
  void Destroy() override;
  bool IsIncognito() const override;
  Dali::WebEngineSettings& GetSettings() const override;
  Dali::WebEngineBackForwardList& GetBackForwardList() const override;
  void LoadUrl(const std::string& url) override;
  std::string GetTitle() const override;
  Dali::PixelData GetFavicon() const override;
  Dali::NativeImagePtr GetNativeImage() override;
  void ChangeOrientation(int orientation) override;
  std::string GetUrl() const override;
  void LoadHtmlString(const std::string& htmlString) override;
  bool LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl) override;
  bool LoadContents(const int8_t* contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri) override;
  void Reload() override;
  bool ReloadWithoutCache() override;
  void StopLoading() override;
  void Suspend() override;
  void Resume() override;
  void SuspendNetworkLoading() override;
  void ResumeNetworkLoading() override;
  bool AddCustomHeader(const std::string& name, const std::string& value) override;
  bool RemoveCustomHeader(const std::string& name) override;
  uint32_t StartInspectorServer(uint32_t port) override;
  bool StopInspectorServer() override;
  void ScrollBy(int32_t deltaX, int32_t deltaY) override;
  bool ScrollEdgeBy(int32_t deltaX, int32_t deltaY) override;
  void SetScrollPosition(int32_t x, int32_t y) override;
  Dali::Vector2 GetScrollPosition() const override;
  Dali::Vector2 GetScrollSize() const override;
  Dali::Vector2 GetContentSize() const override;
  bool CanGoForward() override;
  void GoForward() override;
  bool CanGoBack() override;
  void GoBack() override;
  void EvaluateJavaScript(const std::string& script, JavaScriptMessageHandlerCallback resultHandler) override;
  void AddJavaScriptMessageHandler(const std::string& exposedObjectName, JavaScriptMessageHandlerCallback handler) override;
  void AddJavaScriptEntireMessageHandler(const std::string& exposedObjectName, JavaScriptEntireMessageHandlerCallback handler) override;
  void RemoveJavaScriptMessageHandler(const std::string& exposedObjectName) override;
  void RegisterJavaScriptAlertCallback(JavaScriptAlertCallback callback) override;
  void JavaScriptAlertReply() override;
  void RegisterJavaScriptConfirmCallback(JavaScriptConfirmCallback callback) override;
  void JavaScriptConfirmReply(bool confirmed) override;
  void RegisterJavaScriptPromptCallback(JavaScriptPromptCallback callback) override;
  void JavaScriptPromptReply(const std::string& result) override;
  std::unique_ptr<Dali::WebEngineHitTest> CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode) override;
  bool CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, WebEngineHitTestCreatedCallback callback) override;
  void ClearHistory() override;
  void ClearAllTilesResources() override;
  std::string GetUserAgent() const override;
  void SetUserAgent(const std::string& userAgent) override;
  void SetSize(uint32_t width, uint32_t height) override;
  void SetDocumentBackgroundColor(Dali::Vector4 color) override;
  void ClearTilesWhenHidden(bool cleared) override;
  void SetTileCoverAreaMultiplier(float multiplier) override;
  void EnableCursorByClient(bool enabled) override;
  std::string GetSelectedText() const override;
  bool SendTouchEvent(const Dali::TouchEvent& touch) override;
  bool SendKeyEvent(const Dali::KeyEvent& event) override;
  void EnableMouseEvents(bool enabled) override;
  void EnableKeyEvents(bool enabled) override;
  void SetFocus(bool focused) override;
  bool SetImePositionAndAlignment(Dali::Vector2 position, int alignment) override;
  void SetCursorThemeName(const std::string themeName) override;
  void SetPageZoomFactor(float zoomFactor) override;
  float GetPageZoomFactor() const override;
  void SetTextZoomFactor(float zoomFactor) override;
  float GetTextZoomFactor() const override;
  float GetLoadProgressPercentage() const override;
  void SetScaleFactor(float scaleFactor, Dali::Vector2 point) override;
  float GetScaleFactor() const override;
  void ActivateAccessibility(bool activated) override;
  Dali::Devel::Accessibility::Address GetAccessibilityAddress() override;
  bool SetVisibility(bool visible) override;
  bool HighlightText(const std::string& text, FindOption options, uint32_t maxMatchCount) override;
  void AddDynamicCertificatePath(const std::string& host, const std::string& certPath) override;
  Dali::PixelData GetScreenshot(Dali::BoundsInteger viewArea, float scaleFactor) override;
  bool GetScreenshotAsynchronously(Dali::BoundsInteger viewArea, float scaleFactor, ScreenshotCapturedCallback callback) override;
  bool CheckVideoPlayingAsynchronously(VideoPlayingCallback callback) override;
  void RegisterGeolocationPermissionCallback(GeolocationPermissionCallback callback) override;
  void UpdateDisplayArea(Dali::BoundsInteger displayArea) override;
  void EnableVideoHole(bool enabled) override;
  bool SendHoverEvent(const Dali::HoverEvent& event) override;
  bool SendWheelEvent(const Dali::WheelEvent& event) override;
  void ExitFullscreen() override;
  void RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback) override;
  void RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback) override;
  void RegisterPageLoadInProgressCallback(WebEnginePageLoadCallback callback) override;
  void RegisterPageLoadFinishedCallback(WebEnginePageLoadCallback callback) override;
  void RegisterPageLoadErrorCallback(WebEnginePageLoadErrorCallback callback) override;
  void RegisterScrollEdgeReachedCallback(WebEngineScrollEdgeReachedCallback callback) override;
  void RegisterOverScrolledCallback(WebEngineOverScrolledCallback callback) override;
  void RegisterUrlChangedCallback(WebEngineUrlChangedCallback callback) override;
  void RegisterFormRepostDecidedCallback(WebEngineFormRepostDecidedCallback callback) override;
  void RegisterConsoleMessageReceivedCallback(WebEngineConsoleMessageReceivedCallback callback) override;
  void RegisterResponsePolicyDecidedCallback(WebEngineResponsePolicyDecidedCallback callback) override;
  void RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback callback) override;
  void RegisterNewWindowPolicyDecidedCallback(WebEngineNewWindowPolicyDecidedCallback callback) override;
  void RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback callback) override;
  void RegisterCertificateConfirmedCallback(WebEngineCertificateCallback callback) override;
  void RegisterSslCertificateChangedCallback(WebEngineCertificateCallback callback) override;
  void RegisterHttpAuthHandlerCallback(WebEngineHttpAuthHandlerCallback callback) override;
  void RegisterContextMenuShownCallback(WebEngineContextMenuShownCallback callback) override;
  void RegisterContextMenuHiddenCallback(WebEngineContextMenuHiddenCallback callback) override;
  void RegisterFullscreenEnteredCallback(WebEngineFullscreenEnteredCallback callback) override;
  void RegisterFullscreenExitedCallback(WebEngineFullscreenExitedCallback callback) override;
  void RegisterTextFoundCallback(WebEngineTextFoundCallback callback) override;
  void GetPlainTextAsynchronously(PlainTextReceivedCallback callback) override;
  void WebAuthenticationCancel() override;
  void RegisterWebAuthDisplayQRCallback(WebEngineWebAuthDisplayQRCallback callback) override;
  void RegisterWebAuthResponseCallback(WebEngineWebAuthResponseCallback callback) override;
  void RegisterFileChooserRequestedCallback(WebEngineFileChooserRequestedCallback callback) override;
  void RegisterWebProcessCrashedCallback(WebEngineWebProcessCrashedCallback callback) override;
  void RegisterUserMediaPermissionRequestCallback(WebEngineUserMediaPermissionRequestCallback callback) override;
  void RegisterDeviceConnectionChangedCallback(WebEngineDeviceConnectionChangedCallback callback) override;
  void RegisterDeviceListGetCallback(WebEngineDeviceListGetCallback callback) override;
  void FeedMouseWheel(bool yDirection, int step, int x, int y) override;
  void SetVideoHole(bool enabled, bool isWaylandWindow) override;

  static void ClearSharedCache();

private:
  void CompleteCreate(LWE::WebContainer* webContainer);
  void RegisterEngineCallbacks();
  void DispatchToEventThread(WebEngineLweBackend::Task task);
  void UpdateNavigationState(bool canGoBack, bool canGoForward);

  void DispatchMouseDownEvent(float x, float y);
  void DispatchMouseUpEvent(float x, float y);
  void DispatchMouseMoveEvent(float x, float y);

private:
  static std::mutex              sLiveInstancesMutex;
  static std::set<WebEngineLwe*> sLiveInstances;

  std::unique_ptr<WebEngineLweBackend>          mBackend;
  LWE::WebContainer*                            mWebContainer;
  std::unique_ptr<WebEngineLweSettings>          mWebEngineSettings;
  std::unique_ptr<WebEngineLweBackForwardList>   mBackForwardList;

  std::string mUrl;
  bool        mMouseEventsEnabled;
  bool        mKeyEventsEnabled;
  bool        mMouseLeftButtonDown;
  bool        mCanGoBack;
  bool        mCanGoForward;

  WebEngineFrameRenderedCallback mFrameRenderedCallback;
  WebEnginePageLoadCallback      mLoadStartedCallback;
  WebEnginePageLoadCallback      mLoadFinishedCallback;
  WebEnginePageLoadErrorCallback mLoadErrorCallback;
  WebEngineUrlChangedCallback    mUrlChangedCallback;
  JavaScriptAlertCallback        mJavaScriptAlertCallback;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_EXTENSION_WEB_ENGINE_LWE_H

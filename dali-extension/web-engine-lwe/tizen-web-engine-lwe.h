#ifndef DALI_TIZEN_WEB_ENGINE_LWE_H
#define DALI_TIZEN_WEB_ENGINE_LWE_H

/*
 * Copyright (c) 2024 Samsung Electronics Co., Ltd.
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
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-plugin.h>
#include <dali/public-api/adaptor-framework/timer.h>

#include <tbm_dummy_display.h>
#include <tbm_surface.h>
#include <tbm_surface_internal.h>
#include <tbm_surface_queue.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <LWEWebView.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief Implementation of the Tizen WebEngineLWE class which has Tizen
 * platform dependency.
 */
class TizenWebEngineLWE : public Dali::WebEnginePlugin,
                          public Dali::ConnectionTracker
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineLWE();

  /**
   * @brief Destructor.
   */
  virtual ~TizenWebEngineLWE();

  /**
   * @copydoc Dali::WebEnginePlugin::Create()
   */
  void Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetSettings()
   */
  Dali::WebEngineSettings& GetSettings() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetBackForwardList()
   */
  Dali::WebEngineBackForwardList& GetBackForwardList() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::Create()
   */
  void Create(uint32_t width, uint32_t height, uint32_t argc, char** argv) override;

  /**
   * @copydoc Dali::WebEnginePlugin::Destroy()
   */
  void Destroy() override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetNativeImageSource()
   */
  Dali::NativeImageSourcePtr GetNativeImageSource() override;

  /**
   * @copydoc Dali::WebEnginePlugin::ChangeOrientation()
   */
  void ChangeOrientation(int orientation) override;

  /**
   * @copydoc Dali::WebEnginePlugin::LoadUrl()
   */
  void LoadUrl(const std::string& url) override;

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
   * @copydoc Dali::WebEnginePlugin::LoadHTMLString()
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
   * @copydoc Dali::WebEnginePlugin::GoBack()
   */
  void GoBack() override;

  /**
   * @copydoc Dali::WebEnginePlugin::GoForward()
   */
  void GoForward() override;

  /**
   * @copydoc Dali::WebEnginePlugin::CanGoBack()
   */
  bool CanGoBack() override;

  /**
   * @copydoc Dali::WebEnginePlugin::CanGoForward()
   */
  bool CanGoForward() override;

  /**
   * @copydoc Dali::WebEnginePlugin::EvaluateJavaScript()
   */
  void EvaluateJavaScript(const std::string& script, std::function<void(const std::string&)> resultHandler) override;

  /**
   * @copydoc Dali::WebEnginePlugin::AddJavaScriptMessageHandler()
   */
  void AddJavaScriptMessageHandler(const std::string& exposedObjectName, std::function<void(const std::string&)> handler) override;

  /**
   * @copydoc Dali::WebEnginePlugin::AddJavaScriptEntireMessageHandler()
   */
  void AddJavaScriptEntireMessageHandler(const std::string& exposedObjectName, Dali::WebEnginePlugin::JavaScriptEntireMessageHandlerCallback handler) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterJavaScriptAlertCallback()
   */
  void RegisterJavaScriptAlertCallback(Dali::WebEnginePlugin::JavaScriptAlertCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::JavaScriptAlertReply()
   */
  void JavaScriptAlertReply() override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterJavaScriptConfirmCallback()
   */
  void RegisterJavaScriptConfirmCallback(Dali::WebEnginePlugin::JavaScriptConfirmCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::JavaScriptConfirmReply()
   */
  void JavaScriptConfirmReply(bool confirmed) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterJavaScriptPromptCallback()
   */
  void RegisterJavaScriptPromptCallback(Dali::WebEnginePlugin::JavaScriptPromptCallback callback) override;

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
   * @copydoc Dali::WebEnginePlugin::SetImePositionAndAlignment()
   */
  bool SetImePositionAndAlignment(Dali::Vector2 position, int alignment) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetCursorThemeName()
   */
  void SetCursorThemeName(const std::string themeName) override;

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
  bool HighlightText(const std::string& text, FindOption options, uint32_t maxMatchCount) override;

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
   * @copydoc Dali::WebEnginePlugin::RegisterGeolocationPermissionCallback()
   */
  void RegisterGeolocationPermissionCallback(GeolocationPermissionCallback callback) override;

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
   * @copydoc Dali::WebEnginePlugin::RegisterOverScrolledCallback()
   */
  void RegisterOverScrolledCallback(WebEngineOverScrolledCallback callback) override;

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

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterWebAuthDisplayQRCallback()
   */
  void RegisterWebAuthDisplayQRCallback(WebEngineWebAuthDisplayQRCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterWebAuthResponseCallback()
   */
  void RegisterWebAuthResponseCallback(WebEngineWebAuthResponseCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::WebAuthenticationCancel()
   */
  void WebAuthenticationCancel() override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterUserMediaPermissionRequestCallback()
   */
  void RegisterUserMediaPermissionRequestCallback(WebEngineUserMediaPermissionRequestCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterDeviceConnectionChangedCallback()
   */
  void RegisterDeviceConnectionChangedCallback(WebEngineDeviceConnectionChangedCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::RegisterDeviceListGetCallback()
   */
  void RegisterDeviceListGetCallback(WebEngineDeviceListGetCallback callback) override;

  /**
   * @copydoc Dali::WebEnginePlugin::FeedMouseWheel()
   */
  void FeedMouseWheel(bool yDirection, int step, int x, int y) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetVideoHole()
   */
  void SetVideoHole(bool enabled, bool isWaylandWindow) override;

private:
#ifndef OVER_TIZEN_VERSION_9
  void LegacyUpdateBuffer();
#endif

  void DestroyInstance();

  void DispatchMouseDownEvent(float x, float y);

  void DispatchMouseUpEvent(float x, float y);

  void DispatchMouseMoveEvent(float x, float y, bool isLButtonPressed, bool isRButtonPressed);

  void DispatchKeyDownEvent(LWE::KeyValue keyCode);

  void DispatchKeyPressEvent(LWE::KeyValue keyCode);

  void DispatchKeyUpEvent(LWE::KeyValue keyCode);

  void InitRenderingContext();

  void DestroyRenderingContext();

  void InitRenderingSurface();

  void DestroyRenderingSurface();

  void TryRendering();

  void TryUpdateImage(bool needsSync);

  void PrepareLWERendering();

  void InitGL();

  void OnIdle();

  void OnActive();

  void OnFirstRender();

  void UpdateImage(tbm_surface_h image);

private:
  std::string mUrl;

  bool mIsMouseLbuttonDown;
  bool mCanGoBack;
  bool mCanGoForward;
  bool mInDestroyingLWEInstance;

#ifndef OVER_TIZEN_VERSION_9
  size_t   mOutputWidth;
  size_t   mOutputHeight;
  size_t   mOutputStride;
  uint8_t* mOutputBuffer;

  pthread_mutex_t mOutputBufferMutex;
  tbm_surface_h   mTbmSurface;

  std::function<void(LWE::WebContainer*, const LWE::WebContainer::RenderResult&)> mOnRenderedHandler;

  EventThreadCallback mUpdateBufferTrigger;
#endif

  std::function<void(LWE::WebContainer*, LWE::ResourceError)> mOnReceivedError;
  std::function<void(LWE::WebContainer*, const std::string&)> mOnPageFinishedHandler;
  std::function<void(LWE::WebContainer*, const std::string&)> mOnPageStartedHandler;
  std::function<void(LWE::WebContainer*, const std::string&)> mOnLoadResourceHandler;

  LWE::WebContainer*         mWebContainer;
  Dali::NativeImageSourcePtr mDaliImageSrc;

  EGLNativeDisplayType mNativeDisplay;
  EGLDisplay           mEglDisplay;
  EGLConfig            mEglConfig;
  EGLSurface           mEglSurface;
  EGLContext           mEglContext;
  EGLSyncKHR           mEglSync;

  tbm_surface_queue_h mTbmQueue;
  tbm_surface_h       mLastDrawnTbmSurface;
  tbm_surface_h       mIdleTbmSurface;

  std::function<void()> mLWERenderingFunction;
  std::atomic_bool      mLWERenderingRequested;
  std::atomic_bool      mInImageUpdateState;
  std::atomic_bool      mInIdleState;
  std::atomic_bool      mFirstRenderEnded;
  Dali::Signal<void()>  mFirstRenderSignal;

  WebEngineFrameRenderedCallback mFrameRenderedCallback;
  WebEnginePageLoadCallback      mLoadStartedCallback;
  WebEnginePageLoadCallback      mLoadFinishedCallback;
};

} // namespace Plugin
} // namespace Dali

#endif

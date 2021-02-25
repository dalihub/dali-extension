#ifndef DALI_TIZEN_WEB_ENGINE_CHROMIUM_H
#define DALI_TIZEN_WEB_ENGINE_CHROMIUM_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali-toolkit/dali-toolkit.h>
#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>
#include <dali/public-api/images/native-image-interface.h>
#include <functional>

#include <memory>
#include <tbm_surface.h>
#include <unordered_map>

namespace Dali
{
class PixelData;

namespace Plugin
{
class WebViewContainerForDali;

/**
 * @brief The interface class to represent client of corresponding WebView
 * container (WebViewContainerForDali).
 */
class WebViewContainerClient
{
public:
  /**
   * @brief Callback function to be called by WebViewContainer when surface is
   * updated.
   * @param [in] buffer The surface
   */
  virtual void UpdateImage(tbm_surface_h buffer) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when page loading
   * is started.
   */
  virtual void LoadStarted() = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when page loading
   * is in progress.
   */
  virtual void LoadInProgress() = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when page loading
   * is finished.
   */
  virtual void LoadFinished() = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when an error
   * occurs in page loading.
   * @param [in] error The error when loading a page.
   */
  virtual void LoadError(std::shared_ptr<Dali::WebEngineLoadError> error) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when scroll edge
   * is reached.
   * @param [in] edge Scroll edge reached.
   */
  virtual void ScrollEdgeReached(Dali::WebEnginePlugin::ScrollEdge edge) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when form repost
   * policy would be decided.
   * @param [in] decision The decision policy to show warning when form repost.
   */
  virtual void RequestFormRepostDecision(std::shared_ptr<Dali::WebEngineFormRepostDecision> decision) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when url is
   * changed.
   * @param [in] url New url after url is changed.
   */
  virtual void UrlChanged(const std::string& url) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when http request
   * need be intercepted.
   * @param [in] request The http request interceptor.
   */
  virtual void InterceptRequest(std::shared_ptr<Dali::WebEngineRequestInterceptor> interceptor) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when console
   * message is logged out.
   * @param [in] message Message need be logged out.
   */
  virtual void OnConsoleMessage(std::shared_ptr<Dali::WebEngineConsoleMessage> message) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when new window
   * policy would be decided.
   * @param [in] decision Policy need be decided.
   */
  virtual void NewWindowPolicyDecided(std::shared_ptr<Dali::WebEnginePolicyDecision> decision) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when certificate
   * need be confirmed.
   * @param [in] confirm Policy confirm request.
   */
  virtual void CertificateConfirm(std::shared_ptr<Dali::WebEngineCertificate> confirm) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when certificate
   * is changed.
   * @param [in] info Certificate information.
   */
  virtual void SslCertificateChanged(std::shared_ptr<Dali::WebEngineCertificate> info) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when
   * authentication need be confirmed.
   * @param [in] handler Http authentication challenge handler.
   */
  virtual void AuthenticationChallenge(std::shared_ptr<Dali::WebEngineHttpAuthHandler> handler) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when it gets
   * JavaScript evalution result.
   * @param [in] key An unsigned integer representing the result handler
   * @param [in] result Result string from JavaScript runtime
   * @see Dali::Plugin::TizenWebEngineChromium::EvaluateJavaScript
   */
  virtual void RunJavaScriptEvaluationResultHandler(size_t key, const char* result) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when a message
   * handler is called from JavaScript runtime.
   * @param [in] objectName Exposed object name of the message handler
   * @param [in] message Message from JavaScript runtime
   * @see Dali::Plugin::TizenWebEngineChromium::AddJavaScriptMessageHandler
   */
  virtual void RunJavaScriptMessageHandler(const std::string& objectName, const std::string& message) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when javascript
   * alert is running.
   * @param [in] alertText Alert text
   * @return true if a pop-up is shown, false otherwise
   */
  virtual bool JavaScriptAlert(const std::string& alert_text) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when javascript
   * confirm is running.
   * @param [in] message Message that need be confirmed
   * @return true if a pop-up is shown, false otherwise
   */
  virtual bool JavaScriptConfirm(const std::string& message) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when javascript
   * prompt is running.
   * @param [in] message Message that need be promted
   * @param [in] defaultValue Default value that need be promted
   * @return true if a pop-up is shown, false otherwise
   */
  virtual bool JavaScriptPrompt(const std::string& message, const std::string& default_value) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when screen shot
   * is captured.
   * @param [in] pixelData Pixel data captured
   */
  virtual void ScreenshotCaptured(Dali::PixelData pixelData) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when video is
   * playing.
   * @param [in] isPlaying Video is playing or not
   */
  virtual void VideoPlaying(bool isPlaying) = 0;

  /**
   * @brief Callback function to be called by WebViewContainer when requesting
   * geolocation permissions with host and protocol provided.
   * @param [in] host Host that requests geolocation permission
   * @param [in] protocol Protocol that requests geolocation permission
   * @return true if a pop-up is shown, false otherwise
   */
  virtual bool GeolocationPermission(const std::string& host, const std::string& protocol) = 0;
};

/**
 * @brief A class implements WebViewContainerClient and Dali::WebEnginePlugin
 * for tizen chromium.
 */
class TizenWebEngineChromium : public Dali::WebEnginePlugin, public WebViewContainerClient
{
public:
  using JavaScriptCallback = std::function<void(const std::string&)>;

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
  const std::string& GetUrl() override;

  /**
   * @copydoc Dali::WebEnginePlugin::LoadHtmlString()
   */
  void LoadHtmlString(const std::string& string) override;

  /**
   * @copydoc Dali::WebEnginePlugin::LoadHtmlStringOverrideCurrentEntry()
   */
  bool LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri,
                                          const std::string& unreachableUrl) override;

  /**
   * @copydoc Dali::WebEnginePlugin::LoadContents()
   */
  bool LoadContents(const std::string& contents, uint32_t contentSize, const std::string& mimeType,
                    const std::string& encoding, const std::string& baseUri) override;

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
  void ScrollBy(int deltaX, int deltaY) override;

  /**
   * @copydoc Dali::WebEnginePlugin::ScrollEdgeBy()
   */
  bool ScrollEdgeBy(int deltaX, int deltaY) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetScrollPosition()
   */
  void SetScrollPosition(int x, int y) override;

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
  void EvaluateJavaScript(const std::string& script, std::function<void(const std::string&)> resultHandler) override;

  /**
   * @copydoc Dali::WebEnginePlugin::AddJavaScriptMessageHandler()
   */
  void AddJavaScriptMessageHandler(const std::string& exposedObjectName, std::function<void(const std::string&)> handler) override;

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
  const std::string& GetUserAgent() const override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetUserAgent()
   */
  void SetUserAgent(const std::string& userAgent) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SetSize()
   */
  void SetSize(int width, int height) override;

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
  Dali::PixelData GetScreenshot(Dali::Rect<int> viewArea, float scaleFactor) override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetScreenshotAsynchronously()
   */
  bool GetScreenshotAsynchronously(Dali::Rect<int> viewArea, float scaleFactor, ScreenshotCapturedCallback callback) override;

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
  void UpdateDisplayArea(Dali::Rect<int> displayArea) override;

  /**
   * @copydoc Dali::WebEnginePlugin::EnableVideoHole()
   */
  void EnableVideoHole(bool enabled) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SendHoverEvent()
   */
  bool SendHoverEvent(const Dali::HoverEvent &event) override;

  /**
   * @copydoc Dali::WebEnginePlugin::SendWheelEvent()
   */
  bool SendWheelEvent(const Dali::WheelEvent &event) override;

  /**
   * @copydoc Dali::WebEnginePlugin::PageLoadStartedSignal()
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadStartedSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::PageLoadInProgressSignal()
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadInProgressSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::PageLoadFinishedSignal()
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadFinishedSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::PageLoadErrorSignal()
   */
  Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType& PageLoadErrorSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::ScrollEdgeReachedSignal()
   */
  Dali::WebEnginePlugin::WebEngineScrollEdgeReachedSignalType& ScrollEdgeReachedSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::UrlChangedSignal()
   */
  Dali::WebEnginePlugin::WebEngineUrlChangedSignalType& UrlChangedSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::FormRepostDecisionSignal()
   */
  Dali::WebEnginePlugin::WebEngineFormRepostDecisionSignalType& FormRepostDecisionSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::FrameRenderedSignal()
   */
  Dali::WebEnginePlugin::WebEngineFrameRenderedSignalType& FrameRenderedSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::RequestInterceptorSignal()
   */
  Dali::WebEnginePlugin::WebEngineRequestInterceptorSignalType& RequestInterceptorSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::ConsoleMessageSignal()
   */
  Dali::WebEnginePlugin::WebEngineConsoleMessageSignalType& ConsoleMessageSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::PolicyDecisionSignal()
   */
  Dali::WebEnginePlugin::WebEnginePolicyDecisionSignalType& PolicyDecisionSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::CertificateConfirmSignal()
   */
  Dali::WebEnginePlugin::WebEngineCertificateSignalType& CertificateConfirmSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::SslCertificateChangedSignal()
   */
  Dali::WebEnginePlugin::WebEngineCertificateSignalType& SslCertificateChangedSignal() override;

  /**
   * @copydoc Dali::WebEnginePlugin::HttpAuthHandlerSignal()
   */
  Dali::WebEnginePlugin::WebEngineHttpAuthHandlerSignalType& HttpAuthHandlerSignal() override;

  // WebViewContainerClient Interface

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::UpdateImage()
   */
  void UpdateImage(tbm_surface_h buffer) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::LoadStarted()
   */
  void LoadStarted() override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::LoadInProgress()
   */
  void LoadInProgress() override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::LoadFinished()
   */
  void LoadFinished() override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::LoadError()
   */
  void LoadError(std::shared_ptr<Dali::WebEngineLoadError> error) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::ScrollEdgeReached()
   */
  void ScrollEdgeReached(Dali::WebEnginePlugin::ScrollEdge edge) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::RequestFormRepostDecision()
   */
  void RequestFormRepostDecision(std::shared_ptr<Dali::WebEngineFormRepostDecision> decision) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::NewWindowPolicyDecided()
   */
  void NewWindowPolicyDecided(std::shared_ptr<Dali::WebEnginePolicyDecision> policy) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::UrlChanged()
   */
  void UrlChanged(const std::string& url) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::InterceptRequest()
   */
  void InterceptRequest(std::shared_ptr<Dali::WebEngineRequestInterceptor> interceptor) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::OnConsoleMessage()
   */
  void OnConsoleMessage(std::shared_ptr<Dali::WebEngineConsoleMessage> message) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::CertificateConfirm()
   */
  void CertificateConfirm(std::shared_ptr<Dali::WebEngineCertificate> confirm) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::SslCertificateChanged()
   */
  void SslCertificateChanged(std::shared_ptr<Dali::WebEngineCertificate> info) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::AuthenticationChallenge()
   */
  void AuthenticationChallenge(std::shared_ptr<Dali::WebEngineHttpAuthHandler> handler) override;

  /**
   * @copydoc
   * Dali::Plugin::WebViewContainerClient::RunJavaScriptEvaluationResultHandler()
   */
  void RunJavaScriptEvaluationResultHandler(size_t key, const char* result) override;

  /**
   * @copydoc
   * Dali::Plugin::WebViewContainerClient::RunJavaScriptMessageHandler()
   */
  void RunJavaScriptMessageHandler(const std::string& objectName, const std::string& message) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::JavaScriptAlert()
   */
  bool JavaScriptAlert(const std::string& alert_text) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::JavaScriptConfirm()
   */
  bool JavaScriptConfirm(const std::string& message) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::JavaScriptPrompt()
   */
  bool JavaScriptPrompt(const std::string& message, const std::string& default_value) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::ScreenshotCaptured()
   */
  void ScreenshotCaptured(Dali::PixelData pixelData) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::VideoPlaying()
   */
  void VideoPlaying(bool isPlaying) override;

  /**
   * @copydoc Dali::Plugin::WebViewContainerClient::GeolocationPermission()
   */
  bool GeolocationPermission(const std::string& host, const std::string& protocol) override;

private:
  WebViewContainerForDali*   mWebViewContainer;
  Dali::NativeImageSourcePtr mDaliImageSrc;
  std::string                mUrl;
  size_t                     mJavaScriptEvaluationCount;

  Dali::WebEnginePlugin::WebEnginePageLoadSignalType           mLoadStartedSignal;
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType           mLoadInProgressSignal;
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType           mLoadFinishedSignal;
  Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType      mLoadErrorSignal;
  Dali::WebEnginePlugin::WebEngineUrlChangedSignalType         mUrlChangedSignal;
  Dali::WebEnginePlugin::WebEngineScrollEdgeReachedSignalType  mScrollEdgeReachedSignal;
  Dali::WebEnginePlugin::WebEngineFormRepostDecisionSignalType mFormRepostDecisionSignal;
  Dali::WebEnginePlugin::WebEngineFrameRenderedSignalType      mFrameRenderedSignal;
  Dali::WebEnginePlugin::WebEngineRequestInterceptorSignalType mRequestInterceptorSignal;
  Dali::WebEnginePlugin::WebEngineConsoleMessageSignalType     mConsoleMessageSignal;
  Dali::WebEnginePlugin::WebEnginePolicyDecisionSignalType     mPolicyDecisionSignal;
  Dali::WebEnginePlugin::WebEngineCertificateSignalType        mCertificateConfirmSignal;
  Dali::WebEnginePlugin::WebEngineCertificateSignalType        mSslCertificateChangedSignal;
  Dali::WebEnginePlugin::WebEngineHttpAuthHandlerSignalType    mHttpAuthHandlerSignal;

  std::unordered_map<size_t, JavaScriptCallback>               mJavaScriptEvaluationResultHandlers;
  std::unordered_map<std::string, JavaScriptCallback>          mJavaScriptMessageHandlers;

  Dali::WebEnginePlugin::JavaScriptAlertCallback       mJavaScriptAlertCallback;
  Dali::WebEnginePlugin::JavaScriptConfirmCallback     mJavaScriptConfirmCallback;
  Dali::WebEnginePlugin::JavaScriptPromptCallback      mJavaScriptPromptCallback;
  Dali::WebEnginePlugin::ScreenshotCapturedCallback    mScreenshotCapturedCallback;
  Dali::WebEnginePlugin::VideoPlayingCallback          mVideoPlayingCallback;
  Dali::WebEnginePlugin::GeolocationPermissionCallback mGeolocationPermissionCallback;
};
} // namespace Plugin
} // namespace Dali

#endif

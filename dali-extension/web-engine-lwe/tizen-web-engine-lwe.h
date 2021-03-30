#ifndef DALI_TIZEN_WEB_ENGINE_LWE_H
#define DALI_TIZEN_WEB_ENGINE_LWE_H

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
#include <LWEWebView.h>
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <list>
#include <string.h>
#include <vector>

#ifndef DALI_USE_TBMSURFACE
#define DALI_USE_TBMSURFACE
#endif

#ifdef DALI_USE_TBMSURFACE
#include <tbm_surface.h>
#else
#include <dali/public-api/images/buffer-image.h>
#endif

namespace Dali
{
class PixelData;

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
  void Create(int width, int height, const std::string& locale, const std::string& timezoneId) override;

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
   * @copydoc Dali::WebEnginePlugin::Create()
   */
  void Create(int width, int height, int argc, char** argv) override;

  /**
   * @copydoc Dali::WebEnginePlugin::Destroy()
   */
  void Destroy() override;

  /**
   * @copydoc Dali::WebEnginePlugin::GetNativeImageSource()
   */
  Dali::NativeImageInterfacePtr GetNativeImageSource() override;

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
  const std::string& GetUrl() override;

  /**
   * @copydoc Dali::WebEnginePlugin::LoadHTMLString()
   */
  void LoadHtmlString(const std::string& string) override;

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
   * @brief Update display area.
   * @param[in] displayArea A display area to be updated.
   */
  void UpdateDisplayArea(Dali::Rect<int> displayArea) override;

  /**
   * @copydoc Dali::WebEnginePlugin::EnableVideoHole()
   * @param[in] enabled True if video hole is enabled, false otherwise.
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
   * @copydoc Dali::WebEnginePlugin::PageLoadStartedSignal()
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadStartedSignal() override
  {
    return mPageLoadStartedSignal;
  }

  /**
   * @copydoc Dali::WebEnginePlugin::PageLoadInProgressSignal()
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadInProgressSignal() override
  {
    return mPageLoadInProgressSignal;
  }

  /**
   * @copydoc Dali::WebEnginePlugin::PageLoadFinishedSignal()
   */
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType& PageLoadFinishedSignal() override
  {
    return mPageLoadFinishedSignal;
  }

  /**
   * @copydoc Dali::WebEnginePlugin::PageLoadErrorSignal()
   */
  Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType& PageLoadErrorSignal() override
  {
    return mPageLoadErrorSignal;
  }

  /**
   * @copydoc Dali::WebEnginePlugin::ScrollEdgeReachedSignal()
   */
  Dali::WebEnginePlugin::WebEngineScrollEdgeReachedSignalType& ScrollEdgeReachedSignal() override
  {
    return mScrollEdgeReachedSignal;
  }

  /**
   * @copydoc Dali::WebEnginePlugin::UrlChangedSignal()
   */
  Dali::WebEnginePlugin::WebEngineUrlChangedSignalType& UrlChangedSignal() override
  {
    return mUrlChangedSignal;
  }

  /**
   * @copydoc Dali::WebEnginePlugin::FormRepostDecisionSignal()
   */
  Dali::WebEnginePlugin::WebEngineFormRepostDecisionSignalType& FormRepostDecisionSignal() override
  {
    return mFormRepostDecisionSignal;
  }

  /**
   * @copydoc Dali::WebEnginePlugin::FrameRenderedSignal()
   */
  Dali::WebEnginePlugin::WebEngineFrameRenderedSignalType& FrameRenderedSignal() override
  {
    return mFrameRenderedSignal;
  }

private:
  void UpdateBuffer();

  void DestroyInstance();

  void DispatchMouseDownEvent(float x, float y);

  void DispatchMouseUpEvent(float x, float y);

  void DispatchMouseMoveEvent(float x, float y, bool isLButtonPressed, bool isRButtonPressed);

  void DispatchKeyDownEvent(LWE::KeyValue keyCode);

  void DispatchKeyPressEvent(LWE::KeyValue keyCode);

  void DispatchKeyUpEvent(LWE::KeyValue keyCode);

private:
  std::string mUrl;
  size_t      mOutputWidth;
  size_t      mOutputHeight;
  size_t      mOutputStride;
  uint8_t*    mOutputBuffer;
  bool mIsMouseLbuttonDown;
  bool mCanGoBack;
  bool mCanGoForward;
  pthread_mutex_t    mOutputBufferMutex;
  LWE::WebContainer* mWebContainer;
#ifdef DALI_USE_TBMSURFACE
  tbm_surface_h              mTbmSurface;
  Dali::NativeImageSourcePtr mNativeImageSourcePtr;
#else
  Dali::BufferImage          mBufferImage;
#endif

  std::function<void(LWE::WebContainer *, const LWE::WebContainer::RenderResult&)> mOnRenderedHandler;
  std::function<void(LWE::WebContainer *, LWE::ResourceError)> mOnReceivedError;
  std::function<void(LWE::WebContainer *, const std::string&)> mOnPageFinishedHandler;
  std::function<void(LWE::WebContainer *, const std::string&)> mOnPageStartedHandler;
  std::function<void(LWE::WebContainer *, const std::string&)> mOnLoadResourceHandler;

  EventThreadCallback mUpdateBufferTrigger;

  Dali::WebEnginePlugin::WebEnginePageLoadSignalType           mPageLoadStartedSignal;
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType           mPageLoadInProgressSignal;
  Dali::WebEnginePlugin::WebEnginePageLoadSignalType           mPageLoadFinishedSignal;
  Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType      mPageLoadErrorSignal;
  Dali::WebEnginePlugin::WebEngineScrollEdgeReachedSignalType  mScrollEdgeReachedSignal;
  Dali::WebEnginePlugin::WebEngineUrlChangedSignalType         mUrlChangedSignal;
  Dali::WebEnginePlugin::WebEngineFormRepostDecisionSignalType mFormRepostDecisionSignal;
  Dali::WebEnginePlugin::WebEngineFrameRenderedSignalType      mFrameRenderedSignal;
};

} // namespace Plugin
} // namespace Dali

#endif

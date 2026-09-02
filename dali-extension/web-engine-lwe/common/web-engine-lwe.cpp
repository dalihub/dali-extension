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

#include "web-engine-lwe.h"

#include "web-engine-lwe-back-forward-list.h"
#include "web-engine-lwe-input.h"
#include "web-engine-lwe-load-error.h"
#include "web-engine-lwe-settings.h"

#include <LWEWebView.h>

#include <dali/integration-api/debug.h>
#include <dali/public-api/events/hover-event.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/touch-event.h>
#include <dali/public-api/events/wheel-event.h>
#include <dali/public-api/images/pixel-data.h>

#include <utility>

namespace Dali
{
namespace Plugin
{
namespace
{
class NullWebEngineSettings : public Dali::WebEngineSettings
{
public:
  void AllowMixedContents(bool) override
  {
  }

  void EnableSpatialNavigation(bool) override
  {
  }

  uint32_t GetDefaultFontSize() const override
  {
    return 0u;
  }

  void SetDefaultFontSize(uint32_t) override
  {
  }

  void EnableWebSecurity(bool) override
  {
  }

  void EnableCacheBuilder(bool) override
  {
  }

  void UseScrollbarThumbFocusNotifications(bool) override
  {
  }

  void EnableDoNotTrack(bool) override
  {
  }

  void AllowFileAccessFromExternalUrl(bool) override
  {
  }

  bool IsJavaScriptEnabled() const override
  {
    return false;
  }

  void EnableJavaScript(bool) override
  {
  }

  bool IsAutoFittingEnabled() const override
  {
    return false;
  }

  void EnableAutoFitting(bool) override
  {
  }

  bool ArePluginsEnabled() const override
  {
    return false;
  }

  void EnablePlugins(bool) override
  {
  }

  bool IsPrivateBrowsingEnabled() const override
  {
    return false;
  }

  void EnablePrivateBrowsing(bool) override
  {
  }

  bool IsLinkMagnifierEnabled() const override
  {
    return false;
  }

  void EnableLinkMagnifier(bool) override
  {
  }

  bool IsKeypadWithoutUserActionUsed() const override
  {
    return false;
  }

  void UseKeypadWithoutUserAction(bool) override
  {
  }

  bool IsAutofillPasswordFormEnabled() const override
  {
    return false;
  }

  void EnableAutofillPasswordForm(bool) override
  {
  }

  bool IsFormCandidateDataEnabled() const override
  {
    return false;
  }

  void EnableFormCandidateData(bool) override
  {
  }

  bool IsTextSelectionEnabled() const override
  {
    return false;
  }

  void EnableTextSelection(bool) override
  {
  }

  bool IsTextAutosizingEnabled() const override
  {
    return false;
  }

  void EnableTextAutosizing(bool) override
  {
  }

  bool IsArrowScrollEnabled() const override
  {
    return false;
  }

  void EnableArrowScroll(bool) override
  {
  }

  bool IsClipboardEnabled() const override
  {
    return false;
  }

  void EnableClipboard(bool) override
  {
  }

  bool IsImePanelEnabled() const override
  {
    return false;
  }

  void EnableImePanel(bool) override
  {
  }

  void AllowScriptsOpenWindows(bool) override
  {
  }

  bool AreImagesLoadedAutomatically() const override
  {
    return false;
  }

  void AllowImagesLoadAutomatically(bool) override
  {
  }

  std::string GetDefaultTextEncodingName() const override
  {
    return {};
  }

  void SetDefaultTextEncodingName(const std::string&) override
  {
  }

  bool SetViewportMetaTag(bool) override
  {
    return false;
  }

  bool SetForceZoom(bool) override
  {
    return false;
  }

  bool IsZoomForced() const override
  {
    return false;
  }

  bool SetTextZoomEnabled(bool) override
  {
    return false;
  }

  bool IsTextZoomEnabled() const override
  {
    return false;
  }

  void SetExtraFeature(const std::string&, bool) override
  {
  }

  bool IsExtraFeatureEnabled(const std::string&) const override
  {
    return false;
  }

  bool SetExtraFeatureValue(const std::string&, const std::string&) override
  {
    return false;
  }

  std::string GetExtraFeatureValue(const std::string&) const override
  {
    return {};
  }

  void SetImeStyle(int) override
  {
  }

  int GetImeStyle() const override
  {
    return 0;
  }

  void SetDefaultAudioInputDevice(const std::string&) const override
  {
  }

  void EnableDragAndDrop(bool) override
  {
  }
};

template<typename Callback, typename... Args>
void ExecuteCallback(const Callback& callback, Args&&... args)
{
  if(callback)
  {
    callback(std::forward<Args>(args)...);
  }
}
} // unnamed namespace

std::mutex              WebEngineLwe::sLiveInstancesMutex;
std::set<WebEngineLwe*> WebEngineLwe::sLiveInstances;

WebEngineLwe::WebEngineLwe(std::unique_ptr<WebEngineLweBackend> backend)
: mBackend(std::move(backend)),
  mWebContainer(nullptr),
  mBackForwardList(std::make_unique<WebEngineLweBackForwardList>()),
  mMouseEventsEnabled(true),
  mKeyEventsEnabled(true),
  mMouseLeftButtonDown(false),
  mCanGoBack(false),
  mCanGoForward(false)
{
  DALI_ASSERT_ALWAYS(mBackend && "LWE platform backend is missing");
}

WebEngineLwe::~WebEngineLwe()
{
  Destroy();
}

void WebEngineLwe::Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneId)
{
  Destroy();
  mBackend->SetFrameRenderedCallback([this]()
  {
    ExecuteCallback(mFrameRenderedCallback);
  });
  CompleteCreate(mBackend->Create(width, height, locale, timezoneId));
}

void WebEngineLwe::Create(uint32_t width, uint32_t height, uint32_t argc, char** argv)
{
  Destroy();
  mBackend->SetFrameRenderedCallback([this]()
  {
    ExecuteCallback(mFrameRenderedCallback);
  });
  CompleteCreate(mBackend->Create(width, height, argc, argv));
}

void WebEngineLwe::CompleteCreate(LWE::WebContainer* webContainer)
{
  DALI_ASSERT_ALWAYS(webContainer && "Failed to create LWE WebContainer");

  mWebContainer      = webContainer;
  mWebEngineSettings = std::make_unique<WebEngineLweSettings>(mWebContainer);
  mUrl               = "about:blank";
  mCanGoBack         = false;
  mCanGoForward      = false;

  {
    std::lock_guard<std::mutex> lock(sLiveInstancesMutex);
    sLiveInstances.insert(this);
  }

  RegisterEngineCallbacks();
}

void WebEngineLwe::Destroy()
{
  if(!mWebContainer)
  {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(sLiveInstancesMutex);
    sLiveInstances.erase(this);
  }

  mBackend->SetFrameRenderedCallback({});
  mWebEngineSettings.reset();
  mWebContainer = nullptr;
  mBackend->Destroy();
  mUrl.clear();
  mCanGoBack           = false;
  mCanGoForward        = false;
  mMouseLeftButtonDown = false;
}

void WebEngineLwe::ClearSharedCache()
{
  std::lock_guard<std::mutex> lock(sLiveInstancesMutex);
  if(!sLiveInstances.empty())
  {
    WebEngineLwe* instance = *sLiveInstances.begin();
    if(instance->mWebContainer)
    {
      instance->mWebContainer->ClearCache();
    }
  }
}

void WebEngineLwe::DispatchToEventThread(WebEngineLweBackend::Task task)
{
  mBackend->DispatchToEventThread(std::move(task));
}

void WebEngineLwe::UpdateNavigationState(bool canGoBack, bool canGoForward)
{
  mCanGoBack    = canGoBack;
  mCanGoForward = canGoForward;
}

void WebEngineLwe::RegisterEngineCallbacks()
{
  mWebContainer->RegisterOnReceivedErrorHandler(
    [this](LWE::WebContainer* container, LWE::ResourceError error)
  {
    // LWE reports failures for subresources as well as main-frame navigation.
    const bool        canGoBack        = container->CanGoBack();
    const bool        canGoForward     = container->CanGoForward();
    const std::string errorUrl         = error.GetUrl();
    const std::string errorDescription = error.GetDescription();
    const int         errorCode        = error.GetErrorCode();
    DispatchToEventThread([this, errorUrl, errorDescription, errorCode, canGoBack, canGoForward]()
    {
      if(!mWebContainer)
      {
        return;
      }
      UpdateNavigationState(canGoBack, canGoForward);
      if(mLoadErrorCallback)
      {
        mLoadErrorCallback(std::make_unique<WebEngineLweLoadError>(errorUrl, errorDescription, errorCode));
      }
    });
  });

  mWebContainer->RegisterOnPageStartedHandler(
    [this](LWE::WebContainer* container, const std::string& url)
  {
    const bool canGoBack    = container->CanGoBack();
    const bool canGoForward = container->CanGoForward();
    DispatchToEventThread([this, url, canGoBack, canGoForward]()
    {
      if(!mWebContainer)
      {
        return;
      }
      mUrl = url;
      UpdateNavigationState(canGoBack, canGoForward);
      DALI_LOG_RELEASE_INFO("#LoadStarted : %s\n", url.c_str());
      ExecuteCallback(mUrlChangedCallback, url);
      ExecuteCallback(mLoadStartedCallback, url);
    });
  });

  mWebContainer->RegisterOnPageLoadedHandler(
    [this](LWE::WebContainer* container, const std::string& url)
  {
    const bool canGoBack    = container->CanGoBack();
    const bool canGoForward = container->CanGoForward();
    DispatchToEventThread([this, url, canGoBack, canGoForward]()
    {
      if(!mWebContainer)
      {
        return;
      }
      mUrl = url;
      UpdateNavigationState(canGoBack, canGoForward);
      DALI_LOG_RELEASE_INFO("#LoadFinished : %s\n", url.c_str());
      ExecuteCallback(mLoadFinishedCallback, url);
    });
  });

  mWebContainer->RegisterShowAlertHandler(
    [this](LWE::WebContainer*, const std::string&, const std::string& message)
  {
    DispatchToEventThread([this, message]()
    {
      if(mWebContainer)
      {
        ExecuteCallback(mJavaScriptAlertCallback, message);
      }
    });
  });
}

bool WebEngineLwe::IsIncognito() const
{
  return false;
}

Dali::WebEngineSettings& WebEngineLwe::GetSettings() const
{
  if(mWebEngineSettings)
  {
    return *mWebEngineSettings;
  }
  static NullWebEngineSettings settings;
  return settings;
}

Dali::WebEngineBackForwardList& WebEngineLwe::GetBackForwardList() const
{
  return *mBackForwardList;
}

void WebEngineLwe::LoadUrl(const std::string& url)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mUrl = url;
  mWebContainer->LoadURL(url);
}

std::string WebEngineLwe::GetTitle() const
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mWebContainer->GetTitle();
}

Dali::PixelData WebEngineLwe::GetFavicon() const
{
  return {};
}

Dali::NativeImagePtr WebEngineLwe::GetNativeImage()
{
  return mBackend->GetNativeImage();
}

void WebEngineLwe::ChangeOrientation(int)
{
}

std::string WebEngineLwe::GetUrl() const
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mUrl;
}

void WebEngineLwe::LoadHtmlString(const std::string& htmlString)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mUrl = "about:blank";
  mWebContainer->LoadData(htmlString);
}

bool WebEngineLwe::LoadHtmlStringOverrideCurrentEntry(const std::string&, const std::string&, const std::string&)
{
  return false;
}

bool WebEngineLwe::LoadContents(const int8_t*, uint32_t, const std::string&, const std::string&, const std::string&)
{
  return false;
}

void WebEngineLwe::Reload()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->Reload();
}

bool WebEngineLwe::ReloadWithoutCache()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->ClearCache();
  mWebContainer->Reload();
  return true;
}

void WebEngineLwe::StopLoading()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->StopLoading();
}

void WebEngineLwe::Suspend()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->Pause();
}

void WebEngineLwe::Resume()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->Resume();
}

void WebEngineLwe::SuspendNetworkLoading()
{
}

void WebEngineLwe::ResumeNetworkLoading()
{
}

bool WebEngineLwe::AddCustomHeader(const std::string&, const std::string&)
{
  return false;
}

bool WebEngineLwe::RemoveCustomHeader(const std::string&)
{
  return false;
}

uint32_t WebEngineLwe::StartInspectorServer(uint32_t)
{
  return 0u;
}

bool WebEngineLwe::StopInspectorServer()
{
  return false;
}

void WebEngineLwe::ScrollBy(int32_t deltaX, int32_t deltaY)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->ScrollBy(deltaX, deltaY);
}

bool WebEngineLwe::ScrollEdgeBy(int32_t, int32_t)
{
  return false;
}

void WebEngineLwe::SetScrollPosition(int32_t x, int32_t y)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->ScrollTo(x, y);
}

Dali::Vector2 WebEngineLwe::GetScrollPosition() const
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return Dali::Vector2(static_cast<float>(mWebContainer->GetScrollX()), static_cast<float>(mWebContainer->GetScrollY()));
}

Dali::Vector2 WebEngineLwe::GetScrollSize() const
{
  return Dali::Vector2::ZERO;
}

Dali::Vector2 WebEngineLwe::GetContentSize() const
{
  return Dali::Vector2::ZERO;
}

bool WebEngineLwe::CanGoForward()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mCanGoForward;
}

void WebEngineLwe::GoForward()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->GoForward();
}

bool WebEngineLwe::CanGoBack()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mCanGoBack;
}

void WebEngineLwe::GoBack()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->GoBack();
}

void WebEngineLwe::EvaluateJavaScript(const std::string& script, JavaScriptMessageHandlerCallback resultHandler)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->EvaluateJavaScript(script, [this, resultHandler](const std::string& result)
  {
    DispatchToEventThread([this, resultHandler, result]()
    {
      if(mWebContainer)
      {
        ExecuteCallback(resultHandler, result);
      }
    });
  });
}

void WebEngineLwe::AddJavaScriptMessageHandler(const std::string& exposedObjectName, JavaScriptMessageHandlerCallback handler)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->AddJavaScriptInterface(exposedObjectName, "postMessage", [this, handler](const std::string& message) -> std::string
  {
    DispatchToEventThread([this, handler, message]()
    {
      if(mWebContainer)
      {
        ExecuteCallback(handler, message);
      }
    });
    return {};
  });
}

void WebEngineLwe::AddJavaScriptEntireMessageHandler(const std::string& exposedObjectName, JavaScriptEntireMessageHandlerCallback handler)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->AddJavaScriptInterface(exposedObjectName, "postMessage", [this, exposedObjectName, handler](const std::string& message) -> std::string
  {
    DispatchToEventThread([this, exposedObjectName, handler, message]()
    {
      if(mWebContainer)
      {
        ExecuteCallback(handler, exposedObjectName, message);
      }
    });
    return {};
  });
}

void WebEngineLwe::RemoveJavaScriptMessageHandler(const std::string& exposedObjectName)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->RemoveJavascriptInterface(exposedObjectName, "postMessage");
}

void WebEngineLwe::RegisterJavaScriptAlertCallback(JavaScriptAlertCallback callback)
{
  mJavaScriptAlertCallback = std::move(callback);
}

void WebEngineLwe::JavaScriptAlertReply()
{
}

void WebEngineLwe::RegisterJavaScriptConfirmCallback(JavaScriptConfirmCallback)
{
}

void WebEngineLwe::JavaScriptConfirmReply(bool)
{
}

void WebEngineLwe::RegisterJavaScriptPromptCallback(JavaScriptPromptCallback)
{
}

void WebEngineLwe::JavaScriptPromptReply(const std::string&)
{
}

std::unique_ptr<Dali::WebEngineHitTest> WebEngineLwe::CreateHitTest(int32_t, int32_t, Dali::WebEngineHitTest::HitTestMode)
{
  return nullptr;
}

bool WebEngineLwe::CreateHitTestAsynchronously(int32_t, int32_t, Dali::WebEngineHitTest::HitTestMode, WebEngineHitTestCreatedCallback)
{
  return false;
}

void WebEngineLwe::ClearHistory()
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->ClearHistory();
  UpdateNavigationState(false, false);
}

void WebEngineLwe::ClearAllTilesResources()
{
}

std::string WebEngineLwe::GetUserAgent() const
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mWebContainer->GetUserAgentString();
}

void WebEngineLwe::SetUserAgent(const std::string& userAgent)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->SetUserAgentString(userAgent);
}

void WebEngineLwe::SetSize(uint32_t width, uint32_t height)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mBackend->SetSize(width, height);
}

void WebEngineLwe::SetDocumentBackgroundColor(Dali::Vector4 color)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  DALI_ASSERT_ALWAYS(mWebEngineSettings);
  mWebEngineSettings->SetBaseBackgroundColor(color);
}

void WebEngineLwe::ClearTilesWhenHidden(bool)
{
}

void WebEngineLwe::SetTileCoverAreaMultiplier(float)
{
}

void WebEngineLwe::EnableCursorByClient(bool)
{
}

std::string WebEngineLwe::GetSelectedText() const
{
  return {};
}

void WebEngineLwe::DispatchMouseDownEvent(float x, float y)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->DispatchMouseDownEvent(LWE::MouseButtonValue::LeftButton,
                                        LWE::MouseButtonsValue::LeftButtonDown,
                                        x,
                                        y);
  mMouseLeftButtonDown = true;
}

void WebEngineLwe::DispatchMouseUpEvent(float x, float y)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->DispatchMouseUpEvent(LWE::MouseButtonValue::NoButton,
                                      LWE::MouseButtonsValue::NoButtonDown,
                                      x,
                                      y);
  mMouseLeftButtonDown = false;
}

void WebEngineLwe::DispatchMouseMoveEvent(float x, float y)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->DispatchMouseMoveEvent(mMouseLeftButtonDown ? LWE::MouseButtonValue::LeftButton : LWE::MouseButtonValue::NoButton,
                                        mMouseLeftButtonDown ? LWE::MouseButtonsValue::LeftButtonDown : LWE::MouseButtonsValue::NoButtonDown,
                                        x,
                                        y);
}

bool WebEngineLwe::SendTouchEvent(const Dali::TouchEvent& touch)
{
  if(!mMouseEventsEnabled || touch.GetPointCount() != 1u)
  {
    return false;
  }

  const Dali::Vector2& position = touch.GetLocalPosition(0u);
  switch(touch.GetState(0u))
  {
    case Dali::PointState::DOWN:
      DispatchMouseDownEvent(position.x, position.y);
      break;
    case Dali::PointState::UP:
    case Dali::PointState::LEAVE:
    case Dali::PointState::INTERRUPTED:
      DispatchMouseUpEvent(position.x, position.y);
      break;
    case Dali::PointState::MOTION:
      DispatchMouseMoveEvent(position.x, position.y);
      break;
    default:
      break;
  }
  return false;
}

bool WebEngineLwe::SendKeyEvent(const Dali::KeyEvent& event)
{
  if(!mKeyEventsEnabled)
  {
    return false;
  }

  const LWE::KeyValue keyValue = ToLweKeyValue(event);
  if(event.GetState() == Dali::KeyEvent::DOWN)
  {
    mWebContainer->DispatchKeyDownEvent(keyValue);
    mWebContainer->DispatchKeyPressEvent(keyValue);
  }
  else if(event.GetState() == Dali::KeyEvent::UP)
  {
    mWebContainer->DispatchKeyUpEvent(keyValue);
  }
  return false;
}

void WebEngineLwe::EnableMouseEvents(bool enabled)
{
  mMouseEventsEnabled = enabled;
}

void WebEngineLwe::EnableKeyEvents(bool enabled)
{
  mKeyEventsEnabled = enabled;
}

void WebEngineLwe::SetFocus(bool focused)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  if(focused)
  {
    mWebContainer->Focus();
  }
  else
  {
    mWebContainer->Blur();
  }
}

bool WebEngineLwe::SetImePositionAndAlignment(Dali::Vector2, int)
{
  return false;
}

void WebEngineLwe::SetCursorThemeName(const std::string)
{
}

void WebEngineLwe::SetPageZoomFactor(float zoomFactor)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->SetDevicePixelRatio(zoomFactor);
}

float WebEngineLwe::GetPageZoomFactor() const
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  return mWebContainer->GetDevicePixelRatio();
}

void WebEngineLwe::SetTextZoomFactor(float)
{
}

float WebEngineLwe::GetTextZoomFactor() const
{
  return 0.0f;
}

float WebEngineLwe::GetLoadProgressPercentage() const
{
  return 0.0f;
}

void WebEngineLwe::SetScaleFactor(float, Dali::Vector2)
{
}

float WebEngineLwe::GetScaleFactor() const
{
  return 0.0f;
}

void WebEngineLwe::ActivateAccessibility(bool)
{
}

Dali::Devel::Accessibility::Address WebEngineLwe::GetAccessibilityAddress()
{
  return {};
}

bool WebEngineLwe::SetVisibility(bool visible)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  if(visible)
  {
    mWebContainer->Resume();
  }
  else
  {
    mWebContainer->Pause();
  }
  return true;
}

bool WebEngineLwe::HighlightText(const std::string&, FindOption, uint32_t)
{
  return false;
}

void WebEngineLwe::AddDynamicCertificatePath(const std::string&, const std::string&)
{
}

Dali::PixelData WebEngineLwe::GetScreenshot(Dali::BoundsInteger, float)
{
  return {};
}

bool WebEngineLwe::GetScreenshotAsynchronously(Dali::BoundsInteger, float, ScreenshotCapturedCallback)
{
  return false;
}

bool WebEngineLwe::CheckVideoPlayingAsynchronously(VideoPlayingCallback)
{
  return false;
}

void WebEngineLwe::RegisterGeolocationPermissionCallback(GeolocationPermissionCallback)
{
}

void WebEngineLwe::UpdateDisplayArea(Dali::BoundsInteger displayArea)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mBackend->UpdateDisplayArea(static_cast<uint32_t>(displayArea.width), static_cast<uint32_t>(displayArea.height));
}

void WebEngineLwe::EnableVideoHole(bool)
{
}

bool WebEngineLwe::SendHoverEvent(const Dali::HoverEvent& event)
{
  if(!mMouseEventsEnabled || event.GetPointCount() == 0u)
  {
    return false;
  }

  if(event.GetState(0u) == Dali::PointState::MOTION)
  {
    const Dali::Vector2& position = event.GetScreenPosition(0u);
    DispatchMouseMoveEvent(position.x, position.y);
  }
  return false;
}

bool WebEngineLwe::SendWheelEvent(const Dali::WheelEvent& event)
{
  if(!mMouseEventsEnabled)
  {
    return false;
  }

  DALI_ASSERT_ALWAYS(mWebContainer);
  const Dali::Vector2& point = event.GetPoint();
  mWebContainer->DispatchMouseWheelEvent(point.x, point.y, event.GetDelta());
  return false;
}

void WebEngineLwe::ExitFullscreen()
{
}

void WebEngineLwe::RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback)
{
  mFrameRenderedCallback = std::move(callback);
}

void WebEngineLwe::RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback)
{
  mLoadStartedCallback = std::move(callback);
}

void WebEngineLwe::RegisterPageLoadInProgressCallback(WebEnginePageLoadCallback)
{
}

void WebEngineLwe::RegisterPageLoadFinishedCallback(WebEnginePageLoadCallback callback)
{
  mLoadFinishedCallback = std::move(callback);
}

void WebEngineLwe::RegisterPageLoadErrorCallback(WebEnginePageLoadErrorCallback callback)
{
  mLoadErrorCallback = std::move(callback);
}

void WebEngineLwe::RegisterScrollEdgeReachedCallback(WebEngineScrollEdgeReachedCallback)
{
}

void WebEngineLwe::RegisterOverScrolledCallback(WebEngineOverScrolledCallback)
{
}

void WebEngineLwe::RegisterUrlChangedCallback(WebEngineUrlChangedCallback callback)
{
  mUrlChangedCallback = std::move(callback);
}

void WebEngineLwe::RegisterFormRepostDecidedCallback(WebEngineFormRepostDecidedCallback)
{
}

void WebEngineLwe::RegisterConsoleMessageReceivedCallback(WebEngineConsoleMessageReceivedCallback)
{
}

void WebEngineLwe::RegisterResponsePolicyDecidedCallback(WebEngineResponsePolicyDecidedCallback)
{
}

void WebEngineLwe::RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback)
{
}

void WebEngineLwe::RegisterNewWindowPolicyDecidedCallback(WebEngineNewWindowPolicyDecidedCallback)
{
}

void WebEngineLwe::RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback)
{
}

void WebEngineLwe::RegisterCertificateConfirmedCallback(WebEngineCertificateCallback)
{
}

void WebEngineLwe::RegisterSslCertificateChangedCallback(WebEngineCertificateCallback)
{
}

void WebEngineLwe::RegisterHttpAuthHandlerCallback(WebEngineHttpAuthHandlerCallback)
{
}

void WebEngineLwe::RegisterContextMenuShownCallback(WebEngineContextMenuShownCallback)
{
}

void WebEngineLwe::RegisterContextMenuHiddenCallback(WebEngineContextMenuHiddenCallback)
{
}

void WebEngineLwe::RegisterFullscreenEnteredCallback(WebEngineFullscreenEnteredCallback)
{
}

void WebEngineLwe::RegisterFullscreenExitedCallback(WebEngineFullscreenExitedCallback)
{
}

void WebEngineLwe::RegisterTextFoundCallback(WebEngineTextFoundCallback)
{
}

void WebEngineLwe::GetPlainTextAsynchronously(PlainTextReceivedCallback)
{
}

void WebEngineLwe::WebAuthenticationCancel()
{
}

void WebEngineLwe::RegisterWebAuthDisplayQRCallback(WebEngineWebAuthDisplayQRCallback)
{
}

void WebEngineLwe::RegisterWebAuthResponseCallback(WebEngineWebAuthResponseCallback)
{
}

void WebEngineLwe::RegisterFileChooserRequestedCallback(WebEngineFileChooserRequestedCallback)
{
}

void WebEngineLwe::RegisterWebProcessCrashedCallback(WebEngineWebProcessCrashedCallback)
{
}

void WebEngineLwe::RegisterUserMediaPermissionRequestCallback(WebEngineUserMediaPermissionRequestCallback)
{
}

void WebEngineLwe::RegisterDeviceConnectionChangedCallback(WebEngineDeviceConnectionChangedCallback)
{
}

void WebEngineLwe::RegisterDeviceListGetCallback(WebEngineDeviceListGetCallback)
{
}

void WebEngineLwe::FeedMouseWheel(bool yDirection, int step, int x, int y)
{
  DALI_ASSERT_ALWAYS(mWebContainer);
  mWebContainer->DispatchMouseWheelEvent(x, y, yDirection ? step : -step);
}

void WebEngineLwe::SetVideoHole(bool, bool)
{
}

void WebEngineLwe::RegisterPlaybackVideoReadyCallback(WebEnginePlaybackVideoReadyCallback)
{
}

void WebEngineLwe::RegisterPlaybackVideoStartedCallback(WebEnginePlaybackVideoStartedCallback)
{
}

void WebEngineLwe::RegisterPlaybackVideoFinishedCallback(WebEnginePlaybackVideoFinishedCallback)
{
}

void WebEngineLwe::RegisterPlaybackVideoStoppedCallback(WebEnginePlaybackVideoStoppedCallback)
{
}

void WebEngineLwe::RegisterPlaybackVideoPausedCallback(WebEnginePlaybackVideoPausedCallback)
{
}

} // namespace Plugin
} // namespace Dali

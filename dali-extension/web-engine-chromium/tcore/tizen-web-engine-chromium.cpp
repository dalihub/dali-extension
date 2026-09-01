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

#include "tizen-web-engine-chromium.h"

#include "tizen-web-engine-back-forward-list.h"
#include "tizen-web-engine-certificate.h"
#include "tizen-web-engine-console-message.h"
#include "tizen-web-engine-context-menu-item.h"
#include "tizen-web-engine-context-menu.h"
#include "tizen-web-engine-context.h"
#include "tizen-web-engine-device-list-get.h"
#include "tizen-web-engine-file-chooser-request.h"
#include "tizen-web-engine-form-repost-decision.h"
#include "tizen-web-engine-hit-test.h"
#include "tizen-web-engine-http-auth-handler.h"
#include "tizen-web-engine-load-error.h"
#include "tizen-web-engine-manager.h"
#include "tizen-web-engine-policy-decision.h"
#include "tizen-web-engine-settings.h"
#include "tizen-web-engine-user-media-permission-request.h"

#include "../../integration-api/tizen-core-wl-display-util.h"

#include <glib.h>

// The WV interface is backend neutral: it takes the native window as a void*.
// Only the two places that obtain that handle - and the cursor theme, which has
// no WV counterpart - need to know which windowing backend is in use.
#include <tizen_core_wl.h>

#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/touch-event.h>
#include <dali/public-api/images/pixel-data.h>

#include <wv_geolocation.h>
#include <wv_main_internal.h>
#include <wv_settings_product.h>
#include <wv_view_product.h>

#include <cstring>

using namespace Dali;

namespace Dali
{
namespace Plugin
{
namespace
{
template<typename Callback, typename... Args>
void ExecuteCallback(Callback callback, Args... args)
{
  if(callback)
  {
    callback(args...);
  }
}

template<typename Callback, typename Arg>
void ExecuteCallback(Callback callback, std::unique_ptr<Arg> arg)
{
  if(callback)
  {
    callback(std::move(arg));
  }
}

template<typename Callback, typename Arg, typename... Args>
void ExecuteCallback(Callback callback, std::unique_ptr<Arg> arg, Args... args)
{
  if(callback)
  {
    callback(std::move(arg), args...);
  }
}

template<typename Callback, typename Arg>
void ExecuteCallback(Callback callback, Arg*& arg)
{
  if(callback)
  {
    callback(arg);
  }
}

template<typename Callback, typename Arg, typename... Args>
void ExecuteCallback2(Callback callback, Arg*& arg, Args... args)
{
  if(callback)
  {
    callback(arg, args...);
  }
}

template<typename Ret, typename Callback, typename... Args>
Ret ExecuteCallbackReturn(Callback callback, Args... args)
{
  Ret returnVal = Ret();
  if(callback)
  {
    returnVal = callback(args...);
  }
  return returnVal;
}

template<typename Ret, typename Callback, typename Arg>
Ret ExecuteCallbackReturn(Callback callback, std::unique_ptr<Arg> arg)
{
  Ret returnVal = Ret();
  if(callback)
  {
    returnVal = callback(std::move(arg));
  }
  return returnVal;
}

} // Anonymous namespace

TizenWebEngineChromium::TizenWebEngineChromium()
: mDaliImageSrc(NativeImage::New(0, 0, NativeImage::COLOR_DEPTH_DEFAULT)),
  mWebView(nullptr),
  mWidth(0),
  mHeight(0),
  mIsIncognito(false),
  mWebUserMediaPermissionRequest(nullptr),
  mDeviceListGet(nullptr)
{
}

TizenWebEngineChromium::~TizenWebEngineChromium()
{
  Destroy();
}

void TizenWebEngineChromium::Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneID)
{
  // Check if web engine is available and make sure that web engine is initialized.
  if(!WebEngineManager::IsAvailable())
  {
    DALI_LOG_ERROR("Web engine has been terminated in current process.");
    return;
  }

  mWidth  = width;
  mHeight = height;
  InitWebView(false);
  WebEngineManager::Get().Add(mWebView, this, false);
}

void TizenWebEngineChromium::Create(uint32_t width, uint32_t height, uint32_t argc, char** argv)
{
  // Route through WebEngineManager so the tcore switches are appended to the
  // application's own arguments; base::CommandLine only honours the first
  // caller, so setting argv directly here would drop them.
  WebEngineManager::SetArguments(static_cast<int>(argc), argv);

  // Check if web engine is available and make sure that web engine is initialized.
  if(!WebEngineManager::IsAvailable())
  {
    DALI_LOG_ERROR("Web engine has been terminated in current process.");
    return;
  }

  bool incognito = false;
  for(uint32_t idx = 0; idx < argc; ++idx)
  {
    if(strstr(argv[idx], "--incognito"))
    {
      incognito = true;
      break;
    }
  }

  mWidth  = width;
  mHeight = height;
  InitWebView(incognito);
  WebEngineManager::Get().Add(mWebView, this, incognito);
}

void TizenWebEngineChromium::InitWebView(bool incognito)
{
  mIsIncognito         = incognito;
  wv_context_h context = nullptr;
  if(mIsIncognito)
  {
    // wv_view_create*() takes no canvas: WV owns the surface, so the offscreen
    // Ecore_Evas window that ewk_view_add() needed is gone.
    mWebView = wv_view_create_in_incognito_mode();
    context  = wv_view_context_get(mWebView);
  }
  else
  {
    context  = wv_context_default_get();
    mWebView = wv_view_create();
  }
  WebEngineManager::Get().SetContext(context, mIsIncognito);
  wv_context_max_refresh_rate_set(context, 60);
  wv_view_offscreen_rendering_enabled_set(mWebView, true);

  tizen_core_wl_window_h win = AnyCast<tizen_core_wl_window_h>(Adaptor::Get().GetNativeWindowHandle());
  wv_view_ime_window_set(mWebView, win);

  wv_settings_h settings = wv_view_settings_get(mWebView);
  mWebEngineSettings.reset(new TizenWebEngineSettings(settings));

  wv_back_forward_list_h backForwardList = wv_view_back_forward_list_get(mWebView);
  mWebEngineBackForwardList.reset(new TizenWebEngineBackForwardList(backForwardList));

  wv_settings_drag_drop_enabled_set(settings, false);

  wv_view_add_cb(mWebView, "offscreen,frame,rendered", &TizenWebEngineChromium::OnFrameRendered, this);
  wv_view_add_cb(mWebView, "load,started", &TizenWebEngineChromium::OnLoadStarted, this);
  wv_view_add_cb(mWebView, "load,progress", &TizenWebEngineChromium::OnLoadInProgress, this);
  wv_view_add_cb(mWebView, "load,finished", &TizenWebEngineChromium::OnLoadFinished, this);
  wv_view_add_cb(mWebView, "load,error", &TizenWebEngineChromium::OnLoadError, this);
  wv_view_add_cb(mWebView, "url,changed", &TizenWebEngineChromium::OnUrlChanged, this);
  wv_view_add_cb(mWebView, "console,message", &TizenWebEngineChromium::OnConsoleMessageReceived, this);
  wv_view_add_cb(mWebView, "edge,left", &TizenWebEngineChromium::OnEdgeLeft, this);
  wv_view_add_cb(mWebView, "edge,right", &TizenWebEngineChromium::OnEdgeRight, this);
  wv_view_add_cb(mWebView, "edge,top", &TizenWebEngineChromium::OnEdgeTop, this);
  wv_view_add_cb(mWebView, "edge,bottom", &TizenWebEngineChromium::OnEdgeBottom, this);
  wv_view_add_cb(mWebView, "overscrolled,left", &TizenWebEngineChromium::OnOverScrolledLeft, this);
  wv_view_add_cb(mWebView, "overscrolled,right", &TizenWebEngineChromium::OnOverScrolledRight, this);
  wv_view_add_cb(mWebView, "overscrolled,top", &TizenWebEngineChromium::OnOverScrolledTop, this);
  wv_view_add_cb(mWebView, "overscrolled,bottom", &TizenWebEngineChromium::OnOverScrolledBottom, this);
  wv_view_add_cb(mWebView, "form,repost,warning,show", &TizenWebEngineChromium::OnFormRepostDecided, this);
  wv_view_add_cb(mWebView, "policy,response,decide", &TizenWebEngineChromium::OnResponsePolicyDecided, this);
  wv_view_add_cb(mWebView, "policy,navigation,decide", &TizenWebEngineChromium::OnNavigationPolicyDecided, this);
  wv_view_add_cb(mWebView, "policy,newwindow,decide", &TizenWebEngineChromium::OnNewWindowPolicyDecided, this);
  wv_view_add_cb(mWebView, "create,window", &TizenWebEngineChromium::OnNewWindowCreated, this);
  wv_view_add_cb(mWebView, "request,certificate,confirm", &TizenWebEngineChromium::OnCertificateConfirmed, this);
  wv_view_add_cb(mWebView, "ssl,certificate,changed", &TizenWebEngineChromium::OnSslCertificateChanged, this);
  wv_view_add_cb(mWebView, "contextmenu,show", &TizenWebEngineChromium::OnContextMenuShown, this);
  wv_view_add_cb(mWebView, "contextmenu,hide", &TizenWebEngineChromium::OnContextMenuHidden, this);
  wv_view_add_cb(mWebView, "fullscreen,enterfullscreen", &TizenWebEngineChromium::OnFullscreenEntered, this);
  wv_view_add_cb(mWebView, "fullscreen,exitfullscreen", &TizenWebEngineChromium::OnFullscreenExited, this);
  wv_view_add_cb(mWebView, "text,found", &TizenWebEngineChromium::OnTextFound, this);
  wv_view_add_cb(mWebView, "webauthn,display,qr", &TizenWebEngineChromium::OnWebAuthDisplayQR, this);
  wv_view_add_cb(mWebView, "webauthn,response", &TizenWebEngineChromium::OnWebAuthResponse, this);
  wv_view_add_cb(mWebView, "file,chooser,request", &TizenWebEngineChromium::OnFileChooserRequested, this);
  wv_view_add_cb(mWebView, "webprocess,crashed", &TizenWebEngineChromium::OnWebProcessCrashed, this);

  // The playback notifications are plain string events, so they register like any
  // other WV callback.
  wv_view_add_cb(mWebView, "notification,playback,ready", &TizenWebEngineChromium::OnPlaybackVideoReady, this);
  wv_view_add_cb(mWebView, "notification,playback,start", &TizenWebEngineChromium::OnPlaybackVideoStarted, this);
  wv_view_add_cb(mWebView, "notification,playback,finish", &TizenWebEngineChromium::OnPlaybackVideoFinished, this);
  wv_view_add_cb(mWebView, "notification,playback,stop", &TizenWebEngineChromium::OnPlaybackVideoStopped, this);
  wv_view_add_cb(mWebView, "notification,playback,pause", &TizenWebEngineChromium::OnPlaybackVideoPaused, this);

  wv_view_media_device_list_get(mWebView, TizenWebEngineChromium::OnDeviceListGet, this);
  wv_view_add_cb(mWebView, "device,connection,changed", &TizenWebEngineChromium::OnDeviceConnectionChanged, this);

  wv_view_resize(mWebView, mWidth, mHeight);
  wv_view_visibility_set(mWebView, true);
}

void TizenWebEngineChromium::Destroy()
{
  mJavaScriptInjectedCallbacks.clear();

  if(WebEngineManager::IsAvailable() && mWebView != nullptr)
  {
    WebEngineManager::Get().Remove(mWebView, mIsIncognito);
  }

  wv_view_destroy(mWebView);
  mWebView = nullptr;
}

bool TizenWebEngineChromium::IsIncognito() const
{
  return mIsIncognito;
}

void TizenWebEngineChromium::LoadUrl(const std::string& path)
{
  wv_view_url_set(mWebView, path.c_str());
}

std::string TizenWebEngineChromium::GetTitle() const
{
  const char* title = wv_view_title_get(mWebView);
  return title ? std::string(title) : std::string();
}

Dali::PixelData TizenWebEngineChromium::GetFavicon() const
{
  return ConvertImageBuffer(wv_view_favicon_get(mWebView), true);
}

NativeImagePtr TizenWebEngineChromium::GetNativeImage()
{
  return mDaliImageSrc;
}

void TizenWebEngineChromium::ChangeOrientation(int orientation)
{
  DALI_LOG_RELEASE_INFO("#ChangeOrientation : %d\n", orientation);
  wv_view_orientation_send(mWebView, orientation);
}

std::string TizenWebEngineChromium::GetUrl() const
{
  const char* url = wv_view_url_get(mWebView);
  return url ? std::string(url) : std::string();
}

void TizenWebEngineChromium::LoadHtmlString(const std::string& html)
{
  wv_view_html_string_load(mWebView, html.c_str(), 0, 0);
}

bool TizenWebEngineChromium::LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl)
{
  char* cBasicUri       = basicUri.length() ? (char*)basicUri.c_str() : nullptr;
  char* cUnreachableUrl = unreachableUrl.length() ? (char*)unreachableUrl.c_str() : nullptr;
  return wv_view_html_string_override_current_entry_load(mWebView, html.c_str(), cBasicUri, cUnreachableUrl);
}

bool TizenWebEngineChromium::LoadContents(const int8_t* contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri)
{
  char* cMimeType = mimeType.length() ? (char*)mimeType.c_str() : nullptr;
  char* cEncoding = encoding.length() ? (char*)encoding.c_str() : nullptr;
  char* cBaseUri  = baseUri.length() ? (char*)baseUri.c_str() : nullptr;
  return wv_view_contents_set(mWebView, (const char*)contents, contentSize, cMimeType, cEncoding, cBaseUri);
}

void TizenWebEngineChromium::Reload()
{
  wv_view_reload(mWebView);
}

bool TizenWebEngineChromium::ReloadWithoutCache()
{
  return wv_view_reload_bypass_cache(mWebView);
}

void TizenWebEngineChromium::StopLoading()
{
  wv_view_stop(mWebView);
}

void TizenWebEngineChromium::Suspend()
{
  wv_view_suspend(mWebView);
}

void TizenWebEngineChromium::Resume()
{
  wv_view_resume(mWebView);
}

void TizenWebEngineChromium::SuspendNetworkLoading()
{
  wv_view_suspend_network_loading(mWebView);
}

void TizenWebEngineChromium::ResumeNetworkLoading()
{
  wv_view_resume_network_loading(mWebView);
}

bool TizenWebEngineChromium::AddCustomHeader(const std::string& name, const std::string& value)
{
  return wv_view_custom_header_add(mWebView, name.c_str(), value.c_str());
}

bool TizenWebEngineChromium::RemoveCustomHeader(const std::string& name)
{
  return wv_view_custom_header_remove(mWebView, name.c_str());
}

uint32_t TizenWebEngineChromium::StartInspectorServer(uint32_t port)
{
  return wv_view_inspector_server_start(mWebView, port);
}

bool TizenWebEngineChromium::StopInspectorServer()
{
  return wv_view_inspector_server_stop(mWebView);
}

void TizenWebEngineChromium::ScrollBy(int32_t deltaX, int32_t deltaY)
{
  wv_view_scroll_by(mWebView, deltaX, deltaY);
}

bool TizenWebEngineChromium::ScrollEdgeBy(int32_t deltaX, int32_t deltaY)
{
  return wv_view_edge_scroll_by(mWebView, deltaX, deltaY);
}

void TizenWebEngineChromium::SetScrollPosition(int32_t x, int32_t y)
{
  wv_view_scroll_set(mWebView, x, y);
}

Dali::Vector2 TizenWebEngineChromium::GetScrollPosition() const
{
  int x = 0, y = 0;
  wv_view_scroll_pos_get(mWebView, &x, &y);
  return Dali::Vector2(x, y);
}

Dali::Vector2 TizenWebEngineChromium::GetScrollSize() const
{
  int width = 0, height = 0;
  wv_view_scroll_size_get(mWebView, &width, &height);
  return Dali::Vector2(width, height);
}

Dali::Vector2 TizenWebEngineChromium::GetContentSize() const
{
  int width = 0, height = 0;
  wv_view_contents_size_get(mWebView, &width, &height);
  return Dali::Vector2(width, height);
}

bool TizenWebEngineChromium::CanGoForward()
{
  return wv_view_forward_possible(mWebView);
}

void TizenWebEngineChromium::GoForward()
{
  wv_view_forward(mWebView);
}

bool TizenWebEngineChromium::CanGoBack()
{
  return wv_view_back_possible(mWebView);
}

void TizenWebEngineChromium::GoBack()
{
  wv_view_back(mWebView);
}

void TizenWebEngineChromium::EvaluateJavaScript(const std::string& script, JavaScriptMessageHandlerCallback resultHandler)
{
  mJavaScriptEvaluatedCallback = resultHandler;
  wv_view_script_execute(mWebView, script.c_str(), &TizenWebEngineChromium::OnJavaScriptEvaluated, this);
}

void TizenWebEngineChromium::AddJavaScriptMessageHandler(const std::string& exposedObjectName, JavaScriptMessageHandlerCallback handler)
{
  mJavaScriptInjectedCallbacks.erase(exposedObjectName);
  mJavaScriptInjectedCallbacks.insert(std::pair<std::string, JavaScriptMessageHandlerCallback>(exposedObjectName, handler));
  wv_view_javascript_message_handler_add(mWebView, &TizenWebEngineChromium::OnJavaScriptInjected, exposedObjectName.c_str());
}

void TizenWebEngineChromium::AddJavaScriptEntireMessageHandler(const std::string& exposedObjectName, JavaScriptEntireMessageHandlerCallback handler)
{
  mJavaScriptEntireMessageReceivedCallback = handler;
  wv_view_javascript_message_handler_add(mWebView, &TizenWebEngineChromium::OnJavaScriptEntireMessageReceived, exposedObjectName.c_str());
}

void TizenWebEngineChromium::RegisterJavaScriptAlertCallback(JavaScriptAlertCallback callback)
{
  mJavaScriptAlertCallback = callback;
  if(mJavaScriptAlertCallback)
  {
    wv_view_javascript_alert_callback_set(mWebView, &TizenWebEngineChromium::OnJavaScriptAlert, this);
  }
  else
  {
    wv_view_javascript_alert_callback_set(mWebView, nullptr, nullptr);
  }
}

void TizenWebEngineChromium::JavaScriptAlertReply()
{
  wv_view_javascript_alert_reply(mWebView);
}

void TizenWebEngineChromium::RegisterJavaScriptConfirmCallback(JavaScriptConfirmCallback callback)
{
  mJavaScriptConfirmCallback = callback;
  if(mJavaScriptConfirmCallback)
  {
    wv_view_javascript_confirm_callback_set(mWebView, &TizenWebEngineChromium::OnJavaScriptConfirm, this);
  }
  else
  {
    wv_view_javascript_confirm_callback_set(mWebView, nullptr, nullptr);
  }
}

void TizenWebEngineChromium::JavaScriptConfirmReply(bool confirmed)
{
  wv_view_javascript_confirm_reply(mWebView, confirmed);
}

void TizenWebEngineChromium::RegisterJavaScriptPromptCallback(JavaScriptPromptCallback callback)
{
  mJavaScriptPromptCallback = callback;
  if(mJavaScriptPromptCallback)
  {
    wv_view_javascript_prompt_callback_set(mWebView, &TizenWebEngineChromium::OnJavaScriptPrompt, this);
  }
  else
  {
    wv_view_javascript_prompt_callback_set(mWebView, nullptr, nullptr);
  }
}

void TizenWebEngineChromium::JavaScriptPromptReply(const std::string& result)
{
  wv_view_javascript_prompt_reply(mWebView, result.c_str());
}

void TizenWebEngineChromium::ClearHistory()
{
  wv_view_back_forward_list_clear(mWebView);
}

void TizenWebEngineChromium::ClearAllTilesResources()
{
  wv_view_clear_all_tiles_resources(mWebView);
}

std::string TizenWebEngineChromium::GetUserAgent() const
{
  const char* agent = wv_view_user_agent_get(mWebView);
  return agent ? std::string(agent) : std::string();
}

void TizenWebEngineChromium::SetUserAgent(const std::string& userAgent)
{
  wv_view_user_agent_set(mWebView, userAgent.c_str());
}

Dali::WebEngineSettings& TizenWebEngineChromium::GetSettings() const
{
  static TizenWebEngineSettings dummy(nullptr);
  return mWebEngineSettings ? *mWebEngineSettings : dummy;
}

Dali::WebEngineBackForwardList& TizenWebEngineChromium::GetBackForwardList() const
{
  static TizenWebEngineBackForwardList dummy(nullptr);
  return mWebEngineBackForwardList ? *mWebEngineBackForwardList : dummy;
}

std::unique_ptr<Dali::WebEngineHitTest> TizenWebEngineChromium::CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode)
{
  wv_hit_test_h hitTest = wv_view_hit_test_new(mWebView, x, y, (int)mode);
  // The handle is created for this caller, so the wrapper owns it and frees it.
  return hitTest ? std::unique_ptr<Dali::WebEngineHitTest>(new TizenWebEngineHitTest(hitTest, true)) : nullptr;
}

bool TizenWebEngineChromium::CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, WebEngineHitTestCreatedCallback callback)
{
  mHitTestCreatedCallback = callback;
  return wv_view_hit_test_request(mWebView, x, y, static_cast<wv_hit_test_mode_e>(mode), &TizenWebEngineChromium::OnHitTestCreated, this);
}

void TizenWebEngineChromium::SetSize(uint32_t width, uint32_t height)
{
  mWidth  = width;
  mHeight = height;
  wv_view_resize(mWebView, mWidth, mHeight);
}

void TizenWebEngineChromium::SetDocumentBackgroundColor(Dali::Vector4 color)
{
  wv_view_bg_color_set(mWebView, color.r * 255, color.g * 255, color.b * 255, color.a * 255);
}

void TizenWebEngineChromium::ClearTilesWhenHidden(bool cleared)
{
  // WV GAP (WV_REQUIREMENTS.md B): wv_view_clear_tiles_on_hide_enabled_set() is
  // not declared by the target WV headers, so tiles keep their default
  // hide-time behaviour.
}

void TizenWebEngineChromium::SetTileCoverAreaMultiplier(float multiplier)
{
  // WV GAP (WV_REQUIREMENTS.md B): wv_view_tile_cover_area_multiplier_set() is
  // not declared by the target WV headers, so the tile cover area keeps its
  // default multiplier.
}

void TizenWebEngineChromium::EnableCursorByClient(bool enabled)
{
  wv_view_set_cursor_by_client(mWebView, enabled);
}

std::string TizenWebEngineChromium::GetSelectedText() const
{
  const char* text = wv_view_text_selection_text_get(mWebView);
  return text ? std::string(text) : std::string();
}

bool TizenWebEngineChromium::SendTouchEvent(const Dali::TouchEvent& touch)
{
#if defined(OS_TIZEN_TV)
  return FeedMouseEvent(touch);
#else
  return FeedTouchEvent(touch);
#endif
}

bool TizenWebEngineChromium::FeedMouseEvent(const TouchEvent& touch)
{
  wv_mouse_button_type_e type = (wv_mouse_button_type_e)0;
  switch(touch.GetMouseButton(0))
  {
    case MouseButton::PRIMARY:
    {
      type = WV_MOUSE_BUTTON_LEFT;
      break;
    }
    case MouseButton::TERTIARY:
    {
      type = WV_MOUSE_BUTTON_MIDDLE;
      break;
    }
    case MouseButton::SECONDARY:
    {
      type = WV_MOUSE_BUTTON_RIGHT;
      break;
    }
    default:
    {
      break;
    }
  }

  switch(touch.GetState(0))
  {
    case PointState::DOWN:
    {
      float x = touch.GetScreenPosition(0).x;
      float y = touch.GetScreenPosition(0).y;
      wv_view_feed_mouse_down(mWebView, type, x, y);
      break;
    }
    case PointState::UP:
    {
      float x = touch.GetScreenPosition(0).x;
      float y = touch.GetScreenPosition(0).y;
      wv_view_feed_mouse_up(mWebView, type, x, y);
      break;
    }
    case PointState::MOTION:
    {
      float x = touch.GetScreenPosition(0).x;
      float y = touch.GetScreenPosition(0).y;
      wv_view_feed_mouse_move(mWebView, x, y);
      break;
    }
    default:
    {
      break;
    }
  }
  return false;
}

bool TizenWebEngineChromium::FeedTouchEvent(const TouchEvent& touch)
{
  wv_touch_event_type_e type  = WV_TOUCH_EVENT_START;
  wv_touch_point_state_e state = WV_TOUCH_POINT_STATE_DOWN;

  bool fed = false;
  for(std::size_t i = 0; i < touch.GetPointCount(); i++)
  {
    switch(touch.GetState(i))
    {
      case PointState::DOWN:
      {
        type  = WV_TOUCH_EVENT_START;
        state = WV_TOUCH_POINT_STATE_DOWN;
        break;
      }
      case PointState::UP:
      {
        type  = WV_TOUCH_EVENT_END;
        state = WV_TOUCH_POINT_STATE_UP;
        break;
      }
      case PointState::MOTION:
      {
        type  = WV_TOUCH_EVENT_MOVE;
        state = WV_TOUCH_POINT_STATE_MOVE;
        break;
      }
      case PointState::INTERRUPTED:
      {
        type  = WV_TOUCH_EVENT_CANCEL;
        state = WV_TOUCH_POINT_STATE_CANCEL;
        break;
      }
      default:
      {
        break;
      }
    }

    // Zero-initialised: the struct may grow fields this code does not set, and
    // they would otherwise reach the engine as stack garbage.
    wv_touch_point_s point = {};
    point.id               = static_cast<int>(i);
    point.x                = static_cast<int>(touch.GetScreenPosition(i).x);
    point.y                = static_cast<int>(touch.GetScreenPosition(i).y);
    point.state            = state;
    GList* pointList       = g_list_append(nullptr, &point);
    fed                    = wv_view_feed_touch_event(mWebView, type, pointList, WV_MODIFIER_NONE);
    g_list_free(pointList);
    if(!fed)
    {
      break;
    }
  }
  return fed;
}

void TizenWebEngineChromium::ResetDaliImageSource()
{
  mDaliImageSrc = NativeImage::New(0, 0, NativeImage::COLOR_DEPTH_DEFAULT);
}

namespace
{
/**
 * @brief Maps Dali::KeyEvent::Modifier bits onto wv_modifier_e.
 *
 * Dali defines SHIFT/CTRL/ALT as 0x1/0x2/0x4 and WV defines SHIFT/CONTROL/ALT
 * as the same three bits, so the values line up. Mask anyway rather than cast,
 * so a future Dali modifier cannot leak through as an unrelated WV bit.
 */
wv_modifier_e ToWvModifiers(int32_t daliModifiers)
{
  unsigned int modifiers = WV_MODIFIER_NONE;
  if(daliModifiers & Dali::KeyEvent::SHIFT)
  {
    modifiers |= WV_MODIFIER_SHIFT;
  }
  if(daliModifiers & Dali::KeyEvent::CTRL)
  {
    modifiers |= WV_MODIFIER_CONTROL;
  }
  if(daliModifiers & Dali::KeyEvent::ALT)
  {
    modifiers |= WV_MODIFIER_ALT;
  }
  return static_cast<wv_modifier_e>(modifiers);
}
} // namespace

bool TizenWebEngineChromium::SendKeyEvent(const Dali::KeyEvent& keyEvent)
{
  // WV collapses the whole Evas key apparatus (Evas_Modifier / Evas_Lock /
  // Evas_Device, kept in sync through the canvas) into one flat struct, so the
  // canvas round-trip disappears.
  wv_key_event_s wvKeyEvent = {};

  wvKeyEvent.timestamp = keyEvent.GetTime();
  wvKeyEvent.key_name  = keyEvent.GetKeyName().CStr();
  wvKeyEvent.key       = keyEvent.GetLogicalKey().CStr();
  wvKeyEvent.string    = keyEvent.GetKeyString().CStr();
  wvKeyEvent.key_code  = keyEvent.GetKeyCode();
  wvKeyEvent.modifiers = ToWvModifiers(keyEvent.GetKeyModifier());

  wv_view_send_key_event(mWebView, &wvKeyEvent, keyEvent.GetState() == Dali::KeyEvent::DOWN ? 1 : 0);
  return false;
}

bool TizenWebEngineChromium::SendHoverEvent(const Dali::HoverEvent& event)
{
  switch(event.GetState(0))
  {
    case PointState::MOTION:
    {
      float x = event.GetScreenPosition(0).x;
      float y = event.GetScreenPosition(0).y;
      wv_view_feed_mouse_move(mWebView, x, y);
      break;
    }
    default:
    {
      break;
    }
  }
  return false;
}

bool TizenWebEngineChromium::SendWheelEvent(const Dali::WheelEvent& wheel)
{
  bool direction = wheel.GetDirection() ? false : true;
  int       step      = wheel.GetDelta();
  float     x         = wheel.GetPoint().x;
  float     y         = wheel.GetPoint().y;
  wv_view_feed_mouse_wheel(mWebView, direction, step, x, y);
  return false;
}

void TizenWebEngineChromium::ExitFullscreen()
{
  wv_view_fullscreen_exit(mWebView);
}

void TizenWebEngineChromium::SetFocus(bool focused)
{
  wv_view_focus_set(mWebView, focused);
}

bool TizenWebEngineChromium::SetImePositionAndAlignment(Dali::Vector2 position, int alignment)
{
  return wv_view_ime_position_align_set(mWebView, position.x, position.y, (wv_ime_position_align_e)alignment);
}

void TizenWebEngineChromium::SetCursorThemeName(const std::string themeName)
{
  // tcore has no "give me the already connected display" call, so open one for
  // the duration of the change and close it again.
  tizen_core_wl_display_h display = nullptr;
  if(!Dali::Internal::Adaptor::TcoreWlAcquireDisplay(&display))
  {
    return;
  }

  tizen_core_wl_seat_h seat = nullptr;
  if(tizen_core_wl_display_get_default_seat(display, &seat) == TIZEN_CORE_WL_ERROR_NONE && seat)
  {
    tizen_core_wl_seat_set_cursor_theme(seat, themeName.c_str());
  }

  Dali::Internal::Adaptor::TcoreWlReleaseDisplay(display);
}

void TizenWebEngineChromium::EnableMouseEvents(bool enabled)
{
  wv_view_mouse_events_enabled_set(mWebView, enabled);
}

void TizenWebEngineChromium::EnableKeyEvents(bool enabled)
{
  wv_view_key_events_enabled_set(mWebView, enabled);
}

void TizenWebEngineChromium::SetPageZoomFactor(float zoomFactor)
{
  wv_view_page_zoom_set(mWebView, zoomFactor);
}

float TizenWebEngineChromium::GetPageZoomFactor() const
{
  return wv_view_page_zoom_get(mWebView);
}

void TizenWebEngineChromium::SetTextZoomFactor(float zoomFactor)
{
  wv_view_text_zoom_set(mWebView, zoomFactor);
}

float TizenWebEngineChromium::GetTextZoomFactor() const
{
  return wv_view_text_zoom_get(mWebView);
}

float TizenWebEngineChromium::GetLoadProgressPercentage() const
{
  return wv_view_load_progress_get(mWebView);
}

void TizenWebEngineChromium::SetScaleFactor(float scaleFactor, Dali::Vector2 point)
{
  wv_view_scale_set(mWebView, scaleFactor, point.x, point.y);
}

float TizenWebEngineChromium::GetScaleFactor() const
{
  return wv_view_scale_get(mWebView);
}

void TizenWebEngineChromium::ActivateAccessibility(bool activated)
{
  wv_view_atk_deactivation_by_app(mWebView, activated);
}

void TizenWebEngineChromium::FeedMouseWheel(bool yDirection, int step, int x, int y)
{
  wv_view_feed_mouse_wheel(mWebView, (bool)yDirection, step, x, y);
}

void TizenWebEngineChromium::SetVideoHole(bool enabled, bool isWaylandWindow)
{
  tizen_core_wl_window_h win = AnyCast<tizen_core_wl_window_h>(Adaptor::Get().GetNativeWindowHandle());
  wv_view_set_support_video_hole(mWebView, win, enabled, isWaylandWindow ? true : false);
}

Devel::Accessibility::Address TizenWebEngineChromium::GetAccessibilityAddress()
{
  static const char plugIdKey[] = "__PlugID";
  static const char rootPath[]  = "root";

  std::string_view plugId;

  // WV GAP (WV_REQUIREMENTS.md D-2): wv_view_data_get() is not declared by the
  // target WV headers, so the AT-SPI plug ID cannot be read back from the view
  // and the parse below always fails, yielding an empty address.

  // We expect plugId to be of the form ":1.23:/org/a11y/atspi/accessible/root"
  auto pos = plugId.rfind(':');
  if(pos == std::string_view::npos || pos == 0)
  {
    DALI_LOG_ERROR("Cannot parse PlugID set by Chromium: %s = \"%s\"", plugIdKey, plugId.data());
    return {};
  }

  return {std::string{plugId.substr(0, pos)}, rootPath};
}

bool TizenWebEngineChromium::SetVisibility(bool visible)
{
  return wv_view_visibility_set(mWebView, visible);
}

bool TizenWebEngineChromium::HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount)
{
  return wv_view_text_find(mWebView, text.c_str(), (wv_find_options_e)options, maxMatchCount);
}

void TizenWebEngineChromium::AddDynamicCertificatePath(const std::string& host, const std::string& certPath)
{
  wv_view_add_dynamic_certificate_path(mWebView, host.c_str(), certPath.c_str());
}

Dali::PixelData TizenWebEngineChromium::GetScreenshot(Dali::BoundsInteger viewArea, float scaleFactor)
{
  wv_rectangle_s rect = {viewArea.x, viewArea.y, static_cast<int>(viewArea.width), static_cast<int>(viewArea.height)};
  return ConvertImageBuffer(wv_view_screenshot_contents_get(mWebView, rect, scaleFactor), true);
}

bool TizenWebEngineChromium::GetScreenshotAsynchronously(Dali::BoundsInteger viewArea, float scaleFactor, ScreenshotCapturedCallback callback)
{
  mScreenshotCapturedCallback = callback;
  wv_rectangle_s rect         = {viewArea.x, viewArea.y, static_cast<int>(viewArea.width), static_cast<int>(viewArea.height)};
  return wv_view_screenshot_contents_get_async(mWebView, rect, scaleFactor, &TizenWebEngineChromium::OnScreenshotCaptured, this);
}

bool TizenWebEngineChromium::CheckVideoPlayingAsynchronously(VideoPlayingCallback callback)
{
  mVideoPlayingCallback = callback;
  return wv_view_is_video_playing(mWebView, &TizenWebEngineChromium::OnVideoPlaying, this);
}

void TizenWebEngineChromium::GetPlainTextAsynchronously(PlainTextReceivedCallback callback)
{
  mPlainTextReceivedCallback = callback;
  wv_view_plain_text_get(mWebView, &TizenWebEngineChromium::OnPlainTextReceived, this);
}

void TizenWebEngineChromium::WebAuthenticationCancel()
{
#ifdef OVER_TIZEN_VERSION_9
  wv_view_webauthn_cancel(mWebView);
#endif
}

void TizenWebEngineChromium::RemoveJavaScriptMessageHandler(const std::string& exposedObjectName)
{
  // NOT IMPLEMENTED: Chromium has no corresponding operation.
}

void TizenWebEngineChromium::RegisterGeolocationPermissionCallback(GeolocationPermissionCallback callback)
{
  mGeolocationPermissionCallback = callback;
  wv_view_geolocation_permission_callback_set(mWebView, &TizenWebEngineChromium::OnGeolocationPermission, this);
}

void TizenWebEngineChromium::RegisterUserMediaPermissionRequestCallback(WebEngineUserMediaPermissionRequestCallback callback)
{
  mUserMediaPermissionRequestCallback = callback;
  wv_view_user_media_permission_callback_set(mWebView, &TizenWebEngineChromium::OnUserMediaPermissonRequest, this);
}

void TizenWebEngineChromium::UpdateDisplayArea(Dali::BoundsInteger displayArea)
{
  // Size was changed. Destroy previous native image, and create new one.
  ResetDaliImageSource();

  // WV GAP (WV_REQUIREMENTS.md D-3): wv_view_move() and wv_view_geometry_set()
  // are not declared by the target WV headers, so only the size is updated and
  // the view's origin stays where it was created.
  SetSize(displayArea.width, displayArea.height);
}

void TizenWebEngineChromium::EnableVideoHole(bool enabled)
{
  tizen_core_wl_window_h win = AnyCast<tizen_core_wl_window_h>(Adaptor::Get().GetNativeWindowHandle());
  wv_view_set_support_video_hole(mWebView, win, enabled, false);
}

void TizenWebEngineChromium::RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback)
{
  mFrameRenderedCallback = callback;
}

void TizenWebEngineChromium::RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback)
{
  mLoadStartedCallback = callback;
}

void TizenWebEngineChromium::RegisterPageLoadInProgressCallback(WebEnginePageLoadCallback callback)
{
  mLoadInProgressCallback = callback;
}

void TizenWebEngineChromium::RegisterPageLoadFinishedCallback(WebEnginePageLoadCallback callback)
{
  mLoadFinishedCallback = callback;
}

void TizenWebEngineChromium::RegisterPageLoadErrorCallback(WebEnginePageLoadErrorCallback callback)
{
  mLoadErrorCallback = callback;
}

void TizenWebEngineChromium::RegisterScrollEdgeReachedCallback(WebEngineScrollEdgeReachedCallback callback)
{
  mScrollEdgeReachedCallback = callback;
}

void TizenWebEngineChromium::RegisterOverScrolledCallback(WebEngineOverScrolledCallback callback)
{
  mOverScrolledCallback = callback;
}

void TizenWebEngineChromium::RegisterUrlChangedCallback(WebEngineUrlChangedCallback callback)
{
  mUrlChangedCallback = callback;
}

void TizenWebEngineChromium::RegisterFormRepostDecidedCallback(WebEngineFormRepostDecidedCallback callback)
{
  mFormRepostDecidedCallback = callback;
}

void TizenWebEngineChromium::RegisterConsoleMessageReceivedCallback(WebEngineConsoleMessageReceivedCallback callback)
{
  mConsoleMessageReceivedCallback = callback;
}

void TizenWebEngineChromium::RegisterResponsePolicyDecidedCallback(WebEngineResponsePolicyDecidedCallback callback)
{
  mResponsePolicyDecidedCallback = callback;
}

void TizenWebEngineChromium::RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback callback)
{
  mNavigationPolicyDecidedCallback = callback;
}

void TizenWebEngineChromium::RegisterNewWindowPolicyDecidedCallback(WebEngineNewWindowPolicyDecidedCallback callback)
{
  mNewWindowPolicyDecidedCallback = callback;
}

void TizenWebEngineChromium::RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback callback)
{
  mNewWindowCreatedCallback = callback;
}

void TizenWebEngineChromium::RegisterCertificateConfirmedCallback(WebEngineCertificateCallback callback)
{
  mCertificateConfirmedCallback = callback;
}

void TizenWebEngineChromium::RegisterSslCertificateChangedCallback(WebEngineCertificateCallback callback)
{
  mSslCertificateChangedCallback = callback;
}

void TizenWebEngineChromium::RegisterHttpAuthHandlerCallback(WebEngineHttpAuthHandlerCallback callback)
{
  mHttpAuthHandlerCallback = callback;
  if(mHttpAuthHandlerCallback)
  {
    wv_view_authentication_callback_set(mWebView, &TizenWebEngineChromium::OnAuthenticationChallenged, this);
  }
  else
  {
    wv_view_authentication_callback_set(mWebView, nullptr, nullptr);
  }
}

void TizenWebEngineChromium::RegisterContextMenuShownCallback(WebEngineContextMenuShownCallback callback)
{
  mContextMenuShownCallback = callback;
}

void TizenWebEngineChromium::RegisterContextMenuHiddenCallback(WebEngineContextMenuHiddenCallback callback)
{
  mContextMenuHiddenCallback = callback;
}

void TizenWebEngineChromium::RegisterFullscreenEnteredCallback(WebEngineFullscreenEnteredCallback callback)
{
  mFullscreenEnteredCallback = callback;
}

void TizenWebEngineChromium::RegisterFullscreenExitedCallback(WebEngineFullscreenExitedCallback callback)
{
  mFullscreenExitedCallback = callback;
}

void TizenWebEngineChromium::RegisterTextFoundCallback(WebEngineTextFoundCallback callback)
{
  mTextFoundCallback = callback;
}

void TizenWebEngineChromium::RegisterWebAuthDisplayQRCallback(WebEngineWebAuthDisplayQRCallback callback)
{
  mWebAuthDisplayQRCallback = callback;
}

void TizenWebEngineChromium::RegisterWebAuthResponseCallback(WebEngineWebAuthResponseCallback callback)
{
  mWebAuthResponseCallback = callback;
}

void TizenWebEngineChromium::RegisterFileChooserRequestedCallback(WebEngineFileChooserRequestedCallback callback)
{
  mFileChooserRequestedCallback = callback;
}

void TizenWebEngineChromium::RegisterWebProcessCrashedCallback(WebEngineWebProcessCrashedCallback callback)
{
  mWebProcessCrashedCallback = callback;
}

void TizenWebEngineChromium::RegisterDeviceConnectionChangedCallback(WebEngineDeviceConnectionChangedCallback callback)
{
  mDeviceConnectionChangedCallback = callback;
}

void TizenWebEngineChromium::RegisterDeviceListGetCallback(WebEngineDeviceListGetCallback callback)
{
  mDeviceListGetCallback = callback;
  wv_view_media_device_list_get(mWebView, TizenWebEngineChromium::OnDeviceListGet, this);
}

void TizenWebEngineChromium::RegisterPlaybackVideoReadyCallback(WebEnginePlaybackVideoReadyCallback callback)
{
  mPlaybackVideoReadyCallback = callback;
}

void TizenWebEngineChromium::RegisterPlaybackVideoStartedCallback(WebEnginePlaybackVideoStartedCallback callback)
{
  mPlaybackVideoStartedCallback = callback;
}

void TizenWebEngineChromium::RegisterPlaybackVideoFinishedCallback(WebEnginePlaybackVideoFinishedCallback callback)
{
  mPlaybackVideoFinishedCallback = callback;
}

void TizenWebEngineChromium::RegisterPlaybackVideoStoppedCallback(WebEnginePlaybackVideoStoppedCallback callback)
{
  mPlaybackVideoStoppedCallback = callback;
}

void TizenWebEngineChromium::RegisterPlaybackVideoPausedCallback(WebEnginePlaybackVideoPausedCallback callback)
{
  mPlaybackVideoPausedCallback = callback;
}

Dali::PixelData TizenWebEngineChromium::ConvertImageBuffer(wv_image_buffer_h image, bool ownsImage)
{
  if(!image)
  {
    return Dali::PixelData();
  }

  wv_image_buffer_info_s info = {};
  if(!wv_image_buffer_info_get(image, &info) || !info.data)
  {
    if(ownsImage)
    {
      wv_image_buffer_free(image);
    }
    return Dali::PixelData();
  }

  uint8_t* convertedBuffer = new uint8_t[info.size];
  std::memcpy(convertedBuffer, info.data, info.size);
  uint32_t width  = info.width;
  uint32_t height = info.height;

  if(ownsImage)
  {
    wv_image_buffer_free(image);
  }

  return Dali::PixelData::New(convertedBuffer, info.size, width, height, Dali::Pixel::Format::BGRA8888, Dali::PixelData::ReleaseFunction::DELETE_ARRAY);
}

void TizenWebEngineChromium::UpdateImage(tbm_surface_h buffer)
{
  if(!buffer)
  {
    return;
  }

  uint32_t width  = tbm_surface_get_width(buffer);
  uint32_t height = tbm_surface_get_height(buffer);

  // Upldate source only if buffer size is equal with current image
  if(mWidth == width && mHeight == height)
  {
    Any source(buffer);
    mDaliImageSrc->SetSource(source);
    Dali::Adaptor::Get().RequestProcessEventsAndUpdate();
    ExecuteCallback(mFrameRenderedCallback);
  }
  else
  {
    DALI_LOG_DEBUG_INFO("(WebEngine) WebEngine size not matched with buffer (WebEngine size is %ux%u, but buffer size is %ux%u)\n", mWidth, mHeight, width, height);
  }
}

void TizenWebEngineChromium::OnFrameRendered(wv_view_h, void* buffer, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  pThis->UpdateImage(static_cast<tbm_surface_h>(buffer));
}

void TizenWebEngineChromium::OnLoadStarted(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#LoadStarted : %s\n", pThis->GetUrl().c_str());
  ExecuteCallback(pThis->mLoadStartedCallback, pThis->GetUrl());
}

void TizenWebEngineChromium::OnLoadInProgress(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#LoadInProgress : %s\n", pThis->GetUrl().c_str());
  ExecuteCallback(pThis->mLoadInProgressCallback, pThis->GetUrl().c_str());
}

void TizenWebEngineChromium::OnLoadFinished(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#LoadFinished : %s\n", pThis->GetUrl().c_str());
  ExecuteCallback(pThis->mLoadFinishedCallback, pThis->GetUrl());
}

void TizenWebEngineChromium::OnLoadError(wv_view_h, void* rawError, void* data)
{
  auto                                      pThis = static_cast<TizenWebEngineChromium*>(data);
  wv_error_h error = static_cast<wv_error_h >(rawError);
  std::unique_ptr<Dali::WebEngineLoadError> loadError(new TizenWebEngineLoadError(error));
  DALI_LOG_RELEASE_INFO("#LoadError : %s\n", loadError->GetUrl().c_str());
  ExecuteCallback(pThis->mLoadErrorCallback, std::move(loadError));
}

void TizenWebEngineChromium::OnUrlChanged(wv_view_h, void* newUrl, void* data)
{
  auto        pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string url;
  if(newUrl != nullptr)
  {
    url = static_cast<char*>(newUrl);
    DALI_LOG_RELEASE_INFO("#UrlChanged : %s\n", url.c_str());
  }
  ExecuteCallback(pThis->mUrlChangedCallback, url);
}

void TizenWebEngineChromium::OnConsoleMessageReceived(wv_view_h, void* eventInfo, void* data)
{
  auto                                           pThis   = static_cast<TizenWebEngineChromium*>(data);
  wv_console_message_h message = static_cast<wv_console_message_h >(eventInfo);
  std::unique_ptr<Dali::WebEngineConsoleMessage> webConsoleMessage(new TizenWebEngineConsoleMessage(message));
  DALI_LOG_RELEASE_INFO("#ConsoleMessageReceived : %s\n", webConsoleMessage->GetSource().c_str());
  ExecuteCallback(pThis->mConsoleMessageReceivedCallback, std::move(webConsoleMessage));
}

void TizenWebEngineChromium::OnDeviceConnectionChanged(wv_view_h obj, void* info, void* data)
{
  auto pThis       = static_cast<TizenWebEngineChromium*>(data);
  int* device_type = (int*)info;

  DALI_LOG_RELEASE_INFO("#DeviceConnectionChanged : device_type=%d\n", *device_type);
  ExecuteCallback(pThis->mDeviceConnectionChangedCallback, *device_type);
}

void TizenWebEngineChromium::OnDeviceListGet(wv_media_device_info_s* device_list, int size, void* user_data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(user_data);

  pThis->mDeviceListGet = new TizenWebEngineDeviceListGet(device_list, size);
  ExecuteCallback(pThis->mDeviceListGetCallback, pThis->mDeviceListGet, (int32_t)size);
}

void TizenWebEngineChromium::OnEdgeLeft(wv_view_h, void*, void* data)
{
  DALI_LOG_RELEASE_INFO("#ScrollEdgeReached : LEFT\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mScrollEdgeReachedCallback, Dali::WebEnginePlugin::ScrollEdge::LEFT);
}

void TizenWebEngineChromium::OnEdgeRight(wv_view_h, void*, void* data)
{
  DALI_LOG_RELEASE_INFO("#ScrollEdgeReached : RIGHT\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mScrollEdgeReachedCallback, Dali::WebEnginePlugin::ScrollEdge::RIGHT);
}

void TizenWebEngineChromium::OnEdgeTop(wv_view_h, void*, void* data)
{
  DALI_LOG_RELEASE_INFO("#ScrollEdgeReached : TOP\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mScrollEdgeReachedCallback, Dali::WebEnginePlugin::ScrollEdge::TOP);
}

void TizenWebEngineChromium::OnEdgeBottom(wv_view_h, void*, void* data)
{
  DALI_LOG_RELEASE_INFO("#ScrollEdgeReached : BOTTOM\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mScrollEdgeReachedCallback, Dali::WebEnginePlugin::ScrollEdge::BOTTOM);
}

void TizenWebEngineChromium::OnOverScrolledLeft(wv_view_h, void*, void* data)
{
  DALI_LOG_RELEASE_INFO("#OverScrolled : LEFT\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mOverScrolledCallback, Dali::WebEnginePlugin::OverScrolled::LEFT);
}

void TizenWebEngineChromium::OnOverScrolledRight(wv_view_h, void*, void* data)
{
  DALI_LOG_RELEASE_INFO("#OverScrolled : RIGHT\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mOverScrolledCallback, Dali::WebEnginePlugin::OverScrolled::RIGHT);
}

void TizenWebEngineChromium::OnOverScrolledTop(wv_view_h, void*, void* data)
{
  DALI_LOG_RELEASE_INFO("#OverScrolled : TOP\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mOverScrolledCallback, Dali::WebEnginePlugin::OverScrolled::TOP);
}

void TizenWebEngineChromium::OnOverScrolledBottom(wv_view_h, void*, void* data)
{
  DALI_LOG_RELEASE_INFO("#OverScrolled : BOTTOM\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mOverScrolledCallback, Dali::WebEnginePlugin::OverScrolled::BOTTOM);
}

void TizenWebEngineChromium::OnFormRepostDecided(wv_view_h, void* eventInfo, void* data)
{
  DALI_LOG_RELEASE_INFO("#FormRepostDecidedRequest\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  std::unique_ptr<Dali::WebEngineFormRepostDecision> webDecisionRequest(new TizenWebEngineFormRepostDecision(eventInfo));
  ExecuteCallback(pThis->mFormRepostDecidedCallback, std::move(webDecisionRequest));
}

void TizenWebEngineChromium::OnResponsePolicyDecided(wv_view_h, void* policy, void* data)
{
  DALI_LOG_RELEASE_INFO("#ResponsePolicyDecided.\n");
  auto                                           pThis          = static_cast<TizenWebEngineChromium*>(data);
  wv_policy_decision_h policyDecision = static_cast<wv_policy_decision_h >(policy);
  std::unique_ptr<Dali::WebEnginePolicyDecision> webPolicyDecision(new TizenWebEnginePolicyDecision(policyDecision));
  ExecuteCallback(pThis->mResponsePolicyDecidedCallback, std::move(webPolicyDecision));
}

void TizenWebEngineChromium::OnNavigationPolicyDecided(wv_view_h, void* policy, void* data)
{
  DALI_LOG_RELEASE_INFO("#NavigationPolicyDecided.\n");
  auto                                           pThis          = static_cast<TizenWebEngineChromium*>(data);
  wv_policy_decision_h policyDecision = static_cast<wv_policy_decision_h >(policy);
  std::unique_ptr<Dali::WebEnginePolicyDecision> webPolicyDecision(new TizenWebEnginePolicyDecision(policyDecision));
  ExecuteCallback(pThis->mNavigationPolicyDecidedCallback, std::move(webPolicyDecision));
}

void TizenWebEngineChromium::OnNewWindowPolicyDecided(wv_view_h, void* policy, void* data)
{
  DALI_LOG_RELEASE_INFO("#NewWindowPolicyDecided.\n");
  auto                                           pThis          = static_cast<TizenWebEngineChromium*>(data);
  wv_policy_decision_h policyDecision = static_cast<wv_policy_decision_h >(policy);
  std::unique_ptr<Dali::WebEnginePolicyDecision> webPolicyDecision(new TizenWebEnginePolicyDecision(policyDecision));
  ExecuteCallback(pThis->mNewWindowPolicyDecidedCallback, std::move(webPolicyDecision));
}

void TizenWebEngineChromium::OnNewWindowCreated(wv_view_h, void* out_view, void* data)
{
  DALI_LOG_RELEASE_INFO("#NewWindowCreated.\n");
  auto                   pThis     = static_cast<TizenWebEngineChromium*>(data);
  Dali::WebEnginePlugin* outPlugin = nullptr;
  ExecuteCallback(pThis->mNewWindowCreatedCallback, outPlugin);
  if(outPlugin)
  {
    *static_cast<wv_view_h*>(out_view) = WebEngineManager::Get().Find(outPlugin);
  }
  else
  {
    DALI_LOG_ERROR("Failed to create a new window.\n");
  }
}

void TizenWebEngineChromium::OnCertificateConfirmed(wv_view_h, void* eventInfo, void* data)
{
  DALI_LOG_RELEASE_INFO("#CertificateConfirmed.\n");
  auto                                        pThis          = static_cast<TizenWebEngineChromium*>(data);
  wv_certificate_policy_decision_h policyDecision = static_cast<wv_certificate_policy_decision_h >(eventInfo);
  std::unique_ptr<Dali::WebEngineCertificate> webPolicyDecision(new TizenWebEngineCertificate(policyDecision));
  ExecuteCallback(pThis->mCertificateConfirmedCallback, std::move(webPolicyDecision));
}

void TizenWebEngineChromium::OnSslCertificateChanged(wv_view_h, void* eventInfo, void* data)
{
  DALI_LOG_RELEASE_INFO("#SslCertificateChanged.\n");
  auto                                        pThis = static_cast<TizenWebEngineChromium*>(data);
  wv_certificate_info_h info  = static_cast<wv_certificate_info_h >(eventInfo);
  std::unique_ptr<Dali::WebEngineCertificate> webCertiInfo(new TizenWebEngineCertificate(info));
  ExecuteCallback(pThis->mSslCertificateChangedCallback, std::move(webCertiInfo));
}

void TizenWebEngineChromium::OnContextMenuShown(wv_view_h, void* eventInfo, void* data)
{
  DALI_LOG_RELEASE_INFO("#ContextMenuShown.\n");
  auto                                        pThis = static_cast<TizenWebEngineChromium*>(data);
  wv_context_menu_h menu  = (wv_context_menu_h )eventInfo;
  std::unique_ptr<Dali::WebEngineContextMenu> contextMenu(new TizenWebEngineContextMenu(menu));
  ExecuteCallback(pThis->mContextMenuShownCallback, std::move(contextMenu));
}

void TizenWebEngineChromium::OnContextMenuHidden(wv_view_h, void* eventInfo, void* data)
{
  DALI_LOG_RELEASE_INFO("#ContextMenuHidden.\n");
  auto                                        pThis = static_cast<TizenWebEngineChromium*>(data);
  wv_context_menu_h menu  = (wv_context_menu_h )eventInfo;
  std::unique_ptr<Dali::WebEngineContextMenu> contextMenu(new TizenWebEngineContextMenu(menu));
  ExecuteCallback(pThis->mContextMenuHiddenCallback, std::move(contextMenu));
}

void TizenWebEngineChromium::OnFullscreenEntered(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#FullscreenEntered.\n");
  ExecuteCallback(pThis->mFullscreenEnteredCallback);
}

void TizenWebEngineChromium::OnFullscreenExited(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#FullscreenExited.\n");
  ExecuteCallback(pThis->mFullscreenExitedCallback);
}

void TizenWebEngineChromium::OnTextFound(wv_view_h, void* eventInfo, void* data)
{
  auto     pThis = static_cast<TizenWebEngineChromium*>(data);
  uint32_t count = *((uint32_t*)(eventInfo));
  DALI_LOG_RELEASE_INFO("#TextFound, count:%u.\n", count);
  ExecuteCallback(pThis->mTextFoundCallback, count);
}

void TizenWebEngineChromium::OnWebAuthDisplayQR(wv_view_h, void* contents, void* data)
{
  auto        pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string result;
  if(contents != nullptr)
  {
    result = static_cast<char*>(contents);
    DALI_LOG_RELEASE_INFO("#WebAuthDisplayQR : %s\n", result.c_str());
  }
  ExecuteCallback(pThis->mWebAuthDisplayQRCallback, result);
}

void TizenWebEngineChromium::OnWebAuthResponse(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#WebAuthResponse \n");
  ExecuteCallback(pThis->mWebAuthResponseCallback);
}

void TizenWebEngineChromium::OnFileChooserRequested(wv_view_h, void* request, void* data)
{
  DALI_LOG_RELEASE_INFO("#FileChooserRequested.\n");
  auto                                               pThis      = static_cast<TizenWebEngineChromium*>(data);
  wv_file_chooser_request_h wvRequest = (wv_file_chooser_request_h )request;
  std::unique_ptr<Dali::WebEngineFileChooserRequest> engineRequest(new TizenWebEngineFileChooserRequest(wvRequest));
  ExecuteCallback(pThis->mFileChooserRequestedCallback, std::move(engineRequest));
}

void TizenWebEngineChromium::OnWebProcessCrashed(wv_view_h, void*, void* data)
{
  DALI_LOG_RELEASE_INFO("#WebProcessCrashed.\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mWebProcessCrashedCallback);
}

void TizenWebEngineChromium::OnAuthenticationChallenged(wv_view_h, wv_auth_challenge_h authChallenge, void* data)
{
  DALI_LOG_RELEASE_INFO("#AuthenticationChallenged.\n");
  auto                                            pThis = static_cast<TizenWebEngineChromium*>(data);
  std::unique_ptr<Dali::WebEngineHttpAuthHandler> authHandler(new TizenWebEngineHttpAuthHandler(authChallenge));
  ExecuteCallback(pThis->mHttpAuthHandlerCallback, std::move(authHandler));
}

void TizenWebEngineChromium::OnJavaScriptEvaluated(wv_view_h, const char* result, void* data)
{
  auto        pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string jsResult;
  if(result != nullptr)
  {
    jsResult = result;
  }
  ExecuteCallback(pThis->mJavaScriptEvaluatedCallback, jsResult);
}

void TizenWebEngineChromium::OnJavaScriptInjected(wv_view_h o, wv_script_message_s message)
{
  auto plugin = WebEngineManager::Get().Find(o);
  if(plugin)
  {
    auto        pThis = static_cast<TizenWebEngineChromium*>(plugin);
    std::string resultText;
    if(message.body != nullptr)
    {
      resultText = static_cast<char*>(message.body);
    }

    std::string key            = static_cast<const char*>(message.name);
    auto        targetCallback = pThis->mJavaScriptInjectedCallbacks.find(key);

    if(targetCallback != pThis->mJavaScriptInjectedCallbacks.end())
    {
      ExecuteCallback(targetCallback->second, resultText);
    }
  }
}

void TizenWebEngineChromium::OnJavaScriptEntireMessageReceived(wv_view_h o, wv_script_message_s message)
{
  auto plugin = WebEngineManager::Get().Find(o);
  if(plugin)
  {
    auto        pThis       = static_cast<TizenWebEngineChromium*>(plugin);
    std::string messageName = static_cast<const char*>(message.name);
    std::string messageBody;
    if(message.body != nullptr)
    {
      messageBody = static_cast<char*>(message.body);
    }
    ExecuteCallback(pThis->mJavaScriptEntireMessageReceivedCallback, messageName, messageBody);
  }
}

bool TizenWebEngineChromium::OnJavaScriptAlert(wv_view_h o, const char* alert_text, void* data)
{
  auto        pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string alertText;
  if(alert_text != nullptr)
  {
    alertText = alert_text;
  }
  return ExecuteCallbackReturn<bool>(pThis->mJavaScriptAlertCallback, alertText);
}

bool TizenWebEngineChromium::OnJavaScriptConfirm(wv_view_h o, const char* message, void* data)
{
  auto        pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string messageText;
  if(message != nullptr)
  {
    messageText = message;
  }
  return ExecuteCallbackReturn<bool>(pThis->mJavaScriptConfirmCallback, messageText);
}

bool TizenWebEngineChromium::OnJavaScriptPrompt(wv_view_h o, const char* message, const char* default_value, void* data)
{
  auto        pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string messageText;
  if(message != nullptr)
  {
    messageText = message;
  }
  std::string defaultValueText;
  if(default_value != nullptr)
  {
    defaultValueText = default_value;
  }
  return ExecuteCallbackReturn<bool>(pThis->mJavaScriptPromptCallback, messageText, defaultValueText);
}

void TizenWebEngineChromium::OnHitTestCreated(wv_view_h, int x, int y, wv_hit_test_mode_e hitTestMode, wv_hit_test_h hitTest, void* data)
{
  auto                                    pThis = static_cast<TizenWebEngineChromium*>(data);
  std::unique_ptr<Dali::WebEngineHitTest> webHitTest(new TizenWebEngineHitTest(hitTest, false));
  ExecuteCallbackReturn<bool>(pThis->mHitTestCreatedCallback, std::move(webHitTest));
}

void TizenWebEngineChromium::OnVideoPlaying(wv_view_h, bool isPlaying, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mVideoPlayingCallback, isPlaying);
}

void TizenWebEngineChromium::OnPlainTextReceived(wv_view_h o, const char* plainText, void* data)
{
  auto        pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string resultText;
  if(plainText != nullptr)
  {
    resultText = plainText;
  }
  ExecuteCallback(pThis->mPlainTextReceivedCallback, resultText);
}

void TizenWebEngineChromium::OnGeolocationPermission(wv_view_h, wv_geolocation_permission_request_h request, void* data)
{
  auto                       pThis          = static_cast<TizenWebEngineChromium*>(data);
  const wv_security_origin_h securityOrigin = wv_geolocation_permission_request_origin_get(request);
  std::string                host           = wv_security_origin_host_get(securityOrigin);
  std::string                protocol       = wv_security_origin_protocol_get(securityOrigin);
  // WV GAP (WV_REQUIREMENTS.md D-4): wv_view_geolocation_permission_cb returns
  // void and WV exposes no counterpart to ewk_geolocation_permission_reply(),
  // so the application's allow/deny answer cannot be handed back to the engine
  // and the request falls back to Chromium's default handling.
  ExecuteCallbackReturn<bool>(pThis->mGeolocationPermissionCallback, host, protocol);
}

void TizenWebEngineChromium::OnUserMediaPermissonRequest(wv_view_h, wv_user_media_permission_request_h request, void* data)
{
  auto pThis                            = static_cast<TizenWebEngineChromium*>(data);
  pThis->mWebUserMediaPermissionRequest = new TizenWebEngineUserMediaPermissionRequest(request);

  DALI_LOG_RELEASE_INFO("#UserMediaPermissonRequest: pThis:%p, request:%p\n", pThis, request);

  std::string msg = wv_user_media_permission_request_message_get(request);
  // The decision travels back through wv_user_media_permission_request_set()
  // on the request object, so the callback itself returns nothing.
  ExecuteCallback2(pThis->mUserMediaPermissionRequestCallback, pThis->mWebUserMediaPermissionRequest, msg);

  // The EWK callback returned EINA_FALSE for an empty message, which told the
  // engine to hold the request open until the application answers. The WV
  // callback returns void, so ask for that hold explicitly.
  if(msg.empty())
  {
    wv_user_media_permission_request_suspend(request);
  }
}

void TizenWebEngineChromium::OnScreenshotCaptured(wv_view_h, wv_image_buffer_h image, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  // The runtime frees the handle once this callback returns, so the pixels are
  // copied here and the handle is left alone.
  ExecuteCallback(pThis->mScreenshotCapturedCallback, ConvertImageBuffer(image, false));
}

void TizenWebEngineChromium::OnPlaybackVideoReady(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#PlaybackVideoReady.\n");
  ExecuteCallback(pThis->mPlaybackVideoReadyCallback);
}

void TizenWebEngineChromium::OnPlaybackVideoStarted(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#PlaybackVideoStarted.\n");
  ExecuteCallback(pThis->mPlaybackVideoStartedCallback);
}

void TizenWebEngineChromium::OnPlaybackVideoFinished(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#PlaybackVideoFinished.\n");
  ExecuteCallback(pThis->mPlaybackVideoFinishedCallback);
}

void TizenWebEngineChromium::OnPlaybackVideoStopped(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#PlaybackVideoStopped.\n");
  ExecuteCallback(pThis->mPlaybackVideoStoppedCallback);
}

void TizenWebEngineChromium::OnPlaybackVideoPaused(wv_view_h, void*, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#PlaybackVideoPaused.\n");
  ExecuteCallback(pThis->mPlaybackVideoPausedCallback);
}

} // namespace Plugin
} // namespace Dali

extern "C" DALI_EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin()
{
  return new Dali::Plugin::TizenWebEngineChromium();
}

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin(Dali::WebEnginePlugin* plugin)
{
  if(plugin)
  {
    delete plugin;
  }
}

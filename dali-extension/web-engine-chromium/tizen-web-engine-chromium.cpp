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

#include "tizen-web-engine-chromium.h"

#include "tizen-web-engine-back-forward-list.h"
#include "tizen-web-engine-certificate.h"
#include "tizen-web-engine-console-message.h"
#include "tizen-web-engine-context-menu-item.h"
#include "tizen-web-engine-context-menu.h"
#include "tizen-web-engine-form-repost-decision.h"
#include "tizen-web-engine-hit-test.h"
#include "tizen-web-engine-http-auth-handler.h"
#include "tizen-web-engine-load-error.h"
#include "tizen-web-engine-manager.h"
#include "tizen-web-engine-policy-decision.h"
#include "tizen-web-engine-settings.h"

#include <Ecore_Evas.h>
#include <Ecore_Wl2.h>
#include <Ecore_Input_Evas.h>

#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/pixel-data.h>

#include <ewk_main_internal.h>
#include <ewk_settings_product.h>
#include <ewk_view_product.h>

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

template <typename Callback, typename Arg>
void ExecuteCallback(Callback callback, Arg*& arg)
{
  if (callback)
  {
    callback(arg);
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
: mDaliImageSrc(NativeImageSource::New(0, 0, NativeImageSource::COLOR_DEPTH_DEFAULT)),
  mWebView(nullptr),
  mWidth(0),
  mHeight(0)
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
  WebEngineManager::Get().Add(mWebView, this);
}

void TizenWebEngineChromium::Create(uint32_t width, uint32_t height, uint32_t argc, char** argv)
{
  // This API must be called at first.
  ewk_set_arguments(argc, argv);

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
  WebEngineManager::Get().Add(mWebView, this);
}

void TizenWebEngineChromium::InitWebView(bool incognito)
{
  Ewk_Context* context = nullptr;
  if(incognito)
  {
    mWebView = ewk_view_add_in_incognito_mode(ecore_evas_get(WebEngineManager::Get().GetWindow()));
    context  = ewk_view_context_get(mWebView);
  }
  else
  {
    context  = ewk_context_default_get();
    mWebView = ewk_view_add(ecore_evas_get(WebEngineManager::Get().GetWindow()));
  }
  WebEngineManager::Get().SetContext(context);
  ewk_context_max_refresh_rate_set(context, 60);
  ewk_view_offscreen_rendering_enabled_set(mWebView, true);

  Ecore_Wl2_Window* win = AnyCast<Ecore_Wl2_Window*>(Adaptor::Get().GetNativeWindowHandle());
  ewk_view_ime_window_set(mWebView, win);

  Ewk_Settings* settings = ewk_view_settings_get(mWebView);
  mWebEngineSettings.reset(new TizenWebEngineSettings(settings));

  Ewk_Back_Forward_List* backForwardList = ewk_view_back_forward_list_get(mWebView);
  mWebEngineBackForwardList.reset(new TizenWebEngineBackForwardList(backForwardList));

  ewk_settings_viewport_meta_tag_set(settings, false);

  evas_object_smart_callback_add(mWebView, "offscreen,frame,rendered", &TizenWebEngineChromium::OnFrameRendered, this);
  evas_object_smart_callback_add(mWebView, "load,started", &TizenWebEngineChromium::OnLoadStarted, this);
  evas_object_smart_callback_add(mWebView, "load,progress", &TizenWebEngineChromium::OnLoadInProgress, this);
  evas_object_smart_callback_add(mWebView, "load,finished", &TizenWebEngineChromium::OnLoadFinished, this);
  evas_object_smart_callback_add(mWebView, "load,error", &TizenWebEngineChromium::OnLoadError, this);
  evas_object_smart_callback_add(mWebView, "url,changed", &TizenWebEngineChromium::OnUrlChanged, this);
  evas_object_smart_callback_add(mWebView, "console,message", &TizenWebEngineChromium::OnConsoleMessageReceived, this);
  evas_object_smart_callback_add(mWebView, "edge,left", &TizenWebEngineChromium::OnEdgeLeft, this);
  evas_object_smart_callback_add(mWebView, "edge,right", &TizenWebEngineChromium::OnEdgeRight, this);
  evas_object_smart_callback_add(mWebView, "edge,top", &TizenWebEngineChromium::OnEdgeTop, this);
  evas_object_smart_callback_add(mWebView, "edge,bottom", &TizenWebEngineChromium::OnEdgeBottom, this);
  evas_object_smart_callback_add(mWebView, "form,repost,warning,show", &TizenWebEngineChromium::OnFormRepostDecided, this);
  evas_object_smart_callback_add(mWebView, "policy,response,decide", &TizenWebEngineChromium::OnResponsePolicyDecided, this);
  evas_object_smart_callback_add(mWebView, "policy,navigation,decide", &TizenWebEngineChromium::OnNavigationPolicyDecided, this);
  evas_object_smart_callback_add(mWebView, "policy,newwindow,decide", &TizenWebEngineChromium::OnNewWindowPolicyDecided, this);
  evas_object_smart_callback_add(mWebView, "create,window", &TizenWebEngineChromium::OnNewWindowCreated, this);
  evas_object_smart_callback_add(mWebView, "request,certificate,confirm", &TizenWebEngineChromium::OnCertificateConfirmed, this);
  evas_object_smart_callback_add(mWebView, "ssl,certificate,changed", &TizenWebEngineChromium::OnSslCertificateChanged, this);
  evas_object_smart_callback_add(mWebView, "contextmenu,show", &TizenWebEngineChromium::OnContextMenuShown, this);
  evas_object_smart_callback_add(mWebView, "contextmenu,hide", &TizenWebEngineChromium::OnContextMenuHidden, this);
  evas_object_smart_callback_add(mWebView, "fullscreen,enterfullscreen", &TizenWebEngineChromium::OnFullscreenEntered, this);
  evas_object_smart_callback_add(mWebView, "fullscreen,exitfullscreen", &TizenWebEngineChromium::OnFullscreenExited, this);
  evas_object_smart_callback_add(mWebView, "text,found", &TizenWebEngineChromium::OnTextFound, this);
  evas_object_smart_callback_add(mWebView, "webauth,display,qr", &TizenWebEngineChromium::OnWebAuthDisplayQR, this);
  evas_object_smart_callback_add(mWebView, "webauth,response", &TizenWebEngineChromium::OnWebAuthResponse, this);

  evas_object_resize(mWebView, mWidth, mHeight);
  evas_object_show(mWebView);
}

void TizenWebEngineChromium::Destroy()
{
  mJavaScriptInjectedCallbacks.clear();

  if(WebEngineManager::IsAvailable())
  {
    WebEngineManager::Get().Remove(mWebView);
  }
  evas_object_del(mWebView);
  mWebView = nullptr;
}

void TizenWebEngineChromium::LoadUrl(const std::string& path)
{
  ewk_view_url_set(mWebView, path.c_str());
}

std::string TizenWebEngineChromium::GetTitle() const
{
  const char* title = ewk_view_title_get(mWebView);
  return title ? std::string(title) : std::string();
}

Dali::PixelData TizenWebEngineChromium::GetFavicon() const
{
  Evas_Object* iconObject = ewk_view_favicon_get(mWebView);
  if(!iconObject)
  {
    return Dali::PixelData();
  }
  return ConvertImageColorSpace(iconObject);
}

NativeImageSourcePtr TizenWebEngineChromium::GetNativeImageSource()
{
  return mDaliImageSrc;
}

void TizenWebEngineChromium::ChangeOrientation(int orientation)
{
  DALI_LOG_RELEASE_INFO("#ChangeOrientation : %d\n", orientation);
  ewk_view_orientation_send(mWebView, orientation);
}

std::string TizenWebEngineChromium::GetUrl() const
{
  const char* url = ewk_view_url_get(mWebView);
  return url ? std::string(url) : std::string();
}

void TizenWebEngineChromium::LoadHtmlString(const std::string& html)
{
  ewk_view_html_string_load(mWebView, html.c_str(), 0, 0);
}

bool TizenWebEngineChromium::LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri, const std::string& unreachableUrl)
{
  char* cBasicUri       = basicUri.length() ? (char*)basicUri.c_str() : nullptr;
  char* cUnreachableUrl = unreachableUrl.length() ? (char*)unreachableUrl.c_str() : nullptr;
  return ewk_view_html_string_override_current_entry_load(mWebView, html.c_str(), cBasicUri, cUnreachableUrl);
}

bool TizenWebEngineChromium::LoadContents(const int8_t* contents, uint32_t contentSize, const std::string& mimeType, const std::string& encoding, const std::string& baseUri)
{
  char* cMimeType = mimeType.length() ? (char*)mimeType.c_str() : nullptr;
  char* cEncoding = encoding.length() ? (char*)encoding.c_str() : nullptr;
  char* cBaseUri  = baseUri.length() ? (char*)baseUri.c_str() : nullptr;
  return ewk_view_contents_set(mWebView, (const char*)contents, contentSize, cMimeType, cEncoding, cBaseUri);
}

void TizenWebEngineChromium::Reload()
{
  ewk_view_reload(mWebView);
}

bool TizenWebEngineChromium::ReloadWithoutCache()
{
  return ewk_view_reload_bypass_cache(mWebView);
}

void TizenWebEngineChromium::StopLoading()
{
  ewk_view_stop(mWebView);
}

void TizenWebEngineChromium::Suspend()
{
  ewk_view_suspend(mWebView);
}

void TizenWebEngineChromium::Resume()
{
  ewk_view_resume(mWebView);
}

void TizenWebEngineChromium::SuspendNetworkLoading()
{
  ewk_view_suspend_network_loading(mWebView);
}

void TizenWebEngineChromium::ResumeNetworkLoading()
{
  ewk_view_resume_network_loading(mWebView);
}

bool TizenWebEngineChromium::AddCustomHeader(const std::string& name, const std::string& value)
{
  return ewk_view_custom_header_add(mWebView, name.c_str(), value.c_str());
}

bool TizenWebEngineChromium::RemoveCustomHeader(const std::string& name)
{
  return ewk_view_custom_header_remove(mWebView, name.c_str());
}

uint32_t TizenWebEngineChromium::StartInspectorServer(uint32_t port)
{
  return ewk_view_inspector_server_start(mWebView, port);
}

bool TizenWebEngineChromium::StopInspectorServer()
{
  return ewk_view_inspector_server_stop(mWebView);
}

void TizenWebEngineChromium::ScrollBy(int32_t deltaX, int32_t deltaY)
{
  ewk_view_scroll_by(mWebView, deltaX, deltaY);
}

bool TizenWebEngineChromium::ScrollEdgeBy(int32_t deltaX, int32_t deltaY)
{
  return ewk_view_edge_scroll_by(mWebView, deltaX, deltaY);
}

void TizenWebEngineChromium::SetScrollPosition(int32_t x, int32_t y)
{
  ewk_view_scroll_set(mWebView, x, y);
}

Dali::Vector2 TizenWebEngineChromium::GetScrollPosition() const
{
  int x = 0, y = 0;
  ewk_view_scroll_pos_get(mWebView, &x, &y);
  return Dali::Vector2(x, y);
}

Dali::Vector2 TizenWebEngineChromium::GetScrollSize() const
{
  int width = 0, height = 0;
  ewk_view_scroll_size_get(mWebView, &width, &height);
  return Dali::Vector2(width, height);
}

Dali::Vector2 TizenWebEngineChromium::GetContentSize() const
{
  int width = 0, height = 0;
  ewk_view_contents_size_get(mWebView, &width, &height);
  return Dali::Vector2(width, height);
}

bool TizenWebEngineChromium::CanGoForward()
{
  return ewk_view_forward_possible(mWebView);
}

void TizenWebEngineChromium::GoForward()
{
  ewk_view_forward(mWebView);
}

bool TizenWebEngineChromium::CanGoBack()
{
  return ewk_view_back_possible(mWebView);
}

void TizenWebEngineChromium::GoBack()
{
  ewk_view_back(mWebView);
}

void TizenWebEngineChromium::EvaluateJavaScript(const std::string& script, JavaScriptMessageHandlerCallback resultHandler)
{
  mJavaScriptEvaluatedCallback = resultHandler;
  ewk_view_script_execute(mWebView, script.c_str(), &TizenWebEngineChromium::OnJavaScriptEvaluated, this);
}

void TizenWebEngineChromium::AddJavaScriptMessageHandler(const std::string& exposedObjectName, JavaScriptMessageHandlerCallback handler)
{
  mJavaScriptInjectedCallbacks.erase(exposedObjectName);
  mJavaScriptInjectedCallbacks.insert(std::pair<std::string, JavaScriptMessageHandlerCallback>(exposedObjectName, handler));
  ewk_view_javascript_message_handler_add(mWebView, &TizenWebEngineChromium::OnJavaScriptInjected, exposedObjectName.c_str());
}

void TizenWebEngineChromium::RegisterJavaScriptAlertCallback(JavaScriptAlertCallback callback)
{
  mJavaScriptAlertCallback = callback;
  if (mJavaScriptAlertCallback)
  {
    ewk_view_javascript_alert_callback_set(mWebView, &TizenWebEngineChromium::OnJavaScriptAlert, this);
  }
  else
  {
    ewk_view_javascript_alert_callback_set(mWebView, nullptr, nullptr);
  }
}

void TizenWebEngineChromium::JavaScriptAlertReply()
{
  ewk_view_javascript_alert_reply(mWebView);
}

void TizenWebEngineChromium::RegisterJavaScriptConfirmCallback(JavaScriptConfirmCallback callback)
{
  mJavaScriptConfirmCallback = callback;
  if (mJavaScriptConfirmCallback)
  {
    ewk_view_javascript_confirm_callback_set(mWebView, &TizenWebEngineChromium::OnJavaScriptConfirm, this);
  }
  else
  {
    ewk_view_javascript_confirm_callback_set(mWebView, nullptr, nullptr);
  }
}

void TizenWebEngineChromium::JavaScriptConfirmReply(bool confirmed)
{
  ewk_view_javascript_confirm_reply(mWebView, confirmed);
}

void TizenWebEngineChromium::RegisterJavaScriptPromptCallback(JavaScriptPromptCallback callback)
{
  mJavaScriptPromptCallback = callback;
  if (mJavaScriptPromptCallback)
  {
    ewk_view_javascript_prompt_callback_set(mWebView, &TizenWebEngineChromium::OnJavaScriptPrompt, this);
  }
  else
  {
    ewk_view_javascript_prompt_callback_set(mWebView, nullptr, nullptr);
  }
}

void TizenWebEngineChromium::JavaScriptPromptReply(const std::string& result)
{
  ewk_view_javascript_prompt_reply(mWebView, result.c_str());
}

void TizenWebEngineChromium::ClearHistory()
{
  ewk_view_back_forward_list_clear(mWebView);
}

void TizenWebEngineChromium::ClearAllTilesResources()
{
  ewk_view_clear_all_tiles_resources(mWebView);
}

std::string TizenWebEngineChromium::GetUserAgent() const
{
  const char* agent = ewk_view_user_agent_get(mWebView);
  return agent ? std::string(agent) : std::string();
}

void TizenWebEngineChromium::SetUserAgent(const std::string& userAgent)
{
  ewk_view_user_agent_set(mWebView, userAgent.c_str());
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
  // sync...
  Evas*                                   evas    = ecore_evas_get(WebEngineManager::Get().GetWindow());
  Ewk_Hit_Test*                           hitTest = ewk_view_hit_test_new(mWebView, x, y, (int)mode);
  std::unique_ptr<Dali::WebEngineHitTest> webHitTest(new TizenWebEngineHitTest(hitTest, evas, true));
  return webHitTest;
}

bool TizenWebEngineChromium::CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, WebEngineHitTestCreatedCallback callback)
{
  mHitTestCreatedCallback = callback;
  bool result             = ewk_view_hit_test_request(mWebView, x, y, (int)mode, &TizenWebEngineChromium::OnHitTestCreated, this);
  return result;
}

void TizenWebEngineChromium::SetSize(uint32_t width, uint32_t height)
{
  mWidth  = width;
  mHeight = height;
  evas_object_resize(mWebView, mWidth, mHeight);
}

void TizenWebEngineChromium::SetDocumentBackgroundColor(Dali::Vector4 color)
{
  ewk_view_bg_color_set(mWebView, color.r * 255, color.g * 255, color.b * 255, color.a * 255);
}

void TizenWebEngineChromium::ClearTilesWhenHidden(bool cleared)
{
  ewk_view_clear_tiles_on_hide_enabled_set(mWebView, cleared);
}

void TizenWebEngineChromium::SetTileCoverAreaMultiplier(float multiplier)
{
  ewk_view_tile_cover_area_multiplier_set(mWebView, multiplier);
}

void TizenWebEngineChromium::EnableCursorByClient(bool enabled)
{
  ewk_view_set_cursor_by_client(mWebView, enabled);
}

std::string TizenWebEngineChromium::GetSelectedText() const
{
  const char* text = ewk_view_text_selection_text_get(mWebView);
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
  Ewk_Mouse_Button_Type type = (Ewk_Mouse_Button_Type)0;
  switch(touch.GetMouseButton(0))
  {
    case MouseButton::PRIMARY:
    {
      type = EWK_Mouse_Button_Left;
      break;
    }
    case MouseButton::TERTIARY:
    {
      type = EWK_Mouse_Button_Middle;
      break;
    }
    case MouseButton::SECONDARY:
    {
      type = EWK_Mouse_Button_Right;
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
      ewk_view_feed_mouse_down(mWebView, type, x, y);
      break;
    }
    case PointState::UP:
    {
      float x = touch.GetScreenPosition(0).x;
      float y = touch.GetScreenPosition(0).y;
      ewk_view_feed_mouse_up(mWebView, type, x, y);
      break;
    }
    case PointState::MOTION:
    {
      float x = touch.GetScreenPosition(0).x;
      float y = touch.GetScreenPosition(0).y;
      ewk_view_feed_mouse_move(mWebView, x, y);
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
  Ewk_Touch_Event_Type   type  = EWK_TOUCH_START;
  Evas_Touch_Point_State state = EVAS_TOUCH_POINT_DOWN;

  bool fed = false;
  for(std::size_t i = 0; i < touch.GetPointCount(); i++)
  {
    switch(touch.GetState(i))
    {
      case PointState::DOWN:
      {
        type  = EWK_TOUCH_START;
        state = EVAS_TOUCH_POINT_DOWN;
        break;
      }
      case PointState::UP:
      {
        type  = EWK_TOUCH_END;
        state = EVAS_TOUCH_POINT_UP;
        break;
      }
      case PointState::MOTION:
      {
        type  = EWK_TOUCH_MOVE;
        state = EVAS_TOUCH_POINT_MOVE;
        break;
      }
      case PointState::INTERRUPTED:
      {
        type  = EWK_TOUCH_CANCEL;
        state = EVAS_TOUCH_POINT_CANCEL;
        break;
      }
      default:
      {
        break;
      }
    }

    Eina_List*      pointList = 0;
    Ewk_Touch_Point point;
    point.id    = i;
    point.x     = touch.GetScreenPosition(i).x;
    point.y     = touch.GetScreenPosition(i).y;
    point.state = state;
    pointList   = eina_list_append(pointList, &point);
    fed = ewk_view_feed_touch_event(mWebView, type, pointList, 0);
    eina_list_free(pointList);
    if(!fed)
    {
      break;
    }
  }
  return fed;
}

bool TizenWebEngineChromium::SendKeyEvent(const Dali::KeyEvent& keyEvent)
{
  void* evasKeyEvent = 0;
  if(keyEvent.GetState() == Dali::KeyEvent::DOWN)
  {
    Evas_Event_Key_Down downEvent;
    memset(&downEvent, 0, sizeof(Evas_Event_Key_Down));

    downEvent.timestamp = keyEvent.GetTime();
    downEvent.keyname = const_cast<char *>(keyEvent.GetKeyName().c_str());
    downEvent.key = keyEvent.GetLogicalKey().c_str();
    downEvent.string = keyEvent.GetKeyString().c_str();
    downEvent.keycode = keyEvent.GetKeyCode();
    Evas* evas = ecore_evas_get(WebEngineManager::Get().GetWindow());
    ecore_event_evas_modifier_lock_update(evas, (unsigned int)keyEvent.GetKeyModifier());
    downEvent.modifiers = const_cast<Evas_Modifier*>(evas_key_modifier_get(evas));
    downEvent.locks = const_cast<Evas_Lock*>(evas_key_lock_get(evas));
    downEvent.dev = evas_device_get(evas, keyEvent.GetDeviceName().c_str());

    evasKeyEvent = static_cast<void*>(&downEvent);
    ewk_view_send_key_event(mWebView, evasKeyEvent, true);
  }
  else
  {
    Evas_Event_Key_Up upEvent;
    memset(&upEvent, 0, sizeof(Evas_Event_Key_Up));

    upEvent.timestamp = keyEvent.GetTime();
    upEvent.keyname = const_cast<char *>(keyEvent.GetKeyName().c_str());
    upEvent.key = keyEvent.GetLogicalKey().c_str();
    upEvent.string = keyEvent.GetKeyString().c_str();
    upEvent.keycode = keyEvent.GetKeyCode();
    Evas* evas = ecore_evas_get(WebEngineManager::Get().GetWindow());
    ecore_event_evas_modifier_lock_update(evas, (unsigned int)keyEvent.GetKeyModifier());
    upEvent.modifiers = const_cast<Evas_Modifier*>(evas_key_modifier_get(evas));
    upEvent.locks = const_cast<Evas_Lock*>(evas_key_lock_get(evas));
    upEvent.dev = evas_device_get(evas, keyEvent.GetDeviceName().c_str());

    evasKeyEvent = static_cast<void*>(&upEvent);
    ewk_view_send_key_event(mWebView, evasKeyEvent, false);
  }
  return false;
}

bool TizenWebEngineChromium::SendHoverEvent(const Dali::HoverEvent& event)
{
  return false;
}

bool TizenWebEngineChromium::SendWheelEvent(const Dali::WheelEvent& wheel)
{
  Eina_Bool direction = wheel.GetDirection() ? false : true;
  int       step      = wheel.GetDelta();
  float     x         = wheel.GetPoint().x;
  float     y         = wheel.GetPoint().y;
  ewk_view_feed_mouse_wheel(mWebView, direction, step, x, y);
  return false;
}

void TizenWebEngineChromium::ExitFullscreen()
{
  ewk_view_fullscreen_exit(mWebView);
}

void TizenWebEngineChromium::SetFocus(bool focused)
{
  ecore_evas_focus_set(WebEngineManager::Get().GetWindow(), focused);
  ewk_view_focus_set(mWebView, focused);
}

void TizenWebEngineChromium::EnableMouseEvents(bool enabled)
{
  ewk_view_mouse_events_enabled_set(mWebView, enabled);
}

void TizenWebEngineChromium::EnableKeyEvents(bool enabled)
{
  ewk_view_key_events_enabled_set(mWebView, enabled);
}

void TizenWebEngineChromium::SetPageZoomFactor(float zoomFactor)
{
  ewk_view_page_zoom_set(mWebView, zoomFactor);
}

float TizenWebEngineChromium::GetPageZoomFactor() const
{
  return ewk_view_page_zoom_get(mWebView);
}

void TizenWebEngineChromium::SetTextZoomFactor(float zoomFactor)
{
  ewk_view_text_zoom_set(mWebView, zoomFactor);
}

float TizenWebEngineChromium::GetTextZoomFactor() const
{
  return ewk_view_text_zoom_get(mWebView);
}

float TizenWebEngineChromium::GetLoadProgressPercentage() const
{
  return ewk_view_load_progress_get(mWebView);
}

void TizenWebEngineChromium::SetScaleFactor(float scaleFactor, Dali::Vector2 point)
{
  ewk_view_scale_set(mWebView, scaleFactor, point.x, point.y);
}

float TizenWebEngineChromium::GetScaleFactor() const
{
  return ewk_view_scale_get(mWebView);
}

void TizenWebEngineChromium::ActivateAccessibility(bool activated)
{
  ewk_view_atk_deactivation_by_app(mWebView, !activated);
}

Accessibility::Address TizenWebEngineChromium::GetAccessibilityAddress()
{
  static const char plugIdKey[] = "__PlugID";
  static const char rootPath[]  = "root";

  std::string_view plugId;

  if(auto* data = static_cast<const char*>(evas_object_data_get(mWebView, plugIdKey)))
  {
    plugId = {data};
  }

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
  return ewk_view_visibility_set(mWebView, visible);
}

bool TizenWebEngineChromium::HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount)
{
  return ewk_view_text_find(mWebView, text.c_str(), (Ewk_Find_Options)options, maxMatchCount);
}

void TizenWebEngineChromium::AddDynamicCertificatePath(const std::string& host, const std::string& certPath)
{
  ewk_view_add_dynamic_certificate_path(mWebView, host.c_str(), certPath.c_str());
}

Dali::PixelData TizenWebEngineChromium::GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor)
{
  Eina_Rectangle rect;
  EINA_RECTANGLE_SET(&rect, viewArea.x, viewArea.y, viewArea.width, viewArea.height);
  Evas*        evas           = ecore_evas_get(WebEngineManager::Get().GetWindow());
  Evas_Object* snapShotObject = ewk_view_screenshot_contents_get(mWebView, rect, scaleFactor, evas);
  if(!snapShotObject)
  {
    return Dali::PixelData();
  }
  return ConvertImageColorSpace(snapShotObject);
}

bool TizenWebEngineChromium::GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor, ScreenshotCapturedCallback callback)
{
  mScreenshotCapturedCallback = callback;
  Eina_Rectangle rect;
  EINA_RECTANGLE_SET(&rect, viewArea.x, viewArea.y, viewArea.width, viewArea.height);
  Evas* evas = ecore_evas_get(WebEngineManager::Get().GetWindow());
  return ewk_view_screenshot_contents_get_async(mWebView, rect, scaleFactor, evas, &TizenWebEngineChromium::OnScreenshotCaptured, this);
}

bool TizenWebEngineChromium::CheckVideoPlayingAsynchronously(VideoPlayingCallback callback)
{
  mVideoPlayingCallback = callback;
  return ewk_view_is_video_playing(mWebView, &TizenWebEngineChromium::OnVideoPlaying, this);
}

void TizenWebEngineChromium::GetPlainTextAsynchronously(PlainTextReceivedCallback callback)
{
  mPlainTextReceivedCallback = callback;
  ewk_view_plain_text_get(mWebView, &TizenWebEngineChromium::OnPlainTextReceived, this);
}

void TizenWebEngineChromium::WebAuthenticationCancel()
{
  ewk_view_webauthn_cancel(mWebView);
}

void TizenWebEngineChromium::RegisterWebAuthDisplayQRCallback(WebEngineWebAuthDisplayQRCallback callback)
{
  mWebAuthDisplayQRCallback = callback;
}

void TizenWebEngineChromium::RegisterWebAuthResponseCallback(WebEngineWebAuthResponseCallback callback)
{
  mWebAuthResponseCallback = callback;
}

void TizenWebEngineChromium::RegisterGeolocationPermissionCallback(GeolocationPermissionCallback callback)
{
  mGeolocationPermissionCallback = callback;
  ewk_view_geolocation_permission_callback_set(mWebView, &TizenWebEngineChromium::OnGeolocationPermission, this);
}

void TizenWebEngineChromium::UpdateDisplayArea(Dali::Rect<int32_t> displayArea)
{
  evas_object_move(mWebView, displayArea.x, displayArea.y);
  SetSize(displayArea.width, displayArea.height);
  evas_object_geometry_set(mWebView, displayArea.x, displayArea.y, displayArea.width, displayArea.height);
}

void TizenWebEngineChromium::EnableVideoHole(bool enabled)
{
  Ecore_Wl2_Window* win = AnyCast<Ecore_Wl2_Window*>(Adaptor::Get().GetNativeWindowHandle());
  ewk_view_set_support_video_hole(mWebView, win, enabled, EINA_FALSE);
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
  if (mHttpAuthHandlerCallback)
  {
    ewk_view_authentication_callback_set(mWebView, &TizenWebEngineChromium::OnAuthenticationChallenged, this);
  }
  else
  {
    ewk_view_authentication_callback_set(mWebView, nullptr, nullptr);
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

Dali::PixelData TizenWebEngineChromium::ConvertImageColorSpace(Evas_Object* image)
{
  // color-space is argb8888.
  uint8_t* pixelBuffer = (uint8_t*)evas_object_image_data_get(image, false);
  if(!pixelBuffer)
  {
    return Dali::PixelData();
  }

  int width = 0, height = 0;
  evas_object_image_size_get(image, &width, &height);

  uint32_t bufferSize      = width * height * 4;
  uint8_t* convertedBuffer = new uint8_t[bufferSize];
  std::memcpy(convertedBuffer, pixelBuffer, bufferSize);

  return Dali::PixelData::New(convertedBuffer, bufferSize, width, height, Dali::Pixel::Format::RGBA8888, Dali::PixelData::ReleaseFunction::DELETE_ARRAY);
}

void TizenWebEngineChromium::UpdateImage(tbm_surface_h buffer)
{
  if(!buffer)
  {
    return;
  }
  Any source(buffer);
  mDaliImageSrc->SetSource(source);
  Dali::Stage::GetCurrent().KeepRendering(0.0f);
  ExecuteCallback(mFrameRenderedCallback);
}

void TizenWebEngineChromium::OnFrameRendered(void* data, Evas_Object*, void* buffer)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  pThis->UpdateImage(static_cast<tbm_surface_h>(buffer));
}

void TizenWebEngineChromium::OnLoadStarted(void* data, Evas_Object*, void*)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#LoadStarted : %s\n", pThis->GetUrl().c_str());
  ExecuteCallback(pThis->mLoadStartedCallback, pThis->GetUrl());
}

void TizenWebEngineChromium::OnLoadInProgress(void* data, Evas_Object*, void*)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#LoadInProgress : %s\n", pThis->GetUrl().c_str());
  ExecuteCallback(pThis->mLoadInProgressCallback, pThis->GetUrl().c_str());
}

void TizenWebEngineChromium::OnLoadFinished(void* data, Evas_Object*, void*)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#LoadFinished : %s\n", pThis->GetUrl().c_str());
  ExecuteCallback(pThis->mLoadFinishedCallback, pThis->GetUrl());
}

void TizenWebEngineChromium::OnLoadError(void* data, Evas_Object*, void* rawError)
{
  auto                                      pThis = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Error*                                error = static_cast<Ewk_Error*>(rawError);
  std::unique_ptr<Dali::WebEngineLoadError> loadError(new TizenWebEngineLoadError(error));
  DALI_LOG_RELEASE_INFO("#LoadError : %s\n", loadError->GetUrl().c_str());
  ExecuteCallback(pThis->mLoadErrorCallback, std::move(loadError));
}

void TizenWebEngineChromium::OnUrlChanged(void* data, Evas_Object*, void* newUrl)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string url;
  if (newUrl != nullptr)
  {
    url = static_cast<char*>(newUrl);
    DALI_LOG_RELEASE_INFO("#UrlChanged : %s\n", url.c_str());
  }
  ExecuteCallback(pThis->mUrlChangedCallback, url);
}

void TizenWebEngineChromium::OnConsoleMessageReceived(void* data, Evas_Object*, void* eventInfo)
{
  auto                                           pThis   = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Console_Message*                           message = static_cast<Ewk_Console_Message*>(eventInfo);
  std::unique_ptr<Dali::WebEngineConsoleMessage> webConsoleMessage(new TizenWebEngineConsoleMessage(message));
  DALI_LOG_RELEASE_INFO("#ConsoleMessageReceived : %s\n", webConsoleMessage->GetSource().c_str());
  ExecuteCallback(pThis->mConsoleMessageReceivedCallback, std::move(webConsoleMessage));
}

void TizenWebEngineChromium::OnEdgeLeft(void* data, Evas_Object*, void*)
{
  DALI_LOG_RELEASE_INFO("#ScrollEdgeReached : LEFT\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mScrollEdgeReachedCallback, Dali::WebEnginePlugin::ScrollEdge::LEFT);
}

void TizenWebEngineChromium::OnEdgeRight(void* data, Evas_Object*, void*)
{
  DALI_LOG_RELEASE_INFO("#ScrollEdgeReached : RIGHT\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mScrollEdgeReachedCallback, Dali::WebEnginePlugin::ScrollEdge::RIGHT);
}

void TizenWebEngineChromium::OnEdgeTop(void* data, Evas_Object*, void*)
{
  DALI_LOG_RELEASE_INFO("#ScrollEdgeReached : TOP\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mScrollEdgeReachedCallback, Dali::WebEnginePlugin::ScrollEdge::TOP);
}

void TizenWebEngineChromium::OnEdgeBottom(void* data, Evas_Object*, void*)
{
  DALI_LOG_RELEASE_INFO("#ScrollEdgeReached : BOTTOM\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mScrollEdgeReachedCallback, Dali::WebEnginePlugin::ScrollEdge::BOTTOM);
}

void TizenWebEngineChromium::OnFormRepostDecided(void* data, Evas_Object*, void* eventInfo)
{
  DALI_LOG_RELEASE_INFO("#FormRepostDecidedRequest\n");
  auto                                               pThis           = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Form_Repost_Decision_Request*                  decisionRequest = static_cast<Ewk_Form_Repost_Decision_Request*>(eventInfo);
  std::unique_ptr<Dali::WebEngineFormRepostDecision> webDecisionRequest(new TizenWebEngineFormRepostDecision(decisionRequest));
  ExecuteCallback(pThis->mFormRepostDecidedCallback, std::move(webDecisionRequest));
}

void TizenWebEngineChromium::OnResponsePolicyDecided(void* data, Evas_Object*, void* policy)
{
  DALI_LOG_RELEASE_INFO("#ResponsePolicyDecided.\n");
  auto                                           pThis          = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Policy_Decision*                           policyDecision = static_cast<Ewk_Policy_Decision*>(policy);
  std::unique_ptr<Dali::WebEnginePolicyDecision> webPolicyDecision(new TizenWebEnginePolicyDecision(policyDecision));
  ExecuteCallback(pThis->mResponsePolicyDecidedCallback, std::move(webPolicyDecision));
}

void TizenWebEngineChromium::OnNavigationPolicyDecided(void* data, Evas_Object*, void* policy)
{
  DALI_LOG_RELEASE_INFO("#NavigationPolicyDecided.\n");
  auto                                           pThis          = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Policy_Decision*                           policyDecision = static_cast<Ewk_Policy_Decision*>(policy);
  std::unique_ptr<Dali::WebEnginePolicyDecision> webPolicyDecision(new TizenWebEnginePolicyDecision(policyDecision));
  ExecuteCallback(pThis->mNavigationPolicyDecidedCallback, std::move(webPolicyDecision));
}

void TizenWebEngineChromium::OnNewWindowPolicyDecided(void* data, Evas_Object*, void* policy)
{
  DALI_LOG_RELEASE_INFO("#NewWindowPolicyDecided.\n");
  auto                                           pThis          = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Policy_Decision*                           policyDecision = static_cast<Ewk_Policy_Decision*>(policy);
  std::unique_ptr<Dali::WebEnginePolicyDecision> webPolicyDecision(new TizenWebEnginePolicyDecision(policyDecision));
  ExecuteCallback(pThis->mNewWindowPolicyDecidedCallback, std::move(webPolicyDecision));
}

void TizenWebEngineChromium::OnNewWindowCreated(void* data, Evas_Object*, void* out_view)
{
  DALI_LOG_RELEASE_INFO("#NewWindowCreated.\n");
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  Dali::WebEnginePlugin* outPlugin = nullptr;
  ExecuteCallback(pThis->mNewWindowCreatedCallback, outPlugin);
  if (outPlugin)
  {
    *static_cast<Evas_Object**>(out_view) = WebEngineManager::Get().Find(outPlugin);
  }
  else
  {
    DALI_LOG_ERROR("Failed to create a new window.\n");
  }
}

void TizenWebEngineChromium::OnCertificateConfirmed(void* data, Evas_Object*, void* eventInfo)
{
  DALI_LOG_RELEASE_INFO("#CertificateConfirmed.\n");
  auto                                        pThis          = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Certificate_Policy_Decision*            policyDecision = static_cast<Ewk_Certificate_Policy_Decision*>(eventInfo);
  std::unique_ptr<Dali::WebEngineCertificate> webPolicyDecision(new TizenWebEngineCertificate(policyDecision));
  ExecuteCallback(pThis->mCertificateConfirmedCallback, std::move(webPolicyDecision));
}

void TizenWebEngineChromium::OnSslCertificateChanged(void* data, Evas_Object*, void* eventInfo)
{
  DALI_LOG_RELEASE_INFO("#SslCertificateChanged.\n");
  auto                                        pThis = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Certificate_Info*                       info  = static_cast<Ewk_Certificate_Info*>(eventInfo);
  std::unique_ptr<Dali::WebEngineCertificate> webCertiInfo(new TizenWebEngineCertificate(info));
  ExecuteCallback(pThis->mSslCertificateChangedCallback, std::move(webCertiInfo));
}

void TizenWebEngineChromium::OnContextMenuShown(void* data, Evas_Object*, void* eventInfo)
{
  DALI_LOG_RELEASE_INFO("#ContextMenuShown.\n");
  auto                                        pThis = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Context_Menu*                           menu  = (Ewk_Context_Menu*)eventInfo;
  std::unique_ptr<Dali::WebEngineContextMenu> contextMenu(new TizenWebEngineContextMenu(menu));
  ExecuteCallback(pThis->mContextMenuShownCallback, std::move(contextMenu));
}

void TizenWebEngineChromium::OnContextMenuHidden(void* data, Evas_Object*, void* eventInfo)
{
  auto                                        pThis = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Context_Menu*                           menu  = (Ewk_Context_Menu*)eventInfo;
  std::unique_ptr<Dali::WebEngineContextMenu> contextMenu(new TizenWebEngineContextMenu(menu));
  DALI_LOG_RELEASE_INFO("#ContextMenuHidden.\n");
  ExecuteCallback(pThis->mContextMenuHiddenCallback, std::move(contextMenu));
}

void TizenWebEngineChromium::OnFullscreenEntered(void* data, Evas_Object*, void*)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#FullscreenEntered.\n");
  ExecuteCallback(pThis->mFullscreenEnteredCallback);
}

void TizenWebEngineChromium::OnFullscreenExited(void* data, Evas_Object*, void*)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#FullscreenExited.\n");
  ExecuteCallback(pThis->mFullscreenExitedCallback);
}

void TizenWebEngineChromium::OnTextFound(void* data, Evas_Object*, void* eventInfo)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  uint32_t count = *((uint32_t*)(eventInfo));
  DALI_LOG_RELEASE_INFO("#TextFound, count:%u.\n", count);
  ExecuteCallback(pThis->mTextFoundCallback, count);
}

void TizenWebEngineChromium::OnAuthenticationChallenged(Evas_Object*, Ewk_Auth_Challenge* authChallenge, void* data)
{
  DALI_LOG_RELEASE_INFO("#AuthenticationChallenged.\n");
  auto                                            pThis = static_cast<TizenWebEngineChromium*>(data);
  std::unique_ptr<Dali::WebEngineHttpAuthHandler> authHandler(new TizenWebEngineHttpAuthHandler(authChallenge));
  ExecuteCallback(pThis->mHttpAuthHandlerCallback, std::move(authHandler));
}

void TizenWebEngineChromium::OnJavaScriptEvaluated(Evas_Object*, const char* result, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string jsResult;
  if (result != nullptr)
  {
    jsResult = result;
  }
  ExecuteCallback(pThis->mJavaScriptEvaluatedCallback, jsResult);
}

void TizenWebEngineChromium::OnJavaScriptInjected(Evas_Object* o, Ewk_Script_Message message)
{
  auto plugin = WebEngineManager::Get().Find(o);
  if(plugin)
  {
    auto pThis = static_cast<TizenWebEngineChromium*>(plugin);
    std::string resultText;
    if (message.body != nullptr)
    {
      resultText = static_cast<char*>(message.body);
    }

    std::string key = static_cast<const char*>(message.name);
    auto targetCallback = pThis->mJavaScriptInjectedCallbacks.find(key);

    if (targetCallback != pThis->mJavaScriptInjectedCallbacks.end())
    {
      ExecuteCallback(targetCallback->second, resultText);
    }
  }
}

Eina_Bool TizenWebEngineChromium::OnJavaScriptAlert(Evas_Object* o, const char* alert_text, void* data)
{
  auto pThis  = static_cast<TizenWebEngineChromium*>(data);
  std::string alertText;
  if(alert_text != nullptr)
  {
    alertText = alert_text;
  }
  return ExecuteCallbackReturn<bool>(pThis->mJavaScriptAlertCallback, alertText);
}

Eina_Bool TizenWebEngineChromium::OnJavaScriptConfirm(Evas_Object* o, const char* message, void* data)
{
  auto pThis  = static_cast<TizenWebEngineChromium*>(data);
  std::string messageText;
  if(message != nullptr)
  {
    messageText = message;
  }
  return ExecuteCallbackReturn<bool>(pThis->mJavaScriptConfirmCallback, messageText);
}

Eina_Bool TizenWebEngineChromium::OnJavaScriptPrompt(Evas_Object* o, const char* message, const char* default_value, void* data)
{
  auto pThis  = static_cast<TizenWebEngineChromium*>(data);
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

void TizenWebEngineChromium::OnHitTestCreated(Evas_Object*, int x, int y, int hitTestMode, Ewk_Hit_Test* hitTest, void* data)
{
  auto                                    pThis = static_cast<TizenWebEngineChromium*>(data);
  Evas*                                   evas  = ecore_evas_get(WebEngineManager::Get().GetWindow());
  std::unique_ptr<Dali::WebEngineHitTest> webHitTest(new TizenWebEngineHitTest(hitTest, evas, false));
  ExecuteCallbackReturn<bool>(pThis->mHitTestCreatedCallback, std::move(webHitTest));
}

void TizenWebEngineChromium::OnScreenshotCaptured(Evas_Object* image, void* data)
{
  DALI_LOG_RELEASE_INFO("#ScreenshotCaptured.\n");
  auto            pThis     = static_cast<TizenWebEngineChromium*>(data);
  Dali::PixelData pixelData = ConvertImageColorSpace(image);
  ExecuteCallback(pThis->mScreenshotCapturedCallback, pixelData);
}

void TizenWebEngineChromium::OnVideoPlaying(Evas_Object*, Eina_Bool isPlaying, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  ExecuteCallback(pThis->mVideoPlayingCallback, isPlaying);
}

void TizenWebEngineChromium::OnPlainTextReceived(Evas_Object* o, const char* plainText, void* data)
{
  auto        pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string resultText;
  if(plainText != nullptr)
  {
    resultText = plainText;
  }
  ExecuteCallback(pThis->mPlainTextReceivedCallback, resultText);
}

Eina_Bool TizenWebEngineChromium::OnGeolocationPermission(Evas_Object*, Ewk_Geolocation_Permission_Request* request, void* data)
{
  auto                       pThis          = static_cast<TizenWebEngineChromium*>(data);
  const Ewk_Security_Origin* securityOrigin = ewk_geolocation_permission_request_origin_get(request);
  std::string                host           = ewk_security_origin_host_get(securityOrigin);
  std::string                protocol       = ewk_security_origin_protocol_get(securityOrigin);
  return ExecuteCallbackReturn<bool>(pThis->mGeolocationPermissionCallback, host, protocol);
}

void TizenWebEngineChromium::OnWebAuthDisplayQR(void* data, Evas_Object*, void* contents)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string result;
  if(contents != nullptr)
  {
    result = static_cast<char*>(contents);
    DALI_LOG_RELEASE_INFO("#WebAuthDisplayQR : %s\n", result.c_str());
  }
  ExecuteCallback(pThis->mWebAuthDisplayQRCallback, result);
}

void TizenWebEngineChromium::OnWebAuthResponse(void* data, Evas_Object*, void*)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#WebAuthResponse \n");
  ExecuteCallback(pThis->mWebAuthResponseCallback);
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

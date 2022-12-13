/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd.
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
#include "tizen-web-engine-context.h"
#include "tizen-web-engine-cookie-manager.h"
#include "tizen-web-engine-manager.h"
#include "tizen-web-engine-policy-decision.h"
#include "tizen-web-engine-settings.h"

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Wl2.h>

#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/public-api/images/pixel-data.h>

#include <ewk_console_message_internal.h>
#include <ewk_error.h>
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

template <typename Callback, typename... Args>
void ExecuteCallback(Callback callback, Args... args)
{
  if (callback)
  {
    callback(args...);
  }
}

template <typename Callback, typename Arg>
void ExecuteCallback(Callback callback, std::unique_ptr<Arg> arg)
{
  if (callback)
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

template <typename Ret, typename Callback, typename... Args>
Ret ExecuteCallbackReturn(Callback callback, Args... args)
{
  Ret returnVal = Ret();
  if (callback)
  {
    returnVal = callback(args...);
  }
  return returnVal;
}

template <typename Ret, typename Callback, typename Arg>
Ret ExecuteCallbackReturn(Callback callback, std::unique_ptr<Arg> arg)
{
  Ret returnVal = Ret();
  if (callback)
  {
    returnVal = callback(std::move(arg));
  }
  return returnVal;
}

} // anonymous namespace.

TizenWebEngineChromium::TizenWebEngineChromium()
: mDaliImageSrc(NativeImageSource::New(0, 0, NativeImageSource::COLOR_DEPTH_DEFAULT))
, mWebView(nullptr)
, mWidth(0)
, mHeight(0)
{
}

TizenWebEngineChromium::~TizenWebEngineChromium()
{
  Destroy();
}

void TizenWebEngineChromium::Create(int width, int height, const std::string& locale, const std::string& timezoneID)
{
  // Check if web engine is available and make sure that web engine is initialized.
  if (!WebEngineManager::IsAvailable())
  {
    DALI_LOG_ERROR("Web engine has been terminated in current process.");
    return;
  }

  mWidth = width;
  mHeight = height;
  InitWebView(0, 0);
  WebEngineManager::Get().Add(mWebView, this);
}

void TizenWebEngineChromium::Create(int width, int height, int argc, char** argv)
{
  // Check if web engine is available and make sure that web engine is initialized.
  if (!WebEngineManager::IsAvailable())
  {
    DALI_LOG_ERROR("Web engine has been terminated in current process.");
    return;
  }

  mWidth = width;
  mHeight = height;
  InitWebView(argc, argv);
  WebEngineManager::Get().Add(mWebView, this);
}

void TizenWebEngineChromium::InitWebView(int argc, char** argv)
{
  if (argc > 0)
  {
    ewk_set_arguments(argc, argv);
  }

  Ecore_Wl2_Window* win = AnyCast<Ecore_Wl2_Window*>(Adaptor::Get().GetNativeWindowHandle());
  Ewk_Context* context = ewk_context_default_get();
  ewk_context_max_refresh_rate_set(context, 60);
  mWebView = ewk_view_add(ecore_evas_get(WebEngineManager::Get().GetWindow()));
  ewk_view_offscreen_rendering_enabled_set(mWebView, true);
  ewk_view_ime_window_set(mWebView, win);
  ewk_view_set_support_video_hole(mWebView, win, EINA_TRUE, EINA_FALSE);

  Ewk_Settings* settings = ewk_view_settings_get(mWebView);
  mWebEngineSettings.reset(new TizenWebEngineSettings(settings));

  context = ewk_view_context_get(mWebView);
  mWebEngineContext.reset(new TizenWebEngineContext(context));

  Ewk_Cookie_Manager* manager = ewk_context_cookie_manager_get(context);
  mWebEngineCookieManager.reset(new TizenWebEngineCookieManager(manager));

  Ewk_Back_Forward_List* backForwardList = ewk_view_back_forward_list_get(mWebView);
  mWebEngineBackForwardList.reset(new TizenWebEngineBackForwardList(backForwardList));

  ewk_settings_viewport_meta_tag_set(settings, false);

  evas_object_smart_callback_add(mWebView, "offscreen,frame,rendered", &TizenWebEngineChromium::OnFrameRendered, this);
  evas_object_smart_callback_add(mWebView, "load,started", &TizenWebEngineChromium::OnLoadStarted, this);
  evas_object_smart_callback_add(mWebView, "load,finished", &TizenWebEngineChromium::OnLoadFinished, this);
  evas_object_smart_callback_add(mWebView, "load,error", &TizenWebEngineChromium::OnLoadError, this);
  evas_object_smart_callback_add(mWebView, "url,changed", &TizenWebEngineChromium::OnUrlChanged, this);
  evas_object_smart_callback_add(mWebView, "console,message", &TizenWebEngineChromium::OnConsoleMessage, this);
  evas_object_smart_callback_add(mWebView, "edge,left", &TizenWebEngineChromium::OnEdgeLeft, this);
  evas_object_smart_callback_add(mWebView, "edge,right", &TizenWebEngineChromium::OnEdgeRight, this);
  evas_object_smart_callback_add(mWebView, "edge,top", &TizenWebEngineChromium::OnEdgeTop, this);
  evas_object_smart_callback_add(mWebView, "edge,bottom", &TizenWebEngineChromium::OnEdgeBottom, this);
  evas_object_smart_callback_add(mWebView, "policy,navigation,decide", &TizenWebEngineChromium::OnNavigationPolicyDecided, this);
  evas_object_smart_callback_add(mWebView, "create,window", &TizenWebEngineChromium::OnNewWindowCreated, this);

  evas_object_resize(mWebView, mWidth, mHeight);
  evas_object_show(mWebView);
}

void TizenWebEngineChromium::Destroy()
{
  if(WebEngineManager::IsAvailable())
  {
    WebEngineManager::Get().Remove(mWebView);
  }
  evas_object_del(mWebView);
  mWebView = nullptr;
}

void TizenWebEngineChromium::LoadUrl(const std::string& url)
{
  ewk_view_url_set(mWebView, url.c_str());
}

std::string TizenWebEngineChromium::GetTitle() const
{
  const char* title = ewk_view_title_get(mWebView);
  return title ? std::string(title) : std::string();
}

Dali::PixelData TizenWebEngineChromium::GetFavicon() const
{
  Evas_Object* iconObject = ewk_view_favicon_get(mWebView);
  if (!iconObject)
  {
    return Dali::PixelData();
  }

  // color-space is argb8888.
  uint8_t* pixelBuffer = (uint8_t*) evas_object_image_data_get(iconObject, false);
  if (!pixelBuffer)
  {
    return Dali::PixelData();
  }

  int width = 0, height = 0;
  evas_object_image_size_get(iconObject, &width, &height);

  uint32_t bufferSize = width * height * 4;
  uint8_t* convertedBuffer = new uint8_t[bufferSize];

  // convert the color-space to rgba8888.
  for(uint32_t i = 0; i < bufferSize; i += 4)
  {
    convertedBuffer[i] = pixelBuffer[i + 1];
    convertedBuffer[i + 1] = pixelBuffer[i + 2];
    convertedBuffer[i + 2] = pixelBuffer[i + 3];
    convertedBuffer[i + 3] = pixelBuffer[i];
  }

  return Dali::PixelData::New(convertedBuffer, bufferSize, width, height, Dali::Pixel::Format::RGBA8888, Dali::PixelData::ReleaseFunction::DELETE_ARRAY);
}

NativeImageInterfacePtr TizenWebEngineChromium::GetNativeImageSource()
{
  return mDaliImageSrc;
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

void TizenWebEngineChromium::Reload()
{
  ewk_view_reload(mWebView);
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

void TizenWebEngineChromium::ScrollBy(int deltaX, int deltaY)
{
  ewk_view_scroll_by(mWebView, deltaX, deltaY);
}

void TizenWebEngineChromium::SetScrollPosition(int x, int y)
{
  ewk_view_scroll_set(mWebView, x, y);
}

void TizenWebEngineChromium::GetScrollPosition(int& x, int& y) const
{
  ewk_view_scroll_pos_get(mWebView, &x, &y);
}

void TizenWebEngineChromium::GetScrollSize(int& width, int& height) const
{
  ewk_view_scroll_size_get(mWebView, &width, &height);
}

void TizenWebEngineChromium::GetContentSize(int& width, int& height) const
{
  ewk_view_contents_size_get(mWebView, &width, &height);
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
  mJavaScriptInjectedCallback = handler;
  ewk_view_javascript_message_handler_add(mWebView, &TizenWebEngineChromium::OnJavaScriptInjected, exposedObjectName.c_str());
}

void TizenWebEngineChromium::ClearAllTilesResources()
{
  ewk_view_clear_all_tiles_resources(mWebView);
}

void TizenWebEngineChromium::ClearHistory()
{
  ewk_view_back_forward_list_clear(mWebView);
}

std::string TizenWebEngineChromium::GetUserAgent() const
{
  const char* ua = ewk_view_user_agent_get(mWebView);
  return ua ? std::string(ua) : std::string();
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

Dali::WebEngineContext& TizenWebEngineChromium::GetContext() const
{
  static TizenWebEngineContext dummy( nullptr );
  return mWebEngineContext ? *mWebEngineContext : dummy;
}

Dali::WebEngineCookieManager& TizenWebEngineChromium::GetCookieManager() const
{
  static TizenWebEngineCookieManager dummy( nullptr );
  return mWebEngineCookieManager ? *mWebEngineCookieManager : dummy;
}

Dali::WebEngineBackForwardList& TizenWebEngineChromium::GetBackForwardList() const
{
  static TizenWebEngineBackForwardList dummy(nullptr);
  return mWebEngineBackForwardList ? *mWebEngineBackForwardList : dummy;
}

void TizenWebEngineChromium::SetSize(int width, int height)
{
  mWidth = width;
  mHeight = height;
  evas_object_resize(mWebView, mWidth, mHeight);
}

bool TizenWebEngineChromium::SendTouchEvent(const Dali::TouchEvent& touch)
{
  Ewk_Touch_Event_Type type = EWK_TOUCH_START;
  Evas_Touch_Point_State state = EVAS_TOUCH_POINT_DOWN;
  switch (touch.GetState(0))
  {
    case PointState::DOWN:
    {
      type = EWK_TOUCH_START;
      state = EVAS_TOUCH_POINT_DOWN;
      break;
    }
    case PointState::UP:
    {
      type = EWK_TOUCH_END;
      state = EVAS_TOUCH_POINT_UP;
      break;
    }
    case PointState::MOTION:
    {
      type = EWK_TOUCH_MOVE;
      state = EVAS_TOUCH_POINT_MOVE;
      break;
    }
    case PointState::INTERRUPTED:
    {
      type = EWK_TOUCH_CANCEL;
      state = EVAS_TOUCH_POINT_CANCEL;
      break;
    }
    default:
    {
      break;
    }
  }

  Eina_List* pointList = 0;
  Ewk_Touch_Point point;
  point.id = 0;
  point.x = touch.GetScreenPosition(0).x;
  point.y = touch.GetScreenPosition(0).y;
  point.state = state;
  pointList = eina_list_append(pointList, &point);

  bool fed = ewk_view_feed_touch_event(mWebView, type, pointList, 0);
  eina_list_free(pointList);
  return fed;
}

bool TizenWebEngineChromium::SendKeyEvent(const Dali::KeyEvent& keyEvent)
{
  if (keyEvent.GetState() == Dali::KeyEvent::DOWN)
  {
    Evas_Event_Key_Down downEvent;
    memset(&downEvent, 0, sizeof(Evas_Event_Key_Down));
    downEvent.key = keyEvent.GetKeyName().c_str();
    downEvent.string = keyEvent.GetKeyString().c_str();
    void* evasKeyEvent = static_cast<void*>(&downEvent);
    ewk_view_send_key_event(mWebView, evasKeyEvent, true);
  }
  else
  {
    Evas_Event_Key_Up upEvent;
    memset(&upEvent, 0, sizeof(Evas_Event_Key_Up));
    upEvent.key = keyEvent.GetKeyName().c_str();
    upEvent.string = keyEvent.GetKeyString().c_str();
    void* evasKeyEvent = static_cast<void*>(&upEvent);
    ewk_view_send_key_event(mWebView, evasKeyEvent, false);
  }
  return false;
}

void TizenWebEngineChromium::SetFocus(bool focused)
{
  ecore_evas_focus_set(WebEngineManager::Get().GetWindow(), focused);
  ewk_view_focus_set(mWebView, focused);
}

void TizenWebEngineChromium::UpdateDisplayArea(Dali::Rect<int> displayArea)
{
  evas_object_move(mWebView, displayArea.x, displayArea.y);
  SetSize(displayArea.width, displayArea.height );
  evas_object_geometry_set(mWebView, displayArea.x, displayArea.y, displayArea.width, displayArea.height);
}

void TizenWebEngineChromium::EnableVideoHole(bool enabled)
{
  Ecore_Wl2_Window* win = AnyCast<Ecore_Wl2_Window*>(Adaptor::Get().GetNativeWindowHandle());
  ewk_view_set_support_video_hole(mWebView, win, enabled, EINA_FALSE);
}

void TizenWebEngineChromium::RegisterPageLoadStartedCallback(WebEnginePageLoadCallback callback)
{
  mLoadStartedCallback = callback;
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

void TizenWebEngineChromium::RegisterNavigationPolicyDecidedCallback(WebEngineNavigationPolicyDecidedCallback callback)
{
  mNavigationPolicyDecidedCallback = callback;
}

void TizenWebEngineChromium::RegisterNewWindowCreatedCallback(WebEngineNewWindowCreatedCallback callback)
{
  mNewWindowCreatedCallback = callback;
}

void TizenWebEngineChromium::RegisterFrameRenderedCallback(WebEngineFrameRenderedCallback callback)
{
  mFrameRenderedCallback = callback;
}

void TizenWebEngineChromium::GetPlainTextAsynchronously(PlainTextReceivedCallback callback)
{
  mPlainTextReceivedCallback = callback;
  ewk_view_plain_text_get(mWebView, &TizenWebEngineChromium::OnPlainTextReceived, this);
}

void TizenWebEngineChromium::UpdateImage(tbm_surface_h buffer)
{
  if(!buffer)
  {
    return;
  }
  DALI_LOG_RELEASE_INFO("#UpdateImage : %s\n", GetUrl().c_str());
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

void TizenWebEngineChromium::OnLoadFinished(void* data, Evas_Object*, void*)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  DALI_LOG_RELEASE_INFO("#LoadFinished : %s\n", pThis->GetUrl().c_str());
  ExecuteCallback(pThis->mLoadFinishedCallback, pThis->GetUrl());
}

void TizenWebEngineChromium::OnLoadError(void* data, Evas_Object*, void* rawError)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Error* error = static_cast<Ewk_Error*>(rawError);
  std::string stdUrl;
  const char* url = ewk_error_url_get(error);
  if (url != nullptr)
  {
    stdUrl = url;
    DALI_LOG_RELEASE_INFO("#LoadError : %s\n", stdUrl.c_str());
  }
  int errorCode = ewk_error_code_get(error);
  ExecuteCallback(pThis->mLoadErrorCallback, stdUrl, errorCode);
}

void TizenWebEngineChromium::OnConsoleMessage(void*, Evas_Object*, void* eventInfo)
{
  Ewk_Console_Message* message = (Ewk_Console_Message*)eventInfo;
  DALI_LOG_RELEASE_INFO("console message:%s: %d: %d: %s",
      ewk_console_message_source_get(message),
      ewk_console_message_line_get(message),
      ewk_console_message_level_get(message),
      ewk_console_message_text_get(message));
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

void TizenWebEngineChromium::OnNavigationPolicyDecided(void* data, Evas_Object*, void* policy)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  Ewk_Policy_Decision* policyDecision = static_cast<Ewk_Policy_Decision*>(policy);
  std::unique_ptr<Dali::WebEnginePolicyDecision> webPolicyDecision(new TizenWebEnginePolicyDecision(policyDecision));
  ExecuteCallback(pThis->mNavigationPolicyDecidedCallback, std::move(webPolicyDecision));
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
    ExecuteCallback(pThis->mJavaScriptInjectedCallback, resultText);
  }
}

void TizenWebEngineChromium::OnPlainTextReceived(Evas_Object* o, const char* plainText, void* data)
{
  auto pThis = static_cast<TizenWebEngineChromium*>(data);
  std::string resultText;
  if (plainText != nullptr)
  {
    resultText = plainText;
  }
  ExecuteCallback(pThis->mPlainTextReceivedCallback, resultText);
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

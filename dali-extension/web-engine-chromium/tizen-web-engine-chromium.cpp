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

#include "tizen-web-engine-chromium.h"

#include "tizen-web-engine-back-forward-list.h"
#include "tizen-web-engine-certificate.h"
#include "tizen-web-engine-console-message.h"
#include "tizen-web-engine-context.h"
#include "tizen-web-engine-context-menu.h"
#include "tizen-web-engine-context-menu-item.h"
#include "tizen-web-engine-cookie-manager.h"
#include "tizen-web-engine-form-repost-decision.h"
#include "tizen-web-engine-hit-test.h"
#include "tizen-web-engine-http-auth-handler.h"
#include "tizen-web-engine-load-error.h"
#include "tizen-web-engine-policy-decision.h"
#include "tizen-web-engine-request-interceptor.h"
#include "tizen-web-engine-settings.h"

#include <Ecore_Evas.h>
#include <Ecore_Wl2.h>
#include <Elementary.h>
#include <Evas.h>

#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/images/pixel-data.h>

#include <ewk_main_internal.h>
#include <ewk_settings_product.h>
#include <ewk_view_internal.h>
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

}

class WebViewContainerClientPair
{
public:
  WebViewContainerClientPair(WebViewContainerClient* client, Evas_Object* webView)
  {
    mClient = client;
    mWebView = webView;
  }

  WebViewContainerClient* mClient;
  Evas_Object*            mWebView;
};

//
// A class for managing multiple WebViews
//
class WebEngineManager
{
public:
  static WebEngineManager& Get()
  {
    static WebEngineManager instance;
    return instance;
  }

  WebEngineManager(WebEngineManager const&) = delete;

  void operator=(WebEngineManager const&) = delete;

  Ecore_Evas* GetWindow()
  {
    return mWindow;
  }

  void AddContainerClient(WebViewContainerClient* client, Evas_Object* webView)
  {
    mContainerClients.push_back(WebViewContainerClientPair(client, webView));
  }

  void RemoveContainerClient(Evas_Object* webView)
  {
    for (auto it = mContainerClients.begin(); it != mContainerClients.end();)
    {
      if ((*it).mWebView == webView)
      {
        mContainerClients.erase(it);
        break;
      }
      else
      {
        ++it;
      }
    }
  }

  Evas_Object* FindWebView(WebViewContainerClient* client)
  {
    for (auto& webViewClient : mContainerClients)
    {
      if (webViewClient.mClient == client)
      {
        return webViewClient.mWebView;
      }
    }
    return nullptr;
  }

  WebViewContainerClient* FindContainerClient(Evas_Object* o)
  {
    for (auto& webViewClient : mContainerClients)
    {
      if (webViewClient.mWebView == o)
      {
        return webViewClient.mClient;
      }
    }
    return nullptr;
  }

private:
  WebEngineManager()
  {
    elm_init(0, 0);
    ewk_init();
    mWindow = ecore_evas_new("wayland_egl", 0, 0, 1, 1, 0);
  }

  Ecore_Evas* mWindow;
  std::vector<WebViewContainerClientPair> mContainerClients;
};

class WebViewContainerForDali
{
public:
  WebViewContainerForDali(WebViewContainerClient& client, uint32_t width, uint32_t height)
    : mWebView(nullptr)
    , mClient(client)
    , mWidth(width)
    , mHeight(height)
    , mWebEngineSettings(0)
    , mWebEngineContext(0)
    , mWebEngineCookieManager(0)
    , mWebEngineBackForwardList(0)
  {
    InitWebView(0, 0);

    WebEngineManager::Get().AddContainerClient(&mClient, mWebView);
  }

  WebViewContainerForDali(WebViewContainerClient& client, uint32_t width, uint32_t height, uint32_t argc, char** argv)
    : mWebView(nullptr)
    , mClient(client)
    , mWidth(width)
    , mHeight(height)
    , mWebEngineSettings(0)
    , mWebEngineContext(0)
    , mWebEngineCookieManager(0)
    , mWebEngineBackForwardList(0)
  {
    InitWebView(argc, argv);

    WebEngineManager::Get().AddContainerClient(&mClient, mWebView);
  }

  ~WebViewContainerForDali()
  {
    WebEngineManager::Get().RemoveContainerClient(mWebView);

    evas_object_del(mWebView);
  }

  void InitWebView(uint32_t argc, char** argv)
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
    ecore_wl2_window_alpha_set(win, true);
    ewk_view_ime_window_set(mWebView, win);

    Ewk_Settings* settings = ewk_view_settings_get(mWebView);
    mWebEngineSettings = TizenWebEngineSettings(settings);

    context = ewk_view_context_get(mWebView);
    mWebEngineContext = TizenWebEngineContext(context);

    Ewk_Cookie_Manager* manager = ewk_context_cookie_manager_get(context);
    mWebEngineCookieManager = TizenWebEngineCookieManager(manager);

    Ewk_Back_Forward_List* backForwardList = ewk_view_back_forward_list_get(mWebView);
    mWebEngineBackForwardList = TizenWebEngineBackForwardList(backForwardList);

    ewk_settings_viewport_meta_tag_set(settings, false);

    evas_object_smart_callback_add(mWebView, "offscreen,frame,rendered",
                                   &WebViewContainerForDali::OnFrameRendered,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "load,started",
                                   &WebViewContainerForDali::OnLoadStarted,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "load,progress",
                                   &WebViewContainerForDali::OnLoadInProgress,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "load,finished",
                                   &WebViewContainerForDali::OnLoadFinished,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "load,error",
                                   &WebViewContainerForDali::OnLoadError,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "url,changed",
                                   &WebViewContainerForDali::OnUrlChanged,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "console,message",
                                   &WebViewContainerForDali::OnConsoleMessageReceived,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "edge,left",
                                   &WebViewContainerForDali::OnEdgeLeft,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "edge,right",
                                   &WebViewContainerForDali::OnEdgeRight,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "edge,top",
                                   &WebViewContainerForDali::OnEdgeTop,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "edge,bottom",
                                   &WebViewContainerForDali::OnEdgeBottom,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "form,repost,warning,show",
                                   &WebViewContainerForDali::OnFormRepostDecided,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "policy,response,decide",
                                   &WebViewContainerForDali::OnResponsePolicyDecided,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "request,certificate,confirm",
                                   &WebViewContainerForDali::OnCertificateConfirmed,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "ssl,certificate,changed",
                                   &WebViewContainerForDali::OnSslCertificateChanged,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "contextmenu,show",
                                   &WebViewContainerForDali::OnContextMenuShown,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "contextmenu,hide",
                                   &WebViewContainerForDali::OnContextMenuHidden,
                                   &mClient);

    ewk_view_authentication_callback_set(mWebView, &WebViewContainerForDali::OnAuthenticationChallenge, &mClient);

    evas_object_resize(mWebView, mWidth, mHeight);
    evas_object_show(mWebView);
  }

  void LoadUrl(const std::string& url)
  {
    ewk_view_url_set(mWebView, url.c_str());
  }

  void LoadHtml(const std::string& html)
  {
    ewk_view_html_string_load(mWebView, html.c_str(), 0, 0);
  }

  std::string GetUrl()
  {
    const char* url = ewk_view_url_get(mWebView);
    return url ? std::string(url) : std::string();
  }

  bool LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri,
                                          const std::string& unreachableUrl)
  {
    return ewk_view_html_string_override_current_entry_load(mWebView, html.c_str(), basicUri.c_str(), unreachableUrl.c_str());
  }

  bool LoadContents(const std::string& contents, uint32_t contentSize, const std::string& mimeType,
                    const std::string& encoding, const std::string& baseUri)
  {
    return ewk_view_contents_set(mWebView, contents.c_str(), contentSize, (char *)mimeType.c_str(),
                                 (char *)encoding.c_str(), (char *)baseUri.c_str());
  }

  std::string GetTitle()
  {
    const char* title = ewk_view_title_get(mWebView);
    return title ? std::string(title) : std::string();
  }

  Dali::PixelData GetFavicon()
  {
    Evas_Object* iconObject = ewk_view_favicon_get(mWebView);
    if (!iconObject)
    {
      return Dali::PixelData();
    }
    return ConvertImageColorSpace(iconObject);
  }

  void Reload()
  {
    ewk_view_reload(mWebView);
  }

  void StopLoading()
  {
    ewk_view_stop(mWebView);
  }

  bool ReloadWithoutCache()
  {
    return ewk_view_reload_bypass_cache(mWebView);
  }

  void Suspend()
  {
    ewk_view_suspend(mWebView);
  }

  void Resume()
  {
    ewk_view_resume(mWebView);
  }

  void SuspendNetworkLoading()
  {
    ewk_view_suspend_network_loading(mWebView);
  }

  void ResumeNetworkLoading()
  {
    ewk_view_resume_network_loading(mWebView);
  }

  bool AddCustomHeader(const std::string& name, const std::string& value)
  {
    return ewk_view_custom_header_add(mWebView, name.c_str(), value.c_str());
  }

  bool RemoveCustomHeader(const std::string& name)
  {
    return ewk_view_custom_header_remove(mWebView, name.c_str());
  }

  uint32_t StartInspectorServer(uint32_t port)
  {
    return ewk_view_inspector_server_start(mWebView, port);
  }

  bool StopInspectorServer()
  {
    return ewk_view_inspector_server_stop(mWebView);
  }

  void ScrollBy(int32_t deltaX, int32_t deltaY)
  {
    ewk_view_scroll_by(mWebView, deltaX, deltaY);
  }

  bool ScrollEdgeBy(int32_t deltaX, int32_t deltaY)
  {
    return ewk_view_edge_scroll_by(mWebView, deltaX, deltaY);
  }

  void SetScrollPosition(int32_t x, int32_t y)
  {
    ewk_view_scroll_set(mWebView, x, y);
  }

  Dali::Vector2 GetScrollPosition() const
  {
    int x = 0, y = 0;
    ewk_view_scroll_pos_get(mWebView, &x, &y);
    return Dali::Vector2(x, y);
  }

  Dali::Vector2 GetScrollSize() const
  {
    int width = 0, height = 0;
    ewk_view_scroll_size_get(mWebView, &width, &height);
    return Dali::Vector2(width, height);
  }

  Dali::Vector2 GetContentSize() const
  {
    int width = 0, height = 0;
    ewk_view_contents_size_get(mWebView, &width, &height);
    return Dali::Vector2(width, height);
  }

  void GoBack()
  {
    ewk_view_back(mWebView);
  }

  void GoForward()
  {
    ewk_view_forward(mWebView);
  }

  bool CanGoBack()
  {
    return ewk_view_back_possible(mWebView);
  }

  bool CanGoForward()
  {
    return ewk_view_forward_possible(mWebView);
  }

  void EvaluateJavaScript(size_t key, const std::string& script)
  {
    ewk_view_script_execute(mWebView, script.c_str(), OnEvaluateJavaScript, (void*)key);
  }

  void AddJavaScriptMessageHandler(const std::string& exposedObjectName)
  {
    // |jsFunctionName| is fixed as "postMessage" so we don't use this.
    ewk_view_javascript_message_handler_add(mWebView, &WebViewContainerForDali::OnJavaScriptMessage, exposedObjectName.c_str());
  }

  void RegisterJavaScriptAlertCallback()
  {
    ewk_view_javascript_alert_callback_set(mWebView, &WebViewContainerForDali::OnJavaScriptAlert, 0);
  }

  void JavaScriptAlertReply()
  {
    ewk_view_javascript_alert_reply(mWebView);
  }

  void RegisterJavaScriptConfirmCallback()
  {
    ewk_view_javascript_confirm_callback_set(mWebView, &WebViewContainerForDali::OnJavaScriptConfirm, 0);
  }

  void JavaScriptConfirmReply(bool confirmed)
  {
    ewk_view_javascript_confirm_reply(mWebView, confirmed);
  }

  void RegisterJavaScriptPromptCallback()
  {
    ewk_view_javascript_prompt_callback_set(mWebView, &WebViewContainerForDali::OnJavaScriptPrompt, 0);
  }

  void JavaScriptPromptReply(const std::string& result)
  {
    ewk_view_javascript_prompt_reply(mWebView, result.c_str());
  }

  void ClearHistory()
  {
    ewk_view_back_forward_list_clear(mWebView);
  }

  void ClearAllTilesResources()
  {
    ewk_view_clear_all_tiles_resources(mWebView);
  }

  std::string GetUserAgent()
  {
    const char* agent = ewk_view_user_agent_get(mWebView);
    return agent ? std::string(agent) : std::string();
  }

  void SetUserAgent(const std::string& userAgent)
  {
    ewk_view_user_agent_set(mWebView, userAgent.c_str());
  }

  Dali::WebEngineSettings& GetSettings()
  {
    return mWebEngineSettings;
  }

  Dali::WebEngineContext& GetContext()
  {
    return mWebEngineContext;
  }

  Dali::WebEngineCookieManager& GetCookieManager()
  {
    return mWebEngineCookieManager;
  }

  Dali::WebEngineBackForwardList& GetBackForwardList()
  {
    return mWebEngineBackForwardList;
  }

  std::unique_ptr<Dali::WebEngineHitTest> CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode)
  {
    // sync...
    Evas* evas = ecore_evas_get(WebEngineManager::Get().GetWindow());
    Ewk_Hit_Test* hitTest = ewk_view_hit_test_new(mWebView, x, y, (int)mode);
    std::unique_ptr<Dali::WebEngineHitTest> webHitTest(new TizenWebEngineHitTest(hitTest, evas, true));
    return webHitTest;
  }

  bool CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode)
  {
    // async...
    return ewk_view_hit_test_request(mWebView, x, y, (int)mode,
                                     &WebViewContainerForDali::OnHitTestCreated,
                                     &mClient);
  }

  void SetSize(uint32_t width, uint32_t height)
  {
    mWidth = width;
    mHeight = height;
    evas_object_resize(mWebView, mWidth, mHeight);
  }

  void EnableMouseEvents(bool enabled)
  {
    ewk_view_mouse_events_enabled_set(mWebView, enabled);
  }

  void EnableKeyEvents(bool enabled)
  {
    ewk_view_key_events_enabled_set(mWebView, enabled);
  }

  void SetDocumentBackgroundColor(Dali::Vector4 color)
  {
    ewk_view_bg_color_set(mWebView, color.r * 255, color.g * 255, color.b * 255, color.a * 255);
  }

  void ClearTilesWhenHidden(bool cleared)
  {
    ewk_view_clear_tiles_on_hide_enabled_set(mWebView, cleared);
  }

  void SetTileCoverAreaMultiplier(float multiplier)
  {
    ewk_view_tile_cover_area_multiplier_set(mWebView, multiplier);
  }

  void EnableCursorByClient(bool enabled)
  {
    ewk_view_set_cursor_by_client(mWebView, enabled);
  }

  std::string GetSelectedText() const
  {
    const char* text = ewk_view_text_selection_text_get(mWebView);
    return text ? std::string(text) : std::string();
  }

  bool SendTouchEvent(const TouchEvent& touch)
  {
    Ewk_Mouse_Button_Type type = (Ewk_Mouse_Button_Type)0;
    switch (touch.GetMouseButton(0))
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

    switch (touch.GetState(0))
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

  bool SendKeyEvent(const Dali::KeyEvent& keyEvent)
  {
    void* evasKeyEvent = 0;
    if (keyEvent.GetState() == Dali::KeyEvent::DOWN)
    {
      Evas_Event_Key_Down downEvent;
      memset(&downEvent, 0, sizeof(Evas_Event_Key_Down));
      downEvent.key = keyEvent.GetKeyName().c_str();
      downEvent.string = keyEvent.GetKeyString().c_str();
      evasKeyEvent = static_cast<void*>(&downEvent);
      ewk_view_send_key_event(mWebView, evasKeyEvent, true);
    }
    else
    {
      Evas_Event_Key_Up upEvent;
      memset(&upEvent, 0, sizeof(Evas_Event_Key_Up));
      upEvent.key = keyEvent.GetKeyName().c_str();
      upEvent.string = keyEvent.GetKeyString().c_str();
      evasKeyEvent = static_cast<void*>(&upEvent);
      ewk_view_send_key_event(mWebView, evasKeyEvent, false);
    }
    return false;
  }

  bool SendWheelEvent(const WheelEvent& wheel)
  {
    Eina_Bool direction = wheel.GetDirection() ? false : true;
    int step = wheel.GetDelta();
    float x = wheel.GetPoint().x;
    float y = wheel.GetPoint().y;
    ewk_view_feed_mouse_wheel(mWebView, direction, step, x, y);
    return false;
  }

  bool SendHoverEvent(const HoverEvent& hover)
  {
    return false;
  }

  void SetFocus(bool focused)
  {
    ecore_evas_focus_set(WebEngineManager::Get().GetWindow(), focused);
    ewk_view_focus_set(mWebView, focused);
  }

  bool SetPageZoomFactor(float zoomFactor)
  {
    return ewk_view_page_zoom_set(mWebView, zoomFactor);
  }

  float GetPageZoomFactor() const
  {
    return ewk_view_page_zoom_get(mWebView);
  }

  bool SetTextZoomFactor(float zoomFactor)
  {
    return ewk_view_text_zoom_set(mWebView, zoomFactor);
  }

  float GetTextZoomFactor() const
  {
    return ewk_view_text_zoom_get(mWebView);
  }

  float GetLoadProgressPercentage() const
  {
    return ewk_view_load_progress_get(mWebView);
  }

  bool SetScaleFactor(float scaleFactor, Dali::Vector2 point)
  {
    return ewk_view_scale_set(mWebView, scaleFactor, point.x, point.y);
  }

  float GetScaleFactor() const
  {
    return ewk_view_scale_get(mWebView);
  }

  void ActivateAccessibility(bool activated)
  {
    ewk_view_atk_deactivation_by_app(mWebView, !activated);
  }

  bool SetVisibility(bool visible)
  {
    return ewk_view_visibility_set(mWebView, visible);
  }

  bool HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount)
  {
    return ewk_view_text_find(mWebView, text.c_str(), (Ewk_Find_Options)options, maxMatchCount);
  }

  void AddDynamicCertificatePath(const std::string& host, const std::string& certPath)
  {
    ewk_view_add_dynamic_certificate_path(mWebView, host.c_str(), certPath.c_str());
  }

  Dali::PixelData GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor)
  {
    Eina_Rectangle rect;
    EINA_RECTANGLE_SET(&rect, viewArea.x, viewArea.y, viewArea.width, viewArea.height);
    Evas *evas = ecore_evas_get(WebEngineManager::Get().GetWindow());
    Evas_Object *snapShotObject = ewk_view_screenshot_contents_get(mWebView, rect, scaleFactor, evas);
    if (!snapShotObject)
    {
      return Dali::PixelData();
    }
    return ConvertImageColorSpace(snapShotObject);
  }

  bool GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor)
  {
    Eina_Rectangle rect;
    EINA_RECTANGLE_SET(&rect, viewArea.x, viewArea.y, viewArea.width, viewArea.height);
    Evas *evas = ecore_evas_get(WebEngineManager::Get().GetWindow());
    return ewk_view_screenshot_contents_get_async(mWebView, rect, scaleFactor, evas, &WebViewContainerForDali::OnScreenshotCaptured, &mClient);
  }

  bool CheckVideoPlayingAsynchronously()
  {
    return ewk_view_is_video_playing(mWebView, &WebViewContainerForDali::OnVideoPlaying, &mClient);
  }

  void RegisterGeolocationPermissionCallback()
  {
    ewk_view_geolocation_permission_callback_set(mWebView, &WebViewContainerForDali::OnGeolocationPermission, &mClient);
  }

  void RegisterRequestInterceptorCallback(bool isRegistered)
  {
    Ewk_Context* context = ewk_view_context_get(mWebView);
    if (isRegistered)
    {
      ewk_context_intercept_request_callback_set(context, &WebViewContainerForDali::OnRequestIntercepted, &mClient);
    }
    else
    {
      ewk_context_intercept_request_callback_set(context, nullptr, nullptr);
    }
  }

  void UpdateDisplayArea(Dali::Rect<int32_t> displayArea)
  {
    evas_object_move(mWebView, displayArea.x, displayArea.y);
    SetSize(displayArea.width, displayArea.height);
    evas_object_geometry_set(mWebView, displayArea.x, displayArea.y, displayArea.width, displayArea.height);
  }

  void EnableVideoHole(bool enabled)
  {
    Ecore_Wl2_Window* win = AnyCast<Ecore_Wl2_Window*>(Adaptor::Get().GetNativeWindowHandle());
    ewk_view_set_support_video_hole(mWebView, win, enabled, EINA_FALSE);
    ecore_wl2_window_alpha_set(win, !enabled);
  }

  void GetPlainTextAsynchronously()
  {
    ewk_view_plain_text_get(mWebView, &WebViewContainerForDali::OnPlainTextReceived, &mClient);
  }

private:
  static Dali::PixelData ConvertImageColorSpace(Evas_Object* image)
  {
    // color-space is argb8888.
    uint8_t* pixelBuffer = (uint8_t*)evas_object_image_data_get(image, false);
    if (!pixelBuffer)
    {
      return Dali::PixelData();
    }

    int width = 0, height = 0;
    evas_object_image_size_get(image, &width, &height);

    uint32_t bufferSize = width * height * 4;
    uint8_t* convertedBuffer = new uint8_t[bufferSize];

    // convert the color-space to rgba8888.
    for (uint32_t i = 0; i < bufferSize; i += 4)
    {
      convertedBuffer[i] = pixelBuffer[i + 1];
      convertedBuffer[i + 1] = pixelBuffer[i + 2];
      convertedBuffer[i + 2] = pixelBuffer[i + 3];
      convertedBuffer[i + 3] = pixelBuffer[i];
    }

    return Dali::PixelData::New(convertedBuffer, bufferSize, width, height,
                                Dali::Pixel::Format::RGBA8888,
                                Dali::PixelData::ReleaseFunction::DELETE_ARRAY);
  }

  static void OnFrameRendered(void* data, Evas_Object*, void* buffer)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->UpdateImage(static_cast<tbm_surface_h>(buffer));
  }

  static void OnLoadStarted(void* data, Evas_Object*, void*)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->LoadStarted();
  }

  static void OnLoadInProgress(void* data, Evas_Object*, void*)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->LoadInProgress();
  }

  static void OnLoadFinished(void* data, Evas_Object*, void*)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->LoadFinished();
  }

  static void OnLoadError(void* data, Evas_Object*, void* rawError)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Error* error = static_cast<Ewk_Error*>(rawError);
    std::unique_ptr<Dali::WebEngineLoadError> loadError(new TizenWebEngineLoadError(error));
    client->LoadError(std::move(loadError));
  }

  static void OnRequestIntercepted(Ewk_Context*, Ewk_Intercept_Request* request, void* data)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    std::unique_ptr<Dali::WebEngineRequestInterceptor> webInterceptor(new TizenWebEngineRequestInterceptor(request));
    client->RequestIntercepted(std::move(webInterceptor));
  }

  static void OnUrlChanged(void* data, Evas_Object*, void* newUrl)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    std::string url = static_cast<char*>(newUrl);
    client->UrlChanged(url);
  }

  static void OnConsoleMessageReceived(void* data, Evas_Object*, void* eventInfo)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Console_Message* message = static_cast<Ewk_Console_Message*>(eventInfo);
    std::unique_ptr<Dali::WebEngineConsoleMessage> webConsoleMessage(new TizenWebEngineConsoleMessage(message));
    client->ConsoleMessageReceived(std::move(webConsoleMessage));
  }

  static void OnPlainTextReceived(Evas_Object* o, const char* plainText, void* data)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    std::string resultText;

    if (plainText != nullptr)
    {
      resultText = std::string(plainText);
    }

    client->PlainTextRecieved(resultText);
  }

  static void OnEdgeLeft(void* data, Evas_Object*, void*)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->ScrollEdgeReached(Dali::WebEnginePlugin::ScrollEdge::LEFT);
  }

  static void OnEdgeRight(void* data, Evas_Object*, void*)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->ScrollEdgeReached(Dali::WebEnginePlugin::ScrollEdge::RIGHT);
  }

  static void OnEdgeTop(void* data, Evas_Object*, void*)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->ScrollEdgeReached(Dali::WebEnginePlugin::ScrollEdge::TOP);
  }

  static void OnEdgeBottom(void* data, Evas_Object*, void*)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->ScrollEdgeReached(Dali::WebEnginePlugin::ScrollEdge::BOTTOM);
  }

  static void OnFormRepostDecided(void* data, Evas_Object*, void* eventInfo)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Form_Repost_Decision_Request* decisionRequest = static_cast<Ewk_Form_Repost_Decision_Request*>(eventInfo);
    std::unique_ptr<Dali::WebEngineFormRepostDecision> webDecisionRequest(new TizenWebEngineFormRepostDecision(decisionRequest));
    client->RequestFormRepostDecided(std::move(webDecisionRequest));
  }

  static void OnScreenshotCaptured(Evas_Object* image, void* data)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Dali::PixelData pixelData = ConvertImageColorSpace(image);
    client->ScreenshotCaptured(pixelData);
  }

  static void OnVideoPlaying(Evas_Object*, Eina_Bool isPlaying, void* data)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->VideoPlaying(isPlaying);
  }

  static Eina_Bool OnGeolocationPermission(Evas_Object*, Ewk_Geolocation_Permission_Request* request, void* data)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    const Ewk_Security_Origin* securityOrigin = ewk_geolocation_permission_request_origin_get(request);
    std::string host = ewk_security_origin_host_get(securityOrigin);
    std::string protocol = ewk_security_origin_protocol_get(securityOrigin);
    return client->GeolocationPermission(host, protocol);
  }

  static void OnResponsePolicyDecided(void* data, Evas_Object*, void* policy)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Policy_Decision* policyDecision = static_cast<Ewk_Policy_Decision*>(policy);
    std::unique_ptr<Dali::WebEnginePolicyDecision> webPolicyDecision(new TizenWebEnginePolicyDecision(policyDecision));
    client->ResponsePolicyDecided(std::move(webPolicyDecision));
  }

  static void OnCertificateConfirmed(void* data, Evas_Object*, void* eventInfo)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Certificate_Policy_Decision* policyDecision = static_cast<Ewk_Certificate_Policy_Decision*>(eventInfo);
    std::unique_ptr<Dali::WebEngineCertificate> webPolicyDecision(new TizenWebEngineCertificate(policyDecision));
    client->CertificateConfirmed(std::move(webPolicyDecision));
  }

  static void OnSslCertificateChanged(void* data, Evas_Object*, void* eventInfo)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Certificate_Info* info = static_cast<Ewk_Certificate_Info*>(eventInfo);
    std::unique_ptr<Dali::WebEngineCertificate> webCertiInfo(new TizenWebEngineCertificate(info));
    client->SslCertificateChanged(std::move(webCertiInfo));
  }

  static void OnAuthenticationChallenge(Evas_Object*, Ewk_Auth_Challenge* authChallenge, void* data)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    std::unique_ptr<Dali::WebEngineHttpAuthHandler> authHandler(new TizenWebEngineHttpAuthHandler(authChallenge));
    client->AuthenticationChallenge(std::move(authHandler));
  }

  static void OnContextMenuShown(void* data, Evas_Object*, void* eventInfo)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Context_Menu* menu = (Ewk_Context_Menu*)eventInfo;
    std::unique_ptr<Dali::WebEngineContextMenu> contextMenu(new TizenWebEngineContextMenu(menu));
    client->ContextMenuShown(std::move(contextMenu));
  }

  static void OnContextMenuHidden(void* data, Evas_Object*, void* eventInfo)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Context_Menu* menu = (Ewk_Context_Menu*)eventInfo;
    std::unique_ptr<Dali::WebEngineContextMenu> contextMenu(new TizenWebEngineContextMenu(menu));
    client->ContextMenuHidden(std::move(contextMenu));
  }

  static void OnEvaluateJavaScript(Evas_Object* o, const char* result, void* data)
  {
    auto client = WebEngineManager::Get().FindContainerClient(o);
    if (client)
    {
      client->RunJavaScriptEvaluationResultHandler((size_t)data, result);
    }
  }

  static void OnJavaScriptMessage(Evas_Object* o, Ewk_Script_Message message)
  {
    auto client = WebEngineManager::Get().FindContainerClient(o);
    if (client)
    {
      client->RunJavaScriptMessageHandler(message.name, static_cast<char*>(message.body));
    }
  }

  static Eina_Bool OnJavaScriptAlert(Evas_Object* o, const char* alert_text, void*)
  {
    bool result = false;
    auto client = WebEngineManager::Get().FindContainerClient(o);
    if (client)
    {
      result = client->JavaScriptAlert(const_cast<char*>(alert_text));
    }
    return result;
  }

  static Eina_Bool OnJavaScriptConfirm(Evas_Object* o, const char* message, void*)
  {
    bool result = false;
    auto client = WebEngineManager::Get().FindContainerClient(o);
    if (client)
    {
      result = client->JavaScriptConfirm(const_cast<char*>(message));
    }
    return result;
  }

  static Eina_Bool OnJavaScriptPrompt(Evas_Object* o, const char* message, const char* default_value, void*)
  {
    bool result = false;
    auto client = WebEngineManager::Get().FindContainerClient(o);
    if (client)
    {
      result = client->JavaScriptPrompt(const_cast<char*>(message), const_cast<char*>(default_value));
    }
    return result;
  }

  static void OnHitTestCreated(Evas_Object*, int x, int y, int hitTestMode, Ewk_Hit_Test* hitTest, void* data)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Evas* evas = ecore_evas_get(WebEngineManager::Get().GetWindow());
    std::unique_ptr<Dali::WebEngineHitTest> webHitTest(new TizenWebEngineHitTest(hitTest, evas, false));
    client->HitTestCreated(std::move(webHitTest));
  }

private:
  Evas_Object*            mWebView;
  WebViewContainerClient& mClient;

  uint32_t    mWidth;
  uint32_t    mHeight;

  TizenWebEngineSettings        mWebEngineSettings;
  TizenWebEngineContext         mWebEngineContext;
  TizenWebEngineCookieManager   mWebEngineCookieManager;
  TizenWebEngineBackForwardList mWebEngineBackForwardList;
};

class TBMSurfaceSourceInitializer
{
public:
  explicit TBMSurfaceSourceInitializer(NativeImageSourcePtr& imageSrc, uint32_t width, uint32_t height)
  {
    mSurface = tbm_surface_create(width, height, TBM_FORMAT_ARGB8888);
    if (!mSurface)
    {
      DALI_LOG_ERROR("Failed to create tbm surface.");
    }

    Any tbmSource(mSurface);
    imageSrc = NativeImageSource::New(tbmSource);
    Any emptySource(0);
    imageSrc->SetSource(emptySource);
  }

  ~TBMSurfaceSourceInitializer()
  {
    if (mSurface)
    {
      if (tbm_surface_destroy(mSurface) != TBM_SURFACE_ERROR_NONE)
      {
        DALI_LOG_ERROR("Failed to destroy tbm surface.");
      }
    }
  }

private:
  tbm_surface_h mSurface;
};

TizenWebEngineChromium::TizenWebEngineChromium()
  : mWebViewContainer(0)
  , mJavaScriptEvaluationCount(0)
{
  EventThreadCallback* callback = new Dali::EventThreadCallback(Dali::MakeCallback(this, &TizenWebEngineChromium::OnRequestInterceptedEventCallback));
  mRequestInterceptorEventTrigger = std::unique_ptr<Dali::EventThreadCallback>(callback);
}

TizenWebEngineChromium::~TizenWebEngineChromium()
{
  Destroy();
}

void TizenWebEngineChromium::Create(uint32_t width, uint32_t height, const std::string& locale, const std::string& timezoneID)
{
  mWebViewContainer = new WebViewContainerForDali(*this, width, height);
  TBMSurfaceSourceInitializer initializer(mDaliImageSrc, width, height);
}

void TizenWebEngineChromium::Create(uint32_t width, uint32_t height, uint32_t argc, char** argv)
{
  mWebViewContainer = new WebViewContainerForDali(*this, width, height, argc, argv);
  TBMSurfaceSourceInitializer initializer(mDaliImageSrc, width, height);
}

void TizenWebEngineChromium::Destroy()
{
  if (mWebViewContainer)
  {
    mJavaScriptEvaluationResultHandlers.clear();
    mJavaScriptMessageHandlers.clear();

    delete mWebViewContainer;
    mWebViewContainer = 0;
  }
}

void TizenWebEngineChromium::LoadUrl(const std::string& path)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->LoadUrl(path);
  }
}

std::string TizenWebEngineChromium::GetTitle() const
{
  return mWebViewContainer ? mWebViewContainer->GetTitle() : std::string();
}

Dali::PixelData TizenWebEngineChromium::GetFavicon() const
{
  return mWebViewContainer ? mWebViewContainer->GetFavicon() : Dali::PixelData();
}

NativeImageInterfacePtr TizenWebEngineChromium::GetNativeImageSource()
{
  return mDaliImageSrc;
}

std::string TizenWebEngineChromium::GetUrl() const
{
  return mWebViewContainer ? mWebViewContainer->GetUrl() : std::string();
}

void TizenWebEngineChromium::LoadHtmlString(const std::string& html)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->LoadHtml(html);
  }
}

bool TizenWebEngineChromium::LoadHtmlStringOverrideCurrentEntry(const std::string& html, const std::string& basicUri,
                                                                const std::string& unreachableUrl)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->LoadHtmlStringOverrideCurrentEntry(html, basicUri, unreachableUrl);
  }
  return false;
}

bool TizenWebEngineChromium::LoadContents(const std::string& contents, uint32_t contentSize, const std::string& mimeType,
                                          const std::string& encoding, const std::string& baseUri)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->LoadContents(contents, contentSize, mimeType, encoding, baseUri);
  }
  return false;
}

void TizenWebEngineChromium::Reload()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->Reload();
  }
}

bool TizenWebEngineChromium::ReloadWithoutCache()
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->ReloadWithoutCache();
  }
  return false;
}

void TizenWebEngineChromium::StopLoading()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->StopLoading();
  }
}

void TizenWebEngineChromium::Suspend()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->Suspend();
  }
}

void TizenWebEngineChromium::Resume()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->Resume();
  }
}

void TizenWebEngineChromium::SuspendNetworkLoading()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->SuspendNetworkLoading();
  }
}

void TizenWebEngineChromium::ResumeNetworkLoading()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->ResumeNetworkLoading();
  }
}

bool TizenWebEngineChromium::AddCustomHeader(const std::string& name, const std::string& value)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->AddCustomHeader(name, value);
  }
  return false;
}

bool TizenWebEngineChromium::RemoveCustomHeader(const std::string& name)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->RemoveCustomHeader(name);
  }
  return false;
}

uint32_t TizenWebEngineChromium::StartInspectorServer(uint32_t port)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->StartInspectorServer(port);
  }
  return 0;
}

bool TizenWebEngineChromium::StopInspectorServer()
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->StopInspectorServer();
  }
  return false;
}

void TizenWebEngineChromium::ScrollBy(int32_t deltaX, int32_t deltaY)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->ScrollBy(deltaX, deltaY);
  }
}

bool TizenWebEngineChromium::ScrollEdgeBy(int32_t deltaX, int32_t deltaY)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->ScrollEdgeBy(deltaX, deltaY);
  }
  return false;
}

void TizenWebEngineChromium::SetScrollPosition(int32_t x, int32_t y)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->SetScrollPosition(x, y);
  }
}

Dali::Vector2 TizenWebEngineChromium::GetScrollPosition() const
{
  return mWebViewContainer ? mWebViewContainer->GetScrollPosition() : Dali::Vector2::ZERO;
}

Dali::Vector2 TizenWebEngineChromium::GetScrollSize() const
{
  return mWebViewContainer ? mWebViewContainer->GetScrollSize() : Dali::Vector2::ZERO;
}

Dali::Vector2 TizenWebEngineChromium::GetContentSize() const
{
  return mWebViewContainer ? mWebViewContainer->GetContentSize() : Dali::Vector2::ZERO;
}

bool TizenWebEngineChromium::CanGoForward()
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->CanGoForward();
  }
  return false;
}

void TizenWebEngineChromium::GoForward()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->GoForward();
  }
}

bool TizenWebEngineChromium::CanGoBack()
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->CanGoBack();
  }
  return false;
}

void TizenWebEngineChromium::GoBack()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->GoBack();
  }
}

void TizenWebEngineChromium::EvaluateJavaScript(const std::string& script, JavaScriptMessageHandlerCallback resultHandler)
{
  if (mWebViewContainer)
  {
    bool badAlloc = false;

    try
    {
      mJavaScriptEvaluationResultHandlers.emplace(mJavaScriptEvaluationCount, resultHandler);
    }
    catch (std::bad_alloc& e)
    {
      badAlloc = true;
      DALI_LOG_ERROR("Too many ongoing JavaScript evaluations.");
    }

    if (!badAlloc)
    {
      mWebViewContainer->EvaluateJavaScript(mJavaScriptEvaluationCount++, script);
    }
  }
}

void TizenWebEngineChromium::AddJavaScriptMessageHandler(const std::string& exposedObjectName, JavaScriptMessageHandlerCallback handler)
{
  if (mWebViewContainer)
  {
    if (mJavaScriptMessageHandlers.emplace(exposedObjectName, handler).second)
    {
      mWebViewContainer->AddJavaScriptMessageHandler(exposedObjectName);
    }
    else
    {
      DALI_LOG_ERROR("Callback for (%s) already exists.", exposedObjectName.c_str());
    }
  }
}

void TizenWebEngineChromium::RegisterJavaScriptAlertCallback(JavaScriptAlertCallback callback)
{
  if (!mWebViewContainer)
    return;

  mJavaScriptAlertCallback = callback;
  mWebViewContainer->RegisterJavaScriptAlertCallback();
}

void TizenWebEngineChromium::JavaScriptAlertReply()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->JavaScriptAlertReply();
  }
}

void TizenWebEngineChromium::RegisterJavaScriptConfirmCallback(JavaScriptConfirmCallback callback)
{
  if (!mWebViewContainer)
    return;

  mJavaScriptConfirmCallback = callback;
  mWebViewContainer->RegisterJavaScriptConfirmCallback();
}

void TizenWebEngineChromium::JavaScriptConfirmReply(bool confirmed)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->JavaScriptConfirmReply(confirmed);
  }
}

void TizenWebEngineChromium::RegisterJavaScriptPromptCallback(JavaScriptPromptCallback callback)
{
  if (!mWebViewContainer)
    return;

  mJavaScriptPromptCallback = callback;
  mWebViewContainer->RegisterJavaScriptPromptCallback();
}

void TizenWebEngineChromium::JavaScriptPromptReply(const std::string& result)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->JavaScriptPromptReply(result);
  }
}

void TizenWebEngineChromium::ClearHistory()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->ClearHistory();
  }
}

void TizenWebEngineChromium::ClearAllTilesResources()
{
  if (mWebViewContainer)
  {
    mWebViewContainer->ClearAllTilesResources();
  }
}

std::string TizenWebEngineChromium::GetUserAgent() const
{
  return mWebViewContainer ? mWebViewContainer->GetUserAgent() : std::string();
}

void TizenWebEngineChromium::SetUserAgent(const std::string& userAgent)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->SetUserAgent(userAgent);
  }
}

Dali::WebEngineSettings& TizenWebEngineChromium::GetSettings() const
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->GetSettings();
  }

  DALI_LOG_ERROR("WebViewContainer is null.");
  static TizenWebEngineSettings dummy(nullptr);

  return dummy;
}

Dali::WebEngineContext& TizenWebEngineChromium::GetContext() const
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->GetContext();
  }

  DALI_LOG_ERROR("WebViewContainer is null.");
  static TizenWebEngineContext dummy(nullptr);

  return dummy;
}

Dali::WebEngineCookieManager& TizenWebEngineChromium::GetCookieManager() const
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->GetCookieManager();
  }

  DALI_LOG_ERROR("WebViewContainer is null.");
  static TizenWebEngineCookieManager dummy(nullptr);

  return dummy;
}

Dali::WebEngineBackForwardList& TizenWebEngineChromium::GetBackForwardList() const
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->GetBackForwardList();
  }

  DALI_LOG_ERROR("WebViewContainer is null.");
  static TizenWebEngineBackForwardList dummy(nullptr);

  return dummy;
}

std::unique_ptr<Dali::WebEngineHitTest> TizenWebEngineChromium::CreateHitTest(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->CreateHitTest(x, y, mode);
  }
  return nullptr;
}

bool TizenWebEngineChromium::CreateHitTestAsynchronously(int32_t x, int32_t y, Dali::WebEngineHitTest::HitTestMode mode, WebEngineHitTestCreatedCallback callback)
{
  bool result = false;
  if (mWebViewContainer)
  {
    mHitTestCreatedCallback = callback;
    result = mWebViewContainer->CreateHitTestAsynchronously(x, y, mode);
  }
  return result;
}

void TizenWebEngineChromium::SetSize(uint32_t width, uint32_t height)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->SetSize(width, height);
  }
}

void TizenWebEngineChromium::SetDocumentBackgroundColor(Dali::Vector4 color)
{
  if(mWebViewContainer)
  {
    mWebViewContainer->SetDocumentBackgroundColor(color);
  }
}

void TizenWebEngineChromium::ClearTilesWhenHidden(bool cleared)
{
  if(mWebViewContainer)
  {
    mWebViewContainer->ClearTilesWhenHidden(cleared);
  }
}

void TizenWebEngineChromium::SetTileCoverAreaMultiplier(float multiplier)
{
  if(mWebViewContainer)
  {
    mWebViewContainer->SetTileCoverAreaMultiplier(multiplier);
  }
}

void TizenWebEngineChromium::EnableCursorByClient(bool enabled)
{
  if(mWebViewContainer)
  {
    mWebViewContainer->EnableCursorByClient(enabled);
  }
}

std::string TizenWebEngineChromium::GetSelectedText() const
{
  return mWebViewContainer? mWebViewContainer->GetSelectedText() : std::string();
}

bool TizenWebEngineChromium::SendTouchEvent(const Dali::TouchEvent& touch)
{
  if(mWebViewContainer)
  {
    return mWebViewContainer->SendTouchEvent(touch);
  }
  return false;
}

bool TizenWebEngineChromium::SendKeyEvent(const Dali::KeyEvent& event)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->SendKeyEvent(event);
  }
  return false;
}

void TizenWebEngineChromium::SetFocus(bool focused)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->SetFocus(focused);
  }
}

void TizenWebEngineChromium::EnableMouseEvents(bool enabled)
{
  if(mWebViewContainer)
  {
    mWebViewContainer->EnableMouseEvents(enabled);
  }
}

void TizenWebEngineChromium::EnableKeyEvents(bool enabled)
{
  if(mWebViewContainer)
  {
    mWebViewContainer->EnableKeyEvents(enabled);
  }
}

void TizenWebEngineChromium::SetPageZoomFactor(float zoomFactor)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->SetPageZoomFactor(zoomFactor);
  }
}

float TizenWebEngineChromium::GetPageZoomFactor() const
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->GetPageZoomFactor();
  }
  return 0.0f;
}

void TizenWebEngineChromium::SetTextZoomFactor(float zoomFactor)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->SetTextZoomFactor(zoomFactor);
  }
}

float TizenWebEngineChromium::GetTextZoomFactor() const
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->GetTextZoomFactor();
  }
  return 0.0f;
}

float TizenWebEngineChromium::GetLoadProgressPercentage() const
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->GetLoadProgressPercentage();
  }
  return 0.0f;
}

void TizenWebEngineChromium::SetScaleFactor(float scaleFactor, Dali::Vector2 point)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->SetScaleFactor(scaleFactor, point);
  }
}

float TizenWebEngineChromium::GetScaleFactor() const
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->GetScaleFactor();
  }
  return 0.0f;
}

void TizenWebEngineChromium::ActivateAccessibility(bool activated)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->ActivateAccessibility(activated);
  }
}

bool TizenWebEngineChromium::SetVisibility(bool visible)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->SetVisibility(visible);
  }
  return false;
}

bool TizenWebEngineChromium::HighlightText(const std::string& text, Dali::WebEnginePlugin::FindOption options, uint32_t maxMatchCount)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->HighlightText(text, options, maxMatchCount);
  }
  return false;
}

void TizenWebEngineChromium::AddDynamicCertificatePath(const std::string& host, const std::string& certPath)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->AddDynamicCertificatePath(host, certPath);
  }
}

Dali::PixelData TizenWebEngineChromium::GetScreenshot(Dali::Rect<int32_t> viewArea, float scaleFactor)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->GetScreenshot(viewArea, scaleFactor);
  }
  return Dali::PixelData();
}

bool TizenWebEngineChromium::GetScreenshotAsynchronously(Dali::Rect<int32_t> viewArea, float scaleFactor, ScreenshotCapturedCallback callback)
{
  if (mWebViewContainer)
  {
    mScreenshotCapturedCallback = callback;
    return mWebViewContainer->GetScreenshotAsynchronously(viewArea, scaleFactor);
  }
  return false;
}

bool TizenWebEngineChromium::CheckVideoPlayingAsynchronously(VideoPlayingCallback callback)
{
  if (mWebViewContainer)
  {
    mVideoPlayingCallback = callback;
    return mWebViewContainer->CheckVideoPlayingAsynchronously();
  }
  return false;
}

void TizenWebEngineChromium::RegisterGeolocationPermissionCallback(GeolocationPermissionCallback callback)
{
  if (mWebViewContainer)
  {
    mGeolocationPermissionCallback = callback;
    mWebViewContainer->RegisterGeolocationPermissionCallback();
  }
}

void TizenWebEngineChromium::UpdateDisplayArea(Dali::Rect<int32_t> displayArea)
{
  if (mWebViewContainer)
  {
    mWebViewContainer->UpdateDisplayArea(displayArea);
  }
}

void TizenWebEngineChromium::EnableVideoHole(bool enabled)
{
  if (mWebViewContainer)
  {
    return mWebViewContainer->EnableVideoHole(enabled);
  }
}

bool TizenWebEngineChromium::SendHoverEvent(const Dali::HoverEvent& event)
{
  if(mWebViewContainer)
  {
    return mWebViewContainer->SendHoverEvent( event );
  }
  return false;
}

bool TizenWebEngineChromium::SendWheelEvent( const Dali::WheelEvent& event )
{
  if(mWebViewContainer)
  {
    return mWebViewContainer->SendWheelEvent( event );
  }
  return false;
}

Dali::WebEnginePlugin::WebEngineFrameRenderedSignalType& TizenWebEngineChromium::FrameRenderedSignal()
{
  return mFrameRenderedSignal;
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

void TizenWebEngineChromium::RegisterRequestInterceptorCallback(WebEngineRequestInterceptorCallback callback)
{
  mRequestInterceptedCallback = callback;
  if (mWebViewContainer)
  {
    bool isRegistered = mRequestInterceptedCallback ? true : false;
    mWebViewContainer->RegisterRequestInterceptorCallback(isRegistered);
  }
}

void TizenWebEngineChromium::RegisterConsoleMessageReceivedCallback(WebEngineConsoleMessageReceivedCallback callback)
{
  mConsoleMessageReceivedCallback = callback;
}

void TizenWebEngineChromium::RegisterResponsePolicyDecidedCallback(WebEngineResponsePolicyDecidedCallback callback)
{
  mResponsePolicyDecidedCallback = callback;
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
}

void TizenWebEngineChromium::RegisterContextMenuShownCallback(WebEngineContextMenuShownCallback callback)
{
  mContextMenuShownCallback = callback;
}

void TizenWebEngineChromium::RegisterContextMenuHiddenCallback(WebEngineContextMenuHiddenCallback callback)
{
  mContextMenuHiddenCallback = callback;
}

void TizenWebEngineChromium::GetPlainTextAsynchronously(PlainTextReceivedCallback callback)
{
  if (mWebViewContainer)
  {
    mPlainTextReceivedCallback = callback;
    mWebViewContainer->GetPlainTextAsynchronously();
  }
}

// WebViewContainerClient Interface
void TizenWebEngineChromium::UpdateImage(tbm_surface_h buffer)
{
  if (!buffer)
  {
    return;
  }

  Any source(buffer);
  mDaliImageSrc->SetSource(source);
  Dali::Stage::GetCurrent().KeepRendering(0.0f);

  mFrameRenderedSignal.Emit();
}

void TizenWebEngineChromium::LoadStarted()
{
  DALI_LOG_RELEASE_INFO("#LoadStarted : %s\n", GetUrl().c_str());
  ExecuteCallback(mLoadStartedCallback, GetUrl());
}

void TizenWebEngineChromium::LoadInProgress()
{
  DALI_LOG_RELEASE_INFO("#LoadInProgress : %s\n", GetUrl().c_str());
  ExecuteCallback(mLoadInProgressCallback, "");
}

void TizenWebEngineChromium::LoadFinished()
{
  DALI_LOG_RELEASE_INFO("#LoadFinished : %s\n", GetUrl().c_str());
  ExecuteCallback(mLoadFinishedCallback, GetUrl());
}

void TizenWebEngineChromium::LoadError(std::unique_ptr<Dali::WebEngineLoadError> error)
{
  DALI_LOG_RELEASE_INFO("#LoadError : %s\n", error->GetUrl().c_str());
  ExecuteCallback(mLoadErrorCallback, std::move(error));
}

void TizenWebEngineChromium::ScrollEdgeReached(Dali::WebEnginePlugin::ScrollEdge edge)
{
  DALI_LOG_RELEASE_INFO("#ScrollEdgeReached : %d\n", edge);
  ExecuteCallback(mScrollEdgeReachedCallback, edge);
}

void TizenWebEngineChromium::RequestIntercepted(std::unique_ptr<Dali::WebEngineRequestInterceptor> interceptor)
{
  {
    Mutex::ScopedLock lock(mMutex);
    mRequestInterceptorQueue.push(std::move(interceptor));
    // Trigger an event on main thread.
    mRequestInterceptorEventTrigger->Trigger();
  }

  // Wait for tasks on main thread and execute tasks.
  TizenWebEngineRequestInterceptor* requestInterceptor = static_cast<TizenWebEngineRequestInterceptor*>(interceptor.get());
  requestInterceptor->WaitAndRunTasks();
}

void TizenWebEngineChromium::OnRequestInterceptedEventCallback()
{
  std::unique_ptr<Dali::WebEngineRequestInterceptor> interceptor;
  {
    Mutex::ScopedLock lock(mMutex);
    interceptor = std::move(mRequestInterceptorQueue.front());
    mRequestInterceptorQueue.pop();
  }

  ExecuteCallback(mRequestInterceptedCallback, std::move(interceptor));

  // Notify ui-thread tasks ready on main thread.
  TizenWebEngineRequestInterceptor* requestInterceptor = static_cast<TizenWebEngineRequestInterceptor*>(interceptor.get());
  requestInterceptor->NotifyTaskReady();
}

void TizenWebEngineChromium::UrlChanged(const std::string& url)
{
  DALI_LOG_RELEASE_INFO("#UrlChanged : %s\n", url.c_str());
  ExecuteCallback(mUrlChangedCallback, url);
}

void TizenWebEngineChromium::RequestFormRepostDecided(std::unique_ptr<Dali::WebEngineFormRepostDecision> decision)
{
  DALI_LOG_RELEASE_INFO("#FormRepostDecidedRequest\n");
  ExecuteCallback(mFormRepostDecidedCallback, std::move(decision));
}

void TizenWebEngineChromium::ConsoleMessageReceived(std::unique_ptr<Dali::WebEngineConsoleMessage> message)
{
  DALI_LOG_RELEASE_INFO("#ConsoleMessageReceived : %s\n", message->GetSource().c_str());
  ExecuteCallback(mConsoleMessageReceivedCallback, std::move(message));
}

void TizenWebEngineChromium::ResponsePolicyDecided(std::unique_ptr<Dali::WebEnginePolicyDecision> decision)
{
  DALI_LOG_RELEASE_INFO("#ResponsePolicyDecided.\n");
  ExecuteCallback(mResponsePolicyDecidedCallback, std::move(decision));
}

void TizenWebEngineChromium::CertificateConfirmed(std::unique_ptr<Dali::WebEngineCertificate> confirm)
{
  DALI_LOG_RELEASE_INFO("#CertificateConfirmed.\n");
  ExecuteCallback(mCertificateConfirmedCallback, std::move(confirm));
}

void TizenWebEngineChromium::SslCertificateChanged(std::unique_ptr<Dali::WebEngineCertificate> info)
{
  DALI_LOG_RELEASE_INFO("#SslCertificateChanged.\n");
  ExecuteCallback(mSslCertificateChangedCallback, std::move(info));
}

void TizenWebEngineChromium::AuthenticationChallenge(std::unique_ptr<Dali::WebEngineHttpAuthHandler> handler)
{
  DALI_LOG_RELEASE_INFO("#AuthenticationChallenge.\n");
  ExecuteCallback(mHttpAuthHandlerCallback, std::move(handler));
}

void TizenWebEngineChromium::ContextMenuShown(std::unique_ptr<Dali::WebEngineContextMenu> menu)
{
  DALI_LOG_RELEASE_INFO("#ContextMenuShown.\n");
  ExecuteCallback(mContextMenuShownCallback, std::move(menu));
}

void TizenWebEngineChromium::ContextMenuHidden(std::unique_ptr<Dali::WebEngineContextMenu> menu)
{
  DALI_LOG_RELEASE_INFO("#ContextMenuHidden.\n");
  ExecuteCallback(mContextMenuHiddenCallback, std::move(menu));
}

void TizenWebEngineChromium::RunJavaScriptEvaluationResultHandler(size_t key, const char* result)
{
  auto handler = mJavaScriptEvaluationResultHandlers.find(key);
  if (handler == mJavaScriptEvaluationResultHandlers.end())
  {
    return;
  }

  if (handler->second)
  {
    std::string stored(result);
    handler->second(stored);
  }

  mJavaScriptEvaluationResultHandlers.erase(handler);
}

void TizenWebEngineChromium::RunJavaScriptMessageHandler(const std::string& objectName, const std::string& message)
{
  auto handler = mJavaScriptMessageHandlers.find(objectName);
  if (handler == mJavaScriptMessageHandlers.end())
  {
    return;
  }

  handler->second(message);
}

bool TizenWebEngineChromium::JavaScriptAlert(const std::string& alert_text)
{
  return ExecuteCallbackReturn<bool>(mJavaScriptAlertCallback, alert_text);
}

bool TizenWebEngineChromium::JavaScriptConfirm(const std::string& message)
{
  return ExecuteCallbackReturn<bool>(mJavaScriptConfirmCallback, message);
}

bool TizenWebEngineChromium::JavaScriptPrompt(const std::string& message, const std::string& default_value)
{
  return ExecuteCallbackReturn<bool>(mJavaScriptPromptCallback, message, default_value);
}

void TizenWebEngineChromium::ScreenshotCaptured(Dali::PixelData pixelData)
{
  DALI_LOG_RELEASE_INFO("#ScreenshotCaptured.\n");
  ExecuteCallback(mScreenshotCapturedCallback, pixelData);
}

void TizenWebEngineChromium::VideoPlaying(bool isPlaying)
{
  ExecuteCallback(mVideoPlayingCallback, isPlaying);
}

bool TizenWebEngineChromium::GeolocationPermission(const std::string& host, const std::string& protocol)
{
  return ExecuteCallbackReturn<bool>(mGeolocationPermissionCallback, host, protocol);
}

bool TizenWebEngineChromium::HitTestCreated(std::unique_ptr<Dali::WebEngineHitTest> hitTest)
{
  return ExecuteCallbackReturn<bool>(mHitTestCreatedCallback, std::move(hitTest));
}

void TizenWebEngineChromium::PlainTextRecieved(const std::string& plainText)
{
  ExecuteCallback(mPlainTextReceivedCallback, plainText);
}

} // namespace Plugin
} // namespace Dali

extern "C" DALI_EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin()
{
  return new Dali::Plugin::TizenWebEngineChromium();
}

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin(Dali::WebEnginePlugin* plugin)
{
  if (plugin)
  {
    delete plugin;
  }
}

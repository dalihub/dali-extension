/*
 * Copyright (c) 2018 Samsung Electronics Co., Ltd.
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

#include <tizen-web-engine-chromium.h>

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Evas.h>
#include <Elementary.h>

#include <EWebKit_internal.h>
#include <EWebKit_product.h>

using namespace Dali;

namespace Dali {
namespace Plugin {
class WebViewContainerForDali {
 public:
  static void InitGalobalIfNeeded() {
    if (!window_) {
      elm_init(0, 0);
      ewk_init();
      window_ = ecore_evas_new("wayland_egl", 0, 0, 1, 1, 0);
    }
  }

  static void ClearGlobalIfNeeded() {
    // What we should clear?
  }

  WebViewContainerForDali(WebViewContainerClient& client, int width, int height)
      : client_(client),
        width_(width),
        height_(height) {
    InitGalobalIfNeeded();
    InitWebView();
    instance_count_++;
  }

  ~WebViewContainerForDali() {
    evas_object_del(webview_);
    ClearGlobalIfNeeded();
    instance_count_--;
  }

  void InitWebView() {
    Ewk_Context* context = ewk_context_default_get();
    ewk_context_max_refresh_rate_set(context, 60);
    webview_ = ewk_view_add(ecore_evas_get(window_));
    ewk_view_offscreen_rendering_enabled_set(webview_, true);

    evas_object_smart_callback_add(webview_, "offscreen,frame,rendered",
                                   &WebViewContainerForDali::OnFrameRendered,
                                   &client_);
    evas_object_smart_callback_add(webview_, "load,started",
                                   &WebViewContainerForDali::OnLoadStarted,
                                   &client_);
    evas_object_smart_callback_add(webview_, "load,finished",
                                   &WebViewContainerForDali::OnLoadFinished,
                                   &client_);
    evas_object_smart_callback_add(webview_, "console,message",
                                   &WebViewContainerForDali::OnConsoleMessage,
                                   this);

    evas_object_resize(webview_, width_, height_);
    evas_object_show(webview_);
  }

  void LoadUrl(const std::string& url) {
    ewk_view_url_set(webview_, url.c_str());
  }

  void LoadHtml(const std::string& html) {
    ewk_view_html_string_load(webview_, html.c_str(), 0, 0);
  }

  std::string GetUrl() {
    return std::string(ewk_view_url_get(webview_));
  }

  void Reload() {
    ewk_view_reload(webview_);
  }

  void StopLoading() {
    ewk_view_stop(webview_);
  }

  void GoBack() {
    ewk_view_back(webview_);
  }

  void GoForward() {
    ewk_view_forward(webview_);
  }

  bool CanGoBack() {
    return ewk_view_back_possible(webview_);
  }

  bool CanGoForward() {
    return ewk_view_forward_possible(webview_);
  }

  void EvaluateJavaScript(const std::string& script) {
    ewk_view_script_execute(webview_, script.c_str(), 0, 0);
  }

  void ClearHistory() {
    ewk_view_back_forward_list_clear(webview_);
  }

  void ClearCache() {
    ewk_context_cache_clear(ewk_view_context_get(webview_));
  }

  void SetSize(int width, int height) {
    width_ = width;
    height_ = height;
    evas_object_resize(webview_, width_, height_);
  }

  bool SendTouchEvent(const TouchData& touch) {
    Ewk_Touch_Event_Type type = EWK_TOUCH_START;
    Evas_Touch_Point_State state = EVAS_TOUCH_POINT_DOWN;
    switch (touch.GetState(0)) {
    case PointState::DOWN:
      type = EWK_TOUCH_START;
      state = EVAS_TOUCH_POINT_DOWN;
      break;
    case PointState::UP:
      type = EWK_TOUCH_END;
      state = EVAS_TOUCH_POINT_UP;
      break;
    case PointState::MOTION:
      type = EWK_TOUCH_MOVE;
      state = EVAS_TOUCH_POINT_MOVE;
      break;
    case PointState::INTERRUPTED:
      type = EWK_TOUCH_CANCEL;
      state = EVAS_TOUCH_POINT_CANCEL;
      break;
    default:
      break;
    }

    Eina_List* point_list = 0;
    Ewk_Touch_Point* point = new Ewk_Touch_Point;
    point->id = 0;
    point->x = touch.GetLocalPosition(0).x;
    point->y = touch.GetLocalPosition(0).y;
    point->state = state;
    point_list = eina_list_append(point_list, point);

    ewk_view_feed_touch_event(webview_, type, point_list, 0);
    eina_list_free(point_list);
    return true;
  }

  bool SendKeyEvent(const KeyEvent& keyEvent) {
    void* key_event = 0;
    if (keyEvent.state == KeyEvent::Down) {
      Evas_Event_Key_Down down_event;
      memset(&down_event, 0, sizeof(Evas_Event_Key_Down));
      down_event.key = keyEvent.keyPressedName.c_str();
      down_event.string = keyEvent.keyPressed.c_str();
      key_event = static_cast<void*>(&down_event);
      ewk_view_send_key_event(webview_, key_event, true);
    } else {
      Evas_Event_Key_Up up_event;
      memset(&up_event, 0, sizeof(Evas_Event_Key_Up));
      up_event.key = keyEvent.keyPressedName.c_str();
      up_event.string = keyEvent.keyPressed.c_str();
      key_event = static_cast<void*>(&up_event);
      ewk_view_send_key_event(webview_, key_event, false);
     }
     return true;
  }

 private:
  static void OnFrameRendered(void* data, Evas_Object*, void* buffer) {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->UpdateImage(static_cast<tbm_surface_h>(buffer));
  }

  static void OnLoadStarted(void* data, Evas_Object*, void*) {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->LoadStarted();
  }

  static void OnLoadFinished(void* data, Evas_Object*, void*) {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->LoadFinished();
  }

  static void OnConsoleMessage(void*, Evas_Object*, void* event_info) {
    Ewk_Console_Message* console_message = (Ewk_Console_Message*)event_info;
    printf("console message:%s:", ewk_console_message_source_get(console_message));
    printf("%d:", ewk_console_message_line_get(console_message));
    printf("%d:", ewk_console_message_level_get(console_message));
    printf("%s\n", ewk_console_message_text_get(console_message));
  }

  static void OnJavaScriptMessage(Evas_Object* o, Ewk_Script_Message message) {
  }

 private:
  static Ecore_Evas* window_;
  static int instance_count_;

  Evas_Object* webview_;
  WebViewContainerClient& client_;

  int width_;
  int height_;
};

Ecore_Evas* WebViewContainerForDali::window_ = 0;
int WebViewContainerForDali::instance_count_ = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
class TBMSurfaceSourceInitializer {
 public:
  explicit TBMSurfaceSourceInitializer(NativeImageSourcePtr& image_src,
                                       int width, int height) {
    surface_ = tbm_surface_create(width, height, TBM_FORMAT_ARGB8888);
    if (!surface_)
      printf("Failed to create tbm surface.");

    Any tbm_source(surface_);
    image_src = NativeImageSource::New(tbm_source);
    Any empty_source(0);
    image_src->SetSource(empty_source);
  }

  ~TBMSurfaceSourceInitializer() {
    if (surface_) {
      if (tbm_surface_destroy(surface_) != TBM_SURFACE_ERROR_NONE)
        printf("Failed to destroy tbm surface.");
    }
  }
 private:
  tbm_surface_h surface_;
};

TizenWebEngine::TizenWebEngine()
    : webview_container_(0) {
}

TizenWebEngine::~TizenWebEngine() {
  Destroy();
}

void TizenWebEngine::Create(int width, int height,
                            const std::string& locale,
                            const std::string& timezoneID) {
  webview_container_ =
      new WebViewContainerForDali(*this, width, height);
  TBMSurfaceSourceInitializer initializer(dali_image_src_, width, height);
}

void TizenWebEngine::Destroy() {
  if (webview_container_) {
    delete webview_container_;
    webview_container_ = 0;
  }
}

void TizenWebEngine::LoadUrl(const std::string& path) {
  if (webview_container_)
    webview_container_->LoadUrl(path);
}

NativeImageInterfacePtr TizenWebEngine::GetNativeImageSource() {
  return dali_image_src_;
}

const std::string& TizenWebEngine::GetUrl() {
  if (webview_container_)
    url_ =  webview_container_->GetUrl();
  return url_;
}

void TizenWebEngine::LoadHTMLString(const std::string& string) {
  if (webview_container_)
    return webview_container_->LoadHtml(string);
}

void TizenWebEngine::Reload() {
  if (webview_container_)
    webview_container_->Reload();
}

void TizenWebEngine::StopLoading() {
  if (webview_container_)
    webview_container_->StopLoading();
}

bool TizenWebEngine::CanGoForward() {
  if (webview_container_)
    return webview_container_->CanGoForward();
  return false;
}

void TizenWebEngine::GoForward() {
  if (webview_container_)
    webview_container_->GoForward();
}

bool TizenWebEngine::CanGoBack() {
  if (webview_container_)
    return webview_container_->CanGoBack();
  return false;
}

void TizenWebEngine::GoBack() {
  if (webview_container_)
    webview_container_->GoBack();
}

void TizenWebEngine::EvaluateJavaScript(const std::string& script) {
  if (webview_container_)
    webview_container_->EvaluateJavaScript(script);
}

void TizenWebEngine::AddJavaScriptInterface(
    const std::string& exposedObjectName,
    const std::string& jsFunctionName,
    std::function<std::string(const std::string&)> cb ) {
  // Not implemented yet.
}

void TizenWebEngine::RemoveJavascriptInterface(
    const std::string& exposedObjectName,
    const std::string& jsFunctionName ) {
  // Not implemented yet.
}

void TizenWebEngine::ClearHistory() {
  if (webview_container_)
    webview_container_->ClearHistory();
}

void TizenWebEngine::ClearCache() {
  if (webview_container_)
    webview_container_->ClearCache();
}

void TizenWebEngine::SetSize(int width, int height) {
  if (webview_container_)
    webview_container_->SetSize(width, height);
}

bool TizenWebEngine::SendTouchEvent(const Dali::TouchData& touch) {
  if (webview_container_)
    return webview_container_->SendTouchEvent(touch);
  return false;
}

bool TizenWebEngine::SendKeyEvent(const Dali::KeyEvent& event) {
  if (webview_container_)
    return webview_container_->SendKeyEvent(event);
  return false;
}

Dali::WebEnginePlugin::WebEngineSignalType& TizenWebEngine::PageLoadStartedSignal() {
  return load_started_signal_;
}

Dali::WebEnginePlugin::WebEngineSignalType& TizenWebEngine::PageLoadFinishedSignal() {
  return load_finished_signal_;
}

// WebViewContainerClient Interface
void TizenWebEngine::UpdateImage(tbm_surface_h buffer) {
  if (!buffer)
    return;

  Any source(buffer);
  dali_image_src_->SetSource(source);
  Dali::Stage::GetCurrent().KeepRendering(0.0f);
}

void TizenWebEngine::LoadStarted() {
  printf("# TizenWebEngine LoadStarted : %s\n", GetUrl().c_str());
  load_started_signal_.Emit(GetUrl());
}

void TizenWebEngine::LoadFinished() {
  printf("# TizenWebEngine LoadFinished : %s\n", GetUrl().c_str());
  load_finished_signal_.Emit(GetUrl());
}
}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXPORT_API __attribute__((__visibility__("default")))
///////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin() {
  return new Dali::Plugin::TizenWebEngine();
}

extern "C" EXPORT_API void DestroyWebEnginePlugin(Dali::WebEnginePlugin* plugin) {
  if (plugin)
    delete plugin;
}

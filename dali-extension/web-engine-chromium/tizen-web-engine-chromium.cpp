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

#include <dali/integration-api/debug.h>
#include <dali/integration-api/adaptors/adaptor.h>

using namespace Dali;

namespace Dali
{
namespace Plugin
{

class WebViewContainerClientPair {
public:
  WebViewContainerClientPair( WebViewContainerClient* client, Evas_Object* webView )
  {
    mClient = client;
    mWebView = webView;
  }

  WebViewContainerClient* mClient;
  Evas_Object* mWebView;
};

class WebEngineManager {
  //
  // A class for managing multiple WebViews
  //
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

  void AddContainerClient( WebViewContainerClient* client, Evas_Object* webView )
  {
    mContainerClients.push_back( WebViewContainerClientPair( client, webView ) );
  }

  void RemoveContainerClient( Evas_Object* webView )
  {
    for( auto it = mContainerClients.begin(); it != mContainerClients.end(); )
    {
      if( (*it).mWebView == webView )
      {
        mContainerClients.erase( it );
        break;
      }
    }
  }

  WebViewContainerClient* FindContainerClient( Evas_Object* o )
  {
    for( auto& webViewClient :  mContainerClients )
    {
      if( webViewClient.mWebView == o )
      {
        return webViewClient.mClient;
      }
    }
    return 0;
  }

private:
  WebEngineManager()
  {
    elm_init( 0, 0 );
    ewk_init();
    mWindow = ecore_evas_new( "wayland_egl", 0, 0, 1, 1, 0 );
  }

  Ecore_Evas* mWindow;
  std::vector< WebViewContainerClientPair > mContainerClients;
};

class WebViewContainerForDali
{
public:
  WebViewContainerForDali( WebViewContainerClient& client, int width, int height )
      : mClient( client ),
        mWidth( width ),
        mHeight( height )
  {
    InitWebView();

    WebEngineManager::Get().AddContainerClient( &mClient, mWebView );
  }

  ~WebViewContainerForDali()
  {
    WebEngineManager::Get().RemoveContainerClient( mWebView );

    evas_object_del( mWebView );
  }

  void InitWebView()
  {
    Ecore_Wl2_Window* win = AnyCast< Ecore_Wl2_Window* >( Adaptor::Get().GetNativeWindowHandle() );
    Ewk_Context* context = ewk_context_default_get();
    ewk_context_max_refresh_rate_set( context, 60 );
    mWebView = ewk_view_add( ecore_evas_get( WebEngineManager::Get().GetWindow() ) );
    ewk_view_offscreen_rendering_enabled_set( mWebView, true );
    ewk_view_ime_window_set( mWebView, win );

    evas_object_smart_callback_add( mWebView, "offscreen,frame,rendered",
                                    &WebViewContainerForDali::OnFrameRendered,
                                    &mClient );
    evas_object_smart_callback_add( mWebView, "load,started",
                                    &WebViewContainerForDali::OnLoadStarted,
                                    &mClient );
    evas_object_smart_callback_add( mWebView, "load,finished",
                                    &WebViewContainerForDali::OnLoadFinished,
                                    &mClient );
    evas_object_smart_callback_add( mWebView, "console,message",
                                    &WebViewContainerForDali::OnConsoleMessage,
                                    this );

    evas_object_resize( mWebView, mWidth, mHeight );
    evas_object_show( mWebView );
  }

  void LoadUrl( const std::string& url )
  {
    ewk_view_url_set( mWebView, url.c_str() );
    ewk_view_focus_set( mWebView, true );
  }

  void LoadHtml( const std::string& html )
  {
    ewk_view_html_string_load( mWebView, html.c_str(), 0, 0 );
  }

  std::string GetUrl()
  {
    return std::string( ewk_view_url_get( mWebView ) );
  }

  void Reload()
  {
    ewk_view_reload( mWebView );
  }

  void StopLoading()
  {
    ewk_view_stop( mWebView );
  }

  void GoBack()
  {
    ewk_view_back( mWebView );
  }

  void GoForward()
  {
    ewk_view_forward( mWebView );
  }

  bool CanGoBack()
  {
    return ewk_view_back_possible( mWebView );
  }

  bool CanGoForward()
  {
    return ewk_view_forward_possible( mWebView );
  }

  void EvaluateJavaScript( const std::string& script )
  {
    ewk_view_script_execute( mWebView, script.c_str(), 0, 0 );
  }

  void AddJavaScriptMessageHandler( const std::string& exposedObjectName )
  {
    // |jsFunctionName| is fixed as "postMessage" so we don't use this.
    ewk_view_javascript_message_handler_add( mWebView, &WebViewContainerForDali::OnJavaScriptMessage, exposedObjectName.c_str() );
  }

  void ClearHistory()
  {
    ewk_view_back_forward_list_clear( mWebView );
  }

  void ClearCache()
  {
    ewk_context_cache_clear( ewk_view_context_get( mWebView ) );
  }

  void SetSize( int width, int height )
  {
    mWidth = width;
    mHeight = height;
    evas_object_resize( mWebView, mWidth, mHeight );
  }

  bool SendTouchEvent( const TouchData& touch )
  {
    Ewk_Touch_Event_Type type = EWK_TOUCH_START;
    Evas_Touch_Point_State state = EVAS_TOUCH_POINT_DOWN;
    switch ( touch.GetState( 0 ) )
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
    Ewk_Touch_Point* point = new Ewk_Touch_Point;
    point->id = 0;
    point->x = touch.GetLocalPosition( 0 ).x;
    point->y = touch.GetLocalPosition( 0 ).y;
    point->state = state;
    pointList = eina_list_append( pointList, point );

    ewk_view_feed_touch_event( mWebView, type, pointList, 0 );
    eina_list_free( pointList );
    return false;
  }

  bool SendKeyEvent( const KeyEvent& keyEvent )
  {
    void* evasKeyEvent = 0;
    if ( keyEvent.state == KeyEvent::Down )
    {
      Evas_Event_Key_Down downEvent;
      memset( &downEvent, 0, sizeof(Evas_Event_Key_Down) );
      downEvent.key = keyEvent.keyPressedName.c_str();
      downEvent.string = keyEvent.keyPressed.c_str();
      evasKeyEvent = static_cast<void*>(&downEvent);
      ewk_view_send_key_event( mWebView, evasKeyEvent, true );
    }
    else
    {
      Evas_Event_Key_Up upEvent;
      memset(&upEvent, 0, sizeof(Evas_Event_Key_Up));
      upEvent.key = keyEvent.keyPressedName.c_str();
      upEvent.string = keyEvent.keyPressed.c_str();
      evasKeyEvent = static_cast<void*>(&upEvent);
      ewk_view_send_key_event( mWebView, evasKeyEvent, false );
     }
     return false;
  }

private:
  static void OnFrameRendered( void* data, Evas_Object*, void* buffer )
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->UpdateImage( static_cast<tbm_surface_h>(buffer) );
  }

  static void OnLoadStarted( void* data, Evas_Object*, void* )
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->LoadStarted();
  }

  static void OnLoadFinished( void* data, Evas_Object*, void* )
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    client->LoadFinished();
  }

  static void OnConsoleMessage( void*, Evas_Object*, void* eventInfo )
  {
    Ewk_Console_Message* message = (Ewk_Console_Message*)eventInfo;
    DALI_LOG_RELEASE_INFO( "console message:%s: %d: %d: %s",
        ewk_console_message_source_get( message ),
        ewk_console_message_line_get( message ),
        ewk_console_message_level_get( message ),
        ewk_console_message_text_get( message ) );
  }

  static void OnJavaScriptMessage( Evas_Object* o, Ewk_Script_Message message )
  {
    auto client = WebEngineManager::Get().FindContainerClient( o );
    if( client )
    {
      client->RunJavaScriptInterfaceCallback( message.name, static_cast<char*>( message.body ) );
    }
  }

private:
  Evas_Object* mWebView;
  WebViewContainerClient& mClient;

  int mWidth;
  int mHeight;
};

class TBMSurfaceSourceInitializer
{
public:
  explicit TBMSurfaceSourceInitializer( NativeImageSourcePtr& imageSrc,
                                        int width, int height )
  {
    mSurface = tbm_surface_create( width, height, TBM_FORMAT_ARGB8888 );
    if ( !mSurface )
    {
      DALI_LOG_ERROR( "Failed to create tbm surface." );
    }

    Any tbmSource( mSurface );
    imageSrc = NativeImageSource::New( tbmSource );
    Any emptySource( 0 );
    imageSrc->SetSource( emptySource );
  }

  ~TBMSurfaceSourceInitializer()
  {
    if ( mSurface )
    {
      if ( tbm_surface_destroy( mSurface ) != TBM_SURFACE_ERROR_NONE )
      {
        DALI_LOG_ERROR( "Failed to destroy tbm surface." );
      }
    }
  }
private:
  tbm_surface_h mSurface;
};

TizenWebEngineChromium::TizenWebEngineChromium()
    : mWebViewContainer( 0 )
{
}

TizenWebEngineChromium::~TizenWebEngineChromium()
{
  Destroy();
}

void TizenWebEngineChromium::Create( int width, int height,
                                     const std::string& locale,
                                     const std::string& timezoneID )
{
  mWebViewContainer =
      new WebViewContainerForDali( *this, width, height );
  TBMSurfaceSourceInitializer initializer( mDaliImageSrc, width, height );
}

void TizenWebEngineChromium::Destroy()
{
  if (mWebViewContainer)
  {
    delete mWebViewContainer;
    mWebViewContainer = 0;
  }
}

void TizenWebEngineChromium::LoadUrl( const std::string& path )
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->LoadUrl( path );
  }
}

NativeImageInterfacePtr TizenWebEngineChromium::GetNativeImageSource()
{
  return mDaliImageSrc;
}

const std::string& TizenWebEngineChromium::GetUrl()
{
  if ( mWebViewContainer )
  {
    mUrl =  mWebViewContainer->GetUrl();
  }
  return mUrl;
}

void TizenWebEngineChromium::LoadHTMLString( const std::string& string )
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->LoadHtml( string );
  }
}

void TizenWebEngineChromium::Reload()
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->Reload();
  }
}

void TizenWebEngineChromium::StopLoading()
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->StopLoading();
  }
}

bool TizenWebEngineChromium::CanGoForward()
{
  if ( mWebViewContainer )
  {
    return mWebViewContainer->CanGoForward();
  }
  return false;
}

void TizenWebEngineChromium::GoForward()
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->GoForward();
  }
}

bool TizenWebEngineChromium::CanGoBack()
{
  if ( mWebViewContainer )
  {
    return mWebViewContainer->CanGoBack();
  }
  return false;
}

void TizenWebEngineChromium::GoBack()
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->GoBack();
  }
}

void TizenWebEngineChromium::EvaluateJavaScript( const std::string& script )
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->EvaluateJavaScript( script );
  }
}

void TizenWebEngineChromium::AddJavaScriptMessageHandler( const std::string& exposedObjectName, std::function< void( const std::string& ) > handler )
{
  if( mWebViewContainer )
  {
    for( auto& callback : mJsCallbacks )
    {
      if( callback.mObjectName == exposedObjectName )
      {
        DALI_LOG_ERROR( "Callback for (%s) already exists.", exposedObjectName.c_str() );
        return;
      }
    }
    mJsCallbacks.push_back( JsCallback( exposedObjectName, handler ) );
    mWebViewContainer->AddJavaScriptMessageHandler( exposedObjectName );
  }
}

void TizenWebEngineChromium::ClearHistory()
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->ClearHistory();
  }
}

void TizenWebEngineChromium::ClearCache()
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->ClearCache();
  }
}

void TizenWebEngineChromium::SetSize( int width, int height )
{
  if ( mWebViewContainer )
  {
    mWebViewContainer->SetSize( width, height );
  }
}

bool TizenWebEngineChromium::SendTouchEvent( const Dali::TouchData& touch )
{
  if ( mWebViewContainer )
  {
    return mWebViewContainer->SendTouchEvent( touch );
  }
  return false;
}

bool TizenWebEngineChromium::SendKeyEvent( const Dali::KeyEvent& event )
{
  if ( mWebViewContainer )
  {
    return mWebViewContainer->SendKeyEvent( event );
  }
  return false;
}

Dali::WebEnginePlugin::WebEngineSignalType& TizenWebEngineChromium::PageLoadStartedSignal()
{
  return mLoadStartedSignal;
}

Dali::WebEnginePlugin::WebEngineSignalType& TizenWebEngineChromium::PageLoadFinishedSignal()
{
  return mLoadFinishedSignal;
}

// WebViewContainerClient Interface
void TizenWebEngineChromium::UpdateImage( tbm_surface_h buffer )
{
  if ( !buffer )
  {
    return;
  }

  Any source( buffer );
  mDaliImageSrc->SetSource( source );
  Dali::Stage::GetCurrent().KeepRendering( 0.0f );
}

void TizenWebEngineChromium::LoadStarted()
{
  DALI_LOG_RELEASE_INFO( "#LoadStarted : %s\n", GetUrl().c_str() );
  mLoadStartedSignal.Emit( GetUrl() );
}

void TizenWebEngineChromium::LoadFinished()
{
  DALI_LOG_RELEASE_INFO( "#LoadFinished : %s\n", GetUrl().c_str() );
  mLoadFinishedSignal.Emit( GetUrl() );
}

void TizenWebEngineChromium::RunJavaScriptInterfaceCallback( const std::string& objectName, const std::string& message )
{
  for( auto& callback : mJsCallbacks )
  {
    if( callback.mObjectName == objectName )
    {
      callback.mCallback( message );
      return;
    }
  }
}
} // namespace Plugin
} // namespace Dali


extern "C" DALI_EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin()
{
  return new Dali::Plugin::TizenWebEngineChromium();
}

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin( Dali::WebEnginePlugin* plugin )
{
  if ( plugin )
  {
    delete plugin;
  }
}

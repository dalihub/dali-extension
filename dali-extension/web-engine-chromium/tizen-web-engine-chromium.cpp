/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>

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
      mHeight( height ),
      mCookieAcceptancePolicy( EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY )
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
    evas_object_smart_callback_add( mWebView, "load,error",
                                    &WebViewContainerForDali::OnLoadError,
                                    &mClient );
    evas_object_smart_callback_add( mWebView, "console,message",
                                    &WebViewContainerForDali::OnConsoleMessage,
                                    this );
    evas_object_smart_callback_add( mWebView, "edge,left",
                                    &WebViewContainerForDali::OnEdgeLeft,
                                    &mClient );
    evas_object_smart_callback_add( mWebView, "edge,right",
                                    &WebViewContainerForDali::OnEdgeRight,
                                    &mClient );
    evas_object_smart_callback_add( mWebView, "edge,top",
                                    &WebViewContainerForDali::OnEdgeTop,
                                    &mClient );
    evas_object_smart_callback_add( mWebView, "edge,bottom",
                                    &WebViewContainerForDali::OnEdgeBottom,
                                    &mClient );

    evas_object_resize( mWebView, mWidth, mHeight );
    evas_object_show( mWebView );
  }

  void LoadUrl( const std::string& url )
  {
    ewk_view_url_set( mWebView, url.c_str() );
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

  void Suspend()
  {
    ewk_view_suspend( mWebView );
  }

  void Resume()
  {
    ewk_view_resume( mWebView );
  }

  void ScrollBy( int deltaX, int deltaY )
  {
    ewk_view_scroll_by( mWebView, deltaX, deltaY );
  }

  void SetScrollPosition( int x, int y )
  {
    ewk_view_scroll_set( mWebView, x, y );
  }

  void GetScrollPosition( int& x, int& y ) const
  {
    ewk_view_scroll_pos_get( mWebView, &x, &y );
  }

  void GetScrollSize( int& width, int& height ) const
  {
    ewk_view_scroll_size_get( mWebView, &width, &height );
  }

  void GetContentSize( int& width, int& height ) const
  {
    ewk_view_contents_size_get( mWebView, &width, &height );
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

  void EvaluateJavaScript( size_t key, const std::string& script )
  {
    ewk_view_script_execute( mWebView, script.c_str(), OnEvaluateJavaScript, (void*)key );
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

  void ClearCookies()
  {
    ewk_cookie_manager_cookies_clear( ewk_context_cookie_manager_get( ewk_view_context_get( mWebView ) ) );
  }

  Ewk_Cache_Model GetCacheModel()
  {
    return ewk_context_cache_model_get( ewk_view_context_get( mWebView ) );
  }

  void SetCacheModel( Ewk_Cache_Model cacheModel )
  {
    ewk_context_cache_model_set( ewk_view_context_get( mWebView ), cacheModel );
  }

  Ewk_Cookie_Accept_Policy GetCookieAcceptPolicy()
  {
    return mCookieAcceptancePolicy;
  }

  void SetCookieAcceptPolicy( Ewk_Cookie_Accept_Policy policy )
  {
    ewk_cookie_manager_accept_policy_set( ewk_context_cookie_manager_get( ewk_view_context_get( mWebView ) ), policy );
    mCookieAcceptancePolicy = policy;
  }

  const std::string& GetUserAgent()
  {
    mUserAgent = std::string( ewk_view_user_agent_get( mWebView ) );
    return mUserAgent;
  }

  void SetUserAgent( const std::string& userAgent )
  {
    ewk_view_user_agent_set( mWebView, userAgent.c_str() );
  }

  bool IsJavaScriptEnabled()
  {
    return ewk_settings_javascript_enabled_get( ewk_view_settings_get( mWebView ) );
  }

  void EnableJavaScript( bool enabled )
  {
    ewk_settings_javascript_enabled_set( ewk_view_settings_get( mWebView ), enabled );
  }

  bool AreImagesAutomaticallyLoaded()
  {
    return ewk_settings_loads_images_automatically_get( ewk_view_settings_get( mWebView ) );
  }

  void LoadImagesAutomatically( bool automatic )
  {
    ewk_settings_loads_images_automatically_set( ewk_view_settings_get( mWebView ), automatic );
  }

  const std::string& GetDefaultTextEncodingName()
  {
    mDefaultTextEncodingName = std::string( ewk_settings_default_text_encoding_name_get( ewk_view_settings_get( mWebView ) ) );
    return mDefaultTextEncodingName;
  }

  void SetDefaultTextEncodingName( const std::string& defaultTextEncodingName )
  {
    ewk_settings_default_text_encoding_name_set( ewk_view_settings_get( mWebView ), defaultTextEncodingName.c_str() );
  }

  int GetDefaultFontSize()
  {
    return ewk_settings_default_font_size_get( ewk_view_settings_get( mWebView ) );
  }

  void SetDefaultFontSize( int defaultFontSize )
  {
    ewk_settings_default_font_size_set( ewk_view_settings_get( mWebView ), defaultFontSize );
  }

  void SetSize( int width, int height )
  {
    mWidth = width;
    mHeight = height;
    evas_object_resize( mWebView, mWidth, mHeight );
  }

  bool SendTouchEvent( const TouchEvent& touch )
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

  bool SendKeyEvent( const Dali::KeyEvent& keyEvent )
  {
    void* evasKeyEvent = 0;
    if( keyEvent.GetState() == Dali::KeyEvent::DOWN )
    {
      Evas_Event_Key_Down downEvent;
      memset( &downEvent, 0, sizeof(Evas_Event_Key_Down) );
      downEvent.key = keyEvent.GetKeyName().c_str();
      downEvent.string = keyEvent.GetKeyString().c_str();
      evasKeyEvent = static_cast<void*>(&downEvent);
      ewk_view_send_key_event( mWebView, evasKeyEvent, true );
    }
    else
    {
      Evas_Event_Key_Up upEvent;
      memset(&upEvent, 0, sizeof(Evas_Event_Key_Up));
      upEvent.key = keyEvent.GetKeyName().c_str();
      upEvent.string = keyEvent.GetKeyString().c_str();
      evasKeyEvent = static_cast<void*>(&upEvent);
      ewk_view_send_key_event( mWebView, evasKeyEvent, false );
     }
     return false;
  }

  void SetFocus( bool focused )
  {
    ecore_evas_focus_set( WebEngineManager::Get().GetWindow(), focused );
    ewk_view_focus_set( mWebView, focused );
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

  static void OnLoadError( void* data, Evas_Object*, void* rawError )
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Error* error = static_cast< Ewk_Error* >( rawError );

    client->LoadError( ewk_error_url_get( error ), ewk_error_code_get( error ) );
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

  static void OnEdgeLeft( void* data, Evas_Object*, void* )
  {
    auto client = static_cast<WebViewContainerClient*>( data );
    client->ScrollEdgeReached( Dali::WebEnginePlugin::ScrollEdge::LEFT );
  }

  static void OnEdgeRight( void* data, Evas_Object*, void* )
  {
    auto client = static_cast<WebViewContainerClient*>( data );
    client->ScrollEdgeReached( Dali::WebEnginePlugin::ScrollEdge::RIGHT );
  }

  static void OnEdgeTop( void* data, Evas_Object*, void* )
  {
    auto client = static_cast<WebViewContainerClient*>( data );
    client->ScrollEdgeReached( Dali::WebEnginePlugin::ScrollEdge::TOP );
  }

  static void OnEdgeBottom( void* data, Evas_Object*, void* )
  {
    auto client = static_cast<WebViewContainerClient*>( data );
    client->ScrollEdgeReached( Dali::WebEnginePlugin::ScrollEdge::BOTTOM );
  }

  static void OnEvaluateJavaScript( Evas_Object* o, const char* result, void* data )
  {
    auto client = WebEngineManager::Get().FindContainerClient( o );
    if( client )
    {
      client->RunJavaScriptEvaluationResultHandler( (size_t)data, result );
    }
  }

  static void OnJavaScriptMessage( Evas_Object* o, Ewk_Script_Message message )
  {
    auto client = WebEngineManager::Get().FindContainerClient( o );
    if( client )
    {
      client->RunJavaScriptMessageHandler( message.name, static_cast<char*>( message.body ) );
    }
  }

private:
  Evas_Object* mWebView;
  WebViewContainerClient& mClient;

  int mWidth;
  int mHeight;

  Ewk_Cookie_Accept_Policy mCookieAcceptancePolicy;
  std::string mUserAgent;
  std::string mDefaultTextEncodingName;
};

class TBMSurfaceSourceInitializer
{
public:
  explicit TBMSurfaceSourceInitializer( NativeImageSourcePtr& imageSrc,
                                        int width, int height )
  {
    mSurface = tbm_surface_create( width, height, TBM_FORMAT_ARGB8888 );
    if( !mSurface )
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
    if( mSurface )
    {
      if( tbm_surface_destroy( mSurface ) != TBM_SURFACE_ERROR_NONE )
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
  , mJavaScriptEvaluationCount( 0 )
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
    mJavaScriptEvaluationResultHandlers.clear();
    mJavaScriptMessageHandlers.clear();

    delete mWebViewContainer;
    mWebViewContainer = 0;
  }
}

void TizenWebEngineChromium::LoadUrl( const std::string& path )
{
  if( mWebViewContainer )
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
  if( mWebViewContainer )
  {
    mUrl =  mWebViewContainer->GetUrl();
  }
  return mUrl;
}

void TizenWebEngineChromium::LoadHTMLString( const std::string& string )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->LoadHtml( string );
  }
}

void TizenWebEngineChromium::Reload()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->Reload();
  }
}

void TizenWebEngineChromium::StopLoading()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->StopLoading();
  }
}

void TizenWebEngineChromium::Suspend()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->Suspend();
  }
}

void TizenWebEngineChromium::Resume()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->Resume();
  }
}

void TizenWebEngineChromium::ScrollBy( int deltaX, int deltaY )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->ScrollBy( deltaX, deltaY );
  }
}

void TizenWebEngineChromium::SetScrollPosition( int x, int y )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->SetScrollPosition( x, y );
  }
}

void TizenWebEngineChromium::GetScrollPosition(int& x, int& y) const
{
  if( mWebViewContainer )
  {
    mWebViewContainer->GetScrollPosition( x, y );
  }
}

void TizenWebEngineChromium::GetScrollSize( int& width, int& height ) const
{
  if( mWebViewContainer )
  {
    mWebViewContainer->GetScrollSize( width, height );
  }
}

void TizenWebEngineChromium::GetContentSize( int& width, int& height ) const
{
  if( mWebViewContainer )
  {
    mWebViewContainer->GetContentSize( width, height );
  }
}

bool TizenWebEngineChromium::CanGoForward()
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->CanGoForward();
  }
  return false;
}

void TizenWebEngineChromium::GoForward()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->GoForward();
  }
}

bool TizenWebEngineChromium::CanGoBack()
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->CanGoBack();
  }
  return false;
}

void TizenWebEngineChromium::GoBack()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->GoBack();
  }
}

void TizenWebEngineChromium::EvaluateJavaScript( const std::string& script, std::function< void( const std::string& ) > resultHandler )
{
  if( mWebViewContainer )
  {
    bool badAlloc = false;

    try
    {
      mJavaScriptEvaluationResultHandlers.emplace( mJavaScriptEvaluationCount, resultHandler );
    }
    catch( std::bad_alloc &e )
    {
      badAlloc = true;
      DALI_LOG_ERROR( "Too many ongoing JavaScript evaluations." );
    }

    if( !badAlloc )
    {
      mWebViewContainer->EvaluateJavaScript( mJavaScriptEvaluationCount++, script );
    }
  }
}

void TizenWebEngineChromium::AddJavaScriptMessageHandler( const std::string& exposedObjectName, std::function< void( const std::string& ) > handler )
{
  if( mWebViewContainer )
  {
    if( mJavaScriptMessageHandlers.emplace( exposedObjectName, handler ).second )
    {
      mWebViewContainer->AddJavaScriptMessageHandler( exposedObjectName );
    }
    else
    {
      DALI_LOG_ERROR( "Callback for (%s) already exists.", exposedObjectName.c_str() );
    }
  }
}

void TizenWebEngineChromium::ClearHistory()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->ClearHistory();
  }
}

void TizenWebEngineChromium::ClearCache()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->ClearCache();
  }
}

void TizenWebEngineChromium::ClearCookies()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->ClearCookies();
  }
}

Dali::WebEnginePlugin::CacheModel TizenWebEngineChromium::GetCacheModel() const
{
  if( mWebViewContainer )
  {
    return static_cast< Dali::WebEnginePlugin::CacheModel >( mWebViewContainer->GetCacheModel() );
  }
  return Dali::WebEnginePlugin::CacheModel::DOCUMENT_VIEWER;
}

void TizenWebEngineChromium::SetCacheModel( Dali::WebEnginePlugin::CacheModel cacheModel )
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->SetCacheModel( static_cast< Ewk_Cache_Model >( cacheModel ) );
  }
}

Dali::WebEnginePlugin::CookieAcceptPolicy TizenWebEngineChromium::GetCookieAcceptPolicy() const
{
  if( mWebViewContainer )
  {
    return static_cast< Dali::WebEnginePlugin::CookieAcceptPolicy >( mWebViewContainer->GetCookieAcceptPolicy() );
  }
  return Dali::WebEnginePlugin::CookieAcceptPolicy::NO_THIRD_PARTY;
}

void TizenWebEngineChromium::SetCookieAcceptPolicy( Dali::WebEnginePlugin::CookieAcceptPolicy policy )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->SetCookieAcceptPolicy( static_cast< Ewk_Cookie_Accept_Policy >( policy ) );
  }
}

const std::string kEmpty;

const std::string& TizenWebEngineChromium::GetUserAgent() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->GetUserAgent();
  }
  return kEmpty;
}

void TizenWebEngineChromium::SetUserAgent( const std::string& userAgent )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->SetUserAgent( userAgent );
  }
}

bool TizenWebEngineChromium::IsJavaScriptEnabled() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->IsJavaScriptEnabled();
  }
  return false;
}

void TizenWebEngineChromium::EnableJavaScript( bool enabled )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->EnableJavaScript( enabled );
  }
}

bool TizenWebEngineChromium::AreImagesAutomaticallyLoaded() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->AreImagesAutomaticallyLoaded();
  }
  return false;
}

void TizenWebEngineChromium::LoadImagesAutomatically( bool automatic )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->LoadImagesAutomatically( automatic );
  }
}

const std::string& TizenWebEngineChromium::GetDefaultTextEncodingName() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->GetDefaultTextEncodingName();
  }
  return kEmpty;
}

void TizenWebEngineChromium::SetDefaultTextEncodingName( const std::string& defaultTextEncodingName )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->SetDefaultTextEncodingName( defaultTextEncodingName );
  }
}

int TizenWebEngineChromium::GetDefaultFontSize() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->AreImagesAutomaticallyLoaded();
  }
  return 0;
}

void TizenWebEngineChromium::SetDefaultFontSize( int defaultFontSize )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->SetDefaultFontSize( defaultFontSize );
  }
}

void TizenWebEngineChromium::SetSize( int width, int height )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->SetSize( width, height );
  }
}

bool TizenWebEngineChromium::SendTouchEvent( const Dali::TouchEvent& touch )
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->SendTouchEvent( touch );
  }
  return false;
}

bool TizenWebEngineChromium::SendKeyEvent( const Dali::KeyEvent& event )
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->SendKeyEvent( event );
  }
  return false;
}

void TizenWebEngineChromium::SetFocus( bool focused )
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->SetFocus( focused );
  }
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& TizenWebEngineChromium::PageLoadStartedSignal()
{
  return mLoadStartedSignal;
}

Dali::WebEnginePlugin::WebEnginePageLoadSignalType& TizenWebEngineChromium::PageLoadFinishedSignal()
{
  return mLoadFinishedSignal;
}

Dali::WebEnginePlugin::WebEnginePageLoadErrorSignalType& TizenWebEngineChromium::PageLoadErrorSignal()
{
  return mLoadErrorSignal;
}

Dali::WebEnginePlugin::WebEngineScrollEdgeReachedSignalType& TizenWebEngineChromium::ScrollEdgeReachedSignal()
{
  return mScrollEdgeReachedSignal;
}

// WebViewContainerClient Interface
void TizenWebEngineChromium::UpdateImage( tbm_surface_h buffer )
{
  if( !buffer )
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

void TizenWebEngineChromium::LoadError( const char* url, int errorCode )
{
  std::string stdUrl( url );
  mLoadErrorSignal.Emit( stdUrl, errorCode );
}

void TizenWebEngineChromium::ScrollEdgeReached( Dali::WebEnginePlugin::ScrollEdge edge )
{
  DALI_LOG_RELEASE_INFO( "#ScrollEdgeReached : %d\n", edge );
  mScrollEdgeReachedSignal.Emit( edge );
}

void TizenWebEngineChromium::RunJavaScriptEvaluationResultHandler( size_t key, const char* result )
{
  auto handler = mJavaScriptEvaluationResultHandlers.find( key );
  if( handler == mJavaScriptEvaluationResultHandlers.end() )
  {
    return;
  }

  if( handler->second )
  {
    std::string stored( result );
    handler->second( stored );
  }

  mJavaScriptEvaluationResultHandlers.erase( handler );
}

void TizenWebEngineChromium::RunJavaScriptMessageHandler( const std::string& objectName, const std::string& message )
{
  auto handler = mJavaScriptMessageHandlers.find( objectName );
  if( handler == mJavaScriptMessageHandlers.end() )
  {
    return;
  }

  handler->second( message );
}
} // namespace Plugin
} // namespace Dali


extern "C" DALI_EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin()
{
  return new Dali::Plugin::TizenWebEngineChromium();
}

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin( Dali::WebEnginePlugin* plugin )
{
  if( plugin )
  {
    delete plugin;
  }
}

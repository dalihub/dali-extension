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

#include "tizen-web-engine-chromium.h"

#include "tizen-web-engine-back-forward-list.h"
#include "tizen-web-engine-context.h"
#include "tizen-web-engine-cookie-manager.h"
#include "tizen-web-engine-policy-decision.h"
#include "tizen-web-engine-settings.h"

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_Wl2.h>
#include <Evas.h>
#include <Elementary.h>

#include <EWebKit_internal.h>
#include <EWebKit_product.h>

#include <dali/devel-api/adaptor-framework/lifecycle-controller.h>
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/integration-api/adaptor-framework/adaptor.h>
#include <dali/public-api/images/pixel-data.h>
#include <dali/public-api/signals/slot-delegate.h>

using namespace Dali;

namespace Dali
{
namespace Plugin
{

namespace
{

// const
const std::string EMPTY_STRING;

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

} // anonymous namespace.

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

//
// A class for managing multiple WebViews
//
class WebEngineManager {
public:
  static WebEngineManager& Get()
  {
    static WebEngineManager instance;
    return instance;
  }

  static bool IsAvailable()
  {
    return Get().mWebEngineManagerAvailable;
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
      else
      {
        it++;
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
    return nullptr;
  }

private:
  WebEngineManager()
  : mSlotDelegate(this),
    mWebEngineManagerAvailable(true)
  {
    elm_init( 0, 0 );
    ewk_init();
    mWindow = ecore_evas_new( "wayland_egl", 0, 0, 1, 1, 0 );
    LifecycleController::Get().TerminateSignal().Connect(mSlotDelegate, &WebEngineManager::OnTerminated);
  }

  ~WebEngineManager()
  {
    if(mWebEngineManagerAvailable)
    {
      // Call OnTerminated directly.
      OnTerminated();
    }
  }

  void OnTerminated()
  {
    // Ignore duplicated termination
    if(DALI_UNLIKELY(!mWebEngineManagerAvailable))
    {
      return;
    }

    // App is terminated. Now web engine is not available anymore.
    mWebEngineManagerAvailable = false;

    for (auto it = mContainerClients.begin(); it != mContainerClients.end(); it++)
    {
      evas_object_del(it->mWebView);
    }
    mContainerClients.clear();
    ecore_evas_free(mWindow);
    ewk_shutdown();
    elm_shutdown();
    DALI_LOG_RELEASE_INFO("#WebEngineManager is destroyed fully.\n");
  }

  SlotDelegate<WebEngineManager>          mSlotDelegate;
  Ecore_Evas*                             mWindow;
  std::vector<WebViewContainerClientPair> mContainerClients;
  bool                                    mWebEngineManagerAvailable;
};

class WebViewContainerForDali
{
public:
  WebViewContainerForDali( WebViewContainerClient& client, int width, int height )
    : mClient( client ),
      mWidth( width ),
      mHeight( height ),
      mCookieAcceptancePolicy( EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY ),
      mWebEngineSettings( 0 ),
      mWebEngineContext( 0 ),
      mWebEngineCookieManager( 0 ),
      mWebEngineBackForwardList( 0 )
  {
    InitWebView( 0, 0 );
    WebEngineManager::Get().AddContainerClient( &mClient, mWebView );
  }

  WebViewContainerForDali( WebViewContainerClient& client, int width, int height, int argc, char** argv )
    : mClient( client ),
      mWidth( width ),
      mHeight( height ),
      mCookieAcceptancePolicy( EWK_COOKIE_ACCEPT_POLICY_NO_THIRD_PARTY ),
      mWebEngineSettings( 0 ),
      mWebEngineContext( 0 ),
      mWebEngineCookieManager( 0 ),
      mWebEngineBackForwardList( 0 )
  {
    InitWebView( argc, argv );
    WebEngineManager::Get().AddContainerClient( &mClient, mWebView );
  }

  ~WebViewContainerForDali()
  {
    if(WebEngineManager::IsAvailable())
    {
      WebEngineManager::Get().RemoveContainerClient(mWebView);
    }
    evas_object_del(mWebView);
    mWebView = nullptr;
  }

  void InitWebView( int argc, char** argv )
  {
    // Check if web engine is available and make sure that web engine is initialized.
    if(!WebEngineManager::IsAvailable())
    {
      DALI_LOG_ERROR("Web engine has been terminated in current process.");
      return;
    }

    if ( argc > 0 )
    {
      ewk_set_arguments( argc, argv );
    }

    Ecore_Wl2_Window* win = AnyCast< Ecore_Wl2_Window* >( Adaptor::Get().GetNativeWindowHandle() );
    Ewk_Context* context = ewk_context_default_get();
    ewk_context_max_refresh_rate_set( context, 60 );
    mWebView = ewk_view_add( ecore_evas_get( WebEngineManager::Get().GetWindow() ) );
    ewk_view_offscreen_rendering_enabled_set( mWebView, true );
    ecore_wl2_window_alpha_set( win, false );
    ewk_view_ime_window_set( mWebView, win );
    ewk_view_set_support_video_hole( mWebView, win, EINA_TRUE, EINA_FALSE );

    Ewk_Settings* settings = ewk_view_settings_get( mWebView );
    mWebEngineSettings = TizenWebEngineSettings( settings );

    context = ewk_view_context_get( mWebView );
    mWebEngineContext = TizenWebEngineContext( context );

    Ewk_Cookie_Manager* manager = ewk_context_cookie_manager_get( context );
    mWebEngineCookieManager = TizenWebEngineCookieManager( manager );

    Ewk_Back_Forward_List* backForwardList = ewk_view_back_forward_list_get( mWebView );
    mWebEngineBackForwardList = TizenWebEngineBackForwardList( backForwardList );

    ewk_settings_viewport_meta_tag_set(settings, false);

    evas_object_smart_callback_add(mWebView, "offscreen,frame,rendered",
                                   &WebViewContainerForDali::OnFrameRendered,
                                   &mClient);
    evas_object_smart_callback_add(mWebView, "load,started",
                                   &WebViewContainerForDali::OnLoadStarted,
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
                                   &WebViewContainerForDali::OnConsoleMessage,
                                   this);
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
    evas_object_smart_callback_add(mWebView, "policy,navigation,decide",
                                   &WebViewContainerForDali::OnNavigationPolicyDecided,
                                   &mClient);

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

  std::string GetTitle()
  {
    return std::string( ewk_view_title_get( mWebView ) );
  }

  Dali::PixelData GetFavicon()
  {
    Evas_Object* iconObject = ewk_view_favicon_get( mWebView );
    if ( !iconObject )
    {
      return Dali::PixelData();
    }

    // color-space is argb8888.
    uint8_t* pixelBuffer = ( uint8_t* ) evas_object_image_data_get( iconObject, false );
    if (!pixelBuffer)
    {
      return Dali::PixelData();
    }

    int width = 0, height = 0;
    evas_object_image_size_get( iconObject, &width, &height );

    uint32_t bufferSize = width * height * 4;
    uint8_t* convertedBuffer = new uint8_t[ bufferSize ];

    // convert the color-space to rgba8888.
    for( uint32_t i = 0; i < bufferSize; i += 4 )
    {
      convertedBuffer[ i ] = pixelBuffer[ i + 1 ];
      convertedBuffer[ i + 1 ] = pixelBuffer[ i + 2 ];
      convertedBuffer[ i + 2 ] = pixelBuffer[ i + 3 ];
      convertedBuffer[ i + 3 ] = pixelBuffer[ i ];
    }

    return Dali::PixelData::New( convertedBuffer, bufferSize, width, height,
                                 Dali::Pixel::Format::RGBA8888,
                                 Dali::PixelData::ReleaseFunction::DELETE_ARRAY );
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

  void ClearAllTilesResources()
  {
    ewk_view_clear_all_tiles_resources( mWebView );
  }

  void ClearHistory()
  {
    ewk_view_back_forward_list_clear( mWebView );
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
    Ewk_Touch_Point point;
    point.id = 0;
    point.x = touch.GetScreenPosition( 0 ).x;
    point.y = touch.GetScreenPosition( 0 ).y;
    point.state = state;
    pointList = eina_list_append( pointList, &point );

    bool fed = ewk_view_feed_touch_event( mWebView, type, pointList, 0 );
    eina_list_free( pointList );
    return fed;
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

  void UpdateDisplayArea( Dali::Rect< int > displayArea )
  {
    evas_object_move( mWebView, displayArea.x, displayArea.y );
    SetSize( displayArea.width, displayArea.height );
    evas_object_geometry_set ( mWebView, displayArea.x, displayArea.y, displayArea.width, displayArea.height );
  }

  void EnableVideoHole( bool enabled )
  {
    Ecore_Wl2_Window* win = AnyCast< Ecore_Wl2_Window* >( Adaptor::Get().GetNativeWindowHandle() );
    ewk_view_set_support_video_hole( mWebView, win, enabled, EINA_FALSE );
    ecore_wl2_window_alpha_set( win, !enabled );
  }

  void GetPlainTextAsynchronously()
  {
    ewk_view_plain_text_get(mWebView, &WebViewContainerForDali::OnPlainTextReceived, &mClient);
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

  static void OnUrlChanged(void* data, Evas_Object*, void* newUrl)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    std::string url = static_cast<char*>(newUrl);
    client->UrlChanged(url);
  }

  static void OnNavigationPolicyDecided(void* data, Evas_Object*, void* policy)
  {
    auto client = static_cast<WebViewContainerClient*>(data);
    Ewk_Policy_Decision* policyDecision = static_cast<Ewk_Policy_Decision*>(policy);
    std::unique_ptr<Dali::WebEnginePolicyDecision> webPolicyDecision(new TizenWebEnginePolicyDecision(policyDecision));
    client->NavigationPolicyDecided(std::move(webPolicyDecision));
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

private:
  Evas_Object* mWebView;
  WebViewContainerClient& mClient;

  int mWidth;
  int mHeight;

  Ewk_Cookie_Accept_Policy mCookieAcceptancePolicy;
  std::string mUserAgent;
  std::string mDefaultTextEncodingName;

  TizenWebEngineSettings mWebEngineSettings;
  TizenWebEngineContext mWebEngineContext;
  TizenWebEngineCookieManager mWebEngineCookieManager;
  TizenWebEngineBackForwardList mWebEngineBackForwardList;
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
  mWebViewContainer = new WebViewContainerForDali( *this, width, height );
  TBMSurfaceSourceInitializer initializer( mDaliImageSrc, width, height );
}

void TizenWebEngineChromium::Create( int width, int height,
                                     int argc, char** argv )
{
  mWebViewContainer = new WebViewContainerForDali( *this, width, height, argc, argv );
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

std::string TizenWebEngineChromium::GetTitle() const
{
  return mWebViewContainer ? mWebViewContainer->GetTitle() : EMPTY_STRING;
}

Dali::PixelData TizenWebEngineChromium::GetFavicon() const
{
  return mWebViewContainer ? mWebViewContainer->GetFavicon() : Dali::PixelData();
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

void TizenWebEngineChromium::LoadHtmlString( const std::string& string )
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

void TizenWebEngineChromium::EvaluateJavaScript( const std::string& script, JavaScriptMessageHandlerCallback resultHandler )
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

void TizenWebEngineChromium::AddJavaScriptMessageHandler( const std::string& exposedObjectName, JavaScriptMessageHandlerCallback handler )
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

void TizenWebEngineChromium::ClearAllTilesResources()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->ClearAllTilesResources();
  }
}

void TizenWebEngineChromium::ClearHistory()
{
  if( mWebViewContainer )
  {
    mWebViewContainer->ClearHistory();
  }
}

const std::string& TizenWebEngineChromium::GetUserAgent() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->GetUserAgent();
  }
  return EMPTY_STRING;
}

void TizenWebEngineChromium::SetUserAgent( const std::string& userAgent )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->SetUserAgent( userAgent );
  }
}

Dali::WebEngineSettings& TizenWebEngineChromium::GetSettings() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->GetSettings();
  }

  DALI_LOG_ERROR( "Web engine is not created successfully!" );
  static TizenWebEngineSettings dummy( nullptr );

  return dummy;
}

Dali::WebEngineContext& TizenWebEngineChromium::GetContext() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->GetContext();
  }

  DALI_LOG_ERROR( "Web engine is not created successfully!" );
  static TizenWebEngineContext dummy( nullptr );

  return dummy;
}

Dali::WebEngineCookieManager& TizenWebEngineChromium::GetCookieManager() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->GetCookieManager();
  }

  DALI_LOG_ERROR( "Web engine is not created successfully!" );
  static TizenWebEngineCookieManager dummy( nullptr );

  return dummy;
}

Dali::WebEngineBackForwardList& TizenWebEngineChromium::GetBackForwardList() const
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->GetBackForwardList();
  }

  DALI_LOG_ERROR( "Web engine is not created successfully!" );
  static TizenWebEngineBackForwardList dummy( nullptr );

  return dummy;
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

void TizenWebEngineChromium::UpdateDisplayArea( Dali::Rect< int > displayArea )
{
  if( mWebViewContainer )
  {
    mWebViewContainer->UpdateDisplayArea( displayArea );
  }
}

void TizenWebEngineChromium::EnableVideoHole( bool enabled )
{
  if( mWebViewContainer )
  {
    return mWebViewContainer->EnableVideoHole( enabled );
  }
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

void TizenWebEngineChromium::GetPlainTextAsynchronously(PlainTextReceivedCallback callback)
{
  if (mWebViewContainer)
  {
    mPlainTextReceivedCallback = callback;
    mWebViewContainer->GetPlainTextAsynchronously();
  }
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
  ExecuteCallback(mLoadStartedCallback, GetUrl());
}

void TizenWebEngineChromium::LoadFinished()
{
  DALI_LOG_RELEASE_INFO( "#LoadFinished : %s\n", GetUrl().c_str() );
  ExecuteCallback(mLoadFinishedCallback, GetUrl());
}

void TizenWebEngineChromium::LoadError( const char* url, int errorCode )
{
  std::string stdUrl( url );
  ExecuteCallback(mLoadErrorCallback, stdUrl, errorCode);
}

void TizenWebEngineChromium::ScrollEdgeReached( Dali::WebEnginePlugin::ScrollEdge edge )
{
  DALI_LOG_RELEASE_INFO( "#ScrollEdgeReached : %d\n", edge );
  ExecuteCallback(mScrollEdgeReachedCallback, edge);
}

void TizenWebEngineChromium::UrlChanged(const std::string& url)
{
  DALI_LOG_RELEASE_INFO("#UrlChanged : %s\n", url.c_str());
  ExecuteCallback(mUrlChangedCallback, url);
}

void TizenWebEngineChromium::NavigationPolicyDecided(std::unique_ptr<Dali::WebEnginePolicyDecision> decision)
{
  DALI_LOG_RELEASE_INFO("#NavigationPolicyDecided.\n");
  ExecuteCallback(mNavigationPolicyDecidedCallback, std::move(decision));
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

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin( Dali::WebEnginePlugin* plugin )
{
  if( plugin )
  {
    delete plugin;
  }
}

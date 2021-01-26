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

// CLASS HEADER
#include "tizen-web-engine-lwe.h"

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <dali/devel-api/common/stage.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/touch-event.h>
#include <dali/devel-api/adaptor-framework/application-devel.h>
#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list.h>
#include <dali/devel-api/adaptor-framework/web-engine-back-forward-list-item.h>
#include <dali/devel-api/adaptor-framework/web-engine-context.h>
#include <dali/devel-api/adaptor-framework/web-engine-cookie-manager.h>
#include <dali/devel-api/adaptor-framework/web-engine-settings.h>
#include <dali/public-api/images/pixel-data.h>

#include <unistd.h>
#include <pthread.h>

#define DB_NAME_LOCAL_STORAGE "LWE_localStorage.db"
#define DB_NAME_COOKIES "LWE_Cookies.db"
#define DB_NAME_CACHE "LWE_Cache.db"

// The plugin factories
extern "C" DALI_EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin( void )
{
  return new Dali::Plugin::TizenWebEngineLWE;
}

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin( Dali::WebEnginePlugin* plugin )
{
  if( plugin != NULL )
  {
    delete plugin;
  }
}

namespace Dali
{

namespace Plugin
{

namespace
{

const std::string EMPTY_STRING;

LWE::KeyValue KeyStringToKeyValue( const char* DALIKeyString, bool isShiftPressed )
{
  LWE::KeyValue keyValue = LWE::KeyValue::UnidentifiedKey;
  if( strcmp( "Left", DALIKeyString ) == 0 )
  {
    keyValue = LWE::KeyValue::ArrowLeftKey;
  }
  else if( strcmp( "Right", DALIKeyString ) == 0 )
  {
    keyValue = LWE::KeyValue::ArrowRightKey;
  }
  else if( strcmp( "Up", DALIKeyString ) == 0 )
  {
    keyValue = LWE::KeyValue::ArrowUpKey;
  }
  else if( strcmp( "Down", DALIKeyString ) == 0 )
  {
    keyValue = LWE::KeyValue::ArrowDownKey;
  }
  else if( strcmp( "space", DALIKeyString ) == 0 )
  {
    keyValue = LWE::KeyValue::SpaceKey;
  }
  else if( strcmp( "Return", DALIKeyString ) == 0 )
  {
    keyValue = LWE::KeyValue::EnterKey;
  }
  else if( strcmp( "BackSpace", DALIKeyString ) == 0 )
  {
    keyValue = LWE::KeyValue::BackspaceKey;
  }
  else if( strcmp( "Escape", DALIKeyString ) == 0 )
  {
    keyValue = LWE::KeyValue::EscapeKey;
  }
  else if( strcmp( "minus", DALIKeyString ) == 0 )
  {
    if( isShiftPressed )
    {
      keyValue = LWE::KeyValue::MinusMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::UnderScoreMarkKey;
    }
  }
  else if( strcmp( "equal", DALIKeyString ) == 0 )
  {
    if( isShiftPressed )
    {
      keyValue = LWE::KeyValue::PlusMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::EqualitySignKey;
    }
  }
  else if( strcmp( "bracketleft", DALIKeyString ) == 0 )
  {
    if( isShiftPressed )
    {
      keyValue = LWE::KeyValue::LeftCurlyBracketMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::LeftSquareBracketKey;
    }
  }
  else if( strcmp( "bracketright", DALIKeyString ) == 0 )
  {
    if( isShiftPressed )
    {
      keyValue = LWE::KeyValue::RightCurlyBracketMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::RightSquareBracketKey;
    }
  }
  else if( strcmp( "semicolon", DALIKeyString ) == 0 )
  {
    if( isShiftPressed )
    {
      keyValue = LWE::KeyValue::ColonMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::SemiColonMarkKey;
    }
  }
  else if( strcmp( "apostrophe", DALIKeyString ) == 0 )
  {
    if( isShiftPressed )
    {
      keyValue = LWE::KeyValue::DoubleQuoteMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::SingleQuoteMarkKey;
    }
  }
  else if( strcmp( "comma", DALIKeyString ) == 0 )
  {
    if( isShiftPressed )
    {
      keyValue = LWE::KeyValue::LessThanMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::CommaMarkKey;
    }
  }
  else if( strcmp( "period", DALIKeyString ) == 0 )
  {
    if( isShiftPressed )
    {
      keyValue = LWE::KeyValue::GreaterThanSignKey;
    }
    else
    {
      keyValue = LWE::KeyValue::PeriodKey;
    }
  }
  else if( strcmp( "slash", DALIKeyString ) == 0 )
  {
    if( isShiftPressed )
    {
      keyValue = LWE::KeyValue::QuestionMarkKey;
    }
    else
    {
      keyValue = LWE::KeyValue::SlashKey;
    }
  }
  else if( strlen( DALIKeyString ) == 1 )
  {
    char ch = DALIKeyString[0];
    if( ch >= '0' && ch <= '9' )
    {
      if( isShiftPressed )
      {
        switch( ch )
        {
          case '1':
          {
            keyValue = LWE::KeyValue::ExclamationMarkKey;
            break;
          }
          case '2':
          {
            keyValue = LWE::KeyValue::AtMarkKey;
            break;
          }
          case '3':
          {
            keyValue = LWE::KeyValue::SharpMarkKey;
            break;
          }
          case '4':
          {
            keyValue = LWE::KeyValue::DollarMarkKey;
            break;
          }
          case '5':
          {
            keyValue = LWE::KeyValue::PercentMarkKey;
            break;
          }
          case '6':
          {
            keyValue = LWE::KeyValue::CaretMarkKey;
            break;
          }
          case '7':
          {
            keyValue = LWE::KeyValue::AmpersandMarkKey;
            break;
          }
          case '8':
          {
            keyValue = LWE::KeyValue::AsteriskMarkKey;
            break;
          }
          case '9':
          {
            keyValue = LWE::KeyValue::LeftParenthesisMarkKey;
            break;
          }
          case '0':
          {
            keyValue = LWE::KeyValue::RightParenthesisMarkKey;
            break;
          }
        }
      }
      else
      {
        keyValue = ( LWE::KeyValue )( LWE::KeyValue::Digit0Key + ch - '0' );
      }
    }
    else if( ch >= 'a' && ch <= 'z' )
    {
      int kv = LWE::KeyValue::LowerAKey + ch - 'a';
      if( isShiftPressed )
      {
        kv -= ( 'z' - 'a' );
        kv -= 7;
      }
      keyValue = (LWE::KeyValue)kv;
    }
  }
  return keyValue;
}

} // unnamed namespace

class Locker
{
public:
  Locker( pthread_mutex_t& lock )
    : m_lock( lock )
  {
    pthread_mutex_lock( &m_lock );
  }

  ~Locker()
  {
    pthread_mutex_unlock( &m_lock );
  }
protected:
  pthread_mutex_t m_lock;
};

TizenWebEngineLWE::TizenWebEngineLWE()
: mUrl( "" ),
  mOutputWidth( 0 ),
  mOutputHeight( 0 ),
  mOutputStride( 0 ),
  mOutputBuffer ( NULL ),
  mIsMouseLbuttonDown( false ),
  mCanGoBack( false ),
  mCanGoForward( false ),
  mWebContainer( NULL ),
#ifdef DALI_USE_TBMSURFACE
  mTbmSurface( NULL ),
  mNativeImageSourcePtr( NULL ),
#else
  mBufferImage( NULL ),
#endif
  mUpdateBufferTrigger( MakeCallback( this, &TizenWebEngineLWE::UpdateBuffer ) )
{
  pthread_mutex_init( &mOutputBufferMutex, NULL );
}

TizenWebEngineLWE::~TizenWebEngineLWE()
{
  pthread_mutex_destroy( &mOutputBufferMutex );
}

void TizenWebEngineLWE::UpdateBuffer()
{
  Locker l( mOutputBufferMutex );

#ifdef DALI_USE_TBMSURFACE
  Dali::Stage::GetCurrent().KeepRendering( 0.0f );
#else
  if( mBufferImage )
  {
    mBufferImage.Update();
  }
#endif
}

void TizenWebEngineLWE::Create( int width, int height, const std::string& locale, const std::string& timezoneId )
{
  mOutputWidth = width;
  mOutputHeight = height;
  mOutputStride = width * sizeof( uint32_t );
  mOutputBuffer = ( uint8_t* )malloc( width * height * sizeof( uint32_t ) );

  mOnRenderedHandler = [this]( LWE::WebContainer* c, const LWE::WebContainer::RenderResult& renderResult )
  {
    size_t w = mOutputWidth;
    size_t h = mOutputHeight;
    if( renderResult.updatedWidth != w || renderResult.updatedHeight != h )
    {
      return;
    }
    Locker l(mOutputBufferMutex);
    uint8_t* dstBuffer;
    size_t dstStride;

#ifdef DALI_USE_TBMSURFACE
    tbm_surface_info_s tbmSurfaceInfo;
    if( tbm_surface_map( mTbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &tbmSurfaceInfo ) != TBM_SURFACE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Fail to map tbm_surface\n" );
    }

    DALI_ASSERT_ALWAYS( tbmSurfaceInfo.format == TBM_FORMAT_ARGB8888 && "Unsupported TizenWebEngineLWE tbm format" );
    dstBuffer = tbmSurfaceInfo.planes[0].ptr;
    dstStride = tbmSurfaceInfo.planes[0].stride;
#else
    dstBuffer = mBufferImage.GetBuffer();
    dstStride = mBufferImage.GetBufferStride();
#endif

    uint32_t srcStride = renderResult.updatedWidth * sizeof(uint32_t);
    uint8_t* srcBuffer = static_cast< uint8_t* >( renderResult.updatedBufferAddress );

    if( dstStride == srcStride )
    {
      memcpy( dstBuffer, srcBuffer, tbmSurfaceInfo.planes[0].size );
    }
    else
    {
      for ( auto y = renderResult.updatedY; y < ( renderResult.updatedHeight + renderResult.updatedY ); y++ )
      {
        auto start = renderResult.updatedX;
        memcpy( dstBuffer + ( y * dstStride ) + ( start * 4 ), srcBuffer + ( y * srcStride ) + ( start * 4 ), srcStride );
      }
    }

#ifdef DALI_USE_TBMSURFACE
    if( tbm_surface_unmap( mTbmSurface ) != TBM_SURFACE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Fail to unmap tbm_surface\n" );
    }
#endif
    mUpdateBufferTrigger.Trigger();
  };

  mOnReceivedError = []( LWE::WebContainer* container, LWE::ResourceError error ) {
  };

  mOnPageStartedHandler = []( LWE::WebContainer* container, const std::string& url ) {
  };

  mOnPageFinishedHandler = []( LWE::WebContainer* container, const std::string& url ) {
  };

  mOnLoadResourceHandler = []( LWE::WebContainer* container, const std::string& url ) {
  };

#ifdef DALI_USE_TBMSURFACE
  mTbmSurface = tbm_surface_create( width, height, TBM_FORMAT_ARGB8888 );
  mNativeImageSourcePtr = Dali::NativeImageSource::New( mTbmSurface );
#else
  mBufferImage = Dali::BufferImage::New( width, height, Dali::Pixel::BGRA8888 );
#endif

  if( !LWE::LWE::IsInitialized() )
  {
    std::string dataPath = DevelApplication::GetDataPath();
    LWE::LWE::Initialize( ( dataPath + DB_NAME_LOCAL_STORAGE ).c_str(),
                          ( dataPath + DB_NAME_COOKIES ).c_str(),
                          ( dataPath + DB_NAME_CACHE ).c_str() );
  }
  mWebContainer = LWE::WebContainer::Create( mOutputWidth, mOutputHeight, 1.0, "", locale.data(), timezoneId.data() );

  mWebContainer->RegisterPreRenderingHandler(
    [this]() -> LWE::WebContainer::RenderInfo {

        if(mOutputBuffer == NULL) {
            mOutputBuffer = (uint8_t*)malloc( mOutputWidth * mOutputHeight * sizeof(uint32_t));
            mOutputStride = mOutputWidth * sizeof(uint32_t);
        }

        ::LWE::WebContainer::RenderInfo result;
        result.updatedBufferAddress = mOutputBuffer;
        result.bufferStride = mOutputStride;

        return result;
    }
  );

  mWebContainer->RegisterOnRenderedHandler(
      [ this ]( LWE::WebContainer* container, const LWE::WebContainer::RenderResult& renderResult )
      {
        mOnRenderedHandler( container, renderResult );
      } );
  mWebContainer->RegisterOnReceivedErrorHandler(
      [ this ]( LWE::WebContainer* container, LWE::ResourceError error )
      {
        mCanGoBack = container->CanGoBack();
        mCanGoForward = container->CanGoForward();
        mOnReceivedError( container, error );
      });
  mWebContainer->RegisterOnPageStartedHandler(
      [ this ]( LWE::WebContainer* container, const std::string& url )
      {
        mUrl = url;
        mCanGoBack = container->CanGoBack();
        mCanGoForward = container->CanGoForward();
        mOnPageStartedHandler( container, url );
      });
  mWebContainer->RegisterOnPageLoadedHandler(
      [ this ]( LWE::WebContainer* container, const std::string& url )
      {
        mUrl = url;
        mCanGoBack = container->CanGoBack();
        mCanGoForward = container->CanGoForward();
        mOnPageFinishedHandler( container, url );
      });
  mWebContainer->RegisterOnLoadResourceHandler(
      [ this ]( LWE::WebContainer* container, const std::string& url )
      {
        mUrl = url;
        mCanGoBack = container->CanGoBack();
        mCanGoForward = container->CanGoForward();
        mOnLoadResourceHandler( container, url );
      });

}

void TizenWebEngineLWE::Create( int width, int height, int argc, char** argv )
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::Destroy()
{
  if( !mWebContainer )
  {
    return;
  }

#ifdef DALI_USE_TBMSURFACE
  if( mTbmSurface != NULL && tbm_surface_destroy( mTbmSurface ) != TBM_SURFACE_ERROR_NONE )
  {
    DALI_LOG_ERROR( "Failed to destroy tbm_surface\n" );
  }
#endif

  DestroyInstance();
  mWebContainer = NULL;
}

// NOT IMPLEMENTED
class NullWebEngineSettings : public Dali::WebEngineSettings
{
public:
  void AllowMixedContents( bool allowed ) override { }
  void EnableSpatialNavigation( bool enabled ) override { }
  uint32_t GetDefaultFontSize() const override { return 0; }
  void SetDefaultFontSize( uint32_t size ) override { }
  void EnableWebSecurity( bool enabled ) override { }
  void AllowFileAccessFromExternalUrl( bool allowed ) override { }
  bool IsJavaScriptEnabled() const override { return false; }
  void EnableJavaScript( bool enabled ) override { }
  void AllowScriptsOpenWindows( bool allowed ) override { }
  bool AreImagesLoadedAutomatically() const override { return false; }
  void AllowImagesLoadAutomatically( bool automatic ) override { }
  std::string GetDefaultTextEncodingName() const override { return EMPTY_STRING; }
  void SetDefaultTextEncodingName( const std::string& defaultTextEncodingName ) override { }
};

Dali::WebEngineSettings& TizenWebEngineLWE::GetSettings() const
{
  // NOT IMPLEMENTED
  static NullWebEngineSettings settings;
  return settings;
}

// NOT IMPLEMENTED
class NullWebEngineContext : public Dali::WebEngineContext
{
public:
  CacheModel GetCacheModel() const override { return Dali::WebEngineContext::CacheModel::DOCUMENT_VIEWER; }
  void SetCacheModel( CacheModel cacheModel ) override { }
  void SetProxyUri( const std::string& uri ) override { }
  void SetDefaultProxyAuth( const std::string& username, const std::string& password ) override { }
  void SetCertificateFilePath( const std::string& certificatePath ) override { }
  void DeleteWebDatabase() override { }
  void DeleteWebStorage() override { }
  void DeleteLocalFileSystem() override { }
  void DisableCache( bool cacheDisabled ) override { }
  void ClearCache() override { }
};

Dali::WebEngineContext& TizenWebEngineLWE::GetContext() const
{
  // NOT IMPLEMENTED
  static NullWebEngineContext context;
  return context;
}

// NOT IMPLEMENTED
class NullWebEngineCookieManager : public Dali::WebEngineCookieManager
{
public:
  void SetCookieAcceptPolicy( CookieAcceptPolicy policy ) override { }
  CookieAcceptPolicy GetCookieAcceptPolicy() const override { return Dali::WebEngineCookieManager::CookieAcceptPolicy::ALWAYS; }
  void SetPersistentStorage( const std::string& path, CookiePersistentStorage storage ) override { }
  void ClearCookies() override { }
};

Dali::WebEngineCookieManager& TizenWebEngineLWE::GetCookieManager() const
{
  // NOT IMPLEMENTED
  static NullWebEngineCookieManager cookieManager;
  return cookieManager;
}

// NOT IMPLEMENTED
class NullWebEngineBackForwardListItem : public Dali::WebEngineBackForwardListItem
{
public:
  std::string GetUrl() const override { return EMPTY_STRING; }
  std::string GetTitle() const override { return EMPTY_STRING; }
  std::string GetOriginalUrl() const override { return EMPTY_STRING; }
};

// NOT IMPLEMENTED
class NullWebEngineBackForwardList : public Dali::WebEngineBackForwardList
{
public:
  NullWebEngineBackForwardList( WebEngineBackForwardListItem* pItem ) : item( pItem ) { }
  Dali::WebEngineBackForwardListItem& GetCurrentItem() const override { return *item; }
  Dali::WebEngineBackForwardListItem& GetItemAtIndex( uint32_t index ) const override { return *item; }
  uint32_t GetItemCount() const override { return 1; }
private:
  WebEngineBackForwardListItem* item;
};

Dali::WebEngineBackForwardList& TizenWebEngineLWE::GetBackForwardList() const
{
  // NOT IMPLEMENTED
  static NullWebEngineBackForwardListItem item;
  static NullWebEngineBackForwardList list( &item );
  return list;
}

void TizenWebEngineLWE::DestroyInstance()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->Destroy();
}

Dali::NativeImageInterfacePtr TizenWebEngineLWE::GetNativeImageSource()
{
  return mNativeImageSourcePtr;
}

void TizenWebEngineLWE::LoadUrl( const std::string& url )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->LoadURL( url );
}

std::string TizenWebEngineLWE::GetTitle() const
{
  // NOT IMPLEMENTED
  static const std::string kEmpty;
  return kEmpty;
}

Dali::PixelData TizenWebEngineLWE::GetFavicon() const
{
  // NOT IMPLEMENTED
  return Dali::PixelData();
}

const std::string& TizenWebEngineLWE::GetUrl()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  return mUrl;
}

void TizenWebEngineLWE::LoadHtmlString( const std::string& str )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->LoadData( str );
}

void TizenWebEngineLWE::Reload()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->Reload();
}

void TizenWebEngineLWE::StopLoading()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->StopLoading();
}

void TizenWebEngineLWE::Suspend()
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::Resume()
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::ScrollBy( int deltaX, int deltaY )
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::SetScrollPosition( int x, int y )
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::GetScrollPosition(int& x, int& y) const
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::GetScrollSize( int& width, int& height ) const
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::GetContentSize( int& width, int& height ) const
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::GoBack()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->GoBack();
}

void TizenWebEngineLWE::GoForward()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->GoForward();
}

bool TizenWebEngineLWE::CanGoBack()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  return mCanGoBack;
}

bool TizenWebEngineLWE::CanGoForward()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  return mCanGoForward;
}

void TizenWebEngineLWE::EvaluateJavaScript( const std::string& script, std::function< void(const std::string&) > resultHandler )
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::AddJavaScriptMessageHandler( const std::string& exposedObjectName, std::function< void(const std::string&) > handler )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->AddJavaScriptInterface( exposedObjectName, "postMessage", [handler]( const std::string& data )->std::string {
    handler( data );
    return "";
  } );
}

void TizenWebEngineLWE::ClearAllTilesResources()
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::ClearHistory()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->ClearHistory();
  mCanGoBack = mWebContainer->CanGoBack();
}

const std::string& TizenWebEngineLWE::GetUserAgent() const
{
  // NOT IMPLEMENTED
  return EMPTY_STRING;
}

void TizenWebEngineLWE::SetUserAgent( const std::string& userAgent )
{
  // NOT IMPLEMENTED
}

void TizenWebEngineLWE::SetSize( int width, int height )
{
  DALI_ASSERT_ALWAYS( mWebContainer );

  if( mOutputWidth != ( size_t )width || mOutputHeight != ( size_t )height )
  {
    mOutputWidth = width;
    mOutputHeight = height;
    mOutputStride = width * sizeof(uint32_t);

#ifdef DALI_USE_TBMSURFACE
    tbm_surface_h prevTbmSurface = mTbmSurface;
    mTbmSurface = tbm_surface_create( width, height, TBM_FORMAT_ARGB8888 );
    Dali::Any source( mTbmSurface );
    mNativeImageSourcePtr->SetSource( source );
    if( prevTbmSurface != NULL && tbm_surface_destroy( prevTbmSurface ) != TBM_SURFACE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Failed to destroy tbm_surface\n" );
    }
#endif

    auto oldOutputBuffer = mOutputBuffer;
    mOutputBuffer = ( uint8_t* )malloc( mOutputWidth * mOutputHeight * sizeof( uint32_t ) );
    mOutputStride = mOutputWidth * sizeof( uint32_t );
    mWebContainer->ResizeTo( mOutputWidth, mOutputHeight );

    if( oldOutputBuffer ) {
        free(oldOutputBuffer);
    }
  }
}

void TizenWebEngineLWE::DispatchMouseDownEvent( float x, float y )
{
  DALI_ASSERT_ALWAYS( mWebContainer );

  mWebContainer->DispatchMouseDownEvent( LWE::MouseButtonValue::LeftButton, LWE::MouseButtonsValue::LeftButtonDown, x, y );
}

void TizenWebEngineLWE::DispatchMouseUpEvent( float x, float y )
{
  DALI_ASSERT_ALWAYS( mWebContainer );

  mWebContainer->DispatchMouseUpEvent( LWE::MouseButtonValue::NoButton, LWE::MouseButtonsValue::NoButtonDown, x, y );
}

void TizenWebEngineLWE::DispatchMouseMoveEvent( float x, float y, bool isLButtonPressed, bool isRButtonPressed )
{
  DALI_ASSERT_ALWAYS( mWebContainer );

  mWebContainer->DispatchMouseMoveEvent(
    isLButtonPressed ? LWE::MouseButtonValue::LeftButton
    : LWE::MouseButtonValue::NoButton,
    isLButtonPressed ? LWE::MouseButtonsValue::LeftButtonDown
    : LWE::MouseButtonsValue::NoButtonDown, x, y );
}

bool TizenWebEngineLWE::SendTouchEvent( const TouchEvent& touch )
{
  size_t pointCount = touch.GetPointCount();
  if( pointCount == 1 )
  {
    // Single touch event
    Dali::PointState::Type pointState = touch.GetState( 0 );
    const Dali::Vector2& screen = touch.GetLocalPosition( 0 );

    if( pointState == Dali::PointState::DOWN )
    {
      DispatchMouseDownEvent( screen.x, screen.y );
      mIsMouseLbuttonDown = true;
    }
    else if( pointState == Dali::PointState::UP )
    {
      DispatchMouseUpEvent( screen.x, screen.y );
      mIsMouseLbuttonDown = false;
    }
    else
    {
      DispatchMouseMoveEvent( screen.x, screen.y, mIsMouseLbuttonDown, false );
    }
  }

  return false;
}

void TizenWebEngineLWE::DispatchKeyDownEvent( LWE::KeyValue keyCode )
{
  DALI_ASSERT_ALWAYS( mWebContainer );

  mWebContainer->DispatchKeyDownEvent( keyCode );
}

void TizenWebEngineLWE::DispatchKeyPressEvent( LWE::KeyValue keyCode )
{
  DALI_ASSERT_ALWAYS( mWebContainer );

  mWebContainer->DispatchKeyPressEvent( keyCode );
}

void TizenWebEngineLWE::DispatchKeyUpEvent( LWE::KeyValue keyCode )
{
  DALI_ASSERT_ALWAYS( mWebContainer );

  mWebContainer->DispatchKeyUpEvent(keyCode);
}

bool TizenWebEngineLWE::SendKeyEvent( const Dali::KeyEvent& event )
{
  LWE::KeyValue keyValue = LWE::KeyValue::UnidentifiedKey;
  if( 32 < event.GetKeyString().c_str()[0] && 127 > event.GetKeyString().c_str()[0] )
  {
    keyValue = static_cast<LWE::KeyValue>( event.GetKeyString().c_str()[0] );
  }
  else
  {
    keyValue = KeyStringToKeyValue( event.GetKeyName().c_str(), event.GetKeyModifier() & 1 );
  }
  if( event.GetState() == Dali::KeyEvent::DOWN )
  {
    DispatchKeyDownEvent( keyValue );
    DispatchKeyPressEvent( keyValue );
  }
  else if( event.GetState() == Dali::KeyEvent::UP )
  {
    DispatchKeyUpEvent( keyValue );
  }

  return false;
}

void TizenWebEngineLWE::SetFocus( bool focused )
{
  // NOT IMPLEMENTED
}

} // namespace Plugin
} // namespace Dali

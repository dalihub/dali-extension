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

// CLASS HEADER
#include "tizen-web-engine-lite.h"

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/public-api/common/stage.h>
#include <dali/public-api/events/key-event.h>
#include <dali/public-api/events/touch-data.h>

#include <unistd.h>
#include <pthread.h>

// The plugin factories
extern "C" DALI_EXPORT_API Dali::WebEnginePlugin* CreateWebEnginePlugin( void )
{
  return new Dali::Plugin::TizenWebEngineLite;
}

extern "C" DALI_EXPORT_API void DestroyWebEnginePlugin( Dali::WebEnginePlugin* plugin )
{
  if( plugin != NULL )
  {
    delete plugin;
  }
}

static LWE::KeyValue KeyStringToKeyValue( const char* DALIKeyString, bool isShiftPressed )
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

class Locker {
public:
  Locker(pthread_mutex_t& lock)
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

namespace Dali
{

namespace Plugin
{

namespace
{

const int TIMER_INTERVAL( 20 );

} // unnamed namespace

TizenWebEngineLite::TizenWebEngineLite()
: mIsMouseLbuttonDown( false ),
  mTimer(),
  mUrl( "" ),
  mOutputWidth( 0 ),
  mOutputHeight( 0 ),
  mOutputStride( 0 ),
  mOutputBuffer ( NULL ),
  mCanGoBack( false ),
  mCanGoForward( false ),
  mIsRunning( false ),
  mIsNeedsUpdate( true ),
  mWebContainer( NULL ),
#ifdef STARFISH_DALI_TBMSURFACE
  mTbmSurface( NULL ),
  mNativeImageSourcePtr( NULL )
#else
  mBufferImage( NULL )
#endif
{
  pthread_mutex_init(&mOutputBufferMutex, NULL);
}

TizenWebEngineLite::~TizenWebEngineLite()
{
  pthread_mutex_destroy(&mOutputBufferMutex);
}

bool TizenWebEngineLite::UpdateBuffer()
{
  if( mIsRunning == false )
  {
    return true;
  }

  if( mIsNeedsUpdate )
  {
    Locker l(mOutputBufferMutex);
#ifdef STARFISH_DALI_TBMSURFACE
    Dali::Stage::GetCurrent().KeepRendering( 0.0f );
#else
    if( !mBufferImage )
    {
      return false;
    }
    mBufferImage.Update();
#endif
    mIsNeedsUpdate = false;
  }

  return true;
}

void TizenWebEngineLite::Create( int width, int height, const std::string& locale, const std::string& timezoneId )
{
  mTimer = Dali::Timer::New( TIMER_INTERVAL );
  mTimer.TickSignal().Connect( this, &TizenWebEngineLite::UpdateBuffer );
  mTimer.Start();

  mIsRunning = true;
  mOutputWidth = width;
  mOutputHeight = height;
  mOutputStride = width * sizeof( uint32_t );
  mOutputBuffer = ( uint8_t* )malloc( width * height * sizeof( uint32_t ) );

  onRenderedHandler = [this]( LWE::WebContainer* c, const LWE::WebContainer::RenderResult& renderResult )
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

#ifdef STARFISH_DALI_TBMSURFACE
    tbm_surface_info_s tbmSurfaceInfo;
    if( tbm_surface_map( mTbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &tbmSurfaceInfo ) != TBM_SURFACE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Fail to map tbm_surface\n" );
    }

    DALI_ASSERT_ALWAYS( tbmSurfaceInfo.format == TBM_FORMAT_ARGB8888 && "Unsupported TizenWebEngineLite tbm format" );
    dstBuffer = tbmSurfaceInfo.planes[0].ptr;
    dstStride = tbmSurfaceInfo.planes[0].stride;
#else
    dstBuffer = mBufferImage.GetBuffer();
    dstStride = mBufferImage.GetBufferStride();
#endif

    uint32_t srcStride = renderResult.updatedWidth * sizeof(uint32_t);
    uint8_t* srcBuffer = static_cast< uint8_t* >( renderResult.updatedBufferAddress );

    if (dstStride == srcStride)
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

#ifdef STARFISH_DALI_TBMSURFACE
    if( tbm_surface_unmap( mTbmSurface ) != TBM_SURFACE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Fail to unmap tbm_surface\n" );
    }
#endif
    mIsNeedsUpdate = true;
  };

  onReceivedError = []( LWE::WebContainer* container, LWE::ResourceError error ) {
  };
  onPageStartedHandler = []( LWE::WebContainer* container, const std::string& url ) {
  };
  onPageFinishedHandler = []( LWE::WebContainer* container, const std::string& url ) {
  };
  onLoadResourceHandler = []( LWE::WebContainer* container, const std::string& url ) {
  };

#ifdef STARFISH_DALI_TBMSURFACE
  mTbmSurface = tbm_surface_create( width, height, TBM_FORMAT_ARGB8888 );
  mNativeImageSourcePtr = Dali::NativeImageSource::New( mTbmSurface );
#else
  mBufferImage = Dali::BufferImage::New( width, height, Dali::Pixel::BGRA8888 );
#endif

  if ( !LWE::LWE::IsInitialized() )
  {
    LWE::LWE::Initialize("/tmp/StarFish_localStorage.txt",
                         "/tmp/StarFish_Cookies.txt", "/tmp/StarFish-cache");
  }
  mWebContainer = LWE::WebContainer::Create(
      mOutputBuffer, mOutputWidth, mOutputHeight,
      mOutputStride, 1.0, "SamsungOne", locale.data(), timezoneId.data() );
  mWebContainer->RegisterOnRenderedHandler(
      [ this ]( LWE::WebContainer* container, const LWE::WebContainer::RenderResult& renderResult )
      {
        onRenderedHandler( container, renderResult );
      } );
  mWebContainer->RegisterOnReceivedErrorHandler(
      [ this ]( LWE::WebContainer* container, LWE::ResourceError error )
      {
        mCanGoBack = container->CanGoBack();
        mCanGoForward = container->CanGoForward();
        onReceivedError( container, error );
      });
  mWebContainer->RegisterOnPageStartedHandler(
      [ this ]( LWE::WebContainer* container, const std::string& url )
      {
        mUrl = url;
        mCanGoBack = container->CanGoBack();
        mCanGoForward = container->CanGoForward();
        onPageStartedHandler( container, url );
      });
  mWebContainer->RegisterOnPageLoadedHandler(
      [ this ]( LWE::WebContainer* container, const std::string& url )
      {
        mUrl = url;
        mCanGoBack = container->CanGoBack();
        mCanGoForward = container->CanGoForward();
        onPageFinishedHandler( container, url );
      });
  mWebContainer->RegisterOnLoadResourceHandler(
      [ this ]( LWE::WebContainer* container, const std::string& url )
      {
        mUrl = url;
        mCanGoBack = container->CanGoBack();
        mCanGoForward = container->CanGoForward();
        onLoadResourceHandler( container, url );
      });

}

void TizenWebEngineLite::Destroy()
{
  if( !mWebContainer )
  {
    return;
  }

  if( mIsRunning == true )
  {
    mIsRunning = false;

#ifdef STARFISH_DALI_TBMSURFACE
    if( mTbmSurface != NULL && tbm_surface_destroy( mTbmSurface ) != TBM_SURFACE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Failed to destroy tbm_surface\n" );
    }
#endif

    DestroyInstance();
    mWebContainer = NULL;
  }
}

void TizenWebEngineLite::DestroyInstance()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->Destroy();
}

Dali::NativeImageInterfacePtr TizenWebEngineLite::GetNativeImageSource()
{
  return mNativeImageSourcePtr;
}

void TizenWebEngineLite::LoadUrl( const std::string& url )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->LoadURL( url );
}

const std::string& TizenWebEngineLite::GetUrl()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  return mUrl;
}

void TizenWebEngineLite::LoadHTMLString( const std::string& str )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->LoadData( str );
}

void TizenWebEngineLite::Reload()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->Reload();
}

void TizenWebEngineLite::StopLoading()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->StopLoading();
}

void TizenWebEngineLite::GoBack()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->GoBack();
}

void TizenWebEngineLite::GoForward()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->GoForward();
}

bool TizenWebEngineLite::CanGoBack()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  return mCanGoBack;
}

bool TizenWebEngineLite::CanGoForward()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  return mCanGoForward;
}

void TizenWebEngineLite::EvaluateJavaScript( const std::string& script )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->EvaluateJavaScript( script );
}

void TizenWebEngineLite::AddJavaScriptMessageHandler( const std::string& exposedObjectName, std::function< void(const std::string&) > callback )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->AddJavaScriptInterface( exposedObjectName, "postMessage", [callback]( const std::string& data )->std::string {
    callback( data );
    return "";
  } );
}

void TizenWebEngineLite::ClearHistory()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->ClearHistory();
  mCanGoBack = mWebContainer->CanGoBack();
}

void TizenWebEngineLite::ClearCache()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  mWebContainer->ClearCache();
}

void TizenWebEngineLite::SetSize( int width, int height )
{
  DALI_ASSERT_ALWAYS( mWebContainer );

  if( mOutputWidth != ( size_t )width || mOutputHeight != ( size_t )height )
  {
	  mOutputWidth = width;
	  mOutputHeight = height;
	  mOutputStride = width * sizeof(uint32_t);

#ifdef STARFISH_DALI_TBMSURFACE
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
	  mWebContainer->UpdateBuffer( mOutputBuffer, mOutputWidth,
		mOutputHeight, mOutputStride );

    if (oldOutputBuffer) {
	      free(oldOutputBuffer);
	  }
  }
}

void TizenWebEngineLite::DispatchMouseDownEvent( float x, float y )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  if (!mIsRunning)
  {
    return;
  }

  mWebContainer->DispatchMouseDownEvent( LWE::MouseButtonValue::LeftButton, LWE::MouseButtonsValue::LeftButtonDown, x, y );
}

void TizenWebEngineLite::DispatchMouseUpEvent( float x, float y )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  if (!mIsRunning)
  {
    return;
  }

  mWebContainer->DispatchMouseUpEvent( LWE::MouseButtonValue::NoButton, LWE::MouseButtonsValue::NoButtonDown, x, y );
}

void TizenWebEngineLite::DispatchMouseMoveEvent( float x, float y, bool isLButtonPressed, bool isRButtonPressed )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  if (!mIsRunning)
  {
    return;
  }

  mWebContainer->DispatchMouseMoveEvent(
    isLButtonPressed ? LWE::MouseButtonValue::LeftButton
    : LWE::MouseButtonValue::NoButton,
    isLButtonPressed ? LWE::MouseButtonsValue::LeftButtonDown
    : LWE::MouseButtonsValue::NoButtonDown, x, y );
}

bool TizenWebEngineLite::SendTouchEvent( const TouchData& touch )
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

void TizenWebEngineLite::DispatchKeyDownEvent( LWE::KeyValue keyCode )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  if (!mIsRunning)
  {
    return;
  }

  mWebContainer->DispatchKeyDownEvent( keyCode );
}

void TizenWebEngineLite::DispatchKeyPressEvent( LWE::KeyValue keyCode )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  if (!mIsRunning)
  {
    return;
  }

  mWebContainer->DispatchKeyPressEvent( keyCode );
}

void TizenWebEngineLite::DispatchKeyUpEvent( LWE::KeyValue keyCode )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  if (!mIsRunning)
  {
    return;
  }

  mWebContainer->DispatchKeyUpEvent(keyCode);
}

bool TizenWebEngineLite::SendKeyEvent( const Dali::KeyEvent& event )
{
  LWE::KeyValue keyValue = LWE::KeyValue::UnidentifiedKey;
  if( 32 < event.keyPressed.c_str()[0] && 127 > event.keyPressed.c_str()[0] )
  {
    keyValue = static_cast<LWE::KeyValue>(event.keyPressed.c_str()[0]);
  }
  else
  {
    keyValue = KeyStringToKeyValue( event.keyPressedName.c_str(), event.keyModifier & 1 );
  }
  if( event.state == Dali::KeyEvent::Down )
  {
    DispatchKeyDownEvent( keyValue );
    DispatchKeyPressEvent( keyValue );
  }
  else if( event.state == Dali::KeyEvent::Up )
  {
    DispatchKeyUpEvent( keyValue );
  }

  return false;
}

} // namespace Plugin
} // namespace Dali

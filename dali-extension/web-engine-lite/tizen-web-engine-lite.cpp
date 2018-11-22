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

#include <libtuv/uv.h>
#include <unistd.h>

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

#define TO_CONTAINER(ptr) (((TizenWebEngineLite*)ptr)->mWebContainer)

static bool gIsNeedsUpdate = false;
static bool gIsFirstTime = true;
static uv_async_t gLauncherHandle;
static pthread_mutex_t gMutex;
static bool gIsAliveMainLoop = false;
static int gDaliNumber = 0;

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

struct UVAsyncHandleData {
  std::function<void(void*)> cb;
  void* data;
};

static bool IsAliveMainThread()
{
  return gIsAliveMainLoop;
}

static void InitMainThread( void* (*f)(void*), pthread_t& t )
{
  pthread_mutex_init(&gMutex, NULL);

  pthread_mutex_lock(&gMutex);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_create(&t, &attr, f, NULL);
  pthread_mutex_lock(&gMutex);
  pthread_mutex_unlock(&gMutex);
}

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


namespace Dali
{

namespace Plugin
{

namespace
{

const int TIMER_INTERVAL( 20 );

} // unnamed namespace

TizenWebEngineLite::TizenWebEngineLite()
: mThreadHandle(),
  mIsMouseLbuttonDown( false ),
  mTimer(),
  mUrl( "" ),
  mOutputWidth( 0 ),
  mOutputHeight( 0 ),
  mOutputStride( 0 ),
  mOutputBuffer ( NULL ),
  mCanGoBack( false ),
  mCanGoForward( false ),
  mIsRunning( false ),
  mWebContainer( NULL ),
#ifdef STARFISH_DALI_TBMSURFACE
  mTbmSurface( NULL ),
  mNativeImageSourcePtr( NULL )
#else
  mBufferImage( NULL )
#endif
{
}

TizenWebEngineLite::~TizenWebEngineLite()
{
}

bool TizenWebEngineLite::UpdateBuffer()
{
  if( mIsRunning == false )
  {
    return true;
  }

  if( gIsNeedsUpdate )
  {
    Locker l( gMutex );
#ifdef STARFISH_DALI_TBMSURFACE
    Dali::Stage::GetCurrent().KeepRendering( 0.0f );
#else
    if( !mBufferImage )
    {
      return false;
    }
    mBufferImage.Update();
#endif
    gIsNeedsUpdate = false;
  }

  return true;
}

void TizenWebEngineLite::StartMainThreadIfNeeds()
{
  if ( !IsAliveMainThread() )
  {
    InitMainThread( StartMainThread, mThreadHandle );
  }
}

void TizenWebEngineLite::CreateInstance()
{
  gDaliNumber++;
  auto cb = []( void* data )
  {
    TizenWebEngineLite* engine = static_cast< TizenWebEngineLite* >( data );
    if ( !LWE::LWE::IsInitialized() )
    {
      LWE::LWE::Initialize("/tmp/StarFish_localStorage.txt",
                           "/tmp/StarFish_Cookies.txt", "/tmp/StarFish-cache");
    }
    engine->mWebContainer = LWE::WebContainer::Create(
        engine->mOutputBuffer, engine->mOutputWidth, engine->mOutputHeight,
        engine->mOutputStride, 1.0, "SamsungOne", "ko-KR", "Asia/Seoul" );
    TO_CONTAINER( data )->RegisterOnRenderedHandler(
        [ engine ]( LWE::WebContainer* container, const LWE::WebContainer::RenderResult& renderResult )
        {
          engine->onRenderedHandler( container, renderResult );
        } );
    TO_CONTAINER( data )->RegisterOnReceivedErrorHandler(
        [ engine ]( LWE::WebContainer* container, LWE::ResourceError error )
        {
          engine->mCanGoBack = container->CanGoBack();
          engine->mCanGoForward = container->CanGoForward();
          engine->onReceivedError( container, error );
        });
    TO_CONTAINER( data )->RegisterOnPageStartedHandler(
        [ engine ]( LWE::WebContainer* container, const std::string& url )
        {
          engine->mUrl = url;
          engine->mCanGoBack = container->CanGoBack();
          engine->mCanGoForward = container->CanGoForward();
          engine->onPageStartedHandler( container, url );
        });
    TO_CONTAINER( data )->RegisterOnPageLoadedHandler(
        [ engine ]( LWE::WebContainer* container, const std::string& url )
        {
          engine->mUrl = url;
          engine->mCanGoBack = container->CanGoBack();
          engine->mCanGoForward = container->CanGoForward();
          engine->onPageFinishedHandler( container, url );
        });
    TO_CONTAINER( data )->RegisterOnLoadResourceHandler(
        [ engine ]( LWE::WebContainer* container, const std::string& url )
        {
          engine->mUrl = url;
          engine->mCanGoBack = container->CanGoBack();
          engine->mCanGoForward = container->CanGoForward();
          engine->onLoadResourceHandler( container, url );
        });
    };
    SendAsyncHandle( cb );
}

void TizenWebEngineLite::Create( int width, int height, const std::string& locale, const std::string& timezoneId )
{
  mTimer = Dali::Timer::New( TIMER_INTERVAL );
  mTimer.TickSignal().Connect( this, &TizenWebEngineLite::UpdateBuffer );
  mTimer.Start();

  StartMainThreadIfNeeds();

  mIsRunning = true;
  mOutputWidth = width;
  mOutputHeight = height;
  mOutputStride = width * sizeof( uint32_t );
  mOutputBuffer = ( uint8_t* )malloc( width * height * sizeof( uint32_t ) );

  if( gIsFirstTime == true )
  {
    gIsFirstTime = false;

    onRenderedHandler = [this]( LWE::WebContainer* c, const LWE::WebContainer::RenderResult& renderResult )
    {
      Locker l( gMutex );
      size_t w = mOutputWidth;
      size_t h = mOutputHeight;
      if( renderResult.updatedWidth != w || renderResult.updatedHeight != h )
      {
        return;
      }

      uint8_t* dstBuffer;
      size_t dstStride;

#ifdef STARFISH_DALI_TBMSURFACE
      tbm_surface_info_s tbmSurfaceInfo;
      if( tbm_surface_map( mTbmSurface, TBM_SURF_OPTION_READ | TBM_SURF_OPTION_WRITE, &tbmSurfaceInfo ) != TBM_SURFACE_ERROR_NONE )
      {
        DALI_LOG_ERROR( "Fail to map tbm_surface\n" );
      }

      DALI_ASSERT_ALWAYS( tbmSurfaceInfo.format == TBM_FORMAT_ABGR8888 && "Unsupported TizenWebEngineLite tbm format" );

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
      gIsNeedsUpdate = true;
    };

    onReceivedError = []( LWE::WebContainer* container, LWE::ResourceError error ) {
    };
    onPageStartedHandler = []( LWE::WebContainer* container, const std::string& url ) {
    };
    onPageFinishedHandler = []( LWE::WebContainer* container, const std::string& url ) {
    };
    onLoadResourceHandler = []( LWE::WebContainer* container, const std::string& url ) {
    };
  }

#ifdef STARFISH_DALI_TBMSURFACE
  mTbmSurface = tbm_surface_create( width, height, TBM_FORMAT_ABGR8888 );
  mNativeImageSourcePtr = Dali::NativeImageSource::New( mTbmSurface );
#else
  mBufferImage = Dali::BufferImage::New( width, height, Dali::Pixel::RGBA8888 );
#endif

  CreateInstance();
  while ( true )
  {
      if ( mWebContainer )
      {
          break;
      }
      usleep( 100 );
  }
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
    StopLoop();

    int status;
    pthread_join( mThreadHandle, ( void** )&status );

    mWebContainer = NULL;
  }
}

void TizenWebEngineLite::DestroyInstance()
{
	DALI_ASSERT_ALWAYS( mWebContainer );
	auto cb = []( void* data )
	{
	  TizenWebEngineLite* engine = static_cast< TizenWebEngineLite* >( data );

	  TO_CONTAINER( data )->Destroy();

	  while ( !engine->mAsyncHandlePool.empty() )
	  {
	    UVAsyncHandleData* handleData = NULL;
	    {
	      Locker l( gMutex );
	      handleData = ( UVAsyncHandleData* )*engine->mAsyncHandlePool.begin();
	      engine->mAsyncHandlePool.erase( engine->mAsyncHandlePool.begin() );
	    }

	    if ( handleData ) {
	      handleData->cb( handleData->data );
	      delete handleData;
	    }
	  }
	  gDaliNumber--;
	};
	SendAsyncHandle( cb );
}

Dali::NativeImageInterfacePtr TizenWebEngineLite::GetNativeImageSource()
{
  return mNativeImageSourcePtr;
}

void TizenWebEngineLite::LoadUrl( const std::string& url )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = [url]( void* data )
  {
    TO_CONTAINER( data )->LoadURL( url );
  };
  SendAsyncHandle( cb );
}

const std::string& TizenWebEngineLite::GetUrl()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  return mUrl;
}

void TizenWebEngineLite::LoadHTMLString( const std::string& str )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = [str]( void* data )
  {
    TO_CONTAINER( data )->LoadData( str );
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::Reload()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = []( void* data )
  {
    TO_CONTAINER( data )->Reload();
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::StopLoading()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = []( void* data )
  {
    TO_CONTAINER( data )->StopLoading();
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::GoBack()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = []( void* data )
  {
    TO_CONTAINER( data )->GoBack();
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::GoForward()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = []( void* data )
  {
    TO_CONTAINER( data )->GoForward();
  };
  SendAsyncHandle( cb );
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
  auto cb = [script]( void* data ) {
      TO_CONTAINER( data )->EvaluateJavaScript( script );
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::AddJavaScriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName, std::function< std::string(const std::string&) > callback )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = [exposedObjectName, jsFunctionName, callback]( void* data )
  {
    TO_CONTAINER( data )->AddJavaScriptInterface( exposedObjectName, jsFunctionName, callback );
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::RemoveJavascriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = [exposedObjectName, jsFunctionName]( void* data )
  {
    TO_CONTAINER( data )->RemoveJavascriptInterface( exposedObjectName, jsFunctionName );
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::ClearHistory()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = []( void* data )
  {
    TizenWebEngineLite* engine = static_cast< TizenWebEngineLite* >( data );
    TO_CONTAINER( data )->ClearHistory();
    engine->mCanGoBack = TO_CONTAINER( data )->CanGoBack();
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::ClearCache()
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  auto cb = []( void* data )
  {
    TO_CONTAINER( data )->ClearCache();
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::SetSize( int width, int height )
{
  DALI_ASSERT_ALWAYS( mWebContainer );

  if( mOutputWidth != ( size_t )width || mOutputHeight != ( size_t )height )
  {
    Locker l( gMutex );
	  mOutputWidth = width;
	  mOutputHeight = height;
	  mOutputStride = width * sizeof(uint32_t);

#ifdef STARFISH_DALI_TBMSURFACE
	  tbm_surface_h prevTbmSurface = mTbmSurface;
	  mTbmSurface = tbm_surface_create( width, height, TBM_FORMAT_ABGR8888 );
	  Dali::Any source( mTbmSurface );
	  mNativeImageSourcePtr->SetSource( source );
	  if( prevTbmSurface != NULL && tbm_surface_destroy( prevTbmSurface ) != TBM_SURFACE_ERROR_NONE )
	  {
		  DALI_LOG_ERROR( "Failed to destroy tbm_surface\n" );
	  }
#endif

	  auto cb = []( void* data )
    {
	    TizenWebEngineLite* engine = static_cast< TizenWebEngineLite* >( data );

	    Locker l( gMutex );
	    if (engine->mOutputBuffer) {
	      free(engine->mOutputBuffer);
	      engine->mOutputBuffer = NULL;
	    }

		  engine->mOutputBuffer = ( uint8_t* )malloc( engine->mOutputWidth * engine->mOutputHeight * sizeof( uint32_t ) );
		  engine->mOutputStride = engine->mOutputWidth * sizeof( uint32_t );
		  engine->mWebContainer->UpdateBuffer( engine->mOutputBuffer, engine->mOutputWidth,
		      engine->mOutputHeight, engine->mOutputStride );
	  };
	  SendAsyncHandle( cb );
  }
}

void TizenWebEngineLite::DispatchMouseDownEvent( float x, float y )
{
	DALI_ASSERT_ALWAYS( mWebContainer );
	if (!mIsRunning)
	{
	  return;
	}

	auto cb = [x, y]( void* data )
  {
	  TO_CONTAINER( data )->DispatchMouseDownEvent( LWE::MouseButtonValue::LeftButton, LWE::MouseButtonsValue::LeftButtonDown, x, y );
  };
	SendAsyncHandle( cb );
}

void TizenWebEngineLite::DispatchMouseUpEvent( float x, float y )
{
  DALI_ASSERT_ALWAYS( mWebContainer );
  if (!mIsRunning)
  {
    return;
  }

  auto cb = [x, y]( void* data )
  {
    TO_CONTAINER( data )->DispatchMouseUpEvent( LWE::MouseButtonValue::NoButton, LWE::MouseButtonsValue::NoButtonDown, x, y );
  };
  SendAsyncHandle( cb );
}

void TizenWebEngineLite::DispatchMouseMoveEvent( float x, float y, bool isLButtonPressed, bool isRButtonPressed )
{
	DALI_ASSERT_ALWAYS( mWebContainer );
	if (!mIsRunning)
	{
	  return;
	}

	auto cb = [x, y, isLButtonPressed]( void* data )
  {
	  TO_CONTAINER( data )->DispatchMouseMoveEvent(
	      isLButtonPressed ? LWE::MouseButtonValue::LeftButton
	          : LWE::MouseButtonValue::NoButton,
	      isLButtonPressed ? LWE::MouseButtonsValue::LeftButtonDown
	          : LWE::MouseButtonsValue::NoButtonDown, x, y );
  };
	SendAsyncHandle( cb );
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

	auto cb = [keyCode]( void* data )
  {
	  TO_CONTAINER( data )->DispatchKeyDownEvent( keyCode );
  };
	SendAsyncHandle( cb );
}

void TizenWebEngineLite::DispatchKeyPressEvent( LWE::KeyValue keyCode )
{
	DALI_ASSERT_ALWAYS( mWebContainer );
	if (!mIsRunning)
	{
	  return;
	}

	auto cb = [keyCode]( void* data )
  {
	  TO_CONTAINER( data )->DispatchKeyPressEvent( keyCode );
  };
	SendAsyncHandle( cb );
}

void TizenWebEngineLite::DispatchKeyUpEvent( LWE::KeyValue keyCode )
{
	DALI_ASSERT_ALWAYS( mWebContainer );
	if (!mIsRunning)
	{
	  return;
	}

	auto cb = [keyCode]( void* data )
  {
	  TO_CONTAINER( data )->DispatchKeyUpEvent(keyCode);
  };
	SendAsyncHandle( cb );
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

void TizenWebEngineLite::CallEmptyAsyncHandle()
{
	DALI_ASSERT_ALWAYS( mWebContainer );
	auto cb = []( void* data ) {
  };
	SendAsyncHandle( cb );
}

void TizenWebEngineLite::StopLoop()
{
  gDaliNumber = -1;
  CallEmptyAsyncHandle();
}

void TizenWebEngineLite::SendAsyncHandle(std::function<void(void*)> cb)
{
  UVAsyncHandleData* handle = new UVAsyncHandleData();
  handle->cb = cb;
  handle->data = this;

  {
    Locker l( gMutex );
    mAsyncHandlePool.push_back( ( size_t )handle );
  }

  gLauncherHandle.data = this;
  uv_async_send(&gLauncherHandle);
}

void* TizenWebEngineLite::StartMainThread( void* data )
{
  uv_async_init( uv_default_loop(), &gLauncherHandle, []( uv_async_t* handle )
  {
    Dali::Plugin::TizenWebEngineLite* engine = static_cast< Dali::Plugin::TizenWebEngineLite* >(handle->data);
    while ( !engine->mAsyncHandlePool.empty() )
    {
      UVAsyncHandleData* handleData = NULL;
      {
        Locker l( gMutex );
        handleData = ( UVAsyncHandleData* )*engine->mAsyncHandlePool.begin();
        engine->mAsyncHandlePool.erase( engine->mAsyncHandlePool.begin() );
      }

      if ( handleData )
      {
        handleData->cb( handleData->data );
        delete handleData;
      }
    }
  });

  gIsAliveMainLoop = true;
  pthread_mutex_unlock( &gMutex );
  while ( true )
  {
    uv_run( uv_default_loop(), UV_RUN_ONCE );
    if ( gDaliNumber < 0 )
    {
      break;
    }
  }
  return NULL;
}

} // namespace Plugin
} // namespace Dali

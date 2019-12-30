/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-renderer.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/property-array.h>
#include <dali/integration-api/debug.h>
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/devel-api/images/native-image-interface-extension.h>
#include <cstring> // for strlen()
#include <tbm_surface_internal.h>

// INTERNAL INCLUDES
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-manager.h>

// The plugin factories
extern "C" DALI_EXPORT_API Dali::VectorAnimationRendererPlugin* CreateVectorAnimationRendererPlugin( void )
{
  return new Dali::Plugin::TizenVectorAnimationRenderer;
}

namespace Dali
{

namespace Plugin
{

namespace
{

const char* const DEFAULT_SAMPLER_TYPENAME( "sampler2D" );
const char* const PIXEL_AREA_UNIFORM_NAME( "pixelArea" );

const Vector4 FULL_TEXTURE_RECT( 0.f, 0.f, 1.f, 1.f );

} // unnamed namespace

TizenVectorAnimationRenderer::TizenVectorAnimationRenderer()
: mUrl(),
  mBuffers(),
  mMutex(),
  mRenderer(),
  mTexture(),
  mRenderedTexture(),
  mTargetSurface(),
  mVectorRenderer(),
  mUploadCompletedSignal(),
  mTbmQueue( NULL ),
  mTotalFrameNumber( 0 ),
  mWidth( 0 ),
  mHeight( 0 ),
  mDefaultWidth( 0 ),
  mDefaultHeight( 0 ),
  mFrameRate( 60.0f ),
  mResourceReady( false ),
  mShaderChanged( false ),
  mResourceReadyTriggered( false )
{
}

TizenVectorAnimationRenderer::~TizenVectorAnimationRenderer()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  ResetBuffers();
}

bool TizenVectorAnimationRenderer::Initialize( const std::string& url )
{
  mUrl = url;

  mVectorRenderer = rlottie::Animation::loadFromFile( mUrl );
  if( !mVectorRenderer )
  {
    DALI_LOG_ERROR( "Failed to load a Lottie file [%s]\n", mUrl.c_str() );
    return false;
  }

  mTotalFrameNumber = mVectorRenderer->totalFrame();
  mFrameRate = static_cast< float >( mVectorRenderer->frameRate() );

  size_t w, h;
  mVectorRenderer->size( w, h );
  mDefaultWidth = static_cast< uint32_t >( w );
  mDefaultHeight = static_cast< uint32_t >( h );

  TizenVectorAnimationManager::Get().AddEventHandler( *this );

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::Initialize: file [%s] [%p]\n", url.c_str(), this );

  return true;
}

void TizenVectorAnimationRenderer::Finalize()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  TizenVectorAnimationManager::Get().RemoveEventHandler( *this );

  mRenderer.Reset();
  mTexture.Reset();
  mRenderedTexture.Reset();
  mVectorRenderer.reset();

  mTargetSurface = nullptr;
  mTbmQueue = NULL;
}

void TizenVectorAnimationRenderer::SetRenderer( Renderer renderer )
{
  mRenderer = renderer;
  mShaderChanged = false;

  if( mTargetSurface )
  {
    Dali::Mutex::ScopedLock lock( mMutex );

    if( mResourceReady && mRenderedTexture )
    {
      TextureSet textureSet = renderer.GetTextures();

      textureSet.SetTexture( 0, mRenderedTexture );

      mUploadCompletedSignal.Emit();
    }

    SetShader();
  }

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::SetRenderer [%p]\n", this );
}

void TizenVectorAnimationRenderer::SetSize( uint32_t width, uint32_t height )
{
  Dali::Mutex::ScopedLock lock( mMutex );

  if( mWidth == width && mHeight == height )
  {
    DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::SetSize: Same size (%d, %d) [%p]\n", mWidth, mHeight, this );
    return;
  }

  mTargetSurface = NativeImageSourceQueue::New( width, height, NativeImageSourceQueue::COLOR_DEPTH_DEFAULT );

  mTexture = Texture::New( *mTargetSurface );

  if( mRenderer )
  {
    SetShader();
  }

  mTbmQueue = AnyCast< tbm_surface_queue_h >( mTargetSurface->GetNativeImageSourceQueue() );

  mWidth = width;
  mHeight = height;

  mResourceReady = false;

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::SetSize: width = %d, height = %d [%p]\n", mWidth, mHeight, this );
}

bool TizenVectorAnimationRenderer::Render( uint32_t frameNumber )
{
  Dali::Mutex::ScopedLock lock( mMutex );

  if( !mTbmQueue || !mVectorRenderer )
  {
    return false;
  }

  if( tbm_surface_queue_can_dequeue( mTbmQueue, 0 ) )
  {
    tbm_surface_h tbmSurface;

    if( tbm_surface_queue_dequeue( mTbmQueue, &tbmSurface ) != TBM_SURFACE_QUEUE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Failed to dequeue a tbm_surface [%p]\n", this );
      return false;
    }

    tbm_surface_info_s info;
    tbm_surface_map( tbmSurface, TBM_OPTION_WRITE, &info );

    rlottie::Surface surface;
    bool existing = false;

    if( !mResourceReady )
    {
      // Need to reset buffer list
      ResetBuffers();
    }
    else
    {
      for( auto&& iter : mBuffers )
      {
        if( iter.first == tbmSurface )
        {
          // Find the buffer in the existing list
          existing = true;
          surface = iter.second;
          break;
        }
      }
    }

    if( !existing )
    {
      tbm_surface_internal_ref( tbmSurface );

      unsigned char* buffer = info.planes[0].ptr;

      // Create Surface object
      surface = rlottie::Surface( reinterpret_cast< uint32_t* >( buffer ), mWidth, mHeight, static_cast< size_t >( info.planes[0].stride ) );

      // Push the buffer
      mBuffers.push_back( SurfacePair( tbmSurface, surface ) );
    }

    // Render the frame
    mVectorRenderer->renderSync( frameNumber, surface );

    tbm_surface_unmap( tbmSurface );

    tbm_surface_queue_enqueue( mTbmQueue, tbmSurface );

    if( !mResourceReady )
    {
      mRenderedTexture = mTexture;
      mResourceReady = true;
      mResourceReadyTriggered = true;

      TizenVectorAnimationManager::Get().TriggerEvent( *this );

      DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::Render: Resource ready [current = %d] [%p]\n", frameNumber, this );
    }

    return true;
  }

  return false;
}

uint32_t TizenVectorAnimationRenderer::GetTotalFrameNumber() const
{
  return mTotalFrameNumber;
}

float TizenVectorAnimationRenderer::GetFrameRate() const
{
  return mFrameRate;
}

void TizenVectorAnimationRenderer::GetDefaultSize( uint32_t& width, uint32_t& height ) const
{
  width = mDefaultWidth;
  height = mDefaultHeight;

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::GetDefaultSize: width = %d, height = %d [%p]\n", width, height, this );
}

void TizenVectorAnimationRenderer::GetLayerInfo( Property::Map& map ) const
{
  Dali::Mutex::ScopedLock lock( mMutex );

  if( mVectorRenderer )
  {
    auto layerInfo = mVectorRenderer->layers();

    for( auto&& iter : layerInfo )
    {
      Property::Array frames;
      frames.PushBack( std::get< 1 >( iter ) );
      frames.PushBack( std::get< 2 >( iter ) );
      map.Add( std::get< 0 >( iter ), frames );
    }
  }
}

bool TizenVectorAnimationRenderer::GetMarkerInfo( const std::string& marker, uint32_t& startFrame, uint32_t& endFrame ) const
{
  Dali::Mutex::ScopedLock lock( mMutex );

  if( mVectorRenderer )
  {
    auto markerList = mVectorRenderer->markers();
    for( auto&& iter : markerList )
    {
      if( std::get< 0 >( iter ).compare( marker ) == 0 )
      {
        startFrame = static_cast< uint32_t >( std::get< 1 >( iter ) );
        endFrame = static_cast< uint32_t >( std::get< 2 >( iter ) );
        return true;
      }
    }
  }
  return false;
}

void TizenVectorAnimationRenderer::IgnoreRenderedFrame()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  if( mTargetSurface )
  {
    mTargetSurface->IgnoreSourceImage();
  }
}

VectorAnimationRendererPlugin::UploadCompletedSignalType& TizenVectorAnimationRenderer::UploadCompletedSignal()
{
  return mUploadCompletedSignal;
}

void TizenVectorAnimationRenderer::NotifyEvent()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  if( mResourceReadyTriggered )
  {
    DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::NotifyEvent: Set Texture [%p]\n", this );

    // Set texture
    if( mRenderer && mRenderedTexture )
    {
      TextureSet textureSet = mRenderer.GetTextures();
      textureSet.SetTexture( 0, mRenderedTexture );
    }

    mResourceReadyTriggered = false;

    mUploadCompletedSignal.Emit();
  }
}

void TizenVectorAnimationRenderer::SetShader()
{
  if( mShaderChanged )
  {
    return;
  }

  NativeImageInterface::Extension* extension = static_cast< NativeImageInterface* >( mTargetSurface.Get() )->GetExtension();
  if( extension )
  {
    Shader shader = mRenderer.GetShader();

    std::string fragmentShader;
    std::string vertexShader;

    // Get custom fragment shader prefix
    const char* fragmentPreFix = extension->GetCustomFragmentPreFix();
    if( fragmentPreFix )
    {
      fragmentShader = fragmentPreFix;
      fragmentShader += "\n";
    }

    // Get the current fragment shader source
    Property::Value program = shader.GetProperty( Shader::Property::PROGRAM );
    Property::Map* map = program.GetMap();
    if( map )
    {
      Property::Value* fragment = map->Find( "fragment" );
      if( fragment )
      {
        fragmentShader += fragment->Get< std::string >();
      }

      Property::Value* vertex = map->Find( "vertex" );
      if( vertex )
      {
        vertexShader = vertex->Get< std::string >();
      }
    }

    // Get custom sampler type name
    const char* customSamplerTypename = extension->GetCustomSamplerTypename();
    if( customSamplerTypename )
    {
      size_t position = fragmentShader.find( DEFAULT_SAMPLER_TYPENAME );
      if( position != std::string::npos )
      {
        fragmentShader.replace( position, strlen( DEFAULT_SAMPLER_TYPENAME ), customSamplerTypename );
      }
    }

    // Set the modified shader again
    Shader newShader = Shader::New( vertexShader, fragmentShader );
    newShader.RegisterProperty( PIXEL_AREA_UNIFORM_NAME, FULL_TEXTURE_RECT );

    mRenderer.SetShader( newShader );

    mShaderChanged = true;
  }
}

void TizenVectorAnimationRenderer::ResetBuffers()
{
  for( auto&& iter : mBuffers )
  {
    tbm_surface_internal_unref( iter.first );
  }
  mBuffers.clear();
}

} // namespace Plugin

} // namespace Dali;

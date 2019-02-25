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
#include <dali-extension/vector-animation-renderer/tizen-vector-animation-renderer.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>
#include <dali/devel-api/adaptor-framework/native-image-source-queue.h>
#include <dali/devel-api/images/native-image-interface-extension.h>
#include <cstring> // for strlen()
#include <tbm_surface_internal.h>

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
  mTargetSurface(),
  mVectorRenderer(),
  mTbmQueue( NULL ),
  mTotalFrameNumber( 0 ),
  mWidth( 0 ),
  mHeight( 0 )
{
}

TizenVectorAnimationRenderer::~TizenVectorAnimationRenderer()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  for( auto&& iter : mBuffers )
  {
    tbm_surface_internal_unref( iter.first );
  }
  mBuffers.clear();

  if( mTbmQueue != NULL )
  {
    tbm_surface_queue_destroy( mTbmQueue );
  }
}

void TizenVectorAnimationRenderer::SetUrl( const std::string& url )
{
  mUrl = url;

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::SetUrl: file [%s]\n", url.c_str() );
}

void TizenVectorAnimationRenderer::SetRenderer( Renderer renderer )
{
  mRenderer = renderer;

  if( mTargetSurface )
  {
    TextureSet textureSet = renderer.GetTextures();

    Texture texture = Texture::New( *mTargetSurface );
    textureSet.SetTexture( 0, texture );

    SetShader();
  }

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::SetRenderer\n" );
}

void TizenVectorAnimationRenderer::SetSize( uint32_t width, uint32_t height )
{
  Dali::Mutex::ScopedLock lock( mMutex );

  if( mWidth == width && mHeight == height )
  {
    DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::SetSize: Same size (%d, %d)\n", mWidth, mHeight );
    return;
  }

  if( !mTargetSurface )
  {
    mTargetSurface = NativeImageSourceQueue::New( width, height, NativeImageSourceQueue::COLOR_DEPTH_DEFAULT );

    if( mRenderer )
    {
      TextureSet textureSet = mRenderer.GetTextures();

      Texture texture = Texture::New( *mTargetSurface );
      textureSet.SetTexture( 0, texture );

      SetShader();
    }

    mTbmQueue = AnyCast< tbm_surface_queue_h >( mTargetSurface->GetNativeImageSourceQueue() );
  }
  else
  {
    mTargetSurface->SetSize( width, height );
  }

  mWidth = width;
  mHeight = height;

  // Reset the buffer list
  for( auto&& iter : mBuffers )
  {
    tbm_surface_internal_unref( iter.first );
  }
  mBuffers.clear();

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::SetSize: width = %d, height = %d\n", mWidth, mHeight );
}

bool TizenVectorAnimationRenderer::StartRender()
{
  mVectorRenderer = lottie::Animation::loadFromFile( mUrl );
  if( !mVectorRenderer )
  {
    DALI_LOG_ERROR( "Failed to load a Lottie file [%s]\n", mUrl.c_str() );
    return false;
  }

  mTotalFrameNumber = mVectorRenderer->totalFrame();

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::StartRender: file [%s]\n", mUrl.c_str() );

  return true;
}

void TizenVectorAnimationRenderer::StopRender()
{
  Dali::Mutex::ScopedLock lock( mMutex );

  if( mTbmQueue )
  {
    tbm_surface_queue_flush( mTbmQueue );

    DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::StopRender: Stopped\n" );
  }
}

void TizenVectorAnimationRenderer::Render( uint32_t frameNumber )
{
  if( tbm_surface_queue_can_dequeue( mTbmQueue, 1 ) )
  {
    Dali::Mutex::ScopedLock lock( mMutex );

    tbm_surface_h tbmSurface;

    if( tbm_surface_queue_dequeue( mTbmQueue, &tbmSurface ) != TBM_SURFACE_QUEUE_ERROR_NONE )
    {
      DALI_LOG_ERROR( "Failed to dequeue a tbm_surface\n" );
      return;
    }

    tbm_surface_info_s info;
    tbm_surface_map( tbmSurface, TBM_OPTION_WRITE, &info );

    bool existing = false;
    for( auto&& iter : mBuffers )
    {
      if( iter.first == tbmSurface )
      {
        // Find the buffer in the existing list
        existing = true;

        // Render the frame
        mVectorRenderer->renderSync( frameNumber, iter.second );
        break;
      }
    }

    if( !existing )
    {
      tbm_surface_internal_ref( tbmSurface );

      unsigned char* buffer = info.planes[0].ptr;

      // Create Surface object
      lottie::Surface surface( reinterpret_cast< uint32_t* >( buffer ), mWidth, mHeight, static_cast< size_t >( info.planes[0].stride ) );

      // Push the buffer
      mBuffers.push_back( SurfacePair( tbmSurface, surface ) );

      // Render the frame
      mVectorRenderer->renderSync( frameNumber, surface );
    }

    tbm_surface_unmap( tbmSurface );

    tbm_surface_queue_enqueue( mTbmQueue, tbmSurface );
  }
  else
  {
    DALI_LOG_ERROR( "Cannot dequeue a tbm_surface [%d]\n", frameNumber );
  }
}

uint32_t TizenVectorAnimationRenderer::GetTotalFrameNumber()
{
  return mTotalFrameNumber;
}

void TizenVectorAnimationRenderer::SetShader()
{
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

    DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::SetShader: Shader is changed\n" );
  }
}

} // namespace Plugin

} // namespace Dali;

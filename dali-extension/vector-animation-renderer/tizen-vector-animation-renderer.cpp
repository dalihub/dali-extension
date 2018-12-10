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
  mTargetSurface(),
  mRenderer(),
  mTbmQueue( NULL ),
  mTotalFrameNumber( 0 ),
  mWidth( 0 ),
  mHeight( 0 )
{
}

TizenVectorAnimationRenderer::~TizenVectorAnimationRenderer()
{
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

bool TizenVectorAnimationRenderer::CreateRenderer( const std::string& url, Renderer renderer, uint32_t width, uint32_t height )
{
  mUrl = url;
  mWidth = width;
  mHeight = height;

  mTargetSurface = NativeImageSourceQueue::New( mWidth, mHeight, NativeImageSourceQueue::COLOR_DEPTH_DEFAULT );

  TextureSet textureSet = renderer.GetTextures();

  Texture texture = Texture::New( *mTargetSurface );
  textureSet.SetTexture( 0, texture );

  SetShader( renderer );

  mTbmQueue = AnyCast< tbm_surface_queue_h >( mTargetSurface->GetNativeImageSourceQueue() );

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::CreateRenderer: file [%s]\n", url.c_str() );

  return true;
}

bool TizenVectorAnimationRenderer::StartRender()
{
  mRenderer = lottie::Animation::loadFromFile( mUrl );
  if( !mRenderer )
  {
    DALI_LOG_ERROR( "Failed to load a Lottie file [%s]\n", mUrl.c_str() );
    return false;
  }

  mTotalFrameNumber = mRenderer->totalFrame();

  DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::StartRender: file [%s]\n", mUrl.c_str() );

  return true;
}

void TizenVectorAnimationRenderer::StopRender()
{
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
    tbm_surface_h tbmSurface;

    tbm_surface_queue_dequeue( mTbmQueue, &tbmSurface );

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
        mRenderer->renderSync( frameNumber, iter.second );
        break;
      }
    }

    if( !existing )
    {
      tbm_surface_internal_ref( tbmSurface );

      unsigned char* buffer = info.planes[0].ptr;

      // Create Surface object
      lottie::Surface surface( reinterpret_cast< uint32_t* >( buffer ), static_cast< size_t >( info.width ), static_cast< size_t >( info.height ), static_cast< size_t >( info.planes[0].stride ) );

      // Push the buffer
      mBuffers.push_back( SurfacePair( tbmSurface, surface ) );

      // Render the frame
      mRenderer->renderSync( frameNumber, surface );
    }

    tbm_surface_unmap( tbmSurface );

    tbm_surface_queue_enqueue( mTbmQueue, tbmSurface );
  }
}

uint32_t TizenVectorAnimationRenderer::GetTotalFrameNumber()
{
  return mTotalFrameNumber;
}

void TizenVectorAnimationRenderer::SetShader( Renderer renderer )
{
  NativeImageInterface::Extension* extension = static_cast< NativeImageInterface* >( mTargetSurface.Get() )->GetExtension();
  if( extension )
  {
    Shader shader = renderer.GetShader();

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
      fragmentShader += fragment->Get< std::string >();

      Property::Value* vertex = map->Find( "vertex" );
      vertexShader = vertex->Get< std::string >();
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

    renderer.SetShader( newShader );

    DALI_LOG_RELEASE_INFO( "TizenVectorAnimationRenderer::SetShader: Shader is changed\n" );
  }
}

} // namespace Plugin

} // namespace Dali;
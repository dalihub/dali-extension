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
#include <dali-extension/vector-image-renderer/tizen-vector-image-renderer.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

// The plugin factories
extern "C" DALI_EXPORT_API Dali::VectorImageRendererPlugin* CreateVectorImageRendererPlugin( void )
{
  return new Dali::Plugin::TizenVectorImageRenderer;
}

namespace Dali
{

namespace Plugin
{

TizenVectorImageRenderer::TizenVectorImageRenderer()
: mPicture(nullptr),
  mDefaultWidth(0),
  mDefaultHeight(0),
  mIsFirstRender(true)
{
  tvg::Initializer::init(tvg::CanvasEngine::Sw, 0);

  mSwCanvas = tvg::SwCanvas::gen();
  mSwCanvas->mempool(tvg::SwCanvas::MempoolPolicy::Individual);
}

TizenVectorImageRenderer::~TizenVectorImageRenderer()
{
  //Not yet pushed to Canvas
  if(mIsFirstRender && mPicture)
  {
    delete(mPicture);
  }

  tvg::Initializer::term(tvg::CanvasEngine::Sw);
}

bool TizenVectorImageRenderer::Load(const Vector<uint8_t>& data)
{
  if(!mSwCanvas)
  {
    DALI_LOG_ERROR("TizenVectorImageRenderer::Load Canvas Object is null [%p]\n", this);
    return false;
  }

  if(!mPicture)
  {
    mPicture = tvg::Picture::gen().release();
    if(!mPicture)
    {
      DALI_LOG_ERROR("TizenVectorImageRenderer::Load: Picture gen Fail [%p]\n", this);
      return false;
    }
  }

  if(mPicture->load(reinterpret_cast<char*>(data.Begin()), data.Size(), false) != tvg::Result::Success)
  {
    DALI_LOG_ERROR("TizenVectorImageRenderer::Load Data load Fail %s [%p]\n", data, this);
    return false;
  }

  float w, h;
  mPicture->viewbox(nullptr, nullptr, &w, &h);
  mDefaultWidth = static_cast<uint32_t>(w);
  mDefaultHeight = static_cast<uint32_t>(h);

  return true;
}

bool TizenVectorImageRenderer::Rasterize(Dali::Devel::PixelBuffer& buffer)
{
  if(!mSwCanvas || !mPicture)
  {
    DALI_LOG_ERROR("TizenVectorImageRenderer::Rasterize: either Canvas[%p] or Picture[%p] is invalid [%p]\n", mSwCanvas.get(), mPicture, this);
    return false;
  }

  unsigned char* pBuffer;
  pBuffer = buffer.GetBuffer();

  if(!pBuffer)
  {
    DALI_LOG_ERROR("TizenVectorImageRenderer::Rasterize: pixel buffer is null [%p]\n", this);
    return false;
  }

  auto width = buffer.GetWidth();
  auto height = buffer.GetHeight();

  mSwCanvas->target(reinterpret_cast<uint32_t*>(pBuffer), width, width, height, tvg::SwCanvas::ABGR8888);

  DALI_LOG_RELEASE_INFO("TizenVectorImageRenderer::Rasterize: Buffer[%p] size[%d x %d]! [%p]\n", pBuffer, width, height, this);

  mPicture->size(width, height);

  /* We need to push picture first time, after that we only update its properties. */
  if(mIsFirstRender)
  {
    if(mSwCanvas->push(std::unique_ptr<tvg::Picture>(mPicture)) != tvg::Result::Success)
    {
      DALI_LOG_ERROR("TizenVectorImageRenderer::Rasterize: Picture push fail [%p]\n", this);
      return false;
    }
    mIsFirstRender = false;
  }
  else
  {
    if(mSwCanvas->update(mPicture) != tvg::Result::Success)
    {
      DALI_LOG_ERROR("TizenVectorImageRenderer::Rasterize: Picture update fail [%p]\n", this);
      return false;
    }
  }

  if(mSwCanvas->draw() != tvg::Result::Success)
  {
    DALI_LOG_ERROR("TizenVectorImageRenderer::Rasterize: Draw fail [%p]\n", this);
    return false;
  }

  mSwCanvas->sync();

  return true;
}

void TizenVectorImageRenderer::GetDefaultSize( uint32_t &width, uint32_t &height ) const
{
  width = mDefaultWidth;
  height = mDefaultHeight;
}

} // namespace Plugin

} // namespace Dali;

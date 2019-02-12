#ifndef DALI_TIZEN_WEB_ENGINE_LWE_H
#define DALI_TIZEN_WEB_ENGINE_LWE_H

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

// EXTERNAL INCLUDES
#include <string.h>
#include <vector>
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>
#include <LWEWebView.h>
#include <list>

#ifndef DALI_USE_TBMSURFACE
#define DALI_USE_TBMSURFACE
#endif

#ifdef DALI_USE_TBMSURFACE
#include <tbm_surface.h>
#else
#include <dali/public-api/images/buffer-image.h>
#endif

namespace Dali
{

namespace Plugin
{

/**
 * @brief Implementation of the Tizen WebEngineLWE class which has Tizen platform dependency.
 */
class TizenWebEngineLWE : public Dali::WebEnginePlugin, public Dali::ConnectionTracker
{
public:

  /**
   * @brief Constructor.
   */
  TizenWebEngineLWE();

  /**
   * @brief Destructor.
   */
  virtual ~TizenWebEngineLWE();

  /**
   * @copydoc Dali::WebEnginePlugin::Create()
   */
  virtual void Create( int width, int height, const std::string& locale, const std::string& timezoneId );

  /**
   * @copydoc Dali::WebEnginePlugin::Destroy()
   */
  virtual void Destroy();

  /**
   * @copydoc Dali::WebEnginePlugin::GetNativeImageSource()
   */
  virtual Dali::NativeImageInterfacePtr GetNativeImageSource();

  /**
   * @copydoc Dali::WebEnginePlugin::LoadUrl()
   */
  virtual void LoadUrl( const std::string& url );

  /**
   * @copydoc Dali::WebEnginePlugin::GetUrl()
   */
  virtual const std::string& GetUrl();

  /**
   * @copydoc Dali::WebEnginePlugin::LoadHTMLString()
   */
  virtual void LoadHTMLString( const std::string& string );

  /**
   * @copydoc Dali::WebEnginePlugin::Reload()
   */
  virtual void Reload();

  /**
   * @copydoc Dali::WebEnginePlugin::StopLoading()
   */
  virtual void StopLoading();

  /**
   * @copydoc Dali::WebEnginePlugin::GoBack()
   */
  virtual void GoBack();

  /**
   * @copydoc Dali::WebEnginePlugin::GoForward()
   */
  virtual void GoForward();

  /**
   * @copydoc Dali::WebEnginePlugin::CanGoBack()
   */
  virtual bool CanGoBack();

  /**
   * @copydoc Dali::WebEnginePlugin::CanGoForward()
   */
  virtual bool CanGoForward();

  /**
   * @copydoc Dali::WebEnginePlugin::AddJavaScriptMessageHandler()
   */
  virtual void AddJavaScriptMessageHandler( const std::string& exposedObjectName, std::function< void(const std::string&) > handler );

  /**
   * @copydoc Dali::WebEnginePlugin::EvaluateJavaScript()
   */
  virtual void EvaluateJavaScript( const std::string& script );

  /**
   * @copydoc Dali::WebEnginePlugin::ClearHistory()
   */
  virtual void ClearHistory();

  /**
   * @copydoc Dali::WebEnginePlugin::ClearCache()
   */
  virtual void ClearCache();

  /**
   * @copydoc Dali::WebEnginePlugin::SetSize()
   */
  virtual void SetSize( int width, int height );

  /**
   * @copydoc Dali::WebEnginePlugin::SendTouchEvent()
   */
  virtual bool SendTouchEvent( const Dali::TouchData& touch );

  /**
   * @copydoc Dali::WebEnginePlugin::SendKeyEvent()
   */
  virtual bool SendKeyEvent( const Dali::KeyEvent& event );

  /**
   * @copydoc Dali::WebEnginePlugin::PageLoadStartedSignal()
   */
  virtual Dali::WebEnginePlugin::WebEngineSignalType& PageLoadStartedSignal()
  {
    return mPageLoadStartedSignal;
  }

  /**
   * @copydoc Dali::WebEnginePlugin::PageLoadFinishedSignal()
   */
  virtual Dali::WebEnginePlugin::WebEngineSignalType& PageLoadFinishedSignal()
  {
    return mPageLoadFinishedSignal;
  }

private:

  void UpdateBuffer();

  void DestroyInstance();

  void DispatchMouseDownEvent(float x, float y);

  void DispatchMouseUpEvent(float x, float y);

  void DispatchMouseMoveEvent(float x, float y, bool isLButtonPressed, bool isRButtonPressed);

  void DispatchKeyDownEvent(LWE::KeyValue keyCode);

  void DispatchKeyPressEvent(LWE::KeyValue keyCode);

  void DispatchKeyUpEvent(LWE::KeyValue keyCode);

private:

  std::string                mUrl;
  size_t                     mOutputWidth;
  size_t                     mOutputHeight;
  size_t                     mOutputStride;
  uint8_t*                   mOutputBuffer;
  bool                       mIsMouseLbuttonDown;
  bool                       mCanGoBack;
  bool                       mCanGoForward;
  pthread_mutex_t            mOutputBufferMutex;
  LWE::WebContainer*         mWebContainer;
#ifdef DALI_USE_TBMSURFACE
  tbm_surface_h              mTbmSurface;
  Dali::NativeImageSourcePtr mNativeImageSourcePtr;
#else
  Dali::BufferImage          mBufferImage;
#endif

  std::function<void(LWE::WebContainer*, const LWE::WebContainer::RenderResult&)> mOnRenderedHandler;
  std::function<void(LWE::WebContainer*, LWE::ResourceError)> mOnReceivedError;
  std::function<void(LWE::WebContainer*, const std::string&)> mOnPageFinishedHandler;
  std::function<void(LWE::WebContainer*, const std::string&)> mOnPageStartedHandler;
  std::function<void(LWE::WebContainer*, const std::string&)> mOnLoadResourceHandler;

  EventThreadCallback                        mUpdateBufferTrigger;
  Dali::WebEnginePlugin::WebEngineSignalType mPageLoadStartedSignal;
  Dali::WebEnginePlugin::WebEngineSignalType mPageLoadFinishedSignal;
};

} // namespace Plugin
} // namespace Dali;

#endif

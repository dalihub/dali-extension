#ifndef DALI_TIZEN_WEB_ENGINE_LITE_H
#define DALI_TIZEN_WEB_ENGINE_LITE_H

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
#include <dali/devel-api/threading/mutex.h>
#include <dali/public-api/adaptor-framework/native-image-source.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/devel-api/adaptor-framework/web-engine-plugin.h>
#include <LWEWebView.h>
#include <list>

#ifndef STARFISH_TIZEN_TV
#define STARFISH_TIZEN_TV
#endif
#ifndef STARFISH_DALI_TBMSURFACE
#define STARFISH_DALI_TBMSURFACE
#endif

#ifdef STARFISH_DALI_TBMSURFACE
#include <tbm_surface.h>
#else
#include <dali/public-api/images/buffer-image.h>
#endif

namespace Dali
{

namespace Plugin
{

/**
 * @brief Implementation of the Tizen WebEngineLite class which has Tizen platform dependency.
 */
class TizenWebEngineLite : public Dali::WebEnginePlugin, public Dali::ConnectionTracker
{
public:

  /**
   * @brief Constructor.
   */
  TizenWebEngineLite();

  /**
   * @brief Destructor.
   */
  virtual ~TizenWebEngineLite();

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
  Dali::NativeImageInterfacePtr GetNativeImageSource();

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
   * @copydoc Dali::WebEnginePlugin::AddJavaScriptInterface()
   */
  virtual void AddJavaScriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName, std::function< std::string(const std::string&) > cb );

  /**
   * @copydoc Dali::WebEnginePlugin::EvaluateJavaScript()
   */
  virtual void EvaluateJavaScript( const std::string& script );

  /**
   * @copydoc Dali::WebEnginePlugin::RemoveJavascriptInterface()
   */
  virtual void RemoveJavascriptInterface( const std::string& exposedObjectName, const std::string& jsFunctionName );

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
   * @copydoc Dali::WebEnginePlugin::FinishedSignal()
   */
  virtual Dali::WebEnginePlugin::WebEngineSignalType& PageLoadStartedSignal()
  {
    return mPageLoadStartedSignal;
  }

  virtual Dali::WebEnginePlugin::WebEngineSignalType& PageLoadFinishedSignal()
  {
    return mPageLoadFinishedSignal;
  }

private:

  bool UpdateBuffer();

  void StartMainThreadIfNeeds();
  void CreateInstance();
  void DestroyInstance();
  void CallEmptyAsyncHandle();
  void StopLoop();

  void DispatchMouseDownEvent(float x, float y);
  void DispatchMouseUpEvent(float x, float y);
  void DispatchMouseMoveEvent(float x, float y, bool isLButtonPressed, bool isRButtonPressed);
  void DispatchKeyDownEvent(LWE::KeyValue keyCode);
  void DispatchKeyPressEvent(LWE::KeyValue keyCode);
  void DispatchKeyUpEvent(LWE::KeyValue keyCode);

  void SendAsyncHandle(std::function<void(void*)> cb);
  static void* StartMainThread(void* data);

  pthread_t mThreadHandle;
  bool mIsMouseLbuttonDown;
  Dali::Timer mTimer;

  std::string mUrl;
  size_t mOutputWidth;
  size_t mOutputHeight;
  size_t mOutputStride;
  uint8_t* mOutputBuffer;
  bool mCanGoBack, mCanGoForward;
  bool mIsRunning;

  LWE::WebContainer* mWebContainer;
  std::list<size_t> mAsyncHandlePool;

#ifdef STARFISH_DALI_TBMSURFACE
  tbm_surface_h mTbmSurface;
  Dali::NativeImageSourcePtr mNativeImageSourcePtr;
#else
  Dali::BufferImage mBufferImage;
#endif

  std::function<void(LWE::WebContainer*, const LWE::WebContainer::RenderResult&)> onRenderedHandler;
  std::function<void(LWE::WebContainer*, LWE::ResourceError)> onReceivedError;
  std::function<void(LWE::WebContainer*, const std::string&)> onPageFinishedHandler;
  std::function<void(LWE::WebContainer*, const std::string&)> onPageStartedHandler;
  std::function<void(LWE::WebContainer*, const std::string&)> onLoadResourceHandler;

public:

  Dali::WebEnginePlugin::WebEngineSignalType mPageLoadStartedSignal;
  Dali::WebEnginePlugin::WebEngineSignalType mPageLoadFinishedSignal;
};

} // namespace Plugin
} // namespace Dali;

#endif

#ifndef DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_MANAGER_H
#define DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_MANAGER_H

/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd.
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
#include <dali/integration-api/processor-interface.h>
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/public-api/signals/callback.h>
#include <memory>
#include <mutex>

namespace Dali
{
namespace Extension
{
namespace Internal
{
class RiveAnimationThread;
class RiveAnimationManager;

/**
 * @brief Rive animation manager
 */
class RiveAnimationManager : public Integration::Processor
{
public:
  struct LifecycleObserver
  {
    virtual void RiveAnimationManagerDestroyed() = 0;
  };

  /**
   * @brief GetInstance.
   */
  static RiveAnimationManager& GetInstance();

  /**
   * @brief Constructor.
   */
  RiveAnimationManager();

  /**
   * @brief Destructor.
   */
  ~RiveAnimationManager() override;

  /**
   * Add a lifecycle observer
   * @param[in] observer The object watching this one
   */
  void AddObserver(LifecycleObserver& observer);

  /**
   * Remove a lifecycle observer
   * @param[in] observer The object watching this one
   */
  void RemoveObserver(LifecycleObserver& observer);

  /**
   * Get the rive animation thread.
   * @return A raw pointer pointing to the rive animation thread.
   */
  RiveAnimationThread& GetRiveAnimationThread();

  /**
   * @brief Register a callback.
   *
   * @param callback The callback to register
   * @note Ownership of the callback is passed onto this class.
   */
  void RegisterEventCallback(CallbackBase* callback);

  /**
   * @brief Unregister a previously registered callback
   *
   * @param callback The callback to unregister
   */
  void UnregisterEventCallback(CallbackBase* callback);

protected: // Implementation of Processor
  /**
   * @copydoc Dali::Integration::Processor::Process()
   */
  void Process(bool postProcessor) override;

private:
  // Undefined
  RiveAnimationManager(const RiveAnimationManager& manager) = delete;

  // Undefined
  RiveAnimationManager& operator=(const RiveAnimationManager& manager) = delete;

private:
  static std::unique_ptr<RiveAnimationManager> mInstance;
  static std::once_flag                        mOnceFlag;

  std::vector<CallbackBase*>           mEventCallbacks;
  std::vector<LifecycleObserver*>      mLifecycleObservers;
  std::unique_ptr<RiveAnimationThread> mRiveAnimationThread;
  bool                                 mProcessorRegistered;
};

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif // DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_MANAGER_H

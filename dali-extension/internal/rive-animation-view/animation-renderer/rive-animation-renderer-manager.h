#ifndef DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_RENDERER_MANAGER_H
#define DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_RENDERER_MANAGER_H

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
#include <dali/public-api/common/vector-wrapper.h>
#include <dali/devel-api/threading/mutex.h>
#include <dali/devel-api/adaptor-framework/event-thread-callback.h>
#include <dali/integration-api/processor-interface.h>
#include <memory>

// INTERNAL INCLUDES
#include <dali-extension/internal/rive-animation-view/animation-renderer/rive-animation-renderer-event-handler.h>

namespace Dali
{
namespace Extension
{
namespace Internal
{

/**
 * @brief Rive animation renderer manager
 */
class RiveAnimationRendererManager : public Integration::Processor
{
public:

  /**
   * @brief Create or retrieve RiveAnimationRendererManager singleton.
   *
   * @return A reference to the RiveAnimationRendererManager.
   */
  static RiveAnimationRendererManager& Get();

  /**
   * @brief Add the event handler.
   *
   * @param handler The event handler to add.
   */
  void AddEventHandler(RiveAnimationRendererEventHandler& handler);

  /**
   * @brief Remove the event handler.
   *
   * @param handler The event handler to remove.
   */
  void RemoveEventHandler(RiveAnimationRendererEventHandler& handler);

  /**
   * @brief Trigger the event.
   */
  void TriggerEvent(RiveAnimationRendererEventHandler& handler);

protected: // Implementation of Processor

  /**
   * @copydoc Dali::Integration::Processor::Process()
   */
  void Process(bool postProcessor) override;

private:

  /**
   * @brief Event callback to process events.
   */
  void OnEventTriggered();

private:

  /**
   * @brief Constructor.
   */
  RiveAnimationRendererManager();

  /**
   * @brief Destructor.
   */
  virtual ~RiveAnimationRendererManager();

  // Undefined
  RiveAnimationRendererManager(const RiveAnimationRendererManager&) = delete;

  // Undefined
  RiveAnimationRendererManager& operator=(const RiveAnimationRendererManager&) = delete;

private:

  std::vector<RiveAnimationRendererEventHandler*> mEventHandlers;
  std::vector<RiveAnimationRendererEventHandler*> mTriggeredHandlers;
  Dali::Mutex                                     mMutex;
  std::unique_ptr<EventThreadCallback>            mEventTrigger;
};

} // namespace Internal

} // namespace Extension

} // namespace Dali

#endif // DALI_EXTENSION_INTERNAL_RIVE_ANIMATION_RENDERER_MANAGER_H

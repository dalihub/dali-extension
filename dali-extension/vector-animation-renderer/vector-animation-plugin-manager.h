#ifndef DALI_VECTOR_ANIMATION_PLUGIN_MANAGER_H
#define DALI_VECTOR_ANIMATION_PLUGIN_MANAGER_H

/*
 * Copyright (c) 2023 Samsung Electronics Co., Ltd.
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
#include <dali-extension/vector-animation-renderer/vector-animation-event-handler.h>

namespace Dali
{

namespace Plugin
{

/**
 * @brief Tizen vector animation manager
 */
class VectorAnimationPluginManager : public Integration::Processor
{
public:

  /**
   * @brief Create or retrieve VectorAnimationPluginManager singleton.
   *
   * @return A reference to the VectorAnimationPluginManager.
   */
  static VectorAnimationPluginManager& Get();

  /**
   * @brief Add the event handler.
   *
   * @param handler The event handler to add.
   */
  void AddEventHandler( VectorAnimationEventHandler& handler );

  /**
   * @brief Remove the event handler.
   *
   * @param handler The event handler to remove.
   */
  void RemoveEventHandler( VectorAnimationEventHandler& handler );

  /**
   * @brief Trigger the event.
   */
  void TriggerEvent( VectorAnimationEventHandler& handler );

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
  VectorAnimationPluginManager();

  /**
   * @brief Destructor.
   */
  virtual ~VectorAnimationPluginManager();

  // Undefined
  VectorAnimationPluginManager( const VectorAnimationPluginManager& ) = delete;

  // Undefined
  VectorAnimationPluginManager& operator=( const VectorAnimationPluginManager& ) = delete;

private:

  std::vector< VectorAnimationEventHandler* > mEventHandlers;
  std::vector< VectorAnimationEventHandler* > mTriggeredHandlers;
  Dali::Mutex                                      mMutex;
  std::unique_ptr< EventThreadCallback >           mEventTrigger;
};

} // namespace Plugin

} // namespace Dali;

#endif // DALI_TIZEN_VECTOR_ANIMATION_MANAGER_H

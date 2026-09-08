#ifndef DALI_VECTOR_ANIMATION_EVENT_HANDLER_H
#define DALI_VECTOR_ANIMATION_EVENT_HANDLER_H

/*
 * Copyright (c) 2026 Samsung Electronics Co., Ltd.
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

// INTERNAL INCLUDES
#include <dali/public-api/common/dali-namespace.h>

// EXTERNAL INCLUDES

namespace DALI_NAMESPACE
{

namespace Plugin
{

/**
 * @brief Tizen vector animation event handler
 */
class VectorAnimationEventHandler
{
public:
  /**
   * @brief Notify events
   */
  virtual void NotifyEvent() = 0;

protected:
  /**
   * constructor
   */
  VectorAnimationEventHandler()
  {
  }

  /**
   * virtual destructor
   */
  virtual ~VectorAnimationEventHandler()
  {
  }

  // Undefined copy constructor.
  VectorAnimationEventHandler(const VectorAnimationEventHandler&) = delete;

  // Undefined assignment operator.
  VectorAnimationEventHandler& operator=(const VectorAnimationEventHandler&) = delete;
};

} // namespace Plugin

} //namespace DALI_NAMESPACE

#endif // DALI_TIZEN_VECTOR_ANIMATION_EVENT_HANDLER_H

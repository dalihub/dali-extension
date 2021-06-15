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

// CLASS HEADER
#include <dali-extension/devel-api/rive-animation-view/rive-animation-view.h>

// EXTERNAL INCLUDES
//#include <dali/integration-api/debug.h>
//#include <dali/public-api/object/property-map.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/rive-animation-view/rive-animation-view-impl.h>

namespace Dali
{
namespace Extension
{
RiveAnimationView::RiveAnimationView()
{
}

RiveAnimationView::RiveAnimationView(const RiveAnimationView& riveAnimationView) = default;

RiveAnimationView::RiveAnimationView(RiveAnimationView&& rhs) = default;

RiveAnimationView& RiveAnimationView::operator=(const RiveAnimationView& riveAnimationView) = default;

RiveAnimationView& RiveAnimationView::operator=(RiveAnimationView&& rhs) = default;

RiveAnimationView::~RiveAnimationView()
{
}

RiveAnimationView RiveAnimationView::New()
{
  return Internal::RiveAnimationView::New();
}

RiveAnimationView RiveAnimationView::New(const std::string& url)
{
  RiveAnimationView riveAnimationView = Internal::RiveAnimationView::New();
  riveAnimationView[Extension::RiveAnimationView::Property::URL] = url;
  return riveAnimationView;
}

RiveAnimationView RiveAnimationView::DownCast(BaseHandle handle)
{
  return Control::DownCast<RiveAnimationView, Internal::RiveAnimationView>(handle);
}

RiveAnimationView::RiveAnimationView(Internal::RiveAnimationView& implementation)
: Control(implementation)
{
}

RiveAnimationView::RiveAnimationView(Dali::Internal::CustomActor* internal)
: Control(internal)
{
  VerifyCustomActorPointer<Internal::RiveAnimationView>(internal);
}

void RiveAnimationView::PlayAnimation()
{
  Extension::GetImplementation(*this).PlayAnimation();
}

void RiveAnimationView::StopAnimation()
{
  Extension::GetImplementation(*this).StopAnimation();
}

void RiveAnimationView::PauseAnimation()
{
  Extension::GetImplementation(*this).PauseAnimation();
}

RiveAnimationView::AnimationSignalType& RiveAnimationView::AnimationFinishedSignal()
{
  return Extension::GetImplementation(*this).AnimationFinishedSignal();
}

} // namespace Toolkit

} // namespace Dali

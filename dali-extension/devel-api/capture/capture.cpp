/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali-extension/devel-api/capture/capture.h>
#include <dali-extension/internal/capture/capture-impl.h>

namespace Dali
{

namespace Extension
{

Capture::Capture()
{
}

Capture Capture::New()
{
  Internal::CapturePtr internal = Internal::Capture::New();

  return Capture(internal.Get());
}

Capture Capture::New(Dali::Camera::ProjectionMode mode)
{
  Internal::CapturePtr internal = Internal::Capture::New(mode);

  return Capture(internal.Get());
}

Capture Capture::DownCast(BaseHandle handle)
{
  return Capture(dynamic_cast<Internal::Capture*>(handle.GetObjectPtr()));
}

Capture::~Capture()
{
}

Capture::Capture(const Capture& copy)
: BaseHandle(copy)
{
}

Capture& Capture::operator=(const Capture& rhs)
{
  BaseHandle::operator=(rhs);
  return *this;
}

void Capture::Start(Actor source, const Vector2& size, const std::string &path, const Vector4& clearColor )
{
  GetImpl(*this).Start(source, size, path, clearColor);
}

Capture::FinishState Capture::GetFinishState()
{
  return GetImpl(*this).GetFinishState();
}

Capture::CaptureSignalType& Capture::FinishedSignal()
{
  return GetImpl(*this).FinishedSignal();
}

Capture::Capture(Internal::Capture* internal)
: BaseHandle(internal)
{
}

} // namespace Extension

} // namespace Dali

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

// EXTERNAL INCLUDES
#include <dali/integration-api/adaptor-framework/media-frame-source-plugin.h>
#include <dali/integration-api/debug.h>
#include <media_frame_source_internal.h>
#include <tbm_surface.h>
#include <mutex>
#include <set>
#include <utility>

namespace
{
using namespace Dali::Integration;

std::mutex& GetSubscriptionMutex()
{
  static std::mutex mutex;
  return mutex;
}

std::set<media_frame_source_h>& GetSubscriptions()
{
  static std::set<media_frame_source_h> subscriptions;
  return subscriptions;
}

class MMFrameSource : public MediaFrameSourcePlugin
{
public:
  MMFrameSource() = default;

  ~MMFrameSource() override
  {
    Detach();
  }

  MMFrameSource(const MMFrameSource&)            = delete;
  MMFrameSource& operator=(const MMFrameSource&) = delete;

  bool Attach(Dali::Any source, MediaFrameConsumer& consumer) override
  {
    Detach();
    if(!source.IsType<media_frame_source_h>())
    {
      return false;
    }
    auto handle = source.Get<media_frame_source_h>();
    if(!handle)
    {
      return false;
    }
    {
      std::lock_guard<std::mutex> lock(GetSubscriptionMutex());
      if(!GetSubscriptions().insert(handle).second)
      {
        return false;
      }
    }
    mSource   = handle;
    mConsumer = &consumer;
    if(media_frame_source_set_event_cb(handle, EventCallback, this) != 0)
    {
      ForgetSubscription();
      return false;
    }
    if(media_frame_source_set_frame_cb(handle, FrameCallback, this) != 0)
    {
      media_frame_source_unset_event_cb(handle);
      ForgetSubscription();
      return false;
    }
    return true;
  }

  void Detach() override
  {
    if(mSource)
    {
      // These calls wait out callbacks; stale producer handles are safe.
      media_frame_source_unset_frame_cb(mSource);
      media_frame_source_unset_event_cb(mSource);
      ForgetSubscription();
    }
  }

private:
  void ForgetSubscription()
  {
    std::lock_guard<std::mutex> lock(GetSubscriptionMutex());
    GetSubscriptions().erase(mSource);
    mSource   = nullptr;
    mConsumer = nullptr;
  }

  static void Release(void* packet)
  {
    media_packet_unref(static_cast<media_packet_h>(packet));
  }

  static void FrameCallback(media_frame_source_h, media_packet_h packet, void* data)
  {
    auto&                        self    = *static_cast<MMFrameSource*>(data);
    tbm_surface_h                surface = nullptr;
    tbm_surface_info_s           info;
    media_packet_rotate_method_e rotation = MEDIA_PACKET_ROTATE_IDENTITY;
    if(media_packet_get_tbm_surface(packet, &surface) != 0 || !surface ||
       tbm_surface_get_info(surface, &info) != TBM_SURFACE_ERROR_NONE)
    {
      Release(packet);
      return;
    }
    media_packet_get_rotate_method(packet, &rotation);
    auto transform = MediaFrameTransform::IDENTITY;
    switch(rotation)
    {
      case MEDIA_PACKET_ROTATE_90:
      {
        transform = MediaFrameTransform::ROTATE_90;
        break;
      }
      case MEDIA_PACKET_ROTATE_180:
      {
        transform = MediaFrameTransform::ROTATE_180;
        break;
      }
      case MEDIA_PACKET_ROTATE_270:
      {
        transform = MediaFrameTransform::ROTATE_270;
        break;
      }
      case MEDIA_PACKET_ROTATE_HORIZONTAL_FLIP:
      {
        transform = MediaFrameTransform::HORIZONTAL_FLIP;
        break;
      }
      case MEDIA_PACKET_ROTATE_VERTICAL_FLIP:
      {
        transform = MediaFrameTransform::VERTICAL_FLIP;
        break;
      }
      default:
      {
        break;
      }
    }
    self.mConsumer->OnFrame(MediaFrame(Dali::Any(surface), Dali::Vector2(info.width, info.height), transform, Release, packet));
  }

  static void EventCallback(media_frame_source_h, media_frame_source_event_e event, bool keepScreen, void* data)
  {
    if(event == MEDIA_FRAME_SOURCE_EVENT_RETURN_BUFFER)
    {
      if(keepScreen)
      {
        DALI_LOG_ERROR("MediaView: keep_screen is unsupported; returning all frames\n");
      }
      static_cast<MMFrameSource*>(data)->mConsumer->OnReturnBuffer();
    }
  }

  media_frame_source_h mSource{nullptr};
  MediaFrameConsumer*  mConsumer{nullptr};
};
} // namespace

extern "C" DALI_EXPORT_API Dali::Integration::MediaFrameSourcePlugin* CreateMediaFrameSourcePlugin()
{
  return new MMFrameSource;
}

extern "C" DALI_EXPORT_API void DestroyMediaFrameSourcePlugin(Dali::Integration::MediaFrameSourcePlugin* plugin)
{
  delete plugin;
}

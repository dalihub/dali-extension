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
#include <es-video-player.h>
#include <tizen-video-player.h>

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/video-player-plugin.h>
#include <dali/integration-api/debug.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/object/any.h>
#include <esplusplayer_capi/esplusplayer_capi.h>
#include <esplusplayer_capi/esplusplayer_internal.h>
#include <cstring>
#include <vector>

namespace
{
constexpr const char* TIZEN_MMPLAYER_PROVIDER_ID = "tizen.mmplayer";
constexpr const char* TIZEN_ESPLAYER_PROVIDER_ID = "tizen.esplayer";

class VideoSourceProvider
{
public:
  virtual ~VideoSourceProvider() = default;

  virtual const char* GetProviderId() const = 0;
  virtual bool CanHandle(const Dali::VideoPlayerPlugin::VideoSourceDescriptor& source) const = 0;
  virtual Dali::VideoPlayerPlugin* Create(Dali::Actor actor, const Dali::VideoPlayerPlugin::VideoSourceDescriptor& source, Dali::VideoSyncMode syncMode) const = 0;
};

bool HasNativeSession(const Dali::VideoPlayerPlugin::VideoSourceDescriptor& source)
{
  return !source.GetNativeSession().Empty();
}

bool HasSupportedSourceHeader(const Dali::VideoPlayerPlugin::VideoSourceDescriptor& source)
{
  return source.GetVersion() == 1u && source.GetOwnership() == Dali::VideoPlayerPlugin::VideoSourceOwnership::EXTERNAL;
}

bool ProviderIdEquals(Dali::StringView lhs, Dali::StringView rhs)
{
  return lhs == rhs;
}

class TizenMMPlayerSourceProvider : public VideoSourceProvider
{
public:
  const char* GetProviderId() const override
  {
    return TIZEN_MMPLAYER_PROVIDER_ID;
  }

  bool CanHandle(const Dali::VideoPlayerPlugin::VideoSourceDescriptor& source) const override
  {
    return ProviderIdEquals(source.GetProviderId(), GetProviderId()) &&
           HasSupportedSourceHeader(source) &&
           HasNativeSession(source) &&
           (source.GetNativeSession().IsType<void*>() || source.GetNativeSession().IsType<player_h>());
  }

  Dali::VideoPlayerPlugin* Create(Dali::Actor actor, const Dali::VideoPlayerPlugin::VideoSourceDescriptor& source, Dali::VideoSyncMode syncMode) const override
  {
    return new Dali::Plugin::TizenVideoPlayer(source, syncMode, actor);
  }
};

class TizenESPlayerSourceProvider : public VideoSourceProvider
{
public:
  const char* GetProviderId() const override
  {
    return TIZEN_ESPLAYER_PROVIDER_ID;
  }

  bool CanHandle(const Dali::VideoPlayerPlugin::VideoSourceDescriptor& source) const override
  {
    return ProviderIdEquals(source.GetProviderId(), GetProviderId()) &&
           HasSupportedSourceHeader(source) &&
           HasNativeSession(source) &&
           (source.GetNativeSession().IsType<void*>() || source.GetNativeSession().IsType<esplusplayer_handle>());
  }

  Dali::VideoPlayerPlugin* Create(Dali::Actor actor, const Dali::VideoPlayerPlugin::VideoSourceDescriptor& source, Dali::VideoSyncMode syncMode) const override
  {
    return new Dali::Plugin::EsVideoPlayer(source, syncMode, actor);
  }
};

const std::vector<VideoSourceProvider*>& GetVideoSourceProviders()
{
  static TizenMMPlayerSourceProvider mmPlayerProvider;
  static TizenESPlayerSourceProvider esPlayerProvider;
  static std::vector<VideoSourceProvider*> providers = {
    &mmPlayerProvider,
    &esPlayerProvider,
  };
  return providers;
}

} // namespace

extern "C" DALI_EXPORT_API Dali::VideoPlayerPlugin* CreateVideoPlayerPlugin(Dali::Actor actor, Dali::VideoSyncMode syncMode)
{
  return new Dali::Plugin::TizenVideoPlayer(actor, syncMode);
}

extern "C" DALI_EXPORT_API Dali::VideoPlayerPlugin* CreateVideoPlayerPluginBySource(Dali::Actor actor, Dali::VideoPlayerPlugin::VideoSourceDescriptor source, Dali::VideoSyncMode syncMode)
{
  for(auto* provider : GetVideoSourceProviders())
  {
    if(provider != nullptr && provider->CanHandle(source))
    {
      return provider->Create(actor, source, syncMode);
    }
  }

  DALI_LOG_ERROR("CreateVideoPlayerPluginBySource: Unsupported providerId[%s], version[%u], ownership[%u], nativeSessionEmpty[%d]\n",
                 source.GetProviderId().CStr(),
                 source.GetVersion(),
                 static_cast<uint32_t>(source.GetOwnership()),
                 source.GetNativeSession().Empty());
  return nullptr;
}

extern "C" DALI_EXPORT_API void DestroyVideoPlayerPlugin(Dali::VideoPlayerPlugin* plugin)
{
  if(plugin != NULL)
  {
    delete plugin;
  }
}

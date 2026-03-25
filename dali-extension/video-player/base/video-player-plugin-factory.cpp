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
#include <tizen-video-player.h>
#include <es-video-player.h>

// EXTERNAL INCLUDES
#include <dali/public-api/object/any.h>
#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/integration-api/debug.h>
#include <dali/devel-api/adaptor-framework/video-player-plugin.h>
#include <dali-toolkit/devel-api/controls/video-view/video-view-devel.h>
#include <esplusplayer_capi/esplusplayer_capi.h>
#include <esplusplayer_capi/esplusplayer_internal.h>

extern "C" DALI_EXPORT_API Dali::VideoPlayerPlugin* CreateVideoPlayerPlugin(Dali::Actor actor, Dali::VideoSyncMode syncMode)
{
  return new Dali::Plugin::TizenVideoPlayer(actor, syncMode);
}

extern "C" DALI_EXPORT_API Dali::VideoPlayerPlugin* CreateVideoPlayerPluginByHandle(Dali::Actor actor, Dali::VideoPlayerPlugin::PlayerHandle playerHandle, Dali::VideoSyncMode syncMode)
{
  // Check playerType field to determine which player to create
  switch(playerHandle.playerType)
  {
    case Dali::VideoPlayerPlugin::PlayerHandleType::DEFAULT:
    {
      // Pass PlayerHandle for TizenVideoPlayer
      return new Dali::Plugin::TizenVideoPlayer(playerHandle, syncMode, actor);
    }

    case Dali::VideoPlayerPlugin::PlayerHandleType::EXTERNAL:
    {
      // Pass PlayerHandle for EsVideoPlayer
      return new Dali::Plugin::EsVideoPlayer(playerHandle, syncMode, actor);
    }

    default:
      DALI_LOG_ERROR("CreateVideoPlayerPluginByHandle: Unknown playerType %d\n",
                   static_cast<int>(playerHandle.playerType));
      return nullptr;
  }
  DALI_LOG_ERROR("CreateVideoPlayerPluginByHandle: Unsupported player handle type injected!\n");
  return nullptr;
}


extern "C" DALI_EXPORT_API void DestroyVideoPlayerPlugin(Dali::VideoPlayerPlugin* plugin)
{
  if(plugin != NULL)
  {
    delete plugin;
  }
}

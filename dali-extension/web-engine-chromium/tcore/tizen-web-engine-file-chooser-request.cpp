/*
 * Copyright (c) 2025 Samsung Electronics Co., Ltd.
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

#include "tizen-web-engine-file-chooser-request.h"

#include <glib.h>
namespace Dali
{
namespace Plugin
{

TizenWebEngineFileChooserRequest::TizenWebEngineFileChooserRequest(wv_file_chooser_request_h request)
: wvFileChooserRequest(request)
{
}

TizenWebEngineFileChooserRequest::~TizenWebEngineFileChooserRequest()
{
}

bool TizenWebEngineFileChooserRequest::MultipleFilesAllowed() const
{
  bool isAllowed = false;
  if(wvFileChooserRequest)
  {
    isAllowed = wv_file_chooser_request_allow_multiple_files_get(wvFileChooserRequest);
  }
  return isAllowed;
}

std::vector<std::string> TizenWebEngineFileChooserRequest::AcceptedMimetypes() const
{
  std::vector<std::string> mimetypes;
  if(wvFileChooserRequest)
  {
    GList* list = wv_file_chooser_request_accepted_mimetypes_get(wvFileChooserRequest);
    for(GList* it = list; it != nullptr; it = it->next)
    {
      if(it->data != nullptr)
      {
        std::string mimetype = static_cast<char*>(it->data);
        mimetypes.push_back(mimetype);
      }
    }
    g_list_free(list);
  }
  return mimetypes;
}

bool TizenWebEngineFileChooserRequest::Cancel()
{
  bool isCanceled = false;
  if(wvFileChooserRequest)
  {
    isCanceled = wv_file_chooser_request_cancel(wvFileChooserRequest);
  }
  return isCanceled;
}

bool TizenWebEngineFileChooserRequest::ChooseFiles(std::vector<std::string> files)
{
  GList* list = nullptr;
  for(std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
  {
    // Duplicated: WV does not document whether it copies the paths or keeps
    // the pointers.
    list = g_list_append(list, g_strdup(it->c_str()));
  }
  bool isChoosed = wv_file_chooser_request_files_choose(wvFileChooserRequest, list);
  g_list_free_full(list, g_free);
  return isChoosed;
}

bool TizenWebEngineFileChooserRequest::ChooseFile(std::string file)
{
  bool isChoosed = false;
  if(wvFileChooserRequest)
  {
    isChoosed = wv_file_chooser_request_file_choose(wvFileChooserRequest, file.c_str());
  }
  return isChoosed;
}

} // namespace Plugin
} // namespace Dali

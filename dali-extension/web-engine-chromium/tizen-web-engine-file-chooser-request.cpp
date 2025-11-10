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

namespace Dali
{
namespace Plugin
{

TizenWebEngineFileChooserRequest::TizenWebEngineFileChooserRequest(Ewk_File_Chooser_Request* request)
: ewkFileChooserRequest(request)
{
}

TizenWebEngineFileChooserRequest::~TizenWebEngineFileChooserRequest()
{
}

bool TizenWebEngineFileChooserRequest::MultipleFilesAllowed() const
{
  bool isAllowed = false;
  if(ewkFileChooserRequest)
  {
    isAllowed = ewk_file_chooser_request_allow_multiple_files_get(ewkFileChooserRequest);
  }
  return isAllowed;
}

std::vector<std::string> TizenWebEngineFileChooserRequest::AcceptedMimetypes() const
{
  std::vector<std::string> mimetypes;
  if(ewkFileChooserRequest)
  {
    Eina_List* list = ewk_file_chooser_request_accepted_mimetypes_get(ewkFileChooserRequest);
    Eina_List* it;
    void*      data = nullptr;
    EINA_LIST_FOREACH(list, it, data)
    {
      if(data != nullptr)
      {
        std::string mimetype = (char*)data;
        mimetypes.push_back(mimetype);
      }
    }
    eina_list_free(list);
  }
  return mimetypes;
}

bool TizenWebEngineFileChooserRequest::Cancel()
{
  bool isCanceled = false;
  if(ewkFileChooserRequest)
  {
    isCanceled = ewk_file_chooser_request_cancel(ewkFileChooserRequest);
  }
  return isCanceled;
}

bool TizenWebEngineFileChooserRequest::ChooseFiles(std::vector<std::string> files)
{
  Eina_List* list = nullptr;
  for(std::vector<std::string>::const_iterator it = files.begin(); it != files.end(); ++it)
  {
    list = eina_list_append(list, (*it).c_str());
  }
  bool isChoosed = ewk_file_chooser_request_files_choose(ewkFileChooserRequest, list);
  eina_list_free(list);
  return isChoosed;
}

bool TizenWebEngineFileChooserRequest::ChooseFile(std::string file)
{
  bool isChoosed = false;
  if(ewkFileChooserRequest)
  {
    isChoosed = ewk_file_chooser_request_file_choose(ewkFileChooserRequest, file.c_str());
  }
  return isChoosed;
}

} // namespace Plugin
} // namespace Dali

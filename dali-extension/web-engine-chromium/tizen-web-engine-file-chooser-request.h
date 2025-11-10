#ifndef DALI_PLUGIN_TIZEN_WEB_ENGINE_FILE_CHOOSER_REQUEST_H
#define DALI_PLUGIN_TIZEN_WEB_ENGINE_FILE_CHOOSER_REQUEST_H

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

// EXTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/web-engine/web-engine-file-chooser-request.h>
#include <ewk_file_chooser_request_product.h>

namespace Dali
{
namespace Plugin
{
/**
 * @brief A class for file chooser request.
 */
class TizenWebEngineFileChooserRequest : public Dali::WebEngineFileChooserRequest
{
public:
  /**
   * @brief Constructor.
   */
  TizenWebEngineFileChooserRequest(Ewk_File_Chooser_Request*);

  /**
   * @brief Destructor.
   */
  ~TizenWebEngineFileChooserRequest();

  /**
   * @copydoc Dali::WebEngineFileChooserRequest::MultipleFilesAllowed()
   */
  bool MultipleFilesAllowed() const override;

  /**
   * @copydoc Dali::WebEngineFileChooserRequest::AcceptedMimetypes()
   */
  std::vector<std::string> AcceptedMimetypes() const override;

  /**
   * @copydoc Dali::WebEngineFileChooserRequest::Cancel()
   */
  bool Cancel() override;

  /**
   * @copydoc Dali::WebEngineFileChooserRequest::ChooseFiles()
   */
  bool ChooseFiles(const std::vector<std::string> files) override;

  /**
   * @copydoc Dali::WebEngineFileChooserRequest::ChooseFile()
   */
  bool ChooseFile(const std::string file) override;

private:
  Ewk_File_Chooser_Request* ewkFileChooserRequest;
};

} // namespace Plugin
} // namespace Dali

#endif // DALI_PLUGIN_TIZEN_WEB_ENGINE_FILE_CHOOSER_REQUEST_H

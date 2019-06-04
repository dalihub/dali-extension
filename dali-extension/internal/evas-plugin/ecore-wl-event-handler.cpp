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
#include "ecore-wl-event-handler.h"

namespace Dali
{

namespace Extension
{

namespace Internal
{

namespace
{

const int ECORE_WL_EVENT_COUNT = 4;

//const char* CLIPBOARD_ATOM = "CBHM_MSG";
//const char* CLIPBOARD_SET_OWNER_MESSAGE = "SET_OWNER";

}

struct EcoreWlEventHandler::Impl
{
  Impl(EcoreWlEventHandler* eventHandler, Ecore_Wl2_Window* window)
  : mEventHandler(eventHandler)
  , mWindow(window)
  , mEcoreEventHandlers(ECORE_WL_EVENT_COUNT)
  {
#if 0
    // Register Client message events - accessibility etc
    mEcoreEventHandlers.push_back(ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE,  EcoreWlEventClientMessage, eventHandler));

    // Register Selection events - clipboard selection, Drag & Drop selection etc
    mEcoreEventHandlers.push_back(ecore_event_handler_add(ECORE_X_EVENT_SELECTION_CLEAR, EcoreWlEventSelectionClear, eventHandler));
    mEcoreEventHandlers.push_back(ecore_event_handler_add(ECORE_X_EVENT_SELECTION_NOTIFY, EcoreWlEventSelectionNotify, eventHandler));
#endif
    // Register Window visibility change events
    mEcoreEventHandlers.push_back(ecore_event_handler_add(ECORE_WL2_EVENT_WINDOW_VISIBILITY_CHANGE, EcoreWlEventWindowVisibilityChange, eventHandler));

  }

  ~Impl()
  {
    for(std::vector<Ecore_Event_Handler*>::iterator iter = mEcoreEventHandlers.begin(), endIter = mEcoreEventHandlers.end(); iter != endIter; ++iter)
    {
      ecore_event_handler_del(*iter);
    }
  }

  static Eina_Bool EcoreWlEventClientMessage(void* data, int type, void* event)
  {
#if 0
    Ecore_X_Event_Client_Message* eventClientMessage = static_cast<Ecore_X_Event_Client_Message*>(event);
    EcoreWlEventHandler* eventHandler = static_cast<EcoreWlEventHandler*>(data);

    if (eventClientMessage->win == eventHandler->mImpl->mWindow)
    {
      if (eventClientMessage->message_type == ecore_x_atom_get(CLIPBOARD_ATOM))
      {
        if (!strcmp(eventClientMessage->data.b, CLIPBOARD_SET_OWNER_MESSAGE))
        {
          // Claim the ownership of the SECONDARY selection
          ecore_x_selection_secondary_set(eventClientMessage->win, "", 1);

          // Show the clipboard window
          Clipboard::Get().ShowClipboard();
        }
      }
    }
#endif

    return ECORE_CALLBACK_PASS_ON;
  }

  static Eina_Bool EcoreWlEventSelectionClear(void* data, int type, void* event)
  {
#if 0
    Ecore_X_Event_Selection_Clear* eventSelectionClear = static_cast<Ecore_X_Event_Selection_Clear*>(event);
    EcoreWlEventHandler* eventHandler = static_cast<EcoreWlEventHandler*>(data);

    if (eventSelectionClear->win == eventHandler->mImpl->mWindow)
    {
      if (eventSelectionClear->selection == ECORE_X_SELECTION_SECONDARY)
      {
        // Request to get the content from Ecore
        ecore_x_selection_secondary_request(eventSelectionClear->win, ECORE_X_SELECTION_TARGET_TEXT);
      }
    }
#endif

    return ECORE_CALLBACK_PASS_ON;
  }

  static Eina_Bool EcoreWlEventSelectionNotify(void* data, int type, void* event)
  {
#if 0
    Ecore_X_Event_Selection_Notify* eventSelectionNotify = static_cast<Ecore_X_Event_Selection_Notify*>(event);
    EcoreWlEventHandler* eventHandler = static_cast<EcoreWlEventHandler*>(data);

    if (eventSelectionNotify->win == eventHandler->mImpl->mWindow)
    {
      Ecore_X_Selection_Data* selectionData = static_cast<Ecore_X_Selection_Data*>(eventSelectionNotify->data);
      if (selectionData->data)
      {
        if (eventSelectionNotify->selection == ECORE_X_SELECTION_SECONDARY)
        {
          std::string content(reinterpret_cast<char*>(selectionData->data), selectionData->length);

          ClipboardEventNotifier clipboardEventNotifier = ClipboardEventNotifier::Get();
          clipboardEventNotifier.SetContent(content);
          clipboardEventNotifier.EmitContentSelectedSignal();

          // Claim the ownership of the SECONDARY selection
          ecore_x_selection_secondary_set(eventSelectionNotify->win, "", 1);

          DALI_LOG_INFO( gSelectionEventLogFilter, Debug::General, "EcoreWlEventSelectionNotify: Content(%d):\n" , selectionData->length );
          DALI_LOG_INFO( gSelectionEventLogFilter, Debug::General, "======================================\n" );
          DALI_LOG_INFO( gSelectionEventLogFilter, Debug::General, "%s\n", selectionData->data );
          DALI_LOG_INFO( gSelectionEventLogFilter, Debug::General, "======================================\n" );
        }
      }
    }
#endif

    return ECORE_CALLBACK_PASS_ON;
  }

  static Eina_Bool EcoreWlEventWindowVisibilityChange(void* data, int type, void* event)
  {
    Ecore_Wl2_Event_Window_Visibility_Change* eventWindowVisibilityChange = static_cast<Ecore_Wl2_Event_Window_Visibility_Change*>(event);

    EcoreWlEventHandler* eventHandler = static_cast<EcoreWlEventHandler*>(data);

    // 0 is visible and 1 is invisible
    eventHandler->SendEcoreWlVisibility(!eventWindowVisibilityChange->fully_obscured);

    return ECORE_CALLBACK_PASS_ON;
  }

  EcoreWlEventHandler* mEventHandler;
  Ecore_Wl2_Window* mWindow;
  std::vector<Ecore_Event_Handler*> mEcoreEventHandlers;
};

EcoreWlEventHandler::EcoreWlEventHandler(Ecore_Wl2_Window* window,
                                         EvasPluginVisibilityInterface& evasPluginVisibilityInterface)
: mEvasPluginVisibilityInterface(evasPluginVisibilityInterface)
, mImpl(NULL)
{
  mImpl = new Impl(this, window);
}

EcoreWlEventHandler::~EcoreWlEventHandler()
{
  delete mImpl;
  mImpl = NULL;
}

void EcoreWlEventHandler::SendEcoreWlVisibility(bool visibility)
{
  mEvasPluginVisibilityInterface.OnEcoreWlVisibility(visibility);
}

}  // namespace Internal

}  // namespace Extension

}  // namespace Dali

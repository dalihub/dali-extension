#ifndef __DALI_EXTENSION_EVAS_PLUGIN_H__
#define __DALI_EXTENSION_EVAS_PLUGIN_H__

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

/**
 * @addtogroup CAPI_DALI_EXTENSION_FRAMEWORK_MODULE
 * @{
 */

// EXTERNAL INCLUDES
#include <Evas.h>

#include <dali/public-api/signals/dali-signal.h>
#include <dali/integration-api/adaptor-framework/scene-holder.h>

namespace Dali
{

namespace Extension
{

namespace Internal
{
class EvasPlugin;
}

/**
 * @brief Application class used by EFL applications that wish to use Dali
 *
 * An EvasPlugin class object should be created by EFL applications
 * that wish to use Dali. It provides a mean for initializing the
 * resources required by the Dali::Core.
 *
 * The EvasPlugin class emits several signals which the user can
 * connect to.  The user should not create any Dali objects in the main
 * function and instead should connect to the Init signal of the
 * EvasPlugin and create the Dali objects in the connected callback.
 *
 * Tizen EFL applications should follow the example below:
 *
 * @code
 *
 * #include <app.h>
 * #include <Elementary.h>
 * #include <dali/dali.h>
 * #include <dali-toolkit/dali-toolkit.h>
 * #include <dali-extension/dali-extension.h>
 *
 * using namespace Dali;
 * using namespace Dali::Toolkit;
 * using namespace Dali::Extension;
 *
 * namespace
 * {
 * const char* const APPLICATION_TITLE = "EvasPluginExample";
 * const int EVAS_PLUGIN_WIDTH = 360;
 * const int EVAS_PLUGIN_HEIGHT = 360;
 * }
 *
 * class EvasPluginExample : public ConnectionTracker
 * {
 * public:
 *   EvasPluginExample(EvasPlugin evasPlugin)
 *   : mEvasPlugin(evasPlugin)
 *   {
 *     mEvasPlugin.InitSignal().Connect(this, &EvasPluginExample::OnInitialize);
 *     mEvasPlugin.Run();
 *   }
 *
 *   ~EvasPluginExample()
 *   {
 *     mEvasPlugin.Stop();
 *   }
 *
 *   void OnInitialize()
 *   {
 *     Stage stage = Stage::GetCurrent();
 *     stage.SetBackgroundColor( Color::WHITE );
 *
 *     TextLabel textLabel = TextLabel::New( "Hello World" );
 *     textLabel.SetParentOrigin( ParentOrigin::CENTER );
 *     textLabel.SetAnchorPoint( AnchorPoint::CENTER );
 *     textLabel.SetProperty( TextLabel::Property::HORIZONTAL_ALIGNMENT, HorizontalAlignment::CENTER );
 *     textLabel.SetProperty( TextLabel::Property::POINT_SIZE, 40 );
 *     textLabel.SetName( "helloWorldLabel" );
 *     stage.Add( textLabel );
 *
 *     Animation anim = Animation::New( 3.f );
 *     anim.SetLooping( true );
 *     anim.AnimateBy( Property(textLabel, Actor::Property::ORIENTATION), Quaternion(Degree(0.f), Degree(360.f), Degree(0.f)) );
 *     anim.Play();
 *   }
 *
 * private:
 *   EvasPlugin mEvasPlugin;
 * };
 *
 * struct app_data
 * {
 *   Evas_Object* elm_win;
 *   EvasPluginExample* evasPluginExample;
 * };
 *
 * static void win_del(void *data, Evas_Object * obj, void *event_info)
 * {
 * }
 *
 * static bool app_create(void *data)
 * {
 *   struct app_data* ad = (struct app_data*)data;
 *
 *   // Sets the GL backend for rendering
 *   elm_config_accel_preference_set("3d");
 *
 *   // Creates the elm window
 *   ad->elm_win = elm_win_add(NULL, APPLICATION_TITLE, ELM_WIN_BASIC);
 *
 *   DALI_ASSERT_ALWAYS(ad->elm_win != NULL && "Fail to create elm window.");
 *
 *   elm_win_title_set(ad->elm_win, APPLICATION_TITLE);
 *   elm_win_rotation_with_resize_set(ad->elm_win, 0);
 *   evas_object_smart_callback_add(ad->elm_win, "delete,request", win_del, ad->elm_win);
 *   evas_object_show(ad->elm_win);
 *
 *   // Adds the background
 *   Evas_Object* bg = elm_bg_add(ad->elm_win);
 *   elm_bg_color_set(bg, 255, 255, 255);
 *   evas_object_size_hint_weight_set(bg, 0,0);
 *   elm_win_resize_object_add(ad->elm_win, bg);
 *   evas_object_show(bg);
 *
 *   // Creates an Evas plugin
 *   EvasPlugin evasPlugin = EvasPlugin::New(ad->elm_win, EVAS_PLUGIN_WIDTH, EVAS_PLUGIN_HEIGHT, true);
 *
 *   // Creates a Dali application
 *   ad->evasPluginExample = new EvasPluginExample(evasPlugin);
 *
 *   return true;
 * }
 *
 * static void app_terminate(void *data)
 * {
 *   struct app_data* ad = (struct app_data*)data;
 *
 *   delete ad->evasPluginExample;
 *   ad->evasPluginExample = NULL;
 * }
 *
 * static void app_pause(void *data)
 * {
 * }
 *
 * static void app_resume(void *data)
 * {
 * }
 *
 * static void app_control(app_control_h service, void *data)
 * {
 * }
 *
 * int main(int argc, char *argv[])
 * {
 *   // Initializes the Tizen application framework
 *   ui_app_lifecycle_callback_s event_callback;
 *
 *   event_callback.create = app_create;
 *   event_callback.terminate = app_terminate;
 *   event_callback.pause = app_pause;
 *   event_callback.resume = app_resume;
 *   event_callback.app_control = app_control;
 *
 *   struct app_data ad;
 *   memset(&ad, 0x0, sizeof(ad));
 *
 *   // Runs the Tizen application framework
 *   return ui_app_main(argc, argv, &event_callback, &ad);
 * }
 *
 * @endcode
 */
class DALI_IMPORT_API EvasPlugin : public Dali::Integration::SceneHolder
{
public:

  typedef Signal<void (void)> EvasPluginSignalType;

public:

  /**
   * @brief This is the constructor for Tizen EFL applications
   *
   * @param[in] parentEvasObject Parent of the new Evas object
   * @param[in] width The initial width of the Dali view port
   * @param[in] height The initial height of the Dali view port
   * @param[in] isTranslucent Whether the Evas object is translucent or not
   */
  static EvasPlugin New( Evas_Object* parentEvasObject, int width, int height, bool isTranslucent );

  /**
   * @brief Constructs an empty handle
   */
  EvasPlugin();

  /**
   * @brief Copy constructor
   */
  EvasPlugin( const EvasPlugin& evasPlugin );

  /**
   * @brief Assignment operator
   */
  EvasPlugin& operator=( const EvasPlugin& evasPlugin );

  /**
   * @brief Destructor
   */
  ~EvasPlugin();

public:

  /**
   * @brief Runs the Evas plugin (rendering, event handling, etc)
   */
  void Run();

  /**
   * @brief Pauses the Evas plugin
   */
  void Pause();

  /**
   * @brief Resumes the Evas plugin
   */
  void Resume();

  /**
   * @brief Stops the Evas plugin
   */
  void Stop();

  /**
   * @brief This returns the Evas_Object* which is created internally
   *
   * Applications should append this access object to custom focus chain for accessibility
   *
   * e.g., elm_object_focus_custom_chain_append(layout, dali_access_object, NULL);
   *
   * @return Evas_Object* Elm access object which Dali image Evas object is registered
   */
  Evas_Object* GetAccessEvasObject();

  /**
   * @brief This returns the Evas_Object* which is created internally
   *
   * @return Evas_Object* Evas object which is rendered by Dali
   */
  Evas_Object* GetDaliEvasObject();

public:  // Signals

  /**
   * @brief Signal to notify the client when the application is ready to be initialized
   *
   * @return The signal
   */
  EvasPluginSignalType& InitSignal();

  /**
   * @brief Signal to notify the user when the application is about to be terminated
   *
   * @return The signal
   */
  EvasPluginSignalType& TerminateSignal();

  /**
   * @brief Signal to notify the client when the EvasPlugin is about to be paused
   *
   * The user should connect to this signal if the user needs to perform any special
   * activities when the application is about to be paused.
   * @return The signal
   */
  EvasPluginSignalType& PauseSignal();

  /**
   * @brief Signal to notify the client when the adpator has resumed
   *
   * The user should connect to this signal if the user needs to perform any special
   * activities when the application has resumed.
   * @return The signal
   */
  EvasPluginSignalType& ResumeSignal();

  /**
   * @brief Signal to notify the client when the Evas object is resized
   *
   * @return The signal
   */
  EvasPluginSignalType& ResizeSignal();

  /**
   * @brief Signal to notify the client when the Evas object gets the keyboard focus
   *
   * @return The signal
   */
  EvasPluginSignalType& FocusedSignal();

  /**
   * @brief Signal to notify the client when the Evas object loses the keyboard focus
   *
   * @return The signal
   */
  EvasPluginSignalType& UnFocusedSignal();

public: // Not intended for application developers
  /**
   * @brief Internal constructor
   */
  explicit DALI_INTERNAL EvasPlugin( Internal::EvasPlugin* evasPlugin );

};

/**
 * @}
 */

}  // namespace Extension

}  // namespace Dali

#endif // __DALI_EXTENSION_EVAS_PLUGIN_H__

/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
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

// EXTENRAL INCLUDES
#include <dali/public-api/actors/layer.h>

// INTERNAL INCLUDES
#include <dali-extension/internal/evas-plugin/evas-plugin-impl.h>
#include <dali-extension/internal/evas-plugin/scene-impl.h>

// CLASS HEADER
#include <dali-extension/devel-api/evas-plugin/scene.h>

namespace Dali
{

namespace Extension
{

Scene Scene::New( EvasPlugin evasPlugin, Evas_Object* parentEvasObject, uint16_t width, uint16_t height, bool isTranslucent )
{
  IntrusivePtr< Internal::Scene > impl = Internal::Scene::New( parentEvasObject, width, height, isTranslucent );

  Scene scene = Scene( impl.Get() );

  impl->Initialize( &Internal::GetImplementation( evasPlugin ), false );

  return scene;
}

Scene::Scene()
{
}

Scene::Scene( const Scene& scene )
: BaseHandle( scene )
{
}

Scene& Scene::operator=( const Scene& scene )
{
  if( *this != scene )
  {
    BaseHandle::operator=( scene );
  }
  return *this;
}

Scene::~Scene()
{
}

void Scene::Add( Actor actor )
{
  Internal::GetImplementation( *this ).Add( actor );
}

void Scene::Remove( Actor actor )
{
  Internal::GetImplementation( *this ).Remove( actor );
}

void Scene::SetBackgroundColor( const Vector4& color )
{
  Internal::GetImplementation( *this ).SetBackgroundColor( color );
}

Vector4 Scene::GetBackgroundColor() const
{
  return Internal::GetImplementation( *this ).GetBackgroundColor();
}

Layer Scene::GetRootLayer() const
{
  return Internal::GetImplementation( *this ).GetRootLayer();
}

uint32_t Scene::GetLayerCount() const
{
  return Internal::GetImplementation( *this ).GetLayerCount();
}

Layer Scene::GetLayer( uint32_t depth ) const
{
  return Internal::GetImplementation( *this ).GetLayer( depth );
}

Scene::SceneSize Scene::GetSize() const
{
  return Internal::GetImplementation( *this ).GetSize();
}

Evas_Object* Scene::GetAccessEvasObject()
{
  return Internal::GetImplementation( *this ).GetAccessEvasObject();
}

Evas_Object* Scene::GetDaliEvasObject()
{
  return Internal::GetImplementation( *this ).GetDaliEvasObject();
}

Scene::ResizedSignalType& Scene::ResizedSignal()
{
  return Internal::GetImplementation( *this ).ResizedSignal();
}

Scene::VisibilityChangedSignalType& Scene::VisibilityChangedSignal()
{
  return Internal::GetImplementation( *this ).VisibilityChangedSignal();
}

Scene::FocusChangedSignalType& Scene::FocusChangedSignal()
{
  return Internal::GetImplementation( *this ).FocusChangedSignal();
}

Scene::Scene( Internal::Scene* scene )
: BaseHandle( scene )
{
}

}  // namespace Extension

}  // namespace Dali

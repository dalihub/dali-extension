cmake_minimum_required(VERSION 3.16)
project(web-engine-profile-tests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
enable_testing()
set(source_root ${CMAKE_CURRENT_SOURCE_DIR}/../../dali-extension)
# Copy production sources unchanged so quoted includes resolve to test doubles.
# Only native SDK, wrapper and DALi lifecycle dependencies are mocked.
foreach(backend ecore tcore)
  set(test_dir ${CMAKE_CURRENT_BINARY_DIR}/${backend})
  set(manager_dir ${test_dir}/web-engine-chromium/${backend})
  foreach(file tizen-web-engine-manager.cpp tizen-web-engine-manager.h)
    configure_file(${source_root}/web-engine-chromium/${backend}/${file} ${manager_dir}/${file} COPYONLY)
  endforeach()
  configure_file(${source_root}/web-engine-common/web-engine-arguments.h
    ${test_dir}/web-engine-common/web-engine-arguments.h COPYONLY)
  foreach(header
      tizen-web-engine-context.h tizen-web-engine-cookie-manager.h tizen-web-engine-chromium.h
      Ecore_Evas.h Elementary.h ewk_context.h ewk_context_internal.h ewk_main.h ewk_main_internal.h
      wv_context.h wv_context_internal.h wv_view.h wv_main.h wv_main_internal.h
      dali/devel-api/adaptor-framework/web-engine/web-engine-profile.h
      dali/devel-api/adaptor-framework/lifecycle-controller.h
      dali/public-api/signals/slot-delegate.h dali/integration-api/debug.h)
    file(WRITE ${manager_dir}/${header} "#include \"test-dependencies.h\"\n")
  endforeach()
  add_executable(profiles-${backend} profiles-test.cpp)
  target_include_directories(profiles-${backend} PRIVATE ${manager_dir} ${CMAKE_CURRENT_SOURCE_DIR})
  target_compile_definitions(profiles-${backend} PRIVATE DALI_NAMESPACE=Dali DALI_EXPORT_API=)
  target_compile_options(profiles-${backend} PRIVATE -UNDEBUG -Wall -Wextra -Werror)
  add_test(NAME profiles-${backend} COMMAND profiles-${backend})
endforeach()

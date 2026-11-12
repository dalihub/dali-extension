# Chromium profile tracking regression tests

Build and run without a native Chromium SDK:

```sh
cmake -S automated-tests/web-engine-profiles -B build/profile-tests
cmake --build build/profile-tests -j6
ctest --test-dir build/profile-tests --output-on-failure
```

Each executable compiles an unchanged copy of the production ECORE or TCORE
manager source and header. CMake tracks these inputs and refreshes the copies
when they change. Native APIs, context/cookie wrappers, view and lifecycle
interfaces are test doubles. These tests do not verify native engine execution
or the ABI of those dependencies; use the normal target build for that.

The tests cover 1,000 default-only creation/destruction cycles, a live named
profile retained across default and named creation cycles, bounded weak registry
size, immediate destruction after the last strong owner, borrowed default
contexts, and view/context/engine shutdown order with profiles surviving shutdown.
Assertions remain enabled in Release builds. Registry inspection is test-only;
no product accessor or reset API is added.

# Cookie watch lifetime tests

Build with CMake and run CTest. `ADAPTOR_SOURCE` defaults to the sibling
`dali-adaptor` checkout. Both targets compile the production ECORE/TCORE cookie
manager sources against mock native APIs and the real adaptor interface.

The tests cover replacement without native re-registration, clearing, late
notifications after clearing/destruction/re-registration, profile isolation,
and concurrent callback execution during manager destruction.

The WV mock puts user data in the first argument, matching the native
dispatch in chromium-efl `dad454b88c64`. It tests that the DALi callback
ignores the second argument even when it contains another profile's live
watch token.

## Verification on supported TCORE engines

Repeat on each supported TCORE engine build before merging. Record the
device model, firmware version, and deployed WV engine package version.

1. In a fresh WebView profile, load a page from a test origin. Register a
   cookie change callback and have it increment a thread-safe counter.
2. Set a new cookie for that origin and verify that the callback is invoked.
   Wait for that notification before continuing.
3. Unregister by passing an empty callback. Set a different cookie and
   verify that the cookie is present and no callback is delivered.
4. Register a new callback and set a third cookie. Verify that only the
   new callback is invoked.

The local mock results cover DALi dispatch and lifetime behavior. Record
the on-device results separately for each engine build.

#### Notes & TODOs 

- Better helpers and handling for modules which may be either SHARED or STATIC.

- Proposed solution is to define SHARED_LIBRARY and STATIC_LIBRARY separately, then set
  LIBRARY to whichever one exists, and if both are defined then create an option so the
  user can choose.  Alternatively, we could detect which are available, default to SHARED
  if both are, and expose an Xxx_IMPORT_TYPE cache variable that the user could override.
  This would also let us throw errors if the files for the desired type are unavailable.

- Make a cmake module for installing executables/resources in a platform-agnostic way
  (e.g. Mac has a particular "bundle" format it follows).

- The organization of the Components (with its component and library dependencies) should
  be implemented using a cmake module, so little redundant boilerplate is necessary.

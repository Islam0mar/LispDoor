#ifndef LISPDOOR_CONFIG_H
#define LISPDOOR_CONFIG_H

#define LISP_DOOR_VERSION_MAJOR 0
#define LISP_DOOR_VERSION_MINOR 1
#define LISP_DOOR_VERSION_PATCH 1

// From
// https://stackoverflow.com/questions/5459868/concatenate-int-to-string-using-c-preprocessor
#define __LISP_DOOR_STR_HELPER(x) #x
#define __LISP_DOOR_STR(x) __LISP_DOOR_STR_HELPER(x)
#define LISP_DOOR_VERSION_STRING                                    \
  __LISP_DOOR_STR(LISP_DOOR_VERSION_MAJOR)                          \
  "." __LISP_DOOR_STR(LISP_DOOR_VERSION_MINOR) "." __LISP_DOOR_STR( \
      LISP_DOOR_VERSION_PATCH)

#endif /* LISPDOOR_CONFIG_H */

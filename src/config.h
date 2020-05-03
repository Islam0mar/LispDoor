/**
 *   \file config.h
 *   \brief A Documented file.
 *
 *  Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 */
#ifndef SRC_CONFIG_H_INCLUDED
#define SRC_CONFIG_H_INCLUDED

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

// From
// https://stackoverflow.com/questions/5459868/concatenate-int-to-string-using-c-preprocessor
#define __STR_HELPER(x) #x
#define __STR(x) __STR_HELPER(x)
#define VERSION_STRING \
  __STR(VERSION_MAJOR) \
  "." __STR(VERSION_MINOR) "." __STR(VERSION_PATCH)

#endif /* SRC_CONFIG_H_INCLUDED */

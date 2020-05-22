/*
 *    \file config.h
 *
 * Copyright (c) 2020 Islam Omar (io1131@fayoum.edu.eg)
 *
 * This file is part of LispDoor.
 *
 *     LispDoor is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     LispDoor is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with LispDoor.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *  Copyright (c) 2015: Bart Janssens.
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included
 *   in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 *   INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 *   PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *   LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *   THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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

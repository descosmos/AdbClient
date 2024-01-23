#ifndef PROTOCOL_H_
#define PROTOCOL_H_
// Reference: https://cs.android.com/android/platform/superproject/main/+/main:packages/modules/adb/file_sync_protocol.h

/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdint.h>

#define MKID(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

#define ID_LSTAT_V1 MKID('S', 'T', 'A', 'T')
#define ID_STAT_V2 MKID('S', 'T', 'A', '2')
#define ID_LSTAT_V2 MKID('L', 'S', 'T', '2')

#define ID_LIST_V1 MKID('L', 'I', 'S', 'T')
#define ID_LIST_V2 MKID('L', 'I', 'S', '2')
#define ID_DENT_V1 MKID('D', 'E', 'N', 'T')
#define ID_DENT_V2 MKID('D', 'N', 'T', '2')

#define ID_SEND_V1 MKID('S', 'E', 'N', 'D')
#define ID_SEND_V2 MKID('S', 'N', 'D', '2')
#define ID_RECV_V1 MKID('R', 'E', 'C', 'V')
#define ID_RECV_V2 MKID('R', 'C', 'V', '2')
#define ID_DONE MKID('D', 'O', 'N', 'E')
#define ID_DATA MKID('D', 'A', 'T', 'A')
#define ID_OKAY MKID('O', 'K', 'A', 'Y')
#define ID_FAIL MKID('F', 'A', 'I', 'L')
#define ID_QUIT MKID('Q', 'U', 'I', 'T')

#define SYNC_DATA_MAX (64 * 1024)

#endif  // PROTOCOL_H_
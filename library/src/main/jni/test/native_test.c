/*
 * Copyright 2018 Hippo Seven
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>

#include "com_hippo_image_NativeTest.h"
#include "test_utils.h"

JNIEXPORT jint JNICALL Java_com_hippo_image_NativeTest_nativeTestNative(
    JNIEnv* env,
    jclass __unused clazz,
    jstring temp_dir,
    jstring log_file
) {
  Suite* suite;
  SRunner* runner;
  const char *c_temp_dir;
  const char *c_log_file;
  int number_failed;

  c_temp_dir = (*env)->GetStringUTFChars(env, temp_dir, NULL);
  c_log_file = (*env)->GetStringUTFChars(env, log_file, NULL);

  // Check use system temporary directory to cache files.
  // But on Android, the app don't have permissions to
  // create file in system temporary directory.
  // So change the environmental variable to app-level temporary directory.
  setenv("TEMP", c_temp_dir, 1);

  suite = suite_create("Native");
  suite_add_tcase(suite, utils_case());

  runner = srunner_create(suite);
  srunner_set_xml(runner, c_log_file);
  srunner_run_all(runner, CK_SILENT);
  number_failed = srunner_ntests_failed(runner);
  srunner_free(runner);

  (*env)->ReleaseStringUTFChars(env, temp_dir, c_temp_dir);
  (*env)->ReleaseStringUTFChars(env, log_file, c_log_file);

  return number_failed;
}

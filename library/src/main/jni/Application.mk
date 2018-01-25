# Copyright 2015 Hippo Seven
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

APP_ABI := arm64-v8a x86_64 armeabi-v7a x86
APP_PLATFORM := android-9
APP_OPTIM := release

APP_CFLAGS += -finline-functions -ffast-math -ffunction-sections -fdata-sections -fvisibility=hidden
APP_LDFLAGS += -Wl,--gc-sections

NDK_TOOLCHAIN_VERSION := clang

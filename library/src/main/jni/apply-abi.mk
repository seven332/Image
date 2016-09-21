# Copyright 2016 Hippo Seven
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

LOCAL_SRC_FILES        += $(LOCAL_SRC_FILES_$(TARGET_ARCH_ABI))
LOCAL_C_INCLUDES       += $(LOCAL_C_INCLUDES_$(TARGET_ARCH_ABI))
LOCAL_CFLAGS           += $(LOCAL_CFLAGS_$(TARGET_ARCH_ABI))
LOCAL_CPPFLAGS         += $(LOCAL_CPPFLAGS_$(TARGET_ARCH_ABI))
LOCAL_ASMFLAGS         += $(LOCAL_ASMFLAGS_$(TARGET_ARCH_ABI))
LOCAL_LDFLAGS          += $(LOCAL_LDFLAGS_$(TARGET_ARCH_ABI))
LOCAL_LDLIBS           += $(LOCAL_LDLIBS_$(TARGET_ARCH_ABI))
LOCAL_SHARED_LIBRARIES += $(LOCAL_SHARED_LIBRARIES_$(TARGET_ARCH_ABI))
LOCAL_STATIC_LIBRARIES += $(LOCAL_STATIC_LIBRARIES_$(TARGET_ARCH_ABI))

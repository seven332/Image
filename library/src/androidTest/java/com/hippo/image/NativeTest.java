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

package com.hippo.image;

import android.support.test.InstrumentationRegistry;
import java.io.File;
import java.io.IOException;
import org.jsoup.Jsoup;
import org.jsoup.nodes.Document;
import org.jsoup.nodes.Element;
import org.jsoup.select.Elements;
import org.junit.Test;

public class NativeTest {

  @Test
  public void testNative() throws IOException {
    File tempDir = InstrumentationRegistry.getContext().getCacheDir();
    File logFile = new File(tempDir, "native_test.xml");

    int failed = nativeTestNative(tempDir.getAbsolutePath(), logFile.getAbsolutePath());

    if (failed != 0) {
      String log = readXmlLog(logFile);
      if (log.isEmpty()) {
        throw new IllegalStateException(failed + " test(s) failed, but no log.");
      } else {
        throw new IllegalStateException(failed + " test(s) failed\n\n" + readXmlLog(logFile));
      }
    }
  }

  private String readXmlLog(File logFile) throws IOException {
    StringBuilder builder = new StringBuilder();

    Document document = Jsoup.parse(logFile, "utf-8");

    Elements failures = document.getElementsByAttributeValue("result", "failure");
    for (Element failure : failures) {
      builder.append(failure.getElementsByTag("id").get(0).text()).append("\n");
      builder.append(failure.getElementsByTag("fn").get(0).text()).append("\n");
      builder.append(failure.getElementsByTag("message").get(0).text()).append("\n");
      builder.append("\n");
    }

    Elements errors = document.getElementsByAttributeValue("result", "error");
    for (Element error : errors) {
      builder.append(error.getElementsByTag("id").get(0).text()).append("\n");
      builder.append(error.getElementsByTag("fn").get(0).text()).append("\n");
      builder.append(error.getElementsByTag("message").get(0).text()).append("\n");
      builder.append("\n");
    }

    return builder.toString();
  }

  static {
    System.loadLibrary("image-test");
  }

  private static native int nativeTestNative(String tempDir, String logFile);
}

package com.hippo.image.example;

import android.os.Bundle;
import android.os.Debug;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.ImageView;

import com.hippo.image.Image;
import com.hippo.image.ImageData;

import junit.framework.Assert;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        ImageData data = Image.decode(getResources().openRawResource(R.raw.apng), false);

        ImageView imageView = (ImageView) findViewById(R.id.image);
        final ImageDrawable drawable = new ImageDrawable(new ImageBitmap(data, 1));
        drawable.start();
        imageView.setImageDrawable(drawable);



        //testAPNG();
        //testGIF();
    }

    private void testAPNG() {
        ImageData data;

        for (int i = 0; i < 500; i++) {
            data = Image.decode(getResources().openRawResource(R.raw.apng), true);
            data.complete();
            Assert.assertEquals(true, data.isCompleted());
            data.recycle();

            if (i % 100 == 0) {
                Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
            }
        }
        Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
        Log.d("TAG", "png part-complete done");

        for (int i = 0; i < 500; i++) {
            data = Image.decode(getResources().openRawResource(R.raw.apng), false);
            Assert.assertEquals(true, data.isCompleted());
            data.recycle();

            if (i % 100 == 0) {
                Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
            }
        }
        Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
        Log.d("TAG", "png non-part done");

        for (int i = 0; i < 500; i++) {
            data = Image.decode(getResources().openRawResource(R.raw.apng), true);
            Assert.assertEquals(false, data.isCompleted());
            data.recycle();

            if (i % 100 == 0) {
                Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
            }
        }
        Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
        Log.d("TAG", "png part done");
    }

    private void testGIF() {
        ImageData data;

        for (int i = 0; i < 1; i++) {
            data = Image.decode(getResources().openRawResource(R.raw.gif), true);
            data.complete();
            Assert.assertEquals(true, data.isCompleted());
            data.recycle();

            if (i % 100 == 0) {
                Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
            }
        }
        Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
        Log.d("TAG", "gif part-complete done");

        for (int i = 0; i < 1; i++) {
            data = Image.decode(getResources().openRawResource(R.raw.gif), false);
            Assert.assertEquals(true, data.isCompleted());
            data.recycle();

            if (i % 100 == 0) {
                Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
            }
        }
        Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
        Log.d("TAG", "gif non-part done");

        for (int i = 0; i < 200; i++) {
            data = Image.decode(getResources().openRawResource(R.raw.gif), true);
            Assert.assertEquals(false, data.isCompleted());
            data.recycle();

            if (i % 100 == 0) {
                Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
            }
        }
        Log.i("TAG", "" + Debug.getNativeHeapAllocatedSize());
        Log.d("TAG", "gif part done");
    }
}

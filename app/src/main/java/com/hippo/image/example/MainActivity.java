package com.hippo.image.example;

import android.graphics.Bitmap;
import android.graphics.Rect;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.widget.ImageView;

import com.hippo.image.BitmapDecoder;
import com.hippo.image.BitmapRegionDecoder;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        ImageView imageView = (ImageView) findViewById(R.id.image);

        BitmapRegionDecoder decoder = BitmapRegionDecoder.newInstance(getResources().openRawResource(R.raw.jpeg_ycck));

        Rect rect = new Rect(200, 200, 500, 500);
        Bitmap bitmap = decoder.decodeRegion(rect, BitmapDecoder.CONFIG_AUTO, 3);
        imageView.setImageBitmap(bitmap);


        /*
        ImageData data = Image.decode(getResources().openRawResource(R.raw.skip_frist), false);
        final ImageDrawable drawable = new ImageDrawable(new ImageBitmap(data, 1));
        drawable.start();
        imageView.setImageDrawable(drawable);
        */


        //testFormatSupported();
        //testAPNG();
        //testGIF();

        /*
        final ImageInfo info = new ImageInfo();
        final boolean result = BitmapDecoder.decode(getResources().openRawResource(R.raw.jpeg_cmyk), info);
        if (result) {
            Log.d("TAG", "width = " + info.width);
            Log.d("TAG", "height = " + info.height);
            Log.d("TAG", "format = " + info.format);
            Log.d("TAG", "opaque = " + info.opaque);
            Log.d("TAG", "frameCount = " + info.frameCount);
        } else {
            Log.d("TAG", "result false");
        }
        */


        //final long t0 = System.nanoTime();
        //Bitmap bitmap = BitmapDecoder.decode(getResources().openRawResource(R.raw.skip_frist), BitmapDecoder.CONFIG_ARGB_8888, 1);

        //BitmapFactory.Options options = new BitmapFactory.Options();
        //options.inPreferredConfig = Bitmap.Config.RGB_565;
        //Bitmap bitmap = BitmapFactory.decodeStream(getResources().openRawResource(R.raw.skip_frist), null, options);

        //final long t1 = System.nanoTime();

        //Log.d("TAG", "" + ((t1 - t0) / 1000000));
    }

}

package com.hippo.image.example;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.widget.ImageView;

import com.hippo.image.ImageBitmap;
import com.hippo.image.ImageDrawable;
import com.hippo.image.RecycledException;

import java.io.InputStream;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        ImageView imageView = (ImageView) findViewById(R.id.image);
        if (imageView == null) {
            return;
        }

        InputStream is = getResources().openRawResource(R.raw.ycck);
        ImageBitmap bitmap = ImageBitmap.decode(is);
        if (bitmap == null) {
            return;
        }
        try {
            imageView.setImageDrawable(new ImageDrawable(bitmap));
        } catch (RecycledException e) {
            e.printStackTrace();
        }
    }
}

# couplets

This interactive art piece brings fourteen couples from the Metropolitan Museum back to life. It allows people to discover the stories of different couples in history. Viewers can animate facial expressions of these couples by use of puppeteering. The left half of the viewer’s face puppeteers the male partner, while the right half of the face puppeteers the female partner. By using the two halves of their face, the viewer can simulate the conversation that was potentially happening between these couples back in history.

Check out what the final performance looked like [here](https://vimeo.com/96155271)!

# tech specs

The project was built with the [openFrameworks](https://github.com/openframeworks/openFrameworks) arts-engineering toolkit, and various addons including [ofxCV](https://github.com/kylemcdonald/ofxCv), [ofxDelaunay](https://github.com/obviousjim/ofxDelaunay), [ofxPuppet](https://github.com/ofZach/ofxPuppet), and [ofxFaceTracker](https://github.com/kylemcdonald/ofxFaceTracker).

# instructions

If you'd like to create your own couplets, the only things you will need to do will be to 
* Create a new folder name with a prefix of `couplets` inside `/bin/data`.

Inside this new folder create
* A `bg.jpg` file that serves as a background for your `couplet`. Don't worry about sizes, the software automatically resizes to fit the screen.
* A `data.xml` file that contains some metadata about your couplet. [Here](https://github.com/afahim/couplets/blob/master/bin/data/couple3/data.xml) is an example.
* A `couple.jpg` file that contains a contour of the two partners in the couple you want to puppeteer. Like [this](https://github.com/afahim/couplets/blob/master/bin/data/couple3/couple.jpg) one.
* A `couple.png` file that contains the cutout of your couplet. [Here](https://github.com/afahim/couplets/blob/master/bin/data/couple3/couple.png) is an example.

Once you have a couplet folder, you can now start creating the puppet model for your couplet.
* Run the `couplets` project using openFrameworks.
* Navigate to your new couplet using the right and left keys on your keyboard.
* Once on your new couplet, press `c` and this will initiate the 'create puppet mode`.
* Using your mouse, click on features that you want to be part of your puppet. We identify 5 facial features in this order
  * Five points for the left eye brow.
  * Five points for the right eye brow.
  * Three points for the nose.
  * Four points for the upper lip.
  * Four points for the lower lip.
* Once you have tagged all the above points, press `s` to save the puppet. Now when you navigate to the new couplet again, you should be able to use your face's gestures to control your new `couplet`!

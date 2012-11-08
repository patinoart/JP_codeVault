#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxControlPanel.h"

#define NUM_BYTES 4


class testApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		
        // control panel
		ofxControlPanel			panel;
        
        bool                    bDrawDiagnostic;
        bool                    bLearnBg;
        int                     threshold;
        int                     nDilations;
        int                     nErosions;
        int                     maxOperations;
    
        int						width, height;  // cam w x h
        int                     totalPixels;    // pixels w * h
        int                     numClicks;      // counter to number captured images
        float                   averagingPix;   // number we average pixels with
        
        // declare video camera + color images +  additions
        ofVideoGrabber			video;
        ofImage                 colorBG;
        ofImage                 colorAddition;
        
		ofxCvColorImage			videoColorCvImage;
		
        // our grayscale images + additions
        ofxCvGrayscaleImage		videoGrayscaleCvImage;
		ofxCvGrayscaleImage		videoBgImage;
        ofImage                 grayAddition;
    
        // using absdiff and dilation/erode to get better results
        ofxCvGrayscaleImage		videoDiffImage;
        ofImage                 diffImage;
        ofImage                 imgStckTotal;
    
        vector < ofImage >      imageStack;
    
        /////////////////////////////////////////////////////////
        // Arduino Variables from serialExample
        /////////////////////////////////////////////////////////
        
        int xVal;
        bool bTrigger;
        
        bool bSendSerialMessage; // a flag for sending serial
        // this will be used to count the number of frames
        // that have passed since the last time the app reads from the serial port
        
        //data from serial, we will be reading 4 bytes, two bytes for each integer
        unsigned char bytesRead[NUM_BYTES];
        int countCycles;
        
        ofSerial serial;
        
        
};

#endif

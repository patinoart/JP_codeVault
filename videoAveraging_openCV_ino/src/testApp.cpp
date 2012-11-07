#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    //grabBackground is now bLearnBg
    numClicks = 0;                          // how many times have you captured the background
    
    // get the width and height, and allocate color and grayscale images: 
	width           = 320*3;
	height          = 240*3;
    totalPixels = width * height;           // total pixels we will use later
    
    /* Comparing the variables from the older test program to this one
	// declare video camera + color images + additions
    ofVideoGrabber			video;                      = vidGrabber
    ofImage                 colorBG;                    = colorBG
    ofImage                 colorAddition;              = colorAddition
    
    // our grayscale images + additions
    ofxCvGrayscaleImage		videoGrayscaleCvImage;      = grayFrame
    ofxCvGrayscaleImage		videoBgImage;               = background
    ofImage                 grayAddition;               = grayAddition
    
    // using absdiff and dilation/erode to get better results
    ofxCvGrayscaleImage		videoDiffImage;
    */
    
    // setup video grabber:
	video.setVerbose(true);
    video.initGrabber(width, height);
	
    videoColorCvImage.allocate(width, height);                      // allocate pixels for the BW + RGB vid
	videoGrayscaleCvImage.allocate(width, height);
    
    grayAddition.allocate(width, height, OF_IMAGE_GRAYSCALE);       // allocate the pixels for our averaged images
    colorAddition.allocate(width, height, OF_IMAGE_COLOR);
    diffImage.allocate(width, height, OF_IMAGE_COLOR);
	
    videoBgImage.allocate(width, height);                           // allocate pixels for the adjusted video
	videoDiffImage.allocate(width, height);
    
    unsigned char * startingPixels = video.getPixels();             // we start by having the average starting with base pixels
    
    for (int i = 0; i < totalPixels; i++) {
        startingPixels[i] = 0;
    }
    grayAddition.setFromPixels(startingPixels, width , height, OF_IMAGE_GRAYSCALE);
	colorAddition.setFromPixels(startingPixels, width, height, OF_IMAGE_COLOR);
    diffImage.setFromPixels(startingPixels, width, height, OF_IMAGE_COLOR);
    
    bDrawDiagnostic = true;

	//////////////////////////////////////////////////////////////////  
    ////////////////////////////////////////////////////////////////// GUI SETUP
    //////////////////////////////////////////////////////////////////
    // setup the main panel for the GUI
    panel.setup("cv settings", 1024, 0, 276, 800);
	panel.addPanel("control", 1, false);
	
	panel.setWhichPanel("control");
	panel.setWhichColumn(0);
	
    // toggle to turn on bg
    panel.addToggle("learn background ", "B_LEARN_BG", true);
	panel.addSlider("threshold ", "THRESHOLD", 127, 0, 255, true);
	
	// slider for dilations and erosions
    panel.addSlider("nDilations ", "N_DILATIONS", 2, 0, 20, true);  // dilation to make brighter
	panel.addSlider("nErosions ", "N_EROSION", 0, 0, 20, true);     // erosion to make darker
	
    // number to control how much the pixels are being added to eachother
    panel.addSlider("nAverage", "N_AVERAGE", 2, 0.25, 2.0, false);
    
	// vals are stored in this file in the data folder
    // these are the defaults that load up when the program is compiled
    panel.loadSettings("cvSettings.xml");
    
    //////////////////////////////////////////////////////////////////  
    ////////////////////////////////////////////////////////////////// ARDUINO SETUP
    //////////////////////////////////////////////////////////////////
    
    ofSetVerticalSync(true);
	
	bSendSerialMessage = false;
	ofBackground(255);	
	ofSetLogLevel(OF_LOG_VERBOSE);
	
	font.loadFont("DIN.otf", 64);
	
	serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
	
	// this should be set to whatever com port your serial device is connected to.
	// (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
	// arduino users check in arduino app....		
	serial.setup(5, 9600); //open the first device
	//serial.setup("COM4"); // windows example
	//serial.setup("/dev/tty.usbserial-A4001JEC",9600); // mac osx example
	//serial.setup("/dev/ttyUSB0", 9600); //linux example
	
	nTimesRead = 0;
	nBytesRead = 0;
	readTime = 0;
	memset(bytesReadString, 0, 4);
}

//--------------------------------------------------------------
void testApp::update(){
    
    // set background color
    ofBackground(100,100,100);
		
	panel.update();
	
	
	bLearnBg			= panel.getValueB("B_LEARN_BG");
	threshold			= panel.getValueI("THRESHOLD");
	nDilations			= panel.getValueI("N_DILATIONS");
	nErosions			= panel.getValueI("N_EROSION");
	maxOperations		= MAX(nDilations, nErosions);
    averagingPix            = panel.getValueF("N_AVERAGE");

	video.update();
	
	if (video.isFrameNew()){
		
		videoColorCvImage.setFromPixels(video.getPixels(), width, height);
		videoGrayscaleCvImage = videoColorCvImage;
		
		if ( bLearnBg ){ 
			videoBgImage = videoGrayscaleCvImage;
            colorBG.setFromPixels( video.getPixelsRef() );
            
            // use a forloop to run through the two pixel sets and add them together
            
            // setting the pixels for the grayscale images
            unsigned char * backgroundPixels = videoBgImage.getPixels();
            unsigned char * grayAdditionPixels = grayAddition.getPixels();
            
            for (int i = 0; i < totalPixels; i++) {
                int averageGray = ( grayAdditionPixels[i] +  backgroundPixels[i] )/averagingPix;
                grayAdditionPixels[i] = averageGray;
                
            }
            
            // setting the pixels for the color images
            unsigned char * colorBGPixels = colorBG.getPixels();
            unsigned char * colorAdditionPixels = colorAddition.getPixels();
            
            // setting the pixels for the diffImage
            unsigned char * diffImagePixels = colorBG.getPixels();
            unsigned char * notDiffImgPixels = colorAddition.getPixels();
            unsigned char * vidDiffImagePixels = videoDiffImage.getPixels();
            
            for (int i = 0; i < totalPixels * 3; i++) {
                int averageColor = ( colorAdditionPixels[i] + colorBGPixels[i] )/averagingPix;
                colorAdditionPixels[i] = averageColor;
                if ( vidDiffImagePixels[i/3] == 255 ) {
                    diffImagePixels[i];
                } else {
                    diffImagePixels[i] = notDiffImgPixels[i];
                }   
            }
            
            grayAddition.setFromPixels(grayAdditionPixels, width , height, OF_IMAGE_GRAYSCALE);
            colorAddition.setFromPixels(colorAdditionPixels, width, height, OF_IMAGE_COLOR);
            diffImage.setFromPixels(diffImagePixels, width, height, OF_IMAGE_COLOR);
            
			panel.setValueB("B_LEARN_BG", false);
		}
		
		if (ofGetElapsedTimef() < 1.5){
			videoBgImage = videoGrayscaleCvImage;
		}
		
		videoDiffImage.absDiff(videoGrayscaleCvImage, videoBgImage);
		videoDiffImage.threshold(threshold);
		
		for (int i = 0; i < maxOperations; i++){
			if (i < nErosions)	videoDiffImage.erode();
			if (i < nDilations)	videoDiffImage.dilate();
		}
	}
    
    //////////////////////////////////////////////////////////////////  
    ////////////////////////////////////////////////////////////////// ARDUINO UPDATE
    //////////////////////////////////////////////////////////////////
    
    if (bSendSerialMessage){
		
		// (1) write the letter "a" to serial:
		serial.writeByte('a');
		
		// (2) read
		// now we try to read 3 bytes
		// since we might not get them all the time 3 - but sometimes 0, 6, or something else,
		// we will try to read three bytes, as much as we can
		// otherwise, we may have a "lag" if we don't read fast enough
		// or just read three every time. now, we will be sure to 
		// read as much as we can in groups of three...
		
		nTimesRead = 0;
		nBytesRead = 0;
		int nRead  = 0;  // a temp variable to keep count per read
		
		unsigned char bytesReturned[3];
		
		memset(bytesReadString, 0, 4);
		memset(bytesReturned, 0, 3);
		
		while( (nRead = serial.readBytes( bytesReturned, 3)) > 0){
			nTimesRead++;	
			nBytesRead = nRead;
		};
		
		memcpy(bytesReadString, bytesReturned, 3);
		
		bSendSerialMessage = false;
		readTime = ofGetElapsedTimef();
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	ofSetColor(255, 255, 255);
    
    
    if (bDrawDiagnostic == true){
        // draw first column of images, color
        video.draw(20, 20, width/3, height/3);                         // color video
        colorBG.draw(20, 280, width/3, height/3);                      // still image of our color video capture
        colorAddition.draw(20, 540, width/3, height/3);                // averaged image based on color image
        
        // second column, grayscale
        videoGrayscaleCvImage.draw(360, 20, width/3, height/3);  // grayscale video
        videoBgImage.draw(360, 280, width/3, height/3);          // still image of grayscale video capture
        grayAddition.draw(360, 540, width/3, height/3);                    // averaged image based on grayscale image
        
        // third column, difference
        videoDiffImage.draw(700, 20, width/3, height/3);     // difference video
        diffImage.draw(700, 280, width/3, height/3);
    } else {
        diffImage.draw(20, 20, width, height);
    }
	
	panel.draw();
    
    //////////////////////////////////////////////////////////////////  
    ////////////////////////////////////////////////////////////////// ARDUINO DRAW
    //////////////////////////////////////////////////////////////////
    
    if (nBytesRead > 0 && ((ofGetElapsedTimef() - readTime) < 0.5f)){
		ofSetColor(0);
	} else {
		ofSetColor(220);
	}
	stringstream msg;
    
    msg
	<< "click to test serial:\n" << endl
	<< "nBytes read " + ofToString(nBytesRead) << endl
	<< "nTimes read " + ofToString(nTimesRead) << endl
	<< "read: " + ofToString(bytesReadString) << endl
	<< "at time " + ofToString(readTime, 3) << endl;
    
    ofDrawBitmapString(msg.str(), 1054, 315);
    
//  stringstream msg;
//	
//	msg
//	<< "  Framerate: " << fps << endl
//	<< "Tracking(t): " << statusSkeleton << endl
//	<< "       User: " << UserIdOscV << endl
//	//<< "  Body Dist: " << bodyDist << endl
//	<< "      q+/a-: " << stringMaxPV << endl
//	<< "      w+/e-: " << stringMaxPF << endl
//	<< "      1+/2-: " << stringPartSize << endl;
//	
//	ofDrawBitmapString(msg.str(), 10, 20);

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    
    if (key == 'b' ){
		panel.setValueB("B_LEARN_BG", true);
        bLearnBg = panel.getValueB("B_LEARN_BG");
        diffImage.saveImage("testImage_" + ofToString(numClicks) + ".png");
        numClicks++; 
	} else if (key == ' '){	
		bDrawDiagnostic = !bDrawDiagnostic;
    }

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	panel.mouseDragged(x,y,button);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	panel.mousePressed(x,y,button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	panel.mouseReleased();
    
    cout << mouseX << ", " << mouseY << endl;
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}


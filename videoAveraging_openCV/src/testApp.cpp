#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    //grabBackground is now bLearnBg
    numClicks = 0;                          // how many times have you captured the background
    
    // get the width and height, and allocate color and grayscale images: 
	width           = 320;
	height          = 240;
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
}

//--------------------------------------------------------------
void testApp::update(){
    
    // set background color
    ofBackground(100,100,100);
		
	panel.update();
	
	
	bool bLearnBg			= panel.getValueB("B_LEARN_BG");
	int threshold			= panel.getValueI("THRESHOLD");
	int nDilations			= panel.getValueI("N_DILATIONS");
	int nErosions			= panel.getValueI("N_EROSION");
	int maxOperations		= MAX(nDilations, nErosions);
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
}

//--------------------------------------------------------------
void testApp::draw(){
	
	ofSetColor(255, 255, 255);
    
    // draw first column of images, color
    video.draw(20, 20);                         // color video
    colorBG.draw(20, 280);                      // still image of our color video capture
    colorAddition.draw(20, 540);                // averaged image based on color image
    
    // second column, grayscale
    videoGrayscaleCvImage.draw(360, 20, 320, 240);  // grayscale video
	videoBgImage.draw(360, 280, 320, 240);          // still image of grayscale video capture
    grayAddition.draw(360, 540);                    // averaged image based on grayscale image
    
    // third column, difference
    videoDiffImage.draw(700, 20, 320, 240);     // difference video
    diffImage.draw(700, 280);
	
	panel.draw();

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

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
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}


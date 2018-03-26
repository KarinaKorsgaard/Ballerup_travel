#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // serial
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    bpg.setup();
    

    // this should be set to whatever com port your serial device is connected to.
    // (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
    // arduino users check in arduino app....
    int baud = 9600;
#ifdef __APPLE__
    system("cancel -a"); // clears print que
    serial.setup(0, baud); //open the first device
   // cout << "on mac"<< endl;
#else
   // system("net stop spooler");
   // system("del %systemroot%\System32\spool\printers\* /Q /F /S");
   // system("net start spooler");
	string cwd = ofFilePath::getCurrentWorkingDirectory();
	string command = cwd + "\\data\\clearPrinter.bat";
	system(command.c_str());

    //serial.setup("COM3", baud); // windows example
    //cout << "on windows"<< endl;
#endif
    //serial.setup("/dev/tty.usbserial-A4001JEC", baud); // mac osx example
    //serial.setup("/dev/ttyUSB0", baud); //linux example
    nTimesRead = 0;
    nBytesRead = 0;
    readTime = 0;
    memset(bytesReadString, 0, 4);
    //    //serial
    
    gui.setup();
    gui.add(animationTime.set("AnimationTime",1.,5.,0.01));
    gui.add(randomFlipInterval.set("max FlipInterval",1.,0.,10.));
    gui.add(materialSwapTime.set("materialSwapTime",15.,0.,40.));
    gui.loadFromFile("settings.xml");
    

    string letters = "0 1 2 3 4 5 6 7 8 9 A B C D E F G H I J K L M N O P Q R S T U V X Y Z Æ Ø Å , . + ...";
    vector<string>_alphabet = ofSplitString(letters, " ");
    _alphabet.push_back(" ");
    
    
    //cout << "num letters " << _alphabet.size() << endl;
    
    font.load("fonts/BergenMono/BergenMono-Regular.otf", 20);
    font.setLetterSpacing(0.8);
    ofRectangle r = font.getStringBoundingBox("Åg", 0, 0);
    charWidth = r.width*1.2; //
    charHeight = r.height*2.;
    
    //getting 16:9 w 12 lines and 4+17+17+4+2
    int lines = 12;
    float rows = 4+17+17+4+2;
    float w_total = 1920;
	charWidth = w_total/rows;
    float h_total = (w_total*9.f)/16.f;
    //cout<< w_total <<" "<<h_total<<endl;
    charHeight = 1080/lines;
    
    //cout << charWidth << " "<< charHeight <<endl;
    ofEnableAntiAliasing();
    ofEnableAlphaBlending();
    
	string slash = "\\";
#ifdef __APPLE__
	slash = "/";
#endif
	
    ofDirectory dir;
    dir.listDir("emojis");
    dir.allowExt(".png");
   // cout<< "emoji dir "<< dir.size()<<endl;
    
    for(int i = 0; i<dir.size();i++){
        ofImage e;
        e.load(dir.getPath(i));
        e.resize(charWidth, charWidth * e.getHeight()/e.getWidth());
		//cout << dir.getPath(i) << endl;
        ofBufferObject b = drawTexture(ofColor(255), charWidth, charHeight, "", e);
        emojis[ofSplitString(dir.getPath(i),slash)[1]].allocate(charWidth, charHeight, GL_RGBA);
        emojis[ofSplitString(dir.getPath(i),slash)[1]].loadData(b, GL_RGBA, GL_UNSIGNED_BYTE);
       // cout << dir.getPath(i)<< endl;
    }
    
    ofImage arr;
    arr.load("otherImages/arrow.png");
    arr.resize(charWidth, charWidth * arr.getHeight()/arr.getWidth());
    ofBufferObject arrb = drawTexture(ofColor(255), charWidth, charHeight, "", arr);
    arrow.allocate(charWidth, charHeight, GL_RGBA);
    arrow.loadData(arrb, GL_RGBA, GL_UNSIGNED_BYTE);
    
  
    arr.load("otherImages/empty.png");
    arr.resize(charWidth, charWidth * arr.getHeight()/arr.getWidth());
    ofBufferObject emptyb = drawTexture(ofColor(255), charWidth, charHeight, "", arr);
    empty.allocate(charWidth, charHeight, GL_RGBA);
    empty.loadData(emptyb, GL_RGBA, GL_UNSIGNED_BYTE);
    
    for(int i = 0; i<_alphabet.size();i++){

        ofImage empty;
        ofBufferObject b = drawTexture(ofColor(255), charWidth, charHeight, _alphabet[i], empty);
        ofBufferObject b_yellow = drawTexture(ofColor(255,222,2), charWidth, charHeight, _alphabet[i], empty);
        
        ofTexture tex;
        textures.push_back(tex);
        textures_yellow.push_back(tex);
        
        textures.back().allocate(charWidth, charHeight, GL_RGBA);
        textures_yellow.back().allocate(charWidth, charHeight, GL_RGBA);
        
        textures.back().loadData(b, GL_RGBA, GL_UNSIGNED_BYTE);
        textures_yellow.back().loadData(b_yellow, GL_RGBA, GL_UNSIGNED_BYTE);
        
    }
    
    

    vector<string>gates;
    vector<string>let = {"A", "B","C", "D","E", "F","G", "H","I"};

    for(int i = 0; i<let.size();i++)
        for(int u = 0; u<9;u++)
            gates.push_back(let[i]+ofToString(u+1));
        
    
    // json crap
    ofxJSONElement result;
    std::string file = "example.json";
    
    // Now parse the JSON
    bool parsingSuccessful = result.open(file);
    
    if (parsingSuccessful) {
        ofLogError("ofApp::setup")  << "JSON success" << endl;
    }
    else ofLogError("ofApp::setup")  << "Failed to parse JSON" << endl;
    
    
    desitnations.resize(result.size());
    //cout << "detinations "<< desitnations.size()<<endl;
    
    for(int i = 0; i<desitnations.size();i++){
        const Json::Value& beskrivelse = result[i];
        desitnations[i].destination = beskrivelse["destination"].asString();
        desitnations[i].number = gates[ofRandom(gates.size())];
		desitnations[i].str = beskrivelse["destination"].asString()+".png";
        
        string em = beskrivelse["emoji"].asString();
        if(emojis.find(em)!=emojis.end())
            desitnations[i].emoji = &emojis[em];
        else if(emojis.find(em+".png")!=emojis.end())
            desitnations[i].emoji = &emojis[em+".png"];
        else if(emojis.find(em+".jpg")!=emojis.end())
            desitnations[i].emoji = &emojis[em+".jpg"];
        else
            desitnations[i].emoji = &emojis["emojis/standart.png"];
        
        int mat = 0;
        int u = 0;
        while(mat==0){
            desitnations[i].material.push_back(beskrivelse["materiale"+ofToString(u+1)].asString());
            desitnations[i].materialDescription.push_back(
                                                          bpg.transformToCollumn( beskrivelse["materialeBeskriv"+ofToString(u+1)].asString())
                                                          );
            if(desitnations[i].material.back()==""){
                desitnations[i].material.pop_back();
                desitnations[i].materialDescription.pop_back();
                mat = 1;
            }
            u++;
        }
    }
    
    ofImage imgA;
    imgA.load("otherImages/upper.png");
    imgA.resize(charWidth, charHeight);
    tUp = imgA.getTexture();
    ofImage imgB;
    imgB.load("otherImages/lower.png");
    imgB.resize(charWidth, charHeight);
    tLo = imgB.getTexture();
    
    wh_number.resize(MIN(desitnations.size(),NUM_DESTINATIONS));
    for(int i = 0; i<wh_number.size();i++)
        wh_number[i].setup(charWidth, charHeight, animationTime, &textures, 4, letters, &tUp, &tLo);
    wh_destination.resize(MIN(desitnations.size(),NUM_DESTINATIONS));
    for(int i = 0; i<wh_destination.size();i++)
        wh_destination[i].setup(charWidth, charHeight, animationTime, &textures_yellow, 17, letters, &tUp, &tLo);
    wh_material.resize(MIN(desitnations.size(),NUM_DESTINATIONS));
    for(int i = 0; i<wh_material.size();i++)
        wh_material[i].setup(charWidth, charHeight, animationTime, &textures, 17, letters, &tUp, &tLo);
    wh_numberOfMaterials.resize(MIN(desitnations.size(),NUM_DESTINATIONS));
    for(int i = 0; i<wh_numberOfMaterials.size();i++)
        wh_numberOfMaterials[i].setup(charWidth, charHeight, animationTime, &textures, 4, letters, &tUp, &tLo);
    
    for(int i = 0; i<MIN(desitnations.size(),NUM_DESTINATIONS);i++) {
        destination_indxes.push_back(i);
    }
    
    for(int i = 0; i<MIN(desitnations.size(),NUM_DESTINATIONS);i++){
        int d = destination_indxes[i];
        wh_destination[i].changeString(desitnations[d].destination,desitnations[d].emoji);
        wh_number[i].changeString(desitnations[d].number,desitnations[d].emoji);
        wh_material[i].changeString(desitnations[d].material[desitnations[i].currentMaterial],desitnations[d].emoji);
        wh_numberOfMaterials[i].changeString("+"+ofToString(desitnations[i].material.size()));
        desitnations[d].time = ofRandom(15);
    }
	ofSetWindowShape(1920, 1080);
	ofSetFullscreen(true);
	debug = false;

}

//--------------------------------------------------------------
void ofApp::update(){
    ofSetWindowTitle(ofToString(ofGetFrameRate()));
    timeSinceLastFlip += ofGetLastFrameTime();
    
    for(int i = 0; i<MIN(desitnations.size(),NUM_DESTINATIONS);i++){
        wh_destination[i].update(animationTime);
        wh_number[i].update(animationTime);
        wh_material[i].update(animationTime);
        wh_numberOfMaterials[i].update(animationTime);
        desitnations[i].time += ofGetLastFrameTime();
        
        if(desitnations[i].time > materialSwapTime){
            desitnations[i].time = 0.0;
            desitnations[i].currentMaterial++;
            desitnations[i].currentMaterial = desitnations[i].currentMaterial%desitnations[i].material.size();
            string newMaterial = desitnations[i].material[desitnations[i].currentMaterial];
            if(newMaterial!=wh_material[i].next_string)
                wh_material[i].changeString(newMaterial);
        }
        
    }
    
    
    if(!isInitialized)initialiseArdiono();
    else readArduino();
    
    if(input>-1){
        if (input < MIN(desitnations.size(),NUM_DESTINATIONS))printBoardingPass(input);
        if (input == 11) {
            for (int i = 0; i<MIN(desitnations.size(),NUM_DESTINATIONS); i++) {
                destination_indxes[i] += MIN(desitnations.size(),NUM_DESTINATIONS);
                destination_indxes[i] = destination_indxes[i] % desitnations.size();
                
                int d = destination_indxes[i];
                wh_destination[i].changeString(desitnations[d].destination,desitnations[d].emoji);
                wh_number[i].changeString(desitnations[d].number,desitnations[d].emoji);
                wh_material[i].changeString(desitnations[d].material[int(ofRandom(desitnations[d].material.size()))],desitnations[d].emoji);
                
                wh_numberOfMaterials[i].changeString("+"+ofToString(desitnations[d].material.size()));
                
            }
        }

        
        input = -1;
        //if(debug)cout <<"processed "<< input << endl;
    }
    if (motioninput) {
        doRandomFlip(5);
        motioninput=false;
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){

    ofSetColor(255);
    ofBackground(0);
    
    if(debug)gui.draw();
    ofPushMatrix();
    if(debug)ofScale(0.6, 0.6);
    if(debug)ofTranslate(0,150);
    int totalLength = 2 + wh_destination[0].max_length + wh_number[0].max_length + wh_numberOfMaterials[0].max_length + wh_material[0].max_length;
    
    
    for(int i = 0; i<totalLength;i++){
        empty.draw(i*charWidth*1.05, 0);
    }
    ofTranslate(0,  1.05*charHeight-10);
    
    for(int i = 0; i<MIN(desitnations.size(),NUM_DESTINATIONS);i++){
        ofPushMatrix();
        
        empty.draw(0,10 + 1.05*i*charHeight);
        ofTranslate(1*charWidth*1.05, 0);
        
        wh_number[i].draw(0, 10 + 1.05*i*charHeight);
        ofTranslate(wh_number[i].max_length*charWidth*1.05, 0);
        
        
        wh_destination[i].draw(0, 10 + 1.05*i*charHeight);
        ofTranslate(wh_destination[i].max_length*charWidth*1.05, 0);
        
        wh_material[i].draw(0, 10 + 1.05*i*charHeight);
        ofTranslate(wh_material[i].max_length*charWidth*1.05, 0);
        
        wh_numberOfMaterials[i].draw(0, 10 + 1.05*i*charHeight);
        ofTranslate(wh_numberOfMaterials[i].max_length*charWidth*1.05, 0);
        
        ofSetColor(255);
        arrow.draw(0, 10 + 1.05*i*charHeight);
        ofPopMatrix();
    }
    float numLines = wh_destination.size();
    for(int i = 0; i<totalLength;i++){
        empty.draw(i*charWidth*1.05, 10+ 1.05*charHeight*numLines);
    }
    //ofTranslate(0, 1.05*charHeight);
    
    ofPopMatrix();
    
}
//--------------------------------------------------------------
void ofApp::printBoardingPass(int d){
    // print..
	string cwd = ofFilePath::getCurrentWorkingDirectory();

    string boardingPass = bpg.generate(desitnations[d], desitnations[d].currentMaterial);
#ifdef __APPLE__
	string command = "lp "+ ofSplitString(cwd,"/bin")[0] + "/bin/data/boardingPasses/"+ boardingPass;
    if(!debug)system(command.c_str());
   // cout << "lp "+ ofSplitString(cwd,"/bin")[0] + "/bin/data/boardingPasses/"+ desitnations[d].str << endl;
#else
	string command = "SumatraPDF.exe -print-to-default "+ cwd +"\\data\\boardingPasses\\"+ boardingPass;
	//system(command.c_str());
	cout << command <<" "<< cwd << endl;

	if(!debug)system(command.c_str());

	//string t = ofToDataPath("test.bat");
	//cout << path << endl;
	
#endif

}

//--------------------------------------------------------------
void ofApp::readArduino(){
    int tempinput = -1;
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
    
    //if(nBytesRead>0){
    memcpy(bytesReadString, bytesReturned, 3);
    
    bSendSerialMessage = false;
    readTime = ofGetElapsedTimef();

    string fromArduino = string(bytesReadString);
    char fa = fromArduino[0]-'0';
    tempinput = fa;
    
    if(tempinput == MOTION_INPUT){
        motioninput = true;
    }
    else if(tempinput>-1) {
        
        if(p_input!=tempinput){
            p_input=tempinput;
            input = tempinput;
            if(debug)cout <<"From Arduino "<< input << endl;
        }
    }
}
//--------------------------------------------------------------
void ofApp::initialiseArdiono(){
    serial.listDevices();
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    
    // this should be set to whatever com port your serial device is connected to.
    // (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
    // arduino users check in arduino app....
    int baud = 9600;
	if(deviceList.size()>0) {
#ifdef __APPLE__
    serial.setup(0, baud); //open the first device
    cout << "on mac"<< endl;
#else
    serial.setup(0, baud); // windows example
    cout << "on windows"<< endl;
#endif
    //serial.setup("COM4", baud); // windows example
    //serial.setup("/dev/tty.usbserial-1421", baud); // mac osx example
    //serial.setup("/dev/ttyUSB0", baud); //linux example
    
    nTimesRead = 0;
    nBytesRead = 0;
    readTime = 0;
    memset(bytesReadString, 0, 4);
	}

    isInitialized = serial.isInitialized();
    if(isInitialized)cout << "arduino is on"<<endl;
}


void ofApp::doRandomFlip(int amount){
    if(timeSinceLastFlip>randomFlipInterval){
        
        for(int i = 0; i<amount; i++){
            
            int a = floorf(ofRandom(MIN(desitnations.size(),NUM_DESTINATIONS)));
            int b = floorf(ofRandom(MIN(desitnations.size(),NUM_DESTINATIONS)));
            int c = floorf(ofRandom(MIN(desitnations.size(),NUM_DESTINATIONS)));
            
            int aa =(int)ofRandom(wh_number[a].characters.size());
            int bb =(int)ofRandom(wh_destination[b].characters.size());
            int cc =(int)ofRandom(wh_material[c].characters.size());
            
            if(i==0){
              //  wh_number[a].characters[aa].flipToEmoji();
                wh_destination[b].characters[bb].flipToEmoji();
              //  wh_material[c].characters[cc].flipToEmoji();
            }
            //wh_number[a].characters[aa].flip();
                //wh_destination[b].characters[bb].flip();
            wh_material[c].characters[cc].flip();
            
        }
    }
    // if it is constant noise, nothing happens as it is reset all the time...
    timeSinceLastFlip = 0.0;
    
        
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(debug)doRandomFlip(5);
	int k = key - '0';
	if(k<desitnations.size())
		printBoardingPass(k);
    
    if(key == 'd')debug = !debug;
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}


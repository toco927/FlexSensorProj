#include <dummy.h>

#include <esp_now.h>
#include <WiFi.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32 //maybe change to ox3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// REPLACE WITH YOUR RECEIVER MAC Address
//E0:E2:E6:3D:AE:A4
uint8_t broadcastAddress[] = {0xE0, 0xE2, 0xE6, 0x3D, 0xAE, 0xA4};


const int multiOut = A0;


const int screen_SDA = D4;
const int screen_SCL = D5;


const int multi_B0 = D7;
const int multi_B1 = D8;
const int multi_B2 = D9;
const int multi_B3 = D10; //MSB


const int dot_D2 = D2;
const int dot_D3 = D3;




// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int flexThumb;  //Y0
  int flexPoint;  //Y1
  int flexMid;    //Y2
  int flexIndex;  //Y3
  int flexPinky;  //Y4
  int push1;     //Y5 - digital
  int push2;     //Y6 - digital
  int push3;     //Y7 - digital
  int stickRx;    //Y8
  int stickRy;    //Y9
  int stickPush; //Y10 - digital
  int potent;     //Y11
  int dead1;      //Y12
  int dead2;      //Y13
  int dead3;      //Y14
  int dead4;      //Y15
} struct_message;


// Create a struct_message called myData
struct_message myData;


esp_now_peer_info_t peerInfo;


// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);


  pinMode(multi_B0,OUTPUT);
  pinMode(multi_B1,OUTPUT);
  pinMode(multi_B2,OUTPUT);
  pinMode(multi_B3,OUTPUT);//msb
  pinMode(multiOut,INPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);


  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }


  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
 
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
 
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;); // Don't proceed, loop forever
    }
  display.display();
  delay(1500);
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.println("WORKING!");

}
 
void loop() {
  Serial.println("WORKING");
  int multiControl[] = {0,0,0,0}; //multiControl[0] is MSB
  for(int i = 0; i < 16; i++) {
    intBinaryArray(i,multiControl);
    writingMulti(multiControl);
    int temp = analogRead(multiOut);
    if( i >= 12){ //start of data data in struct
      setDataVar(i,5000,myData);
    }
    else {
      setDataVar(i,temp,myData);
    } 
    delay(20);//SWITCHING TIME FROM MULTIPLEXER
  }
  printStructMessageInfo(myData);
 
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  //delay(200);
}




void intBinaryArray(int value, int *binaryArray) {
    for (int i = 3; i >= 0; --i) {
        binaryArray[i] = value % 2;
        value /= 2;
    }
}


void writingMulti(int *binaryArray) {


  //BINARY [0]
  if(binaryArray[0] == 1) {
    digitalWrite(multi_B3,HIGH);
  }
  else if(binaryArray[0] == 0) {
    digitalWrite(multi_B3,LOW);
  }
  else {
    Serial.println("ERROR! Binary Array Wrong!");
  }


  //BINARY [1]
  if(binaryArray[1] == 1) {
    digitalWrite(multi_B2,HIGH);
  }
  else if(binaryArray[1] == 0) {
    digitalWrite(multi_B2,LOW);
  }
  else {
    Serial.println("ERROR! Binary Array Wrong!");
  }


  //BINARY [2]
  if(binaryArray[2] == 1) {
    digitalWrite(multi_B1,HIGH);
  }
  else if(binaryArray[2] == 0) {
    digitalWrite(multi_B1,LOW);
  }
  else {
    Serial.println("ERROR! Binary Array Wrong!");
  }


  //BINARY[3]
  if(binaryArray[3] == 1) {
    digitalWrite(multi_B0,HIGH);
  }
  else if(binaryArray[3] == 0) {
    digitalWrite(multi_B0,LOW);
  }
  else {
    Serial.println("ERROR! Binary Array Wrong!");
  }
}


void setDataVar(int i, int val1, struct_message &controllerStruct) {
  int val = val1;
  if (i > 15) {
    Serial.println("ERROR! Mem access not valid");
    return;
  }


  if (val > 4096 && (i != 12 && i != 13 && i != 14 && i !=15)) {
    Serial.println("ERROR! Digital Data not valid (0 - 4096)");
    val = 4097;
  }




//  if ((i == 5 || i == 6 || i == 7 || i == 10) && val > 1023) {
//    Serial.println("ERROR! Digital Data not valid (0 - 1023)");
//    return;
//  }


  int* memTemp = nullptr;


  switch (i) {
//    case 0: memTemp = &controllerStruct.flexPinky; break; //Y0
//    case 1: memTemp = &controllerStruct.flexIndex; break; //Y1
//    case 2: memTemp = &controllerStruct.flexMid; break; //Y2
//    case 3: memTemp = &controllerStruct.flexPoint; break; //Y3
//    case 4: memTemp = &controllerStruct.flexThumb; break; //Y4
//    case 5: memTemp = &controllerStruct.push1; break; //Y5
//    case 6: memTemp = &controllerStruct.push2; break; //Y6
//    case 7: memTemp = &controllerStruct.push3; break; //Y7
//    case 8: memTemp = &controllerStruct.stickRx; break; //Y8
//    case 9: memTemp = &controllerStruct.stickRy; break; //Y9
//    case 10: memTemp = &controllerStruct.stickPush; break; //Y10
//    case 11: memTemp = &controllerStruct.potent; break; //Y11
//    case 12: memTemp = &controllerStruct.dead1; break; //Y12
//    case 13: memTemp = &controllerStruct.dead2; break; //Y13
//    case 14: memTemp = &controllerStruct.dead3; break; //Y14
//    case 15: memTemp = &controllerStruct.dead4; break; //Y15
//    default:
//      Serial.println("ERROR! Invalid index");
//      return;
    case 0: memTemp = &controllerStruct.flexThumb; break;
    case 1: memTemp = &controllerStruct.flexPoint; break;
    case 2: memTemp = &controllerStruct.flexMid; break;
    case 3: memTemp = &controllerStruct.flexIndex; break;
    case 4: memTemp = &controllerStruct.flexPinky; break;
    case 5: memTemp = &controllerStruct.push1; break;
    case 6: memTemp = &controllerStruct.push2; break;
    case 7: memTemp = &controllerStruct.push3; break;
    case 8: memTemp = &controllerStruct.stickRx; break;
    case 9: memTemp = &controllerStruct.stickRy; break;
    case 10: memTemp = &controllerStruct.stickPush; break;
    case 11: memTemp = &controllerStruct.potent; break;
    case 12: memTemp = &controllerStruct.dead1; break;
    case 13: memTemp = &controllerStruct.dead2; break;
    case 14: memTemp = &controllerStruct.dead3; break;
    case 15: memTemp = &controllerStruct.dead4; break;
    default:
      Serial.println("ERROR! Invalid index");
      return;
  }


  *memTemp = val;
}


void printStructMessageInfo(const struct_message &data) {
    Serial.println("Flex Thumb: " + String(data.flexThumb));
    Serial.println("Flex Point: " + String(data.flexPoint));
    Serial.println("Flex Mid: " + String(data.flexMid));
    Serial.println("Flex Index: " + String(data.flexIndex));
    Serial.println("Flex Pinky: " + String(data.flexPinky));
    Serial.println("Push 1: " + String(data.push1));
    Serial.println("Push 2: " + String(data.push2));
    Serial.println("Push 3: " + String(data.push3));
    Serial.println("Stick Rx: " + String(data.stickRx));
    Serial.println("Stick Ry: " + String(data.stickRy));
    Serial.println("Stick Push: " + String(data.stickPush));
    Serial.println("Potent: " + String(data.potent));
    Serial.println("Dead 1: " + String(data.dead1));
    Serial.println("Dead 2: " + String(data.dead2));
    Serial.println("Dead 3: " + String(data.dead3));
    Serial.println("Dead 4: " + String(data.dead4));
    Serial.println("");
}

void displayFlexValues(struct_message &data) {
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner

  display.print("Thumb: ");
  display.println(String(data.flexThumb));

  display.print("Point: ");
  display.println(String(data.flexPoint));

  display.print("Middle: ");
  display.println(String(data.flexMid));

  display.print("Index: ");
  display.println(String(data.flexIndex));

  display.print("Pinky: ");
  display.println(String(data.flexPinky));
  
  
}

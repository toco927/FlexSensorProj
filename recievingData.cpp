#include <esp_now.h>
#include <WiFi.h>

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

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  
  printStructMessageInfo(myData);
    
  Serial.println();
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {
  esp_now_register_recv_cb(OnDataRecv);

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

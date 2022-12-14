#include <Arduino.h>


#define PANEL_RES_X 96 // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 64 // Number of pixels tall of each INDIVIDUAL panel module.
#define PIN_E 32

#define NUM_ROWS 2 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 1 // Number of INDIVIDUAL PANELS per ROW
#define PANEL_CHAIN NUM_ROWS*NUM_COLS    // total number of panels chained one to another

// Change this to your needs, for details on VirtualPanel pls read the PDF!
#define SERPENT false
#define TOPDOWN false

// library includes
#include <ESP32-VirtualMatrixPanel-I2S-DMA.h>
#include "bmp.h"
#include "ESP32SSDP.h"


// placeholder for the matrix object
MatrixPanel_I2S_DMA *dma_display = nullptr;

// placeholder for the virtual display object
VirtualMatrixPanel  *virtualDisp = nullptr;


#include <IOManager.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

// Clockface
#include <Clockface.h>
// Commons
#include <CWDateTime.h>


// uint16_t myBLACK = dma_display->color565(0, 0, 0);
// uint16_t myWHITE = dma_display->color565(255, 255, 255);
// uint16_t myRED = dma_display->color565(255, 0, 0);
// uint16_t myGREEN = dma_display->color565(0, 255, 0);
// uint16_t myBLUE = dma_display->color565(0, 0, 255);
// uint16_t myYELLOW = dma_display->color565(255, 255, 0);
// uint16_t myCYAN = dma_display->color565(0, 255, 255);
// uint16_t myMAGENTA = dma_display->color565(255, 0, 255);

// uint16_t myCOLORS[8] = {myRED, myGREEN, myBLUE, myWHITE, myYELLOW, myCYAN, myMAGENTA, myBLACK};

Clockface *clockface;

CWDateTime cwDateTime;
IOManager io;

byte displayBright = 32;

AsyncWebServer server(80);
// void notFound(AsyncWebServerRequest *request) {
//     request->send(404, "text/plain", "Not found");
// }


void setupOtaServer()
{
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(200, "text/plain", "Hi! I am ESP32. ota by ip/update");
  // });

  // server.onNotFound(notFound);


  AsyncElegantOTA.begin(&server);    // Start ElegantOTA


  Serial.println("HTTP server started");
}

void displaySetup() 
{

  HUB75_I2S_CFG mxconfig(
                PANEL_RES_X,   // module width
                PANEL_RES_Y,   // module height
                PANEL_CHAIN    // chain length
  );
  mxconfig.gpio.r1 = GPIO_NUM_25;
  mxconfig.gpio.g1 = GPIO_NUM_26;
  mxconfig.gpio.b1 = GPIO_NUM_27;

  mxconfig.gpio.r2 = GPIO_NUM_14;
  mxconfig.gpio.g2 = GPIO_NUM_12;
  mxconfig.gpio.b2 = GPIO_NUM_13;

  mxconfig.gpio.a = GPIO_NUM_23;
  mxconfig.gpio.b = GPIO_NUM_19;
  mxconfig.gpio.c = GPIO_NUM_5;
  mxconfig.gpio.d = GPIO_NUM_17;
  mxconfig.gpio.e = GPIO_NUM_32;

  mxconfig.gpio.clk = GPIO_NUM_16;
  mxconfig.gpio.lat = GPIO_NUM_4;
  mxconfig.gpio.oe = GPIO_NUM_15;

  
  //mxconfig.driver = HUB75_I2S_CFG::FM6126A;     // in case that we use panels based on FM6126A chip, we can set it here before creating MatrixPanel_I2S_DMA object

  // Sanity checks
  if (NUM_ROWS <= 1) {
    Serial.println(F("There is no reason to use the VirtualDisplay class for a single horizontal chain and row!"));
  }

  // OK, now we can create our matrix object
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);

  // let's adjust default brightness to about 75%
  dma_display->setBrightness8(60);    // range is 0-255, 0 - 0%, 255 - 100%

  // Allocate memory and start DMA display
  if( not dma_display->begin() )
      Serial.println("****** !KABOOM! I2S memory allocation failed ***********");

  // create VirtualDisplay object based on our newly created dma_display object
  virtualDisp = new VirtualMatrixPanel((*dma_display), NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, SERPENT, TOPDOWN);

  // virtualDisp->setRotate(true);
  virtualDisp->setRotation(0);
}

//#include "WifiConfig.h"
#include <ESPNtpClient.h>

// #define ONBOARDLED 5 // Built in LED on some ESP-32 boards

#define SHOW_TIME_PERIOD 1000
#define NTP_TIMEOUT 5000

const PROGMEM char* ntpServer = "time1.cloud.tencent.com";
bool wifiFirstConnected = false;

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPEvent_t ntpEvent; // Last triggered event

void onWifiEvent (arduino_event_id_t event, arduino_event_info_t info) {
  Serial.printf ("[WiFi-event] event: %d\n", event);

  switch (event) {
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.printf ("Connected to %s. Asking for IP address.\r\n", info.wifi_sta_connected.ssid);
      break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.printf ("Got IP: %s\r\n", IPAddress (info.got_ip.ip_info.ip.addr).toString ().c_str ());
      Serial.printf ("Connected: %s\r\n", WiFi.status () == WL_CONNECTED ? "yes" : "no");
      // digitalWrite (ONBOARDLED, LOW); // Turn on LED
      wifiFirstConnected = true;
      break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.printf ("Disconnected from SSID: %s\n", info.wifi_sta_disconnected.ssid);
      Serial.printf ("Reason: %d\n", info.wifi_sta_disconnected.reason);
      // digitalWrite (ONBOARDLED, HIGH); // Turn off LED
      //NTP.stop(); // NTP sync can be disabled to avoid sync errors
      WiFi.reconnect ();
      break;

  default:
      break;
  }
}

void processSyncEvent (NTPEvent_t ntpEvent) {
  Serial.printf ("[NTP-event] %s\n", NTP.ntpEvent2str (ntpEvent));

  switch (ntpEvent.event) {
      case timeSyncd:
            clockface->update(true);

          break;
      case partlySync:
      case syncNotNeeded:
      case accuracyError:
          break;
      default:
          break;
  }
}

void setupNtp()
{
  NTP.onNTPSyncEvent ([] (NTPEvent_t event) {
      ntpEvent = event;
      syncEventTriggered = true;
  });


}

void loopNtp() {
  static int i = 0;
  static int last = 0;

  if (wifiFirstConnected) {
      wifiFirstConnected = false;
      NTP.setTimeZone (TZ_Etc_GMTm8);
      NTP.setInterval (600);
      NTP.setNTPTimeout (NTP_TIMEOUT);
      // NTP.setMinSyncAccuracy (5000);
      // NTP.settimeSyncThreshold (3000);
      NTP.begin (ntpServer);
  }

  if (syncEventTriggered) {
      syncEventTriggered = false;
      processSyncEvent (ntpEvent);
  }

  if ((millis () - last) > SHOW_TIME_PERIOD) {
      last = millis ();
      time_t curTime = time(NULL);
      String monStr  = NTP.getTimeDateString(curTime, "%02y年%02m月");
      String dateStr = NTP.getTimeDateString(curTime, "%02d日");
      String timeStr = NTP.getTimeDateString(curTime, "%02H:%02M:%02S");

      Serial.println(dateStr);
      Serial.println(timeStr);

  }
}

void setupSSDP()
{
        Serial.printf("Starting HTTP...\n");
        // server.on("/index.html", HTTP_GET, [&](AsyncWebServerRequest *request) {
        //     request->send(200, "text/plain", "Hello World!");
        // });
        server.on("/description.xml", HTTP_GET, [&](AsyncWebServerRequest *request) {
            request->send(200, "text/xml", SSDP.schema(false));
        });

        //set schema xml url, nees to match http handler
        //"ssdp/schema.xml" if not set
        SSDP.setSchemaURL("description.xml");
        //set port
        //80 if not set
        SSDP.setHTTPPort(80);
        //set device name
        //Null string if not set
        SSDP.setName("Clock");
        //set Serial Number
        //Null string if not set
        SSDP.setSerialNumber("001788102201");
        //set device url
        //Null string if not set
        SSDP.setURL("index.html");
        //set model name
        //Null string if not set
        SSDP.setModelName("Clock");
        //set model description
        //Null string if not set
        SSDP.setModelDescription("This device can be controled by WiFi");
        //set model number
        //Null string if not set
        SSDP.setModelNumber("929000226503");
        //set model url
        //Null string if not set
        SSDP.setModelURL("http://www.meethue.com");
        //set model manufacturer name
        //Null string if not set
        SSDP.setManufacturer("Manufacturer Clock");
        //set model manufacturer url
        //Null string if not set
        SSDP.setManufacturerURL("http://www.....com");
        //set device type
        //"urn:schemas-upnp-org:device:Basic:1" if not set
        SSDP.setDeviceType("rootdevice"); //to appear as root device, other examples: MediaRenderer, MediaServer ...
        //set server name
        //"Arduino/1.0" if not set
        SSDP.setServerName("SSDPServer/1.0");
        //set UUID, you can use https://www.uuidgenerator.net/
        //use 38323636-4558-4dda-9188-cda0e6 + 4 last bytes of mac address if not set
        //use SSDP.setUUID("daa26fa3-d2d4-4072-bc7a-a1b88ab4234a", false); for full UUID
        SSDP.setUUID("daa26fa3-d2d4-4072-bc7a");
        //Set icons list, NB: optional, this is ignored under windows
        SSDP.setIcons(  "<icon>"
                        "<mimetype>image/png</mimetype>"
                        "<height>48</height>"
                        "<width>48</width>"
                        "<depth>24</depth>"
                        "<url>icon48.png</url>"
                        "</icon>");
        //Set service list, NB: optional for simple device
        SSDP.setServices(  "<service>"
                           "<serviceType>urn:schemas-upnp-org:service:SwitchPower:1</serviceType>"
                           "<serviceId>urn:upnp-org:serviceId:SwitchPower:1</serviceId>"
                           "<SCPDURL>/SwitchPower1.xml</SCPDURL>"
                           "<controlURL>/SwitchPower/Control</controlURL>"
                           "<eventSubURL>/SwitchPower/Event</eventSubURL>"
                           "</service>");

        Serial.printf("Starting SSDP...\n");
        SSDP.begin();
}

const char* ssid = "keaixi2";
const char* password = "ily108Gwifi24";

void setupWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.onEvent (onWifiEvent);
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  virtualDisp->setTextColor(virtualDisp->color565(255,231,186));
  virtualDisp->setTextSize(1); 
  virtualDisp->setCursor(6, 10); 
  virtualDisp->print(WiFi.localIP());
  delay(2000);
}


#include <ESPAsyncWiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include "DNSServer.h"

DNSServer dns;

void setupWifiManager() {

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    AsyncWiFiManager wifiManager(&server, &dns);
    //reset saved settings
    //wifiManager.resetSettings();
    //set custom ip for portal
    //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));
    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("Mario Clock", "12345678");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
}

void setup()
{
  Serial.begin(115200);
  Serial.println(""); Serial.println(""); Serial.println("");
  Serial.println("*****************************************************");
  Serial.println(" HELLO !");
  Serial.println("*****************************************************");

  displaySetup();

  clockface = new Clockface(virtualDisp);

  io.clockwiseLogo();
  io.wifiConnecting();

  // setupWifi();
  setupWifiManager();

  io.ntpConnecting();
  cwDateTime.begin();
  
  clockface->setup(&cwDateTime);

  setupNtp();
  setupOtaServer();
  setupSSDP();

  server.begin();

}

uint8_t sta = 0;
int8_t bum_num = 0;

void loopHub75Test() {
  
  for(int i = 0;i<virtualDisp->height();i++){
    virtualDisp->drawFastHLine(0,i,virtualDisp->width(),virtualDisp->color565(map(i,0,128,0,255),map(i,0,128,0,255),map(i,0,128,0,255)));
    delay(10);
  }
  for(int i = 0;i<virtualDisp->height();i++){
    virtualDisp->drawFastHLine(0,i,virtualDisp->width(),virtualDisp->color565(map(i,0,128,0,255),map(i,0,128,255,0),map(i,0,128,0,255)));
    delay(10);
  }
  for(int i = 0;i<virtualDisp->height();i++){
    virtualDisp->drawFastHLine(0,i,virtualDisp->width(),virtualDisp->color565(map(i,0,128,255,0),map(i,0,128,0,255),map(i,0,128,0,255)));
    delay(10);
  }
  for(int i = 0;i<virtualDisp->height();i++){
    virtualDisp->drawFastHLine(0,i,virtualDisp->width(),virtualDisp->color565(map(i,0,128,0,255),map(i,0,128,0,255),map(i,0,128,255,0)));
    delay(10);
  }
  virtualDisp->fillScreen(virtualDisp->color444(0, 0, 0));
  virtualDisp->drawRGBBitmap((virtualDisp->width()-96)/2, (virtualDisp->height()-96)/2, baogai,96,96);
  delay(2000);

  virtualDisp->fillScreen(virtualDisp->color444(0, 0, 0));
  virtualDisp->setTextColor(virtualDisp->color565(255,231,186));
  virtualDisp->setTextSize(1); 
  virtualDisp->setCursor(0,0); 
  // virtualDisp->printlnUTF8("一二三四五");
  // virtualDisp->printlnUTF8("六七八九十");
  // virtualDisp->setTextColor(virtualDisp->color565(0,255,0));
  // virtualDisp->printlnUTF8("锄禾日当午，");
  // virtualDisp->printlnUTF8("汗滴禾下土。");

  // virtualDisp->setTextColor(virtualDisp->color565(255,0,0));
  // virtualDisp->printlnUTF8("爆改车间");
  // virtualDisp->setTextColor(virtualDisp->color565(0,0,186));
  // virtualDisp->printlnUTF8("彩色LED屏幕");
  // virtualDisp->setTextColor(virtualDisp->color565(150,100,60));
  // virtualDisp->printlnUTF8("Arduino");
  // virtualDisp->setTextColor(virtualDisp->color565(60,231,100));
  // virtualDisp->printlnUTF8("ESP32 YYDS");

  delay(2000);
  virtualDisp->fillScreen(virtualDisp->color444(0, 0, 0));
  
  virtualDisp->setTextColor(virtualDisp->color565(255,231,186));
  virtualDisp->setTextSize(1); 
  virtualDisp->setCursor(6, 10); 
  // virtualDisp->printUTF8("2022年06月");
  // virtualDisp->setTextSize(3);
  // virtualDisp->setCursor(15,40);
  // virtualDisp->printUTF8("10");
  // virtualDisp->setTextSize(1); 
  // virtualDisp->setCursor(65,68);
  // virtualDisp->printUTF8("日");
  
  // virtualDisp->setCursor(13,100);
  // virtualDisp->printUTF8("00:00:00");  

  delay(2000);
  virtualDisp->fillScreen(virtualDisp->color444(0, 0, 0));
  for(int i = 0;i<10;i++){
    virtualDisp->drawRGBBitmap(0,0,bmp3,96,128);
    delay(100);
    virtualDisp->drawRGBBitmap(0,0,bmp4,96,128);
    delay(100);
  }
  virtualDisp->fillScreen(virtualDisp->color444(0, 0, 0));
  for (int i = 0; i<10;i++)
  {
    virtualDisp->drawRGBBitmap(13,29,bmp_70_70[bum_num],70,70);
    bum_num++;
    if(bum_num>19)bum_num=0;  
    delay(50);
  }

  // lvgl_loop();
    
} // end loop

void loop()
{
  clockface->update();

  loopNtp();

  // loopHub75Test();
}

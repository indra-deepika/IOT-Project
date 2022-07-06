#include <WiFi.h>
#include "ThingSpeak.h"
#include "HTTPClient.h"
#include "time.h"
#include "WebServer.h"
#include <ArduinoJson.h>

//Wifi
const char *ssid = "AndroidAPB5E3";     // your network SSID (name)
const char *pwd = "12345678"; // your network password
WiFiClient client;
WebServer server(80);

void handleonconnect()
{
  server.send(200,"text/html",SendHTML());
}
int timer=0;
int glow2=0;
const int pled_2_pin=18;

String cse_ip = "192.168.43.175";
String cse_port = "8080";
String servr = "http://" + cse_ip + ":" + cse_port + "/~/in-cse/in-name/";

void createCI(String& val, String& ae, String& cnt)
{
    HTTPClient http;
    http.begin(servr + String(ae) + "/" + String(cnt) + "/");

    http.addHeader("X-M2M-Origin", "admin:admin");
    http.addHeader("Content-Type", "application/json;ty=4");

    int code = http.POST("{\"m2m:cin\": {\"cnf\":\"application/json\",\"con\": " + String(val) + "}}");

    Serial.println(code);

    if (code == -1)
    {
        Serial.println("Unable to connect");
    }

    http.end();
}

//ldr sensor
const int ldr = 5; //output of LDR sensor

//PIR SENSOR
const int pir = 19; //output of PIR sensor
int flag1;          //pir sensor

//Street lights
const int l1 = 25;

// UltraSonic 
int trigger_pin = 21;
int echo_pin = 15;

int trigger_pin2 = 23;
int echo_pin2 = 22;

long time_taken, distnce;
int glow = 0;
String val;
long int duration_led_on=0;
long int energy_saved;

int flag[4];
int sun = 0; //sun is not present

void setup()
{
    timer=millis();
    flag1 = 0;
    Serial.begin(9600);
    Serial.println("Setting up things!!");
    Serial.println("Less go!!!");
//    delay(15000);
    //ldr sensor
    pinMode(ldr, INPUT);

    //pir sensor
    pinMode(pir, INPUT);

    //street lights
    pinMode(l1, OUTPUT);
    pinMode(18, OUTPUT);

    
    // Ultra Sonic
    pinMode(echo_pin, INPUT);
    pinMode(trigger_pin, OUTPUT);
    pinMode(echo_pin2, INPUT);
    pinMode(trigger_pin2, OUTPUT);  
    // Wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid,pwd);
    while(WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Not connected");
        delay(1000);
    }
    Serial.println("WiFi Connected.");   

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    server.on("/",handleonconnect);
    
    server.begin();
}
int st=0;
int fin=0;
int start=millis();
void loop()
{
    server.handleClient();
    int light = digitalRead(ldr);
    
    delay(30); //reading by LDR sensor

    if (light == HIGH) //no light present so led on
    {
      sun=0;
        flag1 = PIR(); //if pir detects some motion don't relay on it till next 3 sec because it takes 3 sec to set output to low
        
        digitalWrite(trigger_pin, HIGH);
        delay(10);
        digitalWrite(trigger_pin, LOW);

        time_taken = pulseIn(echo_pin, HIGH);
        distnce = (time_taken * 0.034)/2;

        Serial.print("Distance in cm = ");
        Serial.println(distnce);

        if(distnce <= 15 || flag1 == 1)
        {
          if(glow == 0)
          {
            digitalWrite(25,HIGH);
            flag[0]=1;
            flag[1]=1;
            glow = 1; 
            st=millis();
            delay(1000);
          }
        }
        else 
        {
          if(glow == 1)
          {
            fin=millis();
            flag[0]=0;
            flag[1]=0;
            timer+=fin-st;
            digitalWrite(25,LOW);
            glow = 0; 
          }
        }
        
        digitalWrite(trigger_pin2, HIGH);
        delay(10);
        digitalWrite(trigger_pin2, LOW);

        time_taken = pulseIn(echo_pin2, HIGH);
        distnce = (time_taken * 0.034)/2;

        Serial.print("Distance in cm = ");
        Serial.println(distnce);

        if(distnce <= 15||flag1 == 1)
        {
          if(glow2 == 0)
          {
            digitalWrite(18,HIGH);
//            Serial.print("yes");
            glow2 = 1; 
            flag[2]=1;
            flag[3]=1;
            delay(1000);
          }
        }
        else 
        {
          if(glow2 == 1)
          {
            digitalWrite(18,LOW);
            flag[2]=0;
            flag[3]=0;
            glow2 = 0; 
          }
        }
        
        delay(1000);
    }
    //presence of light in room
    else
    {
        if (glow == 1|| glow2==1)
        {
            Leds_off();
            if(glow==1){
              fin=millis();
              timer+=fin-st;
            }
            Serial.println("Turned off LEDs");
        }
        glow = 0;
        glow2=0;
        Serial.println("East or west,Sun is the best!!");
        sun=1;
    }
    String val = String(glow);
    String ae1 = "Intensity";
    String cnt1 = "Cnt1"; 
    createCI(val, ae1, cnt1);
    if(millis()-start>120000){
      start=millis();
      duration_led_on=timer;
      energy_saved=(120000-timer)/1200;
      timer=0;
      String dur=String(duration_led_on);
      String ene=String(energy_saved);
      
      String ae2="LED_Time";
      String cnt2="Cnt2";
      String ae3="Energy_Conserved";
      String cnt3="Cnt3";
      createCI(dur, ae2, cnt2);
      createCI(ene, ae3, cnt3);
    }
    delay(1000);
}

int PIR() //motion detector ...using only ultrasonic sensor would not be accuarte
{
    int motion = digitalRead(pir);
    if (motion == HIGH)
    {
        Serial.println("PIR Detected!!");
        delay(50);
        return 1;
    }
    else
    {
        return 0;
    }
}

String SendHTML(void)
{
        String page = "<!DOCTYPE html>\n";
        page += "<html>\n";
        page += "\n";
        page += "<head>\n";
        page += "<meta name='viewport' content='width=device-width, initial-scale=1'>\n";
        page += "<style>\n";
        page += "body {\n";
        page += "margin: 0;\n";
        page += "font-family: Arial, Helvetica, sans-serif;\n";
        page += "}\n";
        page += "\n";
        page += ".topnav {\n";
        page += "overflow: hidden;\n";
        page += "background-color: #333;\n";
        page += "}\n";
        page += "\n";
        page += ".topnav a {\n";
        page += "float: left;\n";
        page += "color: #f2f2f2;\n";
        page += "text-align: center;\n";
        page += "padding: 14px 16px;\n";
        page += "text-decoration: none;\n";
        page += "font-size: 17px;\n";
        page += "}\n";
        page += "\n";
        page += ".topnav a:hover {\n";
        page += "background-color: #ddd;\n";
        page += "color: black;\n";
        page += "}\n";
        page += "\n";
        page += ".topnav a.active {\n";
        page += "background-color: #04AA6D;\n";
        page += "color: white;\n";
        page += "}\n";
        page += "\n";
        page += "#bg {\n";
        page += "position: fixed;\n";
        page += "height: 100%;\n";
        page += "width: 100%;\n";
        page += "z-index: -10;\n";
        page += "background: linear-gradient(180deg, #6e8ea5, #7cadd0, #7cadd0, #7cadd0, #7cadd0, #6e8ea5, #3a6583, #1a4461, #09283d);\n";
        page += "/* background-image:linear-gradient(to bottom, #180c1c 0%, #172630 100%); */\n";
        page += "}\n";
        page += "\n";
        page += ".lamp {\n";
        page += "position: fixed;\n";
        page += "width: 150px;\n";
        page += "bottom: 0;\n";
        page += "z-index: -5;\n";
        page += "}\n";
        page += "\n";
        page += ".moon {\n";
        page += "height: 100px;\n";
        page += "width: 100px;\n";
        page += "border-radius: 50%;\n";
        page += "background: rgb(248, 236, 64);\n";
        page += "margin: auto;\n";
        page += "box-shadow: 0 0 60px rgb(248, 236, 64), 0 0 100px rgb(248, 236, 64), inset 0 5px 12px 26px rgb(248, 236, 64), inset -2px 8px 15px 36px rgb(248, 236, 64);\n";
        page += "transition: 1s;\n";
        page += "transition: 1s;\n";
        page += "right: 100px;\n";
        page += "top: 80px;\n";
        page += "position: absolute;\n";
        page += "animation: sun-moon 40s 2s linear infinite;\n";
        page += "transform-origin: 50% 500px;\n";
        page += "}\n";
        page += "\n";
        page += ".road {\n";
        page += "width: 250%;\n";
        page += "height: 200px;\n";
        page += "background-color: #585858;\n";
        page += "border-top: 10px solid #756D6D;\n";
        page += "border-bottom: 20px solid #756D6D;\n";
        page += "position: absolute;\n";
        page += "bottom: 0%;\n";
        page += "margin-left: -10px;\n";
        page += "padding: 0;\n";
        page += "/* transform:rotate(-10deg); */\n";
        page += "z-index: 3;\n";
        page += "animation: road-moving 10s infinite linear;\n";
        page += "}\n";
        page += "\n";
        page += ".road::before {\n";
        page += "content: ' ';\n";
        page += "position: absolute;\n";
        page += "z-index: 3;\n";
        page += "top: -17px;\n";
        page += "left: 0px;\n";
        page += "right: 0px;\n";
        page += "border: 5px solid black;\n";
        page += "\n";
        page += "\n";
        page += "}\n";
        page += "\n";
        page += ".road-top-half {\n";
        page += "height: 15px;\n";
        page += "width: 250%;\n";
        page += "position: absolute;\n";
        page += "left: -10%;\n";
        page += "top: 30px;\n";
        page += "z-index: 3;\n";
        page += "border-top: 40px dashed white;\n";
        page += "margin-top: 25px;\n";
        page += "animation: road-moving 10s infinite linear;\n";
        page += "transition: all 3s linear;\n";
        page += "/* transform:rotate(-10deg); */\n";
        page += "}\n";
        page += "\n";
        page += "#led1 {\n";
        page += "height: 600px;\n";
        page += "left: calc(50% - 800px);\n";
        page += "}\n";
        page += "\n";
        page += "#led2 {\n";
        page += "height: 600px;\n";
        page += "left: calc(50% - 600px);\n";
        page += "}\n";
        page += "\n";
        page += "#led3 {\n";
        page += "height: 600px;\n";
        page += "left: calc(50% - 400px);\n";
        page += "}\n";
        page += "\n";
        page += "#led4 {\n";
        page += "height: 600px;\n";
        page += "left: calc(50% - 200px);\n";
        page += "}\n";
        page += "\n";
        page += "#led5 {\n";
        page += "height: 600px;\n";
        page += "left: calc(50%);\n";
        page += "}\n";
        page += "\n";
        page += "#led6 {\n";
        page += "height: 600px;\n";
        page += "left: calc(100% - 725px);\n";
        page += "}\n";
        page += "\n";
        page += "#post,\n";
        page += "#curve,\n";
        page += "#socket1,\n";
        page += "#socket2,\n";
        page += "#socket3,\n";
        page += "#socket4,\n";
        page += "#socket5,\n";
        page += "#socket6,\n";
        page += "#light {\n";
        page += "position: absolute;\n";
        page += "}\n";
        page += "\n";
        page += "#post {\n";
        page += "height: 100%;\n";
        page += "width: 10px;\n";
        page += "background: #070707;\n";
        page += "left: 30px;\n";
        page += "border-right: 5px solid #70643e;\n";
        page += "}\n";
        page += "\n";
        page += "#curve {\n";
        page += "overflow: hidden;\n";
        page += "width: 100%;\n";
        page += "left: 0;\n";
        page += "top: 15px;\n";
        page += "height: 100px;\n";
        page += "transform: rotate(-1deg);\n";
        page += "}\n";
        page += "\n";
        page += "#curve:before,\n";
        page += "#curve:after {\n";
        page += "content: '';\n";
        page += "position: absolute;\n";
        page += "height: 100%;\n";
        page += "width: 250px;\n";
        page += "left: -50px;\n";
        page += "border-radius: 50%;\n";
        page += "}\n";
        page += "\n";
        page += "#curve:before {\n";
        page += "top: 0;\n";
        page += "border-top: 10px solid #070707;\n";
        page += "}\n";
        page += "\n";
        page += "#curve:after {\n";
        page += "top: 6px;\n";
        page += "border-top: 5px solid #70643e;\n";
        page += "}\n";
        page += "\n";
        page += "#socket1 {\n";
        page += "width: 15px;\n";
        page += "height: 0;\n";
        page += "border: solid transparent;\n";
        page += "border-width: 20px 10px;\n";
        page += "border-bottom-color: #070707;\n";
        page += "top: 0;\n";
        page += "right: 13px;\n";
        page += "}\n";
        page += "\n";
        page += "#socket1:after {\n";
        page += "content: '';\n";
        page += "position: absolute;\n";
        page += "top: 14px;\n";
        page += "left: -17px;\n";
        page += "height: 50px;\n";
        page += "width: 50px;\n";
        page += "background: #fff39b;\n";
        page += "/*  */\n";
        page += "border-radius: 50%;\n";
        page += "color: #fff39b;\n";
        page += "/* box-shadow:0 0 0px; */\n";
        page += "background: var(--pseudo-background);\n";
        page += "color: var(--pseudo-color);\n";
        page += "box-shadow: var(--pseudo-box-shadow);\n";
        page += "/* -webkit-animation:flicker 10s ease-out infinite;\n";
        page += "animation:flicker 3s ease-out infinite; */\n";
        page += "z-index: -1;\n";
        page += "}\n";
        page += "\n";
        page += "#socket2 {\n";
        page += "width: 15px;\n";
        page += "height: 0;\n";
        page += "border: solid transparent;\n";
        page += "border-width: 20px 10px;\n";
        page += "border-bottom-color: #070707;\n";
        page += "top: 0;\n";
        page += "right: 13px;\n";
        page += "}\n";
        page += "\n";
        page += "#socket2:after {\n";
        page += "content: '';\n";
        page += "position: absolute;\n";
        page += "top: 14px;\n";
        page += "left: -17px;\n";
        page += "height: 50px;\n";
        page += "width: 50px;\n";
        page += "background: #fff39b;\n";
        page += "/*  */\n";
        page += "border-radius: 50%;\n";
        page += "color: #fff39b;\n";
        page += "/* box-shadow:0 0 0px; */\n";
        page += "background: var(--pseudo-background);\n";
        page += "color: var(--pseudo-color);\n";
        page += "box-shadow: var(--pseudo-box-shadow);\n";
        page += "/* -webkit-animation:flicker 10s ease-out infinite;\n";
        page += "animation:flicker 3s ease-out infinite; */\n";
        page += "z-index: -1;\n";
        page += "}\n";
        page += "\n";
        page += "#socket3 {\n";
        page += "width: 15px;\n";
        page += "height: 0;\n";
        page += "border: solid transparent;\n";
        page += "border-width: 20px 10px;\n";
        page += "border-bottom-color: #070707;\n";
        page += "top: 0;\n";
        page += "right: 13px;\n";
        page += "}\n";
        page += "\n";
        page += "#socket3:after {\n";
        page += "content: '';\n";
        page += "position: absolute;\n";
        page += "top: 14px;\n";
        page += "left: -17px;\n";
        page += "height: 50px;\n";
        page += "width: 50px;\n";
        page += "background: #fff39b;\n";
        page += "/*  */\n";
        page += "border-radius: 50%;\n";
        page += "color: #fff39b;\n";
        page += "/* box-shadow:0 0 0px; */\n";
        page += "background: var(--pseudo-background);\n";
        page += "color: var(--pseudo-color);\n";
        page += "box-shadow: var(--pseudo-box-shadow);\n";
        page += "/* -webkit-animation:flicker 10s ease-out infinite;\n";
        page += "animation:flicker 3s ease-out infinite; */\n";
        page += "z-index: -1;\n";
        page += "}\n";
        page += "\n";
        page += "#socket4 {\n";
        page += "width: 15px;\n";
        page += "height: 0;\n";
        page += "border: solid transparent;\n";
        page += "border-width: 20px 10px;\n";
        page += "border-bottom-color: #070707;\n";
        page += "top: 0;\n";
        page += "right: 13px;\n";
        page += "}\n";
        page += "\n";
        page += "#socket4:after {\n";
        page += "content: '';\n";
        page += "position: absolute;\n";
        page += "top: 14px;\n";
        page += "left: -17px;\n";
        page += "height: 50px;\n";
        page += "width: 50px;\n";
        page += "background: #fff39b;\n";
        page += "/*  */\n";
        page += "border-radius: 50%;\n";
        page += "color: #fff39b;\n";
        page += "/* box-shadow:0 0 0px; */\n";
        page += "background: var(--pseudo-background);\n";
        page += "color: var(--pseudo-color);\n";
        page += "box-shadow: var(--pseudo-box-shadow);\n";
        page += "/* -webkit-animation:flicker 10s ease-out infinite;\n";
        page += "animation:flicker 3s ease-out infinite; */\n";
        page += "z-index: -1;\n";
        page += "}\n";
        page += "       \n";
        page += ".gap {\n";
        page += "padding: 5%;\n";
        page += "display: block;\n";
        page += "color: transparent;\n";
        page += "}\n";
        page += "\n";
        page += ":root {\n";
        page += "--pseudo-background: grey;\n";
        page += "--pseudo-box-shadow: white;\n";
        page += "--pseudo-color: white;\n";
        page += "}\n";
        page += "</style>\n";
        page += "</head>\n";
        page += "\n";
        page += "\n";
        page += "<body>\n";
        page += "<div class='topnav'>\n";
        page += "<a class='active' href='./index.html'>Home</a>\n";
        page += "</div>\n";
        page += "\n";
        page += "<div>\n";
        page += "       \n";
        page += "<!-- partial:index.partial.html -->\n";
        page += "<div id='bg'></div>\n";
        page += "<div class='lamp' id='led1'>\n";
        page += "<div id='post'></div>\n";
        page += "<div id='curve'></div>\n";
        page += "<div id='socket1'></div>\n";
        page += "</div>\n";
        page += "<div class='lamp' id='led2'>\n";
        page += "<div id='post'></div>\n";
        page += "<div id='curve'></div>\n";
        page += "<div id='socket2'></div>\n";
        page += "</div>\n";
        page += "<div class='lamp' id='led3'>\n";
        page += "<div id='post'></div>\n";
        page += "<div id='curve'></div>\n";
        page += "<div id='socket3'></div>\n";
        page += "</div>\n";
        page += "\n";
        page += "<div class='lamp' id='led4'>\n";
        page += "<div id='post'></div>\n";
        page += "<div id='curve'></div>\n";
        page += "<div id='socket4'></div>\n";
        page += "</div>\n";
        page += "\n";
        page += "\n";
        page += "<div class='road'>\n";
        page += "<div class='road-top-half'></div>\n";
        page += "<div class='road-bottom-half'></div>\n";
        page += "</div>\n";

        if (sun == 1)
        {
                page += "<div class='moon'></div>\n";
        }
        page += "\n";
        page += "<script>\n";
        page += "myFunction();\n";
        page += "\n";
        page += "function myFunction() {\n";
        page += "                \n";
        if (flag[0] == 1)
        {

                page += "var root = document.getElementById('socket1');\n";
                page += "root.style.setProperty('--pseudo-box-shadow', '0 0 200px white');\n";
                page += "root.style.setProperty('--pseudo-color', 'white');\n";
                page += "root.style.setProperty('--pseudo-background', 'yellow');\n";
                page += "\n";
        }

        if (flag[1] == 1)
        {

                page += "var root = document.getElementById('socket2');\n";
                page += "root.style.setProperty('--pseudo-box-shadow', '0 0 200px white');\n";
                page += "root.style.setProperty('--pseudo-color', 'white');\n";
                page += "root.style.setProperty('--pseudo-background', 'yellow');\n";
                page += "\n";
        }

        if (flag[2] == 1)
        {

                page += "var root = document.getElementById('socket3');\n";
                page += "root.style.setProperty('--pseudo-box-shadow', '0 0 200px white');\n";
                page += "root.style.setProperty('--pseudo-color', 'white');\n";
                page += "root.style.setProperty('--pseudo-background', 'yellow');\n";
                page += "\n";
        }

        if (flag[3] == 1)
        {

                page += "var root = document.getElementById('socket4');\n";
                page += "root.style.setProperty('--pseudo-box-shadow', '0 0 200px white');\n";
                page += "root.style.setProperty('--pseudo-color', 'white');\n";
                page += "root.style.setProperty('--pseudo-background', 'yellow');\n";
                page += "}\n";
        }

        page += "</script>\n";
        page+="<script type='text/javascript' src='https://livejs.com/live.js'></script>\n";
        page += "\n";
        page += "</div>\n";
        page += "\n";
        page += "</body>\n";
        page += "\n";
        page += "</html>\n";
        return page;
}
void Leds_off()
{
  digitalWrite(18, LOW);
  digitalWrite(25,LOW);
  flag[0]=0;
  flag[1]=0;
  flag[2]=0;
  flag[3]=0;
  delay(10);
}

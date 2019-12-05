
// Test za LDR i taster
// const int pinInput = A0; // pin za LDR[ove] i taster za WiFi
// void setup() {
//   pinMode(pinInput, INPUT);
//   Serial.begin(9600);
// }
// void loop() {
//   int input = analogRead(pinInput);
//   Serial.println(input);
//   delay(1000);
// }

#include <WiFiServerBasics.h>
ESP8266WebServer server(80);

// pins
// const byte pinRelay = D3;

#define DEBUG true
#define FPM_SLEEP_MAX_TIME 0xFFFFFFF

// variables
char configFilePath[] = "/config.ini";
int lightOn;            // koliko je sekundi svetlo upaljeno posle poslednjeg signala sa PIR-a
int backlightLimitLow;  // granica pozadinskog osvetljenja iznad koje se svetlo ne pali
int backlightLimitHigh; // granica pozadinskog osvetljenja iznad koje se svetlo ne pali
int wifiOn;             // broj sekundi koje WiFi i veb server ostaju ukljuceni od poslednjeg pritiska na taster

void ReadConfigFile()
{
    File fp = SPIFFS.open(configFilePath, "r");
    if (fp)
    {
        while (fp.available())
        {
            String l = fp.readStringUntil('\n');
            l.trim();
            if (l.length() == 0 || l.charAt(0) == ';') // preskacemo prazne stringove i komentare
                continue;                              //
            int idx = l.indexOf('=');                  // parsiranje reda u formatu "name=value", npr: moment_mins=10
            if (idx == -1)
                break;
            String name = l.substring(0, idx);
            String value = l.substring(idx + 1);

            if (name.equals("lightOn"))
                lightOn = value.toInt();
            if (name.equals("backlightLimitLow"))
                backlightLimitLow = value.toInt();
            if (name.equals("backlightLimitHigh"))
                backlightLimitHigh = value.toInt();
            if (name.equals("wifiOn"))
                wifiOn = value.toInt();
        }
        fp.close();
    }
    else
        Serial.println("config.ini open (r) faaail.");
    if (DEBUG)
    {
        Serial.println(lightOn);
        Serial.println(backlightLimitLow);
        Serial.println(backlightLimitHigh);
        Serial.println(wifiOn);
    }
}

void WriteParamToFile(File &fp, const char *pname)
{
    fp.print(pname);
    fp.print('=');
    fp.println(server.arg(pname));
}

void HandleSaveConfig()
{
    // lightOn=6&backlightLimitLow=200&backlightLimitHigh=400&wifiOn=2200
    File fp = SPIFFS.open(configFilePath, "w");
    if (fp)
    {
        WriteParamToFile(fp, "lightOn");
        WriteParamToFile(fp, "backlightLimitLow");
        WriteParamToFile(fp, "backlightLimitHigh");
        WriteParamToFile(fp, "wifiOn");
        fp.close();
        ReadConfigFile();
    }
    else
        Serial.println("config.ini open (w) faaail.");

    server.send(200, "text/plain", "");
}

void setup()
{
    // pinMode(pinRelay, OUTPUT);
    // pinMode(pinLed, OUTPUT);
    // digitalWrite(pinRelay, false);
    // digitalWrite(pinLed, true);

    Serial.begin(115200);
    ConnectToWiFi();
    SetupIPAddress(40);

    // server.on("/inc/favicon.ico", []() { HandleDataFile(server, "/inc/favicon.ico", "image/x-icon"); });
    server.on("/", []() { HandleDataFile(server, "/index.html", "text/html"); });
    server.on("/inc/script.js", []() { HandleDataFile(server, "/inc/script.js", "text/javascript"); });
    server.on("/inc/style.css", []() { HandleDataFile(server, "/inc/style.css", "text/css"); });
    server.on("/config.ini", []() { HandleDataFile(server, "/config.ini", "text/x-csv"); });
    server.on("/save_config", HandleSaveConfig);
    server.begin();
    if (DEBUG)
        Serial.println("HTTP server started");
    SPIFFS.begin();
    // ReadConfigFile();
}

void WiFiOn()
{
    // digitalWrite(pinLed, false);
    wifi_fpm_do_wakeup();
    wifi_fpm_close();

    //Serial.println("Reconnecting");
    wifi_set_opmode(STATION_MODE);
    wifi_station_connect();
}

void WiFiOff()
{
    //TODO  https://github.com/esp8266/Arduino/issues/4082

    // wifi_station_disconnect();
    // wifi_set_opmode(NULL_MODE);
    // wifi_set_sleep_type(MODEM_SLEEP_T);
    // wifi_fpm_open();
    // wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
    // digitalWrite(pinLed, true);
}

void loop()
{
    server.handleClient();

    // if (millis() < 30 * 1000)
    //     server.handleClient();
    // else
    // {
    //     Serial.println("Zaustavljanje servera");
    //     server.stop();
    //     delay(2000);
    //     Serial.println("gasim wifi...");
    //     WiFiOff();
    //     while(1)
    //     ;
    // }

    // gasenje WiFi-a x minuta posle paljenja aparata
    // if (millis() > WIFI_ON_INIT && wiFiOn)
    // {
    //     Serial.println("gasim wifi...");
    //     WiFiOff();
    // }
}
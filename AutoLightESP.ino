
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

#define DEBUG false
#define FPM_SLEEP_MAX_TIME 0xFFFFFFF

// variables
char configFilePath[] = "/dat/config.ini";

void ReadConfigFile()
{
    File fp = SPIFFS.open(configFilePath, "r");
    if (fp)
    {
        while (fp.available())
        {
            // String l = fp.readStringUntil('\n');
            // l.trim();
            // if (l.length() == 0 || l.charAt(0) == ';') // preskacemo prazne stringove i komentare
            //     continue;                              //
            // int idx = l.indexOf('=');                  // parsiranje reda u formatu "name=value", npr: moment_mins=10
            // if (idx == -1)
            //     break;
            // String name = l.substring(0, idx);
            // String value = l.substring(idx + 1);

            // if (name.equals("auto"))
            //     autoOn = value.equals("1");
            // if (autoOn && name.equals("auto_from"))
            //     ParseTime(value, autoStartHour, autoStartMin);
            // if (autoOn && name.equals("auto_to"))
            //     ParseTime(value, autoEndHour, autoEndMin);

            // if (name.equals("moment"))
            //     momentOn = value.equals("1");
            // if (momentOn && name.equals("moment_from"))
            //     ParseTime(value, momentStartHour, momentStartMin);
            // if (momentOn && name.equals("moment_mins"))
            // {
            //     int mins = value.toInt();
            //     momentEndMin = (momentStartMin + mins) % 60;
            //     int itvHours = (momentStartMin + mins) / 60;
            //     momentEndHour = (momentStartHour + itvHours) % 24;
            // }
        }
        fp.close();
    }
    else
        Serial.println("config.ini open (r) faaail.");
    // if (DEBUG)
    // {
    //     Serial.println(autoOn);
    //     Serial.println(autoStartHour);
    //     Serial.println(autoStartMin);
    //     Serial.println(autoEndHour);
    //     Serial.println(autoEndMin);
    //     Serial.println(momentStartHour);
    //     Serial.println(momentStartMin);
    //     Serial.println(momentEndHour);
    //     Serial.println(momentEndMin);
    // }
}

void WriteParamToFile(File &fp, const char *pname)
{
    fp.print(pname);
    fp.print('=');
    fp.println(server.arg(pname));
}

void HandleSaveConfig()
{
    // auto=1&auto_from=06:45&auto_to=07:20&moment=1&moment_from=22:59&moment_mins=5
    File fp = SPIFFS.open(configFilePath, "w");
    if (fp)
    {
        WriteParamToFile(fp, "auto");
        WriteParamToFile(fp, "auto_from");
        WriteParamToFile(fp, "auto_to");
        WriteParamToFile(fp, "moment");
        WriteParamToFile(fp, "moment_from");
        WriteParamToFile(fp, "moment_mins");
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

    server.on("/", []() { HandleDataFile(server, "/index.html", "text/html"); });
    // server.on("/inc/favicon.ico", []() { HandleDataFile(server, "/inc/favicon.ico", "image/x-icon"); });
    // server.on("/inc/script.js", []() { HandleDataFile(server, "/inc/script.js", "text/javascript"); });
    // server.on("/inc/style.css", []() { HandleDataFile(server, "/inc/style.css", "text/css"); });
    // server.on("/dat/config.ini", []() { HandleDataFile(server, "/dat/config.ini", "text/x-csv"); });
    // server.on("/save_config", HandleSaveConfig);
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

    //Serial.println("diconnecting client and wifi");
    //client.disconnect();
    wifi_station_disconnect();
    wifi_set_opmode(NULL_MODE);
    wifi_set_sleep_type(MODEM_SLEEP_T);
    wifi_fpm_open();
    wifi_fpm_do_sleep(FPM_SLEEP_MAX_TIME);
    // digitalWrite(pinLed, true);
}

void loop()
{
    server.handleClient();

    // gasenje WiFi-a x minuta posle paljenja aparata
    // if (millis() > WIFI_ON_INIT && wiFiOn)
    // {
    //     Serial.println("gasim wifi...");
    //     WiFiOff();
    // }
}
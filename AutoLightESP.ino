#include <Arduino.h>
#include <EEPROM.h>
#include <LinkedList.h>

#include <WiFiServerBasics.h>
ESP8266WebServer server(80);

#define DEBUG true

// podaci koji opisuju trenutno stanje aparata
struct Status
{
    int id;         // redni broj zapisa
    int ldr;        // vrednost LDR senzora
    int secFromPIR; // sekundi od poslednjeg PIR signala
    bool isLightOn; // da li je svetlo upaljeno
    int secWiFi;    // koliko dugo radi WiFi/server

    Status() {}

    Status(int _id, int _ldr, int _secFromPIR, bool _isLightOn, int _secWiFi)
    {
        id = _id;
        ldr = _ldr;
        secFromPIR = _secFromPIR;
        isLightOn = _isLightOn;
        secWiFi = _secWiFi;
    }

    String ToString()
    {
        String s;
        s += id;
        s += ';';
        s += ldr;
        s += ';';
        s += secFromPIR;
        s += ';';
        s += isLightOn ? 1 : 0;
        //B
        // s += ';';
        // s += secWiFi;
        return s;
    }
};
LinkedList<Status> statuses = LinkedList<Status>(); // lista statusa aparata
int idStatus = 0;
long msLastStatus;

// pins
// INPUT
const int pinPhotoRes = A0; // pin za LDR i taster za WiFi
const int pinPIR = D0;
// OUTPUT
const int pinLight = D8;  // LED svetlo za osvetljavanje prostorije
const int eepromPos = 88; // pozicija u EEPROMu na kojoj ce se cuvati podatak da li se wifi pali ili ne pri sledecm resetu
const int pinLed = 2;     // ugradjena LED dioda na ESPu - upaljena kada radi wifi

// variables
char configFilePath[] = "/config.ini";
int lightLevel = 255;    // jacina LED svetla (0-255)
bool isLightOn;          // da li je svetlo upaljeno ili ne
int lightOn;             // koliko je sekundi svetlo upaljeno posle poslednjeg signala sa PIR-a
int backlightLimitLow;   // granica pozadinskog osvetljenja iznad koje se svetlo ne pali
int backlightLimitHigh;  // granica pozadinskog osvetljenja ispod koje se svetlo ne gasi
int backlightLimit;      // granica pozadinskog osvetljenja ...
int wifiOn;              // broj sekundi koje WiFi i veb server ostaju ukljuceni od poslednjeg pritiska na taster
bool isServerOn;         // da li wifi i veb server treba da budu ukljuceni
long msBtnStart = -1;    // millis za pocetak pritiska na taster
long msLastServerAction; // millis za poslednju akciju sa veb serverom (pokretanje ili ucitavanje neke stranice)
long msLastPir = -1;     // poslednji put kada je signal sa PIRa bio HIGH
int consPirs = 0;        // broj uzastopnih pinPIR HIGH vrednosti
int i = 0;

void ReadConfigFile()
{
    msLastServerAction = millis();
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
            if (name.equals("lightLevel"))
                lightLevel = value.toInt();
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
    {
        Serial.print(configFilePath);
        Serial.println(" open (r) faaail.");
    }

    backlightLimit = backlightLimitLow;
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
    msLastServerAction = millis();
    File fp = SPIFFS.open(configFilePath, "w");
    if (fp)
    {
        WriteParamToFile(fp, "lightOn");
        WriteParamToFile(fp, "lightLevel");
        WriteParamToFile(fp, "backlightLimitLow");
        WriteParamToFile(fp, "backlightLimitHigh");
        WriteParamToFile(fp, "wifiOn");
        fp.close();
        ReadConfigFile();
    }
    else
    {
        Serial.print(configFilePath);
        Serial.println(" open (w) faaail.");
    }
    server.send(200, "text/plain", "");
}

void HandleTest()
{
    Serial.println("test page started");
    server.send(200, "text/html", "<h1>You are connected</h1>");
    Serial.println("test page ended");
}

void HandleGetStatus()
{
    msLastServerAction = millis();
    if (statuses.size() > 0)
    {
        Status last = statuses.get(statuses.size() - 1);
        server.send(200, "text/x-csv", last.ToString());
    }
    else
        server.send(204, "text/x-csv", "");
}

void SetLight(bool isOn)
{
    isLightOn = isOn;
    analogWrite(pinLight, isOn ? lightLevel : 0);
}

void setup()
{
    pinMode(pinPIR, INPUT);
    pinMode(pinPhotoRes, INPUT);
    pinMode(pinLed, OUTPUT);
    digitalWrite(pinLed, true);
    pinMode(pinLight, OUTPUT);
    SetLight(false);

    Serial.begin(115200);
    SPIFFS.begin();
    ReadConfigFile();

    EEPROM.begin(512);
    isServerOn = EEPROM.read(eepromPos);

    SetLight(isServerOn); // ako se pali WiFi, svetlo je upaljeno
    backlightLimit = isServerOn ? backlightLimitHigh : backlightLimitLow;

    if (isServerOn)
    {
        ConnectToWiFi();
        SetupIPAddress(40);

        server.on("/inc/bathroom_light.png", []() { HandleDataFile(server, "/inc/bathroom_light.png", "image/png"); });
        server.on("/test", HandleTest);
        server.on("/", []() { HandleDataFile(server, "/index.html", "text/html"); });
        server.on("/inc/index.js", []() { HandleDataFile(server, "/inc/index.js", "text/javascript"); });
        server.on("/inc/style.css", []() { HandleDataFile(server, "/inc/style.css", "text/css"); });
        server.on(configFilePath, []() { HandleDataFile(server, configFilePath, "text/x-csv"); });
        server.on("/save_config", HandleSaveConfig);
        server.on("/current_data.html", []() { HandleDataFile(server, "/current_data.html", "text/html"); });
        server.on("/inc/current_data.js", []() { HandleDataFile(server, "/inc/current_data.js", "text/javascript"); });
        server.on("/get_status", HandleGetStatus);
        server.begin();
        msLastStatus = msLastServerAction = millis();
        if (DEBUG)
            Serial.println("HTTP server started");
    }
    else
    {
        WiFi.forceSleepBegin();
        delay(10);                  // give RF section time to shutdown
        system_update_cpu_freq(80); //* ovo mozda ne mora da se stavi
    }
}

void loop()
{
    long ms = millis();
    int valPir = digitalRead(pinPIR);
    consPirs = valPir ? consPirs + 1 : 0;

    if (consPirs > 5) // HIGH na PIR-u se prihvata samo ako je ta vrednost 5 puta zaredom ocitana
        msLastPir = ms;
    int valPhotoRes = analogRead(pinPhotoRes);
    if (valPhotoRes > backlightLimit) // prostorija je dovoljno osvetljena
    {
        SetLight(false);
        backlightLimit = backlightLimitLow;
    }
    else // prostorija nije dovoljno osvetljena
    {
        if (msLastPir != -1 && ms - msLastPir < 1000 * lightOn)
        {
            backlightLimit = backlightLimitHigh;
            SetLight(true);
        }
        else
            SetLight(false);
    }

    if (valPhotoRes > 1000) // ako je pritisnut taster nabudzen sa LDRom
    {
        if (msBtnStart == -1)
            msBtnStart = ms;                            // pamti se pocetak pritiska na taster
        else if (ms - msBtnStart > 1000 && !isServerOn) // ako se taster drzi vise od sekunde
        {
            EEPROM.write(eepromPos, true);
            EEPROM.commit();
            delay(10);
            ESP.restart();
        }
    }
    else
        msBtnStart = -1;

    digitalWrite(pinLed, !isServerOn);
    if (isServerOn)
    {
        if (ms - msLastStatus > 1000)
        {
            int wiFiCountdown = wifiOn - (ms - msLastServerAction) / 1000;
            Status s(idStatus++, valPhotoRes, (ms - msLastPir) / 1000, isLightOn, wiFiCountdown);
            statuses.add(s);
            msLastStatus = ms;
        }

        server.handleClient();
        if (ms - msLastServerAction > 1000 * wifiOn)
        {
            EEPROM.write(eepromPos, false);
            EEPROM.commit();
            delay(10);
            ESP.restart();
        }
    }

    delay(10);
}

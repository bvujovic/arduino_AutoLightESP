
const int pinInput = A0; // pin za LDR[ove] i taster za WiFi

void setup() {
  pinMode(pinInput, INPUT);
  Serial.begin(9600);
  
}

void loop() {
  int input = analogRead(pinInput);
  Serial.println(input);
  delay(1000);

}

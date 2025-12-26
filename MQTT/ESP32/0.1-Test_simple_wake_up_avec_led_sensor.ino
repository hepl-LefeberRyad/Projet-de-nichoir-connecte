const int SensorPin = 13;   // Capteur Pir 5578 BS-612 
const int ledPin = 4;       // LED à contrôler

bool ledActive = false;     
unsigned long startTime = 0;

void setup() {
  Serial.begin(9600);

  pinMode(SensorPin, INPUT);   // Capteur Pir 5578 BS-612 
  pinMode(ledPin, OUTPUT);    // led 
  digitalWrite(ledPin, LOW);
}

void loop() {
  static sensor = LOW;  
  bool currentButtonState = digitalRead(SensorPin);

  // Détection du front montant
  if (currentButtonState == HIGH && lastsensorState == LOW && !ledActive) {
    digitalWrite(ledPin, HIGH);
    ledActive = true;
    startTime = millis();

    Serial.println("LED allumee !");
  }

  // Extinction après 5000 ms
  if (ledActive && millis() - startTime >= 5000) {
    digitalWrite(ledPin, LOW);
    ledActive = false;
sensor = currentButtonState;
}

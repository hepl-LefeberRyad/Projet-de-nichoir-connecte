const int sensorPin = 13;   // Capteur Pir 5578 BS-162 
const int ledPin = 4;       // LED

unsigned long motionStartTime = 0;
bool motionDetected = false;
bool ledActive = false;

void setup() {
  Serial.begin(9600);

  pinMode(sensorPin, INPUT);   //Capteur Pir 5578 BS-162 configurée comme entrée
  pinMode(ledPin, OUTPUT);     //Led configurée comme sortie
  digitalWrite(ledPin, LOW);  //Led initialisé en état low (éteint) 
}

void loop() {
  bool sensorState = digitalRead(sensorPin);

  // Si mouvement détecté (niveau HIGH)
  if (sensorState == HIGH) {
    
    // Démarrage du chrono si ce n'était pas déjà en cours
    if (!motionDetected) {
      motionDetected = true;
      motionStartTime = millis();
      Serial.println("Mouvement detecte, debut des 5 secondes...");
    }

    // Vérifie si 5 secondes de mouvement continu sont passées
    if (!ledActive && millis() - motionStartTime >= 5000) {
      digitalWrite(ledPin, HIGH);
      ledActive = true;
      Serial.println("LED est allumee (Après 5 secondes de mouvement) !");
    }

  } else { 
    // Aucun mouvement : on réinitialise tout
    motionDetected = false;

    if (ledActive) {
      digitalWrite(ledPin, LOW);
      ledActive = false;
      Serial.println("LED s'éteint");
    }
  }
}



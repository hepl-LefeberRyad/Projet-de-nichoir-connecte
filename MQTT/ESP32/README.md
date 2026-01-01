
## Patch hepl-SousaJoao-patch-4#  

**Objectif** 
Ce code met en place un système de double réveil pour la TimerCAM :
- Réveil immédiat via le capteur PIR pour signaler un mouvement
- Réveil périodique via le timer RTC toutes les 60 secondes
Le tout en maintenant une consommation minimale grâce au deep sleep et à un timer permanent synchronisé.

1. Configuration matérielle
- LED externe sur le GPIO4
- Capteur PIR sur le GPIO13
- TimerCAM initialisée au démarrage
- RTC interne utilisé pour mesurer le temps et programmer les réveils

2. Gestion des causes de réveil
Au démarrage, le code identifie la source du réveil :

a) Réveil par PIR (EXT0)
- Le PIR est passé à HIGH
- La LED externe s’allume pendant 10 secondes
- Puis elle s’éteint avant le retour en deep sleep
  Ce comportement simule une action simple lors d’un mouvement détecté.

b) Réveil par Timer RTC
- Le timer interne a atteint l’échéance programmée
- Le code affiche simplement : "Reveil par timer permanent"
  Ce réveil sert uniquement à maintenir la synchronisation du timer.

c) Premier démarrage / reset
- Initialisation du timer permanent
- next_wakeup = now + PERIOD_US
  Le premier réveil périodique est programmé exactement 60 secondes plus tard.

3. Timer RTC permanent 
Le code utilise une variable RTC persistante :
RTC_DATA_ATTR uint64_t next_wakeup;


Elle survit au deep sleep et permet de :
- Garder une échéance fixe dans le temps
- Éviter toute dérive, même après plusieurs réveils PIR
- Recalculer la durée restante avant le prochain réveil
Logique :
- On lit l’heure RTC 
- On calcule le temps restant
- Ce temps restant est utilisé pour programmer le prochain réveil timer

Le réveil périodique reste parfaitement aligné toutes les 60 secondes.

4. Réactivation des sources de réveil
Avant chaque deep sleep :
- Le capteur PIR est activé :
esp_sleep_enable_ext0_wakeup(SENSOR_PIN, 1);
- Timer RTC est activé avec la durée restante :
esp_sleep_enable_timer_wakeup;
Ainsi, l’ESP32 peut être réveillé :
- Instantanément par un mouvement
- Régulièrement par le timer interne
- Sans conflit entre les deux mécanismes

5. Deep sleep
Enfin, le module entre en veille profonde:

esp_deep_sleep_start();

Le loop() n’est jamais exécuté : tout se passe dans setup() à chaque réveil.

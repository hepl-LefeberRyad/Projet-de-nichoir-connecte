## Patch hepl-SousaJoaopatch-11#
**Objectif** L’objectif est de faire fonctionner la TimerCAM en ultra basse consommation en combinant trois fonctions totalement indépendantes : la prise de photo uniquement lorsqu’un mouvement est validé par le capteur PIR, l’envoi périodique de l’état de la batterie via MQTT, et un mode deep sleep optimisé utilisant deux sources de réveil — le PIR pour les photos et le timer RTC pour les transmissions programmées.

1. Matériel et configuration
   
- Capteur PIR sur le pin GPIO13 (EXT0)
- LED externe sur le pin GPIO4
- Initialisation de la TimerCAM
- Initialisation du Wi-Fi
- Initialisation de la communication par MQTT 

2. Réveil par détection d'un mouvement par le capteur PIR  
 Au réveil par le PIR :

 - Vérification du contexte s'il n'y a pas de blocage ou si la fenêtre PIR active
 - Validation du mouvement :
  - Observation du PIR pendant 10 s
  - Le signal doit rester majoritairement HIGH
  - Sinon: retour en deep sleep
 - Limitation : 2 photos maximum par fenêtre de 12h  
  - window_start = début de fenêtre
  - pir_count = nombre de photos prises

3. Capture & envoi photo
   
Si le mouvement est validé :
- Initialisation caméra
  - Capture d'une image
  - Découpage en blocs de 4095 octets
  - Encodage Base64
  - Envoi MQTT des chunks de la photo
  - Message final : "Photo taken"
  - Libération mémoire + coupure Wi-Fi
- Activation d’un blocage PIR de 5 minutes

4. Blocage PIR
   
Après chaque photo :
- Le PIR est ignoré pendant 5 minutes
- Le timer RTC gère automatiquement la fin du blocage
- Objectif : éviter les rafales et réduire la consommation

5. Batterie réveil par TIMER
   
Un réveil RTC indépendant envoie la batterie toutes les 24 h :
- Lecture tension + le pourcentage de la batterie
- Publication MQTT sur battery
- Coupure Wi-Fi
- Retour en deep sleep
Ce réveil fonctionne même si plusieurs réveils PIR ont eu lieu entre-temps.

6. Gestion des timers RTC
   
- Timer batterie :
- Stocké en RTC
- Toujours aligné sur un multiple exact de 24 h
- Fenêtre PIR :
- Durée : 12 h
- À expiration alors pir_count = 0
- Avant chaque deep sleep :
- Calcul du prochain événement :
- Envoi batterie
- Fin blocage capteur PIR
- Fin fenêtre capteur PIR
- Le plus proche détermine la durée du sommeil

7. Mise en veille
   
Avant de dormir :
- Timer RTC toujours actif
- PIR activé uniquement si : Pas de blocage ou si la fenêtre PIR n'est pas saturée.
- Fenêtre PIR non saturée

Réveil après dormir si :
- Réveil instantané sur mouvement valide
- Batterie envoye les données régulièrement
- Ce qui permet une consommation minimale




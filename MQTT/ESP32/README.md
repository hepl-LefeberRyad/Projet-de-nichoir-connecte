## Patch hepl-SousaJoao-patch-5# 
**Objectif** 

Ce firmware combine deux mécanismes de réveil pour la TimerCAM :
     - Réveil PIR pour allumer une LED lors d’un mouvement
     - Réveil périodique RTC toutes les 60 secondes
Le tout avec une fenêtre PIR de 10 minutes limitant le nombre d’activations à 2, et un timer permanent synchronisé pour éviter toute dérive temporelle.

1. Configuration matérielle
     - LED externe sur le pin GPIO4
     - Capteur PIR sur le pin GPIO13 
     - TimerCAM initialisée au démarrage
     - RTC interne utilisé pour mesurer le temps et gérer les fenêtres

2. Gestion du réveil PIR
Lors d’un réveil par le capteur PIR :
     - Vérification de la fenêtre PIR de 10 minutes :
     - Si première activation ou fenêtre expirée: remise à zéro, déboute une nouvelle fenêtre
     - Limitation à 2 déclenchements de la LED.
     - La LED s'allumée pendant 10 secondes
     - Sinon la LED est ignorée quand la limite est atteinte

Ce mécanisme évite les déclenchements répétés et stabilise la consommation.

3. Réveil par Timer RTC
Si le réveil provient du timer interne :
     - Le code affiche simplement :"Reveil par timer permanent"
Ce réveil n’exécute aucune action fonctionnelle :
il sert uniquement à maintenir la synchronisation du timer permanent.

4. Premier démarrage
 Lors d’un reset ou du tout premier boot :
   - Initialisation du timer permanent :
     - Initialisation de la fenêtre PIR
     - Remise à zéro du compteur PIR

5. Timer RTC permanent 
 Le code utilise une variable RTC persistante :

 Elle permet de :
  - Garder une échéance fixe toutes les 60 secondes
  - Recalculer le temps restant avant le prochain réveil
  - Éviter toute dérive, même après plusieurs réveils PIR
 
 Logique :
  -  Si l’heure actuelle dépasse l’échéance alors on programme la suivante
  -  Sinon, on calcule le temps restant
  -  Ce temps est utilisé pour programmer le réveil RTC

6. Activation des sources de réveil
 Avant chaque deep sleep :
  - Le capteur PIR activé
  - Timer RTC est activé avec la durée restante

Ainsi, l’ESP32 peut être réveillé :

  - Instantanément par le PIR
  - Régulièrement par le timer RTC
  - Sans conflit entre les deux mécanismes





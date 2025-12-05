# PIR SENSOR

## Introduction
Ce dossier contient les codes de base relatifs à la gestion du capteur PIR 5578 BS-612 sur un ESP32 5TimerCam.  
Ce code est écrit sur l'environnement Arduino IDE et permet la configuration de base du capteur, ainsi que contrôler les moyens d'activation et désactivation du capteur.

---

## Patch #4
**Objectif :** Test simple du capteur BS-612 qui contrôle l'état d'une led.  

- Configuration des pins du matérielle
  - Le capteur PIR BS-612 est configuré comme une entrée sur le GPIO13.
  - La led est configuré comme sortie sur le pin GPIO4.
- Lecture de l'état du capteur
  - Il y a une surveillance continue de l'état logique du capteur PIR.
  - Lorsqu'il y a une détection d'un front montant, le capteur se met en état HIGH.
- Activation de la LED
  - La led s'allume lors de la détection
  - Grâce au compteur millis(), le programme peut vérifier si la led est allumée depuis 5000ms.
  - Message sur le terminal via serial.println( " LED allumee ! ")
- Gestion de la temporisation
  - Comptage jusqu'à 5000ms avant que la led s'éteint.
  - Utilisation d'un flag (ledActive) pour éviter des déclenchements répétés.
- Coupure automatique de la led
  - La led s'éteint.
  - Message sur le terminal via serial.println( " LED éteint ! ").     



## Patch #5
**Objectif :** Test du capteur BS-612 avec un chronomètre qui contrôle l'état d'une led.  



## Patch #6
**Objectif :** Test simple du capteur BS-612 qui contrôle l'état d'une led.  




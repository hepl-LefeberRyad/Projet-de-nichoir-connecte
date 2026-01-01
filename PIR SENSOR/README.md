# PIR SENSOR

## Introduction
Ce dossier contient les codes de base relatifs à la gestion du capteur PIR 5578 BS-612 sur un ESP32 5TimerCam.  
Ce code est écrit sur l'environnement Arduino IDE et permet la configuration de base du capteur, ainsi que contrôler les moyens d'activation et désactivation du capteur.

---

## Patch hepl-SousaJoao-patch-1#
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
- Gestion de la temporisation
  - Comptage jusqu'à 5000ms avant que la led s'éteint.
  - Utilisation d'un flag (ledActive) pour éviter des déclenchements répétés.
- Coupure automatique de la led
  - La led s'éteint.   



## Patch hepl-SousaJoao-patch-2#
**Objectif :** Détecter un mouvement avec le capteur PIR BS-612 et allumer la led uniquement si le mouvement dure au moins 5 secondes.  

- Configuration des pins du matérielle
  - Le capteur PIR BS-612 est configuré  sur le GPIO13 et la led sur le pin GPIO4.
  - L'initialisation de la led à l'état LOW.
- Lecture de l'état du capteur
  - Il y a une surveillance continue de l'état logique du capteur PIR.
  - Lorsqu'il y a une détection d'un front montant, le capteur se met en état HIGH.
- Démarrage du chronomètre sur détection
  - Quand un mouvement est capté, pour la première fois:
    - motionDetected = true
    - motionStartTime = millis(), il y a un enregistrement du moment du début du mouvement.
- Vérification de la durée du mouvement
  - Si le mouvement continue est égale ou dépasse 5 secondes alors:
    - La led s'allume.
    - Un message est envoyé sur le terminal:
- Réinitialisation si pas de mouvement
  - motionDeected = false
  - si la led est allumée, et il n'y a plus de mouvement continue: la led s'éteint.

         

## Patch hepl-SousaJoao-patch-3#
**Objectif :** Réveil du microcôntroleur grâce au capteur PIR BS-612.  

- Configuration des pins du matérielle
  - Le capteur PIR BS-612 est configuré  sur le GPIO13 et la led sur le pin GPIO4.
  - L'initialisation de la led à l'état LOW.
- Réveil par bouton ou capteur
  - Autorisation du réveil depuis l’état de sommeil profond (esp_sleep_enable_ext0_wakeup)
  - Niveau bas sur le bouton ou le capteur déclenche le réveil
  - Permet de garder l’appareil en veille pour économiser de l’énergie
- Gestion du TimerCAM
  - Initialisation du module TimerCAM (TimerCAM.begin(true))
  - Commande d’allumage de la LED via PWM 
  - LED allumée pendant 10 secondes pour indiquer le réveil

- Mise en veille automatique
  - Après affichage / temporisation, l’appareil peut se mettre en veille
  - Le réveil peut être déclenché par :
  - le capteur PIR
  - le bouton physique
  - La fonction TimerCAM.Power.timerSleep(5) déclenche la mise en veille après 5 secondes

- Utilisation du compteur / temporisation
  - vTaskDelay(pdMS_TO_TICKS(10000)) = pause non bloquante pendant 10 secondes avant de lancer la veille
  - Permet d’avoir un délai avant la mise en veille tout en restant compatible avec le multitâche







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

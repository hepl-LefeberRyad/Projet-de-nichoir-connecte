## Patch hepl-SousaJoao-patch-6#  
**Objectif** 
Gestion PIR limitée, envoi batterie MQTT et timer RTC synchronisé.
Ce code met en ouvre un fonctionnement basse consommation combinant trois mécanismes indépendants:
- Déclenchement par le capteur PIR limité à deux activations dans une fenêtre de dix minutes.
- Envoi périodique de l’état de la batterie via MQTT lors d’un réveil par timer RTC.
- Deep sleep systématique avec double source de réveil : capteur PIR (EXT0) et timer RTC.
L’ensemble garantit une consommation minimale tout en assurant un comportement prévisible et synchronisé.

1. Configuration matérielle et réseau
- LED sur GPIO4
- PIR sur GPIO13 (réveil EXT0)
- TimerCAM initialisée au démarrage
- Wi‑Fi activé uniquement lors des transmissions MQTT
- MQTT configuré avec un broker local et un topic batterie

2. Gestion du réveil PIR
Lors d’un réveil PIR :
- Vérification de la fenêtre de dix minutes ; réinitialisation si expirée
- Si pir_count < 2, l’activation est acceptée : incrément du compteur et LED allumée dix secondes
- Si la limite est atteinte, aucune action n’est effectuée
Ce mécanisme limite les déclenchements et réduit la consommation.

3. Réveil par timer RTC et envoi de la batterie
Lors d’un réveil TIMER :
- Lecture du niveau et de la tension de batterie
- Construction d’un message unique
- Activation du Wi‑Fi, connexion MQTT, publication sur timercam/battery
- Désactivation immédiate du Wi‑Fi
Le Wi‑Fi n’est utilisé que pour l’envoi, ce qui optimise l’énergie.

4. Premier démarrage
En cas de reset :
- Initialisation du timer permanent, prochaine échéance dans une minute
- Initialisation de la fenêtre PIR
- Remise à zéro du compteur PIR

5. Timer RTC permanent et synchronisation
Le timer utilise next_wakeup stocké en RTC pour maintenir une périodicité stable :
- Si l’échéance est dépassée, une nouvelle est programmée
- Sinon, le temps restant est calculé
- Le timer RTC est configuré avec cette durée
Ce procédé évite toute dérive, même après plusieurs réveils PIR.

6. Activation des sources de réveil
Avant chaque deep sleep :
- Activation du réveil PIR (EXT0)
- Activation du réveil TIMER avec la durée restante
L’ESP32 peut ainsi être réveillé soit par un mouvement, soit par le timer périodique.





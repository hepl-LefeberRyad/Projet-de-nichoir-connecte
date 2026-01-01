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



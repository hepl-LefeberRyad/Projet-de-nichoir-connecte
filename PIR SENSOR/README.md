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

         

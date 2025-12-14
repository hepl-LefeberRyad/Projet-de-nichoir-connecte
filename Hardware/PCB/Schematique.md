# Schematique 
La schématique suivante montre le branchement des composants principaux du projet.

<img width="1077" height="783" alt="image" src="https://github.com/user-attachments/assets/b491e7c0-1394-4971-ba21-f8c6e427ae72" />


## MOSFET et LED
La tension provient du pin Nr3 du bus de la Timer Cam. Le pin Nr1 du bus correspond au GPIO 13, utilisé pour le capteur PIR, tandis que le pin Nr2 correspond au GPIO 4. Le GPIO 4 est connecté à la gate du MOSFET, et à la source, on trouve une résistance en série avec la LED.

Lorsqu’une impulsion est envoyée sur le GPIO 4, le MOSFET conduit et la LED est connectée à la masse, ce qui permet son allumage.

Pour cette application, nous avons utilisé un IC avec deux MOSFET intégrés, car le MOSFET idéal n’était plus disponible chez Mouser. Le MOSFET choisi est le BSS138PS, adapté pour être activé directement par un GPIO d’un ESP32.
La résistance R4, en série avec la LED, limite le courant à 50 mA et fait chuter la tension de 3,3 V à 1,3 V, protégeant ainsi la LED. La résistance R1, en série avec la gate du MOSFET, limite le courant de charge instantané de la gate à environ 10 mA, protégeant le GPIO de l’ESP32 tout en assurant une commutation rapide. Ces deux résistances garantissent donc la sécurité et le fonctionnement fiable du circuit.

### Caractéristiques du BSS138PS:
- Courant maximal : 320 mA (suffisant pour notre LED consommant 50 mA max)
- Tension drain-source maximale : 60 V
- Tension gate-source maximale : ±20 V
- Tension seuil gate-source (VGS(th)) : 0,9 – 1,5 V

## Régulateur de tension
Nous avons également utilisé le régulateur ADP150AUJZ-3,3R7, un régulateur à ultra faible bruit garantissant une tension de sortie stable de 3,3 V. Deux condensateurs de découplage de 1 µF sont branchés respectivement à l’entrée et à la sortie du régulateur de tension.

### Caractéristiques principales :
- Tension d’entrée : 2,2 V à 5,5 V

- Courant maximal : 150 mA (largement suffisant pour notre application)

## Résistance au Pin REL du PIR
Le pont diviseur, composé de R3 et R2, permet de régler la sensibilité du capteur. Plus la tension au point de mesure se rapproche de 0 V, plus le capteur est sensible. Dans notre application, nous avons utilisé 100 Ω et 1 kΩ. T1 représente un point de test permettant de mesurer la tension du diviseur.
# PCB
Le PCB a été commandé chez EUROCIRCUIT et présente une taille compacte de 3,3 cm de largeur sur 4,3 cm de longueur, afin de s’insérer facilement dans un petit boîtier, minimisant ainsi l’encombrement dans le nichoir. Il comporte deux couches et intègre à la fois des composants SMD et through-hole.
<p align="center">
 <img width="622" height="809" alt="image" src="https://github.com/user-attachments/assets/8ac78bc6-8626-449d-8ed5-bd7d22a209ec" />
</p>

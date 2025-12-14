# Projet-de-nichoir-connecte
## Introduction
Dans ce répertoire, nous décrivons le projet final du premier quadrimestre pour le cours Smart Cities IoT. Le projet consiste en un nichoir intelligent capable de prendre des photos grâce à un ESP32 équipé d’une caméra intégrée. La caméra est déclenchée par un capteur de mouvement et est accompagnée d’une LED permettant d’éclairer l’intérieur du nichoir.

Les photos sont ensuite envoyées à un broker via le protocole MQTT et enregistrées dans une base de données. Elles sont également accessibles sur un site web.

Le projet intègre également une batterie, et une gestion énergétique a été mise en place afin que le nichoir puisse fonctionner pendant au moins six mois. Le niveau de batterie et la tension sont envoyés au broker une fois par jour.

<img width="1183" height="719" alt="image" src="https://github.com/user-attachments/assets/50ef5b32-4293-4fe4-b116-a86eed5db31e" />

## Matériel utilisés
### Timer cam
Le microcontrôleur principal de ce projet est une Timer Cam, qui contient un ESP32, une caméra ainsi qu’un port pour une batterie. Ce microcontrôleur est idéal pour ce type de projet de nichoir, puisque toutes les fonctionnalités nécessaires sont déjà intégrées sur une seule carte.
https://www.mouser.be/ProductDetail/170-U082

<p align="center">
  <img src="https://github.com/user-attachments/assets/967f4863-b6dd-48a9-821b-edaa71eb7e95" width="300" />
</p>

### Raspberry pi
Le Raspberry Pi, qui agit comme broker et qui contient également la base de données dans laquelle les images seront enregistrées, est un Raspberry Pi 5. Il sert également à créer une page web sur laquelle on pourra consulter les images enregistrées, ainsi que l’heure et la date de prise de chaque photo, et le niveau de tension et de charge de la batterie. 
<p align="center">
  <img src="https://github.com/user-attachments/assets/55dc3dc4-65eb-489c-8dc2-33484f79c826" width="500" />
</p>

### Capteur PIR
Le capteur de mouvement utilisé dans ce projet est un capteur PIR à 6 broches. Il s’agit d’un capteur intelligent qui intègre un circuit numérique et un élément sensible dans un boîtier hermétiquement fermé. Il peut s’interfacer directement avec jusqu’à deux capteurs PIR conventionnels via une entrée différentielle à haute impédance. Le signal PIR est converti en valeur numérique directement sur la puce. Une sortie REL indique lorsque le signal PIR dépasse le seuil sélectionné. L’entrée OEN peut contrôler la sortie REL ou passer par le capteur de lumière. Les paramètres de sensibilité et de durée de détection sont réglés en connectant les entrées correspondantes à des tensions continues (DC). Tout le traitement du signal est effectué numériquement. 
https://www.mouser.be/ProductDetail/485-5578
<p align="center">
  <img src="https://github.com/user-attachments/assets/f324353d-df79-4b09-997a-2de0de0ebd41" width="400" />
</p>


### LED
Puisqu’il n’y a pas de lumière à l’intérieur du nichoir, nous utilisons une LED qui s’active uniquement lorsque la caméra prend une photo.
https://www.mouser.be/ProductDetail/442-BIRHO033ATB

### Régulateur de tension 3,3V
Puisque la tension de la batterie est supérieure à 3,3 V, il est nécessaire d’ajouter un régulateur de tension, car la tension idéale pour le fonctionnement du capteur de mouvement est de 3,3 V.
https://www.mouser.be/ProductDetail/584-ADP150AUJZ-3.3R7

### Mosfet
Puisque le courant consommé par la LED est supérieur à ce que la sortie GPIO de l’ESP32 peut fournir, nous avons dû implémenter un MOSFET. Ce MOSFET agit comme un interrupteur, permettant à la LED de s’allumer uniquement lorsqu’une impulsion est appliquée à sa gate. Un GPIO de l’ESP32 est connecté à cette gate.
https://www.mouser.be/ProductDetail/771-BSS138PS115

### Résistance et condensateurs
Nous avons également dû utiliser deux condensateurs, qui agissent comme des condensateurs de découplage entre l’entrée et la sortie du régulateur de tension. Deux résistances sont utilisées pour définir la sensibilité du capteur PIR, une résistance est placée en série avec la LED pour la protéger, et une autre résistance est utilisée sur la gate du MOSFET.

### Batterie 3,7 2000mAh
La batterie utilisée dans ce projet est une batterie de 3,7 V avec une capacité de 2000 mAh.
https://hobbyking.com/fr_fr/turnigy-2000mah-1s-1c-lipoly-w-2-pin-jst-ph-connector.html?wrh_pdp=7&utm_source=google&utm_medium=cpc&utm_campaign=google_fr_shopping&countrycode=FR&srsltid=AfmBOoojFj7o2tuN5JBXxUOu5qoo4e6CM8EVZX2YjXUI8DsWO8e2N4bkuz4

<p align="center">
  <img src="https://github.com/user-attachments/assets/9d06d7d8-ea0f-4dfd-85fe-56c0cb62f035" width="400" />
</p>


## Logiciel utilisé
Pour programmer l’ESP32, nous avons utilisé Arduino IDE (version 2.3.5), qui simplifie le développement grâce à l’implémentation de nombreuses bibliothèques. Pour le Raspberry Pi, nous avons utilisé le langage Python (version 3.12) pour la gestion du MQTT. Nous avons également utilisé MariaDB et MySQL pour la création des bases de données et le traitement des données. Enfin, pour la création du site web, nous avons utilisé le langage HTML.

## Hardware
Pour assurer le bon fonctionnement du projet, il est indispensable de concevoir un PCB et un boîtier afin de protéger à la fois le PCB, les composants électroniques et la batterie. Nous avons créé le boîtier en le modélisant en 3D sur Fusion 360, puis nous l’avons imprimé. Pour la conception du PCB, nous avons utilisé Altium Designer version 25.8.1. 

<p align="center">
  <img src="https://github.com/user-attachments/assets/26a98f76-8504-4297-9214-edb9369dbbc7" width="300" /><br><br>
  <img src="https://github.com/user-attachments/assets/5b50208a-88f8-44b4-8e1b-91183ba52407" width="300" />
</p>




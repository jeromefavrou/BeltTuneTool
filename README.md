# BeltTuneTool

> Outil portable de mesure de la tension des courroies par analyse fréquentielle, basé sur un ESP32, un microphone et un écran TFT.

![Montage](img/Dans_boitier.jpg)

---

# Sommaire

- [Présentation](#présentation)
- [Principe de fonctionnement](#principe-de-fonctionnement)
- [Matériel](#matériel)
- [Assemblage](#assemblage)
- [Câblage](#câblage)
- [Boîtier](#boîtier)
- [Compilation](#compilation)
- [Utilisation](#utilisation)
- [Menus](#menus)
- [Réglages](#réglages)
- [Mesure d'une courroie](#mesure-dune-courroie)
- [Fichiers Hardware](#fichiers-hardware)
- [Licence](#licence)

---

# Présentation

**BeltTuneTool** est un appareil portable permettant de mesurer la fréquence propre d'une courroie à l'aide d'un microphone et d'une FFT.

À partir de cette fréquence, il devient possible de régler précisément la tension d'une courroie en utilisant la relation :

\[
T = 4 \times \mu \times L^2 \times f^2
\]

avec :

- **T** : tension de la courroie (N)
- **μ** : masse linéique (kg/m)
- **L** : longueur libre (m)
- **f** : fréquence mesurée (Hz)

L'appareil est particulièrement adapté aux :

- imprimantes 3D
- CNC
- machines-outils
- robots
- transmissions par courroie

---

# Principe de fonctionnement

L'utilisateur pince la courroie.

Le microphone enregistre le son.

Une FFT est calculée par l'ESP32 afin d'identifier la fréquence fondamentale.

La fréquence détectée est ensuite utilisée pour calculer automatiquement la tension de la courroie.

---

# Matériel

Le projet est basé sur :

- ESP32
- Microphone
- Écran TFT couleur
- 3 boutons
- PCB dédié
- Boîtier imprimé en 3D

---

# Assemblage

## Ensemble des pièces

![Pièces](img/Tout_desassembler.jpg)

Le boîtier est composé de plusieurs éléments imprimés en 3D accueillant :

- la carte ESP32
- le PCB
- l'écran TFT
- les boutons
- le microphone

---

# Câblage

## Connexions

![Câblage](img/Connection_Fils.jpg)

Le câblage est volontairement simple afin de limiter les parasites sur le microphone.

Le schéma électronique complet est disponible dans le dossier :

```
Hardware/
```

---

# Boîtier

Une fois assemblé :

![Boîtier](img/Dans_boitier.jpg)

Les fichiers suivants sont fournis :

- STEP
- STL
- GCode

Ils sont disponibles dans :

```
3D Models/
```

---

# Compilation

Le projet est développé sous Arduino IDE.

## Bibliothèques nécessaires

Selon votre installation :

- TFT_eSPI
- arduinoFFT
- Preferences / EEPROM
- SPI
- Wire

Sélectionner une carte ESP32 compatible puis compiler le projet.

---

# Utilisation

Au démarrage, l'écran principal apparaît.

L'utilisateur :

1. choisit un preset ou règle les paramètres
2. pince la courroie
3. attend la détection
4. lit la fréquence
5. lit la tension calculée

---

# Menus

## Écran principal

![Main](img/Main_page.jpg)

Cet écran affiche :

- fréquence détectée
- tension calculée
- état de la mesure
- paramètres principaux

C'est l'écran utilisé pendant les mesures.

---

## Début d'acquisition

![Start](img/Aquisition_start.jpg)

L'appareil attend que la courroie soit pincée.

Lorsque le niveau sonore dépasse le seuil défini, l'acquisition démarre automatiquement.

---

## Acquisition

![Acquisition](img/Aquisition_en_cour.jpg)

Pendant cette phase :

- acquisition audio
- calcul FFT
- recherche du pic principal
- calcul de la fréquence
- calcul de la tension

---

## Menu Settings

![Settings](img/Setting_Page.jpg)

Ce menu permet de modifier tous les paramètres de fonctionnement.

---

## Menu Presets

![Preset](img/preset_page.jpg)

Les presets permettent de mémoriser rapidement plusieurs configurations de courroies.

Par exemple :

- GT2 6 mm
- GT2 9 mm
- HTD
- Courroie CNC
- Configuration personnelle

Le changement de preset recharge instantanément les paramètres associés.

---

# Réglages

## Mu

Masse linéique de la courroie.

Exprimée en kg/m.

Cette valeur est utilisée pour calculer la tension.

Plus elle est précise, plus la tension calculée sera exacte.

Des valeurs prédéfinit dans les pré-set sont définit , néamoins le mieux reste de pesé la courroie et de ramené sont poid au metre

---

## L

Longueur libre de la courroie.

Distance entre les deux points fixes.

Elle intervient directement dans le calcul de la tension.

---

## Noise

Lance une mesure du bruit ambiant.

Le niveau mesuré est utilisé afin d'améliorer la détection de la vibration réelle de la courroie.

Cette fonction est particulièrement utile dans un atelier bruyant.

---

## Mode

Choix du mode d'affichage.

---

## dB

Seuil de déclenchement.

Si le signal est inférieur à cette valeur :

- aucune acquisition

Si le signal dépasse cette valeur :

- lancement automatique de la mesure.

---

## H

Nombre d'harmoniques analysées.

Une valeur plus élevée peut améliorer la robustesse de la détection sur certaines courroies.

---

## Min F

Fréquence minimale recherchée.

Permet d'éviter la détection de parasites basse fréquence.

---

## Max F

Fréquence maximale recherchée.

Limite la recherche de la FFT afin d'accélérer les calculs et réduire les faux positifs.

---

## NBiais

Nombre d'échantillons utilisés pour l'estimation du bruit.

Une valeur élevée améliore généralement la stabilité au prix d'un temps d'initialisation plus long.

---

## SAVE

Sauvegarde tous les paramètres dans la mémoire non volatile.

Ils seront automatiquement restaurés au prochain démarrage.

---

# Mesure d'une courroie

1. Sélectionner le preset adapté.
2. Vérifier la longueur libre (**L**).
3. Vérifier la masse linéique (**Mu**).
4. Régler le seuil si nécessaire.
5. Pincer la courroie.
6. Attendre la fin de l'acquisition.
7. Lire la fréquence.
8. Lire la tension calculée.

---

# Fichiers Hardware

Le dossier **Hardware** contient :

- schéma électronique
- PCB
- Gerbers

Le dossier **3D Models** contient :

- fichiers STL
- fichier STEP
- GCode

permettant de reconstruire entièrement l'appareil.

---

# Arborescence

```
BeltTuneTool
│
├── BeltTuneTool/        Code source Arduino
├── Hardware/            PCB + schéma
├── 3D Models/           STL / STEP / GCode
├── img/                 Documentation
└── README.md
```

---

# Licence

Ce projet est distribué sous licence **MIT**.

Voir le fichier `LICENSE`.

---

# Auteur

Développé par **Jérôme Favrou**.

Contributions, améliorations et retours d'expérience sont les bienvenus.

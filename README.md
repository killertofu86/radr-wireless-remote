# RADR - OSSM Wireless Remote
**RADR** (pronounced like "radar") is a user friendly every day sex machine controller for the people. It's designed to simply and easily allow you to control many bluetooth sex toys, without reaching for your cellphone.

It was originally designed to control the [Open Source Sex Machine](https://docs.researchanddesire.com/ossm/guides/getting-started/introduction), but it can control much more!&#x20;

Check out the [user documentation](https://docs.researchanddesire.com/radr/guides/getting-started/introduction)

You can purchase complete remotes and OSSM kits on our shop - [RADR Product Page](https://www.researchanddesire.com/products/radr-ossm-wireless-remote)
<p align="center">
  <img src="https://github.com/user-attachments/assets/875674bb-3a8b-48da-9d4c-cbb0e5cf0982" width="500">
</p>


## **Community**
#### [**KinkyMakers Discord Community**](https://discord.gg/VtZcudpxT6)

![Discord](https://img.shields.io/discord/559409652425687041)
Most OSSM development takes place through ideas and CAD contributions made by enthusiastic discord members, join the discussion or show off your build!


## Features:
* 4hr battery life
* Auto-pairing with your ossm via bluetooth
* 2" color screen for showing all your settings
* Supports Lovense bluetooth toys (roadmap to support every toy on [buttplug.io](https://buttplug.io/): complete list [here](https://iostindex.com/))
* QWIIC connector for modding and expansion
* Quick start-stop button
* 6-DoF gryo for activity detection (and fun future features)
* Battery fuel gauge
* Haptic feedback
* Audio tone generator

## Why we made it

We wanted an open source wireless remote that could control not just the ossm, but all our fun BLE devices!

## Future Roadmap
* Full [IOST device list](https://iostindex.com) support
* DG labs Coyote e-stim support
* Ability to synchronize or orchestrate multiple devices
* Dynamic pattern alteration on-remote



## Private Firmware (`private-main`)

Der [private Update-Workflow](docs/PRIVATE_FIRMWARE_UPDATE_WORKFLOW.md) beschreibt die Integration neuer Upstream-Versionen und die Backups auf GitHub und Codeberg.

### Startbilder ergänzen

Das Display hat 320 × 240 Pixel im Querformat (4:3). Die Bilddaten liegen als RGB565 im Firmware-Flash. Jedes neue Bild braucht deshalb einen neuen Build und einen Flash.

1. Das Bild mit dem vorhandenen Konverter umwandeln. Für Python werden Pillow und NumPy benötigt. Der Konverter skaliert proportional und legt den Header unter `src/images/` ab.

   ```sh
   cd software/firmware
   python src/helpers/image_converter.py /pfad/zum/neuen_bild.jpg
   ```

2. Den erzeugten Header in `src/images/startupImages.cpp` einbinden und am Ende von `pictures` einen Eintrag mit Anzeigename, Pixelarray, Breite, Höhe und einer neuen ID ergänzen. Die IDs von 0 bis 31 werden für die Bildauswahl gespeichert. Bestehende IDs bleiben den bisherigen Bildern zugeordnet. ID 4 ist für das entfernte RANDR-Testbild reserviert.
3. Der Konverter setzt das neue Bild in `src/images/current_build.h` als Standard. Falls der bisherige Standard gelten soll, die bisherige Auswahl in dieser Datei wiederherstellen. Eine am RADR gespeicherte Auswahl hat Vorrang vor dem Standard aus dem Build.
4. Die Firmware im selben Verzeichnis bauen und nach erfolgreichem Build auf den verifizierten RADR flashen. `/dev/ttyUSB0` ist ein Beispiel für den USB-Port.

   ```sh
   pio run -e development -t buildprog
   pio run -e development -t upload --upload-port /dev/ttyUSB0
   ```

5. Am Gerät unter **RADR → Startup Image** das neue Bild auswählen und neu starten. **Shuffle** durchläuft alle Bilder in zufälliger Reihenfolge, **Fixed Order** folgt der Reihenfolge von `pictures`. Beide Modi wechseln das Bild bei jedem Start.

### Möglicher Upstream-PR

Für das Menü einen frischen Feature-Branch von `upstream/staging` erstellen und den PR in `staging` des offiziellen Repositories öffnen. Den Menücode mit einem neutralen, für Upstream geeigneten Standardbild übernehmen. Private Bilddateien und private Commits gehören zu `private-main`. Für den PR gelten die Build- und Hardware-Validierung des offiziellen Repositories. [Repository-Regeln](AGENTS.md)

## Contributing

Fetch `origin`, branch feature work from `origin/staging`, and open the pull request back into `staging`. Reserve direct production hotfixes for branches created from `origin/main`; open those PRs into `main` and begin the PR title with the standalone word `hotfix` (case-insensitive).

Feature and hotfix PRs are squash-merged into one focused commit. Promotions from `staging` to `main` and hotfix synchronization from `main` to `staging` use merge commits so the long-lived branches retain shared ancestry.

CERN Open Hardware Licence Version 2 - Strongly Reciprocal
Contributions to this GitHub repository will assume release under the same license.

<!-- rad-app submodule smoke test; safe to remove -->

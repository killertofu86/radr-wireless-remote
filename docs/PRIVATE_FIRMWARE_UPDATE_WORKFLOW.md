# Private RADR Firmware Update Workflow

Dieser Ablauf hält eigene RADR-Anpassungen von Upstream-Updates getrennt. `private-main` enthält nur getestete Stände; Upstream wird nie direkt darauf gemergt.

## Branches und Remotes

- `private-main`: getesteter privater Hauptstand
- `integrate/upstream-<version>`: temporärer Integrationsbranch
- `upstream`: offizielles RADR-Repository
- `origin`: GitHub-Backup
- `private`: Codeberg-Backup

## Update-Ablauf

1. Arbeitsbaum prüfen. Nur mit sauberem Arbeitsbaum beginnen:

   ```bash
   git switch private-main
   git status --short --branch
   ```

2. Upstream-Referenzen aktualisieren und neue Commits ansehen:

   ```bash
   git fetch upstream
   git log --oneline private-main..upstream/main
   git diff --stat private-main..upstream/main
   ```

3. Einen Integrationsbranch vom getesteten privaten Stand erstellen:

   ```bash
   git switch -c integrate/upstream-<version> private-main
   git merge --no-ff upstream/main -m "Merge upstream/main <version> into integrate/upstream-<version>"
   ```

4. Konflikte gezielt prüfen und auflösen. Die private Bildliste liegt in `software/firmware/src/images/startupImages.cpp`, das Standardbild in `software/firmware/src/images/current_build.h`. Bei Änderungen in `software/firmware/src/services/display.cpp` auch die aktuelle Upstream-Display- und PWM-API übernehmen. [Startbilder ergänzen](../README.md#startbilder-ergänzen)

5. Nach jeder Konfliktauflösung den Merge abschließen und den Baum prüfen:

   ```bash
   git add <aufgeloeste-dateien>
   git commit
   git status --short --branch
   ```

6. Firmware bauen. PlatformIO muss über die auf dukix eingerichtete Umgebung bzw. seinen vollständigen Pfad aufgerufen werden:

   ```bash
   cd software/firmware
   pio run -e development -t buildprog
   ```

   Bei Änderungen an Release-Konfiguration oder Abhängigkeiten auch `production` bauen:

   ```bash
   pio run -e production -t buildprog
   ```

7. Erst nach erfolgreichen Builds flashen und testen: Startbild/Display, Buttons, Encoder, BLE-Verbindung, OSSM-Steuerung sowie Pause/Stop. Bei Änderungen an der Bewegungssteuerung nur mit spannungsfrei geprüfter Mechanik und kontrolliertem, kurzen Testlauf arbeiten.

8. Nach bestandenem Test den Integrationsbranch nach `private-main` übernehmen und beide Backups pushen:

   ```bash
   git switch private-main
   git merge --no-ff integrate/upstream-<version> -m "Merge tested upstream <version> integration"
   git push origin private-main
   git push private private-main
   ```

9. Die Implementierung neuer privater Funktionen beginnt erst auf diesem getesteten Stand. Jede Funktion erhält einen separaten Commit; bei sicherheitsrelevanten Bewegungsfunktionen zuerst Firmware-Befehl und Schutzverhalten definieren, dann die RADR-Bedienung ergänzen.

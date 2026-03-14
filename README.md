# ITP Projekt IoT und ESP-NOW

## Gruppenmitglieder
- Arafa
- Hafiz

## Projektbeschreibung
In diesem Projekt wurden zwei ESP32 als IoT-Station aufgebaut.  
Der erste ESP32 liest die Werte eines Helligkeitssensors aus und sendet die Daten über ESP-NOW an einen zweiten ESP32.  
Der zweite ESP32 empfängt die Daten, visualisiert sie über ein Webinterface und sendet sie zusätzlich an einen Flask-Server, der die Werte in einer MariaDB-Datenbank speichert.

## Varianten
- Variante 1: Wi-Fi Manager
- Variante 5: Datenbank

## Verwendete Komponenten
- 2x ESP32
- Helligkeitssensor
- RGB-LED
- Breadboard
- Jumperkabel

## Funktionsweise
1. Der Sender-ESP32 liest den Helligkeitssensor aus.
2. Die Messwerte werden gemittelt und als Lichtwert + Status (HELL/DUNKEL) verarbeitet.
3. Die Daten werden per ESP-NOW an den Empfänger-ESP32 gesendet.
4. Der Empfänger zeigt die Werte auf einer Webseite an.
5. Zusätzlich werden die Daten per HTTP an einen Flask-Server gesendet.
6. Der Flask-Server speichert die Messwerte in einer MariaDB-Datenbank.

## Webinterface
Das Webinterface ist über die IP-Adresse des Empfänger-ESP32 erreichbar und zeigt:
- Lichtwert
- Status
- Zeitstempel
- IP-Adresse

## API
Die Daten können zusätzlich als JSON über `/api/data` abgerufen werden.

## Datenbank
Die empfangenen Daten werden in der Datenbank `station` in der Tabelle `messdaten` gespeichert.

## Dateien im Projekt
- `code/sender_esp32.ino`
- `code/empfaenger_esp32.ino`
- `server/server.py`

## Fotos und Schaltplan
Siehe Ordner `docs/`.

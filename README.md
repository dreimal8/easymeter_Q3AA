# easymeter_Q3AA

## Stromzähler EasyMeter Q3AA auslesen mit ESP8266

Inspiriert von [Alexander Kabza](http://www.kabza.de/MyHome/SmartMeter/SmartMeter.php)

Der Stromzähler wird über eine IR-LED ausgelesen - die Daten werden im Anschluss per MQTT versendet.
Die Schaltung vom Lesekopf ist als Skizze beigefügt: *lesekopf_schaltbild.jpg*

Ausgelesen wird die Info Schnittstelle, vorne am Zähler links obehalb vom Display.

Ausgelesen werden die Zählerstände für Bezug und Einspeisung, und die momentane Leistung.

Als ESP8266 dient in meinem Fall ein *WeMos D1 mini* clone.

Da der Stromzähler EasyMeter Q3AA leider über keine magnetische befestigungsmöglichkeit verfügt, habe ich eine Schablone mit dem 3D-Drucker gedruckt, die an der richtigen Stelle ein Loch hat um die LED aufzunehmen.

Die Schablone habe ich vorsichtig mit Klebeband am Stromzähler befestigt.
Die STL Datei ist ebenfalls beigefügt.

In meinem Fall handelt es sich um einen Easymeter **Q3AA 3064 V10.09**

In der Anleitung zum Zähler (frei im Internet als PDF erhältlich) wird erklärt, dass man eine PIN Nummer eingeben muss, damit die Werte mit Nachkommastellen ausgelesen werden können. Das war bei meinem Zähler nicht so - ich konnte auch ohne PIN Eingabe von anfang an auf detaillierte Werte zugreifen.

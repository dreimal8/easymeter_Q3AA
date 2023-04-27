# easymeter_Q3AA
Stromzähler Easymeter Q3AA auslesen mit ESP8266

Inspiriert von Alexander Kabza:
http://www.kabza.de/MyHome/SmartMeter/SmartMeter.php

Der Stromzähler wird über eine IR-LED ausgelesen - die Daten werden im Anschluss per MQTT versendet.
Die Schaltung vom Lesekopf ist als Skizze beigefügt: lesekopf.jpg

Als ESP8266 dient in meinem Fall ein Wmos D1 mini clone.

Da der Stromzähler Easymeter Q3AA leider über keine magnetische befestigungsmöglichkeit verfügt, habe ich eine Schablone mit dem 3D-Drucker gedruckt, die an der richtigen Stelle ein Loch hat um die LED aufzunehmen.

Die Schablone habe ich vorsichtig mit Klebeband am Stromzähler befestigt.
Die STL Datei ist ebenfalls beigefügt.

In meinem Fall handelt es sich um einen Easymeter Q3AA 3064 V10.09

Ich musste keine PIN Nummer eingeben - der Zähler hat von Anfang an alle Daten mit nachkommastellen ausgegeben. Das steht in der Anleitung zum Zähler anders - aber bei mit war es so.

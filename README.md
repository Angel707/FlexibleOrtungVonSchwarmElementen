FlexibleOrtungVonSchwarmElementen
=================================

Flexible Ortung von Schwarmelementen im Innenraum auf Basis kostengünstiger Kameras

Die Schwarmforschung ist ein aktuelles Forschungsthema in der Robotik.
Zur Unterstützung dieser Forschung wird ein kostengünstiges, flexibles und
separates
Ortungssystem gesucht,
um die Positionen aller Schwarmelemente im Innenraum zu bestimmen.
In diesem Zusammenhang sind kostengünstige Kameras wie Webcams ideal.
Die Kamera muss hierbei keine Senkrechtaufnahme machen, sondern kann in einem
schrägen Winkel nach unten gerichtet sein. Durch Anschluss dieser Kameras an
einen Rechner
mit graphischer Oberfläche können die Bewegungen der Schwarmelemente verfolgt
werden.

Die vorgestellte Ortung kann hierbei in vier Schritte unterteilt werden:
Zuerst findet die Berechnung der inneren Orientierung statt
(Kamerakalibrierung); 
Danach folgt die separate Berechnung der äußeren Orientierung,
welche für die Transformation von Welt- in Bildkoordinaten und umgekehrt
wichtig ist.
Zu diesem Zeitpunkt müssen die Kameras fest montiert sein.
Der dritte Schritt detektiert und positioniert das Objekt, 
indem der GrabCut-Algorithmus auf eine kleine Fläche angewandt 
und aus der daraus resultierenden Kontur der Mittelpunkt berechnet wird.
Das Schwarmelement wird hierbei auf eine ebene Struktur abtrahiert, 
wodurch ein Fehler entsteht.
Zum Schluss sorgt eine Objektverfolgung dafür, dass die Position aktualisiert
wird.
Dabei werden skalierungsinvariante Merkmale im Objekt gesucht, zu denen
der optische Fluss mittels der pyramiden-basierten Lucas-Kanade-Methode 
berechnet wird.

Das System ist dafür ausgelegt, mehrere Schwarmelemente zu orten. Die
Genauigkeit
der ortung hängt vom Winkel der Kamera und der Höhe des Schwarmelements ab. 
In Testreihen wurde eine Genauigkeit zwischen einem und drei Zentimetern
ermittelt.


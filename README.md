Flexible Ortung von Schwarmelementen im Innenraum auf Basis kostengünstiger Kameras
===================================================================================

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


Flexible Indoor Locating of Schwarm Elements on the basis of low-cost Cameras
=============================================================================

Schwarm research is a current robotic research topic.
In support of this research a cost-saving, flexible and separate locating
system
has to be designed to locate the position of every schwarm element in interior
space.
So cost-saving cameras like webcams are optimal. 
It is not important to use a birds eye view, but the camera should be inclined
downhill.
The motion of schwarm elements can be watched 
after connecting the camera to a computer with a graphic user interface.

The following locating system is structured in four parts:
Initially, the intrinsic camera parameters are calculated throuth a camera
calibration.
After this, the calculation of extrinsic camera parameters is important 
so that a transformation of coordinates between world and screen can be used.
Accordingly the cameras must not move.
The third part detects and locates schwarm elements by using the GrabCut
algorithm
which is only used with a small rectangle around one object. 
Then, the mean point can be computed throuth the contour of this object.
This method implies an error because the object is not planar.
In the last part of the locating system motion has to be analysed 
so that the motion tracking system updates the mean point of an object.
The clue is to determine the optical flow for a sparse set of feature points,
extracted from the schwarm element, using the iterative Lucas-Kanade method
with pyramids.

In conclusion, the locating system can detect more than one schwarm element 
with an accuracy of measurement between one and three centimeters.
The accuracy of measurement depends on the camera's angle and 
the height of the schwarm element.'

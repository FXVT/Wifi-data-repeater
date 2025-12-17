Ce projet est la programmation d’une carte Waveshare ESP32 S3 Touch LCD 5B pour l’utiliser comme petit écran répéteur à installer dans une des cabines de notre voilier.

L’idée est d’avoir en un clin d’œil une vue sur les paramètres essentiels lors de la navigation ou au mouillage sans avoir à quitter la cabine pour lire les autres instruments.

Les données sont récupérées du réseau NMEA2000 par un répéteur Wifi Actisense W2K-1. La carte Waveshare capte l’émission Wifi de ce répéteur et les affiche sur un écran unique. Ainsi la carte n’est pas directement reliée au réseau en ne nécessite donc qu’un câblage simple : une alimentation 12V du bord.

Le tactile est utilisé pour :
-	Passer en mode veille afin de diminuer la luminosité de l’écran en touchant le centre de l’écran.
-	Pour remettre à zéro le vent apparent maximum en touchant la zone « xx.x kts MAX »
-	
La programmation a été faite avec l’IDE Arduino v2.3.6 en C++ et LVGL en vibe coding  avec l’assistance d’Antorpic Claude Sonnet 4.5 Pro.
L’écran de splash et les icones ont été générés par Microsoft Copilot.

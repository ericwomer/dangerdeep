// texts
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef TEXTS_H
#define TEXTS_H

#include <vector>
using namespace std;

class texts {
	static vector<string> txts;

public:
	enum languages { english, german };

	static void set_language(languages l = english);
	static string get(unsigned no);
};


// old code


#ifndef TEXTS_H_USECPP

#define DEFTYPE extern
#define content(a, b)

#define ENGLISH 0
#define GERMAN 1

extern unsigned language;

#endif	// TEXTS_H_USECPP

DEFTYPE const char* TXT_Heading[] content("Heading", "Kurs");
DEFTYPE const char* TXT_Bearing[] content("Bearing", "Peilung");
DEFTYPE const char* TXT_Range[] content("Range", "Entfernung");
DEFTYPE const char* TXT_Speed[] content("Speed", "Geschwindigkeit");
DEFTYPE const char* TXT_Depth[] content("Depth", "Tiefe");
DEFTYPE const char* TXT_Target[] content("Target", "Ziel");
DEFTYPE const char* TXT_Angleonthebow[] content("Angle on the bow", "Winkel vor dem Bug");
DEFTYPE const char* TXT_Warshipengagement[] content("Warship engagement", "Angriff auf ein Kriegsschiff");
DEFTYPE const char* TXT_Convoybattle[] content("Convoy battle", "Geleitzugschlacht");
DEFTYPE const char* TXT_Historicalmission[] content("Historical mission", "Historische Mission");
DEFTYPE const char* TXT_Returntomainmenu[] content("Return to main menu", "Zurueck zum Hauptmenue");
DEFTYPE const char* TXT_Targetbearing[] content("Target\nbearing", "Peilung\nzum Ziel");
DEFTYPE const char* TXT_Targetrange[] content("Target\nrange", "Entfernung\nzum Ziel");
DEFTYPE const char* TXT_Targetspeed[] content("Target\nspeed", "Geschwindig-\nkeit des Zieles");
DEFTYPE const char* TXT_Targetcourse[] content("Target\ncourse", "Kurs des\nZieles");
DEFTYPE const char* TXT_Selectsubtype[] content("Select submarine type: ", "U-Bootstyp waehlen: ");
DEFTYPE const char* TXT_subVIIc[] content("type VIIc", "Typ VIIc");
DEFTYPE const char* TXT_subXXI[] content("type XXI", "Typ XXI");
DEFTYPE const char* TXT_Startmission[] content("Start mission", "Mission starten");
DEFTYPE const char* TXT_Returntopreviousmenu[] content("Return to previous menu", "Zurueck zum vorherigen Menue");
DEFTYPE const char* TXT_Playsinglemission[] content("Play single mission", "Einzelne Mission spielen");
DEFTYPE const char* TXT_Multiplayermenu[] content("Multiplayer menu", "Mehrspieler-Menue");
DEFTYPE const char* TXT_Careermenu[] content("Career menu", "Karriere-Menue");
DEFTYPE const char* TXT_Showvessels[] content("Show vessels", "Fahrzeuge ansehen");
DEFTYPE const char* TXT_Halloffame[] content("Hall of Fame", "Ruhmeshalle");
DEFTYPE const char* TXT_Selectlanguage[] content("Select language: ", "Sprache auswaehlen: ");
DEFTYPE const char* TXT_English[] content("english", "englisch");
DEFTYPE const char* TXT_German[] content("german", "deutsch");
DEFTYPE const char* TXT_Options[] content("Options", "Optionen");
DEFTYPE const char* TXT_Quit[] content("Quit", "Ende");
DEFTYPE const char* TXT_Timescaleup[] content("Time scale up", "Zeit schneller");
DEFTYPE const char* TXT_Timescaledown[] content("Time scale down", "Zeit langsamer");
DEFTYPE const char* TXT_Rudderleft[] content("Rudder left", "Ruder backbord");
DEFTYPE const char* TXT_Rudderright[] content("Rudder right", "Ruder steuerbord");
DEFTYPE const char* TXT_Rudderhardleft[] content("Rudder hard left", "Ruder hart backbord");
DEFTYPE const char* TXT_Rudderhardright[] content("Rudder hard right", "Ruder hart steuerbord");
DEFTYPE const char* TXT_Planesup[] content("Planes up", "Tiefenruder oben");
DEFTYPE const char* TXT_Planesdown[] content("Planes down", "Tiefenruder runter");
DEFTYPE const char* TXT_Surface[] content("Go to surface", "Auftauchen");
DEFTYPE const char* TXT_Periscopedepth[] content("Go to periscope depth", "Periskoptiefe ansteuern");
DEFTYPE const char* TXT_Crashdive[] content("Crash dive!", "Alarmtauchen!");
DEFTYPE const char* TXT_Ruddermidships[] content("Rudder midships", "Ruder mittschiffs");
DEFTYPE const char* TXT_Aheadslow[] content("Engine ahead slow", "Maschine langsame Kraft");
DEFTYPE const char* TXT_Aheadhalf[] content("Engine ahead half", "Maschine halbe Kraft");
DEFTYPE const char* TXT_Aheadfull[] content("Engine ahead full", "Maschine volle Kraft");
DEFTYPE const char* TXT_Aheadflank[] content("Engine ahead flank", "Maschine aeusserste Kraft");
DEFTYPE const char* TXT_Enginestop[] content("Engine stop", "Maschine stop");
DEFTYPE const char* TXT_Enginereverse[] content("Engine reverse", "Maschine rueckwaerts");
DEFTYPE const char* TXT_Torpedofired[] content("Torpedo fired!", "Torpedo abgefeuert!");
DEFTYPE const char* TXT_Newtargetselected[] content("New target selected", "Neues Ziel ausgewaehlt");
DEFTYPE const char* TXT_Notargetindirection[] content("No target in direction!", "Kein Ziel in dieser Richtung!");
DEFTYPE const char* TXT_Gamepaused[] content("Game PAUSED.", "Spiel PAUSE.");
DEFTYPE const char* TXT_Gameunpaused[] content("Game continued.", "Spiel fortgesetzt.");
DEFTYPE const char* TXT_Scopedown[] content("Scope down", "Periskop eingefahren");
DEFTYPE const char* TXT_Scopeup[] content("Scope up", "Periskop ausgefahren");
DEFTYPE const char* TXT_Createnetworkgame[] content("Create network game", "Netzwerkspiel erstellen");
DEFTYPE const char* TXT_Joinnetworkgame[] content("Join network game, Server IP adress: ", "An einem Netzwerkspiel teilnehmen, Server IP-Adresse: ");
DEFTYPE const char* TXT_Enternetworkportnr[] content("Enter network port number: ", "Netzwerk Port-Adresse eingeben: ");
DEFTYPE const char* TXT_Torpedodudrangetooshort[] content("Torpedo dud - range too short", "Torpedoversager - Laufstrecke zu kurz");
DEFTYPE const char* TXT_Torpedodud[] content("Torpedo dud", "Torpedoversager!");
DEFTYPE const char* TXT_Time[] content("Time", "Uhrzeit");
DEFTYPE const char* TXT_Warship[] content("Warship", "Kriegsschiff");
DEFTYPE const char* TXT_Battleship[] content("Battleship", "Schlachtschiff");
DEFTYPE const char* TXT_Battleshipmalaya[] content("Battleship Malaya class", "Schlachtschiff Malaya-Klasse");
DEFTYPE const char* TXT_Escort[] content("Escort", "Eskorte");
DEFTYPE const char* TXT_Destroyer[] content("Destroyer", "Zerstoerer");
DEFTYPE const char* TXT_Destroyertribal[] content("Destroyer Tribal class", "Zerstoerer Tribal-Klasse");
DEFTYPE const char* TXT_Merchantship[] content("Merchant ship", "Handelsschiff");
DEFTYPE const char* TXT_Freighter[] content("Freighter", "Frachter");
DEFTYPE const char* TXT_Mediummerchant[] content("Medium merchant freighter", "Mittlerer Frachter");
DEFTYPE const char* TXT_Troopship[] content("Troopship", "Truppenschiff");
DEFTYPE const char* TXT_Mediumtroopship[] content("Medium troopship", "Mittleres Truppenschiff");
DEFTYPE const char* TXT_Submarine[] content("Submarine", "U-Boot");
DEFTYPE const char* TXT_Electrosubmarine[] content("Electro submarine", "Elektroboot");
DEFTYPE const char* TXT_Submarinexxi[] content("Submarine type XXI", "U-Boot Typ XXI");
DEFTYPE const char* TXT_Dieselsubmarine[] content("Diesel submarine", "Diesel U-Boot");
DEFTYPE const char* TXT_Submarineviic[] content("Submarine type VIIc", "U-Boot Typ VIIc");
DEFTYPE const char* TXT_Showvesselinstructions[] content("Cursor keys to rotate, Page up/down to cycle, Escape to exit", "Pfeiltasten: drehen, Bildtasten: wechseln, ESC: verlassen");
DEFTYPE const char* TXT_Identifiedtargetas[] content("Identified target as: ", "Ziel identifiziert als: ");
DEFTYPE const char* TXT_Notargetselected[] content("No target selected", "Kein Ziel ausgewaehlt");
DEFTYPE const char* TXT_Carrier[] content("Aircraft carrier", "Flugzeugtraeger");
DEFTYPE const char* TXT_Carrierbogue[] content("Carrier Bogue class", "Traeger Bogue-Klasse");
DEFTYPE const char* TXT_Shipsunk[] content("Ship was sunk!", "Schiff versenkt!");
DEFTYPE const char* TXT_Selectconvoysize[] content("Select convoy size: ", "Geleitgroesse waehlen: ");
DEFTYPE const char* TXT_small[] content("small", "klein");
DEFTYPE const char* TXT_medium[] content("medium", "mittel");
DEFTYPE const char* TXT_large[] content("large", "gross");
DEFTYPE const char* TXT_Selectconvoyescort[] content("Select convoy escort: ", "Geleiteskorte waehlen: ");
DEFTYPE const char* TXT_none[] content("none", "keine");
DEFTYPE const char* TXT_Selecttimeofday[] content("Select time of day: ", "Tageszeit waehlen: ");
DEFTYPE const char* TXT_night[] content("night", "nachts");
DEFTYPE const char* TXT_morning[] content("morning", "morgens");
DEFTYPE const char* TXT_midday[] content("midday", "mittags");
DEFTYPE const char* TXT_evening[] content("evening", "abends");
DEFTYPE const char* TXT_SnorkelUp [] content("Snorkel up", "Schnorchel ausgefahren");
DEFTYPE const char* TXT_SnorkelDown [] content("Snorkel down", "Schnorchel eingefahren");
DEFTYPE const char* TXT_SnorkelDepth[] content("Go to snorkel depth", "Schnorcheltiefe ansteuern");
DEFTYPE const char* TXT_TimeCompression[] content("Compression", "Kompression");
DEFTYPE const char* TXT_Tonnage[] content("Tons", "Tonnen");
DEFTYPE const char* TXT_CaptLogbookNoEntry[] content("No entry", "Kein Eintrag");
DEFTYPE const char* TXT_FuelGauge[] content("Diesel", "Diesel");
DEFTYPE const char* TXT_BatteriesGauge[] content("Batteries", "Batterien");

/*
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
DEFTYPE const char* TXT_[] content("", "");
*/

#endif

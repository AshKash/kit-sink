/**
  *         Commmon Internet File System Java API (JCIFS)
  *----------------------------------------------------------------
  *  Copyright (C) 1999  Norbert Hranitzky
  *
  *  This program is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU General Public License as
  *  published by the Free Software Foundation; either version 2 of
  *  the License, or (at your option) any later version.

  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  *  General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
  *
  *  The full copyright text: http://www.gnu.org/copyleft/gpl.html
  *
  *----------------------------------------------------------------
  *  Author: Norbert Hranitzky
  *  Email : norbert.hranitzky@mchp.siemens.de
  *  Web   : http://www.hranitzky.purespace.de
  */


package org.gnu.jcifs.i18n;


import java.util.*;
import java.text.*;


public class Resources_de extends ListResourceBundle {

    static final Object[][] fContent = {
        // AuthDialog
         {"DLG_TITLE"       , "CIFS Anmeldung" }
        ,{"DLG_ACCOUNT"     , "Benutzername" }
        ,{"DLG_PASSWORD"    , "Kennwort" }
        ,{"DLG_OK"          , "Ok" }
        ,{"DLG_CANCEL"      , "Abbruch" }
        ,{"DLG_PROMPT"      , "Anmeldung" }

        // Communication errors
        ,{"CM1"             , "Rechner {0} unbekannt" }
        ,{"CM2"             , "Kein Verbindung zum Server: {0}" }
        ,{"CM3"             , "Der NetBIOS-Name {0} kann nicht aufgel�st werden" }


        // NetBIOS negativ response errors

        ,{"NB128"           , "Der gerufene Name h�rt nicht zu" }         // NR_EC_NOT_LISTINING_CALLED_NAME
        ,{"NB129"           , "Der Rufer ist unbekannt" }         // NR_EC_NOT_LISTINING_CALLING_NAME
        ,{"NB130"           , "Der gerufene Name ist unbekannt" }            // NR_EC_CALLED_NAME_NOT_PRESENT
        ,{"NB131"           , "Betriebsmittelmangel im Server" }             // NR_EC_INSUFFICIENT_RESOURCES
        ,{"NB143"           , "Undefinierter NetBIOS-Fehler" } // NR_EC_UNSPECIFIED_ERROR
        ,{"NB9999"          , "Negative NetBIOS-Antwort: {0}" }

        // Own NetBIOS errors
        ,{"NB500"           , "NetBIOS Schreibfehler" }
        ,{"NB501"           , "NetBIOS Lesefehler" }
        ,{"NB502"           , "'Retargeting' ist nicht implementiert" }
        ,{"NB503"           , "Unerwarteter NetBIOS-Antwort {0}" }
        ,{"NB504"           , "NetBIOS-Verbindung geschlossen" }



        // Name
        ,{"MN1"             , "UNC/URL Syntaxfehler: {0}" }
        
        // WWW
        ,{"WWW1"             , "Lesen vom CifsURLConnection ist nicht erlaubt, wenn doInput=false - rufe setDoInput(true)" }
        ,{"WWW2"             , "Schreiben auf CifsURLConnection ist nicht erlaubt, wenn doOutput=false - rufe setDoOutput(true)" }
        ,{"WWW3"             , "Schreiben ist nach dem Lesen nicht erlaubt" }





        /*------------------------ SMB error codes -----------------------*/

        ,{"U:U"             , "Unexpected SMB error code."}
        // DOS errors
        ,{"1:1"             , "Ung�ltige Funktion"}
        ,{"1:2"             , "Die angegebene Datei wurde im aktuellen Verzeichnis bzw. Pfad nicht gefunden"}
        ,{"1:3"             , "Das angegebene Verzeichnis wurde nicht auf dem Laufwerk gefunden"}
        ,{"1:4"             , "Die maximale Anzahl der Dateien wurde ge�ffnet"}
        ,{"1:5"             , "Sie haben derzeit keinen Zugriff auf diese Datei"}
        ,{"1:6"             , "Ung�ltiger Dateideskriptor"}
        ,{"1:7"             , "Der Speicherkontrollblock wurde zers�rt"}
        ,{"1:8"             , "Es ist nicht gen�gend Arbeitsspeicher auf dem Server verf�gbar"}
        ,{"1:9"             , "Ung�ltige Speicherblockadresse"}
        ,{"1:10"            , "Ung�ltige Umgebung"}
        ,{"1:11"            , "Ung�ltiges Format."}
        ,{"1:12"            , "Ung�ltiger Modus bei �ffnen der Datei"}
        ,{"1:13"            , "Ung�ltige Daten auf dem Server"}
        ,{"1:15"            , "Der angegebene Laufwerkbuchstabe oder das angegebene Ger�t ist ung�ltig"}
        ,{"1:16"            , "Das Verzeichnis kann nicht entfernt werden"}
        ,{"1:17"            , "Die Datei kann nicht auf ein anderes Laufwerk verschoben werden"}
        ,{"1:18"            , "Keine passende Datei mehr gefunden"}
        ,{"1:32"            , "Die angegebene Datei wird von einem anderen Prozess verwendet"}
        ,{"1:33"            , "Die Datei ist bereits von einem anderen Prozess gesperrt"}
        ,{"1:50"            , "Der gew�nschte Vorgang kann nicht auf diesem Computer ausgef�hrt"}
        ,{"1:80"            , "Die Datei oder das Verzeichnis existiert bereits"}
        ,{"1:230"           , "Pipe ist ung�ltig."}
        ,{"1:231"           , "All instances of the requested pipe are busy."}
        ,{"1:232"           , "Pipe close in progress."}
        ,{"1:233"           , "No process on other end of pipe."}
        ,{"1:234"           , "There is more data to be returned."}
        ,{"1:267"           , "Ung�ltiges Verzeichnis im Pfadname"}

        // SRV errors
        ,{"2:1"             , "Ein interner Fehler ist im Server aufgetreten"}
        ,{"2:2"             , "Ung�ltiges Kennwort"}
        ,{"2:4"             , "Der Aufrufer hat nicht die erforderlichen Rechte"}
        ,{"2:5"             , "Der TID ist in der Nachricht ung�ltig"}
        ,{"2:6"             , "Ung�ltiger Netzwerkname in 'Tree connect' Aufruf"}
        ,{"2:7"             , "Ung�ltiges Ger�t"}

        ,{"2:49"            , "Die Druckerwarteschlange ist voll"}
        ,{"2:50"            , "Die Druckerwarteschlange ist voll. Kein Platz mehr"}
        ,{"2:51"            , "EOF on print queue dump."}
        ,{"2:52"            , "Ung�ltiger FID beim Drucken der Datei"}
        ,{"2:64"            , "Der Server kennt das Kommando nicht"}
        ,{"2:65"            , "Interner Fehler im Server"}
        ,{"2:67"            , "Der FID und Pfadname enthalten eine ung�ltige Kombination"}
        ,{"2:69"            , "Die angegebenen Zugriffsrechte sind ung�ltig"}
        ,{"2:71"            , "Ung�ltige Dateiattribute"}
        ,{"2:81"            , "Der Server macht eine Pause"}
        ,{"2:82"            , "Der Server empf�ngt keine Aufrufe"}
        ,{"2:83"            , "Kein Puffer"}
        ,{"2:87"            , "Zu viele Benutzernamen"}
        ,{"2:88"            , "Die Zeit f�r die Operation ist abgelaufen"}
        ,{"2:89"            , "Zur Zeit sind keine Betriebsmittel f�r den Aufruf vorhanden"}
        ,{"2:90"            , "Zu viele UIDs sind in dieser Sitzung aktiv"}
        ,{"2:91"            , "Der UID ist unbekannt in dieser Sitzung"}
        ,{"2:250"           , "Zur Zeit ist der RAW-Modus nicht verf�gbar"}
        ,{"2:251"           , "Zur Zeit ist der RAW-Modus nicht verf�gbar"}
        ,{"2:252"           , "Setze im MPX-Modus fort"}
        ,{"2:65535"         , "Funktion wird nicht unterst�tzt"}

        // HW  errors
        ,{"3:19"            , "Der Datentr�ger ist schreibgesch�tzt"}
        ,{"3:20"            , "Das angegebene Ger�t wurde nicht gefunden"}
        ,{"3:21"            , "Das Laufwerk ist nicht bereit"}
        ,{"3:22"            , "Kommando unbekannt"}
        ,{"3:23"            , "Datenfehler (CRC)."}
        ,{"3:24"            , "Ung�ltige L�nge des Aufrufs"}
        ,{"3:25"            , "Ein bestimmter Bereich oder eine bestimmte Spur auf dem Datentr�ger konnte nicht gefunden werden"}
        ,{"3:26"            , "Der Typ des Datentr�gers unbekannt"}
        ,{"3:27"            , "Der angeforderte Sektor konnte nicht gefunden werden"}
        ,{"3:28"            , "Kein Papier im Drucker"}
        ,{"3:29"            , "Beim Schreibzugriff auf dieses Ger�t hat der Server einen Fehler festgestellt"}
        ,{"3:30"            , "Beim Lesezugriff auf dieses Ger�t hat der Server einen Fehler festgestellt"}
        ,{"3:31"            , "Ein an das System angeschlossenes Ger�t funktioniert nicht"}
        ,{"3:32"            , "Die angegebene Datei wird von einem anderen Prozess verwendet"}
        ,{"3:33"            , "A Lock request conflicted with an existing lock or specified an invalid mode,"}
        ,{"3:34"            , "Falsche Platte"}
        ,{"3:35"            , "Das Programm konnte die angeforderte Datei nicht �ffnen, da das FCBS-Limit (FCB = Dateisteuerblock) erreicht wurde"}
        ,{"3:36"            , "Die maximale Anzahl der Dateien im Puffer f�r den gemeinsamen Zugriff wurde vor�bergehend erreicht"}

        // CMD error
        ,{"255:0"           , "Ung�ltiges SMB-Format"}


        // Protocol error
        ,{"PE1"             , "Der Server unters�tzt die gew�nschten Protokolle nicht"}
        ,{"PE2"             , "Nur das Protokoll NT LM 0.12 ist implementiert. {0} wird nicht unterst�tzt"}
        ,{"PE3"             , "Ung�ltige Antwort vom Server"}



        // CifsFile*Stream
        ,{"FS0"             , "Ung�ltige Marke"}
        ,{"FS1"             , "Date ist geschlossen" }
		,{"FS2"             , "Ung�ltiger Modus: r oder rw erwartet" }



        ,{"SS1"             , "Die Sitzung mit dem Name {0} existiert bereits"}
        ,{"SS2"             , "Die Sitzung {0} existiert nicht"}
        // RemoteAdmin

        ,{"RA2"             , "Die RemoteAdmin-Verbindung  {0} existiert nicht"}

        // Disk

        ,{"DK2"             , "Die Platte {0} existiert nicht"}
        ,{"DK3"             , "Die Syntax des Plattennamens {0} ist ing�ltig"}
        ,{"DK4"             , "Die Datei oder das Verzeichnis  {0} existiert bereits"}
        ,{"DK5"             , "Die Datei {0} existiert nicht"}
        ,{"DK6"             , "Das Verzeichnis {0} existiert nicht"}

        // LM Error codes
        ,{"LM5"             , "Der Benutzer hat nicht gen�gend Privilegien"}
        ,{"LM50"            , "Der Netzwerkaufruf wird nicht unterst�tzt"}

        ,{"LM65"            , "Ihnen fehlen die erforderlichen Zugriffsrechte f�r diese Netzwerkressource"}
        ,{"LM86"            , "Das angegebene Netzwerkkennwort ist falsch"}
        ,{"LM2103"          , "Der Server wurde nicht gefunden"}
        ,{"LM2114"          , "Der Dienst l�uft auf dem Server nicht"}
        ,{"LM2141"          , "Der IPC$ ist nicht konfiguriert"}
        ,{"LM2150"          , "Der Name der Druckerwarteschlange ist ung�ltig"}
        ,{"LM2151"          , "Der angegebene Druckauftrag kann nicht gefunden werden"}
        ,{"LM2160"          , "Der Druckprozess antwortet nicht"}
        ,{"LM2161"          , "Das Drucksystem ist auf dem Server nicht gestartet"}
        ,{"LM2164"          , "Die Operation kann f�r diesen Auftrag ausgef�hrt werden"}
        ,{"LM2123"          , "Der Puffer auf dem Server war zu klein"}
        ,{"LM2221"          , "Der Benutzername wurde nicht gefunden"}
        ,{"LM2243"          , "Das Kennwort kann nicht ge�ndert werden"}
        ,{"LM2246"          , "Das Kennwort ist zu kurz"}


        ,{"LMERROR"         , "Serverfehler: {0}"}


    };

    public Object[][] getContents() {
        return fContent;
    }

}
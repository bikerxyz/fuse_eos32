# fuse_eos32
Is a Linux filesystem driver for the EOS32 filesystem. Implement with FUSE.
# Bauen
Erstelle ein Build Ordner  
mkdir PATH_TO_CMAKE_BUILD_FOLDER
cd PATH_TO_CMAKE_BUILD_FOLDER
cmake PATH_TO_FUSE_EOS32_FOLDER  
cmake --build PATH_TO_BUILD_FOLDER --target fuse_EOS32  

# Abhängigkeiten
Cmake  
Make

# Fuse abhängigkeiten installieren
## Arch
fuse3   
fuse_common
# Root Nutzer aktivieren(Optional)
Der Root Benutzer ist Standardmäßig unter Fuse deaktiviert. Um als Root Nutzer auf ein Fuse Dateisystem muss es aktiviert werden.


Unter dem Pfad "/etc/fuse.conf" muss die Zeile "user_allow_other" hinzugefügt werden. 
Danach muss nur noch im Befehl erwähnt werden dass der Root Nutzer erlaubt ist
# Starten/Ausführen

## Mit Root Nutzer
./fuse_EOS32 PATH_TO_USERID_CONVERTER_FILE PATH_TO_DISK PARTITIONS_NR -f -s -d -o allow_root PFAD_ZUM_MOUNTPOINT

## Ohne Root Nutzer
./fuse_EOS32 PATH_TO_USERID_CONVER_FILE PATH_TO_DISK PARTITIONS_NR -f -s -d PFAD_ZUM_MOUNTPOINT

## Benutzer- u. Gruppenid converter
Die Datei converterAdapter.txt ist bereits ein Beispiel für disk.im für das EOS32 Projekt. Es können mit "#" Kommentare verfasst werden.

## Aushängen
fusermount -u PATH_TO_MOUNTPOINT


set -x
# Ideally we should have DRC but it generates spurious errors.

targetdir=~/Documents/Malenki-Nanoi-Fab

set -e

kikit fab jlcpcb \
	--assembly \
	--schematic ../malenki-nanoi/malenki-nanoi.kicad_sch \
	--no-drc \
	malenki-nanoi-panel*.kicad_pcb $targetdir
	
mv $targetdir/gerbers.zip $targetdir/malenki-nanoi-panel.zip


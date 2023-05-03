set -x
# Ideally we should have DRC but it generates spurious errors.

targetdir=~/Documents/Malenki-HV-Fab

set -e

kikit fab jlcpcb \
	--assembly \
	--schematic ../malenki-hv/malenki-hv.kicad_sch \
	--no-drc \
	malenki-hv-panel*.kicad_pcb $targetdir
	
mv $targetdir/gerbers.zip $targetdir/malenki-hv-panel.zip


set -x
outdir=$HOME/Documents/NotMalenki-Fab
kikit fab pcbway \
	--assembly \
	--schematic notmalenki.kicad_sch \
	--no-drc \
	notmalenki.kicad_pcb $outdir
mv $outdir/gerbers.zip $outdir/notmalenki.zip

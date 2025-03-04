set -x
outdir=$HOME/Documents/Scarab-Fab
kikit fab pcbway \
	--assembly \
	--schematic scarab.kicad_sch \
	--no-drc \
	scarab.kicad_pcb $outdir
mv $outdir/gerbers.zip $outdir/scarab.zip

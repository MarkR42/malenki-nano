set -x
kikit panelize \
    --framing 'frame; width: 4mm; space: 3mm; cuts:v' \
    --layout 'grid; rows: 2; cols: 5; vspace: 3mm; hbackbone: 4mm; hspace:3mm' \
    --tabs 'type: fixed; hcount:0; vcount:1; width: 10mm' \
    --cuts 'mousebites; drill: 0.4mm; spacing: 0.6mm; prolong: 1mm' \
    --source 'tolerance: 4mm' \
    --post 'millradius: 1.5mm' \
    --text 'simple; width:2mm; height:2mm; voffset: 2mm; text: Malenki Nano Integrated Panel' \
    ../malenki-nanoi/malenki-nanoi.kicad_pcb malenki-nanoi-panel10.kicad_pcb
    

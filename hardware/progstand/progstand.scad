$fs = 0.4; // millimetres

board_width = (139.827 - 116.078);
board_height = (91.948 - 66.421);

pogo_offset_x = (130.506 - 116.078); // Left-hand pin
pogo_offset_y = (72.263  - 66.421); // Offset from top of board.

module bevelledsquare(size=[1,1], radius=0.1) {
    // 8-sided polygon
    polygon(points=[
        // Left
        [0, radius ],
        [0, size[1] - radius ],
        // Top
        [radius, size[1]],
        [size[0] - radius, size[1]],
        // Right
        [size[0], size[1] - radius],
        [size[0], radius],
        // Bot
        [size[0] -radius, 0],
        [radius, 0]
        ]);
}

module pogo_pins() {
    for (n = [0:3]) {
        offset = n * 2.54;
        translate([pogo_offset_x + offset, board_height - pogo_offset_y])
            circle(0.8);
    }
}

module pogo_pins_surround() {
    w = 5 * 2.54;
    h = 4.0;
    translate([pogo_offset_x + (1.5 * 2.54), board_height - pogo_offset_y])
        square([w,h], center=true);
}

module board_outline() {
    union() {
        bevelledsquare([board_width, board_height], 1.0);
        
        // Un-bevel the bottom
        square([board_width, board_height - 10.0]);
    }
}

module main() {
    difference() {
        translate([-3, -3, 0])
            cube([board_width + 6.0, board_height + 6.0, 11.0]);
        
        // Cut the board out of a cube
        // Give a small offset, to allow the board a little wiggle room
        translate([0,0,7])
            linear_extrude(height=10.0)
                offset(r=0.2) board_outline();
        // Cut out the inner
        translate([0,0,5])
            linear_extrude(height=10.0, convexity=3)
                difference() {
                    offset(r=-1.25) board_outline();
                    pogo_pins_surround();
                }
        
        // Drill through the pogo pins
        linear_extrude(height=20.0)
            pogo_pins();
        // Drill a big hole in the middle to allow use to poke the board
        // back out of the holder
        linear_extrude(height=20.0)
            translate([board_width/2, board_height/2])
                circle(r=3.0);
    }
    // A piece up the top to allow it to stand upright on the bench
    translate([-6,board_height + 3.0, 0])
        cube([board_width + 12.0, 1.5, 20.0]);
}

main();

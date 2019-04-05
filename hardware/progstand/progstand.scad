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

module pogo_pins(r=0.8) {
    for (n = [0:3]) {
        offset = n * 2.54;
        translate([pogo_offset_x + offset, board_height - pogo_offset_y])
            circle(r);
    }
}
module pogo_pins_surround() {
    hull() {
        pogo_pins(r=1.8);
    }
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
            cube([board_width + 6.0, board_height + 6.0, 7.0]);
        
        // Cut the board out of a cube
        // Give a small offset, to allow the board a little wiggle room
        translate([0,0,2])
            linear_extrude(height=10.0)
                offset(r=0.2) board_outline();
        // Cut out the inner
        translate([0,0,1])
            linear_extrude(height=10.0, convexity=3)
                difference() {
                    offset(r=-1.25) board_outline();
                    pogo_pins_surround();
                }
        
        // Drill through the pogo pins
        linear_extrude(height=20.0)
            pogo_pins(r=1.1);
        // Cut off the corners 
        translate([-10, -10, 0]) 
            cube([10+(board_width/2)-3, 10+(board_height/2)-2, 20]);
        translate([(board_width/2) + 3, -10, 0]) 
            cube([10+(board_width/2)-2, 10+(board_height/2)-2, 20]);
    }
    // Back of pogo pins surround
    translate([0,0,-18])
    linear_extrude(height=19.0, convexity=3)
    difference() {
        pogo_pins_surround();
        pogo_pins();
    }
    
    // A piece up the top to allow it to stand upright on the bench
    translate([-6,board_height + 3.0, 0])
        cube([board_width + 12.0, 1.5, 15.0]);
}

main();

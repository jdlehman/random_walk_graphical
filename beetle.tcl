#Jonathan Lehman
#April 21, 2011

set sound "1.0"
set avgLife "1.0"

#send scale value
proc scaleVal {} {
		global ratio
        puts stdout "v$ratio"
        flush stdout
}

#send sound value
proc soundVal {} {
		global sound
		if {$sound == "0.0"} {.bframe.sound configure -text "Silent"; set sound "1.0"} \
		else {.bframe.sound configure -text "Sound"; set sound "0.0"}
		
        puts stdout "s$sound"
        flush stdout
}

#send avglife value
proc avgLifeVal {} {
		global avgLife
		if {$avgLife == "0.0"} {set avgLife "1.0"} \
		else {set avgLife "0.0"}
		
        puts stdout "a$avgLife"
        flush stdout
}

#clean up graphics when done (prevents broken pipe errors)
proc done {} {
		.bframe.sound configure -state disabled
		.sframe.setratio configure -state disabled
		.sframe.ratio configure -state disabled
}

# Create canvas widget
canvas .c -width 500 -height 500 -bg grey
pack .c -side top

#create box for beetle to move in
.c create rect 100 100 400 400 -fill white -tags theSquare

#create text in top left and top right corners
.c create text 4 4 -anchor nw -text "Beetle Number: 0" -tags beetleNumText
.c create text 499 4 -anchor ne -text "Average Life: 0.0" -tags avgLifeText
.c bind avgLifeText <Button-1> "avgLifeVal"

# frame to hold all of buttons
frame .bframe
pack .bframe -expand 1 -fill x -side bottom

#create buttons
button .bframe.sound -text "Silent" -command "soundVal"			
button .bframe.exit -text "Exit" -command {exit}

#pack buttons
pack .bframe.sound .bframe.exit -side left -expand 1 -fill x

#frame to hold scale and button
frame .sframe
pack .sframe -expand 1 -fill x -side bottom

#button for scrollbar
button .sframe.setratio -text "Time Scale" -command "scaleVal"
pack .sframe.setratio -expand 1 -fill x -side left

#scrollbar 
scale .sframe.ratio -label "Time Scaling:" -length 450 -from 0.1 -to 5.0 \
	-resolution .1 -digits 2 -orient horiz -showvalue 1 -variable ratio
.sframe.ratio set "1.0"
pack .sframe.ratio -side left



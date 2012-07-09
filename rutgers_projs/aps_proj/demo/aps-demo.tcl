#!/usr/bin/wish -f

# from the learn tcl/tk in 24 hrs

global pLeft pRight
array set hTable { Landmark {} Mote_ID {} X {} Y {} Correction {} APS_Status {} } 
# these are the map length and height 1 pixel = 1 inch
global map-region
set map-region {-10 -10 2400 2400}
global currentMote
set currentMote -1

#init the array lists
foreach item [array names hTable] {
    set l {}
    for {set i 0} {$i<256} {incr i} {
	lappend l -1
    }
    array set hTable [list $item $l]
}

# display a dialog box
global getvalueprompt
proc showMsg { prompt } {
    set f [toplevel .prompt -borderwidth 10]
    set w .prompt
    message $f.msg -text $prompt
    set b [frame $f.buttons -bd 10]
    pack $f.msg $f.buttons -side top -fill x

    button $b.ok -text OK -command {set getvalueprompt 1}
    pack $b.ok -side bottom 

    grab $w
    tkwait variable getvalueprompt
    grab release $w
    destroy $w
}

proc ScrolledCanvas { c width height region } {
    canvas $c.canvas -width $width -height $height \
	-scrollregion $region \
	-xscrollcommand [list $c.xscroll set] \
	-yscrollcommand [list $c.yscroll set] \
	-bg brown
    scrollbar $c.xscroll -orient horizontal \
	-command [list $c.canvas xview]
    scrollbar $c.yscroll -orient vertical \
	-command [list $c.canvas yview]
    pack $c.xscroll -side bottom -fill x
    pack $c.yscroll -side right -fill y
    pack $c.canvas -side left -fill both -expand true
    pack $c -side top -fill both -expand true
    return $c.canvas
}

# refresh the canvas
# TODO: must broadcast the query and find out the coords of all motes around
proc refreshCanvas { } {
    global hTable
    global pRight
    global my_canvas
    global currentMote

    # clear the canvas
    set currentMote -1
    showProps $currentMote
    foreach id $hTable(Mote_ID) {
	delMote $id
    }

    # broadcast query, this is the initial query which gets only the coordinates
    qGetCoord -1


}

# sends all types of query to the mote. used to initially stup the mote
# the caller must then call procReps{} to actually get the replies
proc queryMote { moteid } {
    qGetCoord $moteid
    qIsLM $moteid
    qGetCorrection $moteid
    qAPSStatus $moteid
}

# this reads the stdin for ***REPLY*** <*> ***REPLY*** string
# it then calls the appropriate repFunc to set state
proc procReps { } {
    global currentMote

    set moteid -1
    # stdin must be nonblocking for this to work
    fconfigure stdin -blocking 0
    while { [gets stdin Str] != -1} {
	puts stderr  "got $Str"
	if { [regexp {\*\*\*REPLY\*\*\* ([0-9]+) ([a-zA-Z_]+) (.*) \*\*\*REPLY\*\*\*} $Str dummy moteid toq repStr]} {
	    puts stderr  "CALL: [string tolower $toq] $moteid $repStr"
	    setEle Mote_ID $moteid $moteid
	    [string tolower $toq] $moteid $repStr
	    addMote $moteid
	}
    }
    fconfigure stdin -blocking 1
    showProps $currentMote
}

# updates all values for the given moteid - sets global state
# call showProp to actually display these
proc updateMote { moteid } {

    queryMote $moteid
}

# sends the query
proc sendQuery { moteid } {
    global pRight
    global hTable
    global currentMote
    variable qFlag 0

    # scan if current mote is in the table
    set qFlag 0
    if {$currentMote == -1} {
	showMsg "Please choose a Mote"
	return
    }

#    # get X and Y from the entry in inches
    foreach item {X Y} {
	variable m 0
	variable f 0
	variable i 0

	# convert a' b" to a*12+b
	set itemVal ["$pRight.entry$item" get]
	regexp {([0-9]*)' *([0-9]*)\"?}  $itemVal m f i
	set inches [expr $f * 12 + $i]

	if {$inches != [getEle $item $moteid]} {
	    set qFlag 1
	    setEle $item $moteid $inches
	}
    }

#    if {$qFlag == 1} {
#	puts stderr  "sending query"
#	sendQCoord $moteid
#    }

    # if the landmark checkbutton was set, send query
    global cb_Landmark
    set st $cb_Landmark
    if {$st != [getEle Landmark $moteid]} {
	puts stderr  "Landmark: Sending Query"
	setEle Landmark $moteid $st
	qSetLM $moteid
    }

    # if restart cb is checked, send restart
    global cb_Restart_APS
    set st $cb_Restart_APS
    if {$st == 1} {
	puts stderr  "Restart_APS: Sending Query"
	qRerunAPS $moteid
    }

}

####
# all procs starting with q inmplement the query method
# typically, these will write out to stdout in a special format
# redirecting this to aps-query will do the appropriate thing.
# Similarly, all procs starting with rep must be called when a query
# reply is received
#

# sets the landmar, outputs stderr  numbers in feet as a float
proc qSetLM { moteid } {

    set x [expr [getEle X $moteid] / 12]
    set y [expr [getEle Y $moteid] / 12]
    puts "QUERY $moteid SET_LM [getEle Landmark $moteid] $x $y"
}

# sets/unset the LM, reply is assumed to be in feet
proc rep_set_lm { moteid repStr } {
    global hTable
    
    # process the reply string and update state
    if { [scan $repStr "%d %f %f" isLM x y] == 3} {
	set xinches [expr int($x * 12)]
	set yinches [expr int($y * 12)]
	puts stderr  "rep_set_lm $moteid $isLM $x $y $xinches $yinches"
	setEle Landmark $moteid $isLM
	setEle X $moteid $xinches
	setEle Y $moteid $yinches
    }
}

# gets LM status
proc qIsLM { moteid } {
    puts "QUERY $moteid IS_LM"
}

# reply
proc rep_is_lm { moteid repStr } {
    if { [scan $repStr "%d" isLM] == 1 } {
	setEle Landmark $moteid $isLM
    }
}

# this will get the coords of the mote
proc qGetCoord { moteid } {
    puts "QUERY $moteid GET_COORD"
}

# this will get the coords of the mote
proc rep_get_coord { moteid repStr } {

    if { [scan $repStr "%f %f" x y] == 2 } {
	set xinches [expr int($x * 12)]
	set yinches [expr int($y * 12)]
	puts stderr  "REP_GET_COORD $xinches $yinches"
	setEle X $moteid $xinches
	setEle Y $moteid $yinches
    }
}

# sends a query to get all LM entries for moteid
proc qDumpLMS { moteid } {
    puts "QUERY $moteid DUMP_LMS"
}

# sends a query to get all LM entries for moteid
proc rep_dump_lms { moteid repStr } {
    scan $repStr "%d %d" whence nargs
    # fixme: reads only upto first 3 entries
    if { [scan $repStr "%d %d %f %f %f %f %f %f %f %f %f" \
	      whence no_lms fpx1 fpy1 h1 fpx2 fpy2 h2 fpx3 fpy3 h3] == 2+3*nargs} {
	
    }
    
}

# restarts the APS algo on moteid
proc qRerunAPS { moteid } {
    puts "QUERY $moteid RERUN_APS"
}

# restarts the APS algo on moteid
proc rep_rerun_aps { moteid repStr } {
    global pRight
    # uncheck the restart aps
    set item Restart_APS
    "$pRight.cb$item" deselect
}

# gets the correction for moteid
proc qGetCorrection { moteid } {
    puts "QUERY $moteid GET_CORRECTION"
}

# gets the correction for moteid
proc rep_get_correction { moteid repStr } {
    set g [scan $repStr "%f" cf]

    if { $g  == 1} {
	puts stderr  "rep_get_correction %cf"
	setEle Correction $moteid $cf
    }
}

# gets the APS status on moteid
proc qAPSStatus { moteid } {
    puts "QUERY $moteid APS_STATUS"
}

# gets the APS status on moteid
proc rep_aps_status { moteid repStr } {
    if {[scan $repStr "%d" st] == 1} {
	puts stderr  "rep_status $st"
	setEle APS_Status $moteid $st
    }
}

# displays the entries for moteid
proc showProps { moteid } {
    global hTable
    global pRight
    global currentMote
    global my_canvas

    # if nothing is selected, delete stuff
    if {$moteid == -1} {
	foreach item {Mote_ID Correction APS_Status X Y} {
	    set st ["$pRight.entry$item" cget -state]
	    "$pRight.entry$item" configure -state normal
	    
	    "$pRight.entry$item" delete 0 end
	    "$pRight.entry$item" configure -state $st
	}
	$pRight.cbRestart_APS deselect
	$pRight.cbLandmark deselect
	return
    }

    set currentMote $moteid

    # if any of the values are not available, send out queries
    # at this stage, the mote_id and the x,y are definitely
    if {[getEle Landmark $moteid] == -1} { qIsLM $moteid }
    if {[getEle Correction $moteid] == -1} { qGetCorrection $moteid }
    if {[getEle APS_Status $moteid] == -1} { qAPSStatus $moteid }


    # set X ann Y entries
    foreach item {Mote_ID Correction APS_Status} {
	set st ["$pRight.entry$item" cget -state]
	"$pRight.entry$item" configure -state normal
	"$pRight.entry$item" delete 0 end
	"$pRight.entry$item" insert end [getEle $item $moteid]
	"$pRight.entry$item" configure -state $st

    }

    foreach item {X Y} {
	set st ["$pRight.entry$item" cget -state]
	"$pRight.entry$item" configure -state normal

	"$pRight.entry$item" delete 0 end
	set text "[expr [getEle $item $moteid] / 12]' [expr [getEle $item $moteid] % 12]\""
	"$pRight.entry$item" insert end $text 
	"$pRight.entry$item" configure -state $st

    }

    # set landmark cb
    set item Landmark
    set st [getEle $item $moteid]
    if {$st == 0} {
	"$pRight.cb$item" deselect
    } elseif {$st == 1} {
	"$pRight.cb$item" select
    }

}

proc setEle { hKey moteid value } {
    global hTable
    array set hTable [list $hKey [lreplace $hTable($hKey) $moteid $moteid $value]]
}

proc getEle { hKey moteid } {
    global hTable
    return [lindex $hTable($hKey) $moteid]
}

# adds a mote at given x y. also adds appropriate tags to the image object
# due to limitations of tk, the conversion from inches is manual
# 1 inch = 1 pixel
# assumes all state is set
proc addMote { moteid } {
    global my_canvas
    global hTable

    set mtag "mote-$moteid"

    set file "mote.gif"
    if {[getEle Landmark $moteid] != 0} { 
	set file "mote-LM.gif"
    }

    set xinches [getEle X $moteid]
    set yinches [getEle Y $moteid]

    if { $moteid == -1 || $file == -1 || $xinches == -1 || $yinches == -1 } return


    puts stderr  "addmote: $file $xinches $yinches"
    $my_canvas delete "mote-$moteid"
    $my_canvas create image $xinches $yinches \
	-image [image create photo -file $file ] -tag $mtag
    $my_canvas bind $mtag <Button-1> "showProps $moteid"
}

# deletes given mote and all its state
proc delMote { moteid } {
    global my_canvas
    global hTable
    if {$moteid == -1} return

    foreach item [array names hTable] {
	setEle $item $moteid -1
    }

    $my_canvas delete "mote-$moteid"
}


# sets up the fames and canvas
proc mapSetup { } {

    # window manager stuff
    wm title . "APS Demo"
    wm minsize . 300 200
    wm geometry . 640x480


    # frames
    frame .f -bg blue -bd 2;# to hold canvas and scrollbars
    pack .f
    frame .f.sub  -bg green -bd 2 ;# to hold canvas

    # canvas
    global my_canvas
    global map-region
    set my_canvas [ScrolledCanvas .f 100 100 ${map-region}]

    # the property frames
    global pRight pLeft
    frame .prop -bg blue
    pack .prop -fill x
    frame .prop.left
    frame .prop.right
    pack .prop.left .prop.right -side left  -padx 4 -pady 3 -ipadx 2 -ipady 1 -fill y
    set pLeft .prop.left
    set pRight .prop.right

    global hTable
    # add all labels ar htable keys
    foreach item {Mote_ID X Y Correction Landmark APS_Status Restart_APS} {
	label "$pLeft.lab$item" -text $item -justify right
	pack "$pLeft.lab$item" -fill y -expand 1
    }
	
    # add fields to the mote property frame
    set item Mote_ID
    entry "$pRight.entry$item" -state disable
    pack "$pRight.entry$item"
    bind "$pRight.entry$item" <Return> {sendQuery $currentMote}

    foreach item {X Y} {
	entry "$pRight.entry$item" -textvariable "tv_$item" -state disable
	pack "$pRight.entry$item"
    }

    set item Correction
    entry "$pRight.entry$item" -state disable
    pack "$pRight.entry$item"
    bind "$pRight.entry$item" <Return> {sendQuery $currentMote}

    # landmark checkbutton
    set item Landmark
    checkbutton "$pRight.cb$item"  -text "set/unset" -variable "cb_$item" \
	-command {
	    if {$cb_Landmark == 1} {
		# enable the coords entry 
		$pRight.entryX configure -state normal 
		$pRight.entryY configure -state normal
	    } else {
		# disable the coords entry
		$pRight.entryX configure -state disable
		$pRight.entryY configure -state disable
	    } 
	}
		
    pack "$pRight.cb$item"
    
    # aps_status
    set item APS_Status
    entry "$pRight.entry$item" -text Wait -state disable
    pack "$pRight.entry$item"
    bind "$pRight.entry$item" <Return> {sendQuery $currentMote}

    # restart aps 
    set item Restart_APS
    checkbutton "$pRight.cb$item" -text "Restart now" -variable "cb_$item"
    pack "$pRight.cb$item"

    # add buttons to update props
    frame .prop.bframe
    pack .prop.bframe -side left  -fill y
    button .prop.bframe.up -text "Update Values" -command {updateMote $currentMote}
    button .prop.bframe.sq -text "Send Query" -command {sendQuery $currentMote}
    button .prop.bframe.rescan -text "Rescan" -command {qGetCoord -1}
    pack .prop.bframe.rescan .prop.bframe.up .prop.bframe.sq -side right -expand 1 -fill x -fill y

    # add buttons
    frame .b -bg pink
    pack .b -fill x
    
    button .b.quit -text Quit -command {exit 0}
    button .b.clear -text Refresh -command {refreshCanvas}
    
    pack .b.quit .b.clear -side right -padx 4 -pady 4

    # draw a grid, every 12 pixels apart - represents 1 feet
    global cFactor
    # place dots every 12 pixels
    set lasty [lindex ${map-region} 3]
    set lastx [lindex ${map-region} 2]
    set width 12
    for {set i 0} {$i< $lasty} {incr i $width} {
	# do special stuff every 10*width

	if {[expr $i % ($width * 10)] == 0} {
	    puts stderr $i
	    # y axis label
	    $my_canvas create line 10 $i $lastx $i -fill gray

	    # text at top
	    for {set j 0} {$j < $lastx} {incr j [expr $width * 10]} {
		$my_canvas create text  $j $i \
		    -text "[expr $j / $width],[expr $i / $width]"  \
		    -font "-adobe-courier-medium-r-normal-*-*-100-*-*-m-*-hp-roman8"
		$my_canvas create line $j 0 $j $lasty -fill gray
	    }
	}
	$my_canvas create line $i 0 $i $lasty -fill gray -dash "1 [expr $width - 1]"
    }
}

# updates the mote property sheet given the mote id
proc updateProp { moteid } {

}


# given a country flash that in canvas

proc highLightCountry { w tn } {
 set old_colour [lindex [$w itemconfig $tn -fill] 4]
 
 $w itemconfigure $tn -fill yellow
 after 50 "$w itemconfigure $tn -fill $old_colour; bell"

}

#########################################################
#
#                       Start
#
mapSetup
fileevent stdin readable procReps
refreshCanvas
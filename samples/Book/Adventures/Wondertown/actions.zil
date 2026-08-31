; === CUSTOM SYNTAX ===

<SYNTAX WIND OBJECT = V-WIND>
<SYNTAX WIND OBJECT WITH OBJECT = V-WIND>
<SYNTAX LUBRICATE OBJECT = V-OIL>
<SYNTAX LUBRICATE OBJECT WITH OBJECT = V-OIL>
<SYNTAX HINT = V-HINTS>
<SYNONYM HINT HINTS>

<ROUTINE V-SCORE ("OPTIONAL" (ASK? T))
    <TELL "Score: " N ,SCORE " of " N ,SCORE-MAX " points, in " N ,MOVES>
    <COND (<1? ,MOVES> <TELL " move.">) (T <TELL " moves.">)>
    <CRLF>
    <TELL "Rank: " <GET ,RANKINGS <+ </ ,SCORE 25> 1>> "." CR>
    ,SCORE>

<ROUTINE SCORE-OBJ (OBJ "AUX" TEMP)
     <COND (<NOT <FSET? .OBJ ,TOUCHBIT>>
           <COND (<G? <SET TEMP <GETP .OBJ ,P?VALUE>> 0>
                 <SCORE-UPD .TEMP>
                 <PUTP .OBJ ,P?VALUE 0>
                 <FSET .OBJ ,TOUCHBIT>)>)>>

; === ENTRY POINT ===

<ROUTINE GO ()
    <SETG HERE ,WORKSHOP-FLOOR>
    <THIS-IS-IT ,KEY-HOOK>
    <SETG LIT T>
    <SETG WINNER ,ADVENTURER>
    <SETG PLAYER ,WINNER>
    <MOVE ,WINNER ,HERE>
    <COND (,TIMER-ACTIVE
           <ENABLE <QUEUE I-TICK 1>>)
          (T
           <TELL "The countdown to dawn has been disabled. Take your time, Pip." CR>)>
    <V-LOOK>
    <MAIN-LOOP>
    <AGAIN>>

; === CLOCK DAEMONS ===

<ROUTINE I-TICK ()
    <QUEUE I-TICK 1>
    <COND (,GAME-WON <RTRUE>)>
    <COND (<L? ,TICK-COUNT 1>
           <TELL "The sun rises over the snowy roofs. The last tick fades into silence. The workshop is still, but Pip stands determined." CR>
           <SETG TICK-COUNT 0>
           <JIGS-UP "Dawn has arrived.">)
          (T
           <SETG TICK-COUNT <- ,TICK-COUNT 1>>)>
    <RTRUE>>

; === ROOM ACTION ROUTINES ===

<ROUTINE TOOL-BENCH-FCN (RARG)
    <COND (<EQUAL? .RARG ,M-LOOK>
           <TELL "The tool bench stretches along the wall under racks of chisels and planes. A low crate and three broad repair books form a sturdy makeshift step to the countertop." CR>)>>

<ROUTINE WORKBENCH-TOP-FCN (RARG)
    <COND (<EQUAL? .RARG ,M-LOOK>
           <TELL "The tabletop spreads around Pip like a wooden plain. Half-finished toys wait beneath the lamp, and the workshop floor lies below." CR>)>>

<ROUTINE WORKBENCH-DOWN-EXIT ()
    <COND (,REPAIR-BOOK-OPEN
           <TELL "Tolliver's open repair book rests near the edge. Pip should close the heavy green leather cover before stepping down." CR>
           <RFALSE>)
          (T
           <RETURN ,WORKSHOP-FLOOR>)>>

<ROUTINE COUNTERTOP-FCN (RARG)
    <COND (<EQUAL? .RARG ,M-LOOK>
           <TELL "The toy display spreads across the countertop. Through the frosted shop window, the snowy street and distant clock tower glow under the winter moon." CR>)>>

<ROUTINE STORAGE-LOFT-FCN (RARG)
    <COND (<EQUAL? .RARG ,M-LOOK>
           <TELL "Dust softens the storage loft, and cobwebs drape the rafters like grey curtains." CR>)>>

; === OBJECT ACTION ROUTINES ===

<ROUTINE KEY-HOOK-F ()
    <COND (<VERB? EXAMINE>
           <TELL "The brass hook is polished smooth from years of use, but Grandfather Tolliver's brass key is missing. Only a short length of frayed string dangles from the peg." CR>
           <RTRUE>)>>

<ROUTINE KEY-STRING-F ()
    <COND (<VERB? EXAMINE>
           <TELL "A short length of coarse twine, snapped clean. Whatever was tied to it was pulled away with sudden force." CR>
           <RTRUE>)>>

<ROUTINE WORKBENCH-F ()
    <COND (<VERB? EXAMINE LOOK-UNDER>
           <TELL "The heavy oak workbench is Pip's primary workspace. At nine years old, Pip can reach the tabletop easily using the sturdy wooden stool, where Tolliver's illustrated repair book and tools wait under the lamp." CR>
           <RTRUE>)
          (<VERB? CLIMB>
           <TELL "Pip steps onto the workbench stool and climbs up to inspect the tabletop." CR>
           <GOTO ,WORKBENCH-TOP>
           <RTRUE>)>>

<ROUTINE OIL-CAN-F ()
    <COND (<VERB? EXAMINE>
           <TELL "A small copper oil can with a narrow spout, half full of fine machine oil." CR>
           <RTRUE>)
          (<VERB? TAKE>
           <SCORE-OBJ ,OIL-CAN>
           <RFALSE>)>>

<ROUTINE PET-DOOR-F ()
    <COND (<VERB? EXAMINE>
           <TELL "A small wooden flap cut into the bottom of the workshop back door, framed in brass. Moonlight glimmers through the crack." CR>
           <RTRUE>)
          (<VERB? OPEN LOOK-INSIDE>
           <TELL "Pip peeks through the pet door. Outside, fresh snow blankets the alley, and tiny toy-sized footprints lead away toward the clock square." CR>
           <RTRUE>)>>

<ROUTINE LOFT-LADDER-F ()
    <COND (<VERB? EXAMINE>
           <TELL "A wooden folding ladder attached to iron wall tracks leading up to the storage loft. The lifting mechanism is rusted stiff." CR>
           <RTRUE>)
          (<VERB? CLIMB UP>
           <COND (,LADDER-OILED
                  <TELL "Pip climbs the wooden ladder smoothly into the loft." CR>
                  <GOTO ,STORAGE-LOFT>
                  <RTRUE>)
                 (T
                  <TELL "The ladder mechanism is jammed with rust. Pip cannot raise or climb it until the iron joint is oiled." CR>
                  <RTRUE>)>)>>

<ROUTINE LADDER-MECH-F ()
    <COND (<VERB? EXAMINE>
           <TELL "An iron winch and hinge assembly. Red rust locks the teeth in place." CR>
           <RTRUE>)
          (<VERB? TURN OIL>
           <COND (,LADDER-OILED
                  <TELL "The mechanism is already oiled and turns freely." CR>
                  <RTRUE>)
                 (<IN? ,OIL-CAN ,WINNER>
                  <TELL "Pip applies a few drops of copper-can oil to the rusty iron gear. With a smooth click, the mechanism frees itself, and the folding ladder lowers into place." CR>
                  <SETG LADDER-OILED T>
                  <SCORE-UPD 5>
                  <RTRUE>)
                 (T
                  <TELL "The mechanism is rusted stiff. Pip needs oil to loosen the joints." CR>
                  <RTRUE>)>)>>

<ROUTINE REPAIR-BOOK-F ()
    <COND (<VERB? EXAMINE>
           <TELL "A heavy volume bound in dark green leather, embossed with gold gears. It contains Grandfather Tolliver's handwritten repair notes and diagrams." CR>
           <RTRUE>)
          (<VERB? OPEN READ>
           <COND (,REPAIR-BOOK-OPEN
                  <TELL "The book is open to a page showing the workshop clockwork mechanism and a note: 'The key must be returned before the ninth chime of midnight.'" CR>
                  <RTRUE>)
                 (T
                  <TELL "Pip opens the heavy leather cover. A detailed diagram of the workshop heart mechanism rests on the first page." CR>
                  <SETG REPAIR-BOOK-OPEN T>
                  <SCORE-UPD 5>
                  <RTRUE>)>)
          (<VERB? CLOSE>
           <COND (,REPAIR-BOOK-OPEN
                  <TELL "Pip closes the heavy green leather cover." CR>
                  <SETG REPAIR-BOOK-OPEN <>>
                  <RTRUE>)
                 (T
                  <TELL "The book is already closed." CR>
                  <RTRUE>)>)>>

<ROUTINE SAWDUST-F ()
    <COND (<VERB? EXAMINE>
           <TELL "Clean pine sawdust scattered across the floor from the day's woodwork." CR>
           <RTRUE>)>>

<ROUTINE CLOCK-FACE-F ()
    <COND (<VERB? EXAMINE>
           <TELL "A carved wooden wall clock depicting a mountain village. Its pendulum swings rhythmically." CR>
           <RTRUE>)>>

<ROUTINE SWEEP-BROOM-F ()
    <COND (<VERB? EXAMINE>
           <TELL "A neat straw hand-broom with a wooden handle, used for clearing sawdust from the workbenches." CR>
           <RTRUE>)>>

<ROUTINE V-OIL ()
    <COND (<EQUAL? ,PRSO ,LADDER-MECH ,LOFT-LADDER>
           <COND (,LADDER-OILED
                  <TELL "The mechanism is already oiled and working smoothly." CR>)
                 (<IN? ,OIL-CAN ,WINNER>
                  <TELL "Pip applies oil to the rusty iron mechanism. The ladder unfolds with a smooth click." CR>
                  <SETG LADDER-OILED T>
                  <SCORE-UPD 5>)
                 (T
                  <TELL "Pip needs oil to loosen the rusty iron." CR>)>
           <RTRUE>)
          (T
           <TELL "That does not need oil." CR>)>>

<ROUTINE V-WIND ()
    <TELL "There is nothing to wind here." CR>>

<ROUTINE V-HINTS ()
    <TELL "Look closely at the items around the workshop floor: the oil can under the bench and the folding ladder mechanism." CR>>

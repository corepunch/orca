<DIRECTIONS NORTH EAST WEST SOUTH NE NW SE SW UP DOWN IN OUT LAND>
<VERSION ZIP>
<CONSTANT RELEASEID 1>

; === GLOBAL FLAGS ===

<GLOBAL TICK-COUNT 200>
<GLOBAL CLOCK-SLOWED <>>
<GLOBAL KEY-FOUND <>>
<GLOBAL KEY-WOUND <>>
<GLOBAL BERTRAND-WOUND <>>
<GLOBAL BERTRAND-POLITE <>>
<GLOBAL MARZIPAN-BUTTON <>>
<GLOBAL OLD-TICK-HEARD <>>
<GLOBAL OLD-TICK-RIDDLES 0>
<GLOBAL NUTMEG-TRUST 0>
<GLOBAL NUTMEG-GIFTS 0>
<GLOBAL LADDER-OILED <>>
<GLOBAL CART-MOVED <>>
<GLOBAL CART-HELPED <>>
<GLOBAL TOWER-WOUND <>>
<GLOBAL STUDY-ACCESS <>>
<GLOBAL HEART-ACCESS <>>
<GLOBAL JOURNAL-READ <>>
<GLOBAL LETTER-READ <>>
<GLOBAL DIAGRAM-READ <>>
<GLOBAL STUDY-JOURNAL-READ <>>
<GLOBAL REPAIR-BOOK-OPEN <>>
<GLOBAL GAME-WON <>>
<GLOBAL ENDING-TIER 0>
<GLOBAL COMPANION-COUNT 0>
<GLOBAL NUTMEG-SAVED <>>
<GLOBAL HINT-KEY 0>
<GLOBAL HINT-LEVEL 0>
<GLOBAL NUTMEG-KEY-METHOD 0>
<GLOBAL PUZZLES-SOLVED 0>
<GLOBAL LORE-DISCOVERED 0>
<GLOBAL SCORE-MAX 100>
<GLOBAL TIMER-ACTIVE T>
<GLOBAL RANKINGS
    <LTABLE "Toymaker's Apprentice"
         "Journeyman Toymaker"
         "Master Toymaker"
         "Grand Artificer"
         "The Last Toymaker">>

; === ROOMS ===

<ROOM WORKSHOP-FLOOR
      (IN ROOMS)
      (DESC "Workshop Floor")
      (LDESC "Grandfather Tolliver's workshop. Golden sawdust blankets the wide floorboards. On the wall, the polished brass key hook hangs empty — only a dangling frayed string remains. To the north, a small pet door glows softly under the moonlight.")
      (EAST TO TOOL-BENCH)
      (NORTH TO SNOWY-ALLEY)
      (UP TO STORAGE-LOFT IF LADDER-OILED)
      (IN TO TOLLIVER-STUDY IF STUDY-ACCESS)
      (FLAGS RLANDBIT ONBIT)
      (GLOBAL WORKSHOP-BUILDING TOPIC-TOLLIVER TOPIC-KEY TOPIC-HEART)>

<ROOM WORKBENCH-TOP
      (IN ROOMS)
      (DESC "Workbench Top")
      (ACTION WORKBENCH-TOP-FCN)
      (LDESC "The broad wooden tabletop spreads around young Pip. Half-finished toys, wood shavings, and tiny chisels rest beneath the lamp. Tolliver's enormous green leather repair book rests near the front edge.")
      (DOWN PER WORKBENCH-DOWN-EXIT)
      (FLAGS RLANDBIT ONBIT)
      (GLOBAL WORKBENCH TOPIC-TOLLIVER TOPIC-HEART)>

<ROOM TOOL-BENCH
      (IN ROOMS)
      (DESC "Tool Bench")
      (ACTION TOOL-BENCH-FCN)
      (LDESC "The long tool bench stretches along the wall under racks of chisels and planes. A low crate and three broad repair books form a makeshift stair to the upper countertop.")
      (WEST TO WORKSHOP-FLOOR)
      (UP TO COUNTERTOP IF BERTRAND-WOUND)
      (FLAGS RLANDBIT ONBIT)
      (GLOBAL TOPIC-TOLLIVER TOPIC-KEY)>

<ROOM COUNTERTOP
      (IN ROOMS)
      (DESC "Countertop")
      (ACTION COUNTERTOP-FCN)
      (LDESC "The toy display countertop looks out over the shop. A glass display case holds forgotten treasures, and through the frosted window the snowy street and clock tower glow under the winter moon.")
      (DOWN TO TOOL-BENCH)
      (FLAGS RLANDBIT ONBIT)
      (GLOBAL WORKSHOP-BUILDING TOPIC-TOLLIVER TOPIC-KEY TOPIC-FOX)>

<ROOM STORAGE-LOFT
      (IN ROOMS)
      (DESC "Storage Loft")
      (ACTION STORAGE-LOFT-FCN)
      (LDESC "The storage loft is quiet and dim, cobwebs draping the rafters like grey curtains. Old wooden crates labelled 'To Repair' sit in the shadows where Tolliver kept work in progress.")
      (DOWN TO WORKSHOP-FLOOR)
      (FLAGS RLANDBIT ONBIT)
      (GLOBAL TOPIC-TOLLIVER)>

<ROOM SNOWY-ALLEY
      (IN ROOMS)
      (DESC "Snowy Alley")
      (LDESC "Fresh snow blankets the cobblestone alley outside the workshop back door. Moonlight casts long blue shadows, and tiny toy-sized footprints lead east into the quiet street.")
      (SOUTH TO WORKSHOP-FLOOR)
      (EAST TO CLOCK-SQUARE)
      (FLAGS RLANDBIT ONBIT)
      (GLOBAL WORKSHOP-BUILDING MOON TOPIC-FOX)>

; === OBJECTS: WORKSHOP-FLOOR ===

<OBJECT KEY-HOOK
        (IN WORKSHOP-FLOOR)
        (SYNONYM HOOK)
        (ADJECTIVE KEY BRASS EMPTY)
        (DESC "brass key hook")
        (LDESC "A brass key hook on the wall hangs empty. A frayed string dangles from it.")
        (FLAGS NDESCBIT)
        (ACTION KEY-HOOK-F)>

<OBJECT WORKBENCH
        (IN WORKSHOP-FLOOR)
        (SYNONYM BENCH WORKBENCH TABLE)
        (ADJECTIVE ENORMOUS WOODEN WORK GIANT CLUTTERED)
        (DESC "enormous workbench")
        (FDESC "Grandfather Tolliver's heavy oak workbench stands center-stage, its surface stacked with tools, wood blocks, and half-finished toys.")
        (LDESC "The heavy oak workbench stands in the center of the workshop, stacked with tools.")
        (FLAGS SURFACEBIT CONTBIT OPENBIT SEARCHBIT CLIMBBIT)
        (ACTION WORKBENCH-F)>

<OBJECT OIL-CAN
        (IN WORKSHOP-FLOOR)
        (SYNONYM CAN OILCAN OIL-CAN)
        (ADJECTIVE COPPER SMALL OIL)
        (DESC "small copper oil can")
        (LDESC "A small copper oil can sits beneath the workbench.")
        (FLAGS TAKEBIT)
        (SIZE 3)
        (ACTION OIL-CAN-F)>

<OBJECT SAWDUST
        (IN WORKSHOP-FLOOR)
        (SYNONYM SAWDUST DUST FLOOR)
        (ADJECTIVE SOFT GOLDEN)
        (DESC "soft sawdust")
        (LDESC "Soft sawdust covers the floorboards like a golden blanket.")
        (FLAGS NDESCBIT)
        (ACTION SAWDUST-F)>

<OBJECT PET-DOOR
        (IN WORKSHOP-FLOOR)
        (SYNONYM DOOR PET)
        (ADJECTIVE PET SMALL WOODEN)
        (DESC "small pet door")
        (LDESC "A small pet door at the base of the back exit glows with moonlight.")
        (FLAGS NDESCBIT)
        (ACTION PET-DOOR-F)>

<OBJECT SWEEP-BROOM
        (IN WORKSHOP-FLOOR)
        (SYNONYM BROOM BRUSH)
        (ADJECTIVE HAND WOODEN SWEEP)
        (DESC "wooden sweep broom")
        (LDESC "Pip's workshop broom leans against the wall beside the workbench.")
        (FLAGS TAKEBIT)
        (SIZE 4)
        (ACTION SWEEP-BROOM-F)>

<OBJECT CLOCK-FACE
        (IN WORKSHOP-FLOOR)
        (SYNONYM CLOCK CUCKOO)
        (ADJECTIVE OLD WOODEN WALL)
        (DESC "old cuckoo clock")
        (LDESC "An old cuckoo clock hangs on the wall, ticking softly.")
        (FLAGS NDESCBIT)
        (ACTION CLOCK-FACE-F)>

<OBJECT KEY-STRING
        (IN WORKSHOP-FLOOR)
        (SYNONYM STRING CORD THREAD TWINE)
        (ADJECTIVE FRAYED DANGLING)
        (DESC "frayed string")
        (LDESC "A frayed string dangles from the empty key hook.")
        (FLAGS TAKEBIT NDESCBIT)
        (SIZE 1)
        (ACTION KEY-STRING-F)>

<OBJECT LOFT-LADDER
        (IN WORKSHOP-FLOOR)
        (SYNONYM LADDER STAIRS STAIRCASE)
        (ADJECTIVE FOLDING LOFT WOODEN)
        (DESC "folding loft ladder")
        (LDESC "A folding wooden ladder leads up toward the storage loft. Its iron hinges are stiff with rust.")
        (FLAGS NDESCBIT)
        (ACTION LOFT-LADDER-F)>

<OBJECT LADDER-MECH
        (IN WORKSHOP-FLOOR)
        (SYNONYM MECHANISM LADDER LIFT WINCH HINGE)
        (ADJECTIVE RUSTY IRON LIFTING)
        (DESC "rusty ladder mechanism")
        (LDESC "The rusty iron lifting mechanism for the loft ladder is frozen solid.")
        (FLAGS TURNBIT NDESCBIT)
        (ACTION LADDER-MECH-F)>

; === OBJECTS: WORKBENCH TOP ===

<OBJECT REPAIR-BOOK
        (IN WORKBENCH-TOP)
        (SYNONYM BOOK MANUAL GUIDE ALBUM)
        (ADJECTIVE REPAIR ILLUSTRATED TOLLIVER LEATHER GREEN)
        (DESC "illustrated repair book")
        (LDESC "Tolliver's green leather repair book rests closed beside a toy sailboat.")
        (FLAGS CONTBIT OPENABLEBIT READBIT NDESCBIT)
        (ACTION REPAIR-BOOK-F)>

<OBJECT HALF-FINISHED-TOYS
        (IN WORKBENCH-TOP)
        (SYNONYM TOYS SAILBOAT BOAT TRAIN DUCK)
        (ADJECTIVE HALF-FINISHED WOODEN TOY)
        (DESC "half-finished toys")
        (LDESC "A half-rigged sailboat, a wooden train, and a toy duck wait beneath the work lamp.")>

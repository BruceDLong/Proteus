# Unified name bindings: templates are read through #, individuals through %.
unified/parse/template, Template reference before application, #planet, #planet, parse
unified/parse/listSpec, Structural template member, #planet.listSpec, #planet.listSpec, parse
unified/parse/cursor, Named occurrence cursor, %Bob, %Bob, parse
unified/template/listSpec, State model reused without duplicating its definition, {#position=_ #planet={T {position}|...}}\n#state=#planet.listSpec\nstate:{position:7}.position, position:7, multi
unified/template/alias, Template alias remains reusable, {#value=_ #box={value}}\n#other=#box\nother:{value:3}.value//:value:3\nother:{value:8}.value//:value:8, -, multi
unified/template/refinement, Construct a refined template before applying it, {#color=$ #size=_ #balloon={color\,size}}\n#kind=#balloon:{color:'red'\,size:3}\nkind.color//:color:'red'\nkind.size//:size:3, -, multi
unified/alias/read, Existing instance becomes a named cursor, {#value=_ #box={value}}\nbox:{value:7}\n#Bob=%ctx.box\n%Bob.value, value:7, multi
unified/alias/stable, Later context does not redirect a cursor, {#value=_ #box={value}}\nbox:{value:7}\n#Bob=%ctx.box\nbox:{value:9}\n%Bob.value, value:7, multi
unified/alias/shared, Two aliases learn facts about the same occurrence, {#value=_ #box={value}}\nbox\n#Bob=%ctx.box\n#Robert=%Bob\n%Robert.value=5//:value:5\n%Bob.value//:value:5, -, multi
unified/scope/template, Nested declaration shadows without leaking, {#value=_ #shape={value:1}}\n{{#shape={value:2} shape.value} shape.value}//:{{#shape = {value:2} value:2} value:1}\nshape.value//:value:1, -, multi
unified/scope/instance, Local cursor shadows a global cursor, {#value=_ #box={value}}\nbox:{value:1}\n#Bob=%ctx.box\nbox:{value:2}\n{{#Bob=%ctx.box %Bob.value} %Bob.value}//:{{#Bob = %ctx.box value:2} value:1}\n%Bob.value//:value:1, -, multi
unified/parse/hyphen, Named cursor accepts word identifiers, %Milky-Way, %Milky-Way, parse
unified/template/unchanged, Refining a template does not constrain its source, {#color=$ #balloon={color}}\n#redBalloon=#balloon:{color:'red'}\nballoon:{color:'blue'}.color, color:'blue', multi
unified/alias/temporal, Stable individual with successive S states, {#angle=_ #planet={T {angle\, ...}|...}}\nplanet:{T {angle:0} {angle:1}}\n#Earth=%ctx.planet\n%Earth@[1].angle//:angle:0\n%Earth@[2].angle//:angle:1, -, multi
unified/scope/embedded, Embedded declarations use the containing scope, {#value=_ &{#shape={value:2}} shape.value}, {#value = _ &{#shape = {value:2}} value:2}, norm
unified/alias/conflict, Conflicting facts do not replace an individual, {#value=_ #box={value}}\nbox:{value:7}\n#Bob=%ctx.box\n%Bob.value=8//:<REJECTED>\n%Bob.value//:value:7, -, multi
unified/alias/noRebinding, A name retains its first referent, {#value=_ #box={value}}\nbox:{value:1}\n#Bob=%ctx.box\nbox:{value:2}\n#Bob=%ctx.box//:<REJECTED>\n%Bob.value//:value:1, -, multi
unified/template/callAlias, Template alias preserves parameter signature, {#ident(item) <en verb> = %item}\n#copy <en> = #ident\ncopy(7), copy(7):7, multi
unified/parse/decorations, Header decorations survive round trip, {#give(actor\,item) <en verb> = %item #Bob <en name> = %ctx.box}, {#give(actor\, item) <en verb >= %item #Bob <en name >= %ctx.box}, parse
unified/scope/applications, Local bindings belong to each application, {#value=_ #box={value} #sample={#picked=%ctx.box %picked.value}}\nbox:{value:7}\nsample@[2]//:value:7\nbox:{value:9}\nsample@[2]//:value:9, -, multi
unified/alias/throughList, Storing a cursor retains its individual, {#value=_ #box={value} #bag={box|...}}\nbox\n#Bob=%ctx.box\nbag:{%Bob}\n#Container=%ctx.bag\n#Other=%Container@[1]\n%Other.value=5//:value:5\n%Bob.value//:value:5, -, multi
unified/template/missingSpec, An absent listSpec does not create an individual, {#scalar=_}\n#bad=#scalar.listSpec//:<REJECTED>, -, multi
unified/alias/constrained, A naming initializer can establish compatible facts, {#value=_ #box={value}}\nbox\n#Bob=%ctx.box\n#Robert=%Bob={value:5}\n%Bob.value, value:5, multi
unified/alias/constrainedConflict, A failed naming initializer publishes no facts, {#value=_ #box={value}}\nbox:{value:7}\n#Bob=%ctx.box\n#Robert=%Bob={value:9}//:<REJECTED>\n%Bob.value//:value:7, -, multi
unified/alias/qualifiedSelection, A qualified context description selects an existing individual, {#color=$ #balloon={color}}\nballoon:{color:'red'}\n#Bob=%ctx.balloon:{color:'red'}\nballoon:{color:'blue'}\n%Bob.color, color:'red', multi

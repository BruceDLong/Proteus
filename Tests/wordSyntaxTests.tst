# Word parameter declarations and temporal-view syntax; execution is separate.
# name,display,input,expected,mode
parser/wordSyntax/parameters, parameters, {#give(actor\, item\, object) = {%actor %item %object}}, {#give(actor\, item\, object) = {%actor %item %object}}, parse
parser/wordSyntax/emptyParameters, emptyParameters, {#idle() = 7}, {#idle() = 7}, parse
parser/wordSyntax/plainDefinition, plainDefinition, {#idle = 7}, {#idle = 7}, parse
parser/wordSyntax/usageTags, usageTags, {#give(actor\,item)<en verb>={%actor %item}}, {#give(actor\, item) <en verb >= {%actor %item}}, parse
parser/wordSyntax/headerWhitespace, headerWhitespace, {#give ( actor \, item ) = {%actor %item}}, {#give(actor\, item) = {%actor %item}}, parse
parser/wordSyntax/headerComments, headerComments, {#give(actor\, /* role */ item) = %item}, {#give(actor\, item) = %item}, parse
parser/wordSyntax/parameterIdentifiers, parameterIdentifiers, {#give(actor_1\, item2) = {%actor_1 %item2}}, {#give(actor_1\, item2) = {%actor_1 %item2}}, parse
parser/wordSyntax/parameterCase, parameterCase, {#give(item\, Item) = {%item %Item}}, {#give(item\, Item) = {%item %Item}}, parse
parser/wordSyntax/contextReference, contextReference, {#walk(destination) = {%destination %self %tprev}}, {#walk(destination) = {%destination % %tprev}}, parse
parser/wordSyntax/nestedDefinitions, nestedDefinitions, {#outer(item) = {#inner(item)=%item %item}}, {#outer(item) = {#inner(item) = %item %item}}, parse
parser/wordSyntax/nestedBody, nestedBody, {#outer(item) = {{%item}}}, {#outer(item) = {{%item}}}, parse
parser/wordSyntax/parameterView, parameterView, {#track(item) = %item^T@[1]^S}, {#track(item) = %item^T@[1]^S}, parse
parser/wordSyntax/temporalView, temporalView, cube^T, cube^T, parse
parser/wordSyntax/structuralView, structuralView, span^S, span^S, parse
parser/wordSyntax/selectedState, selectedState, span@[1]^S, span@[1]^S, parse
parser/wordSyntax/viewChain, viewChain, cube^T@[1]^S.location, cube^T@[1]^S.location, parse
parser/wordSyntax/fieldView, fieldView, cube.face^T@[2]^S, cube.face^T@[2]^S, parse
parser/wordSyntax/viewIdentity, viewIdentity, cube^T = {T 1 2}, cube^T = {T 1 2}, parse
parser/wordSyntax/viewRoundTrip, viewRoundTrip, cube^T^S^T, cube^T^S^T, parse
parser/wordSyntax/externalProperty, externalProperty, cube^size, cube^size, parse
parser/wordSyntax/lowercaseProperty, lowercaseProperty, cube^t, cube^t, parse
parser/wordSyntax/longProperty, longProperty, cube^Timeline, cube^Timeline, parse
parser/wordSyntax/longSProperty, longSProperty, cube^State, cube^State, parse
parser/wordSyntax/legacyProjection, legacyProjection, cube.asTList, cube.asTList, parse

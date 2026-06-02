# default tests migrated from PARSETEST/NORMTEST/MULTITEST/WORLDTEST
# columns: name,display,input,expected,mode(parse|norm|multi|world),world-file optional

parser/misc/tUnknown,  type unknown,                   ?,                                                                      ?,                              parse
parser/misc/isWord,    parse a word,                   ImAWord ,                                                               ImAWord,                        parse
parser/misc/isNot,     Not word,                       !ImAWord,                                                               !ImAWord,                       parse
parser/misc/world,     World cursor,                   %W,                                                                     %W,                             parse
parser/misc/self,      self cursor,                    %self,                                                                  %,                              parse
parser/misc/args,      args cursor,                    %args,                                                                  %args,                          parse
parser/misc/vars,      vars cursor,                    %vars,                                                                  %vars,                          parse
parser/num/bare,       Simple number,                  34,                                                                     34,                             parse
parser/num/plus,       Positive number,                +34,                                                                    +34,                            parse
parser/num/sized,      Sized number,                   *64+34 ,                                                                *64+34,                         parse
parser/num/divd,       Sized number,                   /64-34,                                                                 /64-34,                         parse
parser/num/unknown,    Unknown number,                 _,                                                                      _,                              parse
parser/num/neg,        Negative number,                -34,                                                                    -34,                            parse
parse/largenum,        parse a large number,           12345678901234567890123456789012345678901234567890,                     12345678901234567890123456789012345678901234567890, parse
parse/hexnum,          parse a hex number,             0x149F890204B24B57979284D4060E107E870B684045F15,                        123456789123456789123456789123456789123456789123456789, parse
parse/binnum,          parse a binary number,          0b11001010101010101010101000001111,                                     3400182287,                     parse
parse/decimal,         parse decimal number,           123.456,                                                                123.456,                        parse
parse/frac,            parse fraction number,          123/246,                                                                1/2,                            norm
parser/str/quote1,     Single Quoted,                  'Hello There!',                                                         'Hello There!',                 parse
parser/str/quote2,     Double Quoted,                  "Hello There!",                                                         'Hello There!',                 parse
parser/str/unknown,    Unknown string,                 $,                                                                      $,                              parse
parser/str/sized,      Size given,                     *4+$,                                                                   *4+$,                           parse
parser/list/simple,    simple list,                    {'A' 12 {1 2 3}},                                                       {'A' 12 {1 2 3}},               parse
parser/list/time,      time list,                      {T 'A' 12},                                                             {T 'A' 12},                     parse
parser/list/empty,     empty list,                     {},                                                                     {},                             parse
parser/list/unknown,   Unknown list,                   {1 ... 3},                                                              {1 ... 3},                      parse
parser/list/defClass,  define a class,                 {'A' class item {1 2 3} 12},                                            {'A' class item {1 2 3} 12},    parse
parser/list/classed,   classed list,                   {item| 'A' 12},                                                         {item| 'A' 12},                 parse
parser/list/sized1,    sized list1,                    {~2| 'A' 12},                                                           {~2| 'A' 12},                   parse
parser/list/sized2,    sized list2,                    {~2 'A' 12},                                                            {~2| 'A' 12},                   parse
parser/list/CandS1,    Both specs1,                    {~ 2 item| 'A' 12},                                                     {~2 item| 'A' 12},              parse
parser/list/CandS2,    Both specs2,                    {~2 item | 'A' 12},                                                     {~2 item| 'A' 12},              parse
parser/list/noC,       No C,                           {item| 'A' 12},                                                         {item| 'A' 12},                 parse
parser/list/partial,   partial lists,                  { {...} {1 ... } {1 ... 5} {}},                                         {{ ... } {1 ... } {1 ... 5} {}}, parse
parser/list/embedded,  Embedded list,                  {1 2 &{3 4 5} 6 7},                                                     {1 2 &{3 4 5} 6 7},             parse
parser/list/concat,    concated list,                  (1 2 3),                                                                (1 2 3),                        parse
parser/list/square,    square brackets,                [1 2 3],                                                                [1 2 3],                        parse
parser/list/commas,    comma list,                     {2\, 3\, 4 \, 5 },                                                      {2\, 3\, 4\, 5},                parse
parser/ident/bare,     Simple identity,                _=34,                                                                   _ = 34,                         parse
parser/ident/force,    Force identity,                 123<<=456,                                                              123 <<= 456,                    parse
merge/ident/force,     Force identity,                 123<<=456,                                                              456,                            norm
parser/func/right,     Args on right,                  [_ 2 3]<:1,                                                             [_ 2 3]<:1,                     parse
parser/func/left,      Args on left,                   1:>[_ 2 3],                                                             1:>[_ 2 3],                     parse
parser/func/rightInv,  Invert on right,                [_ 2 3]<!1,                                                             [_ 2 3]<!1,                     parse
parser/func/leftInv,   Invert on left,                 1!>[_ 2 3],                                                             1!>[_ 2 3],                     parse
merge/num/typed1,      typed int merge 1,              *16+10 = *16+10,                                                        *16+10,                         norm
merge/num/typed2,      typed int merge 2,              *16+_ = *16+9,                                                          *16+9,                          norm
merge/num/typed3,      typed int merge 3,              _ = *16+10,                                                             *16+10,                         norm
merge/num/typed4,      typed int merge 4,              *_+_ = *16+9,                                                           *16+9,                          norm
merge/num/typed5,      typed int merge 5,              *_+8 = *16+8,                                                           *16+8,                          norm
merge/num/typed6,      typed int merge 6,              *16+10 = *16+_,                                                         *16+10,                         norm
merge/num/typed7,      typed int merge 7,              *16+_ = *_+9,                                                           *16+9,                          norm
merge/str/typed1,      typed str merge 1,              *5+'Hello' = *5+'Hello',                                                *5+'Hello',                     norm
merge/str/typed2,      typed str merge 2,              *5+$ = 'Hello',                                                         'Hello',                        norm
merge/str/typed3,      typed str merge 3,              $ = 'Hello',                                                            'Hello',                        norm
merge/str/typed4,      typed str merge 4,              *_+$ = 'Hello',                                                         'Hello',                        norm
merge/str/typed5,      typed str merge 5,              'Hello' = *5+$,                                                         'Hello',                        norm
merge/str/loose,       loose str merge,                $ == 'Hello',                                                           'Hello',                        norm
merge/str/looseSize,   loose str merge,                $ =: 'Hello',                                                           *_+'Hello',                     norm
merge/unknown/loose,   unknown merge,                  ? == 'Hello',                                                           'Hello',                        norm
merge/unknown/looseSize, unknown merge,                ? ==: 'Hello',                                                          *_+'Hello',                     norm
merge/unknown/typed,   typed merge,                    ? = 'Hello',                                                            'Hello',                        norm
merge/func/loose,      loose merge,                    [...] == 'Hello',                                                       'Hello',                        norm
merge/func/looseSize,  loose merge,                    [...] =: 'Hello',                                                       *_+'Hello',                     norm
merge/func/typed,      typed merge,                    [...] = 'Hello',                                                        'Hello',                        norm
merge/list/empty,      Merge empty list,               {} = {},                                                                {},                             norm
merge/list/N_dots1,    Merge N items ...,              {{1 ... 4} 5} = {{1 2 3 4} 5},                                          {{1 &{2 3} 4} 5},               norm
merge/list/N_dots2,    Merge N items ...,              {1 ... 4} = {1 2 3 4},                                                  {1 &{2 3} 4},                   norm
merge/list/N_dots3,    Merge N items ...,              *3+{...} = {1 2 3},                                                     {1 2 3},                        norm
merge/list/dots,       Merge to ...,                   *_+{...} = {2 3 4},                                                     {2 3 4},                        norm
merge/list/dotsC,      Merge cont to ...,              {...} = {2 3 4 ...},                                                    {2 3 4 ... },                   norm
merge/list/nums,       Simple list merge,              {_ _ _} = {2 3 4},                                                      {2 3 4},                        norm
merge/list/misc,       Merge misc items,               *4+{3 _ $ *3+{...}} = *4+{3 4 'hi' {5 6 7}},                            {3 4 'hi' {5 6 7}},             norm
merge/list/multiID,    Merge many idents,              {_ _ _} = {_ 2 _} = { 1 _ _} = { _ _ 3},                                {1 2 3},                        norm
merge/calcSize,        Merge calc'd size,              *(10)+5,                                                                *10+5,                          norm
trav/list/withEmpty,   traverse over {},               {{1 2} {} {_=3} _=4},                                                   {{1 2} {} {3} 4},               norm
merge/intersect/onRHS, When [] is on RHS,              _ = *_+[5 6 7],                                                         *_+[t5 t6 t7],                  norm
noRHS/dots/simple,     Test dots in LHS,               {...},                                                                  { ... },                        norm
noRHS/dots/nestedOut,  Test nested LHS dots,           {{...} _=5},                                                            {{ ... } 5},                    norm
noRHS/dots/nestedIn,   Test nested LHS dots,           {1 _=2 3 ...},                                                          {1 2 3 ... },                   norm
merge/dots/align1,     Test 1 of alignment,            {1 ... 4}  = {_ 2 3 _},                                                 {1 &{2 3} 4},                   norm
merge/dots/align2a,    R dots inside L dots,           {{1 ... 5} 6}  = {{_ 2 ... 4 _} 6},                                     {*_+{1 &{2 &{ ... } 4} 5} 6},   norm
merge/dots/align2b,    R dots inside L dots,           {1 ... 5}  = {_ 2 ... 4 _},                                             *_+{1 &{2 &{ ... } 4} 5},       norm
merge/dots/align3,     R dots on left of L,            {1 ... 6}  = {... &{4 5 6}},                                            *_+{1 &{&{ ... } 4 5} 6},       norm
merge/dots/align4,     Harder to detect end,           {1 ... 5 _} = {... 4 5 6},                                              *_+{1 &{&{ ... } 4} 5 6},       norm
merge/dots/align4b,    Ambiguous situation,            {1  4 ... 5 _} = {... 4 5 6},                                           {1 4 &{4 5 6 ... } 5 6},        norm
# merge/dots/align5,    Test 5 of alignment	{1 ... 4 ...} = {... &{4 ...} 7}	{1 ... 4 ... 7}	norm
EmbeddedLists/LHS,     Test &{} on LHS,                {1 2 &{_ _} _} = {1 2 3 4 5},                                           {1 2 &{3 4} 5},                 norm
EmbeddedLists/RHS,     Test &{} on RHS,                {1 2 _ _ _} = {1 2 &{3 4} 5},                                           {1 2 3 4 5},                    norm
EmbeddedLists/eLHS,    Test &{} on LHS,                {1 2 &{} _} = {1 2 3},                                                  {1 2 &{} 3},                    norm
EmbeddedLists/eRHS,    Test &{} on RHS,                {1 2 _ _ _} = {1 2 &{} 3 4 5},                                          {1 2 3 4 5},                    norm
EmbeddedLists/e_e,     &{} on both sides,              {1 2 &{} _ _} = {1 2 &{} 3 4},                                          {1 2 &{} 3 4},                  norm
EmbeddedLists/f_f,     &{..} on both sides,            {1 2 &{_ 4} _} = {1 2 &{3 _} 5},                                        {1 2 &{3 4} 5},                 norm
getLastItem/simple,    Test getLast,                   [2 3 4 5],                                                              5,                              norm
getLastItem/embedded,  getLast with &{},               [2 3 4 &{5 6 7}],                                                       7,                              norm
getLastItem/list,      getLast with {},                [2 3 4 {5 6 7}],                                                        {5 6 7},                        norm
getLastItem/intID,     getLast int,                    [2 3 _] = 5,                                                            5,                              norm
getLastItem/strID,     getLast str,                    [2 3 $] = 'hello',                                                      'hello',                        norm
getLastItem/listID,    getLast list,                   [7 8 9 {...}] = {1 2 3},                                                {1 2 3},                        norm
getLastItem/intIDQ,    getLast int Q,                  [2 3 ?] = 5,                                                            5,                              norm
getLastItem/strIDQ,    getLast str Q,                  [2 3 ?] = 'hello',                                                      'hello',                        norm
getLastItem/listIDQ,   getLast list Q,                 [2 3 ?] = {4 5 6},                                                      {4 5 6},                        norm
altSelect/simple,      Select alternate,               3 = *_+[2 3 4],                                                         3,                              norm
pushArg/rightFirst,    test right arg,                 {_ 6 _}<:5,                                                             {5 6 _},                        norm
pushArg/leftFirst,     test left arg,                  5:>{_ 6 _},                                                             {5 6 _},                        norm
pushArg/rightLast,     test right inverse,             {_ 6 _}<!7,                                                             {_ 6 7},                        norm
pushArg/leftLast,      test left inverse,              7!>{_ 6 _},                                                             {_ 6 7},                        norm
concat/oneItem,        Test null concat,               ('Hello'),                                                              'Hello',                        norm
concat/simpleStr,      simple string concat,           ('Hello' ' there' ' world'),                                            'Hello there world',            norm
concat/twoNums,        Test small concat,              {1 (*8+4 *4+3) 3},                                                      {1 *32+19 3},                   norm
concat/strCat,         string concat,                  (('Hello' ' There!' (' How' ' are' (' you' ' Doing') '?'))),            'Hello There! How are you Doing?', norm
concat/numCat,         integer concat,                 ((*5+2 *6+3 (*7+4 *8+5 (*9+6 *10+7) *1+8))),                            *151200+79005,                  norm
concat/simpleList,     simple list,                    ({1 2} {} {3 4}),                                                       {1 2 3 4},                      norm
concat/listCat,        list concat,                    (({1 2} {} {3} ({4 5} ({} {6 7}) {8} {}))),                             {1 2 3 4 5 6 7 8},              norm
concat/parseSrc,       Parse a concatenated string,    [*4+$ *10+$] <~ ('DO' 'gsTin' 'tinabulation'),                          'Tintinabul',                   norm
tUnknown/simple,       test isolated tUnknown,         ?,                                                                      ?,                              norm
tUnknown/simple2,      test tUnknown = ?,              ?=?,                                                                    ?,                              norm
tUnknown/unknownIsNum, test tUnknown = _,              ?=_,                                                                    _,                              norm
tUnknown/unknownIsStr, test tUnknown = $,              ?=$,                                                                    $,                              norm
tUnknown/unknownIsLst1, test tUnknown = {},            ?={2 3 4},                                                              {2 3 4},                        norm
tUnknown/unknownIsLst2, test tUnknown = {},            {?={2 3 4}},                                                            {{2 3 4}},                      norm
parse/parse3n4,        3 char then 4 char strings,     {*3+$ *4+$} == 'CatDogs',                                               {'Cat' 'Dogs'},                 norm
parse/catHatDogPig,    4 three char strings,           *4 +{*3+$|...} ==: 'catHatDogPig',                                      {'cat' 'Hat' 'Dog' 'Pig'},      norm
parse/simple2,         Simple Parsing 2,               {*3+$|...}=='CatHatBatDog' ,                                            {'Cat' 'Hat' 'Bat' 'Dog'},      norm
# parse/inner,	Inner parsing 1	{ {*3+$}|...}=='CatHatBatDog' 	({{'Cat' } {'Hat' } {'Bat' } {'Dog' }})	norm
# parse/get2nd,	Parse select 2nd	 [...] == 'ARONdacks' = {'AARON' 'ARON'}	'ARON'	norm
parse/reps1,           Parse repetition,               {{'A'|...} 'R'} ==: 'AARON',                                            {{'A' 'A'} 'R'},                norm
parse/reps2,           Parse repetition,               {{'A'|...}'AARON'} == 'AAAARON',                                        {{'A' 'A'} 'AARON'},            norm
parse/innerID,         No parent worklist,             {1{{2 _=3}}},                                                           {1 {{2 3}}},                    norm
parse/grouping1,       Group into pairs 1,             {{_ _}|...} == {1 2 3 4 5 6 7 8},                                       {{1 2} {3 4} {5 6} {7 8}},      norm
parse/grouping2,       Group into pairs 2,             {*2+{...}|...} == {1 2 3 4 5 6 7 8},                                    {{1 2} {3 4} {5 6} {7 8}},      norm
func/simpleList1,      test func = list,               [...] = {1 2 3},                                                        {1 2 3},                        norm
func/simpleList2,      test func == list,              [...] == {1 2 3},                                                       {1 2 3},                        norm
func/simpleList3,      test func ==: list,             [...] ==: {1 2 3},                                                      *_+{1 2 3},                     norm
func/anonFunc1,        test anon functions,            [_ 456 789] <: +123,                                                    789,                            norm
# parse/select1st,	Parse & select option 1	[...]=='AARONdacks' :== {'AARON' 'ARON'} 	('AARON')	norm
# parse/2Itms,	Two item parse	{[...] :== {'AARON' 'BOBO' 'ARON' 'AAAROM'}   'dac'} ==  'ARONdacks'	({'ARON' 'dac' })	norm
# parse/2ItmsGet1st,	Two item parse: get first option	{[...] :== {'ARON' 'BOBO' 'AARON' 'CeCe'}   'dac'} ==  'ARONdacks'	({'ARON' 'dac' })	norm
innr/oneOf,            Choose one of,                  [...] <~ {'ARON' 'AARON' 'ERIN'} =='AARONDacs',                          'AARON',                        norm
query/first,           Get first item,                 *1+['Cat' 'Hat' 'Bat' 'Dog'],                                           'Cat',                          norm
query/getNth,          Get Nth item,                   *3+['Cat' 'Hat' 'Bat' 'Dog'],                                           'Bat',                          norm
# query/firstStrA,	Get first String	[&{&{_|...} $} =: {2 3 'Cat' 'Hat'}]	'Cat'	norm
query/firstStrB,       Get first String,               [&{_|...} $]<~{2 3 'Cat' 'Hat'},                                        'Cat',                          norm
query/firstStrC,       Get first String,               [&{!$|...} $]<~{2 {'Hi' 3 } 4 'Cat' 'Hat'},                             'Cat',                          norm
write/writeStrA,       Write first String,             {2 3 $ 'Hat'}.$ = 'Cat';,                                               {2 3 'Cat' 'Hat'},              norm
write/writeByIdx,      Write third item,               {'Cat' 'Hat' $ 'Dog'}#3 = 'Bat';,                                       {'Cat' 'Hat' 'Bat' 'Dog'},      norm
write/first,           Write first item,               {$ 'Hat' 'Bat' 'Dog'}.first = 'Cat';,                                   {'Cat' 'Hat' 'Bat' 'Dog'},      norm
write/last,            Write last item,                {'Cat' 'Hat' 'Bat' $}.last = 'Dog';,                                    {'Cat' 'Hat' 'Bat' 'Dog'},      norm
write/1stAndLst,       Write 1st & last item,          {$ 'Hat' 'Bat' $}.first='Cat';.last='Dog';,                             {'Cat' 'Hat' 'Bat' 'Dog'},      norm
read/readStrA,         Read first String,              {2 3 'Cat' 'Hat'}.$,                                                    'Cat',                          norm
read/readByIdx,        Read third item,                {'Cat' 'Hat' 'Bat' 'Dog'}#3,                                            'Bat',                          norm
read/first,            Read first item,                {'Cat' 'Hat' 'Bat' 'Dog'}.first,                                        'Cat',                          norm
read/last,             Read last item,                 {'Cat' 'Hat' 'Bat' 'Dog'}.last,                                         'Dog',                          norm
word/useTags,          Usage tags,                     {@howdy <en_US southern slang> ='hello'},                               {@howdy <en_US southern slang >= 'hello'}, norm
word/pluralize,        Singular to plural,             {bike| ... },                                                           bikes:{bike|  ... },            norm
word/findPlural,       Find plural,                    {123\, foot\, {foot|foot1\,foot2}}.feet,                                feet:{foot1\, foot2},           norm
neg/simple,            Negative cnvt,                  *10-1,                                                                  *10+9,                          norm
fracs/simple,          simple,                         *10+8/2,                                                                *10+4,                          norm
range/join,            Range joins,                    *10+(2+3),                                                              *10+5,                          norm

#`range/select1` is not a `GET_LAST` bug. It is a propagation-order bug that shows up as polluted `candidatesForLastItem`.
#- The last build where range/select1 passed was likely fdaa2c1
#- The concrete failing symptom is: the current run reaches cleanup with the real last candidate `6` plus a stale extra candidate that has collapsed to `[]`. Because cleanup does not see a singleton, it cannot do `CLEAN_LAST`, and the final result becomes `{2 3 4 5 ?}`.
#- That extra candidate happens to all the items (1, 2, 3, 4...), not just the last one.
#- The place where that state becomes visible is the last-candidate registration logic at [Proteus.Lib.dog](/home/bruce/devl/Proteus/Proteus.Lib.dog:2083), especially the adds at [2088](/home/bruce/devl/Proteus/Proteus.Lib.dog:2088) and [2093](/home/bruce/devl/Proteus/Proteus.Lib.dog:2093). In the failing run, extra intermediate nodes get registered there; in the passing run, those parent-level intermediates are never registered.
#- The first hard control-flow split is the step-0 fastpath gate at [Proteus.Lib.dog](/home/bruce/devl/Proteus/Proteus.Lib.dog:2467). Current code blocks the fastpath when `RHS.value.format==fConcat` at [2471](/home/bruce/devl/Proteus/Proteus.Lib.dog:2471), which forces the `MG0` RHS-normalization path at [2480](/home/bruce/devl/Proteus/Proteus.Lib.dog:2480). That extra subtree is where the intermediate candidates come from.
#- But that concat guard is not the whole bug. Relaxing it alone was not enough to restore the old behavior, and it regressed `time/t2`.
#
# What is not the cause:
#
#- The wait-state machinery is not the relevant issue here. That path is streaming-oriented, and changing it did not explain the `range/select1` behavior.
#- `RHSNeedsNestedCursorNormalization()` is not the blocker in this case; the decisive blocker was the `fConcat` guard.
#- `GET_LAST` is downstream, not root cause. By the time `GET_LAST` runs, the candidate set is already wrong.
#
# The other proven factor is `propagateProxy()` at [Proteus.Lib.dog](/home/bruce/devl/Proteus/Proteus.Lib.dog:2202). Current code uses the `doReply`/`listClosed` gate at [2216](/home/bruce/devl/Proteus/Proteus.Lib.dog:2216)-[2228](/home/bruce/devl/Proteus/Proteus.Lib.dog:2228), and that changes when propagation happens relative to candidate registration. Instrumentation showed the same intermediate node is skipped in the passing build because its `intersectPos` has already changed, but in the current build it is reached earlier and gets added first. Reverting only `propagateProxy()` did not give a clean fix either; with the current concat guard restored, that combination crashed during cleanup.
#
# So the best current statement is: `range/select1` is caused by an interaction between the step-0 concat fastpath change and the newer non-streaming propagation/subscription timing in `propagateProxy()`. The visible bug is stale last-item candidates; the root bug is earlier scheduling/order. The workspace is back on the clean baseline.

range/select1,         Select in range,                {[&{!(*4+_ +2)|  ... } (*4+_ +2)] | ...} <~ {1 2 0 3 4 5 6 7},          {2 3 4 5},                      norm
range/select2,         Select in range,                {[&{!*4+_|  ... } *4+_] | ...} <~ {1 2 0 3 444 555 666 777 2},          {1 2 0 3 2},                    norm
range/select3,         Select in range,                {[&{!*4+_|  ... } *4+_] | ...} <~ {1 *3+_  5 *4+(*2+_ +1)  *4+_  },     {1 *4+_ *4+(*2+_ +1) *4+_},     norm
range/concat1,         Overlapping range,              *10+(*5+_+2) = *10+(*4+_+5),                                            *10+(*2+_ +5),                  norm
range/dot,             Find with dot,                  {1 0 6 7 8 4 5 6 7}.*10+(*3+_+2) ,                                      4,                              norm
# range/alts	    Select alts	{[&{!*_+[*10+(*3+_+2) *10+(*3+_+6)]|  ... } *_+[*10+(*3+_+2) *10+(*3+_+6)]] | ...} <~ {1 2 3 4 5 6 7 8 9}	{2 3 4 6 7 8}	norm
slice/select,          Select slice,                   [& *3+{...} <&*4+{...}> ] <~ {3 4 5 6 7 8 9 0},                         {6 7 8 9},                      norm
slice/sugar,           Select sugar,                   {1 2 3 4 5 6 7 8 9 0}#3:*4+{ ... } ;,                                   {4 5 6 7},                      norm
lang/splitText,        Split English text,             {english-text:"Whoever says they can't is right"},                      {english-phrase:{Whoever says they can't is right}}, norm
temp/t1,               Merge many idents,              _=_=321,                                                                321,                            norm
time/t2,               Simple T-infon,                 *5+{T *256+_ = (*256+3) | 1 ...},                                       {T 1 *256+3 *256+3 *256+3 *256+3}, norm
prev/plus1,            Simple T-infon,                 *5+{T *256+_ = (%prev+1) | 3 ...},                                      {T 3 4 5 6 7},                  norm


query/getNthSub,       Get Nth item,                   {@myLst={'Cat' 'Hat' 'Bat' 'Dog'}}\n*3+[& myLst]//:'Bat',               -,                              multi
query/getNthSub2,      Get Nth item,                   {@Axx={bat:{cat:{dog:{eel:{fox:{AA:'aaa' BB:'bbb' CC:'ccc' DD:'ddd'}}}}}}}\n*3+[& Axx.bat.cat.dog.eel.fox]//:CC:'ccc', -, multi
word/types,            Check types1,                   {@position=_ @torque=_ @pedals={...}}\npedals:{position=321 torque:4}//:pedals:{position:321 torque:4}, -, multi
word/deref,            deref word1,                    {@tag={1 2 3}}\ntag//:tag:{1 2 3},                                      -,                              multi
query/firstType,       Find by type,                   {@wheel='wheel' @chain='chain' @seat='seat'}\n [&{wheel|...} chain]<~{wheel wheel chain seat}//:chain:'chain', -, multi
query/byType1,         Find by type,                   {@pedals={position:_ torque:_} @wheel={position:_ torque:_}}\n{pedals:{position:321 torque:4} wheel:{position:210 torque:5} breaks}.wheel//:wheel:{position:210 torque:5}, -, multi
query/chain,           Find by type,                   {@pedals={position:_ torque:_} @wheel={position:_ torque:_}}\n{pedals:{position:321 torque:4} wheel:{position:210 torque:5} breaks}.wheel.position//:position:210, -, multi
query/chain2,          Find by type,                   {@color=$ @length=_ @spoke={color:$ length:_} @wheel={position:_ torque:_ spoke} @pedals={position:_ torque:_} @bike={pedals wheel}}\n {pedals:{position:321 torque:4} wheel:{position:210 torque:5 spoke:{length:234}}}.wheel.spoke.length //:length:234, -, multi
query/chain3,          Find by type,                   {@color=$ @length=_ @spoke={color:$ length:_} @wheel={position:_ torque:_ spoke} @pedals={position:_ torque:_} @bike={pedals wheel}}\n {pedals:{position:321 torque:4} wheel:{position:210 torque:5 spoke:{length:_}}}.wheel.spoke.length=123; //:spoke:{length:123}, -, multi
query/chain4,          Find by type,                   {@color=$ @length=_ @spoke={color:$ length:_} @wheel={position:_ torque:_ spoke} @pedals={position:_ torque:_} @bike={pedals wheel}}\n {pedals:{position:321 torque:4} wheel:{position:210 torque:5 spoke:{length:_}}}.wheel.spoke.length=123;; //:wheel:{position:210 torque:5 spoke:{length:123}}, -, multi
parse/lang,            Complex parse,                  {@Fri='Fri'}\n{@Sat='Sat'}\n{@Sun='Sun'}\n{@statement=*_+[ Fri Sat Sun]}\n{@littleLang={statement|...}}\nlittleLang == 'SatFriSunSat'//:littleLang:{Sat:'Sat' Fri:'Fri' Sun:'Sun' Sat:'Sat'}, -, multi
outr/test1,            Outer test1,                    {shape| &*2+{circle|...} &*2+{square|...}}={1 2 3 4},                   -,                              multi
outr/everyOther1,      Every 2nd,                      {[_ _]| ...} <~ {1 2 3 4 5 6 7 8}//:{2 4 6 8},                          -,                              multi
outr/everyOther2,      Every 2nd,                      {*2+[...]| ...} <~ {1 2 3 4 5 6 7 8}//:{2 4 6 8},                          -,                              multi
unordered/seq1,        unordered in seq,               {@color={hue\, bright\, saturation}}\n{_ color _ _}={2 color:{hue:5\, bright:6\, saturation:7} 3 _=4}//:{2 color:{hue:5\, bright:6\, saturation:7} 3 4}, -, multi
cube/getLstSpec,       get list spec,                  %W.myStuff.rubixCube//:rubixCube:{{&thing}|  ... },                     -,                              world, cube.pr
user/getUser,          getUser,                        %W.bruceLong//:bruceLong:{1 2 3},                                       -,                              world, user.pr
user/getUser3,         getUser,                        %W.bruceLong.projects.properties.title//:title:'Slipstream Projects',   -,                              world, user_3.pr
user/getNth,           getNth,                         *2 + [& %W.bruceLong.projects.data.boardElement.data.TaskList.data].properties.member//:member:'Tiffany', -, world, user_3.pr
user/setNth,           setNth,                         *2 + [& %W.bruceLong.projects.data.boardElement.data.TaskList.data].properties.member <<= 'KTiffany'\n*2 + [& %W.bruceLong.projects.data.boardElement.data.TaskList.data].properties.member//:member:'KTiffany', -, world, user_3.pr
this/lvl1,             this lvl1,                      {@hue=_ @color={hue}  @red=color:{hue:0} @pink=color:{hue:345} @paper = paper:{red\, {{%.color}{pink}}}}\npaper//:paper:{red:{hue:0}\,{{red:{hue:0}} {pink:{hue:345}}}}, -, multi
word/setColor,         setColor,                       color=red//:color:{hue:0\, saturation:5\, brightness},                  -,                              world, bike.pr
search/adj1,           Search by adjective,            {{color=red\, ...} { name='Bob'\, color=blue\, ...} {color=green\, ...} {name='Wally'\, color=blue\, ...}}.{color=blue\, ...}//:{name:'Bob'\, color = blue\, ... }, -, world, bike.pr

marked/getNotLast,     Select with <...>,              [1 2 3 <4> 5 6],     4, norm
marked/lookahead,      Select w/lookahead,             [&{_|...} <_> $] <~ {33 22 11 'hi' 'bye' 66 55}, 11, norm
merge/lookahead,       Merge lookahead,                 {&{_|...} _ $} =: {33 22 11 'hi' 'bye' 66 55}, {&{_| 33 22} 11 'hi'}, norm

# Demo tests for foundational selection operations.
# Columns: name,display,input,expected,mode

# 1. Select a single item by ordinal/index using low-level bracket-list syntax.
query/getNth_lowlevel, Get Nth item low-level,         *3+['Cat' 'Hat' 'Bat' 'Dog'],                                           'Bat',                          norm

# 2. Select the same single item by ordinal/index using sugar.
query/getNth_sugar,    Get Nth item sugar,             {'Cat' 'Hat' 'Bat' 'Dog'}#3,                                            'Bat',                          norm

# 3. Select the first item matching a pattern/type using low-level query syntax.
query/firstStrB_lowlevel, Get first String low-level,  [&{_|...} $]<~{2 3 'Cat' 'Hat'},                                        'Cat',                          norm

# 4. Select the first item matching a pattern/type using sugar.
query/firstStrB_sugar, Get first String sugar,         {2 3 'Cat' 'Hat'}.$,                                                    'Cat',                          norm

# 5. Select a slice using the low-level query form.
slice/select_repro,    Select slice low-level,         [& *3+{...} <&*4+{...}> ] <~ {3 4 5 6 7 8 9 0},                         {6 7 8 9},                      norm

# 6. Select the same slice using sugar.
slice/sugar_repro,     Select slice sugar,             {3 4 5 6 7 8 9 0}#3:*4+{ ... } ;,                                       {6 7 8 9},                      norm

# 7. Assert ordered list contains a bike.
insert/ordered/random, Insert with no context,         %W#1={... bike ...} ,                                                   {&{ ... } bike ... },           world, TestFiles/insertIntoLists.pr

# 8. Assert ordered, closed list contains a bike.
insert/ordered/closed, Insert with no context,         %W#2={... bike ...} ,                                                   {cat dog},                      world, TestFiles/insertIntoLists.pr

# 9. Assert ordered, open list contains a bike.
insert/ordered/open,   Insert with no context,         %W#3={... bike ...} ,                                                   {cat dog &{ ... } bike ... },   world, TestFiles/insertIntoLists.pr

# 10. Assert unordered, open list contains a bike.
insert/unordered/closed, Insert with no context,       %W#4={bike\, ...} ,                                                     {cat\, dog\, bike\, ... },      world, TestFiles/insertIntoLists.pr

# 11. Assert unordered, open list contains only a bike.
insert/unordered/olny, Insert with no context,         %W#4={bike\,} ,                                                         {cat\, dog\, bike\, ... },      world, TestFiles/insertIntoLists.pr

# 12. Select the open insertion point after dog.
insert/ordered/afterDog, Insert after dog,             [& { ! dog | ...} dog <&{...}> ] <~ %W#5 ={... goat ...} \n %W#5//:{cat bat ... dog &{ ... } goat ... }, -,  world, TestFiles/insertIntoLists.pr

# 13. Select a later fixed-length slice by start index plus length using sugar.
slice/indexLength_sugar, Select index-to-length slice sugar, {0 1 2 3 4 5 6 7 8 9}#6:*3+{ ... } ;,                         {6 7 8},                        norm

# 14. Select the same later fixed-length slice by start index plus length using low-level query syntax.
slice/indexLength_lowlevel, Select index-to-length slice low-level, [& *6+{...} <&*3+{...}> ] <~ {0 1 2 3 4 5 6 7 8 9},                   {6 7 8},                        norm

# 15. Select the list items coming after dog without writing to the selected slice.
slice/afterDog_read, Select items after dog,          [& { ! dog | ...} dog <&{...}> ] <~ {cat bat ... dog goat pig},                     {goat pig},                      norm

# 16. Select the open insertion point after dog and assert an internally-structured object there.
slice/afterDog_assertObject, Assert object after dog, [& { ! dog | ...} dog <&{...}> ] <~ %W#5 ={... bike:{color=red} ...} \n %W#5//:{cat bat ... dog &{ ... } bike:{color = red} ... }, -, world, TestFiles/insertIntoLists.pr

# 17. Select the open insertion point after dog and assert a short sequence there.
slice/afterDog_assertSequence, Assert sequence after dog, [& { ! dog | ...} dog <&{...}> ] <~ %W#5 ={... goat bike ...} \n %W#5//:{cat bat ... dog &{ ... } goat bike ... }, -, world, TestFiles/insertIntoLists.pr

# Candidate cookbook tests once the corresponding slice-boundary syntax is committed:
# slice/beforeDog_read, Select items before dog,       [& <&{!dog|...}> dog {...} ] <~ {cat bat dog goat pig},                            {cat bat},                       norm
# slice/betweenCatDog_read, Select between cat and dog,[& { ! cat | ...} cat <&{...}> dog {...} ] <~ {cat bat dog goat pig},               {bat},                           norm
# slice/catThroughDog_read, Select cat through dog,    [& { ! cat | ...} <&{cat ... dog}> {...} ] <~ {ant cat bat dog eel},                {cat bat dog},                    norm
# slice/untilEvent_read, Select until event,           [& <&{!dog|...}> dog {...} ] <~ {cat bat dog goat pig},                            {cat bat},                       norm
# slice/fromEventForLength, Select after event length, [& { ! dog | ...} dog <&*2+{...}> ] <~ {cat bat dog goat pig eel},                  {goat pig},                      norm

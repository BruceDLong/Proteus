# Time/unit slice tests for Proteus cookbook examples.
# Columns: name,display,input,expected,mode
#
# These tests intentionally use small units so expected results stay readable:
#   second = base item type
#   minute = 3 seconds
#   hour   = 2 minutes = 6 seconds
#   day    = 2 hours = 4 minutes = 12 seconds
#
# The important technique is that the same sequence can be read or written
# through different unit listSpecs.  The small unit sizes are test fixtures;
# production definitions can use @minute=*60+{second|...}
# and @day=*1440+{minute|...} or equivalent.

# Shared definitions live in TestFiles/timeTestCases.pr:
# {@second=_ @minute=*3+{second|...} @hour=*2+{minute|...} @day=*2+{hour|...}}

# Hour spans defined in seconds, then read as minutes.
unit/hour/fromSeconds/readFirstMinute, Hour from seconds: first minute, hour:{second| 1 2 3 4 5 6}.minute, minute:{1 2 3}, world, TestFiles/timeTestCases.pr
unit/hour/fromSeconds/readSecondMinute, Hour from seconds: second minute, hour:{second| 1 2 3 4 5 6}#2, minute:{4 5 6}, world, TestFiles/timeTestCases.pr

# Hour spans defined in minutes, then read as seconds.
unit/hour/fromMinutes/readFirstSecond, Hour from minutes: first second, hour:{minute| minute:{second| 1 2 3} minute:{second| 4 5 6}}#1#1, 1, world, TestFiles/timeTestCases.pr
unit/hour/fromMinutes/readFourthSecond, Hour from minutes: fourth second, hour:{minute| minute:{second| 1 2 3} minute:{second| 4 5 6}}#2#1, 4, world, TestFiles/timeTestCases.pr

# Day spans defined in seconds, then read as hours and minutes.
unit/day/fromSeconds/readFirstHour, Day from seconds: first hour, day:{second| 1 2 3 4 5 6 7 8 9 10 11 12}.hour, hour:{minute:{1 2 3} minute:{4 5 6}}, world, TestFiles/timeTestCases.pr
unit/day/fromSeconds/readSecondHour, Day from seconds: second hour, day:{second| 1 2 3 4 5 6 7 8 9 10 11 12}#2, hour:{minute:{7 8 9} minute:{10 11 12}}, world, TestFiles/timeTestCases.pr
unit/day/fromSeconds/readThirdMinute, Day from seconds: third minute, day:{second| 1 2 3 4 5 6 7 8 9 10 11 12}#2#1, minute:{7 8 9}, world, TestFiles/timeTestCases.pr

# Day spans defined in minutes, then read as hours and seconds.
unit/day/fromMinutes/readFirstHour, Day from minutes: first hour, day:{minute| minute:{second| 1 2 3} minute:{second| 4 5 6} minute:{second| 7 8 9} minute:{second| 10 11 12}}.hour, hour:{minute:{1 2 3} minute:{4 5 6}}, world, TestFiles/timeTestCases.pr
unit/day/fromMinutes/readSecondHour, Day from minutes: second hour, day:{minute| minute:{second| 1 2 3} minute:{second| 4 5 6} minute:{second| 7 8 9} minute:{second| 10 11 12}}#2, hour:{minute:{7 8 9} minute:{10 11 12}}, world, TestFiles/timeTestCases.pr
unit/day/fromMinutes/readTenthSecond, Day from minutes: tenth second, day:{minute| minute:{second| 1 2 3} minute:{second| 4 5 6} minute:{second| 7 8 9} minute:{second| 10 11 12}}#2#2#1, 10, world, TestFiles/timeTestCases.pr

# Day spans defined in hours, then read as minutes and seconds.
unit/day/fromHours/readFirstMinute, Day from hours: first minute, day:{hour| hour:{minute| minute:{second| 1 2 3} minute:{second| 4 5 6}} hour:{minute| minute:{second| 7 8 9} minute:{second| 10 11 12}}}#1#1, minute:{1 2 3}, world, TestFiles/timeTestCases.pr
unit/day/fromHours/readEighthSecond, Day from hours: eighth second, day:{hour| hour:{minute| minute:{second| 1 2 3} minute:{second| 4 5 6}} hour:{minute| minute:{second| 7 8 9} minute:{second| 10 11 12}}}#2#1#2, 8, world, TestFiles/timeTestCases.pr
unit/day/fromHours/readMinuteByType, Day from hours: first minute by type, day:{hour| hour:{minute| minute:{second| 1 2 3} minute:{second| 4 5 6}} hour:{minute| minute:{second| 7 8 9} minute:{second| 10 11 12}}}.minute, minute:{1 2 3}, world, TestFiles/timeTestCases.pr

# Persistent world item: day is defined in seconds in the fixture, then accessed through larger units.
unit/world/dayFromSeconds/readSecondHour, World day from seconds: second hour, %W.day#2, hour:{minute:{7 8 9} minute:{10 11 12}}, world, TestFiles/timeTestCases.pr
unit/world/dayFromSeconds/writeNestedSecond, World day from seconds: write nested second, %W.day#2#1#2=80\n%W.day, day:{hour:{minute:{1 2 3} minute:{4 5 6}} hour:{minute:{7 80 9} minute:{10 11 12}}}, world, TestFiles/timeTestCases.pr

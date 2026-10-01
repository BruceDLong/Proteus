# Cube transition groundwork; run separately from defaultTests.tst.
# The real-world reads retain open oneOfs for unordered member selection.
# After-state checks await implementation of the agreed preservation rule.
world/cubie/positiveSibling, Known field beside positive coordinate, {#pose={xRot:0 xLoc:1.1} pose.xRot}, {#pose = {xRot:0 xLoc:1.1} xRot:0}, norm
world/cubie/negativeSibling, Known field beside negative coordinate, {#pose={xRot:0 xLoc:-1.1} pose.xRot}, {#pose = {xRot:0 xLoc:-1.1} xRot:0}, norm
world/cubie/negativeIntegerSibling, Known field beside negative integer, {#pose={xRot:0 xLoc:-1} pose.xRot}, {#pose = {xRot:0 xLoc:-1} xRot:0}, norm
world/cubie/ordinaryEqualityRejects, Temporal read is equality rather than replacement, {T *10+0 *10+1 = %tprev}, <REJECTED>, norm
world/cubie/beforeRotation, Read original cube state, %W.cubeTransition@[1]@[2]@[10].thing.orientation.xRot, *_+[xRot:0 ... ], world, TestFiles/rubixTransition.pr
world/cubie/beforeOtherRotation, Read original cube state, %W.cubeTransition@[1]@[2]@[10].thing.orientation.yRot, *_+[yRot:0 ... ], world, TestFiles/rubixTransition.pr
world/cubie/beforeLocation, Read original cube state, %W.cubeTransition@[1]@[2]@[10].thing.location.zLoc, *_+[zLoc:-1.1 ... ], world, TestFiles/rubixTransition.pr
world/cubie/beforeSiblingRotation, Read original cube state, %W.cubeTransition@[1]@[2]@[11].thing.orientation.xRot, *_+[xRot:0 ... ], world, TestFiles/rubixTransition.pr

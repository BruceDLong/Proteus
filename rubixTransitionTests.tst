# Cube transition groundwork; run separately from defaultTests.tst.
# The real-world reads retain open oneOfs for unordered member selection.
# After-state checks are acceptance tests for temporal preservation; failures remain visible.
world/cubie/positiveSibling, Known field beside positive coordinate, {#pose={xRot:0 xLoc:1.1} pose.xRot}, {#pose = {xRot:0 xLoc:1.1} xRot:0}, norm
world/cubie/negativeSibling, Known field beside negative coordinate, {#pose={xRot:0 xLoc:-1.1} pose.xRot}, {#pose = {xRot:0 xLoc:-1.1} xRot:0}, norm
world/cubie/negativeIntegerSibling, Known field beside negative integer, {#pose={xRot:0 xLoc:-1} pose.xRot}, {#pose = {xRot:0 xLoc:-1} xRot:0}, norm
world/cubie/ordinaryEqualityRejects, Temporal read is equality rather than replacement, {T *10+0 *10+1 = %tprev}, <REJECTED>, norm
world/cubie/beforeRotation, Read original cube state, %W.cubeTransition@[1].edgeCubes@[10].thing.orientation.xRot, *_+[xRot:0 ... ], world, TestFiles/rubixTransition.pr
world/cubie/beforeOtherRotation, Read original cube state, %W.cubeTransition@[1].edgeCubes@[10].thing.orientation.yRot, *_+[yRot:0 ... ], world, TestFiles/rubixTransition.pr
world/cubie/beforeLocation, Read original cube state, %W.cubeTransition@[1].edgeCubes@[10].thing.location.zLoc, *_+[zLoc:-1.1 ... ], world, TestFiles/rubixTransition.pr
world/cubie/beforeSiblingRotation, Read original cube state, %W.cubeTransition@[1].edgeCubes@[11].thing.orientation.xRot, *_+[xRot:0 ... ], world, TestFiles/rubixTransition.pr
world/cubie/beforeChangedRotation, First cubie starts unchanged, %W.cubeTransition@[1].edgeCubes@[1].thing.orientation.xRot, *_+[xRot:0 ... ], world, TestFiles/rubixTransition.pr
world/cubie/afterRotation, RHS changes only the later orientation, %W.cubeTransition@[2].edgeCubes@[1].thing.orientation.xRot, *_+[xRot:3.14 ... ], world, TestFiles/rubixTransition.pr
world/cubie/afterOtherRotation, Preserve the changed cubies other rotation, %W.cubeTransition@[2].edgeCubes@[1].thing.orientation.yRot, *_+[yRot:0 ... ], world, TestFiles/rubixTransition.pr
world/cubie/afterLocation, Preserve the changed cubies location, %W.cubeTransition@[2].edgeCubes@[1].thing.location.zLoc, *_+[zLoc:1.1 ... ], world, TestFiles/rubixTransition.pr
world/cubie/afterSiblingRotation, Preserve a different cubies orientation, %W.cubeTransition@[2].edgeCubes@[2].thing.orientation.xRot, *_+[xRot:0 ... ], world, TestFiles/rubixTransition.pr
world/cubie/afterSticker, Preserve the first cubies purple sticker, %W.cubeTransition@[2].edgeCubes@[1].sides@[4].color.hue, *_+[hue:160 ... ], world, TestFiles/rubixTransition.pr
world/cubie/beforeAfterChange, Querying the changed state cannot alter the first, %W.cubeTransition@[2].edgeCubes@[1].thing.orientation.xRot\n%W.cubeTransition@[1].edgeCubes@[1].thing.orientation.xRot//:*_+[xRot:0 ... ], -, world, TestFiles/rubixTransition.pr

world/rubixTransition/wrapped, GUI history wrapper, %W.wrapper.rubixCube@[2].rubixState.edgeCubes@[1].thing.orientation.xRot//:*_+[xRot:3.14 ... ]\n%W.wrapper.rubixCube@[2].rubixState.edgeCubes@[1].thing.orientation.yRot//:*_+[yRot:0 ... ]\n%W.wrapper.rubixCube@[2].rubixState.edgeCubes@[1].sides@[4].color.hue//:*_+[hue:160 ... ]\n%W.wrapper.rubixCube@[1].rubixState.edgeCubes@[1].thing.orientation.xRot//:*_+[xRot:0 ... ], -, world, TestFiles/rubixWrappedTransition.pr

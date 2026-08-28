#!/usr/bin/env python3
# Proteus active WorldManager rule case manager.
# Generates only WorldManagerRules.dog for WorldManager.dog.
from pprint import pprint
import re

debugMode = True

mergeSizeRules = {
    'ID': 'mergeSize',
    'points': [
        # TODO: ['Size-*', 'Size-/'], ["measurable", "!measurable"],l ["sGivn", !sGivn"],
        ['looseSize', '!looseSize'],
        ['lemUnknown', 'lemConcat', 'lemLiteral', 'lemIntersection'],
        ['remUnknown', 'remConcat', 'rsemLiteral', 'remIntersection'],

    ],
    'ifSnips': {
        'l?':            'aItem.LHS_item.pItem.viewMode == vmAny',
        'lNUM':          'aItem.LHS_item.pItem.value.overlayType == NUM',
        'lSTR':          'aItem.LHS_item.pItem.value.overlayType == STR',
        'lLST':          'aItem.LHS_item.pItem.value.overlayType == LST',

        'lemIntersection':    'aItem.LHS_item.pItem.infSize.evalMode == emIntersection',
        'lemUnknown':     'aItem.LHS_item.pItem.infSize.evalMode == emUnknown',
        'lemConcat':      'aItem.LHS_item.pItem.infSize.evalMode == emConcat',
        'lemLiteral':     'aItem.LHS_item.pItem.infSize.evalMode == emLiteral',

        'r?':            'aItem.RHS.pItem.viewMode == vmAny',
        'rNUM':          'aItem.RHS.pItem.value.overlayType == NUM',
        'rSTR':          'aItem.RHS.pItem.value.overlayType == STR',
        'rLST':          'aItem.RHS.pItem.value.overlayType == LST',

        'remIntersection':    'aItem.RHS.pItem.infSize.evalMode == emIntersection',
        'remUnknown':     'aItem.RHS.pItem.infSize.evalMode == emUnknown',
        'remConcat':      'aItem.RHS.pItem.infSize.evalMode == emConcat',
        'rsemLiteral':     'aItem.RHS.pItem.infSize.evalMode == emLiteral',

        'looseSize':     'aItem.looseSize',
        '!looseSize':    '!aItem.looseSize'
    },
    'codeSnips': {
        'copySizeRHStoLHS':         'if(!sizeCopyPlannedByReasoner and (aItem.LHS_item.pItem.viewMode!=vmOverlay or aItem.RHS.pItem.value.overlayType==aItem.LHS_item.pItem.value.overlayType) and (aItem.LHS_item.pItem.value.listSpec==NULL or !aItem.LHS_item.pItem.value.listSpec.asWrkLstOutr)){DO_COPY(aItem.RHS.pItem.infSize, aItem.LHS_item.pItem.infSize, 0)}',
    },
    'rules': [
        ["mergeSize:!looseSize|lemUnknown|rsemLiteral",     "copySizeRHStoLHS"],

    ]
}


mergeRules = {
    'ID': 'merge',
    'points': [
        ['l?', 'lNUM', 'lSTR', 'lLST', 'ltUnknown'],
        ['lemUnknown', 'lemLiteral'],
        ['=', '=='],
        ['r?', 'rNUM', 'rSTR', 'rLST', 'rtUnknown'],
        ['remUnknown', 'remLiteral']
    ],
    'ifSnips': {
        'l?':            'aItem.LHS_item.pItem.viewMode == vmAny',
        'lNUM':          'aItem.LHS_item.pItem.value.overlayType == NUM',
        'lSTR':          'aItem.LHS_item.pItem.value.overlayType == STR',
        'lLST':          'aItem.LHS_item.pItem.value.overlayType == LST',
        'ltUnknown':     'aItem.LHS_item.pItem.value.overlayType == tUnknown',

        'lemUnknown':     'aItem.LHS_item.pItem.value.evalMode == emUnknown',
        'lemConcat':      'aItem.LHS_item.pItem.value.evalMode == emConcat',
        'lemLiteral':     'aItem.LHS_item.pItem.value.evalMode == emLiteral',
        'lemIntersection':'aItem.LHS_item.pItem.value.evalMode == emIntersection',

        'r?':            'aItem.RHS.pItem.viewMode == vmAny',
        'rNUM':          'aItem.RHS.pItem.value.overlayType == NUM',
        'rSTR':          'aItem.RHS.pItem.value.overlayType == STR',
        'rLST':          'aItem.RHS.pItem.value.overlayType == LST',
        'rtUnknown':     'aItem.RHS.pItem.value.overlayType == tUnknown',

        'remUnknown':     'aItem.RHS.pItem.value.evalMode == emUnknown',
        'remConcat':      'aItem.RHS.pItem.value.evalMode == emConcat',
        'remLiteral':     'aItem.RHS.pItem.value.evalMode == emLiteral',
        'remIntersection':'aItem.RHS.pItem.value.evalMode == emIntersection',

        '==':           '(aItem.RHS.looseType())',
        '=':            '!(aItem.RHS.looseType())',
    },
    # The generated merge rule rows now contain only semantic traps. Executable
    # actions formerly named by this table are owned by explicit reasoner strategies.
    'codeSnips': {},
    'rules': [
        # TODO: Define the remaining inverted-any identity/value semantics.
        ["merge:l?||=|rNUM,rSTR,rLST|",           "NOT_IMPLEMENTED_YET"],
        ["merge:l?||==|rNUM,rSTR,rLST|",          "NOT_IMPLEMENTED_YET"],

        # TODO: Define merge semantics for concat/intersection combinations not
        # claimed by ConcatReasoner or IntersectionConsolidationStrategy.
        ["merge:|lemConcat,lemIntersection|||",     "NOT_IMPLEMENTED_YET"],
        ["merge:||||remConcat,remIntersection",     "NOT_IMPLEMENTED_YET"],

        # TODO: Define non-vmAny unknown-overlay identity and type propagation.
        ["merge:ltUnknown||||",                     "NOT_IMPLEMENTED_YET"],
        ["merge:l?,lNUM,lSTR,lLST|||rtUnknown|",    "NOT_IMPLEMENTED_YET"],

        # TODO: Define strict unknown/unknown constraint conjunction for the
        # reference, pending-work, and string-polarity cases not owned by Reasoner.dog.
        ["merge:lNUM|lemUnknown|=|rNUM|remUnknown",         "NOT_IMPLEMENTED_YET"],
        ["merge:lSTR|lemUnknown|=|rSTR|remUnknown",         "NOT_IMPLEMENTED_YET"],

        # TODO: Define inverted and unalignable sparse literal-list conjunction.
        ["merge:lLST|lemLiteral|=|rLST|remLiteral",         "NOT_IMPLEMENTED_YET"],

        # LooseSize
        # TODO: Define loose numeric-to-string/list conversion from a real use case.
        ["merge:lNUM||==|rSTR,rLST|remUnknown,remLiteral",   "NOT_IMPLEMENTED_YET"],
        # TODO: Define loose string-to-number/list conversion from a real use case.
        ["merge:lSTR||==|rNUM,rLST|",                      "NOT_IMPLEMENTED_YET"],
        # TODO: Define complemented loose list-to-scalar propagation.
        ["merge:lLST|lemUnknown,lemLiteral|==|rNUM,rSTR|",   "NOT_IMPLEMENTED_YET"],

        # TODO: Define loose numeric polarity, pending unknowns, and remainder behavior.
        ["merge:lNUM|lemUnknown|==|rNUM|remUnknown",         "NOT_IMPLEMENTED_YET"],
        ["merge:lNUM|lemUnknown|==|rNUM|remLiteral",         "NOT_IMPLEMENTED_YET"],
        ["merge:lNUM|lemLiteral|==|rNUM|remUnknown",         "NOT_IMPLEMENTED_YET"],
        # TODO: Define loose numeric literal width and remainder semantics from a real use case.
        ["merge:lNUM|lemLiteral|==|rNUM|remLiteral",         "NOT_IMPLEMENTED_YET"],

        # TODO: Define loose string polarity, pending unknowns, and remainder behavior.
        ["merge:lSTR|lemUnknown|==|rSTR|remUnknown",         "NOT_IMPLEMENTED_YET"],
        ["merge:lSTR|lemUnknown|==|rSTR|remLiteral",         "NOT_IMPLEMENTED_YET"],
        ["merge:lSTR|lemLiteral|==|rSTR|remUnknown",         "NOT_IMPLEMENTED_YET"],
        ["merge:lSTR|lemLiteral|==|rSTR|remLiteral",         "NOT_IMPLEMENTED_YET"],

        # TODO: Define complemented and unalignable sparse loose-list conjunction.
        ["merge:lLST|lemLiteral|==|rLST|remLiteral",         "NOT_IMPLEMENTED_YET"],

    ]
}
wrkLstRules = {
    'ID': 'wrkLst',
    'points': [["wrkLstEmpty", "!wrkLstEmpty"]],
    'ifSnips': {
        '!wrkLstEmpty':   '!aItem.LHS_item.pItem.wrkList.isEmpty()',
        'wrkLstEmpty':    '!aItem.hasPropagated'
    },
    'codeSnips': {
        'enqueueForMerge':  'enqueueForMerge(aItem); aItem.hasPropagated <- true'
    },
    'rules': [
        ["wrkLst:!wrkLstEmpty",     "enqueueForMerge"],
        ["wrkLst:wrkLstEmpty",      "enqueueForMerge"]
    ]
}
startPropRules = { # Start iterating emLiteral LST = emLiteral LST
    'ID': 'startProp',
    'points': [
        ["looseSize", "!looseSize"],
        ["sizesCompat", "!sizesCompat"],
        ["LHSEmpty", "!LHSEmpty"],
        ["RHSisPureDots", "!RHSisPureDots"]
       # ["merging", "!merging"]
    ],
    'ifSnips': {
        '!looseSize':       '!(aItem.RHS.looseType())',
        'looseSize':        '(aItem.RHS.looseType())',
        'sizesCompat':      'sizesAreCompatable(aItem.LHS_item.pItem, aItem.RHS.pItem)',
        '!sizesCompat':     '!sizesAreCompatable(aItem.LHS_item.pItem, aItem.RHS.pItem)',
        'RHSisPureDots':    '(aItem.RHS.pItem.value.tailUnfinished and aItem.RHS.pItem.value.items.size()==0)',
        '!RHSisPureDots':   '!(aItem.RHS.pItem.value.tailUnfinished and aItem.RHS.pItem.value.items.size()==0)',
        'LHSEmpty':         '(!aItem.LHS_item.pItem.value.tailUnfinished and aItem.LHS_item.pItem.value.items.size() == 0)',
        '!LHSEmpty':        '(aItem.LHS_item.pItem.value.tailUnfinished or aItem.LHS_item.pItem.value.items.size() > 0)'
    },
    'codeSnips': {
        'REJECT':   'aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true;',
        'SKIP':     '//Skip',
        'initListIterators':   'initListIterators(aItem); aItem.mergeStatus<-msUnknown',
    },
    'rules': [
        ["startProp:!looseSize|!sizesCompat||",                               "REJECT"],
        ["startProp:!looseSize|sizesCompat|LHSEmpty|!RHSisPureDots",          "SKIP"],
        ["startProp:!looseSize|sizesCompat||RHSisPureDots",                   "initListIterators"],
        ["startProp:!looseSize|sizesCompat|!LHSEmpty|!RHSisPureDots",         "initListIterators"], # Get first; account for #{}, ..., .first     "initListIterators"],
        ["startProp:looseSize|||",                                            "initListIterators"]
    ]
}
propagationRules = {
    'ID': 'propagation',
    'points': [["infonMode", "mergeMode"],["skipDots1", "skipDots2", "notSkipDots"]],
    'ifSnips': {
        'infonMode':    'aItem.ruleSet == rsInfon',
        'mergeMode':    'aItem.ruleSet == rsMerge',
        'skipDots1':    'aItem.LHS_item.pItem.',
        'skipDots2':    'aItem.',
        'notSkipDots':  ''

    },
    'codeSnips': {
        'getNextExtSkip':   '',
        'getNextExt':       '',
        '':    '',
        '':    ''

    },
    'rules': [
        ["propagation:infonMode|skipDots1", "getNextExtSkip"],
        ["propagation:infonMode|skipDots2", "getNextExtSkip"],
        ["propagation:infonMode|notSkipDots", "getNextExt"],

        ["propagation:mergeMode|skipDots1", "ACTION"],
        ["propagation:mergeMode|skipDots2", "ACTION"],
        ["propagation:mergeMode|notSkipDots", "ACTION"],
    ]
}
resolveRules = {
    'ID': 'resolve',
    'points': [["", ""]],
    'ifSnips': {
        '':   '',
        '':   ''
    },
    'codeSnips': {
        '':  ''
    },
    'rules': [
        ["resolve:",     "ACTION"],
        ["resolve:",     "ACTION"]
    ]
}
symbolRules = {
    'ID': 'symbol',
    'points': [["", ""]],
    'ifSnips': {
        '':   '',
        '':   ''
    },
    'codeSnips': {
        '':  ''
    },
    'rules': [
        ["symbol:",     "ACTION"],
        ["symbol:",     "ACTION"]
    ]
}
rangeRules = {
    'ID': 'range',
    'points': [["", ""]],
    'ifSnips': {
        '':   '',
        '':   ''
    },
    'codeSnips': {
        '':  ''
    },
    'rules': [
        ["range:",     "ACTION"],
        ["range:",     "ACTION"]
    ]
}
timeRules = {
    'ID': 'time',
    'points': [["", ""]],
    'ifSnips': {
        '':   '',
        '':   ''
    },
    'codeSnips': {
        '':  ''
    },
    'rules': [
        ["time:",     "ACTION"],
        ["time:",     "ACTION"]
    ]
}
wordRules = {
    'ID': 'word',
    'points': [["", ""]],
    'ifSnips': {
        '':   '',
        '':   ''
    },
    'codeSnips': {
        '':  ''
    },
    'rules': [
        ["word:",     "ACTION"],
        ["word:",     "ACTION"]
    ]
}
ruleSets = [
    mergeSizeRules,
    mergeRules,
    #wrkLstRules,
    startPropRules,
    #propagationRules,
    #resolveRules,
    #symbolRules,
    #rangeRules,
    #timeRules,
    #wordRules
]

def countCombinations(caseSpec):
    combos = 0;
    for toks in caseSpec:
        if isinstance(toks, str):
            combos += 1
        elif isinstance(toks, list):
            if combos==0: combos=1
            combos *= countCombinations(toks)
    return combos

def enumerateAllCombos(caseSpec):
    firstList = caseSpec[0]
    if len(caseSpec) > 1:
        secondList = enumerateAllCombos(caseSpec[1:])
        retList = []
        for left in firstList:
            for right in secondList:
                retList.append(left+'|'+right)
        return(retList)
    else: return firstList

def doesCaseMatchPattern(toMatch, case):
    caseSegs = case.split('|')
    numPSegs = len(toMatch)
    numCSegs = len(caseSegs)
    if numPSegs != numCSegs:
        print("numPSegs:", numPSegs)
        print("numCSegs:", numCSegs)
        print("ERROR: pattern and case lengths do not match:", toMatch, "\n\n", caseSegs)
        exit(1)
    for i in range(0, numCSegs):
        if not(caseSegs[i] in toMatch[i]):
            return(False)
    return(True)

def stripTags(rules):
    for rule in rules:
        ruleStr = rule[0]
        ruleStr = ruleStr[ruleStr.find(":")+1:]
        rule[0] = ruleStr
    return(rules)

def markHandledCases(ruleSetID, rules, cases, points):
    handledCount = 0
    for rule in rules:
        patternSegs = rule[0].split('|')
        toMatch = []
        idx = 0
        for pseg in patternSegs:
            if pseg =="":
                toMatch.append(points[idx])
            else:
                toMatch.append(pseg.split(','))
            idx += 1
        count = 0
        matchCount = 0
        for case in cases:
            if case[0:2] == "##": print("rules overlap:",case); exit(2)
            if case[0] == "#": caseToPass = case[1:]
            else: caseToPass = case
            if doesCaseMatchPattern(toMatch, caseToPass):
                #if cases[count] != caseToPass: print("cases != case:",case)
                cases[count] = "#"+case
                if case[0]=="#": print("rules overlap:",case); exit(2)
                matchCount += 1
            count +=1
        #print("matchCount:",matchCount)
        handledCount += matchCount
    print("Total cases - handled cases:" , len(cases), "-", handledCount, "=", len(cases) - handledCount, " ("+str(len(rules))+" "+ruleSetID+" Rules)")
    return(handledCount)

def genConditionCode(key, ifSnips):
    kSegs= key.split(',')
    S=""
    count=0
    for kSeg in kSegs:
        if not kSeg in ifSnips:
            print("key not found in genIfs:",kSeg)
            exit(2)
        if count > 0: S+=" or "
        S += ifSnips[kSeg]
        count += 1
    if count > 1: S = "("+S+")"
    return S

def genHandlerCode(ruleSetID, triggers, action, indent):
    handlerID = ruleSetID + ":" + triggers + "->" + action
    unsupported = "true" if action in ("ACTION", "NOT_IMPLEMENTED_YET") else "false"
    return indent + 'aItem.phase1RecordHandler("' + handlerID + '", ' + unsupported + ')\n'

def genActionCode(ruleSetID, codeKeyWords, rule, codeSnips, indent):
    handlerCode = genHandlerCode(ruleSetID, rule, codeKeyWords, indent)
    if codeKeyWords == "ACTION":
        if debugMode:
            actionCode = indent + '//:l/merge::log(indentStr(aItem.indentLvl)+"        TODO: unfinished")\n'
        else:
            actionCode = indent + "//TODO: unfinished\n"
        return(handlerCode + actionCode)
    if codeKeyWords == "NOT_IMPLEMENTED_YET":
        actionCode = indent + "// TODO: Implement when a real use case establishes the required semantics.\n"
        actionCode += indent + 'log("Not Implemented Yet: ' + ruleSetID + ':' + rule + '")\n'
        actionCode += indent + "logFlush()\n"
        actionCode += indent + "exit(2)\n"
        return(handlerCode + actionCode)
    if codeKeyWords == "NONE":
        if debugMode:
            actionCode = indent + '//:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+':'+rule+':Do Nothing")\n'
        else:
            actionCode = indent + "//Do Nothing\n"
        return(handlerCode + actionCode)
    actionCode = ""
    codeKeyWordList = codeKeyWords.split(",")
    for KW in codeKeyWordList:
        actionCode += indent + codeSnips[KW]+"\n"
    if debugMode:
        actionCode = indent+'//:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+'  '+rule+'\t'+KW+'")\n' + actionCode
    return(handlerCode + actionCode)

def genIfs(ruleSetID, ifsTree, binaryPts, ifSnips, codeSnips, indent = "        "):
    count =0
    S = ""
    if "__code" in ifsTree: return(genActionCode(ruleSetID, ifsTree["__code"], ifsTree["__rule"], codeSnips, indent))
    for key,value in ifsTree.items():
        if key in binaryPts and len(ifsTree) == 2:
            isBinary = True
        else: isBinary = False
        S += indent
        if isBinary:
            if count >0:
                S += "else"
            else:
                S += "if("
                S += genConditionCode(key, ifSnips)
                S += ")"
        else:
            if count >0:
                S += "else "
            S += "if("
            S += genConditionCode(key, ifSnips)
            S += ")"
        S += "{\n"
        S += genIfs(ruleSetID, value, binaryPts, ifSnips, codeSnips, indent + "    ")
        S += indent+"}\n"
        count += 1
        #print("KS:",key,S)
    return(S)

def genCodeFullIfs(ruleSetID, rules, ifSnips, codeSnips):
    S = ""
    indent = "        "
    ruleCount = 0
    #print(rules)
    #print(len(rules))
    for rule in rules:
        #print("ruleCount:", ruleCount)
        triggers    = rule[0]
        codeKeyWords     = rule[1]
        #print('triggers:',triggers)
        #print('codeKeyWords:',codeKeyWords)
        #if ruleCount > 9: break
        triggerList = triggers.split('|')
        conditionCode = ""
        condCount = 0
        for triggerList in triggerList:
            conditions = triggerList.split(',')
            count = 0
            subCount = 0
            subCode = ""
            for condition in conditions:
                if condition == 'merge':continue
                if condition == '': continue # any condition
            #    if condition == '=':continue
            #    if condition == '==':continue
                else:
                    #print (condition)
                    if subCount >0:
                        subCode += " or "
                    if condition not in ifSnips: print("ERROR: condition '"+condition+"' not in ifSnips for ruleSet '"+ruleSetID+"'\n"); exit(1)
                    subCode += ifSnips[condition]
                    subCount += 1
            if subCount > 1: subCode="("+subCode+")"
            if subCode != "":
                #print(subCode)
                if condCount > 0: conditionCode += " and "
                conditionCode += subCode
                condCount += 1
        if conditionCode != "":
            #print(conditionCode)
            actionCode = genHandlerCode(ruleSetID, triggers, codeKeyWords, indent + "    ")
            if codeKeyWords =='ACTION':
                if debugMode:
                    actionCode += indent + '    //:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+':'+triggers+':TODO: unfinished")\n'
                else:
                    actionCode += indent + "    //TODO: unfinished\n"
            elif codeKeyWords == "NOT_IMPLEMENTED_YET":
                actionCode += indent + "    // TODO: Implement when a real use case establishes the required semantics.\n"
                actionCode += indent + '    log("Not Implemented Yet: '+ruleSetID+':'+triggers+'")\n'
                actionCode += indent + "    logFlush()\n"
                actionCode += indent + "    exit(2)\n"
            elif codeKeyWords == "NONE":
                if debugMode:
                    actionCode += indent + '    //:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+':'+triggers+':Do Nothing")\n'
                else:
                    actionCode += indent + "    //Do Nothing\n"
            else:
                #print(codeKeyWords)
                codeKeyWordList = codeKeyWords.split(",")
                for KW in codeKeyWordList:
                    actionCode+= indent +"    " + codeSnips[KW]+"\n"
                if ruleSetID !="merge": actionCode+= indent +"    changeMade <- true\n"
                if debugMode:
                    actionLabel = rule[2] if len(rule) > 2 else KW
                    actionCode = indent+'    //:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+'  '+triggers+'\t'+actionLabel+'")\n' + actionCode
            if ruleCount >0: conditionKW = "else if"
            else: conditionKW = "if"
            conditionCode = conditionKW+"("+conditionCode+")"
            codeBody      = "{\n"+actionCode+indent+"}\n"
            S += indent+conditionCode+codeBody
            #print(conditionCode+codeBody)
        ruleCount +=1
    return(S)

def generateCode(ruleSetID, rules, binaryPts, ifSnips, codeSnips):
    topIfs = {}
    for rule in rules:
        crntIfs = topIfs
        for rSeg in rule[0].split("|"):
            if rSeg == "": continue
            if not rSeg in crntIfs:
                crntIfs[rSeg] = {}
            crntIfs = crntIfs[rSeg]
        crntIfs["__code"]=rule[1]
        crntIfs["__rule"]=rule[0]
    #pprint(topIfs)
    S = genIfs(ruleSetID, topIfs, binaryPts, ifSnips, codeSnips)
    return(S)

def pointIsBinary(pointSet):
    if len(pointSet)==2:
        if pointSet[0][:1] == "!" and pointSet[0][1:] == pointSet[1]:
            return(True)
        if pointSet[1][:1] == "!" and pointSet[1][1:] == pointSet[0]:
            return(True)
    return(False)

def addOwnershipAccess(code):
    nonOwningMembers = {'items', 'wrkList'}
    def addMarker(match):
        return match.group(0) if match.group(1) in nonOwningMembers else match.group(1) + '!.'
    return re.sub(r'([A-Za-z_][A-Za-z0-9_]*)\.(?=[A-Za-z_])', addMarker, code)

def generateMemberFunc(ruleSetID, points, rules, ifSnips, codeSnips):
    cases = enumerateAllCombos(points)
    #for case in cases: print(case)
    untagedRules = stripTags(rules)
    binaryPts = []
    for pointSet in points:
         if pointIsBinary(pointSet):
             for point in pointSet:
                binaryPts.append(point)
    markHandledCases(ruleSetID, untagedRules, cases, points)
    if ruleSetID =="merge":
        #print("ruleSetID:"+ruleSetID)
        ifsCode =  '        //if(aItem.LHS_item.accessMode==aRefTo){log("REF_TO:"+aItem.stringify())}\n'
        ifsCode += '        our POV: remainder <- NULL\n'
        ifsCode += '        logSeg(" mRUl")\n'
        ifsCode += genCodeFullIfs(ruleSetID, rules, ifSnips, codeSnips)
        ifsCode += '        else {aItem.phase1RecordHandler("merge:missing", true); log("MERGE_RULE_MISSING: "+ toString(aItem));log("          LHS overlayType:"+ overlayTypeStrings[aItem.LHS_item.pItem.value.overlayType]);log("          LHS evalMode:"+ evalModeStrings[aItem.LHS_item.pItem.value.evalMode]);log("          RHS overlayType:"+ overlayTypeStrings[aItem.RHS.pItem.value.overlayType]); log("EXITING"); exit(2);}\n'
        ifsCode += "        return(remainder)"
        funcCode = "    our POV: "+ruleSetID+"Rules(our AItem: aItem) <- {\n"+ifsCode+"\n    }\n"
    else:
        ifsCode =  "        me bool: changeMade <- false\n"
        if ruleSetID == "mergeSize":
            ifsCode += "        if(inheritDefinitionListShape(aItem)){changeMade <- true}\n"
        ifsCode += genCodeFullIfs(ruleSetID, rules, ifSnips, codeSnips)
        ifsCode += '        //else {log("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ '+ruleSetID+' RULE_MISSING");}\n'
        ifsCode += "        return(changeMade)"
        funcArgs = "our AItem: aItem"
        if ruleSetID == "mergeSize":
            funcArgs += ", me bool: sizeCopyPlannedByReasoner"
        funcCode = "    me bool: "+ruleSetID+"Rules("+funcArgs+") <- {\n"+ifsCode+"\n    }\n"
    return(funcCode)

def generateXformMgr(ruleSets):
    generatedHeader = """// AUTOGENERATED BY ruleMgr.py
// DO NOT EDIT
// Used by WorldManager.dog

"""
    structCode = generatedHeader + "struct WorldManager{\n"
    for ruleSet in ruleSets:
        funcCode = generateMemberFunc(ruleSet['ID'], ruleSet['points'], ruleSet['rules'], ruleSet['ifSnips'], ruleSet['codeSnips'])
        structCode += addOwnershipAccess(funcCode)
    structCode += "}"
    with open("WorldManagerRules.dog", "w") as text_file: print(structCode, file=text_file)

generateXformMgr(ruleSets)

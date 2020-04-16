#!/usr/bin/env python3
# Proteus Rule case manager
from pprint import pprint

sizeRules = {
    'ID': 'size',
    'points': [
        ["measurable", "!measurable"],
        ["sGivn", "s!Gvn"],
        ['fUnknown', 'fConcat', 'fLiteral', 'intersect']
        # TODO: ['Size-*', 'Size-/']
    ],
    'ifSnips': {
        '':   '',
        '':   ''
    },
    'codeSnips': {
        '':  ''
    },
    'rules': [
        ["size:",     "ACTION"],
        ["size:",     "ACTION"]
    ]
}

# Any infon: ?,NUM,STR,LST,LST.fUnknown,fConcat,fLiteral,intersect.Size-0-,Size-0-n,Size-n-m,Size-n,Size-n-,Size-Other
mergeRules = {
    'ID': 'merge',
    'points': [
        ['l?', 'lNUM', 'lSTR', 'lLST', 'lLST'],
        ['lintersect', 'lfUnknown', 'lfConcat', 'lfLiteral'],
        ['=', '=='],
        ['r?', 'rNUM', 'rSTR', 'rLST', 'rLST'],
        ['rintersect', 'rfUnknown', 'rfConcat', 'rfLiteral']
    ],
    'ifSnips': {
        'l?':            'aItem.LHS.infMode == isUnknown',
        'lNUM':          'aItem.LHS.value.fType == NUM',
        'lSTR':          'aItem.LHS.value.fType == STR',
        'lLST':          'aItem.LHS.value.fType == LST',

        'lintersect':    'aItem.LHS.value.intersectPosParse == ipSquareBrackets',
        'lfUnknown':     'aItem.LHS.value.format == fUnknown',
        'lfConcat':      'aItem.LHS.value.format == fConcat',
        'lfLiteral':     'aItem.LHS.value.format == fLiteral',

        'r?':            'aItem.RHS.infMode == isUnknown',
        'rNUM':          'aItem.RHS.value.fType == NUM',
        'rSTR':          'aItem.RHS.value.fType == STR',
        'rLST':          'aItem.RHS.value.fType == LST',

        'rintersect':    'aItem.RHS.value.intersectPosParse == ipSquareBrackets',
        'rfUnknown':     'aItem.RHS.value.format == fUnknown',
        'rfConcat':      'aItem.RHS.value.format == fConcat',
        'rfLiteral':     'aItem.RHS.value.format == fLiteral',

        '==':           'aItem.looseSize',
        '=':            '!aItem.looseSize',
    },
    'codeSnips': {
        'REJECT':                   'aItem.reject <- true',
        'copyValueRHStoLHS':        'DO_COPY(aItem.RHS.value, aItem.LHS.value, aItem.sizeToCopy)',
        'copyValueLHStoRHS':        'DO_COPY(aItem.LHS.value, aItem.RHS.value, aItem.sizeToCopy)',
        'copyRHSTypeToLHS':         'aItem.LHS.value.fType <- aItem.RHS.value.fType; aItem.LHS.infMode <- aItem.RHS.infMode',
        'copySizeRHStoLHS':         'DO_COPY(aItem.RHS.infSize, aItem.LHS.infSize, 0)',
        'rejectIfValueStrNotEqual': 'if(aItem.LHS.value.str != aItem.RHS.value.str){aItem.reject <- true}',
        'rejectIfValueNumNotEqual': 'if(aItem.LHS.value.num != aItem.RHS.value.num){aItem.reject <- true}',
        'StartMergePropogation':    'StartMergePropogation()'
    },
    'rules': [
        ["merge:|||r?|",                           "NULL"],
        ["merge:l?||=|rNUM,rSTR,rLST|",           "copyRHSTypeToLHS,copyValueRHStoLHS,copySizeRHStoLHS"],
        ["merge:l?||==|rNUM,rSTR,rLST|",          "copyRHSTypeToLHS,copyValueRHStoLHS"],

        ["merge:lNUM||=|rSTR,rLST|",             "REJECT"],
        ["merge:lSTR||=|rNUM,rLST|",             "REJECT"],
        ["merge:lLST||=|rNUM,rSTR|",             "REJECT"],

        ["merge:lNUM|lfUnknown|=|rNUM|rfUnknown",         "NULL"],
        ["merge:lNUM|lfUnknown|=|rNUM|rfLiteral",         "copyValueRHStoLHS"],
        ["merge:lNUM|lfLiteral|=|rNUM|rfUnknown",         "copyValueLHStoRHS"],
        ["merge:lNUM|lfLiteral|=|rNUM|rfLiteral",         "rejectIfValueNumNotEqual"],

        ["merge:lSTR|lfUnknown|=|rSTR|rfUnknown",         "NULL"],
        ["merge:lSTR|lfUnknown|=|rSTR|rfLiteral",         "copyValueRHStoLHS"],
        ["merge:lSTR|lfLiteral|=|rSTR|rfUnknown",         "copyValueLHStoRHS"],
        ["merge:lSTR|lfLiteral|=|rSTR|rfLiteral",         "rejectIfValueStrNotEqual"],

        ["merge:lLST|lfUnknown|=|rLST|rfUnknown",        "ACTION"],
        ["merge:lLST|lfUnknown|=|rLST|rfLiteral",        "ACTION"],
        ["merge:lLST|lfLiteral|=|rLST|rfUnknown",        "ACTION"],
        ["merge:lLST|lfLiteral|=|rLST|rfLiteral",        "StartMergePropogation"],

        # LooseSize
        ["merge:lNUM||==|rSTR,rLST|",             "ACTION"],
        ["merge:lSTR||==|rNUM,rLST|",             "ACTION"],
        ["merge:lLST||==|rNUM,rSTR|",             "ACTION"],

        ["merge:lNUM|lfUnknown|==|rNUM|rfUnknown",         "NULL"],
        ["merge:lNUM|lfUnknown|==|rNUM|rfLiteral",         "copyValueRHStoLHS"], # remember size to copy
        ["merge:lNUM|lfLiteral|==|rNUM|rfUnknown",         "copyValueLHStoRHS"],
        ["merge:lNUM|lfLiteral|==|rNUM|rfLiteral",         "ACTION"], #break into 2 cases: LHS.infSize.format = rfUnknown, rfLiteral.  see tryMergeValue()

        ["merge:lSTR|lfUnknown|==|rSTR|rfUnknown",         "NULL"],
        ["merge:lSTR|lfUnknown|==|rSTR|rfLiteral",         "copyValueRHStoLHS"], # sizeToCopy, handleRemainder
        ["merge:lSTR|lfLiteral|==|rSTR|rfUnknown",         "copyValueLHStoRHS"],
        ["merge:lSTR|lfLiteral|==|rSTR|rfLiteral",         "ACTION"],   #break into 2 cases: LHS.infSize.format = rfUnknown, rfLiteral.  see tryMergeValue()

        ["merge:lLST|lfUnknown|==|rLST|rfUnknown",        "ACTION"],
        ["merge:lLST|lfUnknown|==|rLST|rfLiteral",        "ACTION"],
        ["merge:lLST|lfLiteral|==|rLST|rfUnknown",        "ACTION"],
        ["merge:lLST|lfLiteral|==|rLST|rfLiteral",        "ACTION"],

        ##### CONCAT and INTERSECT
        ["merge:lNUM|lfUnknown|=|rNUM|rfConcat",          "ACTION"],
        ["merge:lNUM|lfUnknown|=|rNUM|rintersect",        "ACTION"],
        ["merge:lNUM|lfConcat|=|rNUM|rfUnknown",          "ACTION"],
        ["merge:lNUM|lfConcat|=|rNUM|rfConcat",           "ACTION"],
        ["merge:lNUM|lfConcat|=|rNUM|rfLiteral",          "ACTION"],
        ["merge:lNUM|lfConcat|=|rNUM|rintersect",         "ACTION"],
        ["merge:lNUM|lfLiteral|=|rNUM|rfConcat",          "ACTION"],
        ["merge:lNUM|lfLiteral|=|rNUM|rintersect",        "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|rfUnknown",        "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|rfConcat",         "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|rfLiteral",        "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|rintersect",       "ACTION"],

        ["merge:lSTR|lfUnknown|=|rSTR|rfConcat",          "ACTION"],
        ["merge:lSTR|lfUnknown|=|rSTR|rintersect",        "ACTION"],
        ["merge:lSTR|lfConcat|=|rSTR|rfUnknown",          "ACTION"],
        ["merge:lSTR|lfConcat|=|rSTR|rfConcat",           "ACTION"],
        ["merge:lSTR|lfConcat|=|rSTR|rfLiteral",          "ACTION"],
        ["merge:lSTR|lfConcat|=|rSTR|rintersect",         "ACTION"],
        ["merge:lSTR|lfLiteral|=|rSTR|rfConcat",          "ACTION"],
        ["merge:lSTR|lfLiteral|=|rSTR|rintersect",        "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|rfUnknown",        "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|rfConcat",         "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|rfLiteral",        "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|rintersect",       "ACTION"],

        ["merge:lLST|lfUnknown|=|rLST|rfConcat",         "ACTION"],
        ["merge:lLST|lfUnknown|=|rLST|rintersect",       "ACTION"],
        ["merge:lLST|lfConcat|=|rLST|rfUnknown",         "ACTION"],
        ["merge:lLST|lfConcat|=|rLST|rfConcat",          "ACTION"],
        ["merge:lLST|lfConcat|=|rLST|rfLiteral",         "ACTION"],
        ["merge:lLST|lfConcat|=|rLST|rintersect",        "ACTION"],
        ["merge:lLST|lfLiteral|=|rLST|rfConcat",         "ACTION"],
        ["merge:lLST|lfLiteral|=|rLST|rintersect",       "ACTION"],
        ["merge:lLST|lintersect|=|rLST|rfUnknown",       "ACTION"],
        ["merge:lLST|lintersect|=|rLST|rfConcat",        "ACTION"],
        ["merge:lLST|lintersect|=|rLST|rfLiteral",       "ACTION"],
        ["merge:lLST|lintersect|=|rLST|rintersect",      "ACTION"],

        # LooseSize
        ["merge:lNUM|lfUnknown|==|rNUM|rfConcat",          "ACTION"],
        ["merge:lNUM|lfUnknown|==|rNUM|rintersect",        "ACTION"],
        ["merge:lNUM|lfConcat|==|rNUM|rfUnknown",          "ACTION"],
        ["merge:lNUM|lfConcat|==|rNUM|rfConcat",           "ACTION"],
        ["merge:lNUM|lfConcat|==|rNUM|rfLiteral",          "ACTION"],
        ["merge:lNUM|lfConcat|==|rNUM|rintersect",         "ACTION"],
        ["merge:lNUM|lfLiteral|==|rNUM|rfConcat",          "ACTION"],
        ["merge:lNUM|lfLiteral|==|rNUM|rintersect",        "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|rfUnknown",        "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|rfConcat",         "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|rfLiteral",        "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|rintersect",       "ACTION"],

        ["merge:lSTR|lfUnknown|==|rSTR|rfConcat",          "ACTION"],
        ["merge:lSTR|lfUnknown|==|rSTR|rintersect",        "ACTION"],
        ["merge:lSTR|lfConcat|==|rSTR|rfUnknown",          "ACTION"],
        ["merge:lSTR|lfConcat|==|rSTR|rfConcat",           "ACTION"],
        ["merge:lSTR|lfConcat|==|rSTR|rfLiteral",          "ACTION"],
        ["merge:lSTR|lfConcat|==|rSTR|rintersect",         "ACTION"],
        ["merge:lSTR|lfLiteral|==|rSTR|rfConcat",          "ACTION"],
        ["merge:lSTR|lfLiteral|==|rSTR|rintersect",        "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|rfUnknown",        "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|rfConcat",         "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|rfLiteral",        "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|rintersect",       "ACTION"],

        ["merge:lLST|lfUnknown|==|rLST|rfConcat",         "ACTION"],
        ["merge:lLST|lfUnknown|==|rLST|rintersect",       "ACTION"],
        ["merge:lLST|lfConcat|==|rLST|rfUnknown",         "ACTION"],
        ["merge:lLST|lfConcat|==|rLST|rfConcat",          "ACTION"],
        ["merge:lLST|lfConcat|==|rLST|rfLiteral",         "ACTION"],
        ["merge:lLST|lfConcat|==|rLST|rintersect",        "ACTION"],
        ["merge:lLST|lfLiteral|==|rLST|rfConcat",         "ACTION"],
        ["merge:lLST|lfLiteral|==|rLST|rintersect",       "ACTION"],
        ["merge:lLST|lintersect|==|rLST|rfUnknown",       "ACTION"],
        ["merge:lLST|lintersect|==|rLST|rfConcat",        "ACTION"],
        ["merge:lLST|lintersect|==|rLST|rfLiteral",       "ACTION"],
        ["merge:lLST|lintersect|==|rLST|rintersect",      "ACTION"]
    ]
}
wrkLstRules = {
    'ID': 'wrkLst',
    'points': [["wrkLstIsEmpty", "wrkLstNotEmpty"]],
    'ifSnips': {
        'wrkLstNotEmpty':   '!aItem.item.wrkList.isEmpty()',
        'wrkLstIsEmpty':    'true'
    },
    'codeSnips': {
        'enqueueForMerge':  'enqueueForMerge(aItem)'
    },
    'rules': [
        ["wrkLst:wrkLstNotEmpty",     "enqueueForMerge"],
        ["wrkLst:wrkLstIsEmpty",      "NULL"]
    ]
}
startPropRules = { # Start iterating fLiteral LST = fLiteral LST
    'ID': 'startProp',
    'points': [
        ["looseSize", "!looseSize"],
        ["sizesCompat", "!sizesCompat"],
        ["LHSEmpty", "LHSisDots", "LHShasFirst"],
        ["RHSisPureDots", "!RHSisPureDots"]
       # ["merging", "!merging"]
    ],
    'ifSnips': {
        '!looseSize':   '',
        'looseSize':   '',
        'sizesCompat':   '',
        '!sizesCompat':   '',
        'RHSisPureDots':   '',
        '!RHSisPureDots':   '',
        'LHSEmpty':   '',
        'LHSisDots':   '',
        'LHShasFirst':   ''
    },
    'codeSnips': {
        'REJECT':   '',
        'SKIP':   '',
        'EnqueueFirstsToMerge':   '',
        '':  ''
    },
    'rules': [
        ["startProp:!looseSize|!sizesCompat||",                               "REJECT"],
        ["startProp:!looseSize|sizesCompat|LHSEmpty|!RHSisPureDots",          "SKIP"],
        ["startProp:!looseSize|sizesCompat||RHSisPureDots",                   "SKIP"],
        ["startProp:!looseSize|sizesCompat|LHSisDots|!RHSisPureDots",         "EnqueueFirstsToMerge"], # Get first; account for #{}, ..., .first
        ["startProp:!looseSize|sizesCompat|LHShasFirst|!RHSisPureDots",       "EnqueueFirstsToMerge"],
        ["startProp:looseSize|||",     "ACTION"]
    ]
}
propagationRules = {
    'ID': 'propagation',
    'points': [["infonMode", "mergeMode"],["skipDots1", "skipDots2", "notSkipDots"]],
    'ifSnips': {
        'infonMode':    'aItem.ruleSet == rsInfon',
        'mergeMode':    'aItem.ruleSet == rsMerge',
        'skipDots1':    'aItem.item.',
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
    #sizeRules,
    mergeRules,
    wrkLstRules,
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

def genActionCode(codeKeyWords, codeSnips, indent):
    if codeKeyWords == "ACTION": return(indent + "//TODO: unfinished\n")
    if codeKeyWords == "NULL": return(indent + "//Do Nothing\n")
    codeKeyWordList = codeKeyWords.split(",")
    S = ""
    for KW in codeKeyWordList:
        S+= indent + codeSnips[KW]+"\n"
    return(S)

def genIfs(ifsTree, ifSnips, codeSnips, indent = "        "):
    count =0
    S = ""
    if "__code" in ifsTree: return(genActionCode(ifsTree["__code"], codeSnips, indent))
    for key,value in ifsTree.items():
        S += indent
        if count >0: S += "else "
        S += "if("
        S += genConditionCode(key, ifSnips)
        S += "){\n"
        S += genIfs(value, ifSnips, codeSnips, indent + "    ")
        S += indent+"}\n"
        count += 1
        #print("KS:",key,S)
    return(S)

def generateCode(rules, ifSnips, codeSnips):
    topIfs = {}
    for rule in rules:
        crntIfs = topIfs
        for rSeg in rule[0].split("|"):
            if rSeg == "": continue
            if not rSeg in crntIfs:
                crntIfs[rSeg] = {}
            crntIfs = crntIfs[rSeg]
        crntIfs["__code"]=rule[1]
    #pprint(topIfs)
    S = genIfs(topIfs, ifSnips, codeSnips)
    return(S)

def generateMemberFunc(ruleSetID, points, rules, ifSnips, codeSnips):
    cases = enumerateAllCombos(points)
    #for case in cases: print(case)
    untagedRules = stripTags(rules)
    markHandledCases(ruleSetID, untagedRules, cases, points)
    ifsCode = generateCode(untagedRules, ifSnips, codeSnips)
    funcCode = "    void: "+ruleSetID+"Rules(our AItem: aItem) <- {\n"+ifsCode+"    }\n"
    return(funcCode)

def generateXformMgr(ruleSets):
    structCode = "struct XformMgr{\n"
    for ruleSet in ruleSets:
        funcCode = generateMemberFunc(ruleSet['ID'], ruleSet['points'], ruleSet['rules'], ruleSet['ifSnips'], ruleSet['codeSnips'])
        structCode+=funcCode
    structCode += "}"
    with open("xformMgr.dog", "w") as text_file: print(structCode, file=text_file)

generateXformMgr(ruleSets)

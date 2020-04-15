#!/usr/bin/env python3
# Proteus Rule case manager
from pprint import pprint

sizePoints = [
    [
    "measurable",
    "!measurable"
    ],

    [
    "sGivn",
    "s!Gvn"
    ],

    [
    'fUnknown',
    'fConcat',   # TODO: Concats with ... not counted.
    'fLiteral',
    'intersect'
    ]
    # TODO: ['Size-*', 'Size-/']
]
sizeRules = []
sizeIfSnips = []
sizeCodeSnips = {}

# Any infon: ?,NUM,STR,LST-u,LST-U.fUnknown,fConcat,fLiteral,intersect.Size-0-,Size-0-n,Size-n-m,Size-n,Size-n-,Size-Other
mergePoints = [
    ['l?', 'lNUM', 'lSTR', 'lLST-u', 'lLST-U'],
    ['lintersect', 'lfUnknown', 'lfConcat', 'lfLiteral'],
    ['=', '=='],
    ['r?', 'rNUM', 'rSTR', 'rLST-u', 'rLST-U'],
    ['rintersect', 'rfUnknown', 'rfConcat', 'rfLiteral']
]
mergeRules = [
    ["merge:|||r?|",                           "NULL"],
    ["merge:l?||=|rNUM,rSTR,rLST-U,rLST-u|",           "copyRHSTypeToLHS,copyValueRHStoLHS,copySizeRHStoLHS"],
    ["merge:l?||==|rNUM,rSTR,rLST-U,rLST-u|",          "copyRHSTypeToLHS,copyValueRHStoLHS"],

    ["merge:lNUM||=|rSTR,rLST-U,rLST-u|",             "REJECT"],   #Reject
    ["merge:lSTR||=|rNUM,rLST-U,rLST-u|",             "REJECT"],
    ["merge:lLST-U,lLST-u||=|rNUM,rSTR|",             "REJECT"],


    ["merge:lNUM|lfUnknown|=|rNUM|rfUnknown",         "NULL"],
    ["merge:lNUM|lfUnknown|=|rNUM|rfConcat",          "ACTION"],
    ["merge:lNUM|lfUnknown|=|rNUM|rfLiteral",         "copyValueRHStoLHS"],
    ["merge:lNUM|lfUnknown|=|rNUM|rintersect",        "ACTION"],

    ["merge:lNUM|lfConcat|=|rNUM|rfUnknown",          "ACTION"],
    ["merge:lNUM|lfConcat|=|rNUM|rfConcat",           "ACTION"],
    ["merge:lNUM|lfConcat|=|rNUM|rfLiteral",          "ACTION"],
    ["merge:lNUM|lfConcat|=|rNUM|rintersect",         "ACTION"],

    ["merge:lNUM|lfLiteral|=|rNUM|rfUnknown",         "copyValueLHStoRHS"],
    ["merge:lNUM|lfLiteral|=|rNUM|rfConcat",          "ACTION"],
    ["merge:lNUM|lfLiteral|=|rNUM|rfLiteral",         "rejectIfValuesNotEqual"],
    ["merge:lNUM|lfLiteral|=|rNUM|rintersect",        "ACTION"],

    ["merge:lNUM|lintersect|=|rNUM|rfUnknown",        "ACTION"],
    ["merge:lNUM|lintersect|=|rNUM|rfConcat",         "ACTION"],
    ["merge:lNUM|lintersect|=|rNUM|rfLiteral",        "ACTION"],
    ["merge:lNUM|lintersect|=|rNUM|rintersect",       "ACTION"],

    ["merge:lSTR|lfUnknown|=|rSTR|rfUnknown",         "NULL"],
    ["merge:lSTR|lfUnknown|=|rSTR|rfConcat",          "ACTION"],
    ["merge:lSTR|lfUnknown|=|rSTR|rfLiteral",         "copyValueRHStoLHS"],
    ["merge:lSTR|lfUnknown|=|rSTR|rintersect",        "ACTION"],

    ["merge:lSTR|lfConcat|=|rSTR|rfUnknown",          "ACTION"],
    ["merge:lSTR|lfConcat|=|rSTR|rfConcat",           "ACTION"],
    ["merge:lSTR|lfConcat|=|rSTR|rfLiteral",          "ACTION"],
    ["merge:lSTR|lfConcat|=|rSTR|rintersect",         "ACTION"],

    ["merge:lSTR|lfLiteral|=|rSTR|rfUnknown",         "copyValueLHStoRHS"],
    ["merge:lSTR|lfLiteral|=|rSTR|rfConcat",          "ACTION"],
    ["merge:lSTR|lfLiteral|=|rSTR|rfLiteral",         "rejectIfValuesNotEqual"],
    ["merge:lSTR|lfLiteral|=|rSTR|rintersect",        "ACTION"],

    ["merge:lSTR|lintersect|=|rSTR|rfUnknown",        "ACTION"],
    ["merge:lSTR|lintersect|=|rSTR|rfConcat",         "ACTION"],
    ["merge:lSTR|lintersect|=|rSTR|rfLiteral",        "ACTION"],
    ["merge:lSTR|lintersect|=|rSTR|rintersect",       "ACTION"],


    ["merge:lLST-U,lLST-u|lfUnknown|=|rLST-U,rLST-u|rfUnknown",        "ACTION"],
    ["merge:lLST-U,lLST-u|lfUnknown|=|rLST-U,rLST-u|rfConcat",         "ACTION"],
    ["merge:lLST-U,lLST-u|lfUnknown|=|rLST-U,rLST-u|rfLiteral",        "ACTION"],
    ["merge:lLST-U,lLST-u|lfUnknown|=|rLST-U,rLST-u|rintersect",       "ACTION"],

    ["merge:lLST-U,lLST-u|lfConcat|=|rLST-U,rLST-u|rfUnknown",         "ACTION"],
    ["merge:lLST-U,lLST-u|lfConcat|=|rLST-U,rLST-u|rfConcat",          "ACTION"],
    ["merge:lLST-U,lLST-u|lfConcat|=|rLST-U,rLST-u|rfLiteral",         "ACTION"],
    ["merge:lLST-U,lLST-u|lfConcat|=|rLST-U,rLST-u|rintersect",        "ACTION"],

    ["merge:lLST-U,lLST-u|lfLiteral|=|rLST-U,rLST-u|rfUnknown",        "ACTION"],
    ["merge:lLST-U,lLST-u|lfLiteral|=|rLST-U,rLST-u|rfConcat",         "ACTION"],
    ["merge:lLST-U,lLST-u|lfLiteral|=|rLST-U,rLST-u|rfLiteral",        "ACTION"],
    ["merge:lLST-U,lLST-u|lfLiteral|=|rLST-U,rLST-u|rintersect",       "ACTION"],

    ["merge:lLST-U,lLST-u|lintersect|=|rLST-U,rLST-u|rfUnknown",       "ACTION"],
    ["merge:lLST-U,lLST-u|lintersect|=|rLST-U,rLST-u|rfConcat",        "ACTION"],
    ["merge:lLST-U,lLST-u|lintersect|=|rLST-U,rLST-u|rfLiteral",       "ACTION"],
    ["merge:lLST-U,lLST-u|lintersect|=|rLST-U,rLST-u|rintersect",      "ACTION"],


    ["merge:lNUM||==|rSTR,rLST-U,rLST-u|",             "ACTION"],
    ["merge:lSTR||==|rNUM,rLST-U,rLST-u|",             "ACTION"],
    ["merge:lLST-U,lLST-u||==|rNUM,rSTR|",             "ACTION"],


    ["merge:lNUM|lfUnknown|==|rNUM|rfUnknown",         "NULL"],
    ["merge:lNUM|lfUnknown|==|rNUM|rfConcat",          "ACTION"],
    ["merge:lNUM|lfUnknown|==|rNUM|rfLiteral",         "copyValueRHStoLHS"], # remember size to copy
    ["merge:lNUM|lfUnknown|==|rNUM|rintersect",        "ACTION"],

    ["merge:lNUM|lfConcat|==|rNUM|rfUnknown",          "ACTION"],
    ["merge:lNUM|lfConcat|==|rNUM|rfConcat",           "ACTION"],
    ["merge:lNUM|lfConcat|==|rNUM|rfLiteral",          "ACTION"],
    ["merge:lNUM|lfConcat|==|rNUM|rintersect",         "ACTION"],

    ["merge:lNUM|lfLiteral|==|rNUM|rfUnknown",         "copyValueLHStoRHS"],
    ["merge:lNUM|lfLiteral|==|rNUM|rfConcat",          "ACTION"],
    ["merge:lNUM|lfLiteral|==|rNUM|rfLiteral",         "ACTION"], #break into 2 cases: LHS.infSize.format = rfUnknown, rfLiteral.  see tryMergeValue()
    ["merge:lNUM|lfLiteral|==|rNUM|rintersect",        "ACTION"],

    ["merge:lNUM|lintersect|==|rNUM|rfUnknown",        "ACTION"],
    ["merge:lNUM|lintersect|==|rNUM|rfConcat",         "ACTION"],
    ["merge:lNUM|lintersect|==|rNUM|rfLiteral",        "ACTION"],
    ["merge:lNUM|lintersect|==|rNUM|rintersect",       "ACTION"],


    ["merge:lSTR|lfUnknown|==|rSTR|rfUnknown",         "NULL"],
    ["merge:lSTR|lfUnknown|==|rSTR|rfConcat",          "ACTION"],
    ["merge:lSTR|lfUnknown|==|rSTR|rfLiteral",         "copyValueRHStoLHS"], # sizeToCopy, handleRemainder
    ["merge:lSTR|lfUnknown|==|rSTR|rintersect",        "ACTION"],

    ["merge:lSTR|lfConcat|==|rSTR|rfUnknown",          "ACTION"],
    ["merge:lSTR|lfConcat|==|rSTR|rfConcat",           "ACTION"],
    ["merge:lSTR|lfConcat|==|rSTR|rfLiteral",          "ACTION"],
    ["merge:lSTR|lfConcat|==|rSTR|rintersect",         "ACTION"],

    ["merge:lSTR|lfLiteral|==|rSTR|rfUnknown",         "copyValueLHStoRHS"],
    ["merge:lSTR|lfLiteral|==|rSTR|rfConcat",          "ACTION"],
    ["merge:lSTR|lfLiteral|==|rSTR|rfLiteral",         "ACTION"],   #break into 2 cases: LHS.infSize.format = rfUnknown, rfLiteral.  see tryMergeValue()
    ["merge:lSTR|lfLiteral|==|rSTR|rintersect",        "ACTION"],

    ["merge:lSTR|lintersect|==|rSTR|rfUnknown",        "ACTION"],
    ["merge:lSTR|lintersect|==|rSTR|rfConcat",         "ACTION"],
    ["merge:lSTR|lintersect|==|rSTR|rfLiteral",        "ACTION"],
    ["merge:lSTR|lintersect|==|rSTR|rintersect",       "ACTION"],


    ["merge:lLST-U,lLST-u|lfUnknown|==|rLST-U,rLST-u|rfUnknown",        "ACTION"],
    ["merge:lLST-U,lLST-u|lfUnknown|==|rLST-U,rLST-u|rfConcat",         "ACTION"],
    ["merge:lLST-U,lLST-u|lfUnknown|==|rLST-U,rLST-u|rfLiteral",        "ACTION"],
    ["merge:lLST-U,lLST-u|lfUnknown|==|rLST-U,rLST-u|rintersect",       "ACTION"],

    ["merge:lLST-U,lLST-u|lfConcat|==|rLST-U,rLST-u|rfUnknown",         "ACTION"],
    ["merge:lLST-U,lLST-u|lfConcat|==|rLST-U,rLST-u|rfConcat",          "ACTION"],
    ["merge:lLST-U,lLST-u|lfConcat|==|rLST-U,rLST-u|rfLiteral",         "ACTION"],
    ["merge:lLST-U,lLST-u|lfConcat|==|rLST-U,rLST-u|rintersect",        "ACTION"],

    ["merge:lLST-U,lLST-u|lfLiteral|==|rLST-U,rLST-u|rfUnknown",        "ACTION"],
    ["merge:lLST-U,lLST-u|lfLiteral|==|rLST-U,rLST-u|rfConcat",         "ACTION"],
    ["merge:lLST-U,lLST-u|lfLiteral|==|rLST-U,rLST-u|rfLiteral",        "ACTION"],
    ["merge:lLST-U,lLST-u|lfLiteral|==|rLST-U,rLST-u|rintersect",       "ACTION"],

    ["merge:lLST-U,lLST-u|lintersect|==|rLST-U,rLST-u|rfUnknown",       "ACTION"],
    ["merge:lLST-U,lLST-u|lintersect|==|rLST-U,rLST-u|rfConcat",        "ACTION"],
    ["merge:lLST-U,lLST-u|lintersect|==|rLST-U,rLST-u|rfLiteral",       "ACTION"],
    ["merge:lLST-U,lLST-u|lintersect|==|rLST-U,rLST-u|rintersect",      "ACTION"],
]
mergeIfSnips = {
    'l?':            'aItem.LHS.infMode == isUnknown',
    'lNUM':          'aItem.LHS.value.fType == NUM',
    'lSTR':          'aItem.LHS.value.fType == STR',
    'lLST-u':        '(aItem.LHS.value.fType == LST and aItem.LHS.value.tailUnfinished == false)',
    'lLST-U':        '(aItem.LHS.value.fType == LST and aItem.LHS.value.tailUnfinished == true)',

    'lintersect':    'aItem.LHS.value.intersectPosParse == ipSquareBrackets',
    'lfUnknown':     'aItem.LHS.value.format == fUnknown',
    'lfConcat':      'aItem.LHS.value.format == fConcat',
    'lfLiteral':     'aItem.LHS.value.format == fLiteral',

    'r?':            'aItem.RHS.infMode == isUnknown',
    'rNUM':          'aItem.RHS.value.fType == NUM',
    'rSTR':          'aItem.RHS.value.fType == STR',
    'rLST-u':        '(aItem.RHS.value.fType == LST and aItem.RHS.value.tailUnfinished == false)',
    'rLST-U':        '(aItem.RHS.value.fType == LST and aItem.RHS.value.tailUnfinished == true)',

    'rintersect':    'aItem.RHS.value.intersectPosParse == ipSquareBrackets',
    'rfUnknown':     'aItem.RHS.value.format == fUnknown',
    'rfConcat':      'aItem.RHS.value.format == fConcat',
    'rfLiteral':     'aItem.RHS.value.format == fLiteral',

    '==':           'aItem.looseSize',
    '=':            '!aItem.looseSize',
}
mergeCodeSnips = {
    'REJECT':                   'aItem.reject = true',
    'copyValueRHStoLHS':        '',
    'copyValueLHStoRHS':        '',
    'copyRHSTypeToLHS':         '',
    'copySizeRHStoLHS':         '',
    'rejectIfValuesNotEqual':   '',
}

ruleSets = [
    ["size", sizePoints, sizeRules, sizeIfSnips, sizeCodeSnips],
    ["merge", mergePoints, mergeRules, mergeIfSnips, mergeCodeSnips]
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
    structCode = "struct xformMgr{\n"
    for ruleSet in ruleSets:
        funcCode = generateMemberFunc(ruleSet[0], ruleSet[1], ruleSet[2], ruleSet[3], ruleSet[4])
        structCode+=funcCode
    structCode += "}"
    with open("xformMgr.dog", "w") as text_file: print(structCode, file=text_file)

generateXformMgr(ruleSets)

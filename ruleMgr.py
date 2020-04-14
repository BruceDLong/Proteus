#!/usr/bin/env python3
# Proteus Rule case manager
from pprint import pprint

ruleSetIDs = ['merge']

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

infonPoints = [
    [
    '?',
    'NUM',      # TODO: 'NUM_Inv',
    'STR',
    'LST-u',
    'LST-U'     # TODO: T-Lists
    ],

    [
    'intersect',
    'fUnknown',
    'fConcat',   # TODO: Concats with ... not counted.
    'fLiteral'
    ]
]

infonCodePoints = {
    '?':            'aItem.item.infMode == isUnknown',
    'NUM':          'aItem.item.value.fType == NUM',
    'STR':          'aItem.item.value.fType == STR',
    'LST-u':        '(aItem.item.value.fType == LST and aItem.item.value.tailUnfinished == false)',
    'LST-U':        '(aItem.item.value.fType == LST and aItem.item.value.tailUnfinished == true)',

    'intersect':    'aItem.item.value.intersectPosParse == ipSquareBrackets',
    'fUnknown':     'aItem.item.value.format == fUnknown',
    'fConcat':      'aItem.item.value.format == fConcat',
    'fLiteral':     'aItem.item.value.format == fLiteral',

    '==':           'looseSize()',
    '=':            '!looseSize()',
}
# Any infon: ?,NUM,STR,LST-u,LST-U.fUnknown,fConcat,fLiteral,intersect.Size-0-,Size-0-n,Size-n-m,Size-n,Size-n-,Size-Other
mergePoints =  infonPoints + [['=', '==']] + infonPoints


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

def markHandledCases(rules, cases, points):
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
    print("Handled cases:", handledCount)
    print("Remaining:", len(cases), "-", handledCount, "=", len(cases) - handledCount)
    print("Number of rules:", len(rules))
    return(handledCount)

def generateCode(rules):
    S = ""
    indent = "        "
    ruleCount = 0
    for rule in rules:
        if ruleCount > 9: break
        #print('rule:',rule)
        trigger    = rule[0]
        action     = rule[1]
        conditions = trigger.split('|')
        conditionCode = ""
        condCount = 0
        for condition in conditions:
            subConditions = condition.split(',')
            subCount = 0
            subCode = ""
            for subCondition in subConditions:
                if subCondition == 'merge': continue
                if subCondition == '': continue # any condition
                if subCondition == '=': continue
                if subCondition == '==': continue
                else:
                    if subCount > 0: subCode += " or "
                    subCode += infonCodePoints[subCondition]
                    subCount += 1
            if subCount > 1: subCode="("+subCode+")"
            if subCode != "":
                #print(subCode)
                if condCount > 0: conditionCode += " and "
                conditionCode += subCode
                condCount += 1
        if conditionCode != "":
            #print(conditionCode)
            actionCode = action
            if action =='DO_NOTHING':
                actionCode = '// do nothing'
            if ruleCount >0: conditionKW = "else if"
            else: conditionKW = "if"
            conditionCode = conditionKW+"("+conditionCode+")"
            codeBody      = "{\n    "+indent+"//"+actionCode+"\n"+indent+"}\n"
            S += indent+conditionCode+codeBody
            #print(conditionCode+codeBody)
        ruleCount +=1
    print(S)

def genConditionCode(key):
    kSegs= key.split(',')
    S=""
    count=0
    for kSeg in kSegs:
        if not kSeg in infonCodePoints:
            print("key not found in genIfs:",kSeg)
            exit(2)
        if count > 0: S+=" or "
        S += infonCodePoints[kSeg]
        count += 1
    if count > 1: S = "("+S+")"
    return S

def genIfs(ifsTree, indent = "        "):
    count =0
    S = ""
    if "__code" in ifsTree: return(indent+"// "+ifsTree["__code"]+"\n")
    for key,value in ifsTree.items():
        S += indent
        if count >0: S += "else "
        S += "if("
        S += genConditionCode(key)
        S += "){\n"
        S += genIfs(value, indent + "    ")
        S += indent+"}\n"
        count += 1
        #print("KS:",key,S)
    return(S)

def generateCode2(rules):
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
    S = genIfs(topIfs)
    print(S)

sizeRules = [
]

infRules = [
    ["merge:|||?|",                           "DO_NOTHING"],
    ["merge:?||=|NUM,STR,LST-U,LST-u|",           "copyRHSTypeToLHS,copyValueRHStoLHS,copySizeRHStoLHS"],
    ["merge:?||==|NUM,STR,LST-U,LST-u|",          "copyRHSTypeToLHS,copyValueRHStoLHS"],

    ["merge:NUM||=|STR,LST-U,LST-u|",             "REJECT"],   #Reject
    ["merge:STR||=|NUM,LST-U,LST-u|",             "REJECT"],
    ["merge:LST-U,LST-u||=|NUM,STR|",             "REJECT"],


    ["merge:NUM|fUnknown|=|NUM|fUnknown",         "DO_NOTHING"],
    ["merge:NUM|fUnknown|=|NUM|fConcat",          "ACTION"],
    ["merge:NUM|fUnknown|=|NUM|fLiteral",         "copyValueRHStoLHS"],
    ["merge:NUM|fUnknown|=|NUM|intersect",        "ACTION"],

    ["merge:NUM|fConcat|=|NUM|fUnknown",          "ACTION"],
    ["merge:NUM|fConcat|=|NUM|fConcat",           "ACTION"],
    ["merge:NUM|fConcat|=|NUM|fLiteral",          "ACTION"],
    ["merge:NUM|fConcat|=|NUM|intersect",         "ACTION"],

    ["merge:NUM|fLiteral|=|NUM|fUnknown",         "copyValueLHStoRHS"],
    ["merge:NUM|fLiteral|=|NUM|fConcat",          "ACTION"],
    ["merge:NUM|fLiteral|=|NUM|fLiteral",         "rejectIfValuesNotEqual"],
    ["merge:NUM|fLiteral|=|NUM|intersect",        "ACTION"],

    ["merge:NUM|intersect|=|NUM|fUnknown",        "ACTION"],
    ["merge:NUM|intersect|=|NUM|fConcat",         "ACTION"],
    ["merge:NUM|intersect|=|NUM|fLiteral",        "ACTION"],
    ["merge:NUM|intersect|=|NUM|intersect",       "ACTION"],


    ["merge:STR|fUnknown|=|STR|fUnknown",         "DO_NOTHING"],
    ["merge:STR|fUnknown|=|STR|fConcat",          "ACTION"],
    ["merge:STR|fUnknown|=|STR|fLiteral",         "copyValueRHStoLHS"],
    ["merge:STR|fUnknown|=|STR|intersect",        "ACTION"],

    ["merge:STR|fConcat|=|STR|fUnknown",          "ACTION"],
    ["merge:STR|fConcat|=|STR|fConcat",           "ACTION"],
    ["merge:STR|fConcat|=|STR|fLiteral",          "ACTION"],
    ["merge:STR|fConcat|=|STR|intersect",         "ACTION"],

    ["merge:STR|fLiteral|=|STR|fUnknown",         "copyValueLHStoRHS"],
    ["merge:STR|fLiteral|=|STR|fConcat",          "ACTION"],
    ["merge:STR|fLiteral|=|STR|fLiteral",         "rejectIfValuesNotEqual"],
    ["merge:STR|fLiteral|=|STR|intersect",        "ACTION"],

    ["merge:STR|intersect|=|STR|fUnknown",        "ACTION"],
    ["merge:STR|intersect|=|STR|fConcat",         "ACTION"],
    ["merge:STR|intersect|=|STR|fLiteral",        "ACTION"],
    ["merge:STR|intersect|=|STR|intersect",       "ACTION"],


    ["merge:LST-U,LST-u|fUnknown|=|LST-U,LST-u|fUnknown",        "ACTION"],
    ["merge:LST-U,LST-u|fUnknown|=|LST-U,LST-u|fConcat",         "ACTION"],
    ["merge:LST-U,LST-u|fUnknown|=|LST-U,LST-u|fLiteral",        "ACTION"],
    ["merge:LST-U,LST-u|fUnknown|=|LST-U,LST-u|intersect",       "ACTION"],

    ["merge:LST-U,LST-u|fConcat|=|LST-U,LST-u|fUnknown",         "ACTION"],
    ["merge:LST-U,LST-u|fConcat|=|LST-U,LST-u|fConcat",          "ACTION"],
    ["merge:LST-U,LST-u|fConcat|=|LST-U,LST-u|fLiteral",         "ACTION"],
    ["merge:LST-U,LST-u|fConcat|=|LST-U,LST-u|intersect",        "ACTION"],

    ["merge:LST-U,LST-u|fLiteral|=|LST-U,LST-u|fUnknown",        "ACTION"],
    ["merge:LST-U,LST-u|fLiteral|=|LST-U,LST-u|fConcat",         "ACTION"],
    ["merge:LST-U,LST-u|fLiteral|=|LST-U,LST-u|fLiteral",        "ACTION"],
    ["merge:LST-U,LST-u|fLiteral|=|LST-U,LST-u|intersect",       "ACTION"],

    ["merge:LST-U,LST-u|intersect|=|LST-U,LST-u|fUnknown",       "ACTION"],
    ["merge:LST-U,LST-u|intersect|=|LST-U,LST-u|fConcat",        "ACTION"],
    ["merge:LST-U,LST-u|intersect|=|LST-U,LST-u|fLiteral",       "ACTION"],
    ["merge:LST-U,LST-u|intersect|=|LST-U,LST-u|intersect",      "ACTION"],


    ["merge:NUM||==|STR,LST-U,LST-u|",             "ACTION"],
    ["merge:STR||==|NUM,LST-U,LST-u|",             "ACTION"],
    ["merge:LST-U,LST-u||==|NUM,STR|",             "ACTION"],


    ["merge:NUM|fUnknown|==|NUM|fUnknown",         "DO_NOTHING"],
    ["merge:NUM|fUnknown|==|NUM|fConcat",          "ACTION"],
    ["merge:NUM|fUnknown|==|NUM|fLiteral",         "copyValueRHStoLHS"], # remember size to copy
    ["merge:NUM|fUnknown|==|NUM|intersect",        "ACTION"],

    ["merge:NUM|fConcat|==|NUM|fUnknown",          "ACTION"],
    ["merge:NUM|fConcat|==|NUM|fConcat",           "ACTION"],
    ["merge:NUM|fConcat|==|NUM|fLiteral",          "ACTION"],
    ["merge:NUM|fConcat|==|NUM|intersect",         "ACTION"],

    ["merge:NUM|fLiteral|==|NUM|fUnknown",         "copyValueLHStoRHS"],
    ["merge:NUM|fLiteral|==|NUM|fConcat",          "ACTION"],
    ["merge:NUM|fLiteral|==|NUM|fLiteral",         "ACTION"], #break into 2 cases: LHS.infSize.format = fUnknown, fLiteral.  see tryMergeValue()
    ["merge:NUM|fLiteral|==|NUM|intersect",        "ACTION"],

    ["merge:NUM|intersect|==|NUM|fUnknown",        "ACTION"],
    ["merge:NUM|intersect|==|NUM|fConcat",         "ACTION"],
    ["merge:NUM|intersect|==|NUM|fLiteral",        "ACTION"],
    ["merge:NUM|intersect|==|NUM|intersect",       "ACTION"],


    ["merge:STR|fUnknown|==|STR|fUnknown",         "DO_NOTHING"],
    ["merge:STR|fUnknown|==|STR|fConcat",          "ACTION"],
    ["merge:STR|fUnknown|==|STR|fLiteral",         "copyValueRHStoLHS"], # sizeToCopy, handleRemainder
    ["merge:STR|fUnknown|==|STR|intersect",        "ACTION"],

    ["merge:STR|fConcat|==|STR|fUnknown",          "ACTION"],
    ["merge:STR|fConcat|==|STR|fConcat",           "ACTION"],
    ["merge:STR|fConcat|==|STR|fLiteral",          "ACTION"],
    ["merge:STR|fConcat|==|STR|intersect",         "ACTION"],

    ["merge:STR|fLiteral|==|STR|fUnknown",         "copyValueLHStoRHS"],
    ["merge:STR|fLiteral|==|STR|fConcat",          "ACTION"],
    ["merge:STR|fLiteral|==|STR|fLiteral",         "ACTION"],   #break into 2 cases: LHS.infSize.format = fUnknown, fLiteral.  see tryMergeValue()
    ["merge:STR|fLiteral|==|STR|intersect",        "ACTION"],

    ["merge:STR|intersect|==|STR|fUnknown",        "ACTION"],
    ["merge:STR|intersect|==|STR|fConcat",         "ACTION"],
    ["merge:STR|intersect|==|STR|fLiteral",        "ACTION"],
    ["merge:STR|intersect|==|STR|intersect",       "ACTION"],


    ["merge:LST-U,LST-u|fUnknown|==|LST-U,LST-u|fUnknown",        "ACTION"],
    ["merge:LST-U,LST-u|fUnknown|==|LST-U,LST-u|fConcat",         "ACTION"],
    ["merge:LST-U,LST-u|fUnknown|==|LST-U,LST-u|fLiteral",        "ACTION"],
    ["merge:LST-U,LST-u|fUnknown|==|LST-U,LST-u|intersect",       "ACTION"],

    ["merge:LST-U,LST-u|fConcat|==|LST-U,LST-u|fUnknown",         "ACTION"],
    ["merge:LST-U,LST-u|fConcat|==|LST-U,LST-u|fConcat",          "ACTION"],
    ["merge:LST-U,LST-u|fConcat|==|LST-U,LST-u|fLiteral",         "ACTION"],
    ["merge:LST-U,LST-u|fConcat|==|LST-U,LST-u|intersect",        "ACTION"],

    ["merge:LST-U,LST-u|fLiteral|==|LST-U,LST-u|fUnknown",        "ACTION"],
    ["merge:LST-U,LST-u|fLiteral|==|LST-U,LST-u|fConcat",         "ACTION"],
    ["merge:LST-U,LST-u|fLiteral|==|LST-U,LST-u|fLiteral",        "ACTION"],
    ["merge:LST-U,LST-u|fLiteral|==|LST-U,LST-u|intersect",       "ACTION"],

    ["merge:LST-U,LST-u|intersect|==|LST-U,LST-u|fUnknown",       "ACTION"],
    ["merge:LST-U,LST-u|intersect|==|LST-U,LST-u|fConcat",        "ACTION"],
    ["merge:LST-U,LST-u|intersect|==|LST-U,LST-u|fLiteral",       "ACTION"],
    ["merge:LST-U,LST-u|intersect|==|LST-U,LST-u|intersect",      "ACTION"],

]

print("COMBOS:", countCombinations(sizePoints))
print("COMBOS:", countCombinations(infonPoints))
print("COMBOS:", countCombinations(mergePoints))
sizeCases = enumerateAllCombos(sizePoints)
infCases = enumerateAllCombos(mergePoints)
#for case in sizeCases: print(case)
untagRules = stripTags(infRules)
markHandledCases(untagRules, infCases, mergePoints)
#markHandledCases(sizeRules, sizeCases, sizePoints)

generateCode2(untagRules)

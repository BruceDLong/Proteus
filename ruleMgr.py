#!/usr/bin/env python3
# Proteus Rule case manager

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
    '?':            'infMode == isUnknown',
    'NUM':          'value.fType == NUM',
    'STR':          'value.fType == STR',
    'LST-u':        'value.fType == LST and value.tailUnfinished = false',
    'LST-U':        'value.fType == LST and value.tailUnfinished = true',

    'intersect':    'value.intersectPosParse == ipSquareBrackets',
    'fUnknown':     'value.fromat == fUnknown',
    'fConcat':      'value.fromat == fConcat',
    'fLiteral':     'value.fromat == fLiteral',
}
# Any infon: ?,NUM,STR,LST-u,LST-U.fUnknown,fConcat,fLiteral,intersect.Size-0-,Size-0-n,Size-n-m,Size-n,Size-n-,Size-Other
mergePoints =  [['merge']] + infonPoints + [['=', '==']] + infonPoints


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
    ruleCount = 0
    for rule in rules:
        #if ruleCount > 7: break
        #print('rule:',rule)
        trigger    = rule[0]
        action     = rule[1]
        conditions = trigger.split('|')
        conditionCode = ""
        for condition in conditions:
            subConditions = condition.split(',')
            count = 0
            for subCondition in subConditions:
                if subCondition == 'merge':
                    continue
                if subCondition == '': continue # any condition
                if subCondition == '=':
                    continue
                if subCondition == '==':
                    continue
                else:
                    if count > 0: conditionCode += " or "
                    conditionCode += infonCodePoints[subCondition]
                count += 1
        if conditionCode != "":
            actionCode = action
            if action =='DO_NOTHING':
                actionCode = '// do nothing'
            if ruleCount >0: conditionKW = "else if"
            else: conditionKW = "if"
            conditionCode = conditionKW+"("+conditionCode+")"
            codeBody      = "{\n //"+actionCode+"\n}"
            print(conditionCode+codeBody)
        ruleCount +=1

sizeRules = [
]

infRules = [
    ["merge|||=,==|?|",                           "DO_NOTHING"],
    ["merge|?||=|NUM,STR,LST-U,LST-u|",           "copyRHSTypeToLHS,copyValueRHStoLHS,copySizeRHStoLHS"],
    ["merge|?||==|NUM,STR,LST-U,LST-u|",          "copyRHSTypeToLHS,copyValueRHStoLHS"],

    ["merge|NUM||=|STR,LST-U,LST-u|",             "REJECT"],   #Reject
    ["merge|STR||=|NUM,LST-U,LST-u|",             "REJECT"],
    ["merge|LST-U,LST-u||=|NUM,STR|",             "REJECT"],


    ["merge|NUM|fUnknown|=|NUM|fUnknown",         "DO_NOTHING"],
    ["merge|NUM|fUnknown|=|NUM|fConcat",          "ACTION"],
    ["merge|NUM|fUnknown|=|NUM|fLiteral",         "copyValueRHStoLHS"],
    ["merge|NUM|fUnknown|=|NUM|intersect",        "ACTION"],

    ["merge|NUM|fConcat|=|NUM|fUnknown",          "ACTION"],
    ["merge|NUM|fConcat|=|NUM|fConcat",           "ACTION"],
    ["merge|NUM|fConcat|=|NUM|fLiteral",          "ACTION"],
    ["merge|NUM|fConcat|=|NUM|intersect",         "ACTION"],

    ["merge|NUM|fLiteral|=|NUM|fUnknown",         "copyValueLHStoRHS"],
    ["merge|NUM|fLiteral|=|NUM|fConcat",          "ACTION"],
    ["merge|NUM|fLiteral|=|NUM|fLiteral",         "rejectIfValuesNotEqual"],
    ["merge|NUM|fLiteral|=|NUM|intersect",        "ACTION"],

    ["merge|NUM|intersect|=|NUM|fUnknown",        "ACTION"],
    ["merge|NUM|intersect|=|NUM|fConcat",         "ACTION"],
    ["merge|NUM|intersect|=|NUM|fLiteral",        "ACTION"],
    ["merge|NUM|intersect|=|NUM|intersect",       "ACTION"],


    ["merge|STR|fUnknown|=|STR|fUnknown",         "DO_NOTHING"],
    ["merge|STR|fUnknown|=|STR|fConcat",          "ACTION"],
    ["merge|STR|fUnknown|=|STR|fLiteral",         "copyValueRHStoLHS"],
    ["merge|STR|fUnknown|=|STR|intersect",        "ACTION"],

    ["merge|STR|fConcat|=|STR|fUnknown",          "ACTION"],
    ["merge|STR|fConcat|=|STR|fConcat",           "ACTION"],
    ["merge|STR|fConcat|=|STR|fLiteral",          "ACTION"],
    ["merge|STR|fConcat|=|STR|intersect",         "ACTION"],

    ["merge|STR|fLiteral|=|STR|fUnknown",         "copyValueLHStoRHS"],
    ["merge|STR|fLiteral|=|STR|fConcat",          "ACTION"],
    ["merge|STR|fLiteral|=|STR|fLiteral",         "rejectIfValuesNotEqual"],
    ["merge|STR|fLiteral|=|STR|intersect",        "ACTION"],

    ["merge|STR|intersect|=|STR|fUnknown",        "ACTION"],
    ["merge|STR|intersect|=|STR|fConcat",         "ACTION"],
    ["merge|STR|intersect|=|STR|fLiteral",        "ACTION"],
    ["merge|STR|intersect|=|STR|intersect",       "ACTION"],


    ["merge|LST-U,LST-u|fUnknown|=|LST-U,LST-u|fUnknown",        "ACTION"],
    ["merge|LST-U,LST-u|fUnknown|=|LST-U,LST-u|fConcat",         "ACTION"],
    ["merge|LST-U,LST-u|fUnknown|=|LST-U,LST-u|fLiteral",        "ACTION"],
    ["merge|LST-U,LST-u|fUnknown|=|LST-U,LST-u|intersect",       "ACTION"],

    ["merge|LST-U,LST-u|fConcat|=|LST-U,LST-u|fUnknown",         "ACTION"],
    ["merge|LST-U,LST-u|fConcat|=|LST-U,LST-u|fConcat",          "ACTION"],
    ["merge|LST-U,LST-u|fConcat|=|LST-U,LST-u|fLiteral",         "ACTION"],
    ["merge|LST-U,LST-u|fConcat|=|LST-U,LST-u|intersect",        "ACTION"],

    ["merge|LST-U,LST-u|fLiteral|=|LST-U,LST-u|fUnknown",        "ACTION"],
    ["merge|LST-U,LST-u|fLiteral|=|LST-U,LST-u|fConcat",         "ACTION"],
    ["merge|LST-U,LST-u|fLiteral|=|LST-U,LST-u|fLiteral",        "ACTION"],
    ["merge|LST-U,LST-u|fLiteral|=|LST-U,LST-u|intersect",       "ACTION"],

    ["merge|LST-U,LST-u|intersect|=|LST-U,LST-u|fUnknown",       "ACTION"],
    ["merge|LST-U,LST-u|intersect|=|LST-U,LST-u|fConcat",        "ACTION"],
    ["merge|LST-U,LST-u|intersect|=|LST-U,LST-u|fLiteral",       "ACTION"],
    ["merge|LST-U,LST-u|intersect|=|LST-U,LST-u|intersect",      "ACTION"],


    ["merge|NUM||==|STR,LST-U,LST-u|",             "ACTION"],
    ["merge|STR||==|NUM,LST-U,LST-u|",             "ACTION"],
    ["merge|LST-U,LST-u||==|NUM,STR|",             "ACTION"],


    ["merge|NUM|fUnknown|==|NUM|fUnknown",         "DO_NOTHING"],
    ["merge|NUM|fUnknown|==|NUM|fConcat",          "ACTION"],
    ["merge|NUM|fUnknown|==|NUM|fLiteral",         "copyValueRHStoLHS"], # remember size to copy
    ["merge|NUM|fUnknown|==|NUM|intersect",        "ACTION"],

    ["merge|NUM|fConcat|==|NUM|fUnknown",          "ACTION"],
    ["merge|NUM|fConcat|==|NUM|fConcat",           "ACTION"],
    ["merge|NUM|fConcat|==|NUM|fLiteral",          "ACTION"],
    ["merge|NUM|fConcat|==|NUM|intersect",         "ACTION"],

    ["merge|NUM|fLiteral|==|NUM|fUnknown",         "copyValueLHStoRHS"],
    ["merge|NUM|fLiteral|==|NUM|fConcat",          "ACTION"],
    ["merge|NUM|fLiteral|==|NUM|fLiteral",         "ACTION"], #break into 2 cases: LHS.infSize.format = fUnknown, fLiteral.  see tryMergeValue()
    ["merge|NUM|fLiteral|==|NUM|intersect",        "ACTION"],

    ["merge|NUM|intersect|==|NUM|fUnknown",        "ACTION"],
    ["merge|NUM|intersect|==|NUM|fConcat",         "ACTION"],
    ["merge|NUM|intersect|==|NUM|fLiteral",        "ACTION"],
    ["merge|NUM|intersect|==|NUM|intersect",       "ACTION"],


    ["merge|STR|fUnknown|==|STR|fUnknown",         "DO_NOTHING"],
    ["merge|STR|fUnknown|==|STR|fConcat",          "ACTION"],
    ["merge|STR|fUnknown|==|STR|fLiteral",         "copyValueRHStoLHS"], # sizeToCopy, handleRemainder
    ["merge|STR|fUnknown|==|STR|intersect",        "ACTION"],

    ["merge|STR|fConcat|==|STR|fUnknown",          "ACTION"],
    ["merge|STR|fConcat|==|STR|fConcat",           "ACTION"],
    ["merge|STR|fConcat|==|STR|fLiteral",          "ACTION"],
    ["merge|STR|fConcat|==|STR|intersect",         "ACTION"],

    ["merge|STR|fLiteral|==|STR|fUnknown",         "copyValueLHStoRHS"],
    ["merge|STR|fLiteral|==|STR|fConcat",          "ACTION"],
    ["merge|STR|fLiteral|==|STR|fLiteral",         "ACTION"],   #break into 2 cases: LHS.infSize.format = fUnknown, fLiteral.  see tryMergeValue()
    ["merge|STR|fLiteral|==|STR|intersect",        "ACTION"],

    ["merge|STR|intersect|==|STR|fUnknown",        "ACTION"],
    ["merge|STR|intersect|==|STR|fConcat",         "ACTION"],
    ["merge|STR|intersect|==|STR|fLiteral",        "ACTION"],
    ["merge|STR|intersect|==|STR|intersect",       "ACTION"],


    ["merge|LST-U,LST-u|fUnknown|==|LST-U,LST-u|fUnknown",        "ACTION"],
    ["merge|LST-U,LST-u|fUnknown|==|LST-U,LST-u|fConcat",         "ACTION"],
    ["merge|LST-U,LST-u|fUnknown|==|LST-U,LST-u|fLiteral",        "ACTION"],
    ["merge|LST-U,LST-u|fUnknown|==|LST-U,LST-u|intersect",       "ACTION"],

    ["merge|LST-U,LST-u|fConcat|==|LST-U,LST-u|fUnknown",         "ACTION"],
    ["merge|LST-U,LST-u|fConcat|==|LST-U,LST-u|fConcat",          "ACTION"],
    ["merge|LST-U,LST-u|fConcat|==|LST-U,LST-u|fLiteral",         "ACTION"],
    ["merge|LST-U,LST-u|fConcat|==|LST-U,LST-u|intersect",        "ACTION"],

    ["merge|LST-U,LST-u|fLiteral|==|LST-U,LST-u|fUnknown",        "ACTION"],
    ["merge|LST-U,LST-u|fLiteral|==|LST-U,LST-u|fConcat",         "ACTION"],
    ["merge|LST-U,LST-u|fLiteral|==|LST-U,LST-u|fLiteral",        "ACTION"],
    ["merge|LST-U,LST-u|fLiteral|==|LST-U,LST-u|intersect",       "ACTION"],

    ["merge|LST-U,LST-u|intersect|==|LST-U,LST-u|fUnknown",       "ACTION"],
    ["merge|LST-U,LST-u|intersect|==|LST-U,LST-u|fConcat",        "ACTION"],
    ["merge|LST-U,LST-u|intersect|==|LST-U,LST-u|fLiteral",       "ACTION"],
    ["merge|LST-U,LST-u|intersect|==|LST-U,LST-u|intersect",      "ACTION"],

]

print("COMBOS:", countCombinations(sizePoints))
print("COMBOS:", countCombinations(infonPoints))
print("COMBOS:", countCombinations(mergePoints))
sizeCases = enumerateAllCombos(sizePoints)
infCases = enumerateAllCombos(mergePoints)
#for case in sizeCases: print(case)

markHandledCases(infRules, infCases, mergePoints)
#markHandledCases(sizeRules, sizeCases, sizePoints)

#generateCode(infRules)

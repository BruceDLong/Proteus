#!/usr/bin/env python3
# Proteus Rule case manager

ruleSetIDs = ['merge']

infonPoints = [
    [
    '?',
    'NUM',      # TODO: 'NUM_Inv',
    'STR',
    'LST-u',
    'LST-U'     # TODO: T-Lists
    ],

    [
    'fUnknown',
    'fConcat',   # TODO: Concats with ... not counted.
    'fLiteral',
    'intersect'
    ],

    [
    'Size-0-',
    'Size-0-n',
    'Size-n-m',
    'Size-n',
    'Size-n-',
    'Size-Other'
    ]
        # TODO: ['Size-*', 'Size-/']
]


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
                retList.append(left+'.'+right)
        return(retList)
    else: return firstList

def doesCaseMatchPattern(pattern, case):
    patternSegs = pattern.split('.')
    caseSegs = case.split('.')
    numPSegs = len(patternSegs)
    numCSegs = len(caseSegs)
    toMatch = []
    for pseg in patternSegs:
        toMatch.append(pseg.split(','))
    if numPSegs != numCSegs:
        print("numPSegs:", numPSegs)
        print("numCSegs:", numCSegs)
        print("ERROR: pattern and case lengths do not match:", patternSegs, "\n\n", caseSegs)
        exit(1)
    for i in range(0, numCSegs):
        if not(caseSegs[i] in toMatch[i]):
            return(False)
    return(True)


print("COMBOS:", countCombinations(infonPoints))
print("COMBOS:", countCombinations(mergePoints))
cases = enumerateAllCombos(mergePoints)

rules = [
    "merge.?,NUM,STR,LST-u,LST-U.fUnknown,fConcat,fLiteral,intersect.Size-0-,Size-0-n,Size-n-m,Size-n,Size-n-,Size-Other.=,==.?.fUnknown,fConcat,fLiteral,intersect.Size-0-,Size-0-n,Size-n-m,Size-n,Size-n-,Size-Other",
    "merge.?.fUnknown,fConcat,fLiteral,intersect.Size-0-,Size-0-n,Size-n-m,Size-n,Size-n-,Size-Other.=,==.NUM,STR,LST-u,LST-U.fUnknown,fConcat,fLiteral,intersect.Size-0-,Size-0-n,Size-n-m,Size-n,Size-n-,Size-Other"
]

def markHandledCases(patterns, cases):
    for pattern in patterns:
        count = 0
        for case in cases:
            if doesCaseMatchPattern(pattern, case):
                count +=1
                case = "#"+case
                print(count, case)

markHandledCases(rules, cases)

print("Number of Cases:", len(cases))

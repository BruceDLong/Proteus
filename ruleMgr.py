#!/usr/bin/env python3
# Proteus Rule case manager
from pprint import pprint

debugMode = True

mergeSizeRules = {
    'ID': 'mergeSize',
    'points': [
        # TODO: ['Size-*', 'Size-/'], ["measurable", "!measurable"],l ["sGivn", !sGivn"],
        ['looseSize', '!looseSize'],
        ['lfUnknown', 'lfConcat', 'lfLiteral', 'lintersect'],
        ['rfUnknown', 'rfConcat', 'rsfLiteral', 'rintersect'],

    ],
    'ifSnips': {
        'l?':            'aItem.LHS_item.pItem.infMode == isUnknown',
        'lNUM':          'aItem.LHS_item.pItem.value.fType == NUM',
        'lSTR':          'aItem.LHS_item.pItem.value.fType == STR',
        'lLST':          'aItem.LHS_item.pItem.value.fType == LST',

        'lintersect':    'aItem.LHS_item.pItem.infSize.intersectPosParse == ipSquareBrackets',
        'lfUnknown':     'aItem.LHS_item.pItem.infSize.format == fUnknown',
        'lfConcat':      'aItem.LHS_item.pItem.infSize.format == fConcat',
        'lfLiteral':     'aItem.LHS_item.pItem.infSize.format == fLiteral',

        'r?':            'aItem.RHS.pItem.infMode == isUnknown',
        'rNUM':          'aItem.RHS.pItem.value.fType == NUM',
        'rSTR':          'aItem.RHS.pItem.value.fType == STR',
        'rLST':          'aItem.RHS.pItem.value.fType == LST',

        'rintersect':    'aItem.RHS.pItem.infSize.intersectPosParse == ipSquareBrackets',
        'rfUnknown':     'aItem.RHS.pItem.infSize.format == fUnknown',
        'rfConcat':      'aItem.RHS.pItem.infSize.format == fConcat',
        'rsfLiteral':     'aItem.RHS.pItem.infSize.format == fLiteral',

        'looseSize':     'aItem.looseSize',
        '!looseSize':    '!aItem.looseSize'
    },
    'codeSnips': {
        'copySizeRHStoLHS':         'if((aItem.LHS_item.pItem.infMode!=isLiteral or aItem.RHS.pItem.value.fType==aItem.LHS_item.pItem.value.fType) and (aItem.LHS_item.pItem.value.listSpec==NULL or !aItem.LHS_item.pItem.value.listSpec.asWrkLstOutr)){DO_COPY(aItem.RHS.pItem.infSize, aItem.LHS_item.pItem.infSize, 0)}',
    },
    'rules': [
        ["mergeSize:!looseSize|lfUnknown|rsfLiteral",     "copySizeRHStoLHS"],

    ]
}


mergeRules = {
    'ID': 'merge',
    'points': [
        ['l?', 'lNUM', 'lSTR', 'lLST', 'ltUnknown'],
        ['lintersect', 'lfUnknown', 'lfConcat', 'lfLiteral'],
        ['=', '=='],
        ['r?', 'rNUM', 'rSTR', 'rLST', 'rtUnknown'],
        ['rintersect', 'rfUnknown', 'rfConcat', 'rfLiteral']
    ],
    'ifSnips': {
        'l?':            'aItem.LHS_item.pItem.infMode == isUnknown',
        'lNUM':          'aItem.LHS_item.pItem.value.fType == NUM',
        'lSTR':          'aItem.LHS_item.pItem.value.fType == STR',
        'lLST':          'aItem.LHS_item.pItem.value.fType == LST',

        'lintersect':    'aItem.LHS_item.pItem.value.intersectPosParse == ipSquareBrackets',
        'lfUnknown':     'aItem.LHS_item.pItem.value.format == fUnknown',
        'lfConcat':      'aItem.LHS_item.pItem.value.format == fConcat',
        'lfLiteral':     'aItem.LHS_item.pItem.value.format == fLiteral',

        'r?':            'aItem.RHS.pItem.infMode == isUnknown',
        'rNUM':          'aItem.RHS.pItem.value.fType == NUM',
        'rSTR':          'aItem.RHS.pItem.value.fType == STR',
        'rLST':          'aItem.RHS.pItem.value.fType == LST',
        'rtUnknown':     'aItem.RHS.pItem.value.fType == tUnknown',

        'rintersect':    'aItem.RHS.pItem.intersectPos != ipNoIntersect',
        'rfUnknown':     'aItem.RHS.pItem.value.format == fUnknown',
        'rfConcat':      'aItem.RHS.pItem.value.format == fConcat',
        'rfLiteral':     'aItem.RHS.pItem.value.format == fLiteral',

        '==':           '(aItem.RHS.looseType())',
        '=':            '!(aItem.RHS.looseType())',
    },
    'codeSnips': {
        'REJECT':                   'aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true;',
        'copyValueRHStoLHS':        'DO_COPY(aItem.RHS.pItem.value, aItem.LHS_item.pItem.value, aItem.sizeToCopy)',
        'copyValueLHStoRHS':        'DO_COPY(aItem.LHS_item.pItem.value, aItem.RHS.pItem.value, aItem.sizeToCopy)',
        'copyRHSTypeToLHS':         'aItem.LHS_item.pItem.value.fType <- aItem.RHS.pItem.value.fType; aItem.LHS_item.pItem.infMode <- aItem.RHS.pItem.infMode',
        'copySizeRHStoLHS':         'DO_COPY(aItem.RHS.pItem.infSize, aItem.LHS_item.pItem.infSize, 0)',
        'rejectIfValueStrNotEqual': 'if(aItem.LHS_item.pItem.value.str != aItem.RHS.pItem.value.str){aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true}',
        'rejectIfValueNumNotEqual': 'if(aItem.LHS_item.pItem.value.num != aItem.RHS.pItem.value.num){aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true; logSeg("REJECT")}',
        'copyType':                 'if(aItem.RHS.pItem.type!=NULL){aItem.LHS_item.pItem.type <- aItem.RHS.pItem.type}',
        'StartMergePropogation':    'startPropRules(aItem)',
        'copyIdOrStartMergProp':    'if(aItem.LHS_item.accessMode==aRefTo){copyIdentity(aItem)}else{startPropRules(aItem)}',
        'MergeLooseStrings':        'remainder <- mergeLooseStrings(aItem)',
        'mergeRHSIntersect':        'mergeRHSIntersect(aItem)',
        'mergeANDRanges':           'mergeANDRanges(aItem)',
        'copyIdentity':             'copyIdentity(aItem)',
        'checkNumRange':            'if(!checkNumRange(aItem.LHS_item.pItem, aItem.RHS.pItem)){aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true; logSeg("REJECT")}',
        'checkNumRangeDeepCpy':     """if(!checkNumRange(aItem.LHS_item.pItem, aItem.RHS.pItem)){aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true; logSeg("REJECT")}
            me bool: truReject <- aItem.mergeStatus==msReject; if(aItem.LHS_item.applyAsNot(aItem.RHS)){truReject <- !truReject}
            if(!truReject){aItem.LHS_item.pItem <deep- aItem.RHS.pItem; if(aItem.LHS_item.outerPOV){aItem.LHS_item.outerPOV.pItem.altRulesApplied <- false}}""",
        'checkNumRangeDoCpy':       """if(!checkNumRange(aItem.LHS_item.pItem, aItem.RHS.pItem)){aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true; logSeg("REJECT")}
            me bool: truReject <- aItem.mergeStatus==msReject; if(aItem.LHS_item.applyAsNot(aItem.RHS)){truReject <- !truReject}
            if(!truReject){
                            DO_COPY(aItem.RHS.pItem.value, aItem.LHS_item.pItem.value, aItem.sizeToCopy);
                            aItem.LHS_item.pItem.asNot <- aItem.RHS.pItem.asNot
                            if(aItem.LHS_item.outerPOV){aItem.LHS_item.outerPOV.pItem.altRulesApplied <- false
            }}""",
    },
    'rules': [
        ["merge:|||r?|",                          "copyType"],
        ["merge:l?||=|rNUM,rSTR,rLST|",           "copyIdentity"],  #"copyRHSTypeToLHS,copyValueRHStoLHS,copySizeRHStoLHS"
        ["merge:l?||==|rNUM,rSTR,rLST|",          "copyRHSTypeToLHS,copyValueRHStoLHS"],
        ["merge:l?||=|rtUnknown|rintersect",      "mergeRHSIntersect"],
        ["merge:l?||==|rtUnknown|rintersect",     "mergeRHSIntersect"],

        ["merge:lNUM||=|rSTR,rLST|rfUnknown,rfLiteral",   "REJECT"],
        ["merge:lSTR||=|rNUM,rLST|",                      "REJECT"],
        ["merge:lLST|lfUnknown,lfLiteral|=|rNUM,rSTR|",   "REJECT"],

        ["merge:lNUM|lfUnknown|=|rNUM|rfUnknown",         "copyIdentity"],
        ["merge:lNUM|lfUnknown|=|rNUM|rfLiteral",         "copyValueRHStoLHS"],
        ["merge:lNUM|lfLiteral|=|rNUM|rfUnknown",         "NONE"],
        ["merge:lNUM|lfLiteral|=|rNUM|rfLiteral",         "rejectIfValueNumNotEqual"],

        ["merge:lSTR|lfUnknown|=|rSTR|rfUnknown",         "copyIdentity"],
        ["merge:lSTR|lfUnknown|=|rSTR|rfLiteral",         "copyValueRHStoLHS"],
        ["merge:lSTR|lfLiteral|=|rSTR|rfUnknown",         "NONE"],  # Copy LHS to RHS?
        ["merge:lSTR|lfLiteral|=|rSTR|rfLiteral",         "rejectIfValueStrNotEqual"],

        ["merge:lLST|lfUnknown|=|rLST|rfUnknown",        "ACTION"],
        ["merge:lLST|lfUnknown|=|rLST|rfLiteral",        "ACTION"],
        ["merge:lLST|lfLiteral|=|rLST|rfUnknown",        "ACTION"],
        ["merge:lLST|lfLiteral|=|rLST|rfLiteral",        "copyIdOrStartMergProp"],

        # LooseSize
        ["merge:lNUM||==|rSTR,rLST|rfUnknown,rfLiteral",   "ACTION"],
        ["merge:lSTR||==|rNUM,rLST|",                      "ACTION"],
        ["merge:lLST|lfUnknown,lfLiteral|==|rNUM,rSTR|",   "StartMergePropogation"], # ADD NEW AITEM LHS FIRST FROM LIST & THE WHOLE NUTHER RHS, MAYBE PROPAGATE SHOULD HANDLE

        ["merge:lNUM|lfUnknown|==|rNUM|rfUnknown",         "NONE"],
        ["merge:lNUM|lfUnknown|==|rNUM|rfLiteral",         "checkNumRangeDoCpy"], # remember size to copy
        ["merge:lNUM|lfLiteral|==|rNUM|rfUnknown",         "NONE"],
        ["merge:lNUM|lfLiteral|==|rNUM|rfLiteral",         "ACTION"], #break into 2 cases: LHS.infSize.format = rfUnknown, rfLiteral.  see tryMergeValue()

        ["merge:lSTR|lfUnknown|==|rSTR|rfUnknown",         "NONE"],
        ["merge:lSTR|lfUnknown|==|rSTR|rfLiteral",         "MergeLooseStrings"], # sizeToCopy, handleRemainder
        ["merge:lSTR|lfLiteral|==|rSTR|rfUnknown",         "NONE"],
        ["merge:lSTR|lfLiteral|==|rSTR|rfLiteral",         "MergeLooseStrings"],   #break into 2 cases: LHS.infSize.format = rfUnknown, rfLiteral.  see tryMergeValue()

        ["merge:lLST|lfUnknown|==|rLST|rfUnknown",        "ACTION"],
        ["merge:lLST|lfUnknown|==|rLST|rfLiteral",        "ACTION"],
        ["merge:lLST|lfLiteral|==|rLST|rfUnknown",        "ACTION"],
        ["merge:lLST|lfLiteral|==|rLST|rfLiteral",        "StartMergePropogation"],

        ##### CONCAT and INTERSECT
        ["merge:lNUM,lSTR,lLST|lfConcat|=,==|lNUM,lSTR,lLST|rintersect",                 "mergeRHSIntersect"],

        ["merge:lNUM|lfUnknown|=|rtUnknown,rNUM|rintersect",        "mergeRHSIntersect"],
        ["merge:lNUM|lfLiteral|=|rtUnknown,rNUM|rintersect",        "mergeRHSIntersect"],
        ["merge:lSTR|lfUnknown|=|rtUnknown,rSTR|rintersect",        "mergeRHSIntersect"],
        ["merge:lSTR|lfLiteral|=|rtUnknown,rSTR|rintersect",        "mergeRHSIntersect"],
        ["merge:lLST|lfUnknown|=|rtUnknown,rLST|rintersect",        "mergeRHSIntersect"],
        ["merge:lLST|lfLiteral|=|rtUnknown,rLST|rintersect",        "mergeRHSIntersect"],

        ["merge:lNUM|lfUnknown|=|rNUM,rLST|rfConcat",     "ACTION"],
        ["merge:lNUM|lfConcat|=|rNUM|rfUnknown",          "ACTION"],
        ["merge:lNUM|lfConcat|=|rNUM|rfConcat",           "ACTION"],
        ["merge:lNUM,lLST|lfConcat|=|rNUM|rfLiteral",               "checkNumRangeDeepCpy"],
        ["merge:lNUM|lfLiteral|=|rNUM,rLST|rfConcat",               "checkNumRangeDeepCpy"],
        ["merge:lNUM|lintersect|=|rNUM|rfUnknown",        "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|rfConcat",         "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|rfLiteral",        "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|rintersect",       "ACTION"],

        ["merge:lSTR|lfUnknown|=|rSTR|rfConcat",          "ACTION"],
        ["merge:lSTR|lfConcat|=|rSTR|rfUnknown",          "ACTION"],
        ["merge:lSTR|lfConcat|=|rSTR|rfConcat",           "ACTION"],
        ["merge:lSTR|lfConcat|=|rSTR|rfLiteral",          "ACTION"],
        ["merge:lSTR|lfLiteral|=|rSTR|rfConcat",          "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|rfUnknown",        "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|rfConcat",         "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|rfLiteral",        "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|rintersect",       "ACTION"],

        ["merge:lLST|lfUnknown|=|rLST|rfConcat",         "ACTION"],
        ["merge:lLST|lfConcat|=|rLST|rfUnknown",         "ACTION"],
        ["merge:lLST|lfConcat|=|rLST|rfConcat",          "mergeANDRanges"],
        ["merge:lLST|lfConcat|=|rLST|rfLiteral",         "ACTION"],
        ["merge:lLST|lfLiteral|=|rLST|rfConcat",         "ACTION"],
        ["merge:lLST|lintersect|=|rLST|rfUnknown",       "ACTION"],
        ["merge:lLST|lintersect|=|rLST|rfConcat",        "ACTION"],
        ["merge:lLST|lintersect|=|rLST|rfLiteral",       "ACTION"],
        ["merge:lLST|lintersect|=|rLST|rintersect",      "ACTION"],

        # LooseSize
        ["merge:lSTR|lfUnknown|==|rtUnknown,rSTR|rintersect",        "mergeRHSIntersect"],
        ["merge:lSTR|lfLiteral|==|rtUnknown,rSTR|rintersect",        "mergeRHSIntersect"],
        ["merge:lLST|lfUnknown|==|rtUnknown,rLST|rintersect",        "mergeRHSIntersect"],
        ["merge:lLST|lfLiteral|==|rtUnknown,rLST|rintersect",        "mergeRHSIntersect"],


        ["merge:lNUM|lfUnknown|==|rNUM|rfConcat",          "ACTION"],
        ["merge:lNUM|lfUnknown|==|rLST|rfConcat",          "checkNumRangeDeepCpy"],
        ["merge:lNUM|lfUnknown|==|rNUM|rintersect",        "ACTION"],
        ["merge:lNUM|lfConcat|==|rNUM|rfUnknown",          "ACTION"],
        ["merge:lNUM|lfConcat|==|rNUM|rfConcat",           "ACTION"],
        ["merge:lNUM,lLST|lfConcat|==|rNUM|rfLiteral",          "checkNumRangeDeepCpy"],
        ["merge:lNUM|lfLiteral|==|rNUM,rLST|rfConcat",          "checkNumRange"],
        ["merge:lNUM|lfLiteral|==|rNUM|rintersect",        "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|rfUnknown",        "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|rfConcat",         "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|rfLiteral",        "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|rintersect",       "ACTION"],

        ["merge:lSTR|lfUnknown|==|rSTR|rfConcat",          "ACTION"],
        ["merge:lSTR|lfConcat|==|rSTR|rfUnknown",          "ACTION"],
        ["merge:lSTR|lfConcat|==|rSTR|rfConcat",           "ACTION"],
        ["merge:lSTR|lfConcat|==|rSTR|rfLiteral",          "ACTION"],
        ["merge:lSTR|lfLiteral|==|rSTR|rfConcat",          "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|rfUnknown",        "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|rfConcat",         "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|rfLiteral",        "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|rintersect",       "ACTION"],

        ["merge:lLST|lfUnknown|==|rLST|rfConcat",         "ACTION"],
        ["merge:lLST|lfConcat|==|rLST|rfUnknown",         "ACTION"],
        ["merge:lLST|lfConcat|==|rLST|rfConcat",          "ACTION"],
        ["merge:lLST|lfConcat|==|rLST|rfLiteral",         "ACTION"],
        ["merge:lLST|lfLiteral|==|rLST|rfConcat",         "ACTION"],
        ["merge:lLST|lintersect|==|rLST|rfUnknown",       "ACTION"],
        ["merge:lLST|lintersect|==|rLST|rfConcat",        "ACTION"],
        ["merge:lLST|lintersect|==|rLST|rfLiteral",       "ACTION"],
        ["merge:lLST|lintersect|==|rLST|rintersect",      "ACTION"]
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
startPropRules = { # Start iterating fLiteral LST = fLiteral LST
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

def genActionCode(ruleSetID, codeKeyWords, rule, codeSnips, indent):
    S = ""
    if codeKeyWords == "ACTION":
        if debugMode:
            S= indent + '//:l/merge::log(indentStr(aItem.indentLvl)+"        TODO: unfinished")\n'
        else:
            S= indent + "//TODO: unfinished\n"
        return(S)
    if codeKeyWords == "NONE":
        if debugMode:
            S= indent + '//:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+':'+triggers+':Do Nothing")\n'
        else:
            S= indent + "//Do Nothing\n"
        return(S)
    codeKeyWordList = codeKeyWords.split(",")
    for KW in codeKeyWordList:
        S+= indent + codeSnips[KW]+"\n"
    if debugMode: S = indent+'//:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+'  '+rule+'\t'+KW+'")\n' +S
    return(S)

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
            actionCode = ""
            if codeKeyWords =='ACTION':
                if debugMode:
                    actionCode = indent + '    //:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+':'+triggers+':TODO: unfinished")\n'
                else:
                    actionCode = indent + "    //TODO: unfinished\n"
            elif codeKeyWords == "NONE":
                if debugMode:
                    actionCode = indent + '    //:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+':'+triggers+':Do Nothing")\n'
                else:
                    actionCode = indent + "    //Do Nothing\n"
            else:
                #print(codeKeyWords)
                codeKeyWordList = codeKeyWords.split(",")
                for KW in codeKeyWordList:
                    actionCode+= indent +"    " + codeSnips[KW]+"\n"
                if ruleSetID !="merge": actionCode+= indent +"    changeMade <- true\n"
                if debugMode:
                    actionCode = indent+'    //:l/merge::log(indentStr(aItem.indentLvl)+"        '+ruleSetID+'  '+triggers+'\t'+KW+'")\n' + actionCode
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
        ifsCode += '        logSeg("-mRUl")\n'
        ifsCode += genCodeFullIfs(ruleSetID, rules, ifSnips, codeSnips)
        ifsCode += '        else {log("MERGE_RULE_MISSING: "+ toString(aItem));log("          LHS fType:"+ fTypeStrings[aItem.LHS_item.pItem.value.fType]);log("          LHS format:"+ formatStrings[aItem.LHS_item.pItem.value.format]);log("          RHS fType:"+ fTypeStrings[aItem.RHS.pItem.value.fType]); log("EXITING"); exit(2);}\n'
        ifsCode += "        return(remainder)"
        funcCode = "    our POV: "+ruleSetID+"Rules(our AItem: aItem) <- {\n"+ifsCode+"\n    }\n"
    else:
        ifsCode =  "        me bool: changeMade <- false\n"
        ifsCode += genCodeFullIfs(ruleSetID, rules, ifSnips, codeSnips)
        ifsCode += '        //else {log("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ '+ruleSetID+' RULE_MISSING");}\n'
        ifsCode += "        return(changeMade)"
        funcCode = "    me bool: "+ruleSetID+"Rules(our AItem: aItem) <- {\n"+ifsCode+"\n    }\n"
    return(funcCode)

def generateXformMgr(ruleSets):
    structCode = "struct WorldManager{\n"
    for ruleSet in ruleSets:
        funcCode = generateMemberFunc(ruleSet['ID'], ruleSet['points'], ruleSet['rules'], ruleSet['ifSnips'], ruleSet['codeSnips'])
        structCode+=funcCode
    structCode += "}"
    with open("WorldManager.dog", "w") as text_file: print(structCode, file=text_file)

generateXformMgr(ruleSets)

#!/usr/bin/env python3
# Proteus Rule case manager
from pprint import pprint

debugMode = True

mergeSizeRules = {
    'ID': 'mergeSize',
    'points': [
        # TODO: ['Size-*', 'Size-/'], ["measurable", "!measurable"],l ["sGivn", !sGivn"],
        ['looseSize', '!looseSize'],
        ['lemUnknown', 'lemConcat', 'lemLiteral', 'lintersect'],
        ['remUnknown', 'remConcat', 'rsemLiteral', 'rintersect'],

    ],
    'ifSnips': {
        'l?':            'aItem.LHS_item.pItem.infMode == isUnknown',
        'lNUM':          'aItem.LHS_item.pItem.value.overlayMode == NUM',
        'lSTR':          'aItem.LHS_item.pItem.value.overlayMode == STR',
        'lLST':          'aItem.LHS_item.pItem.value.overlayMode == LST',

        'lintersect':    'aItem.LHS_item.pItem.infSize.intersectPosParse == ipSquareBrackets',
        'lemUnknown':     'aItem.LHS_item.pItem.infSize.evalMode == emUnknown',
        'lemConcat':      'aItem.LHS_item.pItem.infSize.evalMode == emConcat',
        'lemLiteral':     'aItem.LHS_item.pItem.infSize.evalMode == emLiteral',

        'r?':            'aItem.RHS.pItem.infMode == isUnknown',
        'rNUM':          'aItem.RHS.pItem.value.overlayMode == NUM',
        'rSTR':          'aItem.RHS.pItem.value.overlayMode == STR',
        'rLST':          'aItem.RHS.pItem.value.overlayMode == LST',

        'rintersect':    'aItem.RHS.pItem.infSize.intersectPosParse == ipSquareBrackets',
        'remUnknown':     'aItem.RHS.pItem.infSize.evalMode == emUnknown',
        'remConcat':      'aItem.RHS.pItem.infSize.evalMode == emConcat',
        'rsemLiteral':     'aItem.RHS.pItem.infSize.evalMode == emLiteral',

        'looseSize':     'aItem.looseSize',
        '!looseSize':    '!aItem.looseSize'
    },
    'codeSnips': {
        'copySizeRHStoLHS':         'if((aItem.LHS_item.pItem.infMode!=isLiteral or aItem.RHS.pItem.value.overlayMode==aItem.LHS_item.pItem.value.overlayMode) and (aItem.LHS_item.pItem.value.listSpec==NULL or !aItem.LHS_item.pItem.value.listSpec.asWrkLstOutr)){DO_COPY(aItem.RHS.pItem.infSize, aItem.LHS_item.pItem.infSize, 0)}',
    },
    'rules': [
        ["mergeSize:!looseSize|lemUnknown|rsemLiteral",     "copySizeRHStoLHS"],

    ]
}


mergeRules = {
    'ID': 'merge',
    'points': [
        ['l?', 'lNUM', 'lSTR', 'lLST', 'ltUnknown'],
        ['lintersect', 'lemUnknown', 'lemConcat', 'lemLiteral'],
        ['=', '=='],
        ['r?', 'rNUM', 'rSTR', 'rLST', 'rtUnknown'],
        ['rintersect', 'remUnknown', 'remConcat', 'remLiteral']
    ],
    'ifSnips': {
        'l?':            'aItem.LHS_item.pItem.infMode == isUnknown',
        'lNUM':          'aItem.LHS_item.pItem.value.overlayMode == NUM',
        'lSTR':          'aItem.LHS_item.pItem.value.overlayMode == STR',
        'lLST':          'aItem.LHS_item.pItem.value.overlayMode == LST',

        'lintersect':    'aItem.LHS_item.pItem.value.intersectPosParse == ipSquareBrackets',
        'lemUnknown':     'aItem.LHS_item.pItem.value.evalMode == emUnknown',
        'lemConcat':      'aItem.LHS_item.pItem.value.evalMode == emConcat',
        'lemLiteral':     'aItem.LHS_item.pItem.value.evalMode == emLiteral',

        'r?':            'aItem.RHS.pItem.infMode == isUnknown',
        'rNUM':          'aItem.RHS.pItem.value.overlayMode == NUM',
        'rSTR':          'aItem.RHS.pItem.value.overlayMode == STR',
        'rLST':          'aItem.RHS.pItem.value.overlayMode == LST',
        'rtUnknown':     'aItem.RHS.pItem.value.overlayMode == tUnknown',

        'rintersect':    'aItem.RHS.pItem.intersectPos != ipNoIntersect',
        'remUnknown':     'aItem.RHS.pItem.value.evalMode == emUnknown',
        'remConcat':      'aItem.RHS.pItem.value.evalMode == emConcat',
        'remLiteral':     'aItem.RHS.pItem.value.evalMode == emLiteral',

        '==':           '(aItem.RHS.looseType())',
        '=':            '!(aItem.RHS.looseType())',
    },
    'codeSnips': {
        'REJECT':                   'aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true;',
        'copyValueRHStoLHS':        'DO_COPY(aItem.RHS.pItem.value, aItem.LHS_item.pItem.value, aItem.sizeToCopy)',
        'copyValueLHStoRHS':        'DO_COPY(aItem.LHS_item.pItem.value, aItem.RHS.pItem.value, aItem.sizeToCopy)',
        'copyRHSTypeToLHS':         'aItem.LHS_item.pItem.value.overlayMode <- aItem.RHS.pItem.value.overlayMode; aItem.LHS_item.pItem.infMode <- aItem.RHS.pItem.infMode',
        'copySizeRHStoLHS':         'DO_COPY(aItem.RHS.pItem.infSize, aItem.LHS_item.pItem.infSize, 0)',
        'rejectIfValueStrNotEqual': 'if(aItem.LHS_item.pItem.value.str != aItem.RHS.pItem.value.str){aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true}',
        'rejectIfValueNumNotEqual': 'if(aItem.LHS_item.pItem.value.num != aItem.RHS.pItem.value.num){aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true; logSeg("REJECT")}',
        'copyType':                 'if(aItem.RHS.pItem.type!=NULL){aItem.LHS_item.pItem.type <- aItem.RHS.pItem.type}',
        'StartMergePropogation':    'startPropRules(aItem)',
        'copyIdOrStartMergProp':    'if(aItem.LHS_item.accessMode==aRefTo){copyIdentity(aItem)}else{startPropRules(aItem)}',
        'ifRefCopyIdentity':        'if(aItem.LHS_item.accessMode==aRefTo){copyIdentity(aItem)}else if(!aItem.RHS.pItem.wrkList.isEmpty()){aItem.LHS_item.pItem.copyWrkListFrom(aItem.RHS.pItem)}',
        'MergeLooseStrings':        'remainder <- mergeLooseStrings(aItem)',
        'mergeRHSIntersect':        'mergeRHSIntersect(aItem)',
        'mergeANDRanges':           'mergeANDRanges(aItem)',
        'copyIdentity':             'copyIdentity(aItem)',
        'checkNumRange':            'if(!checkNumRange(aItem.LHS_item.pItem, aItem.RHS.pItem)){aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true; logSeg("REJECT")}',
        'checkNumRangeDeepCpy':     """if(!checkNumRange(aItem.LHS_item.pItem, aItem.RHS.pItem)){aItem.mergeStatus<-msReject; aItem.LHS_item.rejected<-true; logSeg("REJECT")}
            me bool: truReject <- aItem.mergeStatus==msReject; if(aItem.LHS_item.applyAsNot(aItem.RHS)){truReject <- !truReject}
            if(!truReject){aItem.LHS_item.pItem! <- aItem.RHS.pItem!; if(aItem.LHS_item.outerPOV){aItem.LHS_item.outerPOV.pItem.altRulesApplied <- false}}""",
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

        ["merge:lNUM||=|rSTR,rLST|remUnknown,remLiteral",   "REJECT"],
        ["merge:lSTR||=|rNUM,rLST|",                      "REJECT"],
        ["merge:lLST|lemUnknown,lemLiteral|=|rNUM,rSTR|",   "REJECT"],

        ["merge:lNUM|lemUnknown|=|rNUM|remUnknown",         "ifRefCopyIdentity"],
        ["merge:lNUM|lemUnknown|=|rNUM|remLiteral",         "copyValueRHStoLHS"],
        ["merge:lNUM|lemLiteral|=|rNUM|remUnknown",         "NONE"],
        ["merge:lNUM|lemLiteral|=|rNUM|remLiteral",         "rejectIfValueNumNotEqual"],

        ["merge:lSTR|lemUnknown|=|rSTR|remUnknown",         "ifRefCopyIdentity"],
        ["merge:lSTR|lemUnknown|=|rSTR|remLiteral",         "copyValueRHStoLHS"],
        ["merge:lSTR|lemLiteral|=|rSTR|remUnknown",         "NONE"],  # Copy LHS to RHS?
        ["merge:lSTR|lemLiteral|=|rSTR|remLiteral",         "rejectIfValueStrNotEqual"],

        ["merge:lLST|lemUnknown|=|rLST|remUnknown",        "ACTION"],
        ["merge:lLST|lemUnknown|=|rLST|remLiteral",        "ACTION"],
        ["merge:lLST|lemLiteral|=|rLST|remUnknown",        "ACTION"],
        ["merge:lLST|lemLiteral|=|rLST|remLiteral",        "copyIdOrStartMergProp"],

        # LooseSize
        ["merge:lNUM||==|rSTR,rLST|remUnknown,remLiteral",   "ACTION"],
        ["merge:lSTR||==|rNUM,rLST|",                      "ACTION"],
        ["merge:lLST|lemUnknown,lemLiteral|==|rNUM,rSTR|",   "StartMergePropogation"], # ADD NEW AITEM LHS FIRST FROM LIST & THE WHOLE NUTHER RHS, MAYBE PROPAGATE SHOULD HANDLE

        ["merge:lNUM|lemUnknown|==|rNUM|remUnknown",         "NONE"],
        ["merge:lNUM|lemUnknown|==|rNUM|remLiteral",         "checkNumRangeDoCpy"], # remember size to copy
        ["merge:lNUM|lemLiteral|==|rNUM|remUnknown",         "NONE"],
        ["merge:lNUM|lemLiteral|==|rNUM|remLiteral",         "ACTION"], #break into 2 cases: LHS.infSize.evalMode = remUnknown, remLiteral.  see tryMergeValue()

        ["merge:lSTR|lemUnknown|==|rSTR|remUnknown",         "NONE"],
        ["merge:lSTR|lemUnknown|==|rSTR|remLiteral",         "MergeLooseStrings"], # sizeToCopy, handleRemainder
        ["merge:lSTR|lemLiteral|==|rSTR|remUnknown",         "NONE"],
        ["merge:lSTR|lemLiteral|==|rSTR|remLiteral",         "MergeLooseStrings"],   #break into 2 cases: LHS.infSize.evalMode = remUnknown, remLiteral.  see tryMergeValue()

        ["merge:lLST|lemUnknown|==|rLST|remUnknown",        "ACTION"],
        ["merge:lLST|lemUnknown|==|rLST|remLiteral",        "ACTION"],
        ["merge:lLST|lemLiteral|==|rLST|remUnknown",        "ACTION"],
        ["merge:lLST|lemLiteral|==|rLST|remLiteral",        "StartMergePropogation"],

        ##### CONCAT and INTERSECT
        ["merge:lNUM,lSTR,lLST|lemConcat|=,==|lNUM,lSTR,lLST|rintersect",                 "mergeRHSIntersect"],

        ["merge:lNUM|lemUnknown|=|rtUnknown,rNUM|rintersect",        "mergeRHSIntersect"],
        ["merge:lNUM|lemLiteral|=|rtUnknown,rNUM|rintersect",        "mergeRHSIntersect"],
        ["merge:lSTR|lemUnknown|=|rtUnknown,rSTR|rintersect",        "mergeRHSIntersect"],
        ["merge:lSTR|lemLiteral|=|rtUnknown,rSTR|rintersect",        "mergeRHSIntersect"],
        ["merge:lLST|lemUnknown|=|rtUnknown,rLST|rintersect",        "mergeRHSIntersect"],
        ["merge:lLST|lemLiteral|=|rtUnknown,rLST|rintersect",        "mergeRHSIntersect"],

        ["merge:lNUM|lemUnknown|=|rNUM,rLST|remConcat",     "ACTION"],
        ["merge:lNUM|lemConcat|=|rNUM|remUnknown",          "ACTION"],
        ["merge:lNUM|lemConcat|=|rNUM|remConcat",           "ACTION"],
        ["merge:lNUM,lLST|lemConcat|=|rNUM|remLiteral",               "checkNumRangeDeepCpy"],
        ["merge:lNUM|lemLiteral|=|rNUM,rLST|remConcat",               "checkNumRangeDeepCpy"],
        ["merge:lNUM|lintersect|=|rNUM|remUnknown",        "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|remConcat",         "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|remLiteral",        "ACTION"],
        ["merge:lNUM|lintersect|=|rNUM|rintersect",       "ACTION"],

        ["merge:lSTR|lemUnknown|=|rSTR|remConcat",          "ACTION"],
        ["merge:lSTR|lemConcat|=|rSTR|remUnknown",          "ACTION"],
        ["merge:lSTR|lemConcat|=|rSTR|remConcat",           "ACTION"],
        ["merge:lSTR|lemConcat|=|rSTR|remLiteral",          "ACTION"],
        ["merge:lSTR|lemLiteral|=|rSTR|remConcat",          "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|remUnknown",        "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|remConcat",         "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|remLiteral",        "ACTION"],
        ["merge:lSTR|lintersect|=|rSTR|rintersect",       "ACTION"],

        ["merge:lLST|lemUnknown|=|rLST|remConcat",         "ACTION"],
        ["merge:lLST|lemConcat|=|rLST|remUnknown",         "ACTION"],
        ["merge:lLST|lemConcat|=|rLST|remConcat",          "mergeANDRanges"],
        ["merge:lLST|lemConcat|=|rLST|remLiteral",         "ACTION"],
        ["merge:lLST|lemLiteral|=|rLST|remConcat",         "ACTION"],
        ["merge:lLST|lintersect|=|rLST|remUnknown",       "ACTION"],
        ["merge:lLST|lintersect|=|rLST|remConcat",        "ACTION"],
        ["merge:lLST|lintersect|=|rLST|remLiteral",       "ACTION"],
        ["merge:lLST|lintersect|=|rLST|rintersect",      "ACTION"],

        # LooseSize
        ["merge:lSTR|lemUnknown|==|rtUnknown,rSTR|rintersect",        "mergeRHSIntersect"],
        ["merge:lSTR|lemLiteral|==|rtUnknown,rSTR|rintersect",        "mergeRHSIntersect"],
        ["merge:lLST|lemUnknown|==|rtUnknown,rLST|rintersect",        "mergeRHSIntersect"],
        ["merge:lLST|lemLiteral|==|rtUnknown,rLST|rintersect",        "mergeRHSIntersect"],


        ["merge:lNUM|lemUnknown|==|rNUM|remConcat",          "ACTION"],
        ["merge:lNUM|lemUnknown|==|rLST|remConcat",          "checkNumRangeDeepCpy"],
        ["merge:lNUM|lemUnknown|==|rNUM|rintersect",        "ACTION"],
        ["merge:lNUM|lemConcat|==|rNUM|remUnknown",          "ACTION"],
        ["merge:lNUM|lemConcat|==|rNUM|remConcat",           "ACTION"],
        ["merge:lNUM,lLST|lemConcat|==|rNUM|remLiteral",          "checkNumRangeDeepCpy"],
        ["merge:lNUM|lemLiteral|==|rNUM,rLST|remConcat",          "checkNumRange"],
        ["merge:lNUM|lemLiteral|==|rNUM|rintersect",        "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|remUnknown",        "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|remConcat",         "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|remLiteral",        "ACTION"],
        ["merge:lNUM|lintersect|==|rNUM|rintersect",       "ACTION"],

        ["merge:lSTR|lemUnknown|==|rSTR|remConcat",          "ACTION"],
        ["merge:lSTR|lemConcat|==|rSTR|remUnknown",          "ACTION"],
        ["merge:lSTR|lemConcat|==|rSTR|remConcat",           "ACTION"],
        ["merge:lSTR|lemConcat|==|rSTR|remLiteral",          "ACTION"],
        ["merge:lSTR|lemLiteral|==|rSTR|remConcat",          "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|remUnknown",        "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|remConcat",         "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|remLiteral",        "ACTION"],
        ["merge:lSTR|lintersect|==|rSTR|rintersect",       "ACTION"],

        ["merge:lLST|lemUnknown|==|rLST|remConcat",         "ACTION"],
        ["merge:lLST|lemConcat|==|rLST|remUnknown",         "ACTION"],
        ["merge:lLST|lemConcat|==|rLST|remConcat",          "ACTION"],
        ["merge:lLST|lemConcat|==|rLST|remLiteral",         "ACTION"],
        ["merge:lLST|lemLiteral|==|rLST|remConcat",         "ACTION"],
        ["merge:lLST|lintersect|==|rLST|remUnknown",       "ACTION"],
        ["merge:lLST|lintersect|==|rLST|remConcat",        "ACTION"],
        ["merge:lLST|lintersect|==|rLST|remLiteral",       "ACTION"],
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
        ifsCode += '        logSeg(" mRUl")\n'
        ifsCode += '        me bool: orderedSpanMergeHandled <- false\n'
        ifsCode += '        our POV: orderedSpanRemainder <- orderedSpanMergeRules(aItem, orderedSpanMergeHandled)\n'
        ifsCode += '        if(orderedSpanMergeHandled){return(orderedSpanRemainder)}\n'
        ifsCode += genCodeFullIfs(ruleSetID, rules, ifSnips, codeSnips)
        ifsCode += '        else {log("MERGE_RULE_MISSING: "+ toString(aItem));log("          LHS overlayMode:"+ overlayModeStrings[aItem.LHS_item.pItem.value.overlayMode]);log("          LHS evalMode:"+ evalModeStrings[aItem.LHS_item.pItem.value.evalMode]);log("          RHS overlayMode:"+ overlayModeStrings[aItem.RHS.pItem.value.overlayMode]); log("EXITING"); exit(2);}\n'
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

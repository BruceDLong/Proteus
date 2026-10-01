#pragma once

// Coordinates retain signed rational offsets. A single
// parenthesized term is the same coordinate; it need not acquire a scalar
// radix merely to reveal its sign. Other expressions remain ordinary goals.
inline bool signedCoordinate(ProteusGraphAccess& access,const std::shared_ptr<infonState>& state,
                             FlexNum& number,Cost& visits,std::set<infonState*> seen={}) {
    ++visits;
    if(!seen.insert(state.get()).second || getFlagBit(state->flags,infonState::undefined)) return false;
    auto overlay=getModeBits(state->flags,infonState::overlayType);
    auto mode=getModeBits(state->flags,infonState::evalMode);
    if(overlay==infonState::NUM && mode==infonState::emLiteral) number=state->num;
    else if(overlay==infonState::LST && mode==infonState::emConcat &&
            !getFlagBit(state->flags,infonState::tailUnfinished) && !state->listSpec && state->items.size()==1) {
        auto term=state->items.head;
        if(!access.plainStructure(term,false,false,false,false,false,true) || access.hasTypeRequirements(term) ||
           !POVDescription(term,access.registry.get()).relationships().empty() ||
           !signedCoordinate(access,readView(term)->value,number,visits,std::move(seen))) return false;
    } else return false;
    if(getFlagBit(state->flags,infonState::inverted)) number.val=-number.val;
    return true;
}

# Crossing examples with canonical printing, plus equivalent legacy spellings.
crossing/left, crossing open ends, {1 ... 6} = {... &{4 5 6}}, *_+{1 &{... 4 5} 6}, norm
crossing/anchored, known suffix correspondence, {1 ... 5 _} = {... 4 5 6}, *_+{1 &{... 4} 5 6}, norm
crossing/right, reverse equality, {... &{4 5 6}} = {1 ... 6}, *_+{&{1 ...} &{4 5 6}}, norm
crossing/legacyLeftSpelling, equivalent unresolved span spelling, *_+{1 &{&{ ... } 4 5} 6}, *_+{1 &{... 4 5} 6}, parse
crossing/legacyAnchoredSpelling, equivalent unresolved span spelling, *_+{1 &{&{ ... } 4} 5 6}, *_+{1 &{... 4} 5 6}, parse

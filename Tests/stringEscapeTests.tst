# The fixture reader consumes one layer of escaping before Proteus sees the input.
stringEscape/quotes, Both quotes and the question mark, {"a\\'b" 'a\\"b' "\\?"}, {'a\\'b' 'a"b' '?'}, parse
stringEscape/slashes, A backslash before the closing quote or another escape, {'end\\\\' 'a\\\\\\'b' 7}, {'end\\\\' 'a\\\\\\'b' 7}, parse
stringEscape/controls, Octal control bytes print as named escapes, '\\007\\010\\014\\012\\015\\011\\013', '\\a\\b\\f\\n\\r\\t\\v', parse
stringEscape/bytes, Numeric byte escapes consume the C digit widths, '\\1012\\x0004aZ\\78\\0f', 'A2JZ\\a8\\000f', parse
stringEscape/unicode, Unicode escapes and literal UTF-8 mix in either quote style, {"é\\u03a9" '\\U0001F642\\u0041b'}, {'éΩ' '🙂Ab'}, parse
stringEscape/scalars, Three-byte and combining scalars retain their spelling, '\\u20ac\\u4e2d\\u0065\\u0301', '€中é', parse
stringEscape/size, Escaped and literal Unicode have the same byte extent, '\\u00e9\\U0001f642', *6+'é🙂', norm,,debug
stringEscape/join, Normalization joins decoded bytes, **('\\x41' "\\u03A9" '\\041'), 'AΩ!', norm
stringEscape/utf8Bytes, Encoded UTF-8 bytes match literal Unicode, '\\303\\251\\xf0\\x9f\\x99\\x82', 'é🙂', parse
stringEscape/nonText, Isolated and ill-formed UTF-8 bytes stay printable, '\\x7f\\x80\\xc0\\xaf\\xed\\xa0\\x80\\xf4\\x90\\x80\\x80\\xff\\x30', '\\177\\200\\300\\257\\355\\240\\200\\364\\220\\200\\200\\3770', parse
stringEscape/line, An escaped physical newline joins the text, 'one\\\ntwo', 'onetwo', parse
stringEscape/binaryJoin, Concatenation keeps bytes after NUL, **('\\xff\\0' '\\x80Z'), *4+'\\377\\000\\200Z', norm,,debug
stringEscape/binaryEqual, Different byte escape spellings compare equal, '\\0\\xFF' = '\\000\\377', '\\000\\377', norm
stringEscape/binaryMismatch, A difference after NUL rejects equality, 'a\\0\\xFE' = 'a\\0\\xFF', <REJECTED>, norm

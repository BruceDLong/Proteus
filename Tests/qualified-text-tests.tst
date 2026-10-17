str/colon, unknown prefix, $ =: 'Hello', *_+'Hello', norm
any/colon, unknown any prefix, ? ==: 'Hello', *_+'Hello', norm
str/sizedPrefix, sized prefix, *3+$ =: 'Hello', 'Hel', norm
str/shortLiteral, literal prefix, 'Hel' =: 'Hello', 'Hel', norm
str/longLiteral, longer template, 'Hello' =: 'Hel', 'Hello', norm
str/sizeKnown, known width, *5+$ ==: 'Hello', 'Hello', norm
str/emptyPrefix, empty unknown prefix, $ =: '', *_+'', norm

fence/emptyBinary, Zero bytes have two framing breaks, '''0\n\n'''00000000, '''0\n\n'''00000000, parse
fence/checked, Canonical checksum and framing, '''9\r\n123456789\r\n  '''E3069283, '''9\n123456789\n'''e3069283, parse
fence/join, Ordered binary packets concatenate, **('''3\nabc\n''' '''3\ndef\n'''), '''6\nabcdef\n''', norm
fence/checkedJoin, Recompute checksum over joined bytes, **('''3\n123\n'''107b2fb2 '''6\n456789\n'''), '''9\n123456789\n'''e3069283, norm
fence/text, Closing indent is removed and inner indent survives, {note:"""END\n  first\n    second\n  """END\, n:2}, {note:"""END\nfirst\n  second\n"""END\, n:2}, parse
fence/blank, Empty text, """END\n"""END, """END\n"""END, parse
fence/tagPrefix, A longer tag in the body is literal, """A\n"""AB\n"""A, """A\n"""AB\n"""A, parse
fence/mixed, Framing preserves interior newlines and raw backslashes, **("""T\n\\n\r\nline\n"""T '''1\n!\n'''), '''9\n\\n\r\nline!\n''', norm
fence/afterCloser, Syntax follows the checked packet directly, {'''9\n123456789\n'''E3069283\, 2}, {'''9\n123456789\n'''e3069283\, 2}, parse

from pathlib import Path
import re,json
p=Path(__file__).parent
rows=[]
for name in ['state','history','independent','changed','spatial']:
 text=(p/f'{name}.log').read_text()
 after=next(s[6:] for s in text.splitlines() if s.startswith('AFTER '))
 assert 'STATUS symbolic=1 declarations_unchanged=1' in text,name
 if name=='state':
  assert 'location:{xloc:*_+1, yloc:*_+2, zloc:*_+3.5}' in after,name
  assert text.count('| Apply finite list type requirements |')==1,name
 else:
  assert 'solved-sample-state:{thing:{location:{xloc:*_+1, yloc:*_+2, zloc:*_+3.5}}}' in after,name
  later=after.split(' sample-state:',1)[1]
  if name=='history':assert 'zloc:*_+3.5' in later and '| Set preserved scalar value |' in text
  if name=='independent':assert 'zloc:%? = *_+_' in later and '| Set preserved scalar value |' not in text
  if name=='changed':assert 'zloc:7' in later and '| Set preserved scalar value |' not in text
  if name=='spatial':assert 'zloc:_' in later and '| Set preserved scalar value |' not in text
 counts=re.search(r'COUNTS events=(\d+) visits=(\d+) distinct_visited=(\d+)',text)
 rows.append(dict(case=name,events=int(counts[1]),visits=int(counts[2]),distinct_visited=int(counts[3]),declarations_unchanged=True))
print('PASS: definition copy, temporal inheritance, independent unknown, explicit change, non-temporal isolation; declarations unchanged in all five cases.')
(p/'results.json').write_text(json.dumps(rows,indent=2)+'\n')

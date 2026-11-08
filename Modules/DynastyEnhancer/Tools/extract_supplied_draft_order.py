"""Extract team/pick facts from the user's supplied Tankathon HTM (not HTML)."""
from pathlib import Path
from html.parser import HTMLParser
import hashlib,json,re,sys
class Order(HTMLParser):
    def __init__(self):super().__init__();self.round=0;self.row=None;self.pick=False;self.rows=[]
    def handle_starttag(self,tag,attrs):
        a=dict(attrs)
        if tag=='div' and a.get('class')=='round-title':self.round+=1
        if tag=='tr' and self.round:self.row={'round':self.round}
        if tag=='td' and a.get('class')=='pick-number':self.pick=True
        if tag=='a' and self.row is not None and a.get('href','').startswith('/nfl/'):self.row.setdefault('slug',a['href'][5:])
    def handle_endtag(self,tag):
        if tag=='td':self.pick=False
        if tag=='tr' and self.row is not None:
            if self.row.get('pick') and self.row.get('slug'):self.rows.append(self.row)
            self.row=None
    def handle_data(self,data):
        if self.pick and self.row is not None and data.strip().isdigit():self.row['pick']=int(data.strip())
source=Path(sys.argv[1]);target=Path(sys.argv[2]);raw=source.read_bytes();parser=Order();parser.feed(raw.decode('utf8'))
clubs=json.loads((Path(__file__).resolve().parents[1]/'Content/Runtime/nfl-teams.json').read_text(encoding='utf8'))['teams']
slugs=['cardinals','falcons','ravens','bills','panthers','bears','bengals','browns','cowboys','broncos','lions','packers','texans','colts','jaguars','chiefs','raiders','chargers','rams','dolphins','vikings','patriots','saints','giants','jets','eagles','steelers','49ers','seahawks','buccaneers','titans','commanders'];mapping=dict(zip(slugs,clubs))
assert len(parser.rows)==257 and [r['pick'] for r in parser.rows]==list(range(1,258))
for row in parser.rows:row['team']=mapping.pop(row['slug'],None) if False else dict(zip(slugs,clubs))[row.pop('slug')]
rounds=[[r['team'] for r in parser.rows if r['round']==i][:32] for i in range(1,8)];assert all(len(r)==32 for r in rounds)
data={'format':1,'referenceId':'supplied-tankathon-2027-'+hashlib.sha256(raw).hexdigest()[:12],'title':'Full 2027 NFL Draft Order','source':'https://www.tankathon.com/nfl/full_draft','suppliedReference':True,'finalHistoricalOrder':False,'auditedOn':'2026-11-07','sourceFileSha256':hashlib.sha256(raw).hexdigest(),'originalFileModifiedUtc':__import__('datetime').datetime.fromtimestamp(source.stat().st_mtime,__import__('datetime').timezone.utc).isoformat(),'nativeMapping':'First 32 supplied entries in each round; extra compensatory slots cannot create players beyond the native 224. Trades and repeated destinations are retained.','firstOffseason':{'currentSeasonYear':2026,'currentYear':0,'nativeWeek':2,'nativeOffseasonStage':3,'userDisplayedWeek':1},'rounds':rounds,'suppliedPicks':parser.rows}
target.parent.mkdir(parents=True,exist_ok=True);target.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf8');print(json.dumps({'picks':len(parser.rows),'roundSizes':[sum(r['round']==i for r in parser.rows) for i in range(1,8)],'sha256':data['sourceFileSha256']}))

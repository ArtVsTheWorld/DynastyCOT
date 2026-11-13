"""Run the emitted report suffix; never touches game files/processes."""
from pathlib import Path
from dataclasses import replace
import copy,sys,time
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'Tools'))
from build_weekly_report_ui import suffix,matchup_suffix,picker_program
from build_recruit_notes_ui import Assembler
from grade_scales import label
from recruit_notes_vm import Machine

def program(scale,fog,colors,emit=suffix):
    a=Assembler();emit(a);p=a.finish();code=bytearray(p.code)
    for offsets,value in [(p.scale_offsets,scale),(p.fog_offsets,int(fog)),(p.color_offsets,int(colors))]:
        for at in offsets:code[at]=value
    return replace(p,code=bytes(code))

def render(rows,title='Players to Watch',scale=0,fog=True,colors=False):
    source=copy.deepcopy(rows);widgets={}
    for i,row in enumerate(rows[:4]):
        data={'display':row['value'],'ExtraInfo':{'fontColor':0xEEEEEE,'font':'native'}}
        w={'mData':data};w['SetLabel']=lambda value,d=data:d.update(display=value)
        w['SetExtraInfo']=lambda value,d=data:d['ExtraInfo'].update(value)
        widgets[f'mcValue{i}']=w
    root={'mData':{'display':title,'ExtraInfo':{'SituationArray':rows}},**widgets}
    Machine({'art_styleguide':{'Font':{'RESIZE_SCALEDOWN':'scaledown'}}},[]).run(program(scale,fog,colors),{1:root})
    assert rows==source,'Report/native producer data was mutated'
    return [widgets[f'mcValue{i}']['mData']['display'] for i in range(min(4,len(rows)))]

checks=0;began=time.perf_counter()
for scale in range(5):
    for fog in [False,True]:
        for colors in [False,True]:
            for rating in range(101):
                rows=[{'label':pos,'value':name+' '+str(rating)} for pos,name in [('QB','J. Henderson'),('RB','K. Udoh'),('WR','M. Jackson'),('TE','L. Johnson Jr.')]]
                actual=render(rows,scale=scale,fog=fog,colors=colors)
                wanted=label(rating,scale) if fog else str(rating)
                assert actual==[row['value'].rsplit(' ',1)[0]+' '+wanted for row in rows],(scale,fog,colors,rating,actual,wanted)
                checks+=1
for text in ['Passing Yards','Opponent Scouting Report','Ball Security','3rd Down','Players to Watch Other']:
    rows=[{'label':'QB','value':'J. Henderson 77'}];assert render(rows,title=text)==[rows[0]['value']];checks+=1
for value in ['J. Smith 101','J. Smith -1','J. Smith 77.0','J. Smith 077','J. Smith A+','J. Smith 77 ','NoRating', 'A'*161+' 77']:
    rows=[{'label':'QB','value':value}];assert render(rows)==[value];checks+=1
for pos in ['Yards/Game','Converted','Natl Rank','Points/Game']:
    rows=[{'label':pos,'value':'Team 77'}];assert render(rows)==['Team 77'];checks+=1
for name in ['L. Johnson Jr.','J. Smith III','A. Very-Long Hyphenated Lastname','J.  Multiple  Spaces','Q. Number 2']:
    rows=[{'label':'TE','value':name+' 75'}];assert render(rows)==[name+' '+label(75,0)];checks+=1

def matchup(value,scale=0,fog=True,colors=False,field='RecordLabel',index=7):
    source={field+str(index):value};before=copy.deepcopy(source);out={'display':value,'ExtraInfo':{'fontColor':0xEEEEEE}}
    root={('mcStatsLabel' if field=='RecordLabel' else 'mcStatsValue')+str(index):{
        'SetLabel':lambda v:out.update(display=v),'SetExtraInfo':lambda v:out['ExtraInfo'].update(v)}}
    Machine({'art_styleguide':{'Font':{'RESIZE_SCALEDOWN':'scaledown'}}},[]).run(program(scale,fog,colors,matchup_suffix),{1:root,3:source})
    assert source==before and out['ExtraInfo']['fontColor']==0xEEEEEE
    return out['display']

for scale in range(5):
    for fog in [False,True]:
        for colors in [False,True]:
            for rating in range(101):
                for field in ['RecordLabel','RecordValue']:
                    source=f'WR: M. Matthews {rating} vs K. Billiot {100-rating}'
                    wanted=f'WR: M. Matthews {label(rating,scale) if fog else rating} vs K. Billiot {label(100-rating,scale) if fog else 100-rating}'
                    assert matchup(source,scale,fog,colors,field)==wanted;checks+=1
for source in ['Record: 3-2','WR: 1 points','Rush yards: Player 72 vs Player 77',
               'WR: M. Matthews 101 vs K. Billiot 77','WR: M. Matthews 72 vs K. Billiot 077',
               'WR: M. Matthews 72 vs K. Billiot 77 vs Extra 50',
               'WR: M. Matthews 72 vs K. Billiot 77 ', 'WR: M. Matthews 72.0 vs K. Billiot 77']:
    assert matchup(source)==source;checks+=1
assert matchup('RT: L. Johnson Jr. 75 vs J. Number 2 88')==f'RT: L. Johnson Jr. {label(75,0)} vs J. Number 2 {label(88,0)}';checks+=1

def picker(rows,scale=0,fog=True,colors=False):
    original=copy.deepcopy(rows);out=[];calls=[]
    def native():calls.append('get');return rows
    root={'mTrainingAdapter':{'GetFocusTrainingPlayerList':native},
          'mView':{'GetTemplate':lambda binding:{'ExtraInfo':{'NativeStyle':'preserved'}}},
          'mOptionsProvider':{'AddItem':lambda row:out.append(row)}}
    Machine({'fb_shared_enum':{'BindingType':{'OPTIONS':1}}},[],max_steps=100000).run(program(scale,fog,colors,picker_program),{1:root})
    assert calls==['get'] and rows==original
    for row,shown in zip(rows,out):
        assert shown['key']==row['key'] and shown['value']==row['value']
        assert shown['display']==row['playerFirstName']+' '+row['playerLastName']
        assert shown['ExtraInfo']['NativeStyle']=='preserved'
        for target,source in [('number','number'),('pos','pos'),('height','height'),('weight','weight'),('college','college'),('xp','xp'),('Label3','devTrait'),('Label4','age'),('yearsPro','yearsPro')]:
            assert shown['ExtraInfo'][target]==row[source]
    return out

base=dict(playerFirstName='L.',playerLastName='Johnson Jr.',key=57,value=12345,number=72,pos='C',
          height='6 ft 3',weight=310,college='JMU',xp=615,devTrait='Normal',age=19,yearsPro=2)
for scale in range(5):
    for fog in [False,True]:
        for colors in [False,True]:
            for rating in range(101):
                row={**base,'overall':rating};out=picker([row],scale,fog,colors)
                wanted=label(rating,scale) if fog else (str(rating) if colors else rating)
                assert out[0]['ExtraInfo']['Label2']==wanted;checks+=1
assert picker([])==[];checks+=1
rows=[{**base,'key':i,'value':2000+i,'overall':i%101} for i in range(85)]
assert len(picker(rows,3))==85;checks+=1
print(f'PASS {checks} report display checks: all grades/options, exact report/position scope, suffixes/long names, malformed values, raw-stat/source preservation; offline VM {time.perf_counter()-began:.3f}s (not live menu timing)')

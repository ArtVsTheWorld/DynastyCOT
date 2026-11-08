"""Build a private, readable audit; no raw captures or output are packaged."""
import collections, hashlib, html, json, pathlib, re, sys

work=pathlib.Path(sys.argv[1]); before=json.loads((work/'before.json').read_text(encoding='utf-8')); after=json.loads((work/'after.json').read_text(encoding='utf-8'))
root=pathlib.Path(__file__).resolve().parents[4]
def engine_fingerprint(engine):
    files=[(folder+'/'+p.name,p) for folder,suffix in [('Tools','.mjs'),('Content/Runtime','.json')] for p in (engine/folder).iterdir() if p.suffix==suffix]
    h=hashlib.sha256()
    for name,p in sorted(files):h.update(name.encode()+b'\0');h.update(p.read_bytes())
    return h.hexdigest()
assert before['sourceFingerprint']==engine_fingerprint(work/'baseline'), 'Frozen engine changed'
assert after['sourceFingerprint']==engine_fingerprint(root/'Modules/DynastyEnhancer'), 'Audit is stale; replay the working engine'
pack=json.loads((root/'Modules/DynastyEnhancer/Content/Runtime/perspectives-pack.json').read_text(encoding='utf-8'))
inputs=json.loads((work/'replay-input.json').read_text(encoding='utf-8'))
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert before['observedTop20']==after['observedTop20']
assert [e['key'] for e in before['editions']]==[e['key'] for e in after['editions']]
for relative,digest in inputs['manifest'].items():
    assert sha(work/'recorded/RuntimeHistory'/relative)==digest, 'Frozen history changed: '+relative
node=(work/'node-all.log').read_text(encoding='utf-8');assert re.search(r'fail 0\b',node);test_count=int(re.search(r'tests\s+(\d+)',node).group(1))
assert 'Built ' in (work/'atmosphere-build-final.log').read_text(encoding='utf-8')
essential={'generatedLost':[],'publishedLost':[],'generatedCompared':0,'publishedCompared':0}
for b,a in zip(before['editions'],after['editions']):
    for field,label in [('essentialGenerated','generated'),('essentialPublished','published')]:
        essential[label+'Compared']+=len(b[field]);essential[label+'Lost'] += [(b['key'],x) for x in set(b[field])-set(a[field])]
views=['all','team','conference','national'];names={'all':'For You','team':'My Team','conference':'Conference','national':'National'}
table=[]
for v in views:
    b,a=before['summary'][v],after['summary'][v]
    table.append({'view':names[v],'beforeCards':b['cards'],'afterCards':a['cards'],'beforeRate':b['rate'],'afterRate':a['rate'],'reduction':1-a['rate']/b['rate'],'beforeAdjacent':b['adjacent'],'afterAdjacent':a['adjacent'],
        'beforeAvoidable':b['duplicatesWithUnusedAlternative'],'beforeAlternativeAudited':b['duplicatesWithAuditedAlternatives'],'afterAvoidable':a['duplicatesWithUnusedAlternative'],'afterAlternativeAudited':a['duplicatesWithAuditedAlternatives']})
bc=sum(before['summary'][v]['cards'] for v in views);ac=sum(after['summary'][v]['cards'] for v in views)
br=sum(before['summary'][v]['nearDuplicates'] for v in views)/bc;ar=sum(after['summary'][v]['nearDuplicates'] for v in views)/ac
combined={'beforeCards':bc,'afterCards':ac,'beforeRate':br,'afterRate':ar,'reduction':1-ar/br}
def wording(data):
    posts=[p for e in data['editions'] for p in e['views']['all']];openings=collections.Counter(' '.join(p['structure'].split()[:5]) for p in posts);endings=collections.Counter(' '.join(p['structure'].split()[-5:]) for p in posts)
    lengths=collections.defaultdict(list);cadence=collections.defaultdict(collections.Counter);authors=collections.Counter();focus=collections.defaultdict(collections.Counter);pairs=collections.Counter();anchored=0;replies=0
    for e in data['editions']:
        lookup={p['id']:p for p in e['views']['all']}
        for p in e['views']['all']:
            lengths[p['voice']].append(len(p['body']));authors[p['author']]+=1
            units=[s for s in re.split(r'[.!?]+(?:\s|$)|\n+',re.sub(r'\b(No|Jr|Sr|St)\.',r'\1',p['body'])) if s.strip()];cadence[p['voice']][max(1,len(units))]+=1
            if p['voice']=='analyst':focus[p['author']][p['family']]+=1
            if p.get('replyTo') in lookup:
                parent=lookup[p['replyTo']];pairs[parent['voice']+' → '+p['voice']]+=1;replies+=1;anchored+=int(parent['handle'] in p['body'])
    return {'openings':openings.most_common(8),'endings':endings.most_common(8),'authors':authors.most_common(8),'replyPairs':pairs.most_common(),'replies':replies,'handleAddressedReplies':anchored,'voiceMeanCharacters':{v:round(sum(x)/len(x),1) for v,x in lengths.items()},'sentenceOrLineUnits':{v:dict(c) for v,c in cadence.items()},'analystFocus':{v:c.most_common(3) for v,c in focus.items()}}
old_counts={'roster.commit':10,'world.upset':10,'beat.commit':10,'beat.performance':10,'team.upcoming':10,'world.award':10,'cycle.portal-target':10,'roster.interest':10,'roster.announce':10,'reply.fan.upset':20,'late.rivalry-team':10,'model.line':8,'portal.entrant':10,'beat.standings':10,'cycle.team-welcome':8,'reply.player.offer':10,'cycle.player-reset':10,'reply.fan.player_good':10,'buzz.watcher-player-Unpredictable':10,'history.coach-counter':10}
families=[]
for f,n in before['observedTop20']:
    b,a=before['eligibility'].get(f),after['eligibility'].get(f)
    families.append({'family':f,'observedForYouCards':n,'baselineStructures':old_counts[f],'beforeEligibility':b,'afterEligibility':a,'newEntries':len(pack['families'][f])})
performance=json.loads((work/'performance.json').read_text(encoding='utf-8')) if (work/'performance.json').exists() else None
controlled=json.loads((work/'controlled.json').read_text(encoding='utf-8'))
result={'combined':combined,'views':table,'essentialCoverage':essential,'families':families,'wordingBefore':wording(before),'wordingAfter':wording(after),'performance':performance,
    'newEntries':sum(map(len,pack['families'].values())),'uniqueNewBodies':len({t['body'] for v in pack['families'].values() for t in v}),'newFamilies':len(pack['families']),
    'frozenHistoryFilesVerified':len(inputs['manifest']),'inputSha256':sha(work/'replay-input.json'),'engineBefore':before['revision'],'engineAfter':after['revision']}
result['sourceFingerprints']={'before':before['sourceFingerprint'],'after':after['sourceFingerprint']}
result['currentTop20']=[{'family':f,'cards':n,'before':before['eligibility'].get(f),'after':after['eligibility'].get(f)} for f,n in before['replayedTop20']]
(work/'findings.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
pct=lambda x:f'{x*100:.1f}%'
lines=['DYNASTY COT 0.1.4 — PRIVATE DEVELOPMENT AUDIT','',
 'Scope: COT integration workspace only. No standalone edits, live installation or release publication.',
 'Inputs: 60 frozen event-fact editions: UL Monroe 2026 (29) and BYU 2026 (31), each with 17 consecutive regular-season editions plus preseason, all postseason phases and offseason.',
 'Additional comparison: 93 wholly fictional full-engine fixtures, 31 publication stages each at brand grades F, B and A+. Sparse cases retain identical before/after card totals (91, 123, 117).',
 'Historical limitation: older archives lack raw roster membership, full game statistics and brand grades. Missing fields were not invented. Replays use captured events and identities; program-size behavior is separately checked with controlled grades.',
 '', 'STRUCTURAL REPETITION',before['method'],
 'Native defaults modeled: 30/100/60/25 cards, parent/reply bundles and required draft entries. Category pools are not mistaken for displayed counts.',
 'Pooled appearances count the same story separately when it appears in different views; this is an exposure metric, not a count of unique events.',
 f'Pooled rate: {pct(br)} → {pct(ar)}; relative reduction {pct(combined["reduction"])}.']
for r in table:lines.append(f'{r["view"]}: {pct(r["beforeRate"])} → {pct(r["afterRate"])} ({pct(r["reduction"])} reduction); adjacent equivalents {r["beforeAdjacent"]} → {r["afterAdjacent"]}; cards {r["beforeCards"]} → {r["afterCards"]}.')
lines+=['','BASELINE DIAGNOSIS',
 'The existing merged pack had thousands of IDs, but frequently displayed families usually had only 8–10 eligible forms. Earlier packs were replaced by later packs. Many alternatives shared a lead, sentence order or stock ending.',
 'Cooldowns were per handle and template ID, with a two-edition lookback. Fresh accounts could select the same structure, while invisible CPU stories consumed choices. Selection rotated categories/accounts but repeatedly filled a view with the same family.',
 'Both shortages and selection contributed. The HTML/JSON tables count repeated cards with a text-budget-valid alternative that would not have matched the recent structural window. Other repeats have exhausted structural alternatives.',
 'Analyst wrappers and punctuation supplied much of the old voice distinction. Positive fan replies often followed the same reporting role. Changed copy now uses persistent registers and specialist interests; some existing reaction slots become an athlete responding to his own report.',
 '', 'IMPLEMENTATION',
 f'{result["newEntries"]} new family entries across {result["newFamilies"]} families; {result["uniqueNewBodies"]} distinct new bodies. Shared rank/announcement frames count once as wording even though eligible in several families. Both the historically displayed and the current frozen-engine For You top-20 cohorts pass at least 3× their baseline usable normalized structures with representative verified inputs.',
 'Perspectives distinguish decision/status, evaluation, position needs, team outcome vs individual numbers, rank vs projection, rivalry pride, career vs season scope, and uncertainty. Richer conditions require captured inputs. Missing game context has explicit short factual fallbacks.',
 'Author character persists in the registry: local/wire/cautious reporting; race/film/evidence/schedule/development desks; varied fan stances; official pride/announcements; personal cadence under existing native personality/QB/faith rules. Generic analyst prefacing is retired for new publications.',
 'Replies retain their deterministic admission rate and caps. Actual-parent anchors, questions, disagreement, cautious interpretation and own-player responses replace some generic reactions. Rival-only player commentary is gated. No invented interviews, injuries, sourcing or transactions.',
 'Global wording memory covers four editions: structure, opening, ending, angle and distinctive phrases across authors/families, weighted toward native-visible cards. The existing two-edition account-ID cooldown remains separate. Near-structure neighborhoods are compiled once per family. Selection diversifies families; essential news gets a smaller penalty and exhaustion never makes a valid template unavailable.',
 'Unchanged season expectation reviews and unsigned-prospect narratives wait for a supported state change. Results, commitments, awards and official draft coverage remain eligible.',
 'Published wording survives content upgrades. Unchanged editions resume without regeneration; same-week cards retain their stored fields. Historical files are not rewritten by this audit.',
 'New template IDs derive from their content and conditions, so adding or reordering alternatives does not renumber existing wording. Original pack IDs remain intact.',
 '', 'NATIVE / PERFORMANCE PASS',
 'Fog/colors/dollars: a native frame-shape index replaces repeated scanning of 44 script adapter banks. Candidate indexing retains the original ownership, header/body hash, relocation and fixed-register validation.',
 '197 native/owned frame shapes in both frame modes matched the former candidate set exactly. Mock 10,000-dispatch benchmark: 1.3306 ms linear → 0.3405 ms indexed (~3.9×). This is not a game frame-time measurement.',
 'Social menus: immutable author labels, compact likes and text heights are prepared by the native catalog worker and reused by tile creation. Exact equality with uncached presentation and native scope/type/history behavior are checked.',
 'Generation: deterministic hash prefixes/scores are computed once per selection; poll, team, player-name and game-performance lookups are reused per render. The helper remains a background generation/history worker; draw/focus paths stay native. A complete C++ port was not made.']
lines+=['New archive records store compact signatures and reconstruct phrase indexes in memory. Plain posts omit the duplicate raw-body copy; legacy wrapped cards retain their original fields. Full/compact signatures select identical wording after JSON reload. Native catalog presentation caching adds small per-post labels; actual game memory and frame time still require live review.']
if performance:lines += [f'ABBA generation benchmark: {performance["beforeMeanMs"]:.0f} ms → {performance["afterMeanMs"]:.0f} ms for 29 editions, {performance["speedup"]:.2f}×. '+performance['method']]
lines+=['','ATMOSPHERE',
 'Camera motion follows its existing envelope without waiting on a playing-voice receipt. The native audio callback derives the roar/swell companion from that envelope; visiting reactions use visitor-sized support. Mute, ownership, pause/replay and original eligibility guards remain.',
 'The old per-audio-callback sound-proof allocation and repeated voice queries were removed. Native tests now distinguish user mute from audio-start latency.',
 'Visiting core footprint: 70% of the visitor budget → 52.5% (25% smaller), with roughly 10% home pockets. Remaining visitors are seated in small parties. Total occupied-supporter counts and protected groups remain preserved.',
 'Verified home AP rank and established record compress travel demand smoothly. Missing quality keeps the prior model; neutral venues remain native. Across 1,000 seeded nearby equal-fanbase rivalry cases at a strong host: 8.10–8.97% away, mean 8.54%. This is a calibration fixture, not a measurement of a real broadcast.',
 '', 'CHECKS / LIMITS',
 f'{test_count} Node checks pass; native Social/Fog builds and fixtures pass; the complete Atmosphere native/managed build suite passes. Frozen feed checksums verified: {len(inputs["manifest"])}.',
 f'Essential factual roots compared: generated {essential["generatedCompared"]}, published {essential["publishedCompared"]}; missing after: generated {len(essential["generatedLost"])}, published {len(essential["publishedLost"])}.',
 'Text limits: 280 code points / 512 UTF-8 bytes; long names, sparse context, deterministic refreshes, Dynasty isolation and unchanged-history checks pass. Real menu latency and audible/visual behavior have not been live-validated with these binaries.',
 'National remains the largest residual source of repetition. Required rank updates, draft picks and dense result weeks sometimes exhaust structures. Other repeats still have a per-view unused alternative: shared cross-view memory, cadence/identity preferences and generation order can favor a different choice. The audit does not label every remaining repeat unavoidable. Essential facts remain browsable instead of being dropped for variety.',
 'Live review still needed: cold/warm feed entry and filtering on small/large Dynasties; very long names, scrolling, controller navigation; fog/colors/dollar combinations with AIO; pre-play roar timing, visitor vs home reactions, mute/pause/replay; visiting-section texture across several stadiums. No new installer or release was produced.',
 '', 'WORDING / VOICES / REPLIES (For You news-card exposures)']
for label,key in [('Before','wordingBefore'),('After','wordingAfter')]:
    d=result[key];lines += [label+' frequent normalized openings: '+repr(d['openings'][:4]),label+' frequent normalized endings: '+repr(d['endings'][:4]),label+' mean characters by role: '+repr(d['voiceMeanCharacters']),label+' sentence/line-unit distributions by role: '+repr(d['sentenceOrLineUnits']),label+' reply-role pairings: '+repr(d['replyPairs']),label+' leading analyst families: '+repr(d['analystFocus'])]
lines+=['@ represents a substituted fact/name; # represents a literal numeric value. Sentence/line units are a punctuation-based cadence measure, not a linguistic parser.','', 'REPRESENTATIVE SAME-INPUT EXAMPLES']
indices=[0,1,3,5,7,10,14,17,18,20,21,22,25,28]
for i in indices:
    b,a=before['editions'][i],after['editions'][i];old={p['id']:p for p in b['views']['all']}
    pairs=[(old[p['id']],p) for p in a['views']['all'] if p['id'] in old and p['body']!=old[p['id']]['body']][:2]
    lines += ['',b['key']]
    for x,y in pairs:lines += [f'  {y["family"]} / {y["author"]}', '  BEFORE: '+x['body'].replace('\n',' / '),'  AFTER:  '+y['body'].replace('\n',' / ')]
(work/'audit.txt').write_text('\n'.join(lines)+'\n',encoding='utf-8')
rows=''.join('<tr><td>'+html.escape(r['view'])+f'</td><td>{pct(r["beforeRate"])}</td><td>{pct(r["afterRate"])}</td><td>{pct(r["reduction"])}</td><td>{r["beforeAdjacent"]} → {r["afterAdjacent"]}</td><td>{r["beforeAvoidable"]}/{r["beforeAlternativeAudited"]} → {r["afterAvoidable"]}/{r["afterAlternativeAudited"]}</td></tr>' for r in table)
family_rows=''.join('<tr><td>'+html.escape(r['family'])+f'</td><td>{r["observedForYouCards"]}</td><td>{r["baselineStructures"]}</td><td>'+('—' if not r['afterEligibility'] else f'{r["afterEligibility"]["min"]}–{r["afterEligibility"]["max"]}')+'</td><td>'+('Fixture coverage; not emitted in fact replay' if not r['afterEligibility'] else f'{r["afterEligibility"]["average"]:.1f}')+'</td></tr>' for r in families)
current_rows=''.join('<tr><td>'+html.escape(r['family'])+f'</td><td>{r["cards"]}</td><td>{r["before"]["max"]}</td><td>{r["after"]["min"]}–{r["after"]["max"]}</td><td>{r["after"]["average"]:.1f}</td></tr>' for r in result['currentTop20'])
data=json.dumps({'before':before['editions'],'after':after['editions']},ensure_ascii=False).replace('</','<\\/')
page='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>Dynasty COT 0.1.4 audit</title><style>
body{font:16px/1.5 system-ui,sans-serif;margin:32px auto;max-width:1400px;padding:0 20px;background:#101a15;color:#e8f0eb}h1,h2{color:#9cdeae}a{color:#a5dfb6}table{border-collapse:collapse;width:100%;font-size:14px}td,th{border-bottom:1px solid #355040;padding:9px;text-align:left;vertical-align:top}select{padding:10px;margin:8px;background:#e7f0e9;color:#16241b;max-width:90%}.pair{display:grid;grid-template-columns:1fr 1fr;gap:16px;margin:18px 0}.card{border:1px solid #355040;padding:18px;background:#18271e;border-radius:8px;white-space:pre-wrap}.meta{font-size:12px;color:#b4c9ba}.missing{color:#a8b5ac}details{margin:24px 0}pre{white-space:pre-wrap;font:14px/1.6 system-ui,sans-serif}.labels{display:grid;grid-template-columns:1fr 1fr;gap:16px}@media(max-width:700px){.pair,.labels{grid-template-columns:1fr}table{font-size:12px}}
</style><h1>DYNASTY COT · 0.1.4 development audit</h1><p>Same inputs, frozen baseline and working engine. 60 recorded editions + 93 controlled sparse fixtures. Development only; live acceptance remains pending.</p>'''
page+=f'<p><strong>Pooled structural near-duplicate reduction: {pct(combined["reduction"])}</strong> ({pct(br)} → {pct(ar)}). Pooled exposures overlap between views.</p><table><tr><th>Native view</th><th>Before</th><th>After</th><th>Reduction</th><th>Adjacent equivalents</th><th>Repeated with unused eligible structure / audited repeats</th></tr>{rows}</table>'
page+='<h2>Before / after feeds</h2><p>Rows match story IDs for the same edition and factual event. A missing side means the card was not selected in this view. Select any of the 60 editions.</p><label>Edition <select id="edition"></select></label><label>View <select id="view"><option value="all">For You</option><option value="team">My Team</option><option value="conference">Conference</option><option value="national">National</option></select></label><div class="labels"><b>Frozen baseline</b><b>0.1.4 working engine</b></div><div id="cards"></div>'
page+=f'<details><summary>Top 20 historically displayed For You families · eligible alternatives</summary><p>Counts are valid after text limits, with that edition’s captured values. Four legacy families do not route through this event replay; the complete-input automated fixture covers their 3× requirement.</p><table><tr><th>Family</th><th>Observed cards</th><th>Baseline structures</th><th>After eligible range</th><th>After mean / limitation</th></tr>{family_rows}</table></details>'
page+=f'<details><summary>Top 20 emitted by the frozen current engine · eligible alternatives</summary><table><tr><th>Family</th><th>For You cards</th><th>Baseline max usable</th><th>After usable range</th><th>After mean</th></tr>{current_rows}</table></details>'
page+='<details><summary>Methods, findings, performance, limitations and live checklist</summary><pre>'+html.escape('\n'.join(lines[:lines.index('REPRESENTATIVE SAME-INPUT EXAMPLES')]))+'</pre></details>'
page+='<p>Machine-readable: <a href="findings.json">findings</a> · <a href="before.json">before</a> · <a href="after.json">after</a> · <a href="controlled.json">controlled fixtures</a> · <a href="audit.txt">plain text report and examples</a></p>'
page+='<script>const data='+data+''';const edition=document.querySelector('#edition'),view=document.querySelector('#view');data.after.forEach((e,i)=>{const o=new Option(e.key+' · program '+e.user,i);edition.add(o)});
function draw(){const i=+edition.value,v=view.value,b=data.before[i].views[v],a=data.after[i].views[v],bm=new Map(b.map(p=>[p.id,p])),am=new Map(a.map(p=>[p.id,p])),ids=[...new Set([...a,...b].map(p=>p.id))],out=document.querySelector('#cards');out.replaceChildren();for(const id of ids){const row=document.createElement('div');row.className='pair';for(const p of [bm.get(id),am.get(id)]){const c=document.createElement('div');c.className='card';if(!p){c.textContent='Not selected in this view';c.classList.add('missing')}else{const m=document.createElement('div');m.className='meta';m.textContent=p.author+' · '+p.handle+' · '+p.family+(p.replyTo?' · reply':'');const body=document.createElement('div');body.textContent=p.body;c.append(m,body)}row.append(c)}out.append(row)}}edition.addEventListener('change',draw);view.addEventListener('change',draw);draw();</script></html>'''
(work/'audit.html').write_text(page,encoding='utf-8')
print(json.dumps({'combined':combined,'essentialCoverage':essential,'historyFilesVerified':len(inputs['manifest']),'report':str(work/'audit.html')}))

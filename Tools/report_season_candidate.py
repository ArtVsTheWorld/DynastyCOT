"""Readable private handoff from frozen evidence, never from the live installation."""
from pathlib import Path
import collections,hashlib,html,json,re,statistics
ROOT=Path(__file__).resolve().parents[1];OUT=ROOT/'Build/RC016'
read=lambda name:json.loads((OUT/name).read_text(encoding='utf-8'))
b,a=read('before-coverage.json'),read('after-coverage.json')
assert len(b['editions'])==len(a['editions'])==32
assert all(x['inputSha256']==y['inputSha256'] for x,y in zip(b['editions'],a['editions']))
coverage=read('coverage-verification.json');assert coverage['lostStillEligibleProtectedRoots']==0
eligibility=read('final-eligibility.json')['eligibility']
corrections=len(json.loads((ROOT/'Modules/DynastyEnhancer/Content/Runtime/season-fixes-pack.json').read_text(encoding='utf-8'))['overrides'])
def near(x,y):
    x,y=x.get('variety',{}).get('structure',''),y.get('variety',{}).get('structure','')
    if not x or not y:return False
    if x==y:return True
    def shingles(s):
        w=s.split();return set(' '.join(w[i:i+4]) for i in range(len(w)-3))
    u,v=shingles(x),shingles(y)
    return bool(u and v and len(u&v)/max(len(u),len(v))>=.7)
def metrics(d,view):
    duplicates=pairs=picks=replies=cards=0
    for e in d['editions']:
        posts=e['views'][view];cards+=len(posts);roots=[p for p in posts if not p.get('replyTo')]
        pairs+=max(0,len(roots)-1);duplicates+=sum(near(x,y) for x,y in zip(roots,roots[1:]))
        top=collections.Counter(p['id'].split(':pick:')[0] for p in posts[:5] if re.fullmatch(r'buzz\.pick-\d+',p.get('family','')))
        picks+=sum(max(0,n-1) for n in top.values())
        seen=set()
        for p in posts:
            if p.get('family')!='buzz.pick-reply':continue
            key=(p['handle'],p['id'].split(':pick:')[0]);replies+=key in seen;seen.add(key)
    return dict(cards=cards,pairs=pairs,adjacentNearDuplicates=duplicates,rate=duplicates/pairs if pairs else 0,redundantTopFiveMatchupPicks=picks,repeatedFanPredictionReplies=replies)
comparisons={v:dict(before=metrics(b,v),after=metrics(a,v)) for v in ['all','national','conference','team']}
native=[]
for fixture in ['2026-PreSeason-0','2026-RegularSeason-12','2026-NationalChampionship-20']:
    old=read('Native/'+fixture+'-before.json');new=read('Native/'+fixture+'-after.json')
    assert old['datasetHash']==new['datasetHash'] and old['datasetBytes']==new['datasetBytes']
    native.append((fixture,statistics.median(old['runsMs']),statistics.median(new['runsMs']),old['rows']))
catalog=read('catalog-performance.json');catalog_medians={m:statistics.median(r['ms'] for r in catalog['runs'] if r['mode']==m) for m in ['before','after']}
worker=read('worker-performance.json')
worker_rows=[]
for fixture in worker['fixtures']:
    med=lambda mode,key:statistics.median(r[key] for r in fixture['runs'] if r['mode']==mode)
    worker_rows.append(f"| {fixture['name']} | {med('before','generationMs'):.0f} → {med('after','generationMs'):.0f} ms | {med('before','totalMs'):.0f} → {med('after','totalMs'):.0f} ms |")
calendars=[(e['key'],body) for e in a['editions'] for body in e['calendar']];assert len(calendars)==5
pff=sum(e['pff'] for e in a['editions']);assert pff>0 and sum(e['pff'] for e in b['editions'])==0
before_checks=read('checkpoints-before.json')
for x in before_checks:assert hashlib.sha256(Path(x['path']).read_bytes()).hexdigest()==x['sha256'],'Checkpoint changed: '+x['path']
# Preserve every original season-test evidence artifact, including the original report.
integrity=json.loads((ROOT/'Build/SeasonTest-OSU-2026/evidence-integrity.json').read_text(encoding='utf-8'))
for rel,expected in integrity.items():assert hashlib.sha256((ROOT/'Build/SeasonTest-OSU-2026'/rel).read_bytes()).hexdigest()==expected,'Original evidence changed: '+rel
families=collections.Counter(p['family'] for e in b['editions'] for p in e['views']['all'])
family_rows=[]
for family,count in families.most_common(20):
    x=eligibility.get(family,{})
    family_rows.append(f"| `{family}` | {count} | {x.get('min','—')} / {x.get('average',0):.1f} / {x.get('max','—')} |")
metric_rows=[]
for view,label in [('all','For You'),('national','National'),('conference','Conference'),('team','My Team')]:
    old,new=comparisons[view]['before'],comparisons[view]['after']
    metric_rows.append(f"| {label} | {old['cards']:,} → {new['cards']:,} | {old['adjacentNearDuplicates']}/{old['pairs']} ({old['rate']:.2%}) → {new['adjacentNearDuplicates']}/{new['pairs']} ({new['rate']:.2%}) |")
def candidates(d):return {(e['key'],p['id']):p for e in d['editions'] for v in ['all','national'] for p in e['views'][v]}
old_posts,new_posts=candidates(b),candidates(a);samples=[]
for key,p in old_posts.items():
    q=new_posts.get(key)
    if not q or p['body']==q['body']:continue
    if any(s[0]==p['family'] for s in samples):continue
    if p['family'] in ['analysis.player.passing-td-leader','analysis.player.sacks-leader','carry.training','review.title-debate','draft.irrelevant','draft.top-ten','roster.commit','portal.entrant','playoff.rank_move'] or re.search(r'\b1 points\b|\ba 11[–-]|\ba \$80K|rankings has',p['body']):samples.append((p['family'],p['body'],q['body']))
samples=samples[:10]
sample_md='\n\n'.join(f"**{f}**\n\nBefore: {old}\n\nAfter: {new}" for f,old,new in samples)
performance_rows='\n'.join(f'| {f} | {old:.0f} ms → {new:.0f} ms | {rows:,} |' for f,old,new,rows in native)
fy=comparisons['all'];national=comparisons['national']
report=f'''# Dynasty COT 0.1.17-private — season review and test candidate

Built from the COT integration workspace against baseline `eb78401` / 0.1.15-private. No live installation, checkpoint overwrite or publication. The Team Needs N avatar is intentional and retained; the earlier report's Nebraska interpretation is superseded.

## What the full-season audit revealed

The earlier live run reached preseason 2027 without an observed crash or blank Social Media page. It exposed a disconnected PFF publication path, missing neutral-site wins, repeated recruit-capture failures, recognized-but-unpublished calendar events, repetitive prediction ordering, awkward wording, and several native/AIO UI gaps. Existing Analytics grade distribution was healthy: the final qualified sample had 5,217 players, mean 68.20, with 685 at 80+. Every position group, including OL, had both high and low grades. Grades were not recalibrated in this candidate.

This pass replays **32 captures / 31 distinct edition keys**, from preseason through regular season, bowls/playoffs, offseason and the next preseason. One archived “Week 5” raw capture instead contains Week 6, and Week 7 appears twice; the original live report documents the capture gap. These are fresh counterfactual publications from identical frozen inputs, not edits to the original published year. They are not a second live season test.

## Implemented and verified

| Finding | Change and evidence |
|---|---|
| No PFF stories | Strict native 32-bit signed/unsigned LeagueID comparison at both interfaces. Same-Dynasty/year/season/sample gates remain. Replay: **0 → {pff} PFF stories**, with no blanket promise of one every week. |
| 2,533 hometown errors in the live run | Explicit recruit capture fields/roots eliminate the include-order dependency. Missing location data defers that prospect's background without creating a different persistent identity. Identical failures back off to 60 seconds; a changed session resets backoff. Native source/persistence fixtures pass; the new live capture path remains untested. |
| Neutral-site results missing | Native current-season WINS/LOSSES/TIES replace Home+Road-only totals. The Wisconsin 8–3 versus native 9–3 case is covered. |
| Calendar publication gap | One root per recognized event survives routine/reaction quotas. All **five** calendar events publish in replay, including both Rivalry Week and Thanksgiving. Captured dates, fourth-Thursday logic, Week 13, rollover and refresh guards remain. |
| Template quality | **{corrections} existing templates corrected; zero new variants.** Render-time agreement handles “1 point,” “an 11–1 record,” and “an $80K change.” Repairs sole/tied leader sentence composition, unsupported prior-pick claims, duplicate certainty/final-pick endings and the training subject fragment. Existing humor, guards, account identities and all **594 families / 8,037 variants** remain. |
| Repeated picks/replies | Equivalent matchup picks are spaced across desks, repeated conclusions receive a penalty, the same fan does not repeat its pick reaction under every desk, and verified local results get prominence. Draft announcements retain pick order. Weekly budgets, important-news protections and the **300-card National cap** remain. |
| Draft fog under AIO | Separately verifies AIO 1.4.1's relocated strategy with the same full 207-byte stock body; no weakened fingerprint or shared mixed-asset bank. Native VM/frame tests retain numbers for sorting and native colors. Live acceptance pending. |
| Analytics UI | Readable postseason week labels; explicit navigation title in addition to roster info title. Cards, full Ratings fog, selection/Back, completed-week gating and year reset have native/offline regression coverage. Signing/Training headings need live confirmation. |
| Weekly explanation clipping | Native scale-down bounds use the actual row highlight. Copy and native text colors remain. VM bounds/reused-widget/color tests pass; live fit pending. |

No still-eligible protected root was lost among **{coverage['checkedProtectedRoots']:,} checked protected roots**. The {len(coverage['factDrivenChanges'])} missing old bowl-watch/no-room/position angles are explained individually in `coverage-verification.json`: corrected native records make their old eligibility false. They were not sacrificed for PFF or seasonal chatter.

## Repetition and eligible alternatives

Metric: adjacent root-card pairs whose normalized template structures are identical or share at least 70% of four-word shingles. Names/numbers are template slots, so renaming a player cannot improve this score. Replies are excluded from adjacency; this is a structural indicator, not a semantic-quality score. Counts cover replayed display windows, not every stored post or manual reading of every live card.

| View | Displayed cards before → after | Adjacent structural near-duplicates before → after |
|---|---:|---|
{chr(10).join(metric_rows)}

For You's redundant same-matchup picks among the first five cards: **{fy['before']['redundantTopFiveMatchupPicks']} → {fy['after']['redundantTopFiveMatchupPicks']}**. Repeated same-account prediction replies: **{fy['before']['repeatedFanPredictionReplies']} → {fy['after']['repeatedFanPredictionReplies']}**. The pass does not claim a new 50% corpus-wide reduction. New factual PFF/calendar coverage can change card counts without changing budgets; obsolete record-based angles can disappear.

Eligible counts below are final-candidate measurements across twelve representative preseason, regular/postseason and offseason fixtures, after fact/role/length guards, per render attempt (including the variety pass), not raw template totals. A dash means that family was not attempted in those twelve fixtures. It should not be read as an audit of every possible native input combination.

| Frequently displayed family | Baseline For You cards | Eligible min / mean / max |
|---|---:|---:|
{chr(10).join(family_rows)}

## Before / after examples

{sample_md}

The complete machine-readable replay remains in the private evidence folder. The WMU–Michigan 31–26 upset, sparse inputs, long names, parent-specific replies, author styles, factual units, deterministic refreshes and published-wording preservation are additionally covered by the existing/new automated fixtures. This pass used programmatic checks across all runtime families and targeted editorial review of displayed problems; it is not a claim that a person read every possible template interpolation.

## Performance

Matched offline native dataset preparation, three runs each; JSON file loading excluded. Before/after serialized dataset fingerprints and byte counts match, with zero capture-adapter issues in these fixtures. No grade model or native result data changed.

| Fixture | Median preparation | Recorded player-game rows |
|---|---:|---:|
{performance_rows}

For a 22-edition championship catalog, repeated sorting was **{catalog_medians['before']:.0f} → {catalog_medians['after']:.0f} ms median**. It now reuses immutable published selections. This is catalog construction, not native widget rendering. Preseason's tiny timing difference is noise/no demonstrated improvement. The idle packed probes were not relaxed; freshness/ownership checks remain.

The separate same-input generation/catalog benchmark is in `worker-performance.json`:

| Fixture | Generation alone, median | Generation + catalog, median |
|---|---:|---:|
{chr(10).join(worker_rows)}

Generation alone was essentially flat in two fixtures and slower in the offseason fixture; catalog savings improved the combined measured work. Replay diagnostics and parallel test/build activity are not used to claim live speedups. The previous live run's roughly 60-FPS observations and generation/capture counters do not prove zero feature overhead. This candidate needs a controlled live menu/frame-time comparison. Social rendering still uses native News widgets; its generation remains in the existing verified Node worker. Analytics capture/grading and cards remain native. This pass adds no external helper or bypass of helper verification.

## Automated verification

- All four modules rebuilt from source; 460 Node checks and 29 offline native UI scripts passed.
- 7,317 Analytics model/distribution checks and 2,491 Analytics integration checks passed; resume identity/journal/development-isolation and stock/AIO draft frame fixtures passed.
- Package checks cover 106 core cases, all 16 module selections, 17 actual assembly/resource/helper cases and 10 packaged Analytics ABI/build-pair/license cases.
- Final versioned installer: 36 GUI/engine fixture checks; manual updater: 95 discovery/download/install/rollback and failure checks; release metadata/privacy: 15 checks.
- Actual setup layout captured at 100/125/150%, including 1080/1152 working-area bounds. The Install captures at 100% and bounded 150% were also visually reviewed; other pages have automated layout coverage.
- Native fingerprints, scope/ownership checks, per-Dynasty profiles, helper integrity, numeric sorting/edit data and history contracts remain enforced. These results are not live-game acceptance.

## Full module review and remaining limits

| Module | Bounded review / regression coverage | Remaining live acceptance |
|---|---|---|
| Beyond the Field | Feed/history/scopes/budgets, draft, seasonal dates, PFF model inputs/distribution, cards/Back/week lifecycle, fog/colors, money edits/redraw/reused widgets and native ownership. | Actual new capture/retry behavior; Signing/Training titles; Draft OVR with active AIO; footer fit/selected contrast; money after edits. |
| Gameday Atmosphere | Existing tests for visitor influence, travel/neutral/protected groups, saved plans, winning-away thinning, sound/shake, cheer/TD priority and replay handoffs. No unrelated model changes. | Winning visitors thin slowly in a blowout; relevant sound accompanies shake; touchdown cheer/replay presentation. Kicker animation support remains unproven. |
| Extra Settings | Per-Dynasty clock identity/defaults, native menu writer/creation controls, helper integrity, offline eligibility and all module combinations. | Create/save/reload and switch between two Dynasties with different clocks. |
| Expanded Practice | Native hook ownership and UI/transition contract tests; existing modes retained. | Change modes while lined up, start another play, switch sides and exit cleanly. |

**Unresolved:** Donor Ledger's duplicated amounts need a callback/ownership trace in the active AIO combination. No speculative suppression was added. Native/AIO decommit/tampering/dummy-player inconsistencies from the live run remain unassigned without an isolated comparison. No new native icon replacement is proposed. View Player Stats stays hidden and National Analytics remains removed as requested.

## Safety, distribution and handoff

All {len(before_checks)} discovered Ohio State/EPUFT checkpoint files retain their recorded hashes. The original `DYNASTY-TOSUPRESZN` is still `8c3f7c7ecd0c…3f6ff3ed4`. All {len(integrity)} original season-test evidence artifacts retain their SHA-256 hashes. Existing saves, settings, account identities, modeled recruit journals and published archives were not modified. Earlier season archives remain stored even though the normal week picker shows only the active season.

Build/verification logs and fixture reports accompany this handoff. The installer is source-built, keeps independent module selection, selected/full uninstall and transactional backups, and retains manual update/download/rollback controls. No GitHub release or live installation was performed. Installer/ZIP/SHA256 links and the prioritized checkpoint checklist are delivered separately.
'''
(OUT/'report.md').write_text(report,encoding='utf-8')
# A standalone HTML reading view, retaining the exact report text and a visual sample table.
sample_html=''.join('<tr><td>'+html.escape(f)+'</td><td>'+html.escape(old)+'</td><td>'+html.escape(new)+'</td></tr>' for f,old,new in samples)
document='''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1"><title>Dynasty COT 0.1.17 private report</title><style>body{background:#f1f5f2;color:#183426;font:16px/1.55 system-ui;margin:0}main{max-width:1150px;margin:auto;padding:28px}h1{font-size:30px}pre{white-space:pre-wrap;overflow-wrap:anywhere;font:15px/1.6 system-ui;background:white;padding:24px;border-radius:14px}table{border-collapse:collapse;background:white;width:100%}td,th{padding:14px;border:1px solid #cfddd2;text-align:left;vertical-align:top}a{color:#006b39}</style><main><h1>Dynasty COT 0.1.17-private</h1><p>Private candidate • no live installation or publishing</p><p><a href="testing-checklist.md">Checkpoint testing checklist</a> · <a href="report.md">Markdown report</a></p><h2>Representative wording</h2><table><tr><th>Family</th><th>Before</th><th>After</th></tr>'''+sample_html+'</table><h2>Full evidence report</h2><pre>'+html.escape(report)+'</pre></main></html>'
(OUT/'report.html').write_text(document,encoding='utf-8')
(OUT/'comparison-summary.json').write_text(json.dumps(dict(comparisons=comparisons,pff=pff,calendarStories=len(calendars),nativePreparation=native,catalogMedians=catalog_medians,checkpointHashesUnchanged=len(before_checks)),indent=2),encoding='utf-8')
print(json.dumps(dict(pff=pff,calendars=len(calendars),forYou=fy,catalog=catalog_medians,checkpointHashesUnchanged=len(before_checks))))

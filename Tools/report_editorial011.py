"""Summarize private replay evidence without committing captures or histories."""
from pathlib import Path
import collections
import json
ROOT=Path(__file__).resolve().parents[1]
work=ROOT/'Build/Editorial011'
read=lambda name:json.loads((work/name).read_text(encoding='utf-8'))
before,after=read('before.json'),read('after-final.json')
supplemental=read('supplemental-after.json')
decisions=read('editorial-decisions.json')
perf=read('performance-final.json')
assert before['inputSha256']==after['inputSha256']
assert len(before['editions'])==len(after['editions'])==31
lost=[];changed_identities=[]
for old,new in zip(before['editions'],after['editions']):
    assert old['key']==new['key'] and old['generated']==new['generated'] and old['roots']==new['roots']
    missing=set(old['essentialPublished'])-set(new['essentialPublished'])
    if missing:lost.append((old['key'],sorted(missing)))
    assert new['primaryAwardConservation']
    for view in ('all','team','conference','national'):
        old_posts={p['id']:p for p in old['views'][view]}
        for p in new['views'][view]:
            prev=old_posts.get(p['id'])
            if prev and any(prev[k]!=p[k] for k in ('author','handle','voice')):changed_identities.append(p['id'])
assert not lost and not changed_identities
new_templates=sum('.editorial011.' in p['templateId'] for e in after['editions'] for p in e['views']['national'])
rows=[]
for view,label in [('all','For You'),('team','My Team'),('conference','Conference'),('national','National')]:
    old,new=before['summary'][view],after['summary'][view]
    assert old['cards']==new['cards']
    rows.append(f"| {label} | {old['cards']:,} | {old['nearDuplicates']:,} → {new['nearDuplicates']:,} | {(1-new['nearDuplicates']/old['nearDuplicates'])*100:.1f}% | {old['adjacent']:,} → {new['adjacent']:,} |")
table='\n'.join(rows)
national=collections.Counter(p['family'] for e in before['editions'] for p in e['views']['national'])
personal=collections.Counter(p['family'] for e in before['editions'] for p in e['views']['all'])
family_rows=[]
for f in decisions['primaryFamilies']:
    b,a=before['eligibility'][f],after['eligibility'][f]
    family_rows.append(f"| `{f}` | {personal[f]} / {national[f]} | {b['average']:.1f} → {a['average']:.1f} | {b['min']} → {a['min']} |")
family_table='\n'.join(family_rows)
voice_rows=[]
for role in ('beat','analyst','official','fan','player','recruit'):
    old=[len(p['body'].split()) for e in before['editions'] for p in e['views']['all'] if p['voice']==role]
    new=[len(p['body'].split()) for e in after['editions'] for p in e['views']['all'] if p['voice']==role]
    voice_rows.append(f"| {role} | {len(new)} | {sum(old)/len(old):.1f} → {sum(new)/len(new):.1f} | {min(new)}–{max(new)} |")
voice_table='\n'.join(voice_rows)
total=lambda d,k:sum(e[k] for e in d['editions'])
reason_counts=collections.Counter(d['decision'] for d in decisions['decisions'])
retained=sum(not d['changed'] for d in decisions['decisions'])
public=f"""# Dynasty COT 0.1.11 editorial verification

Private source-built test candidate, compared with 0.1.10 (`f40a80fe27a3cc23cbd53d255325c9e563b375f4`). No live installation or publication. Author: Ace.

## Audit and finite implementation

Reviewed the supplied checkpoint-09 proposal against its exact exported source hash `{decisions['sourceSha256']}`: 585 families, 7,824 existing templates, 6,129 proposed replacements, zero proposed additions. Import checks require the original ID, family and wording to match; a stale proposal fails rather than patching another template.

The review retained {retained:,} proposed originals where voice, humor, historical wording, observation timing, projection provenance or meaningful status details were stronger. It changes {decisions['changedExisting']:,} existing bodies, including unit corrections to proposal-unchanged entries. Metadata, guards and existing IDs stay in place. For example, the player's opponent is `vs. opponent`, never a team attribution; complete result sentences stay separate; losses after a Q3 lead do not imply a 15-minute quarter or a regulation finish. Catch-up commitments retain their observation interval. Per-game and pass-attempt denominators are explicit. Coach/transfer/undrafted and named-poll distinctions survive shortening.

Exactly 150 new variants: six apiece in 20 primary families (120), plus 30 compact factual alternatives and parent-engaged replies across ten supplemental families. The latter cover dense statistical/NIL/matchup copy, upset celebration/frustration/rivalry responses and restrained player replies. No second expansion was performed.

Family choice used the current native displayed windows from the recorded replay, plus historical full-year audit evidence. The high-volume National families differ from the personalized feed. Counts below are displayed cards, not stored totals; eligibility averages/minima are final rendered candidates per audit callback, including both rendering passes, not raw template counts or unique events.

| Primary family | For You / National baseline cards | Mean usable alternatives before → after | Minimum before → after |
|---|---:|---:|---:|
{family_table}

The selection audit found retired 30/25/60/100-card assumptions in repetition memory/refinement, although the native UI shows complete local scopes and up to 250 National cards. The candidate now uses that actual native selection contract. A weak-key cache reuses immutable archive projections; near-shape matching reuses structural matches while reading current weights. Native/JavaScript parity is tested over sparse/busy/draft/orphan/reply/fractional-priority fixtures, all four scopes and all twelve types (288 comparisons). The native list implementation and National limit are preserved.

## Same-input comparison

31 recorded DYNASTY-EPUFT editions: preseason, 17 consecutive regular-season editions (0–16), all three bowl stages, the title stage, offseason 0–7 and 9. Offseason 8 is absent from the source and was not reconstructed. Six additional explicit synthetic fixtures cover WMU 31–26 over #14 Michigan, missing context, long names, recruit records, a title result and offseason draft reporting. These fixtures are labeled test inputs, never historical reporting.

The replay uses retained event facts. Missing old stat tables, roster membership and brand grades are not invented. It is a new-publication replay, not a claim to reproduce every original live menu visit. The fixture/input hashes and detailed outputs remain in ignored private Build files.

Near-duplicate metric: the normalized template structure is identical, or its word four-grams overlap by at least 70% of the larger set, against earlier cards in that view and four preceding editions. Placeholders replace names/numbers: changing teams, players or scores cannot create diversity. Views overlap and must not be summed into a single independent corpus.

| View | Displayed cards, each version | Near-duplicate cards before → after | Reduction | Adjacent near-duplicates before → after |
|---|---:|---:|---:|---:|
{table}

**Limits:** the broad National multi-edition metric improves modestly; it does not meet a 50% reduction. Adjacent repetition improves sharply. Required results, awards, signings and draft selections can exhaust factual alternatives even when dozens are eligible. The candidate preserves that news rather than dropping it or inventing angles. Across the 31 National windows, {new_templates:,} displayed cards use one of the 150 additions. All six supplemental fixtures have zero measured near-duplicates, but this small result is not a substitute for the recorded-year metric.

Generated posts stay exactly {total(before,'generated'):,} in both versions; primary stories stay {total(before,'roots'):,}. All 31 views retain identical displayed counts. Stored posts are {total(before,'kept'):,} → {total(after,'kept'):,}: two already-generated replies no longer collapse under exact-wording deduplication. There are no extra generation slots, replies-per-parent allowances or higher team/National caps. The 15 routine roots per team, recruit-statline exemption, important-news protections, all 224 draft slots, one edition per week and existing categories/scopes remain intact.

## Checks and factual safeguards

- No baseline protected event lost; primary award conservation checked in every edition. Identity/handle/role mismatches among compared displayed IDs: zero.
- Tests verify source-backed units, attribution, timing, sparse guards, long names, 280 Unicode-codepoint and 512 UTF-8-byte bounds, no unresolved placeholders, thread ancestry, relevant upset replies and differentiated celebration/frustration/analysis/rival positions. Overlong individual values still suppress an impossible variant rather than truncating a name or inventing facts.
- Existing account/style rules remain in force. General rewriting does not overwrite personality-tagged copy. Fans retain jokes and pettiness; restrained player alternatives are available without converting every account to the same vocabulary.
- Same-week published cards, their wording, identities and ancestry survive content upgrades. Tests cover stable refresh, deterministic fresh generation and Dynasty isolation. No save/profile/history migration or mutation is introduced.
- Recruit game records remain persistent, mod-generated records, separate from native data. Their native type label is **COT HS Game**; Social Media's setting description explains the source. Stored generated posts retain `recordSource`. No native rating, trait or record writes are added. There is no technical prefix forced into every body.
- Existing native container rendering is retained. The disabled experimental list/sidebar path stays disabled. Hook ownership, exact fingerprints, scope checks, sibling-build identity and helper verification remain on the existing paths.

In the personalized corpus, roles retain different lengths and cadences. Length alone does not prove personality; the private examples also show different claims and emotional positions, and persistent account character/analyst-interest tests still pass.

| Role | Cards | Mean words before → after | After range |
|---|---:|---:|---:|
{voice_table}

## Performance

Final warm ABBA comparison: two 31-edition runs per engine, identical input, author-input copying outside timing; no exhaustive audit callbacks. Capture, process startup, disk IO and live menu/FPS time are excluded. Before mean: {perf['beforeMeanMs']/1000:.3f} seconds/year; after: {perf['afterMeanMs']/1000:.3f} seconds/year ({perf['changePercent']:+.1f}%). Full generation includes more accurate visible-window refinement. This is a measured generation cost, not evidence of a live menu slowdown or speedup. Per-run median/p95/max values are in the private readable report.

An initial native settings test encountered Win32 access-denied during an isolated atomic-file commit. The unchanged test passed on direct rerun; final build/check results are recorded separately. No permission bypass, live settings edit or production retry was added.

## Live acceptance still required

Use copies of DYNASTY-EPUFT checkpoints, preserving their originals. Start with offseason signing/draft and busy regular-season editions; verify National scroll/category latency, compact numbering and complete 224-pick Transactions coverage. Check the WMU-style upset voices and parent replies. At the preseason checkpoint, check poll/honor wording and account identities. At a recruiting checkpoint, check COT HS Game labeling, Recruit Notes/colors and monetary edits. Reopen old editions and confirm old text; refresh a newly published edition twice and confirm stability. Measure actual menu entry/scroll timing and FPS separately. Exact AIO/TeamBuilder/game-build compatibility remains pending live acceptance.
"""
setup_path=ROOT/'Build/setup-verification.json'
update_path=ROOT/'Build/updater-verification.json'
if setup_path.exists() and update_path.exists():
    setup=json.loads(setup_path.read_text(encoding='utf-8'));update=json.loads(update_path.read_text(encoding='utf-8'))
    if setup.get('version')==update.get('version')=='0.1.11-private':
        assert setup['uiChecks']==34 and update['checks']==95 and update['crossVersion'] and not update['liveInstall']
        public+='''
## Completed candidate checks

- 425 Node tests pass, including editorial, persistence, filtering, budgets, reply context, dates, grades/currency and Analytics model checks.
- Native source build passes: 288 native/JavaScript scope/category parity comparisons, 974 Analytics integration assertions, and the existing native Social Media, Fog, currency, grade, settings and ABI checks.
- Package verification passes: 154 payload files, 983 original standalone source files verified unchanged, all 16 module selections, 17 actual host/resource/path checks, nine Analytics binary ABI/build-pair/license checks, and helper/tamper/traversal checks. Deliberate changes to imported COT copies are declared in the baseline ledger.
- 116 installer cases pass. The versioned single-file installer passes 34 real GUI/engine checks, including bounded layouts/scaling, module selection, uninstall and rollback.
- 95 updater checks pass for exact 0.1.10 → 0.1.11 discovery/download/install/rollback using isolated fixture endpoints. Corrupt payloads, failure recovery and retained data are covered. This is not a claim of a published GitHub release.
- 15 release metadata/provenance/privacy checks pass. Public-source boundary scan and source whitespace checks pass. Live installation and publishing remain false.
'''
(ROOT/'docs/verification-0.1.11.md').write_text(public,encoding='utf-8')
samples=[]
for index in [0,1,3,6,9,12,15,18,20,22,25,30]:
    old,new=before['editions'][index],after['editions'][index]
    prior={p['id']:p for p in old['views']['all']}
    pairs=[(prior[p['id']],p) for p in new['views']['all'] if p['id'] in prior and p['body']!=prior[p['id']]['body']]
    if pairs:
        a,b=pairs[0];samples.append(f"### {new['phase']} {new['week']} — {b['voice']} / {b['family']}\n\nBefore: {a['body']}\n\nAfter: {b['body']}\n")
wmu=supplemental['editions'][0]['views']['all']
samples.append('### WMU upset acceptance fixture\n\n'+'\n\n'.join(f"**{p['voice']} — {p['family']}:** {p['body']}" for p in wmu))
details='\n\n## Private before/after reading sample\n\nThese are generated outputs from identical retained inputs; their original publication archives are untouched.\n\n'+'\n'.join(samples)
details+='\n## Generation timing trials\n\n```json\n'+json.dumps(perf,indent=2)+'\n```\n'
details+='\n## Editorial decision counts\n\n```json\n'+json.dumps(reason_counts,indent=2)+'\n```\n'
(ROOT/'Release/editorial-report-0.1.11-private.md').write_text(public+details,encoding='utf-8')
verification={'format':1,'beforeRevision':read('baseline.json')['revision'],'inputSha256':before['inputSha256'],'recordedEditions':31,'supplementalEditions':6,'existingBodiesChanged':decisions['changedExisting'],'newVariants':150,'generatedBefore':total(before,'generated'),'generatedAfter':total(after,'generated'),'rootsBefore':total(before,'roots'),'rootsAfter':total(after,'roots'),'protectedEventsLost':lost,'displayedIdentityChanges':len(changed_identities),'before':before['summary'],'after':after['summary'],'performance':perf,'liveAcceptance':'pending','installedIntoLiveMmc':False,'published':False}
(work/'verification.json').write_text(json.dumps(verification,indent=2),encoding='utf-8')
print('Prepared readable aggregate verification and private before/after sample.')

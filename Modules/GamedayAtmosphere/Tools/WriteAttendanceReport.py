"""Render the offline C++ harness outputs; standard library only."""
import csv
import html
import statistics
import sys
from collections import defaultdict
from pathlib import Path

root = Path(sys.argv[1])
version = sys.argv[2] if len(sys.argv)>2 else '1.91'
def read(name):
    with (root / name).open(encoding="utf-8-sig", newline="") as f:
        return list(csv.DictReader(f))
curves = read("game-curves.csv")
summary = read("scenario-summary.csv")
sensitivity = read("sensitivity.csv")
cohort = read("cohort-peak-sweep.csv") if (root / "cohort-peak-sweep.csv").exists() else []
groups = defaultdict(list)
for row in curves:
    groups[row["scenario"], row["seats"]].append(row)
def number(row, field):
    return float(row[field])
def clock(t):
    t = int(float(t))
    if t >= 2880:
        return "Final"
    q, used = divmod(t, 720)
    left = 720 - used
    return f"Q{q+1} {left//60}:{left%60:02}"
def table(headers, rows):
    return '<div class="table"><table><thead><tr>' + ''.join(f'<th>{html.escape(str(x))}</th>' for x in headers) + '</tr></thead><tbody>' + ''.join('<tr>'+''.join(f'<td>{html.escape(str(x))}</td>' for x in row)+'</tr>' for row in rows) + '</tbody></table></div>'
def bands(name):
    svg = ['<svg viewBox="0 0 1100 235" role="img" aria-label="Camera qualification windows for three venue sizes">']
    for i, seats in enumerate(("6060", "30000", "61888")):
        for j, field in enumerate(("ordinary_down_camera", "critical_down_camera")):
            y = 25 + (2*i+j)*30
            label = f'{int(seats):,} — {"ordinary" if j == 0 else "3rd & 10"}'
            svg.append(f'<text x="0" y="{y+12}" font-size="14">{label}</text><path d="M200 {y+9} H1080" stroke="#ddd"/>')
            for p in groups[name, seats]:
                t = number(p, "elapsed_reference_seconds")
                if p[field] == "1" and t < 2880:
                    x = 200 + 880*t/2880
                    svg.append(f'<rect x="{x:.3f}" y="{y}" width="{880*30/2880+.1:.3f}" height="18" fill="{"#19835e" if j == 0 else "#2574a9"}"/>')
    for t in (0, 720, 1440, 2160, 2880):
        x = 200 + 880*t/2880
        anchor = 'end' if t == 2880 else 'start'
        svg.append(f'<path d="M{x} 22 V205" stroke="#bbb" stroke-dasharray="2 3"/><text x="{x}" y="225" text-anchor="{anchor}" font-size="14">{clock(t)}</text>')
    return ''.join(svg)+'</svg>'
large = [r for r in summary if r["seats"] == "61888"]
rows = []
for r in large:
    p = groups[r['scenario'], '61888']
    half = next(x for x in p if x['elapsed_reference_seconds'] == '1440')
    rows.append([r['scenario'],f"{number(r,'peak_capacity_pct'):.1f}%",f"{number(p[0],'fullness_pct'):.1f}%",f"{number(half,'fullness_pct'):.1f}%",f"{number(p[-1],'fullness_pct'):.1f}%",f"{number(r,'final_departures_pct_of_peak'):.0f}%",f"{p[-1]['home']}–{p[-1]['away']}"])
spreads = defaultdict(list)
for r in sensitivity:
    spreads[r['scenario'], r['seats'], r['quarter_length']].append(number(r,'final_capacity_pct'))
max_window_spread = max(max(v)-min(v) for v in spreads.values())
length_spreads = defaultdict(list)
for r in sensitivity:
    length_spreads[r['scenario'], r['seats'], r['hidden_window_reference_seconds']].append(number(r,'final_capacity_pct'))
max_length_spread = max(max(v)-min(v) for v in length_spreads.values())
distribution=[]
for label, night, rain, rival in [('Day ordinary','0','0','0'),('Night ordinary','1','0','0'),('Rain ordinary','0','1','0'),('Day rivalry','0','0','1')]:
    values=sorted(number(x,'peak_pct') for x in cohort if x['night']==night and x['rain']==rain and x['rivalry']==rival)
    if values:
        distribution.append([label,len(values),f'{values[0]:.1f}%',f'{values[len(values)//10]:.1f}%',f'{statistics.median(values):.1f}%',f'{values[len(values)*9//10]:.1f}%',f'{100*sum(x>=99.99 for x in values)/len(values):.1f}%'])
curve=[]
for n in (6060,10000,20000,30000,40000,51424,56099,61888):
    factor=.35+.75*min(1,max(0,(n-6060)/(61888-6060)))
    curve.append([f'{n:,}',f'{factor:.3f}×',f'{15*factor:.2f}%',f'{15*factor*.75:.2f}%'])
parts=['''<!doctype html><html lang="en"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>DGA 1.91 attendance and atmosphere review</title>
<style>body{font:16px/1.55 system-ui,sans-serif;color:#17232e;background:#fff;max-width:1500px;margin:28px auto;padding:0 24px}h1,h2,h3{line-height:1.2}h2{margin-top:2em}a{color:#145c92}img,svg{width:100%;height:auto}svg text{fill:#17232e}table{border-collapse:collapse;width:100%;font-variant-numeric:tabular-nums}th,td{padding:9px 12px;border-bottom:1px solid #ccd4db;text-align:left}th{background:#edf2f5}.table{overflow-x:auto}details{border-bottom:1px solid #ccd4db;padding:14px 0}summary{cursor:pointer;font-weight:650}small{color:#485866}.note{border-left:4px solid #2574a9;padding-left:16px}@media print{details{break-inside:avoid}.table{overflow:visible}h2{break-before:auto}body{padding:0;font-size:11px}}</style>
<h1>DGA 1.91 — attendance and atmosphere review</h1>
<p>Offline production-policy simulations, 29 October 2026. The reviewed candidate is staged separately; these are synthetic games, not live measurements.</p>
<p class="note"><b>Read fullness as percentage of total capacity.</b> Kickoff share and departures use the planned peak crowd as their denominator. For example, a 25% peak × 50% kickoff share means a visibly 12.5% full stadium. Shake qualification uses that visible 12.5%, never the 50% share of peak.</p>
<h2>Full-game results</h2><p>The table uses the 61,888-position layout. Curves below compare all three authored seating sizes. Halftime is the first hidden update at the Q3 boundary, including temporary absences; those fans can return. Permanent departures do not return.</p>''']
parts.append(table(['Synthetic profile','Planned peak','Kickoff fullness','Halftime seated','Final fullness','Final departures / peak','Final score'],rows))
close_loss=next(r['final_departures_pct_of_peak'] for r in large if r['scenario']=='Middle support close')
parts.append(f'''<p>Close ordinary games finish {close_loss}% below their own peak in this score path; elite rivalry games retain more fans. A rainy 35-point home win can finish at about 23% visible capacity for middle support, while a ranked home winner keeps its existing 35%-of-peak floor. Review the paired early/late-rout fixtures before further blowout tuning.</p><h2>Visible crowd curves</h2><p>Solid lines: actual simulated seated crowd after safe-window pacing, arrivals, temporary halftime absences and permanent exits. The dashed line is the absolute 50% minimum for added crowd shake; individual routes can require more. Nearly overlapping lines are expected: venue size primarily changes qualification and shake strength, with a separate packed-house halftime patience effect.</p><img src="fullness-1.png" alt="Fullness curves for ten ordinary, elite, late-arrival and rainy home-win profiles"><img src="fullness-2.png" alt="Fullness curves for blowouts, comeback, upset, neutral and postseason profiles">''')
if len(large)>20:parts.append('<img src="fullness-3.png" alt="Matched early and late rout fullness curves">')
parts.append('''<h2>When camera atmosphere qualifies</h2><p>Expand a profile. Green marks show eligibility on an ordinary offensive down; blue marks show eligibility on a close third-and-10. These are standardized opportunities at the measured fullness, not predicted shake events. Each bucket is 30 reference game seconds; actual qualification can change within a bucket. Opening possession is assumed to last the first 2:30. No event or cheer is invented by this probe.</p>''')
for r in large:
    name=r['scenario']
    parts.append(f'<details><summary>{html.escape(name)}</summary>{bands(name)}</details>')
parts.append('<h2>Venue strength curve</h2><p>All added crowd-motion families receive the factor once, before their attack, handoff and release envelopes. Fullness admission is unchanged. Neutral crowd motion retains a further 0.75× factor for either team. Native cannon impact and EA camera motion have their separate existing policies. Counts below/above the endpoints clamp to 0.35×/1.10×.</p>')
parts.append(table(['Authored seating positions','Venue multiplier','Configured 15% target','Same target at neutral site'],curve))
parts.append('<h2>Formula cohort sweep</h2><p>All 18,906 ordered pairings of the captured 138-school input cohort, repeated for four launch conditions. This is a stress-test distribution, not a real schedule or a fresh census. Output omits school identities. Starting attendance is still a share of the revised peak.</p>')
parts.append(table(['Launch condition','Cases','Minimum peak','10th percentile','Median peak','90th percentile','At 100% peak'],distribution))
parts.append(f'''<h2>Coverage and sensitivity</h2><p>{len(summary)} detailed traces: {len(large)} score/demand profiles × three venue sizes. The parameter sweep checks {len(sensitivity)} full-game combinations across 1, 5, 12 and 20-minute quarters and hidden windows every 10, 30 or 120 reference seconds. Maximum final-fullness spread from quarter length at a fixed reference cadence: {max_length_spread:.2f} capacity points. Maximum spread across hidden-window cadence: {max_window_spread:.2f} capacity points. Safe-window timing and halftime conversion can legitimately change visible settlement.</p>
<p>Assertions checked finite bounded fullness, monotonic permanent departures, applicable crowd floors, preserved native-peak bounds, no shake below absolute 50% fullness and deterministic repeated results. Existing unit/integration suites additionally cover resume/history, Super Sim/replay restoration, ownership, settings migration and native adapters.</p>
<h2>What these simulations do not establish</h2><ul><li>Rendered/native seating masks can differ from the peak/capacity ratio approximation used here. Actual section layouts and streaming need live checks.</li><li>Generic profiles specify support percentiles directly; they do not represent a particular team. The cohort sweep separately computes standings from captured grades/ranks.</li><li>Postseason examples explicitly assume EA supplied a full peak crowd. DGA still leaves their native peak unchanged.</li><li>No weather changes, injuries, penalties, turnovers, replay interruptions or late-score exit events are fabricated. The score paths exercise continuous policies and halftime conversion; dedicated tests cover those event routes.</li><li>Qualification probes use production admission helpers with ordinary down / third-and-10 situations; they do not predict play calls, audible sound, rendered shake or replays.</li><li>Night currently raises peak/opening attendance. A separate night-only departure adjustment or blanket 10% threshold reduction is not active and has not been silently added during this audit.</li></ul>
<h2>Files and reproduction</h2><ul><li><a href="game-curves.csv">Every detailed fullness / qualification point</a></li><li><a href="fixture-inputs.csv">Fixture inputs: support, rankings, weather, night, rivalry and mode</a></li><li><a href="scenario-summary.csv">Scenario endpoints</a></li><li><a href="sensitivity.csv">All 684 length / hidden-window results</a></li><li><a href="cohort-peak-sweep.csv">75,624 launch-formula cases</a></li><li><a href="checks.txt">Harness coverage</a></li><li><a href="audit.md">Code review findings and live-test checklist</a></li></ul>
<p>Reproduce using Tools/SimulateAttendance.cpp (built by Source/Build.ps1 -Automatic), then Tools/RenderAttendanceReport.ps1 and Tools/WriteAttendanceReport.py. The private captured cohort is optional; it is not included in the plugin.</p></html>''')
report=''.join(parts).replace('DGA 1.91','DGA '+version).replace('All 684 length / hidden-window results',f'All {len(sensitivity)} length / hidden-window results')
if (root/'follow-up.html').exists():report=report.replace('</html>',(root/'follow-up.html').read_text(encoding='utf8')+'</html>')
(root/'report.html').write_text(report,encoding='utf-8')
print(f'Wrote report for {len(large)} profiles and {len(curves)} detailed points.')

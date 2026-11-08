"""Prepare private, fact-only renderer replays. Never reads/writes live MMC.

Usage: python prepare_recorded.py <frozen RuntimeHistory> <output.json> <pack dir>
Unavailable historical fields stay absent; this is not a recovered raw save.
"""
import argparse, collections, hashlib, json, pathlib

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('frozen_history',type=pathlib.Path);parser.add_argument('output',type=pathlib.Path);parser.add_argument('packs',type=pathlib.Path)
parser.add_argument('--scope',action='append',required=True,help='Frozen league/user/year directory; repeat for multiple programs')
args=parser.parse_args();root,output,packs=args.frozen_history,args.output,args.packs
ids = {}
for p in sorted(packs.glob('*.json')):
    for family, lines in json.loads(p.read_text(encoding='utf-8')).get('families', {}).items():
        for t in lines:
            ids[t['id']] = family
all_editions, observed, unmapped, manifest = [], collections.Counter(), collections.Counter(), {}
phases = ['PreSeason', 'RegularSeason', 'BowlSeason1', 'BowlSeason2', 'BowlSeason3', 'NationalChampionship', 'OffSeason']
# Two controlled programs; every available consecutive publication in 2026.
for scope in args.scope:
    parts=scope.split('/')
    if len(parts)!=3 or any(str(int(x))!=x for x in parts): raise ValueError('Expected numeric league/user/year scope')
    league,user,year=parts
    rows = []
    for phase in phases:
        p = root / league / user / year / phase / 'history.index.json'
        if not p.exists():
            continue
        index = json.loads(p.read_text(encoding='utf-8'))
        for meta in sorted(index['editions'], key=lambda x: x['week']):
            fp = p.parent / '.history-data' / meta['feedFile']
            raw = fp.read_bytes(); manifest[str(fp.relative_to(root))] = hashlib.sha256(raw).hexdigest()
            feed = json.loads(raw)
            snap = json.loads((p.parent / '.history-data' / meta['snapshotFile']).read_text(encoding='utf-8')) if meta.get('snapshotFile') else {}
            teams, players, games = {}, {}, {}
            for event in feed['events']:
                f = event['facts']
                for c in event.get('teamContext', []):
                    teams[c['teamId']] = {**teams.get(c['teamId'], {}), **c, 'id': c['teamId'], 'name': c['school'], 'logo': c['teamId']}
                pid = event.get('playerId')
                if pid and f.get('playerName'):
                    players[pid] = {**players.get(pid, {}), 'id': pid, 'name': f['playerName'], 'team': event['teamIds'][0] if event['teamIds'] else 255,
                        'position': event.get('position') or f.get('position'), 'personality': event.get('personality'), 'injury': 'Uninjured', 'starRating': f.get('stars')}
                if event['category'] == 'results' and f.get('gameId') and 'homeScore' in f:
                    byname = {t['name']: t['id'] for t in teams.values()}
                    games[f['gameId']] = {'id': f['gameId'], 'home': byname.get(f['home'], 255), 'away': byname.get(f['away'], 255), 'homeName': f['home'], 'awayName': f['away'],
                        'homeScore': f['homeScore'], 'awayScore': f['awayScore'], 'status': 'Tie' if f['homeScore']==f['awayScore'] else 'HomeWon' if f['homeScore']>f['awayScore'] else 'AwayWon',
                        'published': True, 'year': 0, 'phase': phase, 'week': event.get('eventWeek', feed['week']-1)}
            for key, a in index.get('authors', {}).items():
                if ':person:' not in key or ':retired:' in key: continue
                pid = key.split(':person:', 1)[1]
                if pid in players: continue
                # Only names/identities are available; never invent roster membership.
                players[pid] = {'id': pid, 'name': a.get('personIdentity', {}).get('name') or a['author'].split(' | ')[0], 'team': 255, 'injury': 'Uninjured', 'position': None}
            ranks = snap.get('publicPoll', {}).get('ranks', {})
            for t in teams.values():
                t['mediaRank'] = ranks.get(str(t['id']))
            # Logos, ranks and historical brand data are not interchangeable.
            # Missing brand remains the runtime's neutral default.
            s = {'format': 2, 'leagueId': feed['leagueId'], 'userTeamIndex': feed['userTeamIndex'], 'sourceKind': 'runtime', 'sha256': feed['sha256'], 'source': 'private-fact-replay',
                'season': {'CurrentSeasonYear': feed['season'], 'CurrentYear': 0, 'CurrentWeekType': phase, 'CurrentWeek': feed['week'], 'CurrentStage': 'NFLSeason'},
                'teams': list(teams.values()), 'players': list(players.values()), 'games': list(games.values()), 'performances': [], 'heisman': [], 'offers': [], 'commitments': [], 'recruitStatuses': []}
            # Prefer explicit metadata, then ALL archived source pack IDs (not only the final merged pack).
            for story in feed['posts']:
                fam = story.get('templateFamily') or ids.get(story.get('templateId'))
                if fam: observed[fam] += 1
                else: unmapped[story.get('templateId', '?')] += 1
            rows.append({'snapshot': s, 'events': feed['events'], 'publishedPosts': feed['posts'], 'authors': index.get('authors', {}), 'phase': phase, 'week': feed['week']})
    all_editions.extend(rows)
data = {'method': 'Recorded event-fact replay; missing raw historical stats, brand grades and roster membership are not reconstructed.',
    'observedFamilies': observed.most_common(), 'unmappedTemplates': unmapped.most_common(), 'manifest': manifest, 'editions': all_editions}
output.parent.mkdir(parents=True, exist_ok=True); output.write_text(json.dumps(data, ensure_ascii=False), encoding='utf-8')
print(json.dumps({'editions': len(all_editions), 'observedTop20': observed.most_common(20), 'unmapped': sum(unmapped.values())}))

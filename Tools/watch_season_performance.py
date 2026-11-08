"""Opt-in bounded season observer: query-only counters and existing log tails.

No hooks, game calls, setting edits, thread suspension or process-memory writes.
Stop by creating OUTPUT/stop, closing the game, or reaching --seconds.
"""
import argparse
import ctypes as c
from ctypes import wintypes as w
import datetime
import importlib.util
import json
from pathlib import Path
import subprocess
import time

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('cot_process_observer', ROOT / 'Modules/DynastyEnhancer/Tools/observe_runtime_performance.py')
observer = importlib.util.module_from_spec(spec)
spec.loader.exec_module(observer)


class ProcessEntry(c.Structure):
    _fields_ = [('size', w.DWORD), ('usage', w.DWORD), ('pid', w.DWORD),
                ('heap', c.c_size_t), ('module', w.DWORD), ('threads', w.DWORD),
                ('parent', w.DWORD), ('priority', w.LONG), ('flags', w.DWORD),
                ('exe', w.WCHAR * 260)]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--pid', type=int, required=True)
    p.add_argument('--mmc-pid', type=int, required=True)
    p.add_argument('--runtime', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--seconds', type=int, default=14400)
    p.add_argument('--presentmon', type=Path)
    a = p.parse_args()
    out = a.output.resolve()
    if not out.is_relative_to((ROOT / 'Build').resolve()) or not 1 <= a.seconds <= 14400:
        raise ValueError('Use a bounded private workspace capture')
    out.mkdir(parents=True, exist_ok=True)
    k = c.WinDLL('kernel32', use_last_error=True)
    psapi = c.WinDLL('psapi', use_last_error=True)
    k.OpenProcess.argtypes = [w.DWORD, w.BOOL, w.DWORD]; k.OpenProcess.restype = w.HANDLE
    k.CloseHandle.argtypes = [w.HANDLE]
    k.GetProcessTimes.argtypes = [w.HANDLE] + [c.POINTER(w.FILETIME)] * 4
    k.GetExitCodeProcess.argtypes = [w.HANDLE, c.POINTER(w.DWORD)]
    k.GetProcessIoCounters.argtypes = [w.HANDLE, c.POINTER(observer.Io)]
    psapi.GetProcessMemoryInfo.argtypes = [w.HANDLE, c.POINTER(observer.Memory), w.DWORD]
    k.CreateToolhelp32Snapshot.argtypes = [w.DWORD, w.DWORD]; k.CreateToolhelp32Snapshot.restype = w.HANDLE
    for fn in ('Process32FirstW', 'Process32NextW'):
        getattr(k, fn).argtypes = [w.HANDLE, c.POINTER(ProcessEntry)]
    k.GetTickCount64.restype = c.c_uint64
    utc = lambda: datetime.datetime.now(datetime.timezone.utc).isoformat()
    handles, prior, maxima, positions = {}, {}, {}, {}
    logs = ['social-media.jsonl', 'recruiting-fog.jsonl', 'runtime-generator.log']
    for name in logs:
        file = a.runtime / name
        positions[name] = file.stat().st_size if file.exists() else 0
    meta = dict(gamePid=a.pid, mmcPid=a.mmc_pid, startUtc=utc(), startTick=k.GetTickCount64(),
                startQpcMs=time.perf_counter()*1000, secondsLimit=a.seconds, intervalSeconds=2,
                readOnly=True, runtime=str(a.runtime), logStartOffsets=positions.copy(),
                limitations=['Process counters do not attribute render-thread cost.',
                             'Fog counters cap each path at 8192 samples and exclude AS2 body execution.',
                             'Native preparation times do not measure a fully rendered menu.',
                             'Feature attribution requires matched same-save On/Off comparisons.'])
    pm, pm_log = None, None
    if a.presentmon:
        session = f'COTSeason-{a.pid}-{k.GetTickCount64()}'
        pm_log = (out / 'presentmon.log').open('w', encoding='utf-8')
        command = [str(a.presentmon.resolve()), '--process_id', str(a.pid), '--output_file', str(out/'frames.csv'),
                   '--timed', str(a.seconds), '--terminate_after_timed', '--session_name', session,
                   '--no_console_stats', '--no_track_gpu', '--no_track_input', '--no_track_display',
                   '--v1_metrics', '--qpc_time_ms', '--set_circular_buffer_size', '16384']
        pm = subprocess.Popen(command, stdout=pm_log, stderr=subprocess.STDOUT, creationflags=subprocess.CREATE_NO_WINDOW)
        meta['presentmon'] = dict(pid=pm.pid, session=session, command=command,
                                 metric='CPU present intervals, not GPU time or verified display intervals',
                                 status='best effort; reject missing/invalid or event-loss-affected data')
    (out / 'observer-session.json').write_text(json.dumps(meta, indent=2))
    (out / 'observer-ready.json').write_text(json.dumps(dict(pid=a.pid, atUtc=utc(), readOnly=True)))
    began, reason = time.monotonic(), 'time limit'
    try:
        with (out / 'processes.jsonl').open('a', encoding='utf-8', buffering=1) as samples, (out / 'events.jsonl').open('a', encoding='utf-8', buffering=1) as events:
            while time.monotonic() - began < a.seconds:
                if (out / 'stop').exists(): reason = 'stop marker'; break
                snapshot = k.CreateToolhelp32Snapshot(2, 0)
                table = {}
                if snapshot != c.c_void_p(-1).value:
                    try:
                        e = ProcessEntry(size=c.sizeof(ProcessEntry)); ok = k.Process32FirstW(snapshot, c.byref(e))
                        while ok:
                            table[e.pid] = (e.parent, e.exe); ok = k.Process32NextW(snapshot, c.byref(e))
                    finally: k.CloseHandle(snapshot)
                if table and a.pid not in table: reason = 'game exited'; break
                wanted = {a.pid: 'game', a.mmc_pid: 'MMC'}
                children = {a.pid}
                for _ in range(3): children |= {pid for pid, (parent, _) in table.items() if parent in children}
                wanted.update({pid: 'generator' for pid in children if table.get(pid, (0,''))[1].lower() == 'node.exe'})
                now = time.monotonic()
                for pid, role in wanted.items():
                    if pid not in handles:
                        h = k.OpenProcess(0x410, False, pid)
                        if h: handles[pid] = h
                    h = handles.get(pid)
                    if not h: continue
                    exit_code = w.DWORD()
                    if not k.GetExitCodeProcess(h, c.byref(exit_code)) or exit_code.value != 259: continue
                    times = [w.FILETIME() for _ in range(4)]
                    if not k.GetProcessTimes(h, *(c.byref(t) for t in times)): continue
                    total = sum((t.dwHighDateTime << 32) | t.dwLowDateTime for t in times[2:]) / 1e7
                    mem = observer.Memory(size=c.sizeof(observer.Memory)); io = observer.Io()
                    psapi.GetProcessMemoryInfo(h, c.byref(mem), c.sizeof(mem)); k.GetProcessIoCounters(h, c.byref(io))
                    before, at = prior.get(pid, (total, now)); prior[pid] = total, now
                    row = dict(utc=utc(), tick=k.GetTickCount64(), pid=pid, role=role,
                               elapsed=max(0, now-at), cpuSeconds=max(0, total-before),
                               privateBytes=mem.private_bytes, workingSet=mem.working_set,
                               io={name:getattr(io,name) for name,_ in observer.Io._fields_})
                    samples.write(json.dumps(row)+'\n')
                    maxima[pid] = max(maxima.get(pid,0), mem.private_bytes)
                for name in logs:
                    file = a.runtime / name
                    if not file.exists(): continue
                    if file.stat().st_size < positions[name]: positions[name] = 0
                    with file.open('rb') as f:
                        f.seek(positions[name]); data = f.read(4*1024*1024)
                        complete = data.rfind(b'\n') + 1
                        positions[name] += complete
                    for line in data[:complete].splitlines():
                        try: item = json.loads(line)
                        except (ValueError, UnicodeError): continue
                        fields = {key:value for key,value in item.items() if isinstance(value,(str,int,float,bool)) or key in ('rows','nativeRowsIncludingCollector','scriptAdapters','columns','retention')}
                        events.write(json.dumps(dict(observedUtc=utc(), source=name, data=fields))+'\n')
                time.sleep(2)
    finally:
        for h in handles.values(): k.CloseHandle(h)
        if pm is not None:
            if pm.poll() is None:
                subprocess.run([str(a.presentmon.resolve()), '--session_name', session, '--terminate_existing_session'],
                               stdout=pm_log, stderr=subprocess.STDOUT, timeout=15, creationflags=subprocess.CREATE_NO_WINDOW)
                try: pm.wait(timeout=10)
                except subprocess.TimeoutExpired: pm.terminate(); pm.wait(timeout=10)
            pm_log.close()
        (out / 'observer-stopped.json').write_text(json.dumps(dict(atUtc=utc(), reason=reason, maximumPrivateBytes=maxima), indent=2))


if __name__ == '__main__': main()

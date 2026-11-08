import fs from 'node:fs';
import {supplementalEditions} from './fixtures.mjs';
fs.writeFileSync(process.argv[2],JSON.stringify({format:1,method:'Explicit synthetic acceptance cases, not historical game claims.',editions:supplementalEditions()}));

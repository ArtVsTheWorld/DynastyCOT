#pragma once
// Position-model sparse-cohort priors; no ratings or development traits.
namespace analytics {inline Json calibration(){return Json::parse(R"COTMODEL({
  "modelVersion": "0.2.0-production",
  "calibrationSource": "Released native game-stat production; sparse-cohort fallback only. Full cohorts use position/year/edition percentiles.",
  "calibrationSourceSha256": "ff8e1de00aec66ae15b51c9b5666098f91aa178a593ae6b42b18888d58789c1e",
  "variation": 0,
  "maxStandardScore": 4.0,
  "baselines": {
    "CB": {
      "mean": 0.13154164395137596,
      "sd": 0.12228244142504982,
      "samples": 5281,
      "shrinkSnaps": 24.0,
      "centerCorrection": 0.0
    },
    "DL": {
      "mean": 0.18209375029744315,
      "sd": 0.1720576323742993,
      "samples": 7987,
      "shrinkSnaps": 24.0,
      "centerCorrection": 0.0
    },
    "K": {
      "mean": 2.2829756517991813,
      "sd": 2.9274602389658777,
      "samples": 1105,
      "shrinkSnaps": 3.0,
      "centerCorrection": 0.0
    },
    "LB": {
      "mean": 0.17829352536990603,
      "sd": 0.16382627361792254,
      "samples": 7261,
      "shrinkSnaps": 24.0,
      "centerCorrection": 0.0
    },
    "OL": {
      "mean": -0.627966522058078,
      "sd": 0.4647364195192191,
      "samples": 8784,
      "shrinkSnaps": 24.0,
      "centerCorrection": 0.0
    },
    "P": {
      "mean": 0.6179954697325143,
      "sd": 1.748708724259322,
      "samples": 1563,
      "shrinkSnaps": 3.0,
      "centerCorrection": 0.0
    },
    "QB": {
      "mean": 0.30904170096137634,
      "sd": 0.4887690703812005,
      "samples": 1724,
      "shrinkSnaps": 24.0,
      "centerCorrection": 0.0
    },
    "RB": {
      "mean": 1.017120625434836,
      "sd": 0.8961952178887554,
      "samples": 3384,
      "shrinkSnaps": 24.0,
      "centerCorrection": 0.0
    },
    "S": {
      "mean": 0.1512446019456961,
      "sd": 0.11533201565335474,
      "samples": 5332,
      "shrinkSnaps": 24.0,
      "centerCorrection": 0.0
    },
    "TE": {
      "mean": 0.33082107528114885,
      "sd": 0.3188863986668822,
      "samples": 2786,
      "shrinkSnaps": 24.0,
      "centerCorrection": 0.0
    },
    "WR": {
      "mean": 0.6683006188196828,
      "sd": 0.48196756443627464,
      "samples": 5616,
      "shrinkSnaps": 24.0,
      "centerCorrection": 0.0
    }
  }
}
)COTMODEL");}}

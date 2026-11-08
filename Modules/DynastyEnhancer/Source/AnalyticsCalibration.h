#pragma once
// Frozen standalone production model; grading never consumes ratings or development traits.
namespace analytics {inline Json calibration(){return Json::parse(R"COTMODEL({
  "baselines": {
    "CB": {
      "centerCorrection": -0.005563638010523785,
      "mean": 0.13908072295119617,
      "samples": 4271,
      "sd": 0.1337508050070115,
      "shrinkSnaps": 24.0
    },
    "DL": {
      "centerCorrection": -0.009764804416598395,
      "mean": 0.15209396204908152,
      "samples": 7975,
      "sd": 0.22274378597647015,
      "shrinkSnaps": 24.0
    },
    "K": {
      "centerCorrection": -0.0017254843522001443,
      "mean": 1.0383943940326705,
      "samples": 1009,
      "sd": 1.1446166899964032,
      "shrinkSnaps": 3.0
    },
    "LB": {
      "centerCorrection": -0.004805678948869662,
      "mean": 0.17786248589383294,
      "samples": 6732,
      "sd": 0.18666894387500696,
      "shrinkSnaps": 24.0
    },
    "OL": {
      "centerCorrection": 0.0064929295059912625,
      "mean": -0.05665277933130746,
      "samples": 7560,
      "sd": 0.13880562686752387,
      "shrinkSnaps": 24.0
    },
    "P": {
      "centerCorrection": -0.00028291171406194006,
      "mean": 0.9964809013634115,
      "samples": 1004,
      "sd": 0.7547213605518748,
      "shrinkSnaps": 3.0
    },
    "QB": {
      "centerCorrection": -0.00034942639527811055,
      "mean": 0.4865926033753265,
      "samples": 1480,
      "sd": 0.5118733383945265,
      "shrinkSnaps": 24.0
    },
    "RB": {
      "centerCorrection": -0.0044416626169544665,
      "mean": 0.8820939310586436,
      "samples": 3602,
      "sd": 1.0388622518651756,
      "shrinkSnaps": 24.0
    },
    "S": {
      "centerCorrection": -0.004063227975085452,
      "mean": 0.14172566059596148,
      "samples": 5128,
      "sd": 0.11908956042260961,
      "shrinkSnaps": 24.0
    },
    "TE": {
      "centerCorrection": -0.0014997980687982096,
      "mean": 0.23005319250641926,
      "samples": 4302,
      "sd": 0.3273485520236076,
      "shrinkSnaps": 24.0
    },
    "WR": {
      "centerCorrection": -0.004234065347277099,
      "mean": 0.6686039277351953,
      "samples": 5750,
      "sd": 0.6223897029308025,
      "shrinkSnaps": 24.0
    }
  },
  "calibrationLeagueId": 1584917829,
  "calibrationSourceSha256": "96be3534f9e02aaef157845a320a1625e0325b03c2828450915a22724383d4bf",
  "maxStandardScore": 4.0,
  "modelVersion": "0.1.0-research",
  "variation": 2.0
})COTMODEL");}}

#pragma once
namespace attendance::matchup {
inline bool top(int rank,int cutoff){return rank>=1&&rank<=cutoff;}
// Shared identity only. Each effect still checks its own crowd and game context.
inline bool big(int home,int away,bool rivalry,bool playoff=false,bool conference=false){
    return playoff||conference||(top(home,25)&&top(away,25))||
        (rivalry&&(top(home,25)||top(away,25)||(top(home,50)&&top(away,50))));
}
}

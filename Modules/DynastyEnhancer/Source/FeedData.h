#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <array>
#include <algorithm>
#include <string_view>
#include "FeedRefresh.h"
namespace social {
struct FeedPost {std::string author,handle,body,stamp;int imageLibrary=79,imageValue=598;std::string category,voice,replyTo;bool transaction=false;int likes=-1,uniformTeamLogo=-1,draftPick=-1,draftRound=0;int bodyHeight=0;double priority=0;bool important=false;std::string displayAuthor,displayLikes,displayStamp;FeedPost(std::string a,std::string h,std::string b,std::string s,int l=79,int v=598):author(std::move(a)),handle(std::move(h)),body(std::move(b)),stamp(std::move(s)),imageLibrary(l),imageValue(v){displayAuthor=author+" ("+handle+")";};};
inline std::string compactCount(int value){
    if(value<1000)return std::to_string(value);
    const int unit=value>=999950?1000000:1000;const auto tenths=(static_cast<long long>(value)*10+unit/2)/unit;
    auto out=std::to_string(tenths/10);if(tenths%10)out+="."+std::to_string(tenths%10);return out+(unit==1000000?"M":"K");
}
inline std::string storyLabel(const nlohmann::json& post){
    if(post.contains("replyTo"))return "Reply";
    const auto category=post.value("category",std::string());
    if(category=="draft-discourse")return "Draft";
    if(category=="draft")return post.contains("draftRound")?"Round "+std::to_string(post.at("draftRound").get<int>())+" - Pick "+std::to_string(post.at("draftPick").get<int>()):"Draft";
    if(category=="offseason")return "Offseason";
    if(category=="upsets")return "Upset";
    if(category=="conference-talk")return "Conference Talk";
    if(category=="transactions")return "Transaction";
    if(category=="signings")return "Signing";
    if(category=="coaching")return "Coaching";
    if(category=="postseason")return "Postseason";
    if(category=="season-end")return "Season Review";
    if(category=="matchup-preview")return "Matchup";
    if(category=="matchup-recap")return "Matchup Recap";
    if(category=="history")return "Milestone";
    if(category=="championship")return "Title Race";
    if(category=="bowls")return "Bowl Watch";
    if(category=="records")return "Record Book";
    if(category=="streaks")return "Streak";
    if(category=="analytics")return "Analysis";
    if(category=="preseason")return "Preseason";
    if(category=="results"||category=="team-stats"||category=="player-stats")return "Recap";
    if(category=="recruiting"||category=="offers")return "Recruiting";
    if(category=="schedule")return "Upcoming";
    if(category=="polls")return "Rankings";
    if(category=="heisman")return "Heisman";
    if(category=="cfp")return "Playoff Watch";
    if(category=="injuries")return "Injury Report";
    if(category=="standings")return "Standings";
    if(category=="watch")return "Spotlight";
    if(category=="lines")return "Odds";
    if(category=="holiday")return "Calendar";
    if(category=="awards")return "Awards";
    if(category=="nil")return "NIL";
    if(category=="recruit-film")return "COT HS Game";
    if(category=="transfer")return "Transfers";
    if(category=="depth-chart")return "Roster";
    return "News";
}
// The native multiline body now spans the available 1065 layout pixels.
// Conservative glyph widths and 36-pixel lines leave breathing room without
// truncating the text or reducing its font size. Long words/newlines count too.
inline int postHeight(std::string_view body){
    constexpr int width=1065;int lines=1,line=0,word=0;
    auto flush=[&](){if(!word)return;if(line&&line+8+word>width){++lines;line=0;}if(line)line+=8;while(word>width){++lines;word-=width;}line+=word;word=0;};
    for(size_t i=0;i<body.size();){
        auto ch=static_cast<unsigned char>(body[i++]);
        if(ch==' '||ch=='\t'||ch=='\n'||ch=='\r'){flush();if(ch=='\n'){++lines;line=0;}continue;}
        if(ch>=128){while(i<body.size()&&(static_cast<unsigned char>(body[i])&0xc0)==0x80)++i;word+=32;continue;}
        if(ch=='W'||ch=='M')word+=28;
        else if(ch=='w'||ch=='m'||ch=='@')word+=24;
        else if(ch=='i'||ch=='l'||ch=='I'||ch=='.'||ch==','||ch=='!'||ch=='\''||ch==':'||ch==';')word+=8;
        else if(ch>='A'&&ch<='Z')word+=20;
        else word+=16;
    }
    flush();return (std::max)(160,((68+lines*36+15)/16)*16);
}
inline constexpr std::array<const char*,4> scopeKeys={"all","national","conference","team"};
inline constexpr std::array<const char*,4> scopeLabels={"For You","National","Conference","My Team"};
struct Scope {std::string key,label;};
struct SidebarCard {std::string title;std::vector<std::string> lines;};
struct Edition {std::vector<SidebarCard> sidebar;std::string key,label,conference,phase="RegularSeason";int season=0,week=0;std::map<std::string,FeedPost> stories;std::map<std::string,std::vector<std::string>> views;};
struct Catalog {std::vector<Edition> weeks;size_t active=0;std::vector<Scope> scopes;};
struct Feed {std::vector<FeedPost> posts;std::string key="runtime-waiting",label="Social Media";std::shared_ptr<const Catalog> catalog;size_t edition=0;unsigned scope=0,type=0;size_t total=0,page=0,pages=1;};
inline const std::vector<Scope>& scopesFor(const Catalog& c){static const std::vector<Scope> legacy={{"all","For You"},{"national","National"},{"conference","Conference"},{"team","My Team"}};return c.scopes.empty()?legacy:c.scopes;}
inline constexpr std::array<const char*,12> typeLabels={"All Types","Recruiting","Recaps","Upcoming","Rankings & Awards","Analysts","Beat Reporters","Official Accounts","Players","Injuries","Fans","Transactions"};
inline bool matchesType(const FeedPost& p,unsigned type){switch(type){case 0:return true;case 1:return p.category=="recruiting"||p.category=="offers"||p.category=="signings"||p.voice=="recruit";case 2:return p.category=="results"||p.category=="team-stats"||p.category=="player-stats"||p.category=="upsets"||p.category=="streaks"||p.category=="matchup-recap"||p.category=="season-end";case 3:return p.category=="schedule"||p.category=="watch"||p.category=="lines"||p.category=="preseason"||p.category=="matchup-preview"||p.category=="offseason";case 4:return p.category=="polls"||p.category=="heisman"||p.category=="cfp"||p.category=="awards"||p.category=="records"||p.category=="history"||p.category=="championship"||p.category=="bowls";case 5:return p.voice=="analyst"||p.voice=="model";case 6:return p.voice=="beat";case 7:return p.voice=="official"||p.voice=="cfp"||p.voice=="nflDraft";case 8:return p.voice=="player";case 9:return p.category=="injuries";case 10:return p.voice=="fan";case 11:return p.transaction||(p.category=="coaching"||p.category=="nil"||p.category=="transactions"||p.category=="transfer"||p.category=="signings")&&(p.voice=="analyst"||p.voice=="beat");default:return false;}}
// System status is the only fallback; there are no illustrative tweet cards.
inline std::shared_ptr<const Feed> statusFeed(unsigned count=1){
    auto f=std::make_shared<Feed>();
    for(unsigned n=0;n<count;++n)f->posts.push_back({"Beyond the Field","@DynastyEnhancer","Preparing news for the active Dynasty. Return to the hub briefly, then reopen Social Media.","Status",79,598});
    return f;
}
// Category matching precedes display limits. A matching child brings its
// original ancestors as context; it never becomes an orphan or a second root.
inline std::vector<std::string> feedCandidates(const Edition& e,const std::vector<std::string>& view,unsigned type){
    std::set<std::string> members(view.begin(),view.end()),included,full;
    std::vector<std::string> roots;std::map<std::string,std::vector<std::string>> children;
    for(const auto& id:view){const auto& p=e.stories.at(id);if(p.replyTo.empty())roots.push_back(id);else if(members.count(p.replyTo))children[p.replyTo].push_back(id);}
    for(const auto& id:view)if(matchesType(e.stories.at(id),type)){
        auto ancestor=id;std::set<std::string> seen;
        while(members.count(ancestor)&&seen.insert(ancestor).second){included.insert(ancestor);ancestor=e.stories.at(ancestor).replyTo;}
    }
    if(type==11)std::stable_sort(roots.begin(),roots.end(),[&](const auto& a,const auto& b){const auto x=e.stories.at(a).draftPick,y=e.stories.at(b).draftPick;return x>0&&y>0?x<y:x>0&&y<=0;});
    std::vector<std::string> result;std::set<std::string> visited;
    std::function<void(const std::string&,bool)> append=[&](const auto& id,bool whole){if(!visited.insert(id).second)return;const auto& p=e.stories.at(id);whole=whole||matchesType(p,type);if(!whole&&!included.count(id))return;result.push_back(id);for(const auto& child:children[id])append(child,whole);};
    for(const auto& id:roots)append(id,false);return result;
}
inline constexpr size_t nationalCardLimit=250;
// The game creates a real native widget for each returned card. Rank after
// category matching, retain actual ancestry and reserve the official draft.
inline std::vector<std::string> nationalSelection(const Edition& e,const std::vector<std::string>& candidates,unsigned type){
    if(candidates.size()<=nationalCardLimit)return candidates;
    const std::set<std::string> members(candidates.begin(),candidates.end());
    struct Thread{std::string root;std::vector<std::string> ids,minimum;size_t order=0;bool draft=false;double score=0;};
    std::vector<Thread> threads;std::map<std::string,size_t> positions;
    for(const auto& id:candidates){auto root=id;std::set<std::string> seen;while(members.count(e.stories.at(root).replyTo)&&seen.insert(root).second)root=e.stories.at(root).replyTo;
        if(!positions.count(root)){positions[root]=threads.size();Thread t;t.root=root;t.order=threads.size();threads.push_back(std::move(t));}threads[positions.at(root)].ids.push_back(id);}
    for(auto& t:threads){const auto& p=e.stories.at(t.root);t.draft=p.voice=="nflDraft"&&p.draftPick>0&&matchesType(p,type);
        const bool headline=p.category=="results"||p.category=="upsets"||p.category=="polls"||p.category=="cfp"||p.category=="awards"||p.category=="championship"||p.category=="draft";
        t.score=(p.important?10000:0)+(headline?1000:0)+p.priority;
        auto subject=std::find_if(t.ids.begin(),t.ids.end(),[&](const auto& id){return matchesType(e.stories.at(id),type);});
        if(subject!=t.ids.end()){auto id=*subject;while(members.count(id)){t.minimum.push_back(id);id=e.stories.at(id).replyTo;}std::reverse(t.minimum.begin(),t.minimum.end());}}
    std::stable_sort(threads.begin(),threads.end(),[](const auto& a,const auto& b){if(a.draft!=b.draft)return a.draft;return a.score!=b.score?a.score>b.score:a.order<b.order;});
    std::set<std::string> selected,selectedRoots;const auto draftCount=std::count_if(threads.begin(),threads.end(),[](const auto& t){return t.draft;});
    const auto protectedCount=std::count_if(threads.begin(),threads.end(),[&](const auto& t){return t.draft||e.stories.at(t.root).important;});
    const size_t rootTarget=(std::min)(nationalCardLimit,(std::max)(size_t(200),static_cast<size_t>((std::max)(draftCount,protectedCount))));
    auto take=[&](const Thread& t){if(t.minimum.empty()||selectedRoots.count(t.root)||selected.size()+t.minimum.size()>nationalCardLimit)return;selected.insert(t.minimum.begin(),t.minimum.end());selectedRoots.insert(t.root);};
    for(const auto& t:threads)if(t.draft)take(t);
    for(const auto& t:threads)if(selectedRoots.size()<rootTarget)take(t);
    for(const auto& id:candidates){const auto& p=e.stories.at(id);if(selected.size()<nationalCardLimit&&!p.replyTo.empty()&&selected.count(p.replyTo))selected.insert(id);}
    for(const auto& t:threads)if(selected.size()<nationalCardLimit)take(t);
    std::vector<std::string> result;for(const auto& id:candidates)if(selected.count(id))result.push_back(id);return result;
}
inline std::shared_ptr<const Feed> selectFeed(std::shared_ptr<const Catalog> catalog,size_t week,unsigned scope,unsigned type=0,size_t page=0){
    if(!catalog||week>=catalog->weeks.size()||scope>=scopesFor(*catalog).size()||type>=typeLabels.size())throw std::runtime_error("Invalid single-week feed selection");
    const auto& e=catalog->weeks.at(week);auto f=std::make_shared<Feed>();f->catalog=catalog;f->edition=week;f->scope=scope;f->type=type;f->key=e.key;f->page=page;
    f->label="Social Media";if(type)f->label+=" - "+std::string(typeLabels[type]);
    const auto& option=scopesFor(*catalog).at(scope);
    const auto broad=e.views.find(option.key=="all"?(e.views.count("all:expanded")?"all:expanded":e.views.count("all:default")?"all:default":"all"):option.key);
    std::vector<std::string> candidates=broad==e.views.end()?std::vector<std::string>{}:feedCandidates(e,broad->second,type);
    // One continuous native list per edition. Generation owns the team budget;
    // National is a bounded importance-ranked display; the stored archive is retained.
    (void)page;f->page=0;f->pages=1;f->total=candidates.size();
    if(candidates.size()>32768)throw std::runtime_error("Social Media edition exceeds the verified native list limit");
    if(option.key=="national")candidates=nationalSelection(e,candidates,type);
    for(const auto& id:candidates)f->posts.push_back(e.stories.at(id));
    for(size_t i=0;i<f->posts.size();++i)f->posts[i].displayStamp=f->posts[i].stamp+" | "+std::to_string(i+1)+"/"+std::to_string(f->posts.size());
    if(f->posts.empty())f->posts.push_back({"Beyond the Field","@DynastyEnhancer","No "+option.label+(type?" "+std::string(typeLabels[type]):"")+" stories match this weekly edition. Broader scopes and other categories can contain different news.","News",79,598});
    return f;
}
struct EditionChoice {size_t edition=0,page=0;std::string label;};
inline std::vector<EditionChoice> editionChoices(std::shared_ptr<const Catalog> catalog,size_t selected,unsigned scope,unsigned type,size_t currentPage=0){
    (void)selected;(void)scope;(void)type;(void)currentPage;std::vector<EditionChoice> choices;
    for(size_t i=0;i<catalog->weeks.size();++i)choices.push_back({i,0,catalog->weeks[i].label});
    return choices;
}
inline std::shared_ptr<const Feed> decodeFeed(const nlohmann::json& j){
    auto catalog=std::make_shared<Catalog>();
    if(j.contains("scopeOptions"))for(const auto& o:j.at("scopeOptions"))catalog->scopes.push_back({o.at("key").get<std::string>(),o.at("label").get<std::string>()});
    for(const auto& w:j.at("weeks")){
        Edition e;e.key=w.at("key").get<std::string>();e.week=w.at("week").get<int>();e.season=w.at("season").get<int>();e.conference=w.value("conference",nlohmann::json()).is_string()?w.at("conference").get<std::string>():"";
        e.phase=w.value("phase",std::string("RegularSeason"));
        e.label=e.phase=="PreSeason"?"Preseason, "+std::to_string(e.season):e.phase=="RegularSeason"?"Week "+std::to_string(e.week)+", "+std::to_string(e.season):e.phase+" - Week "+std::to_string(e.week)+", "+std::to_string(e.season);
        if(w.contains("publicationLabel"))e.label=w.at("publicationLabel").get<std::string>();
        if(w.value("reconstructed",false))e.label+=" (recovered)";
        if(w.contains("sidebar"))for(const auto& card:w.at("sidebar")){
            SidebarCard item;item.title=card.at("title").get<std::string>();item.lines=card.at("lines").get<std::vector<std::string>>();e.sidebar.push_back(std::move(item));if(e.sidebar.size()==3)break;
        }
        for(const auto& p:w.at("stories")){
            const auto stamp=storyLabel(p);
            FeedPost post{p.at("author").get<std::string>(),p.at("handle").get<std::string>(),p.at("body").get<std::string>(),stamp,p.at("imageLibrary").get<int>(),p.at("imageValue").get<int>()};post.category=p.value("category",std::string());post.voice=p.value("voice",std::string());post.replyTo=p.value("replyTo",std::string());post.likes=p.value("likes",-1);post.uniformTeamLogo=p.value("uniformTeamLogo",-1);post.transaction=p.value("transaction",false);post.draftPick=p.value("draftPick",-1);post.draftRound=p.value("draftRound",0);post.priority=p.value("priority",0.0);post.important=p.value("essentialNews",false)||p.value("coverageExempt",false)||p.value("budgetProtected",false);post.bodyHeight=postHeight(post.body);post.displayAuthor=post.author+" ("+post.handle+")";post.displayLikes=post.likes>=0?compactCount(post.likes)+" likes":std::string();e.stories.emplace(p.at("id").get<std::string>(),std::move(post));
        }
        for(const auto& [key,view]:w.at("views").items())e.views[key]=view.get<std::vector<std::string>>();
        catalog->weeks.push_back(std::move(e));
    }
    std::sort(catalog->weeks.begin(),catalog->weeks.end(),[](const Edition& a,const Edition& b){return refresh::phaseIndex(a.phase)!=refresh::phaseIndex(b.phase)?refresh::phaseIndex(a.phase)<refresh::phaseIndex(b.phase):a.week<b.week;});
    const auto activeKey=j.at("activeWeekKey").get<std::string>();
    const auto active=std::find_if(catalog->weeks.begin(),catalog->weeks.end(),[&](const Edition& e){return e.key==activeKey;});
    if(active==catalog->weeks.end())throw std::runtime_error("Active publication week is missing");
    catalog->active=static_cast<size_t>(active-catalog->weeks.begin());
    return selectFeed(catalog,catalog->active,0);
}
}

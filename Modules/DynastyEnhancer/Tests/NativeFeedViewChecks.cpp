#include "../Source/FeedData.h"
#include <iostream>
int main(int argc,char** argv){
    try{
        if(argc!=2)throw std::runtime_error("Expected isolated native-view fixture");
        nlohmann::json data;std::ifstream file(argv[1]);file>>data;unsigned checks=0;
        for(const auto& test:data.at("cases")){
            auto catalog=std::make_shared<social::Catalog>();social::Edition edition;edition.key="parity";
            const auto& feed=test.at("feed");
            for(const auto& p:feed.at("stories")){
                const auto id=p.at("id").get<std::string>();social::FeedPost post{id,"@fixture","Fixture body","News"};
                post.category=p.value("category",std::string());post.voice=p.value("voice",std::string());post.replyTo=p.value("replyTo",std::string());
                post.priority=p.value("priority",0.0);post.important=p.value("essentialNews",false)||p.value("coverageExempt",false)||p.value("budgetProtected",false);
                post.transaction=p.value("transaction",false);post.draftPick=p.value("draftPick",-1);edition.stories.emplace(id,std::move(post));
            }
            for(const auto& [key,value]:feed.at("views").items())edition.views[key]=value.get<std::vector<std::string>>();
            catalog->weeks.push_back(std::move(edition));
            for(unsigned scope=0;scope<4;++scope)for(unsigned type=0;type<12;++type){
                const auto expected=test.at("expected").at(social::scopeKeys[scope]).at(type).get<std::vector<std::string>>();
                const auto actual=social::selectFeed(catalog,0,scope,type);
                if(expected.empty()){if(actual->total!=0||actual->posts.size()!=1)throw std::runtime_error("Empty placeholder differs");}
                else{if(actual->posts.size()!=expected.size())throw std::runtime_error("Native/JS size differs: "+test.at("name").get<std::string>());for(size_t i=0;i<expected.size();++i)if(actual->posts[i].author!=expected[i])throw std::runtime_error("Native/JS ordering differs");}
                ++checks;
            }
        }
        std::cout<<checks<<" native/JS scope/category parity checks passed. No live game access.\n";
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
    return 0;
}

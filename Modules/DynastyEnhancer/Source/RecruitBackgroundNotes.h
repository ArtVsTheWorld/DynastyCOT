#pragma once
#include "RecruitNotes.h"
#include "ScoutingRoleNotes.h"
#include <array>
#include <string_view>

namespace recruitingfog::evidence {
using BackgroundGroup=std::array<std::string_view,10>;
inline constexpr std::array<std::string_view,40> commonBackground={
 "Works the Saturday opening shift at a grocery store. Usually gets the bread aisle.",
 "His school counselor says he tutors freshmen in algebra during lunch.",
 "Runs the sound board for school plays. Prefers the booth to the stage.",
 "Grows peppers with his granddad. Says the mild ones are a waste of salsa.",
 "He told us he fixes old game controllers rather than buying new ones.",
 "Keeps the clock at his younger sister's rec basketball games.",
 "His ceramics teacher says he stays late when there are mugs left to glaze.",
 "Works the shoe counter at the bowling alley and bowls a frame when it gets quiet.",
 "Plays drums in the school pep band when the football schedule lets him.",
 "Volunteers at the animal shelter. Usually gets put on dog walks.",
 "Fishes the same bend in the river with his cousins when they can get there early.",
 "Helps print shirts for school clubs. Says the small orders are the fiddly ones.",
 "Joined a school cooking club and now brings cornbread to the team potluck.",
 "Works the register at a family diner on Sundays.",
 "He likes old mystery novels and swaps paperbacks with his English teacher.",
 "Builds model cars. Says painting them is the part that takes all night.",
 "Keeps score at summer league softball. Knows the concession menu by heart.",
 "Edits the morning announcements. His favorite part is picking the walk-in music.",
 "Takes photos on hikes and prints a few for the school photography club.",
 "Helps an uncle tune up lawn mowers before the busy season.",
 "He's on the yearbook crew and usually covers the events that run late.",
 "Works at a plant nursery in summer. Brings home anything they'll let him rescue.",
 "He told us he's learning bass from his older brother.",
 "Helps shelve returns at the library after school.",
 "Plays in a pickup volleyball group at the community center.",
 "Spends some Saturdays helping his aunt set up at the farmers market.",
 "He and a friend make short stop-motion films for a school media class.",
 "Repairs the brakes and cables on neighborhood bikes.",
 "Works on costumes for the winter musical. Learned to sew buttons in a hurry.",
 "Plays card games with his grandparents on Sunday nights.",
 "Works early shifts at a bakery when school is out.",
 "Keeps going back to chess club even when his usual opening lets him down.",
 "Does the posters for student council. Usually draws the lettering by hand.",
 "He told us he works on crossword puzzles on the bus.",
 "Helps sort food donations through a service club at school.",
 "Makes playlists for the team bus. Gives everyone one veto.",
 "Takes a woodworking elective. Started with a birdhouse and is trying a bookshelf.",
 "Lifeguards at the public pool in the summer.",
 "Runs a table at the school's board-game night.",
 "His family says he's the one who puts together the furniture when a box shows up."
};

inline constexpr BackgroundGroup quarterbackBackground={
 "Keeps the book for his brother's Little League team.",
 "Does the weather segment for the school video announcements.",
 "Plays catcher in a summer baseball league.",
 "Helps organize the doubles bracket at the park's tennis tournament.",
 "Volunteers to read with younger kids at an after-school program.",
 "Works the ball machine at the local batting cages.",
 "His debate partner says they practice on the ride to school.",
 "Shoots pool with his uncle at the rec hall on Saturdays.",
 "His woodworking project this term is a set of cornhole boards.",
 "Helps tape school concerts for families who can't be there."
};
inline constexpr BackgroundGroup backfieldBackground={
 "Runs the second leg of his school's spring relay.",
 "Works at a bike shop over the summer and takes long rides on his days off.",
 "Plays left field in a rec baseball league with his cousins.",
 "He and his sister trade off feeding the chickens before school.",
 "Works weekend shifts at an ice cream stand. Still orders vanilla.",
 "Has a part in the school's step team showcase.",
 "Takes kayak trips with his family when the river is up.",
 "Helps paint sets for the spring play. Volunteers for anything that needs a ladder.",
 "He told us he likes inline skating at the old rink.",
 "Handles the warm-up music at his younger brother's soccer games."
};
inline constexpr BackgroundGroup receiverBackground={
 "He runs track in the spring and likes the relay more than the open races.",
 "Plays outside hitter in a weekend volleyball league.",
 "Helps frame prints for a small photo shop.",
 "His art teacher says he's working on a series of drawings of old gymnasiums.",
 "Works the front desk at a dance studio some evenings.",
 "Likes to go rock climbing with friends when he has a free Saturday.",
 "Volunteers to keep score at the neighborhood three-on-three tournament.",
 "He told us he is learning to string a tennis racket himself.",
 "Plays alto sax in a jazz group at school.",
 "He and a teammate restore old sneakers and trade cleaning tips."
};
inline constexpr BackgroundGroup offensiveLineBackground={
 "Wrestles in the winter. His family says he likes the bus rides as much as the meets.",
 "Throws discus in the spring. Keeps an old practice disc in his trunk.",
 "Works an evening shift at the hardware store when school lets out.",
 "His shop project is a tool chest that he plans to give his dad.",
 "Helps run the smoker at his family's neighborhood cookout.",
 "Volunteers to move sets for the school theater club.",
 "Repairs fishing reels with his grandfather.",
 "He told us he got into bread making after a school food science class.",
 "Works the loading counter at a feed store on weekends.",
 "Plays the big drum at the winter band concert."
};
inline constexpr BackgroundGroup defensiveLineBackground={
 "Throws shot put for the school in the spring.",
 "Works with his cousin at a garage repainting old steel bike frames.",
 "Helps unload equipment at the community theater.",
 "He told us he makes hot sauce with a friend from school.",
 "Plays in a winter rec basketball league. Usually asks for the late game.",
 "Fixes up old record players and collects used records.",
 "Helps maintain the garden beds behind his school.",
 "Holds pads for the kids' class at a local martial arts gym.",
 "Works the evening closing shift at a car wash.",
 "Learned to fly-fish from a neighbor and ties some of his own flies."
};
inline constexpr BackgroundGroup linebackerBackground={
 "Sets the bases and chalks the lines before youth baseball games.",
 "Plays catcher in a rec softball league with his cousins.",
 "Helps check in runners at the local Saturday 5K.",
 "He told us he spends weeknights working on a community radio show.",
 "Wrestles in the winter and helps roll out the mats before the first match.",
 "Leaves school early twice a week for an auto shop elective.",
 "He helps make the pancake breakfast at the community center.",
 "Teaches his younger cousins how to ride bikes in the school parking lot.",
 "Works the paint desk at a hardware store. Says the shade names are half the job.",
 "Keeps a list of the local trails he wants to hike before graduation."
};
inline constexpr BackgroundGroup secondaryBackground={
 "Plays center field in a summer baseball league.",
 "His friends rope him into refereeing pickup soccer.",
 "Takes a photography class and likes shooting the lights at night games.",
 "Runs hurdles in the spring. Helps the coach carry them out on meet days.",
 "Spends some weekends playing doubles in a park tennis league.",
 "Works at a screen-printing shop and helps with the school club orders.",
 "Covers school concerts for the student newspaper.",
 "Has been learning to play the piano from his aunt.",
 "He and his friends organize a video-game tournament at the library.",
 "Told us he started birding after borrowing a pair of binoculars on a hike."
};
inline constexpr BackgroundGroup specialistBackground={
 "Plays club soccer in the spring.",
 "Works the starter's stand at the municipal golf course.",
 "His class plays in a lunchtime badminton league. He brings the shuttlecocks.",
 "Helps hand out skates during public sessions at the ice rink.",
 "Plays trumpet in the winter concert band.",
 "Keeps a running list of the miniature golf courses he's played.",
 "His part-time job is stringing rackets at a sporting goods store.",
 "Spends some weekends playing disc golf with his cousin.",
 "Volunteers as a line judge at his sister's volleyball matches.",
 "He told us he makes wooden picture frames in shop class."
};
inline constexpr BackgroundGroup athleteBackground={
 "Bounces between open gym and the track when neither team has practice.",
 "Started bouldering after a friend's birthday trip to the climbing gym.",
 "Plays shortstop in summer ball and fills in at catcher when they need him.",
 "Builds theater props. This year's project is a door that won't stay shut.",
 "Works the rental counter at a park that has both bikes and canoes.",
 "He and his cousin keep a standing doubles pickleball game on Sundays.",
 "Runs sound for the school talent show and helps stack chairs when it's over.",
 "Takes the early fishing spot when his siblings want to sleep in.",
 "Plays pickup basketball with friends who go to another school.",
 "Wants to finish the community center's stained-glass class before he graduates."
};

inline const BackgroundGroup& roleBackground(NR role){
 switch(role){
 case NR::Qb:return quarterbackBackground;
 case NR::Halfback:case NR::Fullback:return backfieldBackground;
 case NR::Receiver:case NR::TightEnd:return receiverBackground;
 case NR::Tackle:case NR::Guard:case NR::Center:return offensiveLineBackground;
 case NR::Edge:case NR::Interior:return defensiveLineBackground;
 case NR::InsideLb:case NR::OutsideLb:case NR::Sam:case NR::Will:return linebackerBackground;
 case NR::Corner:case NR::FreeSafety:case NR::StrongSafety:return secondaryBackground;
 case NR::Kicker:case NR::Punter:case NR::Snapper:return specialistBackground;
 default:return athleteBackground;
 }
}

inline std::string_view newBackground(const NotesProspect& p){
    auto draw=notesHash(p.identity+"|recruit-background-v1");
    draw^=draw>>30;draw*=0xbf58476d1ce4e5b9ull;draw^=draw>>27;draw*=0x94d049bb133111ebull;draw^=draw>>31;
    // Native development is consulted only on an independent 1-in-128 draw.
    // The wording is a coach's impression, never a declaration of his trait.
    if((draw>>57)==0){
        if(p.backgroundDevelopment=="College_Elite")return "His coach says he rarely has to go over the same correction twice.";
        if(p.backgroundDevelopment=="College_Star")return "His coach says he usually brings something from a lesson into the next practice.";
        if(p.backgroundDevelopment=="College_Impact")return "His coach says a few good reps tend to make a new assignment click.";
        if(p.backgroundDevelopment=="Normal")return "His coach says he likes a few walk-throughs before a new routine feels right.";
    }
    const auto i=draw%(commonBackground.size()+BackgroundGroup{}.size());
    return i<commonBackground.size()?commonBackground[i]:roleBackground(noteRole(p.position))[i-commonBackground.size()];
}

inline bool revealBackground(const NotesProspect& p){
    return p.schoolBackgroundEligible&&!p.activeTransfer&&p.progress==100&&!p.identity.empty();
}
}

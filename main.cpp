#include <iostream>
#include <ctime>

#include <cstdlib>
#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>

#include "./algorithms/htree.h"
#include "./algorithms/opIndex.h"
#include "./algorithms/rein.h"
#include "./algorithms/siena.h"
#include "./algorithms/tama.h"
#include "./algorithms/subscriptionForest.h"


using namespace std;
using json = nlohmann::json;

void runAlgos(json j, string outputFileName) {
    vector<string> algos = j["algos"];        //
    vector<unsigned> subsRange = j["number_of_subscriptions"];           // Number of subscriptions.
    int pubs = j["number_of_events"];           // Number of publications.
    vector<unsigned> attsRange = j["number_of_dimensions"];           // Total number of attributes, i.e. dimensions.
    int consPer = j["percentage_of_subscription_attributes"];           // Percentage of dimensions as constraints(predicates) in one sub.
    int mPer = j["percentage_of_event_attributes"];              // Number of constraints in one pub.
    int valDom = j["cardinality_of_values"];         // Cardinality of values.
    int attDis = j["attribute_distribution"];         // The distribution of attributes in subs and pubs. 0:uniform distribution | 1:Zipf distribution
    int valDis = j["value_distribution"];        // The distribution of values in subs and pubs. 0:uniform
    double alpha = j["alpha"];       // Parameter for Zipf distribution.
    double width = j["width"];       // Width of a predicate.

    for (unsigned subs = subsRange.at(0); subs <= subsRange.at(1); subs += subsRange.at(2)) {
        for (unsigned atts = attsRange.at(0); atts <= attsRange.at(1); atts += attsRange.at(2)) {
            const unsigned cons = (atts * consPer) / 100;           // Number of constraints(predicates) in one sub.
            const unsigned m = (atts*mPer)/100;              // Number of constraints in one pub.

            cout << "Atts: " << atts << " Cons: " << cons << " Subs: " << subs << endl;
            intervalGenerator gen(subs, pubs, atts, cons, m, attDis, valDis, valDom, alpha, width);
            gen.GenSubList();
            gen.GenPubList();

            vector<pair<string, Broker *>> toRun;

            for (std::vector<string>::iterator it = algos.begin(); it != algos.end(); ++it) {
                std::cout << *it << endl;
                if ((*it).compare("htree") == 0) {
                    std::ifstream i("./config/algos/htree.json");
                    json htreeConfig;
                    i >> htreeConfig;
                    int levels = htreeConfig["levels"];
                    int cells = htreeConfig["cells"];
                    toRun.push_back(make_pair("h-tree", new Htree(atts, levels, cells, valDis, valDom)));
                } else if ((*it).compare("siena") == 0) {
                    toRun.push_back(make_pair("siena", new Siena()));
                } else if ((*it).compare("subscriptionForest") == 0) {
                    const unsigned constAtts = atts;
                toRun.push_back(make_pair("subscriptionForest", new SubscriptionForest(atts,m) ));
                } else if ((*it).compare("rein") == 0) {
                    toRun.push_back(make_pair("rein", new Rein(valDom)));
                } else if ((*it).compare("opIndex") == 0) {
                    toRun.push_back(make_pair("opIndex", new opIndex()));
                } else if ((*it).compare("tama") == 0) {
                    std::ifstream i("./config/algos/tama.json");
                    json tamaConfig;
                    i >> tamaConfig;
                    int levels = tamaConfig["levels"];
                    toRun.push_back(make_pair("tama", new Tama(atts, valDom, levels)));
                }

                while (!toRun.empty()) {
                    cout << "Running " << (toRun.at(0)).first << endl;

                    vector<double> insertTimeList;
                    vector<double> matchTimeList;
                    vector<double> matchSubList;

                    for (int i = 0; i < subs; i++) {
                        Timer subStart;

                        (*((toRun.at(0)).second)).insert(
                                gen.subList[i]);                       // Insert sub[i] into data structure.

                        int64_t insertTime = subStart.elapsed_nano();   // Record inserting time in nanosecond.
                        insertTimeList.push_back((double) insertTime / 1000000);
                    }


                    // match
                    for (int i = 0; i < pubs; i++) {
                        int matchSubs = 0;                              // Record the number of matched subscriptions.
                        Timer matchStart;

                        (*((toRun.at(0)).second)).match(gen.pubList[i], matchSubs, gen.subList);

                        int64_t eventTime = matchStart.elapsed_nano();  // Record matching time in nanosecond.
                        matchTimeList.push_back((double) eventTime / 1000000);
                        matchSubList.push_back(matchSubs);
                    }


                    // output
                    string content =
                            (toRun.back()).first + "," + Util::Int2String(subs) + ","+ Util::Int2String(atts) + "," +
                            Util::Double2String(Util::mean(insertTimeList)) + "," +
                            Util::Double2String(Util::mean(matchTimeList)) + "," +
                            Util::Double2String(Util::mean(matchSubList));
                    Util::WriteData(outputFileName.c_str(), content);

                    toRun.erase(toRun.begin());
                }
            }
        }
    }
}


int main(int argc, char **argv) {
    std::ifstream i("./config/config.json");
    json j;
    i >> j;

    std::time_t timestamp = std::time(nullptr);
    string outputFileName = "output/" + std::to_string(timestamp) + ".csv";

    string header = "Algorithm,Subscriptions,Dimensions,Mean insert subscription time,Mean event match time,Mean number of matched subscriptions";
    Util::WriteData(outputFileName.c_str(), header);

    runAlgos(j, outputFileName);

    return 0;

}


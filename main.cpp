#include <iostream>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>

#include "./algorithms/htree.h"
#include "./algorithms/opIndex.h"
#include "./algorithms/rein.h"
#include "./algorithms/siena.h"
#include "./algorithms/tama.h"


using namespace std;
using json = nlohmann::json;

//int main(int argc, char **argv)
//{
//    string algo;        //
//    int subs;           // Number of subscriptions.
//    int pubs;           // Number of publications.
//    int atts;           // Total number of attributes, i.e. dimensions.
//    int cons;           // Number of constraints(predicates) in one sub.
//    int m;              // Number of constraints in one pub.
//    int valDom;         // Cardinality of values.
//    int attDis;         // The distribution of attributes in subs and pubs. 0:uniform distribution | 1:Zipf distribution
//    int valDis;         // The distribution of values in subs and pubs. 0:uniform
//    double alpha;       // Parameter for Zipf distribution.
//    double width;       // Width of a predicate.
//
//    freopen("paras.txt", "r", stdin);
//    cin >> algo >> subs >> pubs >> atts >> cons >> m >> attDis >> valDis >> valDom;
//    cin >> alpha >> width;
//
//    m = atts;           // Note that Rein requires m == atts.
//    vector<double> insertTimeList;
//    vector<double> matchTimeList;
//    vector<double> matchSubList;
//
//    // Initiate generator
//    intervalGenerator gen(subs, pubs, atts, cons, m, attDis, valDis, valDom, alpha, width);
//    gen.GenSubList();
//    gen.GenPubList();
//
//
//    Rein a(valDom);
////    Siena a;
//
//    // insert
//    for (int i = 0; i < subs; i++)
//    {
//        Timer subStart;
//
//        a.insert(gen.subList[i]);                       // Insert sub[i] into data structure.
//
//        int64_t insertTime = subStart.elapsed_nano();   // Record inserting time in nanosecond.
//        insertTimeList.push_back((double) insertTime / 1000000);
//    }
//
//
//    // match
//    for (int i = 0; i < pubs; i++)
//    {
//        int matchSubs = 0;                              // Record the number of matched subscriptions.
//        Timer matchStart;
//
//        a.match(gen.pubList[i], matchSubs, gen.subList);
//
//        int64_t eventTime = matchStart.elapsed_nano();  // Record matching time in nanosecond.
//        matchTimeList.push_back((double) eventTime / 1000000);
//        matchSubList.push_back(matchSubs);
//    }
//
//
//    // output
//    string outputFileName = "results.csv";
//    string content = algo + "," + Util::Int2String(subs) + "," + Util::Double2String(Util::mean(insertTimeList)) + "," +
//                     Util::Double2String(Util::mean(matchTimeList)) + "," +
//                     Util::Double2String(Util::mean(matchSubList));
//    Util::WriteData(outputFileName.c_str(), content);
//
//
//    // check®
//
//    return 0;
//}


void runAlgo(json j)
{
    vector<string> algos = j["algos"];        //
    int subs = j["number_of_subscriptions"];           // Number of subscriptions.
    int pubs = j["number_of_events"];           // Number of publications.
    int atts = j["number_of_dimensions"];           // Total number of attributes, i.e. dimensions.
    int cons = j["number_of_subscription_attributes"];           // Number of constraints(predicates) in one sub.
    int m = j["number_of_event_attributes"];              // Number of constraints in one pub.
    int valDom = j["cardinality_of_values"];         // Cardinality of values.
    int attDis = j["attribute_distribution"];         // The distribution of attributes in subs and pubs. 0:uniform distribution | 1:Zipf distribution
    int valDis = j["value_distribution"];        // The distribution of values in subs and pubs. 0:uniform
    double alpha = j["alpha"];       // Parameter for Zipf distribution.
    double width = j["width"];       // Width of a predicate.

    vector<double> insertTimeList;
    vector<double> matchTimeList;
    vector<double> matchSubList;

    // Initiate generator
    intervalGenerator gen(subs, pubs, atts, cons, m, attDis, valDis, valDom, alpha, width);
    gen.GenSubList();
    gen.GenPubList();

    vector<pair <string, Broker*>> toRun;

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
        } else if ((*it).compare("rein") == 0) {
            toRun.push_back(make_pair("rein", new Rein(valDom)));
        }
    }

//        if (algo.compare("opIndex") == 0) {
//            cout << "Running opIndex" << endl;
//            opIndex newA;
//            newA.calcFrequency(gen.subList);
//            a = newA;
//            return a;
//        }

//        if (algo.compare("tama") == 0) {
//            cout << "Running TAMA" << endl;
////        std::ifstream i("./config/algos/tama.json");
////        json tamaConfig;
////        i >> tamaConfig;
////        int levels = tamaConfig["levels"];
////        cout << levels << endl;
//            Tama newA(atts, valDom, 10);
//            a = newA;
//            return a;
//        }

    while(!toRun.empty()) {
        cout << "Running " << (toRun.back()).first << endl;

        for (int i = 0; i < subs; i++) {
            Timer subStart;

            (*((toRun.back()).second)).insert(gen.subList[i]);                       // Insert sub[i] into data structure.

            int64_t insertTime = subStart.elapsed_nano();   // Record inserting time in nanosecond.
            insertTimeList.push_back((double) insertTime / 1000000);
        }


        // match
        for (int i = 0; i < pubs; i++) {
            int matchSubs = 0;                              // Record the number of matched subscriptions.
            Timer matchStart;

            (*((toRun.back()).second)).match(gen.pubList[i], matchSubs, gen.subList);

            int64_t eventTime = matchStart.elapsed_nano();  // Record matching time in nanosecond.
            matchTimeList.push_back((double) eventTime / 1000000);
            matchSubList.push_back(matchSubs);
        }


        // output
        string outputFileName = "results.csv";
        string content =
            (toRun.back()).first + "," + Util::Int2String(subs) + "," + Util::Double2String(Util::mean(insertTimeList)) + "," +
                Util::Double2String(Util::mean(matchTimeList)) + "," +
                Util::Double2String(Util::mean(matchSubList));
        Util::WriteData(outputFileName.c_str(), content);

        toRun.pop_back();
    }
}


int main(int argc, char **argv) {
    std::ifstream i("./config/config.json");
    json j;
    i >> j;

    vector<string> algos = j["algos"];        //

//    std::for_each(algos.begin(), algos.end(),
//            [=](string algo){ runAlgo(algo, j); });
    runAlgo(j);


//    for (std::vector<string>::iterator it = algos.begin(); it != algos.end(); ++it){
//        std::cout << ' ' << *it;
//        std::cout << '\n';
//        runAlgo(*it, j);
//    }

    return 0;

}


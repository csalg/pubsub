/*
    CONTENT-BASED PUBLISH-SUBSCRIBE TESTING FRAMEWORK
    Developed at Shanghai Jiaotong University, Computer Science Department.
    GPL License. Please keep any contributions to this software open source.
    Only works on POSIX systems.
*/


#include <iostream>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <regex>
#include <nlohmann/json.hpp>
#include <dirent.h>

#include "./common/csv_parser.hpp"
#include "./algorithms/htree.h"
#include "./algorithms/opIndex.h"
#include "./algorithms/rein.h"
#include "./algorithms/siena.h"
#include "./algorithms/vari-siena.h"
#include "./algorithms/tama.h"
#include "./algorithms/subscriptionForest.h"
#include "./algorithms/ACTreeLogVolume.h"
#include "./algorithms/ACTreeNearestCenter.h"



using namespace std;
using json = nlohmann::json;

void evaluate(
        int atts,            // Number of attributes in total.
        int constraints_,     // Number of constraints(predicates) in one sub.
        int m,               // Number of values in one pub.
        double width,           // Interval width
        int alpha,           // Zipf distribution parameter
        double distance_from_mean,           // Interval width
        int valDom,
        int valDis,
        vector<string> &algos,
        const vector<Pub> &events,
        const vector<IntervalSub> &subscriptions,
        string outputFileName
) {
    // This is the function that actually constructs and evaluates the algorithms and writes to csv with the results.

    Broker* willRun;
    struct AlgorithmRunnable {
        Broker*     algo;
        string      name;
        double      param;
        AlgorithmRunnable(string name, Broker * algo, double param): algo(algo), name(name), param(param){};
    };
    vector<AlgorithmRunnable> toRun; // This is a workaround for dynamic allocation, casting to Broker means that it inherits the superclass' methods

    for (auto & algo : algos) {
        cout<< algo <<endl;
        if (algo == "H-Tree") {
            std::ifstream i("./config/algos/htree.json");
            json htreeConfig;
            i >> htreeConfig;
            int levels = htreeConfig["levels"];
            int cells = htreeConfig["cells"];
            toRun.emplace_back("H-Tree", new Htree(atts, levels, cells, valDis, valDom), 0);
        }
//        else if (algo == "Siena")
//            toRun.emplace_back("Siena", new Siena());
//        else if (algo == "Vari-Siena")
//            toRun.emplace_back("Vari-Siena", new VariSiena());
        else if (algo == "ACTree")
        {
            std::ifstream i("./config/algos/ACTree.json");
            json algoConfig;
            i >> algoConfig;
            vector<unsigned short>  maxSpans        = algoConfig["MAX_SPAN"];
            unsigned short          leaveOutRate    = algoConfig["LEAVE_OUT"];
            unsigned short          levels          = algoConfig["LEVELS"];
            for (auto maxSpan= maxSpans[0]; maxSpan <= maxSpans[1]; maxSpan += maxSpans[2])
                toRun.emplace_back("ACTree", new SubscriptionClusterTree(leaveOutRate, levels, maxSpan), maxSpan);
        }
        else if (algo == "REIN")
            toRun.emplace_back("REIN", new Rein(MAX_CARDINALITY), 0);
        else if (algo == "OpIndex")
            toRun.emplace_back("OpIndex", new opIndex(), 0);
        else if (algo == "TAMA")
        {
            std::ifstream i("./config/algos/tama.json");
            json tamaConfig;
            i >> tamaConfig;
            int levels = tamaConfig["levels"];
            toRun.emplace_back("TAMA", new Tama(atts, valDom, levels), 0);
        }


        while (!toRun.empty()) {
            cout << (toRun.back()).name << endl;

            vector<double> insertTimeList;
            vector<double> matchTimeList;
            vector<double> matchSubList;

            // Insert operation
            auto i = 1;
            for (const auto & subscription : subscriptions) {
//                cout << "Sub " <<q i << endl;
                ++i;
                Timer subStart;

                toRun.back().algo->insert(subscription);   // Insert sub[i] into data structure.

                int64_t insertTime = subStart.elapsed_nano();   // Record inserting time in nanoseconds
                insertTimeList.push_back((double) insertTime / 1000000);
            }



            // Stabbing query
            for (int i = 0; i < 1; i++) {
                int matchSubs = 0;                              // Record the number of matched subscriptions.
                Timer matchStart;

                toRun.back().algo->match(events.at(i), matchSubs, subscriptions);

                int64_t eventTime = matchStart.elapsed_nano();  // Record matching time in nanosecond.
                matchTimeList.push_back((double) eventTime / 1000000);
                matchSubList.push_back(matchSubs);
//                cout << "Matched: " << matchSubs << endl;
            }

            // Write output
            cout.precision(3);
            string content =
                    toRun.back().name + "," +
                    Util::Double2String(toRun.back().param) + "," +
                    Util::Int2String(subscriptions.size()) + "," +
                    Util::Int2String(atts) + "," +
                    Util::Int2String(constraints_) + "," +
                    Util::Double2String(alpha) + "," +
                    Util::Int2String(m) + "," +
                    Util::Double2String(width) + "," +
                    Util::Double2String(distance_from_mean) + "," +
                    Util::Double2String(Util::mean(insertTimeList)) + "," +
                    Util::Double2String(Util::mean(matchTimeList)) + "," +
                    Util::Double2String(Util::mean(matchSubList));
            Util::WriteData(outputFileName.c_str(), content);
            cout << content << endl;
//            cout <<                     Util::Double2String(Util::mean(insertTimeList)) + "," +
//                                        Util::Double2String(Util::mean(matchTimeList)) + "," +
//                                        Util::Double2String(Util::mean(matchSubList)) << endl;

            toRun.erase(toRun.end());
        }
    }
}

//
//void evaluate(
//        int atts,            // Number of attributes in total.
//        int constraints_,     // Number of constraints(predicates) in one sub.
//        int m,               // Number of values in one pub.
//        int width,           // Interval width
//        int alpha,           // Zipf distribution parameter
//        double distance_from_mean,           // Interval width
//        int valDom,
//        int valDis,
//        vector<string> &algos,
//        const vector<Pub> &events,
//        const vector<IntervalSub> &subscriptions,
//        string outputFileName
//) {
//    // This is the function that actually constructs and evaluates the algorithms and writes to csv with the results.
//
//    SubscriptionClusterTree willRun(0, 3, 20 );
//    string name = "ACTree";
//    string algoParam;
//
//    vector<double> insertTimeList;
//    vector<double> matchTimeList;
//    vector<double> matchSubList;
//
//    // Insert operation
//    for (const auto &subscription : subscriptions) {
//        Timer subStart;
//
//        willRun.insert(subscription);                       // Insert sub[i] into data structure.
//
//        int64_t insertTime = subStart.elapsed_nano();   // Record inserting time in nanoseconds
//        insertTimeList.push_back((double) insertTime / 1000000);
//    }
//
////    willRun.print();
////    assert(false);
////
//
//    // Stabbing query
//    for (int i = 0; i < events.size(); i++) {
//        int matchSubs = 0;                              // Record the number of matched subscriptions.
//        Timer matchStart;
//
//        willRun.match(events.at(i), matchSubs, subscriptions);
//
//        int64_t eventTime = matchStart.elapsed_nano();  // Record matching time in nanosecond.
//        matchTimeList.push_back((double) eventTime / 1000000);
//        matchSubList.push_back(matchSubs);
////                cout << "Matched: " << matchSubs << endl;
//    }
//
//    // Write output
//    cout.precision(3);
//    string content =
//            name + "," +
//            Util::Int2String(subscriptions.size()) + "," +
//            Util::Int2String(atts) + "," +
//            Util::Int2String(constraints_) + "," +
//            std::to_string(alpha) + "," +
//            Util::Int2String(m) + "," +
//            std::to_string(width) + "," +
//            std::to_string(distance_from_mean) + "," +
//            Util::Double2String(Util::mean(insertTimeList)) + "," +
//            Util::Double2String(Util::mean(matchTimeList)) + "," +
//            Util::Double2String(Util::mean(matchSubList));
//    Util::WriteData(outputFileName.c_str(), content);
//    cout << content << endl;
////            cout <<                     Util::Double2String(Util::mean(insertTimeList)) + "," +
////                                        Util::Double2String(Util::mean(matchTimeList)) + "," +
////                                        Util::Double2String(Util::mean(matchSubList)) << endl;
//
//}

void evaluateUsingSyntheticData(json j, string outputFileName) {
    /* As the name implies, this function will read the json file parameters,
     * and use that to generate purely synthetic data, with which it then
     * calls the evaluator.
     * */

    cout << "Evaluating using synthetic data." << endl;

    // Parsing and casting from json
    vector<string> algos = j["algos"];
    vector<unsigned> subsRange = j["number_of_subscriptions"];             // Number of subscriptions.
    int pubs = j["number_of_events"];                    // Number of publications.
    vector<unsigned> attsRange = j["number_of_dimensions"];                // Total number of attributes, i.e. dimensions.
    int consPer = j["subscription_constraints"]; // Percentage of dimensions as constraints(predicates) in one sub.
    int mPer = j["event_pairs"];      // Number of constraints in one pub.
    int valDom = j["cardinality_of_values"];               // Cardinality of values.
    int attDis = j["attribute_distribution"];              // The distribution of attributes in subs and pubs. 0:uniform distribution | 1:Zipf distribution
    int valDis = j["value_distribution"];                  // The distribution of values in subs and pubs. 0:uniform
    double alpha = j["alpha"];                               // Parameter for Zipf distribution.
    vector<double> widths = j["width"];                               // Width of a predicate.

    for (double width = widths.at(0); width <= widths.at(1); width += widths.at(2)) {
        for (unsigned subs = subsRange.at(0); subs <= subsRange.at(1); subs += subsRange.at(2)) {
            for (unsigned atts = attsRange.at(0); atts <= attsRange.at(1); atts += attsRange.at(2)) {

                unsigned constraints_ = (atts * consPer) / 100; // Number of constraints(predicates) in one sub.
                unsigned m = (atts * mPer) / 100;               // Number of constraints in one pub.

                cout << "Dimensions: " << atts << ", Width: " << width << ", Constraints: " << constraints_
                     << ", Subscriptions: " << subs << endl;
                intervalGenerator gen(subs, pubs, atts, constraints_, m, attDis, valDis, valDom, alpha, width);
                gen.GenSubList();
                gen.GenPubList();

                vector<pair<string, Broker *>> toRun; // This is a workaround for dynamic allocation, casting to Broker means that it inherits the superclass' methods

                evaluate(atts,            // Number of attributes in total.
                         constraints_,     // Number of constraints(predicates) in one sub.
                         m,               // Number of values in one pub.
                         width,           // Interval width
                         0,
                         0,
                         valDom,
                         valDis,
                         algos,
                         gen.pubList,
                         gen.subList,
                         outputFileName
                );

                std::vector<IntervalSub>().swap(gen.subList);   // Defensive coding:
                std::vector<Pub>().swap(gen.pubList);           // preventing memory leaks.

            }
        }
    }
}

void evaluateUsingDataAugmentation(json j, string augmentedDataDirectory_, string outputFileName){
    /* As the name implies, this function will read the generated CSV, and
     * generate subscriptions and events by adding and subtracting the std. deviation
     * from the given values according to the parameters.
     * Then it passes this to the evaluator.
    */

    static string augmentedDataDirectory = augmentedDataDirectory_; // This is necessary so that it can be used in a struct later

//    vector<double> widths = j["width"];                               // Width of a predicate.
//    int number_of_constraints = j["number_of_constraints"];

    cout << "Evaluating using augmented data." << endl;

    cout << "Parsing events" << endl;
    vector<Pub> events;
    vector<double> events_attributes;
    vector<double> standard_deviations;

    cout << augmentedDataDirectory + "events.csv" << endl;
    {
        std::ifstream f(augmentedDataDirectory + "events.csv");
        aria::csv::CsvParser parser = aria::csv::CsvParser(f).delimiter(' ');

        for (auto &row : parser) {
            int attribute = 0;
            events.emplace_back();
            for (auto &field : row) {
                Pair pair;
                pair.att = attribute, pair.value = Util::safeValue((stof(field) * 100)+5000);

                events.back().pairs.push_back(pair);

                ++attribute;
                events.back().size=attribute;
            }
            events_attributes.push_back(attribute);
        }
    }

    for (auto pair : events[0].pairs) {
        cout << pair.att << ": " << pair.value << ", ";
    }
    cout << endl;

    cout << "Parsing standard deviations" << endl;
    {
        std::ifstream f( augmentedDataDirectory + "standard_deviations.csv");
        aria::csv::CsvParser parser = aria::csv::CsvParser(f).delimiter(' ');
        for (auto &row: parser){
            for (auto &field : row) standard_deviations.push_back(stof(field));
        }
    }

    cout << "Looking inside the " << augmentedDataDirectory << "subscriptions folder" << endl;
    {
        struct AugmentedDataFile {
            string timestamp;
            string subscriptions()  { return augmentedDataDirectory + "subscriptions/" + timestamp + ".csv"; }
            string config()         { return augmentedDataDirectory + "subscriptions/" + timestamp + ".json"; }
            explicit AugmentedDataFile(string timestamp) : timestamp(timestamp){};
        };

        vector<AugmentedDataFile> augmentedDataFiles;

        // Rather ugly C code to read files in directory using dirent.h (only works on POSIX)
        // Since this is the only place in the codebase where it is used, it stays here.
        // Otherwise it would make sense to throw it into a helper function in the common module.
        DIR             *dir;
        struct dirent   *ent;

        if ((dir = opendir("./data/augmented/subscriptions")) != NULL) {
            while ((ent = readdir(dir)) != NULL) {
                if (std::regex_match(ent->d_name, std::regex("(.*)(.json)"))) {
                    string timestamp = ent->d_name;
                    string timestampParsed = timestamp.substr(0,timestamp.find("."));
                    augmentedDataFiles.emplace_back(timestampParsed);
                }
            }
            closedir(dir);
        } else {
            perror("Could not open directory");
        }

        cout << augmentedDataFiles.size() << " augmented data files found" << endl;


        intervalGenerator gen; // This is used for generating the attributes using a Zipf.

        for (auto augmentedDataFile : augmentedDataFiles) {
            // Read and parse json
            cout << "Evaluating file " << augmentedDataFile.config() << "." << endl;
            json config;
            std::ifstream i(augmentedDataFile.config());
            i >> config;

            unsigned    number_of_subscriptions  = config["number_of_subscriptions"];
            double      width                    = config["width"];
            unsigned    maxAtts                  = config["maxAtts"];
            unsigned    alpha                    = config["alpha"];
            double      distance_from_mean       = config["distance_from_mean"];
            unsigned    number_of_constraints    = config["number_of_constraints"];

            cout << "Number of subscriptions: " << number_of_subscriptions << endl;
            cout << "Number of constraints: "   << number_of_constraints << endl;
            cout << "Width: "                   << width << endl;
            cout << "Distance from mean: "      << distance_from_mean << endl;

            vector<IntervalSub> subscriptions;

            std::ifstream f(augmentedDataFile.subscriptions());
            aria::csv::CsvParser parser = aria::csv::CsvParser(f).delimiter(' ');

            int id = 1;
            for (auto &row : parser) {
                int attribute = 0;
                vector<bool> zipfAttributes = gen.GenZipfAtts(maxAtts,alpha,number_of_constraints);
                IntervalSub sub;
                sub.id = id, sub.size = 0;
                for (auto &field : row) {
                    if (zipfAttributes[attribute]) {
                        IntervalCnt interval;

                        interval.att = attribute;
                        double stdDevTimeWidth  = width * standard_deviations[attribute];
                        interval.highValue      = Util::safeValue(100 * (stof(field) + (stdDevTimeWidth/2.0)) +5000);
                        interval.lowValue       = Util::safeValue(100 * (stoi(field) - (stdDevTimeWidth/2.0)) +5000);

                        sub.constraints.push_back(interval);
                        ++sub.size;
                    }
                    ++attribute;
                }
                subscriptions.push_back(sub);
                ++id;
            }
            vector<string> algos = j["algos"];



            for (auto constraint : subscriptions[0].constraints) {
                cout << constraint.att << ": (" << constraint.lowValue << ", " << constraint.highValue << "), ";
            }
            cout << "Size:" << subscriptions[0].size << endl;

            const vector<Pub> &const_events = events;
            const vector<IntervalSub> &const_subscriptions = subscriptions;
            vector<double> constraintsAmount;

            for (auto sub : subscriptions) constraintsAmount.push_back(sub.size);

            evaluate(Util::mean(events_attributes),            // Number of attributes in total.
                     Util::mean(constraintsAmount),           // Number of constraints(predicates) in one sub.
                     Util::mean(events_attributes),            // Number of values in one pub.
                     width,                                    // Interval width
                     distance_from_mean,
                     alpha,
                     MAX_CARDINALITY,
                     0,
                     algos,
                     const_events,
                     const_subscriptions,
                     outputFileName
            );
        }
    }
}

int main(int argc, char **argv) {
    json modeConfig, config;
    {
        std::ifstream i("./config/config.json");
        i >> modeConfig;
    }
    string mode = modeConfig["mode"];

    if (mode.compare("synthetic") != 0 && mode.compare("augmentation") != 0) {
        cout << "Mode must be 'synthetic' or 'augmentation', but got: '" << mode << "'." << endl;
        std::perror("No such mode. Exiting.");
        return 1;
    }

    std::time_t timestamp = std::time(nullptr);
    string outputFileName = "./output/" + std::to_string(timestamp) + ".csv";
    string configCopyFileName = "./output/" + std::to_string(timestamp) + ".json";

    // Copy config.
    std::ifstream i("./config/"+mode+".json");
    i >> config;
    std::ofstream  dst(configCopyFileName, std::ios::binary);
    i.clear();
    i.seekg(0);
    dst << i.rdbuf();
    dst.close();

    // Prep csv
    string header = "Algorithm,Algorithm parameter,Subscriptions,Dimensions,Subscription constraints, Alpha, Event pairs,Width,Distance from mean,Mean insert subscription time,Mean event match time,Mean number of matched subscriptions";
    Util::WriteData(outputFileName.c_str(), header);

    if (mode.compare("synthetic") == 0){
        evaluateUsingSyntheticData(config, outputFileName);
    } else if (mode.compare("augmentation") == 0){
        evaluateUsingDataAugmentation(config, "data/augmented/", outputFileName);
    }

    return 0;

}


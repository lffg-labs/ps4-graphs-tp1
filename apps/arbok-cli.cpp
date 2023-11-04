
#include <iostream>
#include <fstream>
#include <random>
#include <chrono>
#include <queue>
#include <map>

// #include <arbok/utils/paths.h>
#include "algos/utils/utils.h" // ok
#include "algos/gabow/gabow.h" // ok
// #include <arbok/others/lemon.h>
// #include <arbok/others/atofigh.h>
#include "algos/atofigh/atofigh.h"
// #include <arbok/others/felerius.h>
#include "algos/tarjan/felerius.h"
// #include <arbok/others/spaghetti.h>
// #include <arbok/others/yosupo.h>
// #include <arbok/tarjan/tarjan_set.h>
// #include <arbok/tarjan/tarjan_pq.h>
// #include <arbok/tarjan/tarjan_matrix.h>
// #include <arbok/tarjan/tarjan_treap.h>
// #include <arbok/tarjan/tarjan_hollow.h>

#ifdef __linux__
#include <sys/resource.h>
#endif

using namespace std;

struct Timer {
    map<string, chrono::steady_clock::time_point> begin;
    string last;
    void start(const string& desc) { last = desc; begin[desc] = chrono::steady_clock::now(); }
    auto stop(const string& desc) {
        auto end = chrono::steady_clock::now();
        auto dur = chrono::duration_cast<chrono::milliseconds>(end-begin[desc]).count();
        cout << desc << " finished in " << dur << " ms" << endl;
        return dur;
    }
    auto stop() { return stop(last); }
};
Timer t;

map<string, string> parseArgs(int argc, char** argv) {
    cout << "reading arguments (" << argc << ")...\n";
    map<string, string> params;
    for (int i = 1; i < argc; i++) {
        // Get current and next argument
        if (argv[i][0] != '-')
            continue;
        std::string arg = argv[i] + 1; // +1 to skip the -
        // advance one additional position if next is used
        std::string next = (i + 1 < argc ? argv[i++ + 1] : "");
        cout << "got arg k=(" << arg << "), v=(" << next << ")\n";
        params[std::move(arg)] = std::move(next);
    }
    cout << "  done\n";
    return params;
}

template<class Algo>
void run(map<string, string>& args) {

    t.start("load graph");
    auto graph = arbok::fromFile(args["input"]);
    t.stop("load graph");

    cout << "n      =" << graph.n << endl;
    cout << "m      =" << size(graph.edges) << endl;

    if(args["giantCC"]=="1") {
        t.start("find giant cc");
        graph = giantCC(graph);
        t.stop("find giant cc");

        cout << "n      =" << graph.n << endl;
        cout << "m      =" << size(graph.edges) << endl;
    }

    if(!graph.weighted) {
        t.start("add random weights");
        mt19937 gen(1337);
        uniform_int_distribution dist(1, 20);
        graph.weighted = true;
        for(auto& [u,v,w] : graph.edges)
            w = dist(gen);
        t.stop("add random weights");
    } else {
        t.start("check weights"); // this is done so that INF does not overflow int
        int mn_weight = 0;
        for(auto& [u,v,w] : graph.edges)
            mn_weight = min(mn_weight,w);
        if(mn_weight<0) {
            long long mx_weight = 0;
            for(auto& [u,v,w] : graph.edges) {
                w -= mn_weight;
                mx_weight = max<long long>(mx_weight, w);
            }
            cout << "WARNING: found negative weights. scaling up to be positive; new max weight is " << mx_weight << endl;
        }
        t.stop("check weights");
    }

    const int INF = 1e9;

    int root = graph.n;
    if(args["root"].empty()) {
        t.start("add supernode");
        for (int i = 0; i < graph.n; ++i)
            graph.edges.push_back({root, i, INF});
        graph.n++;
        t.stop("add supernode");
    } else {
        root = stoi(args["root"]);
    }

    long con, run, rec;
    long long res;
    vector<int> arbo;
    t.start("total");
    {
        t.start("construct algo DS");
        Algo alg(graph.n, (int)graph.edges.size());
        for (auto e: graph.edges) alg.create_edge(e.from, e.to, e.weight);
        con = t.stop();

        t.start("run algo");
        res = alg.run(root);
        run = t.stop();

        t.start("reconstruct");
        arbo = alg.reconstruct(root);
        rec = t.stop();

        t.start("destroy");
    }
    auto del = t.stop();
    t.stop("total");

    if(args["check"]!="0") {
        t.start("validate");
        if(!arbok::isArborescence(graph, arbo, res, root))
            cout << "output no valid arborescence" << endl, exit(1);
        t.stop();
    }

    if(!empty(args["csv"])) {
        ofstream ouf(args["csv"], ios::app);
        ouf << args["input"]
            << ',' << graph.n
            << ',' << size(graph.edges)
            << ',' << graph.weighted
            << ',' << res
            << ',' << args["algo"]
            << ',' << con
            << ',' << run
            << ',' << rec
            << ',' << del
            << endl;
    }

    cout << "n      =" << graph.n << endl;
    cout << "m      =" << size(graph.edges) << endl;
    cout << "w      =" << res << endl;
    cout << "w%1e9  =" << res%int(1e9) << endl;
    cout << "w/1e9  =" << res/1'000'000'000 << endl;

    if(!empty(args["outfile"])) {
        ofstream out(args["outfile"]);
        for(auto e : arbo) out << e << '\n';
    }
}

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);

#ifdef __linux__
    rlimit rl;
    getrlimit(RLIMIT_STACK,&rl);
    rl.rlim_cur = 4 * 1024L * 1024L * 1024L; // 4GB
    setrlimit(RLIMIT_STACK,&rl);
#else
    cout << "WARNING: on non-linux targets stack size might be too small for yosupo solver on very large instances (>10kk edges)" << endl;
#endif

    map<string,string> defaults{
            {"input",   ""},
            {"csv",     ""},
            {"algo",    "pq"},
            {"root",    ""},
            {"giantCC", "0"},
            {"check",   "1"},
            {"outfile",   ""},
    };
    auto args = parseArgs(argc, argv);
    cout << "PARAMETER: " << endl;
    for(auto [key,val] : defaults) {
        if(args.contains(key)) continue;
        cout << key << " not given, default assumed (" << val << ")" << endl;
        args[key] = val;
    }
    for(auto [key,val] : args) {
        if(!defaults.contains(key)) cerr << "ERROR: unrecognized param: " << key, exit(1);
        cout << '\t' << key << ": " << val << endl;
    }

    auto algo = args["algo"];
    if(algo=="invalid") cerr << "invalid algo: " << algo, exit(1);
    // if(algo=="set") run<arbok::TarjanSet>(args);
    // else if(algo=="matrix") run<arbok::TarjanMatrix>(args);
    // else if(algo=="pq") run<arbok::TarjanPQ>(args);
    // else if(algo=="treap") run<arbok::TarjanTreap>(args);
    // else if(algo=="hollow") run<arbok::TarjanHollow>(args);
    else if(algo=="gabow") run<arbok::Gabow>(args);
    // else if(algo=="lemon") run<arbok::Lemon>(args);
    else if(algo=="atofigh") run<arbok::Atofigh>(args);
    else if(algo=="felerius") run<arbok::Felerius>(args);
    // else if(algo=="spaghetti") run<arbok::Spaghetti>(args);
    // else if(algo=="yosupo") run<arbok::Yosupo>(args);
    else cerr << "invalid algo: " << algo, exit(1);

    return 0;
}


#include <iostream>

#include "nlohmann/json.hpp"
#include "tuft.hpp"

int main()
{
    using namespace std;

    cout << "Starting tuft tests..." << endl;

    using namespace std;
    using json = nlohmann::json;

    auto tmpl = "{{message}}\n{{#list}}\t<b>{{& name}}</b>\n{{/list}}";
    json hash;

    hash["message"] = "Current employees:";
    hash["list"].push_back({{"name", "Jared"}});
    hash["list"].push_back({{"name", "Mark"}});
    hash["list"].push_back({{"name", "Jeff"}});
    hash["list"].push_back({{"name", "<i>Cameron</i>"}});


    cout << hash.dump(4) << endl << endl;
    cout << tuft::render(tmpl, hash) << endl;
    cout << "Tests finished!" << endl;

    return 0;
}
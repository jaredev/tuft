#include <iostream>

#include "nlohmann/json.hpp"
#include "tuft.hpp"

int main()
{
    using namespace std;

    cout << "Starting tuft tests..." << endl;

    using namespace std;
    using json = nlohmann::json;

    auto tmpl = R"(
    {{message}}
    {{#employees}}
        Name: {{{name}}}
        Age: {{age}}
    {{/employees}}
    {{#numbers}}
        {{.}}
    {{/numbers}}
    )";

    json hash = json::parse(R"(
    {
        "message": "Employees",
        "numbers": [1, 2, 3, 4],

        "employees":
        [
            { "name": "<i>Jared</i>", "age": 26 },
            { "name": "Mark",  "age": 35 }
        ]
    })");

    cout << hash.dump(4) << endl << endl;
    cout << tuft::render(tmpl, hash) << endl;
    cout << "Tests finished!" << endl;

    return 0;
}
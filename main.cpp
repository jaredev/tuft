/*
 * Copyright 2016 Charles Jared Jetsel
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons
 * to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>

#include <cstdint>
#include "nlohmann/json.hpp"
#include "tuft.hpp"

int main()
{
    using namespace std;

    cout << "Starting tuft tests..." << endl;

    using namespace std;
    using json = nlohmann::json;

    auto tmpl = R"(
    <html>
    <h1>{{message}}</h1>
    <ul>
    {{#employees}}
    
        <li><strong>{{name}}</strong> is {{age}} years old</li>
    {{/employees}}
    </ul>
    <ul>{{#numbers}}
    <li>{{.}}</li>{{/numbers}}
    </ul>
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

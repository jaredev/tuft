# TUFT
---

A mustache-like template rendering for modern C++. Tuft is header only and relies upon the [nlohmann/json](https://github.com/nlohmann/json) for the rendering context.

To use tuft, just call `tuft::render()` with the template string and json object:

```cpp    
    auto html_template = R"(
    <h1>{{message}}</h1>
    <ul>
    {{#people}}
        <li><strong>{{name}}</strong></li>
    {{/people}}
    </ul>
    )";

    auto hash = json::parse(R"(
    {
        "message": "Here are a list of names:",
        "people": [
            { "name": "Albert" },
            { "name": "Bernard" },
            { "name": "Catheline" },
            { "name": "Daniel" },
            { "name": "Elanor" }
        ]
    })");

    auto rendered = tuft::render(html_template, hash);
```

## Features

### Supported
* Mustache-like interpolation
* HTML escaping
* Triple mustache and ampersand
* Basic context scope
* Implicit iterators
* Global custom delimiters
* Truthy and falsey sections rendering
* Falsey null
* Surroundings and outlying whitespace
* Pair with padding

### Not supported
* Partials
* Changing custom delimiters in a template
* Dotted names, parented context
* Comments
* Standalone tags and line endings
* Partial inheritance, post-partial behaviors
* Superfluous in tag whitespace
* Lambda functions



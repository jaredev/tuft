//
// Created by jaredev on 7/14/16.
//

#pragma once

#include <string>
#include <algorithm>

namespace tuft
{
    /** @brief  String containing mustache template */
    using template_t = std::string;

    using json_t     = nlohmann::json;

    /**
     * options_t
     *
     * @brief   Holds configurable options used to render the template
     */
    struct options_t
    {
        /** Opening delimiter. Default is "{{" */
        std::string delim_open;

        /** Closing delimiter. Default is "}}" */
        std::string delim_close;
    };

    /** @brief  Default delimiters and options for mustache rendering */
    const options_t default_options{"{{", "}}"};

    /** @brief  Exception type that is thrown from tuft */
    struct exception : public std::runtime_error
    {
        explicit exception(const std::string& what_arg) : std::runtime_error(what_arg) {};
        explicit exception(const char* what_arg) : std::runtime_error(what_arg) {};
    };

    /**
     * render
     * @brief   Renders hash/json values into mustache template
     *
     * @param   templ   Mustache template string
     * @param   hash    JSON object
     * @param   options Configuration
     * @return
     */
    std::string render(const template_t & templ, const json_t & hash, const options_t options = default_options);

    namespace detail
    {
        using iter = std::string::const_iterator;

        /** @brief  Enumeration of symbol characters that represent a tag type */
        enum class tag_type : char
        {
            /** Normal tag mustache variable */
            variable = 0,

            /** Escaped variable tag */
            escaped = '&',

            /** Section tag beginning with '#' */
            section = '#',

            /** Section tag beginning with '^' */
            inverted_section = '^',

            /** Section close tag beginning with '/' */
            end_section = '/',

            /** Section beginning with '!' denoting a comment block */
            comment = '!',

            invalid = 0x0F,
        };

        const std::string tag_type_symbols("&#^/!");
        const std::string mustaches("{ }");

        /**
         * render_next
         *
         * @brief Renders the json elements and its descendants into the current range of the template
         *
         * @param begin         String iterator to the beginning of the remainder of the template string or current section
         * @param end           String iterator to the end of the current section
         * @param current_elem  Current json element that is being rendered
         *
         * @note  This is a recursive function.
         */
        void render_next(const template_t& t, const iter& begin, const iter& end, std::string& rendered, const json_t& current_elem, const options_t& opts);
    }

    std::string render(const template_t & t, const json_t & hash, options_t options)
    {
        using namespace std;

        string rendered;

        if (t.size() == 0)
            return rendered;

        rendered.reserve(t.size()); // approx. tag names are removed so it should be fairly close

        detail::render_next(t, t.begin(), t.end(), rendered, hash, options);

        return rendered;
    }

    namespace detail
    {
        /**
         * find_next_tag
         *
         * @param   b   Beginning of range to search for tag
         * @param   e   End of range to search for tag
         *
         * @param   tag_begin  Out: Iterator to the beginning of the tag before delimiters
         * @param   tag_end    Out: Iterator to the end of the tag after delimiters
         *
         * @return  true if next tag/section is found
         *
         * @note    Output iters will be equal to e if not found.
         */
        bool find_next_tag(const iter& b, const iter& e, iter& tag_begin, iter& tag_end, const options_t& opts)
        {
            using namespace std;

            tag_begin = e;
            tag_end = e;
            tag_begin = search(b, e, opts.delim_open.begin(), opts.delim_open.end());

            string delim_close = opts.delim_close;

            // Special case for triple mustache escape.
            if (opts.delim_open == "{{" && opts.delim_close == "}}")
            {
                string triple_open = "{{{";

                auto triple_begin  = search(b, e, triple_open.begin(), triple_open.end());

                if (tag_begin == triple_begin)
                    delim_close = "}}}";
            }

            if (tag_begin != e)
            {
                auto after_tag_begin = next(tag_begin, opts.delim_open.size());

                tag_end = search(after_tag_begin, e, delim_close.begin(), delim_close.end());

                if (tag_end != e)
                    tag_end = next(tag_end, delim_close.size()); // move after end delimiter
            }

            return (tag_begin != tag_end);
        }

        /**
         * inside_tag
         *
         * @param b Iterator pointing to beginning of tag before starting delimiter
         * @param e Iterator pointing to end of tag after ending delimiter
         *
         * @return  Pair of begin/end iterators pointing to the interior of the tag between delimiters
         */
        std::pair<iter, iter> inside_tag(const iter& b, const iter& e, const options_t& opts)
        {
            auto inside_begin = std::next(b,  opts.delim_open.size());  // after  "{{"
            auto inside_end   = std::next(e, -opts.delim_close.size()); // before "}}"

            return std::make_pair(inside_begin, inside_end);
        }

        /**
         * tag_name
         *
         * @brief   Remove symbols and spaces from tag iterior to get name
         *
         * @param b Iterator pointing to beginning of tag before starting delimiter
         * @param e Iterator pointing to end of tag after ending delimiter
         *
         * @return  variable/section name of tag
         */
        std::string get_tag_name(const iter& b, const iter& e, const options_t& opts)
        {
            using namespace std;

            if (b == e)
                return "";

            auto inside = inside_tag(b, e, opts);
            string name(inside.first, inside.second);

            auto name_size = name.length();

            name.erase(remove_if(name.begin(), name.end(), [](const char& x)
                    {
                        return (tag_type_symbols.find(x) != string::npos) || (x == '{') || (x == '}');
                    }),
                    name.begin());

            return name;
        }

        /**
         * get_tag_type
         *
         * @param b Iterator pointing to beginning of tag before starting delimiter
         * @param e Iterator pointing to end of tag after ending delimiter
         *
         * @return  Tag type enum value
         *
         * @note    Assumes that the mustache symbol is the first character in the tag (after whitespace is removed)
         */
        tag_type get_tag_type(const iter& b, const iter& e, const options_t& opts)
        {
            auto inside = inside_tag(b, e, opts);

            tag_type tag(tag_type::variable);

            if (inside.first == inside.second) // empty tag
                return tag;

            for (auto it = inside.first; it != inside.second; ++it)
            {
                if (tag_type_symbols.find(*it) != std::string::npos)
                {
                    tag = static_cast<tag_type>(*it);
                    break;
                }
            }

            return tag;
        }

        /**
         * should_escape
         *
         * @param b Iterator pointing to beginning of tag before starting delimiter
         * @param e Iterator pointing to end of tag after ending delimiter
         *
         * @return  True if the tag's contents should be escaped of all special html chars
         */
        bool should_escape(const iter& b, const iter& e, const options_t& opts)
        {
            using namespace std;

            bool escape = true;

            if (get_tag_type(b, e, opts) == tag_type::escaped)
            {
                escape = false;
            }
            else if (distance(b, e) >= 6)
            {
                // Special case for triple mustache. Inner mustaches are ignored if delim is not default.
                string test(b, e);

                string first_three(b, next(b, 3));
                string last_three(next(e, -3), e);

                if (first_three == "{{{" && last_three == "}}}")
                {
                    escape = false;
                }
            }

            return escape;
        }

        /**
         * escape_html
         *
         * @return  String with special html characters escaped.
         */
        std::string escape_html(const std::string& html)
        {
            std::string escaped;
            escaped.reserve(html.size());

            for (auto it = html.begin(); it != html.end(); ++it)
            {
                switch (*it)
                {
                case '&':  escaped += "&amp;";  break;
                case '<':  escaped += "&lt;";   break;
                case '>':  escaped += "&gt;";   break;
                case '"':  escaped += "&quot;"; break;
                case '\'': escaped += "&#39;";  break;
                case '/':  escaped += "&#x2F;"; break;
                default:   escaped += *it;      break;
                }
            }

            return escaped;
        }

        void render_section(const template_t& t, const iter& begin, const iter& end, std::string& rendered, const json_t& current_elem, const options_t& opts, bool is_inverted)
        {
            bool render_interior = false;

            switch (current_elem.type())
            {
                case json_t::value_t::array:
                case json_t::value_t::object:
                    render_interior = true;
                    break;

                case json_t::value_t::boolean:
                    render_interior = current_elem.get<bool>();
                    break;

                case json_t::value_t::discarded:
                case json_t::value_t::null:
                case json_t::value_t::number_float:
                case json_t::value_t::number_integer:
                case json_t::value_t::number_unsigned:
                default:
                    break;
            }

            if (is_inverted)
            {
                render_interior = !render_interior;
            }

            if (render_interior)
            {
                render_next(t, begin, end, rendered, current_elem, opts);
            }
        }

        void render_next(const template_t& t, const iter& begin, const iter& end, std::string& rendered, const json_t& element, const options_t& opts)
        {
            using namespace std;

            // If it is an array then we'll need to loop through each element
            bool   is_array   = element.is_array();
            size_t loop_count = 1;

            if (is_array)
                loop_count = element.size();

            for (size_t i = 0; i < loop_count; ++i)
            {
                iter remaining_begin = begin;
                iter tag_begin  = end; // Before "{{"
                iter tag_end    = end; // After  "}}"

                const json_t& current_elem = is_array ? element[i] : element;

                while (find_next_tag(remaining_begin, end, tag_begin, tag_end, opts))
                {
                    rendered.append(remaining_begin, tag_begin); // This is the stuff between tags. Leave it alone.

                    auto name = get_tag_name(tag_begin, tag_end, opts);
                    bool in_array = name == ".";

                    bool is_inverted_section = false;

                    tag_type type = get_tag_type(tag_begin, tag_end, opts);

                    switch (type)
                    {
                        case tag_type::variable:
                        case tag_type::escaped:
                        {
                            bool found = current_elem.count(name) > 0;
                            json_t elem = found ? current_elem[name] : current_elem;

                            string val;

                            if (elem.is_object() || elem.is_array())
                            {
                                val = elem.dump();
                            }
                            else if (elem.is_null())
                            {
                                val = "null";
                            }
                            else
                            {
                                val = elem; // implicit conversion
                            }

                            if (should_escape(tag_begin, tag_end, opts))
                                rendered += escape_html(val);
                            else
                                rendered += val;

                            break;
                        }
                        // Fall through sections
                        case tag_type::inverted_section:
                            is_inverted_section = true;

                        case tag_type::section:
                        {
                            auto close_section_tag = opts.delim_open + "/" + name + opts.delim_close;
                            auto close_tag_begin   = search(tag_end, end, close_section_tag.begin(), close_section_tag.end());

                            if (close_tag_begin == end)
                                throw exception("tuft::render - Could not find closing tag '" + close_section_tag + "'");

                            render_section(t, tag_end, close_tag_begin, rendered, current_elem[name], opts, is_inverted_section);

                            // Move after section's closing tag for next round
                            tag_end = next(close_tag_begin, close_section_tag.size());
                            break;
                        }

                        case tag_type::comment:
                            // Comments aren't altered
                            rendered.append(tag_begin, next(tag_end, opts.delim_close.size()));
                            break;

                        // Fall through bad tags
                        case tag_type::invalid:
                        default:
                            throw exception("tuft::render - Unknown tag: '" + string(tag_begin, tag_end));
                            break;
                    }

                    // Move past current tag.
                    remaining_begin = tag_end;
                }

                // Append anything remaining after tag. In a section this would be the remainder of the section interior
                rendered.append(remaining_begin, end);
            }
        }

    } // detail
} // tuft
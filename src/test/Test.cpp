//
// Created by hans on 18-6-2.
//

#include "Test.h"

void Test::test() {
    cout << "it sucks" << endl;
    GumboOutput *output = gumbo_parse("<h1>Hello, World!</h1>");
    // Do stuff with output->root
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    cout << "well... it works" << endl;
}

void Test::clean_text() {
    const char *filename = "/home/hans/CLionProjects/Novel/novel/novel_yes_html/1.html";
    ifstream in(filename, ios::in | ios::binary);
    if (!in) {
        cout << "File " << filename << " not found!\n";
        exit(EXIT_FAILURE);
    }

    string contents;
    in.seekg(0, ios::end);
    contents.resize(in.tellg());
    in.seekg(0, ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    GumboOutput *output = gumbo_parse(contents.c_str());
    cout << clean_text_helper(output->root) << endl;
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

void Test::find_links() {
    const char *filename = "/home/hans/CLionProjects/Novel/novel/novel_yes_html/1.html";

    ifstream in(filename, ios::in | ios::binary);
    if (!in) {
        cout << "File " << filename << " not found!\n";
        exit(EXIT_FAILURE);
    }

    string contents;
    in.seekg(0, ios::end);
    contents.resize(in.tellg());
    in.seekg(0, ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    GumboOutput *output = gumbo_parse(contents.c_str());
    search_for_links(output->root);
    gumbo_destroy_output(&kGumboDefaultOptions, output);

}

void Test::get_title() {
    const char *filename = "/home/hans/CLionProjects/Novel/novel/novel_yes_html/1.html";

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("File %s not found!\n", filename);
        exit(EXIT_FAILURE);
    }

    char *input;
    int input_length;
    read_file(fp, &input, &input_length);
    GumboOutput *output = gumbo_parse_with_options(
            &kGumboDefaultOptions, input, input_length);
    const char *title = find_title(output->root);
    printf("%s\n", title);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
    free(input);
}

void Test::positions_of_class() {
    const char *filename = "/home/hans/CLionProjects/Novel/novel/novel_yes_html/1.html";

    const char *cls = "DivMain";

    ifstream in(filename, ios::in | ios::binary);
    if (!in) {
        cout << "File " << filename << " not found!\n";
        exit(EXIT_FAILURE);
    }

    string contents;
    in.seekg(0, ios::end);
    contents.resize(in.tellg());
    in.seekg(0, ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    // If you used contents.c_str(), it'd be harder to match up original
    // positions, because c_str() creates a copy of the string and you can't do
    // pointer arithmetic betweent contents.data() and the original_* pointers.
    GumboOutput *output = gumbo_parse_with_options(
            &kGumboDefaultOptions, contents.data(), contents.length());
    search_for_class(output->root, contents, cls);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

void Test::pretty_print() {
    const char *filename = "/home/hans/CLionProjects/Novel/novel/novel_yes_html/1.html";

    ifstream in(filename, ios::in | ios::binary);
    if (!in) {
        cout << "File " << filename << " not found!\n";
        exit(EXIT_FAILURE);
    }

    string contents;
    in.seekg(0, ios::end);
    contents.resize(in.tellg());
    in.seekg(0, ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    GumboOptions options = kGumboDefaultOptions;

    GumboOutput *output = gumbo_parse_with_options(&options, contents.data(), contents.length());
    string indent_chars = "  ";
    cout << pretty_print(output->document, 0, indent_chars) << endl;
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}

void Test::serialize() {

    const char *filename = "/home/hans/CLionProjects/Novel/novel/novel_yes_html/1.html";

    ifstream in(filename, ios::in | ios::binary);
    if (!in) {
        cout << "File " << filename << " not found!\n";
        exit(EXIT_FAILURE);
    }

    string contents;
    in.seekg(0, ios::end);
    contents.resize(in.tellg());
    in.seekg(0, ios::beg);
    in.read(&contents[0], contents.size());
    in.close();

    GumboOptions options = kGumboDefaultOptions;

    GumboOutput *output = gumbo_parse_with_options(&options, contents.data(), contents.length());
    cout << serialize(output->document) << endl;
    gumbo_destroy_output(&kGumboDefaultOptions, output);

}

string Test::clean_text_helper(GumboNode *node) {
    if (node->type == GUMBO_NODE_TEXT) {
        return string(node->v.text.text);
    } else if (node->type == GUMBO_NODE_ELEMENT &&
               node->v.element.tag != GUMBO_TAG_SCRIPT &&
               node->v.element.tag != GUMBO_TAG_STYLE) {
        string contents = "";
        GumboVector *children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            const string text = clean_text_helper((GumboNode *) children->data[i]);
            if (i != 0 && !text.empty()) {
                contents.append(" ");
            }
            contents.append(text);
        }
        return contents;
    } else {
        return "";
    }
}

void Test::search_for_links(GumboNode *node) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    GumboAttribute *href;
    if (node->v.element.tag == GUMBO_TAG_A &&
        (href = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        cout << href->value << endl;
    }

    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        search_for_links(static_cast<GumboNode *>(children->data[i]));
    }
}

void Test::read_file(FILE *fp, char **output, int *length) {
    struct stat filestats;
    int fd = fileno(fp);
    fstat(fd, &filestats);
    *length = static_cast<int>(filestats.st_size);
    *output = static_cast<char *>(malloc(*length + 1));
    int start = 0;
    int bytes_read;
    while ((bytes_read = fread(*output + start, 1, *length - start, fp))) {
        start += bytes_read;
    }
}

const char *Test::find_title(const GumboNode *root) {
    assert(root->type == GUMBO_NODE_ELEMENT);
    assert(root->v.element.children.length >= 2);

    const GumboVector *root_children = &root->v.element.children;
    GumboNode *head = NULL;
    for (int i = 0; i < root_children->length; ++i) {
        GumboNode *child = static_cast<GumboNode *>(root_children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT &&
            child->v.element.tag == GUMBO_TAG_HEAD) {
            head = child;
            break;
        }
    }
    assert(head != NULL);

    GumboVector *head_children = &head->v.element.children;
    for (int i = 0; i < head_children->length; ++i) {
        GumboNode *child = static_cast<GumboNode *>(head_children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT &&
            child->v.element.tag == GUMBO_TAG_TITLE) {
            if (child->v.element.children.length != 1) {
                return "<empty title>";
            }
            GumboNode *title_text = static_cast<GumboNode *>(child->v.element.children.data[0]);
            assert(title_text->type == GUMBO_NODE_TEXT || title_text->type == GUMBO_NODE_WHITESPACE);
            return title_text->v.text.text;
        }
    }
    return "<no title found>";
}

string Test::find_line(const string &original_text, const GumboAttribute &attr) {
    size_t attr_index = attr.original_value.data - original_text.data();
    size_t begin = original_text.rfind("\n", attr_index) + 1;
    size_t end = original_text.find("\n", attr_index);
    if (end != string::npos) {
        end--;
    } else {
        end = (size_t) original_text.length() - 1;
    }
    end = min(end, attr_index + 40);
    begin = max(begin, attr_index - 40);
    return original_text.substr(begin, end - begin);
}

void Test::search_for_class(GumboNode *node, const string &original_text, const char *cls_name) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    GumboAttribute *cls_attr;
    if ((cls_attr = gumbo_get_attribute(&node->v.element.attributes, "class")) &&
        strstr(cls_attr->value, cls_name) != NULL) {
        cout << cls_attr->value_start.line << ":"
             << cls_attr->value_start.column << " - "
             << find_line(original_text, *cls_attr) << endl;
    }

    GumboVector *children = &node->v.element.children;
    for (int i = 0; i < children->length; ++i) {
        search_for_class(
                static_cast<GumboNode *>(children->data[i]), original_text, cls_name);
    }
}

void Test::replace_all(string &s, const char *s1, const char *s2) {
    string t1(s1);
    size_t len = t1.length();
    size_t pos = s.find(t1);
    while (pos != string::npos) {
        s.replace(pos, len, s2);
        pos = s.find(t1, pos + len);
    }
}

string Test::substitute_xml_entities_into_text(const string &text) {
    string result = text;
    // replacing & must come first 
    replace_all(result, "&", "&amp;");
    replace_all(result, "<", "&lt;");
    replace_all(result, ">", "&gt;");
    return result;
}

string Test::substitute_xml_entities_into_attributes(char quote, const string &text) {
    string result = substitute_xml_entities_into_text(text);
    if (quote == '"') {
        replace_all(result, "\"", "&quot;");
    } else if (quote == '\'') {
        replace_all(result, "'", "&apos;");
    }
    return result;
}

string Test::handle_unknown_tag(GumboStringPiece *text) {
    string tagname = "";
    if (text->data == NULL) {
        return tagname;
    }
    // work with copy GumboStringPiece to prevent asserts 
    // if try to read same unknown tag name more than once
    GumboStringPiece gsp = *text;
    gumbo_tag_from_original_text(&gsp);
    tagname = string(gsp.data, gsp.length);
    return tagname;
}

string Test::get_tag_name(GumboNode *node) {
    string tagname;
    // work around lack of proper name for document node
    if (node->type == GUMBO_NODE_DOCUMENT) {
        tagname = "document";
    } else {
        tagname = gumbo_normalized_tagname(node->v.element.tag);
    }
    if (tagname.empty()) {
        tagname = handle_unknown_tag(&node->v.element.original_tag);
    }
    return tagname;
}

string Test::prettyprint_contents(GumboNode *node, int lvl, const string indent_chars) {

    string contents = "";
    string tagname = get_tag_name(node);
    string key = "|" + tagname + "|";
    bool no_entity_substitution = no_entity_sub.find(key) != string::npos;
    bool keep_whitespace = preserve_whitespace.find(key) != string::npos;
    bool is_inline = nonbreaking_inline.find(key) != string::npos;
    bool pp_okay = !is_inline && !keep_whitespace;

    GumboVector *children = &node->v.element.children;

    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode *child = static_cast<GumboNode *> (children->data[i]);

        if (child->type == GUMBO_NODE_TEXT) {

            string val;

            if (no_entity_substitution) {
                val = string(child->v.text.text);
            } else {
                val = substitute_xml_entities_into_text(string(child->v.text.text));
            }

            if (pp_okay) rtrim(val);

            if (pp_okay && (contents.length() == 0)) {
                // add required indentation
                char c = indent_chars.at(0);
                int n = indent_chars.length();
                contents.append(string((lvl - 1) * n, c));
            }

            contents.append(val);


        } else if ((child->type == GUMBO_NODE_ELEMENT) || (child->type == GUMBO_NODE_TEMPLATE)) {

            string val = pretty_print(child, lvl, indent_chars);

            // remove any indentation if this child is inline and not first child
            string childname = get_tag_name(child);
            string childkey = "|" + childname + "|";
            if ((nonbreaking_inline.find(childkey) != string::npos) && (contents.length() > 0)) {
                ltrim(val);
            }

            contents.append(val);

        } else if (child->type == GUMBO_NODE_WHITESPACE) {

            if (keep_whitespace || is_inline) {
                contents.append(string(child->v.text.text));
            }

        } else if (child->type != GUMBO_NODE_COMMENT) {

            // Does this actually exist: (child->type == GUMBO_NODE_CDATA)
            fprintf(stderr, "unknown element of type: %d\n", child->type);

        }

    }

    return contents;
}

string Test::pretty_print(GumboNode *node, int lvl, const string indent_chars) {

    // special case the document node
    if (node->type == GUMBO_NODE_DOCUMENT) {
        string results = build_doctype(node);
        results.append(prettyprint_contents(node, lvl + 1, indent_chars));
        return results;
    }

    string close = "";
    string closeTag = "";
    string atts = "";
    string tagname = get_tag_name(node);
    string key = "|" + tagname + "|";
    bool need_special_handling = special_handling.find(key) != string::npos;
    bool is_empty_tag = empty_tags.find(key) != string::npos;
    bool no_entity_substitution = no_entity_sub.find(key) != string::npos;
    bool keep_whitespace = preserve_whitespace.find(key) != string::npos;
    bool is_inline = nonbreaking_inline.find(key) != string::npos;
    bool inline_like = treat_like_inline.find(key) != string::npos;
    bool pp_okay = !is_inline && !keep_whitespace;
    char c = indent_chars.at(0);
    int n = indent_chars.length();

    // build attr string
    const GumboVector *attribs = &node->v.element.attributes;
    for (int i = 0; i < attribs->length; ++i) {
        GumboAttribute *at = static_cast<GumboAttribute *>(attribs->data[i]);
        atts.append(build_attributes(at, no_entity_substitution));
    }

    // determine closing tag type
    if (is_empty_tag) {
        close = "/";
    } else {
        closeTag = "</" + tagname + ">";
    }

    string indent_space = string((lvl - 1) * n, c);

    // pretty_print your contents 
    string contents = prettyprint_contents(node, lvl + 1, indent_chars);

    if (need_special_handling) {
        rtrim(contents);
        contents.append("\n");
    }

    char last_char = ' ';
    if (!contents.empty()) {
        last_char = contents.at(contents.length() - 1);
    }

    // build results
    string results;
    if (pp_okay) {
        results.append(indent_space);
    }
    results.append("<" + tagname + atts + close + ">");
    if (pp_okay && !inline_like) {
        results.append("\n");
    }
    if (inline_like) {
        ltrim(contents);
    }
    results.append(contents);
    if (pp_okay && !contents.empty() && (last_char != '\n') && (!inline_like)) {
        results.append("\n");
    }
    if (pp_okay && !inline_like && !closeTag.empty()) {
        results.append(indent_space);
    }
    results.append(closeTag);
    if (pp_okay && !closeTag.empty()) {
        results.append("\n");
    }

    return results;
}

string Test::build_attributes(GumboAttribute *at, bool no_entities) {
    string atts = "";
    atts.append(" ");
    atts.append(at->name);

    // how do we want to handle attributes with empty values
    // <input type="checkbox" checked />  or <input type="checkbox" checked="" /> 

    if ((!string(at->value).empty()) ||
        (at->original_value.data[0] == '"') ||
        (at->original_value.data[0] == '\'')) {

        // determine original quote character used if it exists
        char quote = at->original_value.data[0];
        string qs = "";
        if (quote == '\'') qs = string("'");
        if (quote == '"') qs = string("\"");

        atts.append("=");

        atts.append(qs);

        if (no_entities) {
            atts.append(at->value);
        } else {
            atts.append(substitute_xml_entities_into_attributes(quote, string(at->value)));
        }

        atts.append(qs);
    }
    return atts;
}

string Test::build_doctype(GumboNode *node) {
    string results = "";
    if (node->v.document.has_doctype) {
        results.append("<!DOCTYPE ");
        results.append(node->v.document.name);
        string pi(node->v.document.public_identifier);
        if ((node->v.document.public_identifier != NULL) && !pi.empty()) {
            results.append(" PUBLIC \"");
            results.append(node->v.document.public_identifier);
            results.append("\" \"");
            results.append(node->v.document.system_identifier);
            results.append("\"");
        }
        results.append(">\n");
    }
    return results;
}

string Test::serialize(GumboNode *node) {
    // special case the document node
    if (node->type == GUMBO_NODE_DOCUMENT) {
        string results = build_doctype(node);
        results.append(serialize_contents(node));
        return results;
    }

    string close = "";
    string closeTag = "";
    string atts = "";
    string tagname = get_tag_name(node);
    string key = "|" + tagname + "|";
    bool need_special_handling = special_handling.find(key) != string::npos;
    bool is_empty_tag = empty_tags.find(key) != string::npos;
    bool no_entity_substitution = no_entity_sub.find(key) != string::npos;
    bool is_inline = nonbreaking_inline.find(key) != string::npos;

    // build attr string
    const GumboVector *attribs = &node->v.element.attributes;
    for (int i = 0; i < attribs->length; ++i) {
        GumboAttribute *at = static_cast<GumboAttribute *>(attribs->data[i]);
        atts.append(build_attributes(at, no_entity_substitution));
    }

    // determine closing tag type
    if (is_empty_tag) {
        close = "/";
    } else {
        closeTag = "</" + tagname + ">";
    }

    // serialize your contents
    string contents = serialize_contents(node);

    if (need_special_handling) {
        ltrim(contents);
        rtrim(contents);
        contents.append("\n");
    }

    // build results
    string results;
    results.append("<" + tagname + atts + close + ">");
    if (need_special_handling) results.append("\n");
    results.append(contents);
    results.append(closeTag);
    if (need_special_handling) results.append("\n");
    return results;
}

string Test::serialize_contents(GumboNode *node) {
    string contents = "";
    string tagname = get_tag_name(node);
    string key = "|" + tagname + "|";
    bool no_entity_substitution = no_entity_sub.find(key) != string::npos;
    bool keep_whitespace = preserve_whitespace.find(key) != string::npos;
    bool is_inline = nonbreaking_inline.find(key) != string::npos;

    // build up result for each child, recursively if need be
    GumboVector *children = &node->v.element.children;

    for (unsigned int i = 0; i < children->length; ++i) {
        GumboNode *child = static_cast<GumboNode *> (children->data[i]);

        if (child->type == GUMBO_NODE_TEXT) {
            if (no_entity_substitution) {
                contents.append(string(child->v.text.text));
            } else {
                contents.append(substitute_xml_entities_into_text(string(child->v.text.text)));
            }

        } else if (child->type == GUMBO_NODE_ELEMENT || child->type == GUMBO_NODE_TEMPLATE) {
            contents.append(serialize(child));

        } else if (child->type == GUMBO_NODE_WHITESPACE) {
            // keep all whitespace to keep as close to original as possible
            contents.append(string(child->v.text.text));

        } else if (child->type != GUMBO_NODE_COMMENT) {
            // Does this actually exist: (child->type == GUMBO_NODE_CDATA)
            fprintf(stderr, "unknown element of type: %d\n", child->type);
        }
    }
    return contents;
}

//
// Created by hans on 18-6-2.
//

#ifndef NOVEL_TEST_H
#define NOVEL_TEST_H

#include "../config.h"

class Test {
public:
    void test();

    void clean_text();

    void find_links();

    void get_title();

    void positions_of_class();

    void pretty_print();

    void serialize();

private:


    string nonbreaking_inline = "|a|abbr|acronym|b|bdo|big|cite|code|dfn|em|font|i|img|kbd|nobr|s|small|span|strike|strong|sub|sup|tt|";
    string empty_tags = "|area|base|basefont|bgsound|br|command|col|embed|event-source|frame|hr|image|img|input|keygen|link|menuitem|meta|param|source|spacer|track|wbr|";
    string preserve_whitespace = "|pre|textarea|script|style|";
    string special_handling = "|html|body|";
    string no_entity_sub = "|script|style|";
    string treat_like_inline = "|p|";

    string clean_text_helper(GumboNode *node);

    void search_for_links(GumboNode *node);

    void read_file(FILE *fp, char **output, int *length);

    const char *find_title(const GumboNode *root);

    string find_line(
            const string &original_text, const GumboAttribute &attr);

    void search_for_class(
            GumboNode *node, const string &original_text, const char *cls_name);

    inline void rtrim(string &s) {
        s.erase(s.find_last_not_of(" \n\r\t") + 1);
    }


    inline void ltrim(string &s) {
        s.erase(0, s.find_first_not_of(" \n\r\t"));
    }


    void replace_all(string &s, const char *s1, const char *s2);


    string substitute_xml_entities_into_text(const string &text);


    string substitute_xml_entities_into_attributes(char quote, const string &text);


    string handle_unknown_tag(GumboStringPiece *text);


    string get_tag_name(GumboNode *node);


    string build_doctype(GumboNode *node);


    string build_attributes(GumboAttribute *at, bool no_entities);

    // pretty_print children of a node
    // may be invoked recursively
    string prettyprint_contents(GumboNode *node, int lvl, const string indent_chars);


    // pretty_print a GumboNode back to html/xhtml
    // may be invoked recursively
    string pretty_print(GumboNode *node, int lvl, const string indent_chars);


    // serialize children of a node
    // may be invoked recursively
    string serialize_contents(GumboNode *node);


    // serialize a GumboNode back to html/xhtml
    // may be invoked recursively
    string serialize(GumboNode *node);


};


#endif //NOVEL_TEST_H

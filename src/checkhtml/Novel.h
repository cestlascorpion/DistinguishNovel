//
// Created by hans on 18-6-2.
//

#ifndef NOVEL_NOVEL_H
#define NOVEL_NOVEL_H

#include "../config.h"

class Novel {
public:
    Novel();

    void judgement();

private:
    void get_files(const string &path, vector<string> &files);

    void read_url(const string &file, vector<string> &res_url);

    double evaluate_from_url(const string &url);

    void evaluate_all_url(vector<string> &url, vector<double> &weight_of_url);

    string clean_text_helper(GumboNode *node);

    void get_clean_text(const string &res_html);

    double evaluate_from_clean_text(const string &text);

    double evaluate_from_title(const string &title);

    void evaluate_all_html(vector<string> &res_html, vector<double> &weight_of_html);

    void get_title(const string &html);

    void read_file(FILE *fp, char **output, int *length);

    const char *find_title(const GumboNode *root);

    static bool cmp(const string &str_a, const string &str_b);

    string m_data_dir;

    string m_filePath_yes;
    string m_filePath_no;

    string m_sample_yes;
    string m_sample_no;

    vector<string> m_yes_url;
    vector<string> m_no_url;
    vector<double> m_weight_of_yes_url;
    vector<double> m_weight_of_no_url;;

    string m_title_in_html;
    string m_clean_text;
    int m_min_text_value = 1024;

    vector<string> m_yes_html;
    vector<string> m_no_html;
    vector<double> m_weight_of_yes_html;
    vector<double> m_weight_of_no_html;

    double m_avg_weight_of_yes = 0.0;
    double m_avg_weight_of_no = 0.0;

    double m_TP = 0.0, m_FN = 0.0, m_FP = 0.0, m_TN = 0.0;

    struct some_param {
        double tp;
        double fn;
        double fp;
        double tn;

        some_param(double p1, double p2, double p3, double p4) { tp = p1, fn = p2, fp = p3, tn = p4; }
    };

    some_param m_some_param{0, 0, 0, 0};

    vector<pair<double, some_param >> m_sample_result;

    double m_maxPercision_at = 0.0;
    double m_maxRecall_at = 0.0;
    double m_maxPercision = 0.98;
    double m_maxRecall = 0.98;
    double m_percision = 0.0;
    double m_recall = 0.0;

    double m_threshold = 0.0;

    unordered_map<string, const double> url_match = {
            {"xiaoshuo", 0.25},
            {"book",     0.25},
            {"chapter",  0.15},
            {"txt",      0.15},
            {"kan",      0.10},
            {"shu",      0.10},
            {"novel",    0.10},
            {"read",     0.05},
            {"view",     0.05},
            {"article",  0.05},
            {"du",       0.05},
            {"blog",     -0.25},
            {"forum",    -0.25},
            {"shop",     -0.25},
            {"bbs",      -0.20},
            {"show",     -0.20},
            {"china",    -0.10},
            {"renren",   -0.10},
            {"baidu",    -0.10}
    };
};


#endif //NOVEL_NOVEL_H

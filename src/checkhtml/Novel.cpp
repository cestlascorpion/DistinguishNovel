//
// Created by hans on 18-6-2.
//

#include "Novel.h"

void Novel::get_files(const string &path, vector<string> &files) {
    if (!path.empty()) {
        struct dirent *ptr;
        DIR *dir;
        dir = opendir(path.c_str());
        while ((ptr = readdir(dir)) != nullptr) {
            if (ptr->d_name[0] == '.')
                continue;
            files.push_back(path + ptr->d_name);
        }
        closedir(dir);
        sort(files.begin(), files.end(), cmp);
        if (!files.empty()) {
            return;
        }
    }
}

void Novel::get_title(const string &html) {
    const char *filename = (char *) html.c_str();

    FILE *fp = fopen(filename, "r");
    if (!fp) {
        printf("File %s not found!\n", filename);
        exit(EXIT_FAILURE);
    }

    char *input;
    int input_length;
    read_file(fp, &input, &input_length);
    GumboOutput *output = gumbo_parse_with_options(&kGumboDefaultOptions, input, static_cast<size_t>(input_length));
    const char *title = find_title(output->root);

    m_title_in_html = title;

    gumbo_destroy_output(&kGumboDefaultOptions, output);
    free(input);
    fclose(fp);
}

void Novel::read_file(FILE *fp, char **output, int *length) {
    struct stat filestats{};
    int fd = fileno(fp);
    fstat(fd, &filestats);
    *length = static_cast<int>(filestats.st_size);
    *output = static_cast<char *>(malloc(*length + 1));
    int start = 0;
    int bytes_read;
    while ((bytes_read = static_cast<int>(fread(*output + start, 1, static_cast<size_t>(*length - start), fp)))) {
        start += bytes_read;
    }
}

void Novel::get_clean_text(const string &res_html) {
    ifstream in(res_html, ios::in | ios::binary);
    if (in) {
        string contents;
        in.seekg(0, ios::end);
        contents.resize(in.tellg());
        in.seekg(0, ios::beg);
        in.read(&contents[0], contents.size());
        in.close();

        GumboOutput *output = gumbo_parse(contents.c_str());
        m_clean_text = clean_text_helper(output->root);
        gumbo_destroy_output(&kGumboDefaultOptions, output);
    }
}

string Novel::clean_text_helper(GumboNode *node) {
    if (node->type == GUMBO_NODE_TEXT) {
        return string(node->v.text.text);
    } else if (node->type == GUMBO_NODE_ELEMENT &&
               node->v.element.tag != GUMBO_TAG_SCRIPT &&
               node->v.element.tag != GUMBO_TAG_STYLE) {
        string contents;
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

const char *Novel::find_title(const GumboNode *root) {
    assert(root->type == GUMBO_NODE_ELEMENT);
    assert(root->v.element.children.length >= 2);

    const GumboVector *root_children = &root->v.element.children;
    GumboNode *head = nullptr;
    for (int i = 0; i < root_children->length; ++i) {
        auto *child = static_cast<GumboNode *>(root_children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT &&
            child->v.element.tag == GUMBO_TAG_HEAD) {
            head = child;
            break;
        }
    }
    assert(head != nullptr);

    GumboVector *head_children = &head->v.element.children;
    for (int i = 0; i < head_children->length; ++i) {
        auto *child = static_cast<GumboNode *>(head_children->data[i]);
        if (child->type == GUMBO_NODE_ELEMENT &&
            child->v.element.tag == GUMBO_TAG_TITLE) {
            if (child->v.element.children.length != 1) {
                return "";//<empty title>
            }
            auto *title_text = static_cast<GumboNode *>(child->v.element.children.data[0]);
            assert(title_text->type == GUMBO_NODE_TEXT || title_text->type == GUMBO_NODE_WHITESPACE);
            return title_text->v.text.text;
        }
    }
    return "";//<no title found>
}

void Novel::read_url(const string &file, vector<string> &res_url) {
    if (!file.empty()) {
        ifstream input(file);

        string url;
        while (getline(input, url)) {
            if (!url.empty())
                res_url.push_back(url);
        }
        input.close();
        if (!res_url.empty()) {
            return;
        }
    }
}

double Novel::evaluate_from_url(const string &url) {
    double ans = 0.0;

    if (!url.empty()) {
        for (auto each_url:url_match) {
            regex r(each_url.first, regex::icase);
            if (regex_search(url, r)) {
                ans += each_url.second;
            }
        }
    }
    return ans;
}

void Novel::evaluate_all_url(vector<string> &url, vector<double> &weight_of_url) {
    if (!url.empty()) {
        for (const auto &each_url:url) {
            weight_of_url.push_back(evaluate_from_url(each_url));
        }
        if (!weight_of_url.empty()) {
            return;
        }
    }
}

double Novel::evaluate_from_clean_text(const string &text) {
    double ans = 0.0;
    //std::cout << text << endl;
    if (!text.empty()) {
        if (text.size() < m_min_text_value) {
            ans -= 0.5;
            return ans;
        }
        auto pre_page = text.find("上一页") || text.find("上一章");
        auto nxt_page = text.find("下一页") || text.find("下一章");
        auto turnpage = text.find("翻页");
        auto b_t_con = text.find("返回目录");
        auto download = text.find("txt下载");
        auto free_read = text.find("免费阅读");
        if (pre_page != string::npos && nxt_page != string::npos) ans += 0.2;
        if (pre_page != string::npos)ans += 0.1;
        if (nxt_page != string::npos)ans += 0.1;
        if (turnpage != string::npos)ans += 0.1;
        if (b_t_con != string::npos)ans += 0.1;
        if (download != string::npos)ans += 0.1;
        if (free_read != string::npos)ans += 0.1;
    }
    return ans;
}

double Novel::evaluate_from_title(const string &title) {
    double ans = 0.0;
    if (!title.empty()) {

        if (title.find("（上）") != string::npos
            || title.find("（下）") != string::npos)
            ans += 0.5;
        if (title.find("《") != string::npos
            && title.find("》") != string::npos
            && title.find("《") < title.find("》"))
            ans += 0.5;

        if (title.find("第") != string::npos
            && title.find("卷") != string::npos
            && title.find("第") < title.find("卷"))
            ans += 0.5;

        if (title.find("第") != string::npos
            && title.find("章") != string::npos
            && title.find("第") < title.find("章"))
            ans += 0.5;

        if (title.find("第") != string::npos
            && title.find("节") != string::npos
            && title.find("第") < title.find("节"))
            ans += 0.5;

        if (title.find("第") != string::npos
            && title.find("回") != string::npos
            && title.find("第") < title.find("回"))
            ans += 0.5;

        if (title.find("章节") != string::npos)
            ans += 0.5;

        if (title.find("卷") != string::npos)
            ans += 0.2;
        if (title.find("章") != string::npos)
            ans += 0.2;
        if (title.find("节") != string::npos)
            ans += 0.2;
        if (title.find("回") != string::npos)
            ans += 0.2;


        if (title.find("小说") != string::npos) ans += 0.5;
        if (title.find("作者") != string::npos) ans += 0.2;
        if (title.find("文学") != string::npos) ans += 0.5;
        if (title.find("中文") != string::npos) ans += 0.3;
        if (title.find("目录") != string::npos) ans += 0.2;
        if (title.find("正文") != string::npos) ans += 0.4;
        if (title.find("番外") != string::npos) ans += 0.5;
        if (title.find("阅读") != string::npos) ans += 0.5;
        if (title.find("txt下载") != string::npos) ans += 0.5;
        if (title.find("TXT下载") != string::npos) ans += 0.5;
        if (title.find("全文下载") != string::npos) ans += 0.5;
        if (title.find("全集") != string::npos) ans += 0.4;
        if (title.find("读") != string::npos) ans += 0.1;
        if (title.find("看") != string::npos) ans += 0.1;
        if (title.find("短篇") != string::npos) ans += 0.4;
        if (title.find("长篇") != string::npos) ans += 0.4;
        if (title.find("都市") != string::npos) ans += 0.4;
        if (title.find("科幻") != string::npos) ans += 0.4;
        if (title.find("武侠") != string::npos) ans += 0.4;
        if (title.find("玄幻") != string::npos) ans += 0.4;
        if (title.find("魔幻") != string::npos) ans += 0.4;
        if (title.find("言情") != string::npos) ans += 0.4;
        if (title.find("肉文") != string::npos) ans += 0.4;
        if (title.find("耽美") != string::npos) ans += 0.4;

        if (title.find("博客") != string::npos) ans -= 0.5;
        if (title.find("漫画") != string::npos) ans -= 0.5;
        if (title.find("论坛") != string::npos) ans -= 0.5;
        if (title.find("新闻") != string::npos) ans -= 0.5;
        if (title.find("视频") != string::npos) ans -= 0.5;
        if (title.find("电影") != string::npos) ans -= 0.5;
        if (title.find("图片") != string::npos) ans -= 0.5;
        if (title.find("游戏") != string::npos) ans -= 0.5;
        if (title.find("直播") != string::npos) ans -= 0.5;
        if (title.find("资讯") != string::npos) ans -= 0.5;
        if (title.find("财经") != string::npos) ans -= 0.5;
        if (title.find("汽车") != string::npos) ans -= 0.5;
        if (title.find("旅游") != string::npos) ans -= 0.5;
        if (title.find("项目") != string::npos) ans -= 0.5;
        if (title.find("咨询") != string::npos) ans -= 0.5;
        if (title.find("查询") != string::npos) ans -= 0.5;
        if (title.find("服务") != string::npos) ans -= 0.5;
        if (title.find("业务") != string::npos) ans -= 0.5;
        if (title.find("产品") != string::npos) ans -= 0.5;
        if (title.find("销售") != string::npos) ans -= 0.5;
        if (title.find("健康") != string::npos) ans -= 0.5;
        if (title.find("保健") != string::npos) ans -= 0.5;
        if (title.find("医疗") != string::npos) ans -= 0.5;
        if (title.find("医院") != string::npos) ans -= 0.5;
        if (title.find("购物") != string::npos) ans -= 0.5;
        if (title.find("品牌") != string::npos) ans -= 0.5;
        if (title.find("价格") != string::npos) ans -= 0.5;
        if (title.find("包邮") != string::npos) ans -= 0.5;
        if (title.find("表演") != string::npos) ans -= 0.5;
        if (title.find("经济") != string::npos) ans -= 0.5;
        if (title.find("百科") != string::npos) ans -= 0.5;
        if (title.find("知道") != string::npos) ans -= 0.5;
        if (title.find("贴吧") != string::npos) ans -= 0.5;
        if (title.find("地图") != string::npos) ans -= 0.5;
    }
    return ans;
}

void Novel::evaluate_all_html(vector<string> &res_html, vector<double> &weight_of_html) {
    if (!res_html.empty()) {
        for (const auto &html_it : res_html) {
            get_title(html_it);
            get_clean_text(html_it);
            if (m_title_in_html.empty()) {
                weight_of_html.push_back(0);
            } else {
                double weight1 = evaluate_from_title(m_title_in_html);
                double weight2 = weight1 == 0.0 ? evaluate_from_clean_text(m_clean_text) : 0.0;
                weight_of_html.push_back(weight1 + weight2);
            }
        }
        if (!weight_of_html.empty()) {
            return;
        }
    }
}

Novel::Novel() {
    m_data_dir = "/home/tian/Workspace/Gumbo-isNovel/novel/";
    m_filePath_yes = m_data_dir + "novel_yes_sample_html/";
    m_filePath_no = m_data_dir + "novel_no_sample_html/";
    m_sample_yes = m_data_dir + "novel_urllist_sample.yes.txt";
    m_sample_no = m_data_dir + "novel_urllist_sample.no.txt";

    //url
    read_url(m_sample_yes, m_yes_url);
    evaluate_all_url(m_yes_url, m_weight_of_yes_url);
    read_url(m_sample_no, m_no_url);
    evaluate_all_url(m_no_url, m_weight_of_no_url);

    //html
    get_files(m_filePath_yes, m_yes_html);
    evaluate_all_html(m_yes_html, m_weight_of_yes_html);
    get_files(m_filePath_no, m_no_html);
    evaluate_all_html(m_no_html, m_weight_of_no_html);

    // take all url and html
    for (double threshold = 0.1; threshold <= 0.9; threshold += 0.01) {
        // cal tp fn fp tn
        for (auto it1 = m_weight_of_yes_url.begin(), it2 = m_weight_of_yes_html.begin();
             it1 != m_weight_of_yes_url.end(); ++it1, ++it2) {
            if (*it1 + *it2 > threshold) {
                m_TP++;
            } else {
                m_FN++;
            }
            m_avg_weight_of_yes += *it1 + *it2;
        }
        m_avg_weight_of_yes /= m_weight_of_yes_html.size();

        for (auto it1 = m_weight_of_no_url.begin(), it2 = m_weight_of_no_html.begin();
             it1 != m_weight_of_no_url.end(); ++it1, ++it2) {
            if (*it1 + *it2 > threshold) {
                m_FP++;
            } else {
                m_TN++;
            }
            m_avg_weight_of_no += *it1 + *it2;
        }
        m_avg_weight_of_no /= m_weight_of_no_html.size();

        m_sample_result.emplace_back(threshold, some_param(m_TP, m_FN, m_FP, m_TP));
        m_TP = 0.0, m_FN = 0.0, m_FP = 0.0, m_TN = 0.0;
    }

    m_maxPercision_at = 0.0;
    m_maxPercision = 0.97;
    for (auto it = m_sample_result.begin(); it != m_sample_result.end(); ++it) {
        double Percision = it->second.tp / (it->second.tp + it->second.fp);
        if (Percision > m_maxPercision) {
            m_maxPercision_at = it->first;
            break;
        }
    }
    m_maxRecall_at = 0.0;
    m_maxRecall = 0.97;
    for (auto it = m_sample_result.rbegin(); it != m_sample_result.rend(); ++it) {
        double Recall = it->second.tp / (it->second.tp + it->second.fn);
        if (Recall > m_maxRecall) {
            m_maxRecall_at = it->first;
            break;
        }
    }

    m_sample_result.clear();

    cout << m_maxPercision_at << ", " << m_maxRecall_at << endl;

    m_threshold = m_maxPercision_at * 0.68 + m_maxRecall_at * 0.32;

    // cal tp fn fp tn
    for (auto it1 = m_weight_of_yes_url.begin(), it2 = m_weight_of_yes_html.begin();
         it1 != m_weight_of_yes_url.end(); ++it1, ++it2) {
        if (*it1 + *it2 > m_threshold) {
            m_TP++;
        } else {
            m_FN++;
        }
        m_avg_weight_of_yes += *it1 + *it2;
    }
    m_avg_weight_of_yes /= m_weight_of_yes_html.size();

    for (auto it1 = m_weight_of_no_url.begin(), it2 = m_weight_of_no_html.begin();
         it1 != m_weight_of_no_url.end(); ++it1, ++it2) {
        if (*it1 + *it2 > m_threshold) {
            m_FP++;
        } else {
            m_TN++;
        }
        m_avg_weight_of_no += *it1 + *it2;
    }
    m_avg_weight_of_no /= m_weight_of_no_html.size();

    m_percision = m_TP / (m_TP + m_FP);
    m_recall = m_TP / (m_TP + m_FN);

    cout << "-------------------------------" << endl;
    cout << "TP \t\t FP \nFN \t\t TN = \n" << m_TP << "\t\t" << m_FP << "\n" << m_FN << "\t\t" << m_TN << endl;
    cout << "-------------------------------" << endl;
    cout << "Percision = \n--- " << m_percision << endl;
    cout << "Recall = \n--- " << m_recall << endl;
    cout << "-------------------------------" << endl;
    cout << "avg weight of = \n--- yes = " << m_avg_weight_of_yes << "\n--- no = " << m_avg_weight_of_no << endl;
    cout << "-------------------------------" << endl;


    ofstream init(m_data_dir + "init.txt");
    init << "-------------------------------" << endl;
    init << "TP \t\t FP \nFN \t\t TN = \n" << m_TP << "\t\t" << m_FP << "\n" << m_FN << "\t\t" << m_TN << endl;
    init << "-------------------------------" << endl;
    init << "Percision = \n--- " << m_percision << endl;
    init << "Recall = \n--- " << m_recall << endl;
    init << "-------------------------------" << endl;
    init << "avg weight of = \n--- yes = " << m_avg_weight_of_yes << "\n--- no = " << m_avg_weight_of_no << endl;
    init << "-------------------------------" << endl;

    init.close();

    m_yes_url.clear();
    m_no_url.clear();
    m_weight_of_yes_url.clear();
    m_weight_of_no_url.clear();

    m_yes_html.clear();
    m_no_html.clear();
    m_weight_of_yes_html.clear();
    m_weight_of_no_html.clear();

    m_avg_weight_of_yes = 0.0;
    m_avg_weight_of_no = 0.0;

    m_TP = 0.0, m_FN = 0.0, m_FP = 0.0, m_TN = 0.0;

    m_some_param.fn = 0.0;
    m_some_param.fp = 0.0;
    m_some_param.tp = 0.0;
    m_some_param.tn = 0.0;

    m_percision = 0.0;
    m_recall = 0.0;
}

bool Novel::cmp(const string &str_a, const string &str_b) {
    auto a_begin = str_a.rfind('/') + 1;
    auto a_end = str_a.rfind(".html");
    auto a_num = stoi(str_a.substr(a_begin, a_end - a_begin));
    auto b_begin = str_b.rfind('/') + 1;
    auto b_end = str_b.rfind(".html");
    auto b_num = stoi(str_b.substr(b_begin, b_end - b_begin));
    return a_num < b_num;
}

void Novel::judgement() {
    cout << "work on all html..." << endl;

    ofstream yes_save(m_data_dir + "yes_save.txt", ios::ate);
    ofstream no_save(m_data_dir + "no_save.txt", ios::ate);

    m_filePath_yes = m_data_dir + "novel_yes_html/";
    m_filePath_no = m_data_dir + "novel_no_html/";


    //url
    read_url(m_data_dir + "novel_urllist.yes.txt", m_yes_url);
    evaluate_all_url(m_yes_url, m_weight_of_yes_url);
    read_url(m_data_dir + "novel_urllist.no.txt", m_no_url);
    evaluate_all_url(m_no_url, m_weight_of_no_url);

    //html
    get_files(m_filePath_yes, m_yes_html);
    evaluate_all_html(m_yes_html, m_weight_of_yes_html);
    get_files(m_filePath_no, m_no_html);
    evaluate_all_html(m_no_html, m_weight_of_no_html);

    bool used;
    int number_yes = 1;
    for (auto it1 = m_weight_of_yes_url.begin(), it2 = m_weight_of_yes_html.begin();
         it1 != m_weight_of_yes_url.end(); ++it1, ++it2) {
        used = (number_yes - 1) % 20 == 0;
        if (*it1 + *it2 > m_threshold) {
            if (!used)m_TP++;
            yes_save << number_yes << "\t" << "yes" << endl;
        } else {
            if (!used)m_FN++;
            yes_save << number_yes << "\t" << "no" << endl;
        }
        number_yes++;
    }


    int number_no = 1;
    for (auto it1 = m_weight_of_no_url.begin(), it2 = m_weight_of_no_html.begin();
         it1 != m_weight_of_no_url.end(); ++it1, ++it2) {
        used = (number_no - 1) % 20 == 0;
        if (*it1 + *it2 > m_threshold) {
            if (!used)m_FP++;
            no_save << number_no << "\t" << "yes" << endl;
        } else {
            if (!used)m_TN++;
            no_save << number_no << "\t" << "no" << endl;
        }
        number_no++;
    }


    m_percision = m_TP / (m_TP + m_FP);
    m_recall = m_TP / (m_TP + m_FN);

    cout << "-------------------------------" << endl;
    cout << "TP \t\t FP \nFN \t\t TN = \n" << m_TP << "\t\t" << m_FP << "\n" << m_FN << "\t\t" << m_TN << endl;
    cout << "-------------------------------" << endl;
    cout << "Percision = \n--- " << m_percision << endl;
    cout << "Recall = \n--- " << m_recall << endl;
    cout << "-------------------------------" << endl;

    ofstream test(m_data_dir + "test_all.txt");
    test << "-------------------------------" << endl;
    test << "TP \t\t FP \nFN \t\t TN = \n" << m_TP << "\t\t" << m_FP << "\n" << m_FN << "\t\t" << m_TN << endl;
    test << "-------------------------------" << endl;
    test << "Percision = \n--- " << m_percision << endl;
    test << "Recall = \n--- " << m_recall << endl;
    test << "-------------------------------" << endl;
    test.close();

    yes_save.close();
    no_save.close();
}







#include "parser/parser.h"

bool is_split_symbol(char ch) {
    vec<char> split_symbols = {
        '+', '-', '*', '/', '^', '@', '|', '&', '!',
        '(', ')', '[', ']', '{', '}', '"',
        ',', '=', '%', ':', '>', '<', ' ' // 空格比较特殊，不计入词元
    };
    for(auto &c : split_symbols) {
        if(c == ch) return true;
    }
    return false;
}
#define M_ADD_TOKEN(type, idx)  { tokens.push_back({type, semi_tokens[idx]}); }

bool is_word_in(str &my_word, const vec<str> &words) {
    for (auto &word : words) {
        if(my_word == word) {
            return true;
        }
    }
    return false;
}
// 分割token
bool split_tokens(const str &_line, vec<SemiToken> &semi_tokens) {
    i32 l = 0;
    str line = _line;
    // 哨兵位，处理最后一个词元
    line += " ";
    for (i32 r = 0; r < line.length(); ++r) {
        if (is_split_symbol(line[r]) == true) {
            // 处理[l, r)，不处理当前符号
            i32 len = r - l; // (r - 1) - l + 1
            // 长度为正则存在token
            if(len > 0) semi_tokens.push_back({l, line.substr(l, len)});
            l = r;
            // 处理注释，但避免跳过了除号处理
            if(r + 1 < line.length() && line[r] == '/' && line[r + 1] == '/') { 
                // 剩下的都是注释，除了最后一个多加的空格，len = line.length() - 1 - r + 1
                semi_tokens.push_back({r, line.substr(r, line.length() - r - 1)});
                break;
            } else if (line[r] == '"') { // 处理字符串字面量
                i32 next_quota_pos = line.find_first_of('"', r + 1);
                // printf("line: %s, l = %d, r = %d\n", line.c_str(), r, next_quota_pos);
                if (next_quota_pos != line.length()) {
                    // 从这个引号到下个引号全是字符串
                    semi_tokens.push_back({r, line.substr(r, next_quota_pos - r + 1)});
                    // printf("(%d, %d) = %s\n", r, next_quota_pos, line.substr(next_quota_pos - r + 1).c_str());
                    r = next_quota_pos; // for循环有自增，此处不应该设置为next_quota_pos + 1
                    l = r + 1;
                } else {
                    std::cerr << "quotation match failed" << std::endl;
                    return false;
                }
            } else {
                if(line[r] != ' ') { // 如果不是空格、注释或者字符串，则将当前符号计入
                    semi_tokens.push_back({r, str(1, line[r])});
                }
                l = r + 1;
            }
        }
    }
    return true;
}

bool Parser::parse(const vec<str> &qs_lines) {
    for(auto &qs_line : qs_lines){
        // 分割词元，形成semi_tokens
        if(split_tokens(qs_line, this->semi_tokens) == false) {
            std::cerr << "lex error" << std::endl; // 后面改成传递parse
            return false;
        }
    }
    // 测试词元分割结果
    // for(auto &semi_token : semi_tokens) {
    //     std::cout << semi_token.val << "|";
    // }
    // std::cout << std::endl;
    for(i32 i = 0; i < semi_tokens.size(); ++i) {
        auto &word = semi_tokens[i].val;
        // 字符串
        if (word.front() == '"' && word.back() == '"') {
            M_ADD_TOKEN(STRING, i);
            continue;
        }
        // 注释
        if (word.length() >= 2 && word[0] == '/' && word[1] == '/') {
            M_ADD_TOKEN(NOTATION, i);
            continue;
        }
        // 符号
        if (word.length() == 1 && is_split_symbol(word[0]) == true) {
            M_ADD_TOKEN(SYMBOL, i);
            continue;
        }
        // 括号关键字
        if (is_word_in(word, {
            "qs_begin", "qs_end",
            "begin", "end",
            "func_begin", "func_end",
        })) {
            M_ADD_TOKEN(BRACE, i);
            continue;
        }
        // 类型关键字
        if (is_word_in(word, {
            "var", "circuit", "stage", "fgate", "ugate"
        })) {
            M_ADD_TOKEN(TYPE, i);
            continue;
        }
        // 功能关键字
        if (is_word_in(word, {
            "run_all", "run_by_stage", "run_by_gate",
            "show"
        })) {
            M_ADD_TOKEN(FUNCTION, i);
            continue;
        }
        // 一般关键字
        if (is_word_in(word, {
            "while", 
            "assign", "declare", "start",
            "state", "new_state", 
            "apply", "on", "at", "call", "atall", "by",
            "upcount", "downcount"
        })) {
            M_ADD_TOKEN(KEYWORD, i);
            continue;
        }
        // if(is_type(word)) {
        //     M_ADD_TOKEN(TYPE, i);
        //     continue;
        // }
        // if(is_keyword(word)) {
        //     M_ADD_TOKEN(KEYWORD, i);
        //     continue;
        // }
        // if(word[0] == '"') {
        //     M_ADD_TOKEN(STRING, i);
        //     continue;
        // }
        // if(is_split_symbol(word[0])) {
        //     M_ADD_TOKEN(SYMBOL, i);
        //     continue;
        // }
        bool is_number = true;
        for(i32 j = 0; j < word.length(); ++j) {
            if(!(word[j] >= '0' && word[j] <= '9') && word[j] != '-' && word[j] != '.') {
                is_number = false;
                break;
            }
        }
        M_ADD_TOKEN((is_number ? NUMBER : VARIABLE), i);
    }
    // 进行脚本执行
    // if(run_script(this) == false) {
    //     std::cerr << "syntax error" << std::endl;
    //     return false;
    // }
    return true;
}

bool Parser::parse(str qs_file_path) {
    // 打开qs源文件
    std::ifstream qs_file(qs_file_path);
    if(qs_file.is_open() == false) {
        std::cerr << std::format("qs file '{}' open failed", qs_file_path) << std::endl;
        return false;
    }
    // 按行读入，每行单独分割，如此可以形成全篇的token列表，同时保持行内position用于渲染
    str qs_line;
    // 用于统计词元分割后未确定类型的词元列表
    vec<str> qs_lines;
    while(std::getline(qs_file, qs_line)) {
        qs_lines.push_back(qs_line);
    }
    return parse(qs_lines);
}
#include "common/common.h"
#include "circuit/circuit.h"

enum TokenType {
    STRING,
    NOTATION,
    SYMBOL,
    BRACE,
    TYPE,
    FUNCTION,
    KEYWORD,
    NUMBER,
    VARIABLE
};

// 未确定类型的词元
struct SemiToken {
    i32 pos;
    str val;
};

struct Token {
    TokenType type;
    i32 pos; // highlightBlock方法只关注行内位置
    size_t size;
    Token(TokenType token_type, SemiToken &semi_token) : type(token_type), pos(semi_token.pos), size(semi_token.val.length()) {}
};

struct Parser {
    // 解析器
    /*
        目标：前后端分别运行，前端编写完保存本地后，点击运行qs脚本，然后后端把qs脚本解析成cpp，编译之后输出结果给后端。
        所以前端单独编译，后端也单独编译，运行文件也单独编译，这样就不会因为一个东西爆掉就彻底卡死。主要是cpp不支持热编译导致的。
        交互接口要约定清楚，数据传输分好几层。
        电路数据：前端/本地文件创建/编辑qs脚本，后端解析为结构化电路数据
        状态数据：后端根据电路数据进行运算，最终结果保存在本地，前端可以读本地结果，也可以读内存中的中间结果
        qs文件数据：保存在本地，前端可以读写qs脚本，后端用parse将其变成一个main中的circuit代码，然后编译成以qs文件名为名的可执行文件，运行途中和后端共享结果变量
    */
    // 解析产生一个cpp文件，链接non_qt_lib，运行产生电路结构体，发给qt端后结束
    // 解析产生一个token序列，对可被识别的单元挨个进行涂色

    // 用于渲染的token序列，发送给QTextEdit
    vec<Token> tokens;
    // 用于存储未定类型的token，parse完成后清空
    vec<SemiToken> semi_tokens;
    // 用于生成图像的电路结构体，发送给QGraphicsView
    Circuit circuit;
    // 解析并执行qs脚本，返回成功与否
    bool parse(str qs_file_path);
    bool parse(const vec<str> &qs_lines);

    bool run_script();
};
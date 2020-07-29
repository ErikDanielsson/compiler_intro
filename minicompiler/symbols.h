

struct VarSymbol {
    char* type;
    char* decl_line;
};

struct StructSymbol {
    struct VarSymbol** fields;
    char** decl_lines;
};

struct FuncSymbol {
    char* type;
    struct VarSymbol** params;
    char** decl_lines;
}

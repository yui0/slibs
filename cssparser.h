/*
 * cssparser.h - Single-header CSS (+ HTML) Parser Library for C
 *
 * No flex/bison dependency. Hand-written recursive descent parser.
 * Supports CSS2.1 + CSS3 common features.
 *
 * USAGE:
 *   #define CSS_PARSER_IMPLEMENTATION
 *   #include "cssparser.h"
 *
 * Copyright © 2026 Yuichiro Nakada / Project Vespera
 * License: Mozilla Public License 2.0
 */

#ifndef CSSPARSER_H
#define CSSPARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>

/* ── Limits ──────────────────────────────────────────────────────────── */
#define CSS_MAX_SELECTORS     8
#define CSS_MAX_PARTS        16
#define CSS_MAX_DECLS       128
#define CSS_MAX_RULES       512
#define CSS_MAX_ATRULES      64
#define CSS_MAX_STR         512
#define CSS_MAX_VALUE      1024

/* ── Enums ───────────────────────────────────────────────────────────── */

typedef enum {
    CSS_SEL_UNIVERSAL = 0,  /* *          */
    CSS_SEL_TYPE,           /* div        */
    CSS_SEL_CLASS,          /* .foo       */
    CSS_SEL_ID,             /* #bar       */
    CSS_SEL_ATTR,           /* [attr]     */
    CSS_SEL_ATTR_EQ,        /* [attr=val] */
    CSS_SEL_ATTR_INCLUDES,  /* [attr~=v]  */
    CSS_SEL_ATTR_DASH,      /* [attr|=v]  */
    CSS_SEL_ATTR_PREFIX,    /* [attr^=v]  */
    CSS_SEL_ATTR_SUFFIX,    /* [attr$=v]  */
    CSS_SEL_ATTR_SUBSTR,    /* [attr*=v]  */
    CSS_SEL_PSEUDO_CLASS,   /* :hover     */
    CSS_SEL_PSEUDO_ELEM,    /* ::before   */
} CSSSelectorPartType;

typedef enum {
    CSS_COMB_NONE = 0,   /* (start)          */
    CSS_COMB_DESC,       /* ' ' descendant   */
    CSS_COMB_CHILD,      /* >  child         */
    CSS_COMB_ADJACENT,   /* +  adjacent      */
    CSS_COMB_SIBLING,    /* ~  general sibling */
} CSSCombinator;

typedef enum {
    CSS_AT_NONE = 0,
    CSS_AT_CHARSET,
    CSS_AT_IMPORT,
    CSS_AT_MEDIA,
    CSS_AT_KEYFRAMES,
    CSS_AT_FONT_FACE,
    CSS_AT_SUPPORTS,
    CSS_AT_PAGE,
    CSS_AT_UNKNOWN,
} CSSAtRuleType;

/* ── Structures ──────────────────────────────────────────────────────── */

/* One simple selector part (e.g., div, .class, #id, [attr=val], :hover) */
typedef struct {
    CSSSelectorPartType type;
    char name[CSS_MAX_STR];       /* tag / class / id / attr name / pseudo */
    char attr_op[4];              /* =, ~=, |=, ^=, $=, *= */
    char attr_val[CSS_MAX_STR];   /* attribute value */
    char pseudo_arg[CSS_MAX_STR]; /* :nth-child(2n+1) argument */
} CSSSelectorPart;

/* One compound selector: sequence of parts with a combinator prefix */
typedef struct {
    CSSCombinator combinator;            /* relation to previous compound */
    CSSSelectorPart parts[CSS_MAX_PARTS];
    int part_count;
    int specificity;                     /* (id<<16)|(cls<<8)|type */
} CSSCompound;

/* One full selector (chain of compounds) */
typedef struct {
    CSSCompound compounds[CSS_MAX_PARTS];
    int compound_count;
    int specificity;
} CSSSelector;

/* One CSS declaration: property + value + !important */
typedef struct {
    char property[CSS_MAX_STR];
    char value[CSS_MAX_VALUE];
    bool important;
} CSSDeclaration;

/* One CSS rule: selector list + declaration block */
typedef struct {
    CSSSelector selectors[CSS_MAX_SELECTORS];
    int selector_count;
    CSSDeclaration decls[CSS_MAX_DECLS];
    int decl_count;
    int specificity; /* max of all selectors */
} CSSRule;

/* One @-rule */
typedef struct {
    CSSAtRuleType type;
    char prelude[CSS_MAX_VALUE];  /* e.g., media query, charset string */
    /* For @media / @keyframes: nested rules */
    CSSRule *nested_rules;
    int nested_rule_count;
    int nested_rule_cap;
} CSSAtRule;

/* Parsed stylesheet */
typedef struct {
    CSSRule  *rules;
    int       rule_count;
    int       rule_cap;
    CSSAtRule *at_rules;
    int        at_rule_count;
    int        at_rule_cap;
    char      *charset;
    /* error info */
    int        error_count;
    char       last_error[256];
    /* CSS custom properties from :root / any rule */
    struct { char name[64]; char value[1024]; } custom_props[128];
    int custom_prop_count;
} CSSStyleSheet;

/* ── Callback-based (SAX-style) API ──────────────────────────────────── */

typedef struct {
    void *user_data;
    void (*on_selector)(void *ud, const CSSSelector *sel, int sel_idx, int sel_count);
    void (*on_declaration)(void *ud, const CSSDeclaration *decl);
    void (*on_rule_start)(void *ud);
    void (*on_rule_end)(void *ud);
    void (*on_at_rule)(void *ud, const CSSAtRule *at);
    void (*on_error)(void *ud, const char *msg, int line);
} CSSCallbacks;

/* ── DOM-style API ───────────────────────────────────────────────────── */

CSSStyleSheet *css_parse(const char *text, size_t len);
CSSStyleSheet *css_parse_file(const char *path);
void           css_free(CSSStyleSheet *sheet);

/* Apply callbacks while parsing (streaming) */
void css_parse_cb(const char *text, size_t len, CSSCallbacks *cb);

/* HTML integration: extract <style> blocks + style="" attributes */
CSSStyleSheet *css_parse_html(const char *html, size_t len);

/* Lookup helpers */
const CSSDeclaration *css_find_decl(const CSSRule *rule, const char *property);

/* Debug */
void css_dump(const CSSStyleSheet *sheet, FILE *fp);

/* Resolve var(--name) references in all declarations using collected custom props */
void css_resolve_vars(CSSStyleSheet *sheet);

#endif /* CSSPARSER_H */

/* ═══════════════════════════════════════════════════════════════════════
 * IMPLEMENTATION
 * ═════════════════════════════════════════════════════════════════════ */
#ifdef CSS_PARSER_IMPLEMENTATION

/* ── Internal lexer ──────────────────────────────────────────────────── */

typedef struct {
    const char *src;
    size_t      len;
    size_t      pos;
    int         line;
} Lexer;

static void lex_init(Lexer *l, const char *src, size_t len) {
    l->src = src;
    l->len = len;
    l->pos = 0;
    l->line = 1;
}

static inline int lex_eof(const Lexer *l) { return l->pos >= l->len; }
static inline char lex_peek(const Lexer *l) {
    return l->pos < l->len ? l->src[l->pos] : '\0';
}
static inline char lex_peek2(const Lexer *l) {
    return (l->pos + 1) < l->len ? l->src[l->pos + 1] : '\0';
}
static inline char lex_advance(Lexer *l) {
    char c = l->src[l->pos++];
    if (c == '\n') l->line++;
    return c;
}

/* Skip whitespace and CSS comments */
static void lex_skip_ws(Lexer *l) {
    while (!lex_eof(l)) {
        char c = lex_peek(l);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f') {
            lex_advance(l);
        } else if (c == '/' && lex_peek2(l) == '*') {
            /* block comment */
            l->pos += 2;
            while (!lex_eof(l)) {
                if (lex_peek(l) == '*' && lex_peek2(l) == '/') {
                    l->pos += 2;
                    break;
                }
                if (lex_advance(l) == '\n') l->line++;
            }
        } else {
            break;
        }
    }
}

/* Read a CSS identifier into buf (max buflen-1 chars). Returns length. */
static int lex_ident(Lexer *l, char *buf, int buflen) {
    int n = 0;
    /* allow leading hyphen(s) for vendor prefixes and custom props */
    while (!lex_eof(l)) {
        char c = lex_peek(l);
        if (c == '\\' && (l->pos + 1) < l->len) {
            /* simple escape: skip backslash, take next char */
            lex_advance(l);
            c = lex_advance(l);
            if (n < buflen - 1) buf[n++] = c;
            continue;
        }
        if (isalnum((unsigned char)c) || c == '-' || c == '_' ||
            (unsigned char)c >= 0x80) {
            if (n < buflen - 1) buf[n++] = c;
            lex_advance(l);
        } else {
            break;
        }
    }
    buf[n] = '\0';
    return n;
}

/* Read a CSS string (quoted). Strips quotes. Returns length. */
static int lex_string(Lexer *l, char *buf, int buflen) {
    char quote = lex_advance(l); /* ' or " */
    int n = 0;
    while (!lex_eof(l)) {
        char c = lex_peek(l);
        if (c == quote) { lex_advance(l); break; }
        if (c == '\\') {
            lex_advance(l);
            if (!lex_eof(l)) {
                c = lex_advance(l);
                if (c == '\n') continue; /* escaped newline = ignore */
                if (n < buflen - 1) buf[n++] = c;
            }
            continue;
        }
        if (n < buflen - 1) buf[n++] = c;
        lex_advance(l);
    }
    buf[n] = '\0';
    return n;
}

/* Read until one of the stop chars (or EOF), respecting parens/quotes.
   Returns chars written (excluding NUL). Stops before the stop char. */
static int lex_until(Lexer *l, char *buf, int buflen, const char *stops) {
    int n = 0;
    int depth = 0;
    while (!lex_eof(l)) {
        char c = lex_peek(l);
        if (depth == 0 && strchr(stops, c)) break;
        if (c == '(') depth++;
        else if (c == ')') { if (depth > 0) depth--; else break; }
        else if (c == '"' || c == '\'') {
            /* read quoted string as a unit */
            char q = c;
            if (n < buflen - 1) buf[n++] = q;
            lex_advance(l);
            while (!lex_eof(l)) {
                char qc = lex_advance(l);
                if (n < buflen - 1) buf[n++] = qc;
                if (qc == q) break;
                if (qc == '\\' && !lex_eof(l)) {
                    qc = lex_advance(l);
                    if (n < buflen - 1) buf[n++] = qc;
                }
            }
            continue;
        } else if (c == '/' && lex_peek2(l) == '*') {
            /* skip comment inside value */
            l->pos += 2;
            while (!lex_eof(l)) {
                if (lex_peek(l) == '*' && lex_peek2(l) == '/') {
                    l->pos += 2; break;
                }
                if (lex_advance(l) == '\n') l->line++;
            }
            continue;
        }
        if (n < buflen - 1) buf[n++] = c;
        lex_advance(l);
    }
    buf[n] = '\0';
    return n;
}

/* ── String utilities ────────────────────────────────────────────────── */

static void str_trim(char *s) {
    if (!s || !*s) return;
    char *e = s + strlen(s) - 1;
    while (e > s && isspace((unsigned char)*e)) *e-- = '\0';
    char *p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
}

static int css_starts_with_ci(const char *s, const char *prefix) {
    while (*prefix) {
        if (tolower((unsigned char)*s) != tolower((unsigned char)*prefix)) return 0;
        s++; prefix++;
    }
    return 1;
}

/* ── Specificity ─────────────────────────────────────────────────────── */

static int calc_specificity(const CSSSelector *sel) {
    int a = 0, b = 0, c = 0;
    for (int i = 0; i < sel->compound_count; i++) {
        const CSSCompound *cmp = &sel->compounds[i];
        for (int j = 0; j < cmp->part_count; j++) {
            const CSSSelectorPart *p = &cmp->parts[j];
            switch (p->type) {
            case CSS_SEL_ID:            a++; break;
            case CSS_SEL_CLASS:
            case CSS_SEL_ATTR:
            case CSS_SEL_ATTR_EQ:
            case CSS_SEL_ATTR_INCLUDES:
            case CSS_SEL_ATTR_DASH:
            case CSS_SEL_ATTR_PREFIX:
            case CSS_SEL_ATTR_SUFFIX:
            case CSS_SEL_ATTR_SUBSTR:
            case CSS_SEL_PSEUDO_CLASS:  b++; break;
            case CSS_SEL_TYPE:          c++; break;
            default: break;
            }
        }
    }
    return (a << 16) | (b << 8) | c;
}

/* ── Selector parser ─────────────────────────────────────────────────── */

/* Forward declaration */
static void parse_rule_block(Lexer *l, CSSRule *rule, CSSCallbacks *cb);

static void parse_selector_part(Lexer *l, CSSSelectorPart *part) {
    char c = lex_peek(l);

    if (c == '*') {
        lex_advance(l);
        part->type = CSS_SEL_UNIVERSAL;
        strcpy(part->name, "*");
    } else if (c == '.') {
        lex_advance(l);
        part->type = CSS_SEL_CLASS;
        lex_ident(l, part->name, CSS_MAX_STR);
    } else if (c == '#') {
        lex_advance(l);
        part->type = CSS_SEL_ID;
        lex_ident(l, part->name, CSS_MAX_STR);
    } else if (c == ':') {
        lex_advance(l);
        bool dbl = (lex_peek(l) == ':');
        if (dbl) lex_advance(l);
        part->type = dbl ? CSS_SEL_PSEUDO_ELEM : CSS_SEL_PSEUDO_CLASS;
        lex_ident(l, part->name, CSS_MAX_STR);
        if (lex_peek(l) == '(') {
            lex_advance(l);
            lex_until(l, part->pseudo_arg, CSS_MAX_STR, ")");
            if (lex_peek(l) == ')') lex_advance(l);
        }
    } else if (c == '[') {
        lex_advance(l);
        lex_skip_ws(l);
        char attr[CSS_MAX_STR] = {0};
        lex_ident(l, attr, CSS_MAX_STR);
        strncpy(part->name, attr, CSS_MAX_STR - 1);
        lex_skip_ws(l);
        char nc = lex_peek(l);
        if (nc == ']') {
            lex_advance(l);
            part->type = CSS_SEL_ATTR;
        } else {
            /* parse operator */
            char op[4] = {0};
            if (nc != '=') {
                op[0] = nc; op[1] = '='; op[2] = '\0';
                lex_advance(l); lex_advance(l);
            } else {
                op[0] = '='; op[1] = '\0';
                lex_advance(l);
            }
            strncpy(part->attr_op, op, 3);
            lex_skip_ws(l);
            if (lex_peek(l) == '"' || lex_peek(l) == '\'') {
                lex_string(l, part->attr_val, CSS_MAX_STR);
            } else {
                lex_ident(l, part->attr_val, CSS_MAX_STR);
            }
            lex_skip_ws(l);
            if (lex_peek(l) == ']') lex_advance(l);

            if      (strcmp(op, "=")  == 0) part->type = CSS_SEL_ATTR_EQ;
            else if (strcmp(op, "~=") == 0) part->type = CSS_SEL_ATTR_INCLUDES;
            else if (strcmp(op, "|=") == 0) part->type = CSS_SEL_ATTR_DASH;
            else if (strcmp(op, "^=") == 0) part->type = CSS_SEL_ATTR_PREFIX;
            else if (strcmp(op, "$=") == 0) part->type = CSS_SEL_ATTR_SUFFIX;
            else if (strcmp(op, "*=") == 0) part->type = CSS_SEL_ATTR_SUBSTR;
            else                            part->type = CSS_SEL_ATTR_EQ;
        }
    } else if (isalpha((unsigned char)c) || c == '_' ||
               (unsigned char)c >= 0x80 || c == '-') {
        part->type = CSS_SEL_TYPE;
        lex_ident(l, part->name, CSS_MAX_STR);
    }
}

/* Parse one complex selector (no commas). Returns 0 on empty. */
static int parse_one_selector(Lexer *l, CSSSelector *sel) {
    memset(sel, 0, sizeof(*sel));
    int ci = 0;

    while (!lex_eof(l)) {
        char c = lex_peek(l);
        /* stop at comma, open brace, or end */
        if (c == ',' || c == '{' || c == ')') break;

        CSSCompound *cmp = &sel->compounds[ci];
        cmp->combinator = (ci == 0) ? CSS_COMB_NONE : CSS_COMB_DESC;

        /* check combinator */
        if (ci > 0) {
            bool had_ws = false;
            /* skip whitespace, detect combinator chars */
            while (!lex_eof(l) && (lex_peek(l) == ' ' || lex_peek(l) == '\t' ||
                   lex_peek(l) == '\r' || lex_peek(l) == '\n')) {
                had_ws = true;
                lex_advance(l);
            }
            c = lex_peek(l);
            if (c == ',' || c == '{' || c == ')') break;
            if (c == '>') { cmp->combinator = CSS_COMB_CHILD;    lex_advance(l); lex_skip_ws(l); }
            else if (c == '+') { cmp->combinator = CSS_COMB_ADJACENT; lex_advance(l); lex_skip_ws(l); }
            else if (c == '~') { cmp->combinator = CSS_COMB_SIBLING;  lex_advance(l); lex_skip_ws(l); }
            else if (had_ws) { cmp->combinator = CSS_COMB_DESC; }
        }

        /* parse parts of this compound */
        int pi = 0;
        c = lex_peek(l);
        while (!lex_eof(l) && c != ',' && c != '{' && c != ')' &&
               c != ' ' && c != '\t' && c != '\n' && c != '\r' &&
               c != '>' && c != '+' && c != '~') {
            if (pi >= CSS_MAX_PARTS) break;
            parse_selector_part(l, &cmp->parts[pi++]);
            c = lex_peek(l);
        }
        cmp->part_count = pi;

        /* compute compound specificity */
        int sa = 0, sb = 0, sc = 0;
        for (int i = 0; i < pi; i++) {
            switch (cmp->parts[i].type) {
            case CSS_SEL_ID: sa++; break;
            case CSS_SEL_CLASS: case CSS_SEL_ATTR: case CSS_SEL_ATTR_EQ:
            case CSS_SEL_ATTR_INCLUDES: case CSS_SEL_ATTR_DASH:
            case CSS_SEL_ATTR_PREFIX: case CSS_SEL_ATTR_SUFFIX:
            case CSS_SEL_ATTR_SUBSTR: case CSS_SEL_PSEUDO_CLASS: sb++; break;
            case CSS_SEL_TYPE: sc++; break;
            default: break;
            }
        }
        cmp->specificity = (sa << 16) | (sb << 8) | sc;

        if (pi == 0) break;
        if (++ci >= CSS_MAX_PARTS) break;
    }

    sel->compound_count = ci;
    sel->specificity = calc_specificity(sel);
    return ci > 0 || (sel->compound_count == 1 && sel->compounds[0].part_count == 0) ? 1 : 0;
}

/* Parse selector list (comma-separated). Returns count. */
static int parse_selector_list(Lexer *l, CSSSelector *sels, int max) {
    int count = 0;
    while (!lex_eof(l) && lex_peek(l) != '{') {
        lex_skip_ws(l);
        if (lex_peek(l) == '{') break;
        if (count >= max) {
            /* skip remaining selectors */
            while (!lex_eof(l) && lex_peek(l) != '{') lex_advance(l);
            break;
        }
        if (parse_one_selector(l, &sels[count])) count++;
        lex_skip_ws(l);
        if (lex_peek(l) == ',') lex_advance(l);
    }
    return count;
}

/* ── Declaration parser ──────────────────────────────────────────────── */

static void parse_declaration(Lexer *l, CSSDeclaration *decl) {
    memset(decl, 0, sizeof(*decl));
    lex_skip_ws(l);
    lex_ident(l, decl->property, CSS_MAX_STR);
    /* handle custom properties --foo */
    if (decl->property[0] == '\0') {
        /* try to pick up -- prefix */
        if (lex_peek(l) == '-' && lex_peek2(l) == '-') {
            int n = 0;
            while (!lex_eof(l) && lex_peek(l) != ':' && n < CSS_MAX_STR - 1)
                decl->property[n++] = lex_advance(l);
            decl->property[n] = '\0';
        }
    }
    lex_skip_ws(l);
    if (lex_peek(l) != ':') return;
    lex_advance(l); /* consume ':' */
    lex_skip_ws(l);

    /* read value until ';' or '}' */
    char raw[CSS_MAX_VALUE] = {0};
    lex_until(l, raw, CSS_MAX_VALUE, ";}");
    str_trim(raw);

    /* detect !important */
    char *imp = strstr(raw, "!important");
    if (!imp) imp = strstr(raw, "! important");
    if (imp) {
        decl->important = true;
        *imp = '\0';
        str_trim(raw);
    }
    strncpy(decl->value, raw, CSS_MAX_VALUE - 1);
    str_trim(decl->value);
}

static void parse_rule_block(Lexer *l, CSSRule *rule, CSSCallbacks *cb) {
    if (lex_peek(l) != '{') return;
    lex_advance(l); /* consume '{' */

    while (!lex_eof(l)) {
        lex_skip_ws(l);
        char c = lex_peek(l);
        if (c == '}') { lex_advance(l); break; }
        if (c == ';') { lex_advance(l); continue; }
        if (c == '\0') break;

        if (rule->decl_count < CSS_MAX_DECLS) {
            CSSDeclaration *d = &rule->decls[rule->decl_count];
            parse_declaration(l, d);
            if (d->property[0]) {
                if (cb && cb->on_declaration) cb->on_declaration(cb->user_data, d);
                rule->decl_count++;
            }
        } else {
            /* skip remaining */
            lex_until(l, (char[CSS_MAX_VALUE]){0}, CSS_MAX_VALUE, "}");
            break;
        }
        lex_skip_ws(l);
        if (lex_peek(l) == ';') lex_advance(l);
    }
}

/* ── @rule parser ────────────────────────────────────────────────────── */

static CSSAtRuleType classify_at(const char *kw) {
    if (css_starts_with_ci(kw, "charset"))    return CSS_AT_CHARSET;
    if (css_starts_with_ci(kw, "import"))     return CSS_AT_IMPORT;
    if (css_starts_with_ci(kw, "media"))      return CSS_AT_MEDIA;
    if (css_starts_with_ci(kw, "keyframes") ||
        css_starts_with_ci(kw, "-webkit-keyframes") ||
        css_starts_with_ci(kw, "-moz-keyframes")) return CSS_AT_KEYFRAMES;
    if (css_starts_with_ci(kw, "font-face")) return CSS_AT_FONT_FACE;
    if (css_starts_with_ci(kw, "supports"))  return CSS_AT_SUPPORTS;
    if (css_starts_with_ci(kw, "page"))       return CSS_AT_PAGE;
    return CSS_AT_UNKNOWN;
}

/* Skip a {...} block including nested braces */
static void skip_block(Lexer *l) {
    if (lex_peek(l) != '{') return;
    lex_advance(l);
    int depth = 1;
    while (!lex_eof(l) && depth > 0) {
        char c = lex_advance(l);
        if (c == '{') depth++;
        else if (c == '}') depth--;
        else if (c == '"' || c == '\'') {
            /* skip string */
            char q = c;
            while (!lex_eof(l)) {
                c = lex_advance(l);
                if (c == q) break;
                if (c == '\\' && !lex_eof(l)) lex_advance(l);
            }
        }
    }
}

/* ── Main stylesheet parser ──────────────────────────────────────────── */

static void sheet_add_rule(CSSStyleSheet *sheet, const CSSRule *rule) {
    if (sheet->rule_count >= sheet->rule_cap) {
        sheet->rule_cap = sheet->rule_cap ? sheet->rule_cap * 2 : 64;
        sheet->rules = realloc(sheet->rules, sheet->rule_cap * sizeof(CSSRule));
    }
    sheet->rules[sheet->rule_count++] = *rule;
}

static void sheet_add_at(CSSStyleSheet *sheet, const CSSAtRule *at) {
    if (sheet->at_rule_count >= sheet->at_rule_cap) {
        sheet->at_rule_cap = sheet->at_rule_cap ? sheet->at_rule_cap * 2 : 16;
        sheet->at_rules = realloc(sheet->at_rules, sheet->at_rule_cap * sizeof(CSSAtRule));
    }
    sheet->at_rules[sheet->at_rule_count++] = *at;
}

static void parse_stylesheet_inner(Lexer *l, CSSStyleSheet *sheet, CSSCallbacks *cb) {
    while (!lex_eof(l)) {
        lex_skip_ws(l);
        if (lex_eof(l)) break;

        char c = lex_peek(l);

        /* CDO/CDC (HTML comment markers in CSS) */
        if (c == '<' && (l->pos + 3 < l->len) &&
            memcmp(l->src + l->pos, "<!--", 4) == 0) { l->pos += 4; continue; }
        if (c == '-' && (l->pos + 2 < l->len) &&
            memcmp(l->src + l->pos, "-->", 3) == 0)  { l->pos += 3; continue; }

        if (c == '@') {
            lex_advance(l); /* consume '@' */
            char kw[128] = {0};
            lex_ident(l, kw, 128);
            lex_skip_ws(l);

            CSSAtRule at = {0};
            at.type = classify_at(kw);

            if (at.type == CSS_AT_CHARSET) {
                if (lex_peek(l) == '"' || lex_peek(l) == '\'') {
                    lex_string(l, at.prelude, CSS_MAX_VALUE);
                    if (!sheet->charset) sheet->charset = strdup(at.prelude);
                }
                lex_skip_ws(l);
                if (lex_peek(l) == ';') lex_advance(l);
                if (cb && cb->on_at_rule) cb->on_at_rule(cb->user_data, &at);
                sheet_add_at(sheet, &at);
                continue;
            }

            if (at.type == CSS_AT_IMPORT) {
                char tmp[CSS_MAX_VALUE] = {0};
                lex_until(l, tmp, CSS_MAX_VALUE, ";{");
                str_trim(tmp);
                strncpy(at.prelude, tmp, CSS_MAX_VALUE - 1);
                if (lex_peek(l) == ';') lex_advance(l);
                if (cb && cb->on_at_rule) cb->on_at_rule(cb->user_data, &at);
                sheet_add_at(sheet, &at);
                continue;
            }

            if (at.type == CSS_AT_MEDIA || at.type == CSS_AT_SUPPORTS) {
                /* read prelude until '{' */
                char tmp[CSS_MAX_VALUE] = {0};
                lex_until(l, tmp, CSS_MAX_VALUE, "{");
                str_trim(tmp);
                strncpy(at.prelude, tmp, CSS_MAX_VALUE - 1);
                lex_skip_ws(l);
                if (lex_peek(l) == '{') {
                    lex_advance(l);
                    at.nested_rule_cap = 16;
                    at.nested_rules = calloc(at.nested_rule_cap, sizeof(CSSRule));
                    at.nested_rule_count = 0;
                    while (!lex_eof(l)) {
                        lex_skip_ws(l);
                        if (lex_peek(l) == '}') { lex_advance(l); break; }
                        if (lex_peek(l) == '@') { skip_block(l); continue; }
                        CSSRule inner = {0};
                        inner.selector_count = parse_selector_list(l, inner.selectors, CSS_MAX_SELECTORS);
                        lex_skip_ws(l);
                        if (lex_peek(l) == '{') {
                            parse_rule_block(l, &inner, NULL);
                            if (at.nested_rule_count >= at.nested_rule_cap) {
                                at.nested_rule_cap *= 2;
                                at.nested_rules = realloc(at.nested_rules,
                                    at.nested_rule_cap * sizeof(CSSRule));
                            }
                            at.nested_rules[at.nested_rule_count++] = inner;
                        } else {
                            /* skip garbage */
                            while (!lex_eof(l) && lex_peek(l) != '}' && lex_peek(l) != '{')
                                lex_advance(l);
                        }
                    }
                }
                if (cb && cb->on_at_rule) cb->on_at_rule(cb->user_data, &at);
                sheet_add_at(sheet, &at);
                continue;
            }

            if (at.type == CSS_AT_KEYFRAMES) {
                char tmp[CSS_MAX_VALUE] = {0};
                lex_until(l, tmp, CSS_MAX_VALUE, "{");
                str_trim(tmp);
                strncpy(at.prelude, tmp, CSS_MAX_VALUE - 1);
                lex_skip_ws(l);
                /* Parse keyframe blocks as nested rules with stop selectors */
                if (lex_peek(l) == '{') {
                    lex_advance(l);
                    at.nested_rule_cap = 16;
                    at.nested_rules = calloc(at.nested_rule_cap, sizeof(CSSRule));
                    while (!lex_eof(l)) {
                        lex_skip_ws(l);
                        if (lex_peek(l) == '}') { lex_advance(l); break; }
                        CSSRule kf = {0};
                        /* keyframe selector: from / to / percentage */
                        char stop_sel[CSS_MAX_STR] = {0};
                        lex_until(l, stop_sel, CSS_MAX_STR, "{");
                        str_trim(stop_sel);
                        /* store as type selector for simplicity */
                        if (kf.selector_count < CSS_MAX_SELECTORS) {
                            strncpy(kf.selectors[0].compounds[0].parts[0].name, stop_sel, CSS_MAX_STR - 1);
                            kf.selectors[0].compounds[0].parts[0].type = CSS_SEL_TYPE;
                            kf.selectors[0].compounds[0].part_count = 1;
                            kf.selectors[0].compound_count = 1;
                            kf.selector_count = 1;
                        }
                        lex_skip_ws(l);
                        if (lex_peek(l) == '{') parse_rule_block(l, &kf, NULL);
                        if (at.nested_rule_count >= at.nested_rule_cap) {
                            at.nested_rule_cap *= 2;
                            at.nested_rules = realloc(at.nested_rules,
                                at.nested_rule_cap * sizeof(CSSRule));
                        }
                        at.nested_rules[at.nested_rule_count++] = kf;
                    }
                }
                if (cb && cb->on_at_rule) cb->on_at_rule(cb->user_data, &at);
                sheet_add_at(sheet, &at);
                continue;
            }

            if (at.type == CSS_AT_FONT_FACE || at.type == CSS_AT_PAGE) {
                char tmp[CSS_MAX_VALUE] = {0};
                lex_until(l, tmp, CSS_MAX_VALUE, "{");
                str_trim(tmp);
                strncpy(at.prelude, tmp, CSS_MAX_VALUE - 1);
                lex_skip_ws(l);
                CSSRule inner = {0};
                if (lex_peek(l) == '{') parse_rule_block(l, &inner, NULL);
                at.nested_rule_cap = 1;
                at.nested_rules = malloc(sizeof(CSSRule));
                at.nested_rules[0] = inner;
                at.nested_rule_count = 1;
                if (cb && cb->on_at_rule) cb->on_at_rule(cb->user_data, &at);
                sheet_add_at(sheet, &at);
                continue;
            }

            /* Unknown @rule: skip to ';' or end of block */
            char tmp[CSS_MAX_VALUE] = {0};
            lex_until(l, tmp, CSS_MAX_VALUE, ";{");
            str_trim(tmp); strncpy(at.prelude, tmp, CSS_MAX_VALUE - 1);
            if (!lex_eof(l)) {
                if (lex_peek(l) == ';') lex_advance(l);
                else if (lex_peek(l) == '{') skip_block(l);
            }
            sheet_add_at(sheet, &at);
            continue;
        }

        /* Normal rule */
        CSSRule rule = {0};
        if (cb && cb->on_rule_start) cb->on_rule_start(cb->user_data);

        rule.selector_count = parse_selector_list(l, rule.selectors, CSS_MAX_SELECTORS);
        lex_skip_ws(l);

        if (lex_peek(l) == '{') {
            /* emit selectors */
            if (cb && cb->on_selector) {
                for (int i = 0; i < rule.selector_count; i++)
                    cb->on_selector(cb->user_data, &rule.selectors[i], i, rule.selector_count);
            }
            parse_rule_block(l, &rule, cb);

            /* compute max specificity */
            for (int i = 0; i < rule.selector_count; i++) {
                if (rule.selectors[i].specificity > rule.specificity)
                    rule.specificity = rule.selectors[i].specificity;
            }

            if (rule.selector_count > 0 || rule.decl_count > 0)
                sheet_add_rule(sheet, &rule);
        } else {
            /* malformed: skip to next brace or semicolon */
            while (!lex_eof(l) && lex_peek(l) != '{' && lex_peek(l) != ';')
                lex_advance(l);
            if (!lex_eof(l)) {
                if (lex_peek(l) == '{') skip_block(l);
                else lex_advance(l);
            }
        }

        if (cb && cb->on_rule_end) cb->on_rule_end(cb->user_data);
    }
}

/* ── CSS custom property resolver ───────────────────────────────── */

static void css_collect_custom_props(CSSStyleSheet *sheet) {
    sheet->custom_prop_count = 0;
    /* Pass 1: collect all --name: value declarations */
    for (int ri = 0; ri < sheet->rule_count; ri++) {
        CSSRule *rule = &sheet->rules[ri];
        for (int di = 0; di < rule->decl_count; di++) {
            const char *prop = rule->decls[di].property;
            if (prop[0] == '-' && prop[1] == '-') {
                int found = 0;
                for (int ci = 0; ci < sheet->custom_prop_count; ci++) {
                    if (strcmp(sheet->custom_props[ci].name, prop) == 0) {
                        strncpy(sheet->custom_props[ci].value, rule->decls[di].value, 1023);
                        sheet->custom_props[ci].value[1023] = '\0';
                        found = 1; break;
                    }
                }
                if (!found && sheet->custom_prop_count < 128) {
                    strncpy(sheet->custom_props[sheet->custom_prop_count].name, prop, 63);
                    sheet->custom_props[sheet->custom_prop_count].name[63] = '\0';
                    strncpy(sheet->custom_props[sheet->custom_prop_count].value, rule->decls[di].value, 1023);
                    sheet->custom_props[sheet->custom_prop_count].value[1023] = '\0';
                    sheet->custom_prop_count++;
                }
            }
        }
    }
    /* Also check nested (at-rule) rules */
    for (int ai = 0; ai < sheet->at_rule_count; ai++) {
        CSSAtRule *at = &sheet->at_rules[ai];
        for (int ri2 = 0; ri2 < at->nested_rule_count; ri2++) {
            CSSRule *rule = &at->nested_rules[ri2];
            for (int di = 0; di < rule->decl_count; di++) {
                const char *prop = rule->decls[di].property;
                if (prop[0] == '-' && prop[1] == '-') {
                    int found = 0;
                    for (int ci = 0; ci < sheet->custom_prop_count; ci++) {
                        if (strcmp(sheet->custom_props[ci].name, prop) == 0) {
                            strncpy(sheet->custom_props[ci].value, rule->decls[di].value, 1023);
                            found = 1; break;
                        }
                    }
                    if (!found && sheet->custom_prop_count < 128) {
                        strncpy(sheet->custom_props[sheet->custom_prop_count].name, prop, 63);
                        sheet->custom_props[sheet->custom_prop_count].name[63] = '\0';
                        strncpy(sheet->custom_props[sheet->custom_prop_count].value, rule->decls[di].value, 1023);
                        sheet->custom_prop_count++;
                    }
                }
            }
        }
    }
}

static void resolve_var_in_value(const CSSStyleSheet *sheet, char *value, int maxlen) {
    if (!strstr(value, "var(")) return;
    char out[1024] = {0};
    size_t oi = 0;
    const char *p = value;
    while (*p && oi < (size_t)(maxlen - 1)) {
        if (strncmp(p, "var(", 4) == 0) {
            const char *ns = p + 4;
            while (*ns == ' ') ns++;
            const char *ne = ns;
            while (*ne && *ne != ',' && *ne != ')') ne++;
            char vname[64] = {0};
            int nl = (int)(ne - ns);
            if (nl > 63) nl = 63;
            strncpy(vname, ns, nl);
            /* trim trailing spaces */
            int vl = (int)strlen(vname);
            while (vl > 0 && vname[vl-1] == ' ') vname[--vl] = '\0';
            /* find value */
            const char *found = NULL;
            for (int i = 0; i < sheet->custom_prop_count; i++) {
                if (strcmp(sheet->custom_props[i].name, vname) == 0) {
                    found = sheet->custom_props[i].value; break;
                }
            }
            /* skip to closing paren */
            int depth = 1; p += 4;
            const char *fallback = NULL;
            while (*p && depth > 0) {
                if (*p == '(') depth++;
                else if (*p == ')') { depth--; if (depth == 0) break; }
                else if (*p == ',' && depth == 1 && !fallback) fallback = p + 1;
                p++;
            }
            if (*p == ')') p++;
            if (found) {
                size_t fl = strlen(found);
                if (oi + fl >= (size_t)(maxlen - 1)) fl = (size_t)(maxlen - 1) - oi;
                memcpy(out + oi, found, fl);
                oi += fl;
            } else if (fallback) {
                /* use fallback: trim and copy */
                const char *fb = fallback;
                while (*fb == ' ') fb++;
                /* end is at p (char after the ')' we already skipped) */
                size_t fbl = (size_t)(p - 1 - fallback); /* approx */
                if (fbl > 0 && fbl < 511) {
                    char fbbuf[512] = {0};
                    strncpy(fbbuf, fallback, fbl); fbbuf[fbl] = '\0';
                    /* basic trim */
                    int ftl = (int)strlen(fbbuf);
                    while (ftl > 0 && (fbbuf[ftl-1] == ')' || fbbuf[ftl-1] == ' ')) fbbuf[--ftl] = '\0';
                    size_t rfl = strlen(fbbuf);
                    if (oi + rfl >= (size_t)(maxlen - 1)) rfl = (size_t)(maxlen - 1) - oi;
                    memcpy(out + oi, fbbuf, rfl);
                    oi += rfl;
                }
            }
        } else {
            out[oi++] = *p++;
        }
    }
    out[oi] = '\0';
    strncpy(value, out, (size_t)(maxlen - 1));
    value[maxlen - 1] = '\0';
}

void css_resolve_vars(CSSStyleSheet *sheet) {
    if (!sheet) return;
    css_collect_custom_props(sheet);
    /* Pass 2: resolve var() in all non-custom declarations */
    for (int ri = 0; ri < sheet->rule_count; ri++) {
        CSSRule *rule = &sheet->rules[ri];
        for (int di = 0; di < rule->decl_count; di++) {
            if (rule->decls[di].property[0] == '-' && rule->decls[di].property[1] == '-') continue;
            resolve_var_in_value(sheet, rule->decls[di].value, CSS_MAX_VALUE);
        }
    }
    for (int ai = 0; ai < sheet->at_rule_count; ai++) {
        CSSAtRule *at = &sheet->at_rules[ai];
        for (int ri2 = 0; ri2 < at->nested_rule_count; ri2++) {
            CSSRule *rule = &at->nested_rules[ri2];
            for (int di = 0; di < rule->decl_count; di++) {
                if (rule->decls[di].property[0] == '-' && rule->decls[di].property[1] == '-') continue;
                resolve_var_in_value(sheet, rule->decls[di].value, CSS_MAX_VALUE);
            }
        }
    }
}

/* ── Public DOM API ──────────────────────────────────────────────────── */

CSSStyleSheet *css_parse(const char *text, size_t len) {
    if (!text) return NULL;
    if (len == 0) len = strlen(text);
    CSSStyleSheet *sheet = calloc(1, sizeof(CSSStyleSheet));
    Lexer l;
    lex_init(&l, text, len);
    parse_stylesheet_inner(&l, sheet, NULL);
    css_resolve_vars(sheet);
    return sheet;
}

CSSStyleSheet *css_parse_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    rewind(f);
    char *buf = malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    CSSStyleSheet *sheet = css_parse(buf, (size_t)sz);
    free(buf);
    return sheet;
}

void css_parse_cb(const char *text, size_t len, CSSCallbacks *cb) {
    if (!text || !cb) return;
    if (len == 0) len = strlen(text);
    CSSStyleSheet *sheet = calloc(1, sizeof(CSSStyleSheet));
    Lexer l;
    lex_init(&l, text, len);
    parse_stylesheet_inner(&l, sheet, cb);
    css_free(sheet);
}

void css_free(CSSStyleSheet *sheet) {
    if (!sheet) return;
    for (int i = 0; i < sheet->at_rule_count; i++) {
        free(sheet->at_rules[i].nested_rules);
    }
    free(sheet->rules);
    free(sheet->at_rules);
    free(sheet->charset);
    free(sheet);
}

const CSSDeclaration *css_find_decl(const CSSRule *rule, const char *property) {
    for (int i = 0; i < rule->decl_count; i++) {
        if (strcmp(rule->decls[i].property, property) == 0)
            return &rule->decls[i];
    }
    return NULL;
}

/* ── HTML integration ────────────────────────────────────────────────── */

/* Extract text content of all <style> blocks from HTML.
   Also parse inline style="" attributes as individual rules. */
CSSStyleSheet *css_parse_html(const char *html, size_t len) {
    if (!html) return NULL;
    if (len == 0) len = strlen(html);

    /* Accumulate CSS text */
    size_t css_cap = 4096, css_len = 0;
    char *css_buf = malloc(css_cap);
    if (!css_buf) return NULL;
    css_buf[0] = '\0';

    const char *p = html;
    const char *end = html + len;

    while (p < end) {
        /* Find <style or <STYLE */
        const char *tag = p;
        while (tag < end) {
            if (*tag == '<') {
                if ((end - tag) >= 7 &&
                    (strncasecmp(tag, "<style", 6) == 0) &&
                    (tag[6] == '>' || tag[6] == ' ' || tag[6] == '\t' ||
                     tag[6] == '\r' || tag[6] == '\n')) break;
            }
            tag++;
        }
        if (tag >= end) break;

        /* skip to end of opening tag */
        const char *content_start = tag;
        while (content_start < end && *content_start != '>') content_start++;
        if (content_start < end) content_start++; /* skip '>' */

        /* find </style> */
        const char *content_end = content_start;
        while (content_end < end) {
            if ((end - content_end) >= 8 &&
                strncasecmp(content_end, "</style", 7) == 0) break;
            content_end++;
        }

        size_t block_len = (size_t)(content_end - content_start);
        if (css_len + block_len + 2 > css_cap) {
            css_cap = (css_len + block_len + 2) * 2;
            css_buf = realloc(css_buf, css_cap);
        }
        memcpy(css_buf + css_len, content_start, block_len);
        css_len += block_len;
        css_buf[css_len++] = '\n';
        css_buf[css_len] = '\0';

        p = content_end;
        /* skip </style> */
        while (p < end && *p != '>') p++;
        if (p < end) p++;
    }

    /* Also parse inline style="" attributes */
    /* We synthesize rules like: [data-inline-N] { ... } */
    p = html;
    int inline_idx = 0;
    while (p < end) {
        /* find style=" */
        const char *attr = p;
        while (attr < end) {
            if ((end - attr) >= 7 &&
                strncasecmp(attr, "style=", 6) == 0 &&
                (attr[6] == '"' || attr[6] == '\'')) {
                break;
            }
            attr++;
        }
        if (attr >= end) break;

        /* back-track to find element tag name */
        const char *tag_start = attr - 1;
        while (tag_start > html && *tag_start != '<') tag_start--;
        char elem[64] = {0};
        if (*tag_start == '<') {
            const char *ts = tag_start + 1;
            while (*ts && !isspace((unsigned char)*ts) && *ts != '>' && *ts != '/')
                ts++;
            size_t tl = (size_t)(ts - (tag_start + 1));
            if (tl > 63) tl = 63;
            memcpy(elem, tag_start + 1, tl);
            elem[tl] = '\0';
        }

        char quote = attr[6];
        const char *val_start = attr + 7;
        const char *val_end = val_start;
        while (val_end < end && *val_end != quote) val_end++;

        /* synthesize: elem[data-inline-N] { ... } */
        size_t val_len = (size_t)(val_end - val_start);
        char rule_text[CSS_MAX_VALUE * 2];
        snprintf(rule_text, sizeof(rule_text) - 1,
                 "%s[data-inline-%d] { %.*s }\n",
                 elem[0] ? elem : "*", inline_idx++,
                 (int)val_len, val_start);

        size_t rl = strlen(rule_text);
        if (css_len + rl + 2 > css_cap) {
            css_cap = (css_len + rl + 2) * 2;
            css_buf = realloc(css_buf, css_cap);
        }
        memcpy(css_buf + css_len, rule_text, rl);
        css_len += rl;
        css_buf[css_len] = '\0';

        p = val_end + 1;
    }

    CSSStyleSheet *sheet = NULL;
    if (css_len > 0) sheet = css_parse(css_buf, css_len);
    free(css_buf);
    return sheet;
}

/* ── Debug dump ──────────────────────────────────────────────────────── */

static const char *comb_str(CSSCombinator c) {
    switch (c) {
    case CSS_COMB_DESC:     return " ";
    case CSS_COMB_CHILD:    return " > ";
    case CSS_COMB_ADJACENT: return " + ";
    case CSS_COMB_SIBLING:  return " ~ ";
    default: return "";
    }
}

static void dump_selector(const CSSSelector *sel, FILE *fp) {
    for (int ci = 0; ci < sel->compound_count; ci++) {
        const CSSCompound *cmp = &sel->compounds[ci];
        if (ci > 0) fputs(comb_str(cmp->combinator), fp);
        for (int pi = 0; pi < cmp->part_count; pi++) {
            const CSSSelectorPart *p = &cmp->parts[pi];
            switch (p->type) {
            case CSS_SEL_UNIVERSAL:    fputc('*', fp); break;
            case CSS_SEL_TYPE:         fputs(p->name, fp); break;
            case CSS_SEL_CLASS:        fprintf(fp, ".%s", p->name); break;
            case CSS_SEL_ID:           fprintf(fp, "#%s", p->name); break;
            case CSS_SEL_ATTR:         fprintf(fp, "[%s]", p->name); break;
            case CSS_SEL_ATTR_EQ:      fprintf(fp, "[%s=%s]", p->name, p->attr_val); break;
            case CSS_SEL_ATTR_INCLUDES:fprintf(fp, "[%s~=%s]", p->name, p->attr_val); break;
            case CSS_SEL_ATTR_DASH:    fprintf(fp, "[%s|=%s]", p->name, p->attr_val); break;
            case CSS_SEL_ATTR_PREFIX:  fprintf(fp, "[%s^=%s]", p->name, p->attr_val); break;
            case CSS_SEL_ATTR_SUFFIX:  fprintf(fp, "[%s$=%s]", p->name, p->attr_val); break;
            case CSS_SEL_ATTR_SUBSTR:  fprintf(fp, "[%s*=%s]", p->name, p->attr_val); break;
            case CSS_SEL_PSEUDO_CLASS: {
                fprintf(fp, ":%s", p->name);
                if (p->pseudo_arg[0]) fprintf(fp, "(%s)", p->pseudo_arg);
                break;
            }
            case CSS_SEL_PSEUDO_ELEM:  fprintf(fp, "::%s", p->name); break;
            }
        }
    }
}

void css_dump(const CSSStyleSheet *sheet, FILE *fp) {
    if (!sheet || !fp) return;
    if (sheet->charset) fprintf(fp, "@charset \"%s\";\n\n", sheet->charset);

    for (int i = 0; i < sheet->at_rule_count; i++) {
        const CSSAtRule *at = &sheet->at_rules[i];
        fprintf(fp, "@at{type=%d prelude=\"%s\"}\n", at->type, at->prelude);
    }

    for (int i = 0; i < sheet->rule_count; i++) {
        const CSSRule *r = &sheet->rules[i];
        for (int si = 0; si < r->selector_count; si++) {
            if (si) fputs(", ", fp);
            dump_selector(&r->selectors[si], fp);
        }
        fputs(" {\n", fp);
        for (int di = 0; di < r->decl_count; di++) {
            fprintf(fp, "  %s: %s%s;\n",
                    r->decls[di].property,
                    r->decls[di].value,
                    r->decls[di].important ? " !important" : "");
        }
        fputs("}\n", fp);
    }
    fprintf(fp, "/* %d rules, %d @rules */\n", sheet->rule_count, sheet->at_rule_count);
}

#endif /* CSS_PARSER_IMPLEMENTATION */
